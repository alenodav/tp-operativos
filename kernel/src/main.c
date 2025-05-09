#include<main.h>
t_log *logger;

int main(int argc, char* argv[]) {
    t_config *config = crear_config("kernel");
    logger = crear_log(config, "kernel");
    log_debug(logger, "Config y Logger creados correctamente.");

    //uint32_t fd_escucha_dispatch = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"));

    pthread_t thread_io;
    pthread_create(&thread_io, NULL, escucha_io, config); // Se crea un thread que corra escucha_io(config)
    pthread_detach(thread_io);  // Se separa la ejecución del thread de la del programa principal

    // pthread_t thread_memoria;
    // pthread_create(&thread_memoria, NULL, handshake_memoria, config);
    // pthread_detach(thread_memoria); 

    // //sleep(1);

    // //Creo un hilo para que corra escucha_cpu(config)
    // pthread_t thread_cpu;
    // pthread_create(&thread_cpu, NULL, escucha_cpu, config);
    // pthread_detach(thread_cpu);

    getchar(); // Para que el progrma no termine antes que los threads

    config_destroy(config);
    log_info(logger, "Finalizo el proceso.");
    log_destroy(logger);

    return 0;
}

void escucha_io(t_config* config){
    uint32_t fd_escucha_io = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_IO"));    
    uint32_t socket_io = esperar_cliente(fd_escucha_io);
    char* identificador = recibir_handshake(socket_io);
    if (!string_is_empty(identificador)) {
        log_info(logger, "Handshake IO a Kernel OK.");
        log_info(logger, string_from_format("Se inicio la instancia: %s", identificador));
    }
    else {
        log_error(logger, "Handshake IO a Kernel fallido, la instancia de IO debe tener un nombre.");
    }
    free(identificador);
    enviar_handshake(socket_io, "KERNEL");
    
    //test
    t_buffer* buffer = buffer_create(sizeof(uint32_t)*2);
    buffer_add_uint32(buffer, 1);
    buffer_add_uint32(buffer, 1000000*5);
    t_paquete* paquete = crear_paquete(IO, buffer);
    enviar_paquete(paquete, socket_io);
    recibir_paquete(socket_io);

    getchar();

    liberar_conexion(socket_io);
    liberar_conexion(fd_escucha_io);
    return;
}

//Creo escucha cpu
void escucha_cpu(t_config* config){
    uint32_t fd_escucha_cpu = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"));    
    uint32_t socket_cpu = esperar_cliente(fd_escucha_cpu);
    char *identificador = recibir_handshake(socket_cpu);
    if (string_equals_ignore_case(identificador, "cpu")) {
        log_debug(logger, "Handshake CPU a Kernel OK.");
    }
    else {
        log_error(logger, "Handshake CPU a Kernel error.");
    }
    free(identificador);
    enviar_handshake(socket_cpu, "KERNEL");
    liberar_conexion(socket_cpu);
    liberar_conexion(fd_escucha_cpu);
    return;
}


void handshake_memoria(t_config *config) {
    uint32_t fd_conexion_memoria = crear_socket_cliente(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"));

    enviar_handshake(fd_conexion_memoria, "KERNEL");

    char* identificador = recibir_handshake(fd_conexion_memoria);
    if (string_equals_ignore_case(identificador, "memoria")) {
        log_debug(logger, "Handshake Memoria a Kernel OK.");
    }
    else {
        log_error(logger, "Handshake Memoria a Kernel error.");
    }

    free(identificador);
    liberar_conexion(fd_conexion_memoria);
    return;
}
