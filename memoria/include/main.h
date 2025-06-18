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
    uint32_t pid;
    t_list* lista_instrucciones;
} t_proceso;

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

void leer_configuracion(char *);
bool recibir_consulta_memoria(uint32_t, t_paquete*);
void recibir_instrucciones(t_paquete*);
bool verificar_espacio_memoria(uint32_t);
kernel_to_memoria* deserializar_kernel_to_memoria(t_buffer*);
void cargar_instrucciones(char*, uint32_t pid_t);
struct_memoria_to_cpu* parsear_linea(char* linea);
bool enviar_instruccion(uint32_t fd_cpu);
void liberar_lista_instrucciones(t_list* lista);
void liberar_diccionario();
cpu_read *deserializar_cpu_read(t_buffer *data);
cpu_write *deserializar_cpu_write(t_buffer *data);