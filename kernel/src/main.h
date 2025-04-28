#include <commons/log.h>
#include <utils/hello.h>
#include <utils/conexion.h>
#include <utils/paquete.h>
#include <commons/config.h>
#include <pthread.h>
#include <utils/config.h>
#include <utils/log.h>
#include <commons/temporal.h>
#include <commons/collections/queue.h>
#include <utils/estructuras.h>

typedef enum {
    NEW,
    READY,
    EXEC,
    BLOCKED,
    SUSP_BLOCKED,
    SUSP_READY,
    EXIT_STATUS
} t_estado;

typedef struct {
    t_estado estado;
    uint32_t ME;
    t_temporal *MT;
} t_estado_metricas;

typedef struct {
    uint32_t pid;
    uint32_t pc;
    t_list* metricas;
    t_estado estado_actual;
} t_pcb;

void escucha_io();
void handshake_memoria();
void escucha_cpu();
void largo_plazo(kernel_to_memoria*);
void planificar_fifo(kernel_to_memoria*);
t_estado_metricas *crear_metrica_estado(t_estado);
t_pcb *crear_proceso(kernel_to_memoria*);
bool consultar_a_memoria(uint32_t tamanio_proceso, uint32_t pid);
void enviar_instrucciones(kernel_to_memoria* archivo);
t_buffer *serializar_kernel_to_memoria(kernel_to_memoria* archivo);
void pasar_ready();

t_log *logger;
t_config* config;
uint32_t pid_counter = 0;
t_list *cola_new;
t_list *cola_ready;
t_list *cola_blocked;