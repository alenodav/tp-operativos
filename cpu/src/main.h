#include <utils/config.h>
#include <utils/log.h>
#include <utils/hello.h>
#include <utils/conexion.h>
#include <utils/paquete.h>
#include <pthread.h>


/* --- declaraciones de logging --- */

t_log* iniciar_logger(void);

/* --- declaraciones de config --- */

t_config* iniciar_config(void);