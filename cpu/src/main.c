#include <main.h>

t_log* logger;
t_config* config;

int main(int argc, char* argv[]){
     if (argc < 2 || argc > 2) {
        log_debug(logger,"Uso: %s <ID_CPU>\n", argv[0]);
        return EXIT_FAILURE;
    }
    char* id_cpu = argv[1];

    char log_filename[64];
    sprintf(log_filename, "cpu-%s.log", id_cpu);
    logger = log_create(log_filename, "CPU", true, LOG_LEVEL_DEBUG);
    log_debug(logger, "Logger de CPU id:%s creado.", id_cpu);

    config = crear_config("cpu");

    pthread_t thread_memoria;
    pthread_create(&thread_memoria, NULL, handshake_memoria, (void*)id_cpu);
    pthread_detach(thread_memoria); 

    pthread_t thread_kernel;
    pthread_create(&thread_kernel, NULL, handshake_kernel, (void*)id_cpu);
    pthread_detach(thread_kernel);

    getchar();

    log_destroy(logger);
    config_destroy(config);
    return 0;
}

void handshake_memoria(void* arg){
    char* id_cpu = (char*)arg;
    uint32_t fd_cpu_memoria = crear_socket_cliente(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"));

    enviar_handshake(fd_cpu_memoria, "CPU");
    char* identificador = recibir_handshake(fd_cpu_memoria);

    if (string_equals_ignore_case(identificador, "memoria")) {
        log_debug(logger, "Handshake Memoria a CPU-%s OK.",id_cpu);
    }
    else {
        log_error(logger, "Handshake Memoria a CPU-%s error.",id_cpu);
    }

    free(identificador);
    liberar_conexion(fd_cpu_memoria);
}

void handshake_kernel(void* arg){
    char* id_cpu = (char*)arg;
    uint32_t fd_cpu_kernel = crear_socket_cliente(config_get_string_value(config, "IP_KERNEL"), config_get_string_value(config, "PUERTO_KERNEL_DISPATCH"));

    enviar_handshake(fd_cpu_kernel,id_cpu);
    char* identificador = recibir_handshake(fd_cpu_kernel);

    if (string_equals_ignore_case(identificador, "kernel")) {
        log_debug(logger, "Handshake Kernel a CPU-%s OK.",id_cpu);
    }
    else {
        log_error(logger, "Handshake Kernel a CPU-%s error.",id_cpu);
    }

    free(identificador);
    liberar_conexion(fd_cpu_kernel);
}
