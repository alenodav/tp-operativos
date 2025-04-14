#include <main.h>
t_log *logger;

int main(int argc, char* argv[]) {
    t_config *config = crear_config("memoria");
    logger = crear_log(config, "memoria");
    log_debug(logger, "Config y Logger creados correctamente.");

    pthread_t thread_escucha;
    pthread_create(&thread_escucha, NULL, handshake_kernel, config);
    pthread_detach(thread_escucha);

    getchar();

    config_destroy(config);
    log_info(logger, "Finalizo el proceso.");
    log_destroy(logger);

}

void handshake_kernel(t_config* config) {
    uint32_t fd_escucha_memoria = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"));

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
    liberar_conexion(fd_escucha_memoria);

    return;
}
