#ifndef MMU_H_
#define MMU_H_

#include <math.h>
#include <stdint.h>
#include <utils/config.h>
#include<utils/paquete.h>
#include<utils/estructuras.h>

uint32_t numero_pagina(uint32_t direccion_logica);
uint32_t entrada_nivel_X(uint32_t direccion_logica, uint32_t nivel);
uint32_t desplazamiento(uint32_t direccion_logica);

#endif