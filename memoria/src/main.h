#include <utils/config.h>
#include <utils/log.h>
#include <utils/hello.h>
#include <utils/conexion.h>
#include <utils/paquete.h>
#include <pthread.h>
#include <utils/estructuras.h>

void handshake_kernel(uint32_t);
void handshake_cpu(uint32_t);

typedef struct{
    uint32_t pid;
    t_list* lista_instrucciones;
} t_proceso;
typedef struct{
    char* puerto_escucha;
    int tamanio_memoria;
    int tamanio_pagina;
    int entradas_por_tabla;
    int cantidad_niveles;
    int retardo_memoria;
    char* path_swapfile;
    char* retardo_swap;
    char* log_level_trace;
    char* dump_path;
    char* path_instrucciones;
} t_config_memoria;

void leer_configuracion(char *);