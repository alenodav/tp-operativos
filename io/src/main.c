#include<main.h>

t_log *logger;

int main(int argc, char* argv[]) {
    
    t_config *config = crear_config("io");
    logger = crear_log(config, "io");
    log_debug(logger, "Config y Logger creados correctamente.");

    if (argc < 2){
        log_error(logger, "Se debe proveer un identificador para la instancia de IO");
        abort();
    }
    char* identificador = argv[1];

    uint32_t fd_kernel = crear_socket_cliente(config_get_string_value(config, "IP"), config_get_string_value(config, "PUERTO_KERNEL"));

    
    

    enviar_handshake(fd_kernel, identificador);
    char* handshake = recibir_handshake(fd_kernel);

    if (string_equals_ignore_case(handshake, "KERNEL")) {
        log_info(logger, "Handshake Kernel a IO OK, esperando requests.");
    }
    else {
        log_error(logger, "Handshake Kernel a IO error, no se pudo establecer la conexión.");
        abort();
    }
    
    while (true) {
        t_paquete* kernel_a_io = recibir_paquete(fd_kernel);        

        uint32_t pid = buffer_read_uint32(kernel_a_io->buffer);
        uint32_t tiempo_bloqueo = buffer_read_uint32(kernel_a_io->buffer);

        destruir_paquete(kernel_a_io);

        log_info(logger, "## PID: %i - Inicio de IO - Tiempo: %i", pid, tiempo_bloqueo);

        usleep(tiempo_bloqueo);

        log_info(logger, "## PID: %i - Fin de IO", pid);

        t_buffer* buffer = buffer_create(sizeof(uint32_t));
        buffer_add_uint32(buffer, pid);
        t_paquete* respuesta = crear_paquete(IO, buffer);
        enviar_paquete(respuesta, fd_kernel);

        log_info(logger, "Esperando proxima request.");
    }
    

    free(identificador);
    liberar_conexion(fd_kernel);



    return 0;
}
