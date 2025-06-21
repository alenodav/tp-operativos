#ifndef MANEJO_MEMORIA_H
#define MANEJO_MEMORIA_H

#include <utils/config.h>

void* crear_espacio_de_memoria();
void escribir_en_memoria(void* memoria, void* contenido, uint32_t marco, uint32_t desplazamiento, uint32_t size);
void* leer_de_memoria(void* memoria, uint32_t marco, uint32_t desplazamiento, uint32_t size);

#endif