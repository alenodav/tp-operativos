#include <main.h>

t_log *logger;
t_list* instrucciones;
t_memoria tmemoria;

int main(int argc, char* argv[]) {
    t_config *config = crear_config("memoria");
    logger = crear_log(config, "memoria");
    log_debug(logger, "Config y Logger creados correctamente.");

    uint32_t fd_escucha_memoria = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"));

    pthread_t thread_escucha_cpu;
    pthread_create(&thread_escucha_cpu, NULL, handshake_cpu, fd_escucha_memoria);
    pthread_detach(thread_escucha_cpu);
   
    pthread_t thread_escucha_kernel;
    pthread_create(&thread_escucha_kernel, NULL, handshake_kernel, fd_escucha_memoria);
    pthread_detach(thread_escucha_kernel);

    getchar();

    liberar_conexion(fd_escucha_memoria);
    config_destroy(config);
    log_info(logger, "Finalizo el proceso.");
    log_destroy(logger);

}

void handshake_kernel(uint32_t fd_escucha_memoria) {
    uint32_t cliente = esperar_cliente(fd_escucha_memoria);
    
    char* identificador = recibir_handshake(cliente);

    if (string_equals_ignore_case(identificador, "KERNEL")) {
        log_debug(logger, "Handshake Kernel a Memoria OK.");
    }
    else {
        log_error(logger, "Handshake Kernel a Memoria error.");
    }

    free(identificador);

    enviar_handshake(cliente, "MEMORIA");

    liberar_conexion(cliente);
    
    return;
}

void handshake_cpu(uint32_t fd_escucha_memoria) {
    uint32_t cliente = esperar_cliente(fd_escucha_memoria);

    char* identificador = recibir_handshake(cliente);

    if (string_equals_ignore_case(identificador, "cpu")) {
        log_debug(logger, "Handshake CPU a Memoria OK.");
    }
    else {
        log_error(logger, "Handshake CPU a Memoria error.");
    }

    free(identificador);

    enviar_handshake(cliente, "MEMORIA");

    liberar_conexion(cliente);
    
    return;
}

/*esta funcion lo que hace es que recibe de kernel, pid y tamaño y devuelve si hay tamaño o no para que kernel me envie la estructura
con las instrucciones. */
void recibir_consulta_memoria(uint32_t fd_kernel){
    t_paquete* paquete = recibir_paquete(fd_kernel);

    if(paquete->codigo_operacion != CONSULTA_MEMORIA_PROCESO){
        log_error(logger, "Codigo de operacion incorrecto");
        return;
    }

    //creo buffer para leer el paquete con pid y tamaño que se me envio
    t_buffer* buffer = paquete->buffer;
    uint32_t pid = buffer_read_uint32(buffer);
    uint32_t tamanio = buffer_read_uint32(buffer);

    //logueo
    log_info(logger, "Recibo consulta de espacio para PID %d, tamaño %d", pid, tamanio);

    bool hay_espacio = verificar_espacio_memoria(pid, tamanio);

    //preparo el paquete de respuesta--                          --creo buffer y le asigno tamaño del bool  que devuelve verificar_espacio_memoria 
    t_paquete* respuesta = crear_paquete(CONSULTA_MEMORIA_PROCESO, buffer_create(sizeof(uint8_t)));
    buffer_add_uint8(respuesta, hay_espacio);
    enviar_paquete(respuesta, fd_kernel);

    destruir_paquete(paquete);
    destruir_paquete(respuesta);

}

void recibir_instrucciones(uint32_t fd_kernel, uint32_t pid){
    t_paquete* paquete = recibir_paquete(fd_kernel);

    if(paquete->codigo_operacion != SAVE_INSTRUCTIONS){
        log_error(logger, "Codigo de operacion incorrecto para guardar las instrucciones");
        return;
    }

    kernel_to_memoria* kernelToMemoria = deserializar_kernel_to_memoria(paquete->buffer);

    log_info(logger, "Recibo archivo con instrucciones para PID %d", pid);

    cargar_instruciones(kernelToMemoria->archivo);

    free(kernelToMemoria->archivo);
    free(kernelToMemoria);
    destruir_paquete(paquete);


}

/* esta funcion tiene que ser distinta porque el tamaño de ña memoria tiene que ir cambiando a medida que entran y salen nuevos procesos de memoria
para esta ocasion y para hacerlo de manera simplificada lo modelo de manera tal
para que si el tamaño es menor al de la memoria, hay espacio, devuelve true y kernel puede enviar las instrucciones */
bool verificar_espacio_memoria(uint32_t pid, uint32_t tamanio){
    uint32_t tamanio_memoria_default = 4096; //esta la tengo que leer del archivo de configuracion
    return tamanio_memoria_default > tamanio;

}

// <<1>> -- Recibo la estructura kernel_to_memoria de kernel serializada, con esta funcion la deserializo y la guardo en
//la estructura kernel_to_memoria de memoria. --
kernel_to_memoria* deserializar_kernel_to_memoria(t_buffer* buffer) {
    kernel_to_memoria* data = malloc(sizeof(kernel_to_memoria));

    // Leo la longitud del archivo
    data->archivo_length = buffer_read_uint32(buffer);

    // Leo el archivo 
    data->archivo = buffer_read_string(buffer, &data->archivo_length);

    // Leo el tamaño del proceso
    data->tamanio = buffer_read_uint32(buffer);

    return data;
}


//<<2>> -- El archivo que viene en el struct kernel_to_memoria, lo tengo que leer para guardar las instrucciones en una lista --
void cargar_instrucciones(char* path) {
    FILE* archivo = fopen(path, "r");
    if (!archivo) {
        log_error(logger, "No se pudo abrir el archivo: %s", archivo);
        return;
    }

    instrucciones = list_create(); 
    char* linea = NULL;
    size_t len = 0;

    while (getline(&linea, &len, archivo) != -1) {
        linea[strcspn(linea, "\n")] = 0;

        t_instruccion* nueva_instruccion = malloc(sizeof(t_instruccion));
        if (strcmp(linea, "NOOP") == 0) *nueva_instruccion = NOOP;
        else if (strcmp(linea, "READ") == 0) *nueva_instruccion = READ;
        else if (strcmp(linea, "WRITE") == 0) *nueva_instruccion = WRITE;
        else if (strcmp(linea, "GOTO") == 0) *nueva_instruccion = GOTO;
        else if (strcmp(linea, "IO") == 0) *nueva_instruccion = IO_SYSCALL;
        else if (strcmp(linea, "INIT_PROC") == 0) *nueva_instruccion = INIT_PROC;
        else if (strcmp(linea, "DUMP") == 0) *nueva_instruccion = DUMP_MEMORY;
        else if (strcmp(linea, "EXIT") == 0) *nueva_instruccion = EXIT;
        else {
            log_warning(logger, "Instrucción desconocida: %s", linea);
            free(nueva_instruccion);
            continue;
        }

        list_add(instrucciones, nueva_instruccion);
    }

    free(linea);
    fclose(archivo);
}


