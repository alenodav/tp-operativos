#include "buffer.h"

t_buffer *buffer_create(u_int32_t size) {
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = size;
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);
    return buffer;
}

void buffer_destroy(t_buffer *buffer) {
    free(buffer->stream);
    free(buffer);
}

void buffer_add(t_buffer *buffer, void *data, uint32_t size) {
    memcpy(buffer->stream + buffer->offset, data, size);
    buffer->offset += size;
}

void buffer_read(t_buffer *buffer, void *data, uint32_t size) {
    memcpy(data, buffer->stream + buffer->offset, size);
    buffer->offset += size;
}

void buffer_add_uint32(t_buffer *buffer, uint32_t data) {
    buffer_add(buffer, &data, sizeof(uint32_t));
}

uint32_t buffer_read_uint32(t_buffer *buffer) {
    uint32_t res = 0;
    buffer_read(buffer, &res, sizeof(uint32_t));
    return res;
}

void buffer_add_uint8(t_buffer *buffer, uint8_t data) {
    buffer_add(buffer, &data, sizeof(uint32_t));
}

uint8_t buffer_read_uint8(t_buffer *buffer) {
    uint8_t res = 0;
    buffer_read(buffer, &res, sizeof(uint8_t));
    return res;
}

void buffer_add_string(t_buffer *buffer, uint32_t length, char *string) {
    buffer_add(buffer, &string, length);
}

char *buffer_read_string(t_buffer *buffer, uint32_t *length) {
    char *res = malloc(*length + 1);
    buffer_read(buffer, &res, *length);
    return res;
}