#ifndef METRICAS_H
#define METRICAS_H

#include <commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "common.h"

extern t_list* metricas_por_procesos;



void mostrar_metricas(uint32_t pid);

#endif