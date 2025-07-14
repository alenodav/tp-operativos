#ifndef MANEJO_MEMORIA_H
#define MANEJO_MEMORIA_H

#include <utils/config.h>
#include <stdint.h>
#include "common.h"

void* crear_espacio_de_memoria();
bool escribir_en_memoria(uint32_t direccion_fisica, uint32_t size, void* contenido, uint32_t pid);
void* leer_de_memoria(uint32_t direccion_fisica, uint32_t size, uint32_t pid);

#endif