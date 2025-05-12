#include <utils/config.h>
#include <utils/log.h>
#include <utils/hello.h>
#include <utils/conexion.h>
#include <utils/paquete.h>
#include <pthread.h>
#include <utils/estructuras.h>
#include<commons/collections/dictionary.h>

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
bool recibir_consulta_memoria(uint32_t);
void recibir_instrucciones(uint32_t, uint32_t);
bool verificar_espacio_memoria(uint32_t, uint32_t);
kernel_to_memoria* deserializar_kernel_to_memoria(t_buffer*);
void cargar_instrucciones(char*, uint32_t pid_t);
memoria_to_cpu* parsear_linea(char* linea);