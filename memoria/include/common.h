#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>
#include <utils/config.h>
#include <utils/log.h>
#include <utils/hello.h>
#include <utils/conexion.h>
#include <utils/paquete.h>
#include <pthread.h>
#include <utils/estructuras.h>
#include <commons/collections/dictionary.h>
#include <semaphore.h>

/*Variables Globales*/

extern t_log *logger;
extern t_dictionary* diccionario_procesos;
extern pthread_mutex_t mutex_diccionario;

/*Funciones para el handshake inicial*/

void handshake_kernel(uint32_t);
void handshake_cpu(uint32_t);


typedef struct{
    uint32_t tamanio;
    t_list* lista_instrucciones;
    struct t_metricas* lista_metricas;
} t_proceso;

typedef struct t_memoria{
    void* datos;
} t_memoria;

extern t_memoria* memoria_principal;

typedef struct{
    char* PUERTO_ESCUCHA;
    int TAM_MEMORIA;
    int TAM_PAGINA;
    int ENTRADAS_POR_TABLA;
    int CANTIDAD_NIVELES;
    int RETARDO_MEMORIA;
    char* PATH_SWAPFILE;
    char* RETARDO_SWAP;
    char* LOG_LEVEL;
    char* DUMP_PATH;
    char* PATH_INSTRUCCIONES;
} t_config_memoria;

extern t_config_memoria *memoria_cfg;

typedef struct t_metricas{
    uint32_t pid;
    uint32_t accesos_tablas_paginas;
    uint32_t instrucciones_solicitadas;
    uint32_t bajadas_a_swap;
    uint32_t subidas_a_mp;
    uint32_t cantidad_lecturas;
    uint32_t cantidad_escrituras;
} t_metricas;

extern t_list* metricas_por_procesos;

//Tabla de paginas.
typedef struct tabla_paginas{
    uint32_t nivel;     //nivel(1...n)
    struct entrada_tabla* entradas;   //lista de struct entrada_pagina
} tabla_paginas;

typedef struct tablas_por_pid{
    uint32_t pid;
    tabla_paginas* tabla_raiz;
    uint32_t* marcos;
    uint32_t cant_marcos;
} tablas_por_pid;

extern t_list *lista_tablas_por_pid;

void leer_configuracion(t_config *);
bool recibir_consulta_memoria(uint32_t, t_paquete*);
void recibir_instrucciones(t_paquete*);
bool verificar_espacio_memoria(uint32_t);
kernel_to_memoria* deserializar_kernel_to_memoria(t_buffer*);
void cargar_instrucciones(char *path_archivo, kernel_to_memoria* proceso_recibido);
struct_memoria_to_cpu* parsear_linea(char* linea);
bool enviar_instruccion(uint32_t fd_cpu);
void liberar_lista_instrucciones(t_list* lista);
void liberar_diccionario();
cpu_read *deserializar_cpu_read(t_buffer *data);
cpu_write *deserializar_cpu_write(t_buffer *data);
tablas_por_pid* tablas_por_pid_remove_by_pid(t_list* tablas_por_pid_list, uint32_t pid);
tablas_por_pid* tablas_por_pid_get_by_pid(t_list* tablas_por_pid_list, uint32_t pid);
t_buffer* serializar_config_to_cpu(t_config_to_cpu* cfg_to_cpu);
void enviar_config_to_cpu(t_config_to_cpu* cfg_to_cpu, uint32_t socket);

void* leer_pagina_completa(uint32_t direccion_fisica, uint32_t pid, t_metricas *metricas_proceso);
bool actualizar_pagina_completa(uint32_t direccion_fisica, void* pagina, uint32_t pid, t_metricas *metricas_proceso);
void asignar_marcos(tabla_paginas* tabla_actual, uint32_t* tamanio_proceso, uint32_t nivel, uint32_t* marcos, uint32_t* indice_marcos, t_metricas* metricas_proceso);

int obtener_marco_libre();
void liberar_marco(int marco);

void dessuspender_procesos (tablas_por_pid* contenido, uint32_t tamanio_proceso, t_metricas *metricas_proceso);
bool tiene_entradas_swap(tablas_por_pid* proceso);
void liberar_proceso_swap(tablas_por_pid* contenido, uint32_t tamanio_proceso, t_metricas *metricas_proceso); 

#endif