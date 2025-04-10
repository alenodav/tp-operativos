#include<main.h>

int main(int argc, char* argv[]) {
    t_config *config = config_create("kernel.config");
    if(config == NULL) {
        abort();
    }
    t_log_level log_level = log_level_from_string(config_get_string_value(config, "LOG_LEVEL"));
    if (log_level == -1) {
        abort();
    }
    t_log *logger = log_create("kernel.log", "KERNEL", true, log_level);
    if (logger == NULL) {
        abort();
    }
    log_debug(logger, "Config y Logger creados correctamente.");

    uint32_t fd_escucha_dispatch = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"));

    while (1) {
        pthread_t thread;
        int *fd_conexion_dispatch = malloc(sizeof(int));
        *fd_conexion_dispatch = accept(fd_escucha_dispatch, NULL, NULL);
        pthread_create(&thread, NULL, (void*) atender_cliente, fd_conexion_dispatch);
        pthread_detach(thread);
    }    

    return 0;
}
