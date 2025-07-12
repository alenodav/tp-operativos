#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include<commons/collections/list.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

typedef enum {
    NOOP,
    WRITE,
    READ,
    GOTO,
    IO_SYSCALL,
    INIT_PROC,
    DUMP_MEMORY,
    EXIT
} t_instruccion;

typedef struct {
    uint32_t pid;
    uint32_t tiempo_bloqueo;
} kernel_to_io;

typedef struct {
    void* datos;
    uint32_t datos_length;
} cpu_to_kernel;

typedef struct {
    t_instruccion syscall;
    char* parametros;
    uint32_t parametros_length;
    uint32_t pid;
    uint32_t pc;
} t_syscall;

typedef struct {
    char* archivo;
    uint32_t archivo_lenght;
    uint32_t tamanio_proceso;
} init_proc_parameters;

typedef struct {
    uint32_t tiempo_bloqueo;
    char* identificador;
    uint32_t identificador_length;
} io_parameters;

//hay otra estructura para cpu que no se cual es la correcta, si la de arriba o esta de abajo:

typedef struct{
    uint32_t pid;
    uint32_t pc;
} kernel_to_cpu;

typedef struct {
    char* archivo;
    uint32_t archivo_length;
    uint32_t tamanio;
    uint32_t pid;
} kernel_to_memoria;

typedef struct {
    uint32_t direccion;
    uint32_t datos_length;
    char* datos;
} cpu_write;

typedef struct {
    uint32_t direccion;
    uint32_t tamanio;
} cpu_read;


typedef struct {
    t_instruccion instruccion;
    uint32_t parametros_length;
    char* parametros;
} struct_memoria_to_cpu;

typedef struct{
    uint32_t tam_paginas;
    uint32_t cantidad_niveles;
} t_config_to_cpu;

#endif // ESTRUCTURAS_H