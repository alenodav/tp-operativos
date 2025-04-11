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