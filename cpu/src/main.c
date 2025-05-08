#include <main.h>

t_log* logger;
t_config* config;
uint32_t fd_dispatch;
uint32_t fd_interrupt;
uint32_t fd_memoria;

int main(int argc, char* argv[]){
    char* id_cpu = argv[1];
    char log_filename[64];
    sprintf(log_filename, "cpu-%s.log", id_cpu);
    logger = log_create(log_filename, "CPU", true, LOG_LEVEL_DEBUG);
    log_debug(logger, "Logger de CPU id:%s creado.", id_cpu);

    config = crear_config("cpu");

    // Conexion a Memoria
    pthread_t thread_memoria;
    pthread_create(&thread_memoria, NULL, handshake_memoria, (void*)id_cpu);
    pthread_detach(thread_memoria); 

    // Conexion a Kernel DISPATCH
    t_handshake_args* args_dispatch = malloc(sizeof(t_handshake_args));
    args_dispatch->id_cpu = id_cpu;
    args_dispatch->tipo_conexion = "DISPATCH";

    pthread_t thread_kernel_dispatch;
    pthread_create(&thread_kernel_dispatch, NULL, handshake_kernel, (void*)args_dispatch);
    pthread_detach(thread_kernel_dispatch);

    // Conexion a Kernel INTERRUPT
    t_handshake_args* args_interrupt = malloc(sizeof(t_handshake_args));
    args_interrupt->id_cpu = id_cpu;
    args_interrupt->tipo_conexion = "INTERRUPT";

    pthread_t thread_kernel_interrupt;
    pthread_create(&thread_kernel_interrupt, NULL, handshake_kernel, (void*)args_interrupt);
    pthread_detach(thread_kernel_interrupt);

    getchar();

    log_destroy(logger);
    config_destroy(config);
    return 0;
}

