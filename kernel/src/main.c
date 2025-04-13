#include<main.h>
t_log *logger;

int main(int argc, char* argv[]) {
    t_config *config = crear_config("kernel");
    logger = crear_log(config, "kernel");
    log_debug(logger, "Config y Logger creados correctamente.");

    //uint32_t fd_escucha_dispatch = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"));
    
    pthread_t thread_io;
    pthread_create(&thread_io, NULL, escucha_io, config); // Se crea un thread que corra escucha_io(config)
    pthread_detach(thread_io); // Se separa la ejecución del thread de la del programa principal

    pthread_t thread_memoria;
    pthread_create(&thread_memoria, NULL, handshake_memoria, config);
    pthread_detach(thread_memoria);

    getchar(); // Para que el progrma no termine antes que los threads

    config_destroy(config);
    log_info(logger, "Finalizo el proceso.");
    log_destroy(logger);

    return 0;
}

void escucha_io(t_config* config){
    uint32_t fd_escucha_io = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_IO"));    
    uint32_t socket_io = esperar_cliente(fd_escucha_io);
    t_paquete *handshake = recibir_paquete(socket_io);
    if (handshake->codigo_operacion == HANDSHAKE){
        log_debug(logger, "Handshake de IO recibido.");
        liberar_conexion(socket_io);
    } else {
        log_debug(logger, "Handshake de IO no recibido.");
    }

    return;
}

void handshake_memoria(t_config *config) {
    uint32_t fd_conexion_memoria = crear_socket_cliente(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"));

    t_buffer *inicio = buffer_create(sizeof(uint32_t));

    buffer_add_uint32(inicio, 1);

    t_paquete *handshake = crear_paquete(HANDSHAKE, inicio);

    enviar_paquete(handshake, fd_conexion_memoria);

    t_paquete *retorno_handshake = recibir_paquete(fd_conexion_memoria);

    uint8_t cod_op = retorno_handshake->codigo_operacion;

    if(cod_op != HANDSHAKE) {
        log_error(logger, "Codigo operación incorrecto para Handshake.");
        abort();
    }

    uint32_t retorno = buffer_read_uint32(retorno_handshake->buffer);

    if (retorno != 1) {
        log_error(logger, "Retorno handshake es distinto de 1.");
        abort();
    }

    log_info(logger, "Handshake Memoria a Kernel OK.");
    destruir_paquete(retorno_handshake);
    liberar_conexion(fd_conexion_memoria);
    return;
}
