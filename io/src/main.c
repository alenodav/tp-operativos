#include<main.h>

t_log *logger;

int main(int argc, char* argv[]) {
    
    t_config *config = crear_config("io");
    logger = crear_log(config, "io");
    
    log_debug(logger, "Config y Logger creados correctamente.");

    uint32_t fd_kernel = crear_socket_cliente(config_get_string_value(config, "IP"), config_get_string_value(config, "PUERTO_KERNEL"));

    t_buffer *mensaje = buffer_create(sizeof(int));
    buffer_add_uint32(mensaje, 1);

    t_paquete *handshake = crear_paquete(HANDSHAKE, mensaje);

    enviar_paquete(handshake, fd_kernel);
    log_debug(logger, "Handshake enviado a Kernel con exito.");

    return 0;
}
