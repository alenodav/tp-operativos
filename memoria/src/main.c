#include <main.h>

t_log *logger;
t_list* lista_instrucciones;
t_dictionary* diccionario_procesos;

int main(int argc, char* argv[]) {
    t_config *config = crear_config("memoria");
    logger = crear_log(config, "memoria");
    log_debug(logger, "Config y Logger creados correctamente.");

    uint32_t fd_escucha_memoria = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"));

    //Thread para CPU;
    pthread_t thread_escucha_cpu;
    pthread_create(&thread_escucha_cpu, NULL, handshake_cpu, fd_escucha_memoria);
    pthread_detach(thread_escucha_cpu);
   
    //Thread para atender a Kernel
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

/*
Esta funcion lo que hace es que recibe de kernel, pid y tamaño y devuelve si hay tamaño o no para que kernel me envie la estructura
con las instrucciones.  
esta es la primera funcion a ejecutar para que kernel me envie las instrucciones completas. si hay tamanio le mando true y me envia
las instrucciones
*/
bool recibir_consulta_memoria(uint32_t fd_kernel){
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

    return hay_espacio;

}

//aca uso deserializar() para guardarme en una estructura las instrucciones, y despues las cargo en una lista, con cargar_instrucciones
void recibir_instrucciones(uint32_t fd_kernel, uint32_t pid){
    t_paquete* paquete = recibir_paquete(fd_kernel);

    if(paquete->codigo_operacion != SAVE_INSTRUCTIONS){
        log_error(logger, "Codigo de operacion incorrecto para guardar las instrucciones");
        return;
    }

    kernel_to_memoria* kernelToMemoria = deserializar_kernel_to_memoria(paquete->buffer); // <<1>>

    log_info(logger, "Recibo archivo con instrucciones para PID %d", kernelToMemoria->pid);

    cargar_instrucciones(kernelToMemoria->archivo, kernelToMemoria->pid); // <<2>>

    free(kernelToMemoria->archivo);
    free(kernelToMemoria);
    destruir_paquete(paquete);
}

/*
    Funcion simplificada para uso practico de checkpoint 2.
*/
bool verificar_espacio_memoria(uint32_t pid, uint32_t tamanio){
    uint32_t tamanio_memoria_default = 4096; //esta constante la tengo que leer del archivo de configuracion
    return tamanio_memoria_default > tamanio;
}

/*
    <<1>> Recibo la estructura kernel_to_memoria de kernel serializada, con esta funcion la deserializo y la guardo en
    la estructura kernel_to_memoria de memoria. --
*/
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

memoria_to_cpu* parsear_linea(char *linea){
    char** token = string_split(linea, " ");

    memoria_to_cpu* struct_memoria_to_cpu = malloc(sizeof(memoria_to_cpu));
    struct_memoria_to_cpu->argumentos = NULL;

    if(string_equals_ignore_case(token[0], "READ")) {
        struct_memoria_to_cpu->instruccion = READ;
    } else if (string_equals_ignore_case(token[0], "WRITE")) {
        struct_memoria_to_cpu->instruccion = WRITE;
    } else if (string_equals_ignore_case(token[0], "GOTO")) {
        struct_memoria_to_cpu->instruccion = GOTO;
    } else if (string_equals_ignore_case(token[0], "NOOP")) {
        struct_memoria_to_cpu->instruccion = NOOP;
    } else if (string_equals_ignore_case(token[0], "EXIT")) {
        struct_memoria_to_cpu->instruccion = EXIT;
    } else {
        // Manejo de error
        log_error(logger, "Instrucción desconocida: %s\n", token[0]); 
    }

    //Como se separaron los demas tokens, los que quedaron son los parametros, entonces los uno en un nuevo string.
    if (token[1] != NULL) {
        struct_memoria_to_cpu->argumentos = string_new();
        for (int i = 1; token[i] != NULL; i++) {
            string_append_with_format(&struct_memoria_to_cpu->argumentos, "%s%s", token[i], token[i+1] ? " " : "");
        }
    } else {
        struct_memoria_to_cpu->argumentos = string_duplicate(""); // vacío
    }

    string_iterate_lines(token, free);
    free(token);

    return struct_memoria_to_cpu;
    
}

//esta funcion abre el archivo, usa parsear_linea(), guarda todo en una lista del tipo memoria_to_cpu y guarda en el diccionario.
void cargar_instrucciones(char* path_archivo, uint32_t pid){
    
    FILE* archivo = fopen(path_archivo, "r");
    if (!archivo) {
        perror("No se pudo abrir el archivo de instrucciones");
        return;
    }

    lista_instrucciones = list_create();
    char* linea = NULL;
    size_t len = 0;

    while (getline(&linea, &len, archivo) != -1) {
        string_trim(&linea);
        if (string_is_empty(linea)) continue;

        memoria_to_cpu* instruccion = parsear_linea(linea);
        list_add(lista_instrucciones, instruccion);
    }

    free(linea);
    fclose(archivo);

    char* pid_str = string_itoa(pid);
    dictionary_put(diccionario_procesos, pid_str, lista_instrucciones);
    free(pid_str);
}

/* void leer_configuracion(char* config){
    
} */

//nota: decile a tomas que me mande el pid cuando me manda tambien el archivo en la funcion recibir_instrucciones, porque despues yo lo uso
//para mandarlo al diccionario de una.

//o segunda opcion, declarar una estructura kernel_to_memoria y de ahi voy sacando los atributos para usar en el diccionario. 
//no si tiene sentido eso.¿?

//faltaria que en los hilos empiece a llamar a las funciones y eso.