void handshake_memoria(void* arg){
    char* id_cpu = (char*)arg;
    fd_memoria = crear_socket_cliente(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"));

    enviar_handshake(fd_memoria, "CPU");
    char* identificador = recibir_handshake(fd_memoria);

    if (string_equals_ignore_case(identificador, "memoria")) {
        log_debug(logger, "Handshake Memoria a CPU-%s OK.",id_cpu);
    }
    else {
        log_error(logger, "Handshake Memoria a CPU-%s error.",id_cpu);
    }

    free(identificador);
}

void handshake_kernel(void* arg){
    t_handshake_args* args = (t_handshake_args*)arg;
    char* id_cpu = args->id_cpu;
    char* tipo = args->tipo_conexion;

    // conexion dispatch
    if(string_equals_ignore_case(tipo,"DISPATCH")){
        fd_dispatch = crear_socket_cliente(config_get_string_value(config,"IP_KERNEL"),config_get_string_value(config,"PUERTO_KERNEL_DISPATCH"));

        enviar_handshake(fd_dispatch,id_cpu);
        char* identificador = recibir_handshake(fd_dispatch);

        if (string_equals_ignore_case(identificador, "kernel")) {
            log_debug(logger, "Handshake Kernel DISPATCH a CPU-%s OK.", id_cpu);
        } else {
            log_error(logger, "Handshake Kernel DISPATCH a CPU-%s error.", id_cpu);
        }

        free(identificador);

    }else if(string_equals_ignore_case(tipo,"INTERRUPT")){ 

    // conexion interrupt
        fd_interrupt = crear_socket_cliente(config_get_string_value(config,"IP_KERNEL"),config_get_string_value(config,"PUERTO_KERNEL_INTERRUPT"));

        enviar_handshake(fd_interrupt,id_cpu);
        char* identificador = recibir_handshake(fd_interrupt);

        if (string_equals_ignore_case(identificador, "kernel")) {
            log_debug(logger, "Handshake Kernel INTERRUPT a CPU-%s OK.", id_cpu);
        } else {
            log_error(logger, "Handshake Kernel INTERRUPT a CPU-%s error.", id_cpu);
        }

        free(identificador);
    }

    free(args);
}

void recibir_proceso(void* _){
    while(1){
        // recibo del kernel el pid y pc
        t_paquete* paquete = recibir_paquete(fd_dispatch);

        kernel_to_cpu paquete_proceso;
        paquete_proceso.pid = buffer_read_uint32(paquete->buffer);
        paquete_proceso.pc = buffer_read_uint32(paquete->buffer);

        log_debug(logger, "Recibido PID: %d - PC: %d", paquete_proceso.pid, paquete_proceso.pc);


        // FETCH a memoria
        solicitar_instruccion(paquete_proceso.pid, paquete_proceso.pc);

        destruir_paquete(paquete);
    }
}

void solicitar_instruccion(uint32_t pid,uint32_t pc){
    t_buffer* buffer = buffer_create(sizeof(uint32_t)*2);
    buffer_add_uint32(buffer,pid);
    buffer_add_uint32(buffer,pc);

    t_paquete* paquete = crear_paquete(FETCH,buffer);
    enviar_paquete(paquete,fd_memoria);

    buffer_destroy(buffer);
    destruir_paquete(paquete);

    // recibo la siguiente instruccion de memoria
    t_paquete* siguiente_instruccion = recibir_paquete(fd_memoria);
    
    // DECODE
    uint32_t instruccion_int = buffer_read_uint32(siguiente_instruccion->buffer);
    t_instruccion instruccion = (t_instruccion)instruccion_int;

    switch (instruccion) {
        case NOOP:
            log_debug(logger, "PID: %d - Ejecutando: NOOP", pid);
            break;
        case WRITE: {
            uint32_t direccion = buffer_read_uint32(siguiente_instruccion->buffer);
            uint32_t length = buffer_read_uint32(siguiente_instruccion->buffer);
            char* valor = buffer_read_string(siguiente_instruccion->buffer, &length);
            log_debug(logger, "PID: %d - Ejecutando: WRITE - Direccion: %d, valor: %d", pid, direccion, valor);
            break;
        }
        case READ: {
            uint32_t direccion = buffer_read_uint32(siguiente_instruccion->buffer);
            uint32_t tamanio = buffer_read_uint32(siguiente_instruccion->buffer);
            log_debug(logger, "PID: %d - Ejecutando: READ - Direccion: %d, tamanio: %d", pid, direccion, tamanio);
            break;
        }
        case GOTO: {
            uint32_t direccion = buffer_read_uint32(siguiente_instruccion->buffer);
            log_debug(logger, "PID: %d - Ejecutando: GOTO - direccion: %d", pid, direccion);
            break;
        }
        case IO_SYSCALL: {
             uint32_t length = buffer_read_uint32(siguiente_instruccion->buffer);
            char* dispositivo = buffer_read_string(siguiente_instruccion->buffer, &length);
            uint32_t tiempo = buffer_read_uint32(siguiente_instruccion->buffer);
            log_debug(logger, "PID: %d - Ejecutando: IO - Dispositivo: %s, Tiempo: %d", pid, dispositivo, tiempo);
            break;
        }
        case INIT_PROC: {
            uint32_t length = buffer_read_uint32(siguiente_instruccion->buffer);
            char* nombre = buffer_read_string(siguiente_instruccion->buffer, &length);
            uint32_t tamanio = buffer_read_uint32(siguiente_instruccion->buffer);
            log_debug(logger, "PID: %d - Ejecutando: INIT_PROC - Nombre: %s, Tamaño: %d", pid, nombre, tamanio);
            free(nombre);
            break;
        }
        case DUMP_MEMORY:
            log_debug(logger, "PID: %d - Ejecutando: DUMP_MEMORY", pid);
            break;
        case EXIT:
            log_debug(logger, "PID: %d - Ejecutando: EXIT", pid);
            break;
        default:
            log_error(logger, "PID: %d - Instrucción desconocida", pid);
            break;
    }



    destruir_paquete(siguiente_instruccion);
}