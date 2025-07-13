#ifndef MANEJO_DISCO_H
#define MANEJO_DISCO_H

#include <utils/config.h>
#include<commons/txt.h>
#include<commons/temporal.h>
#include "common.h"

void dump_memory(tablas_por_pid* contenido); 
void suspender_proceso(tablas_por_pid* contenido);
void dessuspender_procesos (tablas_por_pid* contenido, uint32_t tamanio_proceso);

#endif