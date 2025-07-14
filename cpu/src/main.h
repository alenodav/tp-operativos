#ifndef MAIN_H_
#define MAIN_H_

#include <commons/log.h>
#include <utils/hello.h>
#include <utils/conexion.h>
#include <utils/paquete.h>
#include <commons/config.h>
#include <pthread.h>
#include <utils/config.h>
#include <utils/log.h>
#include <utils/estructuras.h>
#include <semaphore.h>
#include <commons/temporal.h>
#include "../include/mmu.h"

// Variables globales
extern t_log* logger;
extern t_config* config;
extern uint32_t fd_dispatch;
extern uint32_t fd_interrupt;
extern uint32_t fd_memoria;
bool interrupcion;

void handshake_memoria(void*);
void handshake_kernel(void*);
void recibir_proceso(void*);
void solicitar_instruccion(kernel_to_cpu* instruccion);
t_buffer *serializar_cpu_write(cpu_write *data);
t_buffer *serializar_cpu_read(cpu_read *data);
t_buffer *serializar_t_syscall(t_syscall *data);
void destruir_t_syscall(t_syscall *data);
t_buffer *serializar_kernel_to_cpu(kernel_to_cpu* param); 
void check_interrupt(uint32_t pid);
void interrumpir_proceso(uint32_t pid, uint32_t pc);

//TLB


t_list* tlb;
int cant_entradas_tlb;
char* algoritmo_tlb;

void inicializar_tlb();

void correr_algoritmo_tlb();
entrada_tlb* entrada_tlb_get_by_pagina(t_list* entrada_tlb_list, uint32_t pagina);
bool es_mas_reciente(void* a, void* b);



#endif