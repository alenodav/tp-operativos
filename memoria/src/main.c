#include <main.h>
t_log *logger;

int main(int argc, char* argv[]) {
    t_config *config = crear_config("memoria");
    logger = crear_log(config, "memoria");
    log_debug(logger, "Config y Logger creados correctamente.");

    uint32_t fd_escucha_memoria = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"));

    pthread_t thread_escucha_kernel;
    pthread_create(&thread_escucha_kernel, NULL, handshake_kernel, fd_escucha_memoria);
    pthread_detach(thread_escucha_kernel);

    pthread_t thread_escucha_cpu;
    pthread_create(&thread_escucha_cpu, NULL, handshake_cpu, fd_escucha_memoria);
    pthread_detach(thread_escucha_cpu);

    getchar();

    liberar_conexion(fd_escucha_memoria);
    config_destroy(config);
    log_info(logger, "Finalizo el proceso.");
    log_destroy(logger);

}

void handshake_kernel(uint32_t fd_escucha_memoria) {
    

    uint32_t cliente = esperar_cliente(fd_escucha_memoria);
    t_paquete *handshake = recibir_paquete(cliente);
    if (handshake->codigo_operacion != HANDSHAKE) {
        log_error(logger, "Codigo operación incorrecto para Handshake.");
        abort();
    }

    uint32_t numero_recibido = buffer_read_uint32(handshake->buffer);

    if (numero_recibido != 1) {
        log_error(logger, "Hndshake recibido es distinto de 1.");
        abort();
    }

    destruir_paquete(handshake);

    log_info(logger, "Handshake Kernel a Memoria OK.");

    t_buffer *inicio = buffer_create(sizeof(uint32_t));

    buffer_add_uint32(inicio, 1);

    t_paquete *handshake_retorno = crear_paquete(HANDSHAKE, inicio);

    enviar_paquete(handshake_retorno, cliente);

    liberar_conexion(cliente);
    

    return;
}

void handshake_cpu(uint32_t fd_escucha_memoria) {
    uint32_t cliente = esperar_cliente(fd_escucha_memoria);
    t_paquete *handshake = recibir_paquete(cliente);
    if (handshake->codigo_operacion != HANDSHAKE) {
        log_error(logger, "Codigo operación incorrecto para Handshake.");
        abort();
    }

    uint32_t numero_recibido = buffer_read_uint32(handshake->buffer);

    if (numero_recibido != 1) {
        log_error(logger, "Hndshake recibido es distinto de 1.");
        abort();
    }

    destruir_paquete(handshake);

    log_info(logger, "Handshake CPU a Memoria OK.");

    t_buffer *inicio = buffer_create(sizeof(uint32_t));

    buffer_add_uint32(inicio, 1);

    t_paquete *handshake_retorno = crear_paquete(HANDSHAKE, inicio);

    enviar_paquete(handshake_retorno, cliente);

    liberar_conexion(cliente);
    
    return;
}
