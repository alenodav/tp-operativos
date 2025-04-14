#include <main.h>

t_log* logger;
t_config* config;

int main(int argc, char* argv[]) {
    saludar("cpu");

    /* --- config --- */

    config = iniciar_config();
    if(config==NULL)
    {
        printf("No se pudo crear el config\n");
        abort();
    }
    char* ip = config_get_string_value(config,"IP_MEMORIA");
    char* puerto = config_get_string_value(config,"PUERTO_MEMORIA");
    char* log_level = config_get_string_value(config,"LOG_LEVEL");

    /* --- logging --- */

    logger = iniciar_logger();

    /* --- Conexion (cliente) --- */
    uint32_t socket_cliente = crear_socket_cliente(ip,puerto);

    /* --- Liberar --- */

    log_destroy(logger);
    config_destroy(config);
    liberar_conexion(socket_cliente);

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
