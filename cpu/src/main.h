#include <commons/log.h>
#include <utils/hello.h>
#include <utils/conexion.h>
#include <utils/paquete.h>
#include <commons/config.h>
#include <pthread.h>
#include <utils/config.h>
#include <utils/log.h>
#include <utils/estructuras.h>

void handshake_memoria(t_config* config);
void handshake_kernel(t_config* config);
void recibir_proceso(uint32_t);
void solicitar_instruccion(uint32_t, uint32_t,uint32_t);