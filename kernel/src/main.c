#include<main.h>

int main(int argc, char* argv[]) {
    config = crear_config("kernel");
    logger = crear_log(config, "kernel");
    log_debug(logger, "Config y Logger creados correctamente.");

    pid_counter = 0;
    kernel_to_memoria *archivo_inicial = malloc(sizeof(kernel_to_memoria));
    archivo_inicial->archivo = argv[1];
    archivo_inicial->archivo_length = string_length(archivo_inicial->archivo) + 1;
    archivo_inicial->tamanio = atoi(argv[2]);
    cola_new = list_create();
    cola_ready = list_create();
    cola_blocked = list_create();
    cola_exec = list_create();
    archivos_instruccion = list_create();
    list_add(archivos_instruccion, archivo_inicial);
    sem_init(sem_largo_plazo, 0, 1);
    
    // pthread_t thread_io;
    // pthread_create(&thread_io, NULL, escucha_io, config); // Se crea un thread que corra escucha_io(config)
    // pthread_detach(thread_io);  // Se separa la ejecución del thread de la del programa principal

    // pthread_t thread_memoria;
    // pthread_create(&thread_memoria, NULL, handshake_memoria, config);
    // pthread_detach(thread_memoria); 

    // //sleep(1);

    // //Creo un hilo para que corra escucha_cpu(config)
    // pthread_t thread_cpu;
    // pthread_create(&thread_cpu, NULL, escucha_cpu, config);
    // pthread_detach(thread_cpu);

    getchar(); // Para que el progrma no termine antes que los threads

    pthread_t planificador_largo_plazo;
    pthread_create(&planificador_largo_plazo, NULL, (void*)largo_plazo, (void*)archivo_inicial);
    pthread_join(planificador_largo_plazo, NULL);

    // pthread_t administrador_dispositivos;
    // pthread_create(&administrador_dispositivos, NULL, (void*)administrar_dispositivos, NULL);
    // pthread_join(administrador_dispositivos, NULL);

    list_destroy(cola_new);
    list_destroy(cola_ready);
    list_destroy(cola_blocked);
    list_destroy(cola_exec);
    list_destroy(archivos_instruccion);

    config_destroy(config);
    log_info(logger, "Finalizo el proceso.");
    log_destroy(logger);

    return 0;
}

void escucha_io(){
    uint32_t fd_escucha_io = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_IO"));    
    uint32_t socket_io = esperar_cliente(fd_escucha_io);
    char *identificador = recibir_handshake(socket_io);
    if (string_equals_ignore_case(identificador, "io")) {
        log_debug(logger, "Handshake IO a Kernel OK.");
    }
    else {
        log_error(logger, "Handshake IO a Kernel error.");
    }
    free(identificador);
    enviar_handshake(socket_io, "KERNEL");
    liberar_conexion(socket_io);
    liberar_conexion(fd_escucha_io);
    return;
}

//Creo escucha cpu
void escucha_cpu(){
    uint32_t fd_escucha_cpu = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"));    
    uint32_t socket_cpu = esperar_cliente(fd_escucha_cpu);
    char *identificador = recibir_handshake(socket_cpu);
    if (string_equals_ignore_case(identificador, "cpu")) {
        log_debug(logger, "Handshake CPU a Kernel OK.");
    }
    else {
        log_error(logger, "Handshake CPU a Kernel error.");
    }
    free(identificador);
    enviar_handshake(socket_cpu, "KERNEL");
    liberar_conexion(socket_cpu);
    liberar_conexion(fd_escucha_cpu);
    return;
}


