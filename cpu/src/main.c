#include <main.h>

t_log* logger;

int main(int argc, char* argv[]) {
    // config
    t_config *config = crear_config("cpu");

    // logging 
    logger = crear_log(config, "cpu");
    log_debug(logger, "Config y Logger de cpu creados correctamente.");

    // conexion (cliente a memoria)

    pthread_t thread_memoria;
    pthread_create(&thread_memoria, NULL, handshake_memoria, config);
    pthread_detach(thread_memoria); 

    // conexion (cliente a kernel)
    pthread_t thread_kernel;
    pthread_create(&thread_kernel, NULL, handshake_kernel, config);
    pthread_detach(thread_kernel);

    getchar();

    //log_debug(logger, "finalizo el proceso");


    // liberar
    log_destroy(logger);
    config_destroy(config);
    return 0;
}

void handshake_memoria(t_config* config){
    uint32_t fd_cpu_memoria = crear_socket_cliente(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"));

    enviar_handshake(fd_cpu_memoria, "CPU");
    char* identificador = recibir_handshake(fd_cpu_memoria);

    if (string_equals_ignore_case(identificador, "memoria")) {
        log_debug(logger, "Handshake Memoria a CPU OK.");
    }
    else {
        log_error(logger, "Handshake Memoria a CPU error.");
    }

    free(identificador);
    liberar_conexion(fd_cpu_memoria);
}

void handshake_kernel(t_config* config){
    uint32_t fd_cpu_kernel = crear_socket_cliente(config_get_string_value(config, "IP_KERNEL"), config_get_string_value(config, "PUERTO_KERNEL_DISPATCH"));

    enviar_handshake(fd_cpu_kernel, "CPU");
    char* identificador = recibir_handshake(fd_cpu_kernel);

    if (string_equals_ignore_case(identificador, "kernel")) {
        log_debug(logger, "Handshake Kernel a CPU OK.");
    }
    else {
        log_error(logger, "Handshake Kernel a CPU error.");
    }

    free(identificador);
    liberar_conexion(fd_cpu_kernel);
}
