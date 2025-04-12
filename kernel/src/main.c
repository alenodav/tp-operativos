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

    getchar(); // Para que el progrma no termine antes que los threads


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
