#include<main.h>

t_log *logger;

int main(int argc, char* argv[]) {
    
    t_config *config = crear_config("io");
    logger = crear_log(config, "io");
    
    log_debug(logger, "Config y Logger creados correctamente.");

    uint32_t fd_kernel = crear_socket_cliente(config_get_string_value(config, "IP"), config_get_string_value(config, "PUERTO_KERNEL"));

    enviar_handshake(fd_kernel);
    recibir_handshake(fd_kernel);
    log_info(logger, "Handshake Kernel a IO OK.");

    liberar_conexion(fd_kernel);

    return 0;
}
