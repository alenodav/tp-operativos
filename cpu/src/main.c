#include <main.h>

t_log* logger;

int main(int argc, char* argv[]) {
    /* --- config --- */
    t_config *config = crear_config("cpu");

    /* --- logging --- */
    logger = crear_log(config, "cpu");
    log_debug(logger, "Config y Logger de cpu creados correctamente.");

    /* --- Conexion (cliente) --- */
    uint32_t fd_cpu_cliente = crear_socket_cliente(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"));

    // buffer
    t_buffer *mensaje = buffer_create(sizeof(int));
    buffer_add_uint32(mensaje,1);

    // paquete
    t_paquete *handshake_cpu = crear_paquete(HANDSHAKE,mensaje);
    enviar_paquete(handshake_cpu,fd_cpu_cliente);

    /* --- Liberar --- */

    log_destroy(logger);
    config_destroy(config);
    liberar_conexion(fd_cpu_cliente);

    return 0;
}

/* --- funciones de logging --- */

t_log* iniciar_logger(void)
{
	t_log* nuevo_logger = log_create ("cpu.log", "CPU_LOGGER", 1, LOG_LEVEL_TRACE);

	return nuevo_logger;
}

/* --- funciones de config --- */

t_config* iniciar_config(void)
{
	t_config* nuevo_config = config_create("cpu.config");

	return nuevo_config;
}
