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

    // Conexion a Kernel , dispatch y interrupt
    pthread_t thread_kernel;
    pthread_create(&thread_kernel, NULL, handshake_kernel, (void*)id_cpu);
    pthread_detach(thread_kernel);

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
    char* id_cpu = (char*)arg;

    // Conexion dispatch
    fd_dispatch = crear_socket_cliente(config_get_string_value(config,"IP_KERNEL"),config_get_string_value(config,"PUERTO_KERNEL_DISPATCH"));
    enviar_handshake(fd_dispatch, id_cpu);
    char* identificador_dispatch = recibir_handshake(fd_dispatch);

    if (string_equals_ignore_case(identificador_dispatch, "kernel")) {
        log_debug(logger, "Handshake Kernel DISPATCH a CPU-%s OK.", id_cpu);
    } else {
        log_error(logger, "Handshake Kernel DISPATCH a CPU-%s error.", id_cpu);
    }
    free(identificador_dispatch);

    // Conexion interrupt
    fd_interrupt = crear_socket_cliente(config_get_string_value(config,"IP_KERNEL"),config_get_string_value(config,"PUERTO_KERNEL_INTERRUPT"));
    enviar_handshake(fd_interrupt, id_cpu);
    char* identificador_interrupt = recibir_handshake(fd_interrupt);

    if (string_equals_ignore_case(identificador_interrupt, "kernel")) {
        log_debug(logger, "Handshake Kernel INTERRUPT a CPU-%s OK.", id_cpu);
    } else {
        log_error(logger, "Handshake Kernel INTERRUPT a CPU-%s error.", id_cpu);
    }
    free(identificador_interrupt);
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
    // fetch
    t_buffer* buffer = buffer_create(sizeof(kernel_to_cpu));
    kernel_to_cpu instruccion;
    instruccion.pid = pid;
    instruccion.pc = pc;
    buffer_add(buffer, &instruccion, sizeof(kernel_to_cpu));

    t_paquete* paquete = crear_paquete(FETCH,buffer);
    enviar_paquete(paquete,fd_memoria);

    buffer_destroy(buffer);
    destruir_paquete(paquete);

    // recibo la siguiente instruccion de memoria
    t_paquete* siguiente_instruccion = recibir_paquete(fd_memoria);
    
    t_syscall syscall;
    syscall.syscall = buffer_read_uint32(siguiente_instruccion->buffer);
    syscall.parametros = NULL;
    syscall.parametros_length = buffer_read_uint32(siguiente_instruccion->buffer);
    syscall.pid = buffer_read_uint32(siguiente_instruccion->buffer);
    
    if(syscall.parametros_length > 0) {
        syscall.parametros = malloc(syscall.parametros_length);
        buffer_read(siguiente_instruccion->buffer, syscall.parametros, syscall.parametros_length);
    }

    switch (syscall.syscall) {
        case NOOP:
            log_debug(logger, "PID: %d - EXECUTE - NOOP", pid);
            break;
        case WRITE: {
            // DECODE
            cpu_write* write_params = (cpu_write*)syscall.parametros;
            
            // EXECUTE
            log_debug(logger, "PID: %d - EXECUTE - WRITE - Dirección: %d, Valor: %s", pid, write_params->direccion, write_params->datos);

            t_buffer* buffer = buffer_create(sizeof(cpu_write));
            buffer_add(buffer, write_params, sizeof(cpu_write));

            t_paquete* paquete = crear_paquete(WRITE, buffer);
            enviar_paquete(paquete, fd_memoria);

            buffer_destroy(buffer);
            destruir_paquete(paquete);

            t_paquete* respuesta = recibir_paquete(fd_memoria);
            uint32_t length_respuesta = buffer_read_uint32(respuesta->buffer);
            char* mensaje = buffer_read_string(respuesta->buffer,&length_respuesta);
            
            if(string_equals_ignore_case(mensaje, "OK")) {
                log_debug(logger, "PID: %d - WRITE completado exitosamente", pid);
            } else {
                log_error(logger, "PID: %d - Error en WRITE: %s", pid, mensaje);
            }
            
            free(mensaje);
            destruir_paquete(respuesta);
            break;
        }
        case READ: {
            // DECODE
            cpu_read* read_params = (cpu_read*)syscall.parametros;
            
            // EXECUTE
            log_debug(logger, "PID: %d - EXECUTE - READ - Dirección: %d, Tamaño: %d", pid, read_params->direccion, read_params->tamanio);            
            
            t_buffer* buffer = buffer_create(sizeof(cpu_read));
            buffer_add(buffer, read_params, sizeof(cpu_read));

            t_paquete* paquete = crear_paquete(READ,buffer);
            enviar_paquete(paquete,fd_memoria);

            buffer_destroy(buffer);
            destruir_paquete(paquete);
    
            t_paquete* respuesta = recibir_paquete(fd_memoria);
            uint32_t length_respuesta = buffer_read_uint32(respuesta->buffer);
            char* valor_leido = buffer_read_string(respuesta->buffer, &length_respuesta);
            
            printf("PID: %d - READ - Valor leído: %s\n", pid, valor_leido);
            log_debug(logger, "PID: %d - READ - Valor leído: %s", pid, valor_leido);
            
            free(valor_leido);
            destruir_paquete(respuesta);
            break;
        }
        case GOTO: {
            // DECODE
            uint32_t nueva_direccion = *(uint32_t*)syscall.parametros;

            log_debug(logger, "PID: %d - EXECUTE - GOTO - Nueva dirección: %d", pid, nueva_direccion);

            t_buffer* buffer = buffer_create(sizeof(uint32_t));
            buffer_add_uint32(buffer, nueva_direccion);

            t_paquete* paquete = crear_paquete(GOTO, buffer);
            enviar_paquete(paquete, fd_memoria);

            buffer_destroy(buffer);
            destruir_paquete(paquete);

            t_paquete* respuesta = recibir_paquete(fd_memoria);
            uint32_t length_respuesta = buffer_read_uint32(respuesta->buffer);
            char* mensaje = buffer_read_string(respuesta->buffer, &length_respuesta);
            
            if(string_equals_ignore_case(mensaje, "OK")) {
                log_debug(logger, "PID: %d - GOTO completado exitosamente", pid);
            } else {
                log_error(logger, "PID: %d - Error en GOTO: %s", pid, mensaje);
            }
            
            free(mensaje);
            destruir_paquete(respuesta);
            break;
        }
        case IO_SYSCALL: {
            io_parameters* io_params = (io_parameters*)syscall.parametros; 
            log_debug(logger, "PID: %d - EXECUTE - IO - Dispositivo: %s, Tiempo: %d",pid, io_params->identificador, io_params->tiempo_bloqueo);

            t_buffer* buffer = buffer_create(sizeof(io_parameters));
            buffer_add(buffer, io_params, sizeof(io_parameters));

            t_paquete* paquete = crear_paquete(IO, buffer);
            enviar_paquete(paquete, fd_interrupt);

            buffer_destroy(buffer);
            destruir_paquete(paquete);
            break;
        }
        case INIT_PROC: {
            init_proc_parameters* init_params = (init_proc_parameters*)syscall.parametros;

            log_debug(logger, "PID: %d - EXECUTE - INIT_PROC - Nombre: %s, Tamaño: %d",pid, init_params->archivo, init_params->archivo_lenght);

            t_buffer* buffer = buffer_create(sizeof(init_proc_parameters));
            buffer_add(buffer, init_params, sizeof(init_proc_parameters));

            t_paquete* paquete = crear_paquete(INIT_PROC, buffer);
            enviar_paquete(paquete, fd_interrupt);

            buffer_destroy(buffer);
            destruir_paquete(paquete);
            break;
        }
        case DUMP_MEMORY: {
            log_debug(logger, "PID: %d - EXECUTE - DUMP_MEMORY", pid);

            t_buffer* buffer = buffer_create(sizeof(uint32_t));
            buffer_add_uint32(buffer, pid);

            t_paquete* paquete = crear_paquete(DUMP_MEMORY, buffer);
            enviar_paquete(paquete, fd_interrupt);

            buffer_destroy(buffer);
            destruir_paquete(paquete);
            break;
        }
        case EXIT: {
            log_debug(logger, "PID: %d - EXECUTE - EXIT", pid);

            t_buffer* buffer = buffer_create(sizeof(uint32_t));
            buffer_add_uint32(buffer, pid);

            t_paquete* paquete = crear_paquete(EXIT, buffer);
            enviar_paquete(paquete, fd_interrupt);

            buffer_destroy(buffer);
            destruir_paquete(paquete);
            break;
        }
        default:
            log_error(logger, "PID: %d - Instrucción desconocida", pid);
            break;
    }
    if(syscall.syscall != GOTO) {
        pc++;
    }
    if(syscall.parametros != NULL) {
        free(syscall.parametros);
    }
    destruir_paquete(siguiente_instruccion);
    
    // Check Interrupt de kernel
    t_paquete* interrupcion = recibir_paquete(fd_interrupt);
    if(interrupcion != NULL) {
        uint32_t pid_interrupcion = buffer_read_uint32(interrupcion->buffer);
        if(pid_interrupcion == pid) {
            log_debug(logger, "PID: %d - Interrupción recibida del kernel", pid);
            t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2);
            buffer_add_uint32(buffer, pid);
            buffer_add_uint32(buffer, pc);
            
            t_paquete* respuesta = crear_paquete(INTERRUPT, buffer);
            enviar_paquete(respuesta, fd_interrupt);

            buffer_destroy(buffer);
            destruir_paquete(respuesta);
        } else {
            log_debug(logger, "PID: %d - Interrupción descartada (para PID: %d)", pid, pid_interrupcion);
        }
        destruir_paquete(interrupcion);
    }

}