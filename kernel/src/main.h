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
#include <semaphore.h>

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

typedef struct {
    char* identificador;
    bool estado;
    uint32_t socket_dispatch;
    uint32_t socket_interrupt;
} t_cpu;

typedef struct {
    char* identificador;
    bool estado;
    uint32_t socket;
    uint32_t proceso_ejecucion;
} t_io;

typedef struct {
    char* id;
    t_queue *cola_procesos;
} t_io_queue;

void iniciar_modulo();
void finalizar_modulo();
void escucha_io();
void handshake_memoria();
void escucha_cpu();
void largo_plazo();
void planificar_fifo_largo_plazo();
t_estado_metricas *crear_metrica_estado(t_estado);
void agregar_metricas_estado(t_pcb *);
void pasar_por_estado(t_pcb *, t_estado, t_estado );
t_pcb *crear_proceso();
bool consultar_a_memoria();
void enviar_instrucciones();
t_buffer *serializar_kernel_to_memoria(kernel_to_memoria*);
void pasar_ready(t_pcb *, t_estado_metricas*);
bool terminar_proceso_memoria (uint32_t);
void terminar_proceso(uint32_t);
t_pcb* pcb_by_pid(t_list*, uint32_t);
void loggear_metricas_estado(t_pcb*);
char* t_estado_to_string(t_estado);
void administrar_cpus_dispatch();
void agregar_cpu_dispatch(uint32_t*);
void administrar_cpus_interrupt();
void agregar_cpu_interrupt(uint32_t*);
t_cpu *cpu_find_by_id (char *);
void agregar_io (uint32_t *);
void administrar_dispositivos_io();
void ejecutar_io_syscall (uint32_t , char* , uint32_t );
void enviar_kernel_to_io (char*);
void manejar_respuesta_io(t_io *);
t_list *io_filter_by_id (char *);
t_io_queue *io_queue_find_by_id (char *);
bool io_liberada(void* );
t_buffer *serializar_kernel_to_io (kernel_to_io* );
void corto_plazo();
void planificar_fifo_corto_plazo();
bool find_cpu_libre(void*);
void pasar_exec(t_pcb *);
void enviar_kernel_to_cpu(uint32_t , t_pcb *); 
t_buffer *serializar_kernel_to_cpu(kernel_to_cpu* );
void atender_respuesta_cpu(t_cpu *);
char *t_instruccion_to_string(t_instruccion ); 
t_syscall *deserializar_t_syscall(t_buffer* );
void ejecutar_init_proc(uint32_t , char* , uint32_t , t_cpu* ); 


t_log *logger;
t_config* config;
uint32_t pid_counter;
uint32_t fd_escucha_cpu;
uint32_t fd_escucha_cpu_interrupt;
uint32_t fd_escucha_io;
t_list *cola_new;
t_list *cola_ready;
t_list *cola_exec;
t_list *cola_blocked;
t_list *archivos_instruccion;
t_list *io_list;
t_list *io_queue_list;
t_list *cpu_list;
bool inicio_modulo;
sem_t *sem_largo_plazo;
sem_t *sem_cpus;
sem_t *sem_io;
sem_t *sem_execute;
sem_t *sem_corto_plazo;
sem_t *sem_ready;
sem_t *sem_blocked;