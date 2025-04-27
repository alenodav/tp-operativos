#include <commons/log.h>
#include <utils/hello.h>
#include <utils/conexion.h>
#include <utils/paquete.h>
#include <commons/config.h>
#include <pthread.h>
#include <utils/config.h>
#include <utils/log.h>
#include <utils/estructuras.h>

void handshake_memoria(void*);
void handshake_kernel(void*);

typedef struct {
    char* id_cpu;
    char* tipo_conexion;
} t_handshake_args;