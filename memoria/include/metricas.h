#ifndef METRICAS_H
#define METRICAS_H

#include <commons/collections/list.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

t_list* metricas_por_procesos;

typedef struct{
    uint32_t pid;
    uint32_t accesos_tablas_paginas;
    uint32_t instrucciones_solicitadas;
    uint32_t bajadas_a_swap;
    uint32_t subidas_a_mp;
    uint32_t cantidad_lecturas;
    uint32_t cantidad_escrituras;
} t_metricas;

#endif