void handshake_memoria() {
    uint32_t fd_conexion_memoria = crear_socket_cliente(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"));

    enviar_handshake(fd_conexion_memoria, "KERNEL");

    char* identificador = recibir_handshake(fd_conexion_memoria);
    if (string_equals_ignore_case(identificador, "memoria")) {
        log_debug(logger, "Handshake Memoria a Kernel OK.");
    }
    else {
        log_error(logger, "Handshake Memoria a Kernel error.");
    }

    free(identificador);
    liberar_conexion(fd_conexion_memoria);
    return;
}

void largo_plazo() {
    //Seleccion de algoritmo
    char* algoritmo_config = config_get_string_value(config, "ALGORITMO_COLA_NEW");
    if (string_equals_ignore_case(algoritmo_config, "FIFO")) {
        planificar_fifo_largo_plazo();
    }
    else if (string_equals_ignore_case(algoritmo_config, "PMCP")) {

    }
    else {
        log_error(logger, "Algoritmo desconocido para planificador de largo plazo.");
    }
    return;
}

void planificar_fifo_largo_plazo() {
    while(1) {
        if (list_is_empty(archivos_instruccion)) {
            sem_wait(sem_largo_plazo);
        }
        t_pcb *pcb = crear_proceso();
        bool consulta_memoria = consultar_a_memoria();
        if (consulta_memoria) {
            enviar_instrucciones();
            pasar_ready();
        }
        else {
            log_debug(logger, "## (%d) No hay espacio suficiente para inicializar el proceso", pcb->pid);
            sem_wait(sem_largo_plazo);
        }
    }
}

t_estado_metricas *crear_metrica_estado(t_estado estado) {
    t_estado_metricas *metrica_agregar = malloc(sizeof(t_estado_metricas));
    metrica_agregar->estado = estado;
    metrica_agregar->ME = 1;
    metrica_agregar->MT = temporal_create();
    return metrica_agregar;
}

t_pcb* crear_proceso() {
    kernel_to_memoria *archivo = list_get(archivos_instruccion, 0);
    t_pcb *pcb = malloc(sizeof(t_pcb));
    pcb->pid = pid_counter;
    pcb->pc = 0;
    pcb->metricas = list_create();
    t_estado_metricas *new = crear_metrica_estado(NEW);
    list_add(pcb->metricas, new);
    pcb->estado_actual = NEW;
    list_add(cola_new, pcb);
    archivo->pid = pid_counter;
    pid_counter += 1;
    return pcb;
}

bool consultar_a_memoria() {
    kernel_to_memoria *archivo = list_get(archivos_instruccion, 0);
    uint32_t tamanio_proceso = archivo->tamanio;
    uint32_t pid = archivo->pid;
    bool ret = false;
    uint32_t fd_conexion_memoria = crear_socket_cliente(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"));
    t_buffer* buffer = buffer_create(sizeof(uint32_t));
    buffer_add_uint32(buffer, tamanio_proceso);
    t_paquete* paquete = crear_paquete(CONSULTA_MEMORIA_PROCESO, buffer);
    enviar_paquete(paquete, fd_conexion_memoria);
    t_paquete *retorno = recibir_paquete(fd_conexion_memoria);
    if (retorno->codigo_operacion == CONSULTA_MEMORIA_PROCESO) {
        ret = buffer_read_bool(paquete->buffer);
    }
    else {
        log_error(logger, "## (%d) Codigo de operación incorrecto para consultar a memoria.", pid);
    }
    free(retorno);
    liberar_conexion(fd_conexion_memoria);
    return ret;
}

void enviar_instrucciones() {
    kernel_to_memoria *archivo = list_remove(archivos_instruccion, 0);
    uint32_t fd_conexion_memoria = crear_socket_cliente(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"));
    t_buffer *buffer = serializar_kernel_to_memoria(archivo);
    t_paquete *consulta = crear_paquete(SAVE_INSTRUCTIONS, buffer);
    enviar_paquete(consulta, fd_conexion_memoria);
    liberar_conexion(fd_conexion_memoria);
    free(archivo);
    return;
}

t_buffer *serializar_kernel_to_memoria(kernel_to_memoria* archivo) {
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * 2 + archivo->archivo_length);
    buffer_add_string(buffer, archivo->archivo_length, archivo->archivo);
    buffer_add_uint32(buffer, archivo->archivo_length);
    buffer_add_uint32(buffer, archivo->tamanio);
    return buffer;
}

void pasar_ready() {
    t_pcb *pcb = list_remove(cola_new, 0);
    t_estado_metricas *metricas_new = list_get(pcb->metricas, 0);
    temporal_stop(metricas_new->MT);
    pcb->estado_actual = READY;
    t_estado_metricas *ready = crear_metrica_estado(READY);
    list_add(pcb->metricas, ready);
    list_add(cola_ready, pcb);
}

bool terminar_proceso_memoria (uint32_t pid) {
    bool retorno = false;
    uint32_t fd_conexion_memoria = crear_socket_cliente(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"));
    t_buffer *buffer = buffer_create(sizeof(uint32_t));
    buffer_add_uint32(buffer, pid);
    t_paquete *paquete = crear_paquete(TERMINAR_PROCESO, buffer);
    enviar_paquete(paquete, fd_conexion_memoria);
    t_paquete *respuesta = recibir_paquete(fd_conexion_memoria);
    if(respuesta->codigo_operacion != TERMINAR_PROCESO) {
        log_error(logger, "(%d) Codigo de operacion incorrecto para terminar_proceso.", pid);
    }
    else {
        bool confirmacion = buffer_read_bool(respuesta->buffer);
        if (!confirmacion) {
            log_error(logger, "(%d) Error en terminar_proceso. Revisar log memoria.", pid);
        }
        else{
            retorno = true;
        }
    }
    liberar_conexion(fd_conexion_memoria);
    destruir_paquete(respuesta);
    return retorno;
}

void terminar_proceso(uint32_t pid) {
    bool consulta_memoria = terminar_proceso_memoria(pid);
    if (!consulta_memoria) {
        return;
    }
    t_pcb *proceso = pcb_by_pid(cola_exec, pid);
    t_estado_metricas *metricas_exec = list_get(proceso->metricas, EXEC);
    temporal_stop(metricas_exec->MT);
    proceso->estado_actual = EXIT_STATUS;
    t_estado_metricas *exit = crear_metrica_estado(EXIT_STATUS);
    list_add(proceso->metricas, exit);
    loggear_metricas_estado(proceso);
    free(proceso);
    sem_post(sem_largo_plazo);

    return;
}

t_pcb* pcb_by_pid(t_list* pcb_list, uint32_t pid) {
    bool pid_equals(void *pcb) {
        t_pcb *pcb_cast = (t_pcb*)pcb;
        return pcb_cast->pid == pid;
    }
    return list_remove_by_condition(pcb_list, pid_equals);
}

void loggear_metricas_estado(t_pcb* proceso) {
    char* metricas_estado = "";
    t_list_iterator *metricas_iterator = list_iterator_create(proceso->metricas);
    while(list_iterator_has_next(metricas_iterator)){
        t_estado_metricas* metrica = list_iterator_next(metricas_iterator);
        string_append_with_format(&metricas_estado, "%s  (%d) (%lu), ", t_estado_to_string(metrica->estado), metrica->ME, metrica->MT->elapsed_ms);
        list_iterator_remove(metricas_iterator);
        free(metrica);
    }
    list_destroy(proceso->metricas);
    log_info(logger, "## (%d) - Metricas de estado: %s", proceso->pid, metricas_estado);
}

char* t_estado_to_string(t_estado estado) {
    switch (estado)
    {
    case NEW:
        return "NEW";
        break;
    case READY:
        return "READY";
        break;
    case EXEC: 
        return "EXEC";
        break;
    case BLOCKED:
        return "BLOCKED";
        break;
    case SUSP_BLOCKED:
        return "SUSPENDED BLOCKED";
        break;
    case SUSP_READY:
        return "SUSPENDED READY";
        break;
    case EXIT_STATUS:
        return "EXIT";
        break;

    default:
        return "";
        break;
    }
}
