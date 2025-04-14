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

    // conexion (cliente a kernel)
    pthread_t thread_kernel;
    pthread_create(&thread_kernel, NULL, handshake_kernel, config);

    pthread_detach(thread_memoria);
    pthread_detach(thread_kernel);


    // liberar
    log_destroy(logger);
    config_destroy(config);
    return 0;
}

void handshake_memoria(t_config* config){
    uint32_t fd_cpu_memoria = crear_socket_cliente(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"));

    // buffer
    t_buffer *mensaje = buffer_create(sizeof(int));
    buffer_add_uint32(mensaje,1);

    // paquete
    t_paquete *handshake_cpu_memoria = crear_paquete(HANDSHAKE,mensaje);
    enviar_paquete(handshake_cpu_memoria,fd_cpu_memoria);

    t_paquete *retorno_handshake = recibir_paquete(fd_cpu_memoria);

    uint8_t cod_op = retorno_handshake->codigo_operacion;

    if(cod_op != HANDSHAKE) {
        log_error(logger, "Codigo operación incorrecto para Handshake.");
        abort();
    }

    uint32_t retorno = buffer_read_uint32(retorno_handshake->buffer);

    if (retorno != 1) {
        log_error(logger, "Retorno handshake es distinto de 1.");
        abort();
    }

    log_info(logger, "Handshake memoria a cpu OK.");
    destruir_paquete(retorno_handshake);
    liberar_conexion(fd_cpu_memoria);
}

void handshake_kernel(t_config* config){
    uint32_t fd_cpu_kernel = crear_socket_cliente(config_get_string_value(config, "IP_KERNEL"), config_get_string_value(config, "PUERTO_KERNEL"));

    // buffer
    t_buffer *mensaje = buffer_create(sizeof(int));
    buffer_add_uint32(mensaje,1);

    // paquete
    t_paquete *handshake_cpu_kernel = crear_paquete(HANDSHAKE,mensaje);
    enviar_paquete(handshake_cpu_kernel,fd_cpu_kernel);

    t_paquete *retorno_handshake = recibir_paquete(fd_cpu_kernel);

    uint8_t cod_op = retorno_handshake->codigo_operacion;

    if(cod_op != HANDSHAKE) {
        log_error(logger, "Codigo operación incorrecto para Handshake.");
        abort();
    }

    uint32_t retorno = buffer_read_uint32(retorno_handshake->buffer);

    if (retorno != 1) {
        log_error(logger, "Retorno handshake es distinto de 1.");
        abort();
    }

    log_info(logger, "Handshake kernel a cpu OK.");
    destruir_paquete(retorno_handshake);
    liberar_conexion(fd_cpu_kernel);
}
