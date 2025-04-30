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