#include <../include/tablas_paginas.h>
#include<commons/collections/list.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

t_config_memoria *memoria_cfg;
t_memoria* memoria_principal;

tabla_paginas* crear_tabla(uint32_t nivel){
    tabla_paginas* tabla = malloc(sizeof(tabla_paginas));
    tabla->entradas = malloc(memoria_cfg->ENTRADAS_POR_TABLA * sizeof(entrada_tabla));
    tabla->nivel = nivel;

    for (int i = 0; i < memoria_cfg->ENTRADAS_POR_TABLA; i++) {
        tabla->entradas[i].marco = -1;
        tabla->entradas[i].tabla_siguiente = NULL;
    }

    return tabla;
}

tablas_por_pid* crear_tabla_raiz(uint32_t pid, uint32_t tamanio_proceso) {
    tablas_por_pid* tabla_raiz_pid = malloc(sizeof(tablas_por_pid));
    tabla_raiz_pid->pid = pid;
    tabla_raiz_pid->tabla_raiz = crear_tabla(1);
    tabla_raiz_pid->cant_marcos = ceil(tamanio_proceso / memoria_cfg->TAM_PAGINA);
    tabla_raiz_pid->marcos = malloc(tabla_raiz_pid->cant_marcos * sizeof(int));
    return tabla_raiz_pid;
}

void asignar_marcos(tabla_paginas* tabla_actual, uint32_t* tamanio_proceso, uint32_t nivel, int* marcos, int* indice_marcos) {
    for (int entrada = 0; entrada < memoria_cfg->ENTRADAS_POR_TABLA; entrada++) {
        if (*tamanio_proceso > 0) {
            if (nivel < memoria_cfg->CANTIDAD_NIVELES) {
                if (tabla_actual->entradas[entrada].tabla_siguiente == NULL) {
                    tabla_actual->entradas[entrada].tabla_siguiente = crear_tabla(nivel + 1);
                    tabla_actual->entradas[entrada].marco = -1;
                }

                asignar_marcos(tabla_actual->entradas[entrada].tabla_siguiente, tamanio_proceso, nivel + 1, marcos, indice_marcos);
            }
            else {
                *tamanio_proceso = *tamanio_proceso - memoria_cfg->TAM_PAGINA;
                tabla_actual->entradas[entrada].tabla_siguiente = NULL;
                tabla_actual->entradas[entrada].marco = obtener_marco_libre();
                marcos[*indice_marcos] = tabla_actual->entradas[entrada].marco;
                (*indice_marcos)++;
            }
        }
        else {
            break;
        }
    }
}

uint32_t devolver_marco(tabla_paginas* tabla_actual, uint32_t* indices, uint32_t nivel, t_metricas *metricas_proceso) {
    uint32_t marco = -1;
    usleep(memoria_cfg->RETARDO_MEMORIA);
    int indice_actual = indices[nivel-1];
    if (nivel < memoria_cfg->CANTIDAD_NIVELES) {
        marco = devolver_marco(tabla_actual->entradas[indice_actual].tabla_siguiente, indices, nivel + 1, metricas_proceso);
    }
    else {
        marco =  tabla_actual->entradas[indice_actual].marco;
    }
    metricas_proceso->accesos_tablas_paginas++;
    return marco;
}

void* leer_pagina_completa(uint32_t direccion_fisica) {
    void* retorno = malloc(memoria_cfg->TAM_PAGINA);
    memcpy(retorno, memoria_principal->datos + direccion_fisica, memoria_cfg->TAM_PAGINA);
    return retorno;
}

bool actualizar_pagina_completa(uint32_t direccion_fisica, char* pagina) {
    memcpy(memoria_principal->datos + direccion_fisica, pagina, memoria_cfg->TAM_PAGINA);
    return true;
}
