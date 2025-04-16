#include "paquete.h"
extern t_log *logger;

t_paquete *crear_paquete(u_int8_t operacion, t_buffer *buffer) {
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = operacion;
    paquete->buffer = buffer;
    return paquete;
}

void destruir_paquete(t_paquete *paquete) {
    buffer_destroy(paquete->buffer);
    free(paquete);
}

void enviar_paquete(t_paquete *paquete, uint32_t socket) {
    uint32_t err;
    void* a_enviar = malloc(paquete->buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
    int offset = 0;
    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

    err = send(socket, a_enviar, paquete->buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);

    if(err == -1) {
        log_error(logger, "Error al enviar el paquete: %s", strerror(errno));
        abort();
    }

    free(a_enviar);
    destruir_paquete(paquete);
}

t_paquete *recibir_paquete(uint32_t socket) {
    uint32_t err;
    t_paquete* paquete = malloc(sizeof(t_paquete));
    paquete->buffer = malloc(sizeof(t_buffer));
    // Primero recibimos el codigo de operacion
    paquete->codigo_operacion = recibir_operacion(socket);
    // Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
    err = recv(socket, &(paquete->buffer->size), sizeof(uint32_t), 0);
    if(err == -1) {
        log_error(logger, "Error al recibir tamaño de buffer: %s", strerror(errno));
        abort();
    }
    paquete->buffer->stream = malloc(paquete->buffer->size);
    err = recv(socket, paquete->buffer->stream, paquete->buffer->size, 0);
    if(err == -1) {
        log_error(logger, "Error al recibir buffer: %s", strerror(errno));
        abort();
    }
    paquete->buffer->offset = 0;

    return paquete;
}

void recibir_handshake(uint32_t fd_conexion) {
    t_paquete *handshake = recibir_paquete(fd_conexion);
    if (handshake->codigo_operacion != HANDSHAKE) {
        log_error(logger, "Codigo operación incorrecto para Handshake.");
        abort();
    }

    uint32_t numero_recibido = buffer_read_uint32(handshake->buffer);

    if (numero_recibido != 1) {
        log_error(logger, "Handshake recibido es distinto de 1.");
        abort();
    }

    destruir_paquete(handshake);

    return;
}

void enviar_handshake(uint32_t fd_conexion) {
    t_buffer *inicio = buffer_create(sizeof(uint32_t));

    buffer_add_uint32(inicio, 1);

    t_paquete *handshake_retorno = crear_paquete(HANDSHAKE, inicio);

    enviar_paquete(handshake_retorno, fd_conexion);

    return;
}