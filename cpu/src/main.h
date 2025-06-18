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

// Variables globales
extern t_log* logger;
extern t_config* config;
extern uint32_t fd_dispatch;
extern uint32_t fd_interrupt;
extern uint32_t fd_memoria;

void handshake_memoria(void*);
void handshake_kernel(void*);
void recibir_proceso(void*);
void solicitar_instruccion(uint32_t pid, uint32_t pc);

#endif