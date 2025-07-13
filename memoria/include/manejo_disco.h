#ifndef MANEJO_DISCO_H
#define MANEJO_DISCO_H

#include <utils/config.h>
#include<commons/txt.h>
#include<commons/temporal.h>
#include "common.h"

typedef struct contenido_swap {
    uint32_t pid;
    uint32_t cant_paginas;
    char** paginas;
} contenido_swap;

void dump_memory(tablas_por_pid* contenido); 
void suspender_proceso(tablas_por_pid* contenido);

#endif