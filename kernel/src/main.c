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
    sem_init(sem_cpus, 0, 1);
    sem_init(sem_io, 0, 1);
    inicio_modulo = false;
    fd_escucha_cpu = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"));
    fd_escucha_cpu_interrupt = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT"));
    cpu_list = list_create();
    fd_escucha_io = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_IO"));  
    io_list = list_create();
    io_queue_list = list_create();
    pthread_t administrador_cpu_dispatch;
    pthread_create(&administrador_cpu_dispatch, NULL, (void*)administrar_cpus_dispatch, NULL);
    pthread_detach(administrador_cpu_dispatch);

    pthread_t administrador_cpu_interrupt;
    pthread_create(&administrador_cpu_interrupt, NULL, (void*)administrar_cpus_interrupt, NULL);
    pthread_detach(administrador_cpu_interrupt);

    pthread_t administrador_io;
    pthread_create(&administrador_io, NULL, (void*)administrar_dispositivos_io, NULL);
    pthread_detach(administrador_io);
    
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
    inicio_modulo = true;
    liberar_conexion(fd_escucha_cpu);
    liberar_conexion(fd_escucha_cpu_interrupt);

    pthread_t planificador_largo_plazo;
    pthread_create(&planificador_largo_plazo, NULL, (void*)largo_plazo, (void*)archivo_inicial);
    pthread_join(planificador_largo_plazo, NULL);

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
            list_remove(cola_new, 0);
            pasar_ready(pcb, list_get(pcb->metricas, NEW));
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

void pasar_ready(t_pcb *pcb, t_estado_metricas* metricas) {
    temporal_stop(metricas->MT);
    pcb->estado_actual = READY;
    if (metricas->estado == NEW) {
        t_estado_metricas *ready = crear_metrica_estado(READY);
        list_add(pcb->metricas, ready);
    }
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

void administrar_cpus_dispatch() {   
    while (inicio_modulo) {
        pthread_t thread;
        uint32_t *socket_cpu = malloc(sizeof(uint32_t));
        *socket_cpu = esperar_cliente(fd_escucha_cpu);
        pthread_create(&thread, NULL, (void*)agregar_cpu_dispatch, socket_cpu);
        pthread_detach(thread);
    } 
}

void agregar_cpu_dispatch(uint32_t* socket) {
    char *identificador = recibir_handshake(*socket);
    if (string_contains(identificador, "cpu")) {
        log_debug(logger, "Handshake CPU a Kernel OK.");
    }
    else {
        log_error(logger, "Handshake CPU a Kernel error.");
    }
    enviar_handshake(*socket, "KERNEL");
    t_cpu *cpu_agregar = malloc(sizeof(t_cpu));
    cpu_agregar->identificador = identificador;
    cpu_agregar->socket_dispatch = *socket;
    cpu_agregar->estado = false;
    sem_wait(sem_cpus);
    list_add(cpu_list, cpu_agregar);
    sem_post(sem_cpus);
    return;
}

void administrar_cpus_interrupt() {   
    while (inicio_modulo) {
        pthread_t thread;
        uint32_t *socket_cpu = malloc(sizeof(uint32_t));
        *socket_cpu = esperar_cliente(fd_escucha_cpu);
        pthread_create(&thread, NULL, (void*)agregar_cpu_interrupt, socket_cpu);
        pthread_detach(thread);
    } 
}

void agregar_cpu_interrupt(uint32_t* socket) {
    char *identificador = recibir_handshake(*socket);
    if (string_contains(identificador, "cpu")) {
        log_debug(logger, "Handshake CPU a Kernel OK.");
    }
    else {
        log_error(logger, "Handshake CPU a Kernel error.");
    }
    enviar_handshake(*socket, "KERNEL");
    t_cpu *cpu_a_guardar = cpu_find_by_id(identificador);
    sem_wait(sem_cpus);
    cpu_a_guardar->socket_interrupt = *socket;
    sem_post(sem_cpus);
    free(identificador);
    return;
}

t_cpu *cpu_find_by_id (char *id) {
    t_cpu *cpu_ret = malloc(sizeof(t_cpu));
    bool id_equals(void *cpu) {
        t_cpu *cpu_cast = (t_cpu*)cpu;
        return string_equals_ignore_case(cpu_cast->identificador, id);
    }   
    cpu_ret = list_find(cpu_list, id_equals);
    return cpu_ret;
}

void administrar_dispositivos_io () {
    while (1) {
        pthread_t thread;
        uint32_t *socket_io = malloc(sizeof(uint32_t));
        *socket_io = esperar_cliente(fd_escucha_cpu);
        pthread_create(&thread, NULL, (void*)agregar_io, socket_io);
        pthread_detach(thread);
    } 
}

void agregar_io (uint32_t *socket) {
    char *identificador = recibir_handshake(*socket);
    if (string_contains(identificador, "io")) {
        log_debug(logger, "Handshake IO a Kernel OK.");
    }
    else {
        log_error(logger, "Handshake IO a Kernel error.");
    }
    enviar_handshake(*socket, "KERNEL");
    t_io *io_agregar = malloc(sizeof(t_io));
    io_agregar->identificador = identificador;
    io_agregar->estado = false;
    io_agregar->socket = *socket;
    io_agregar->proceso_ejecucion = -1;
    if (io_queue_find_by_id(identificador) == NULL) {
        t_io_queue *io_queue_agregar = malloc(sizeof(t_io_queue));
        io_queue_agregar->id = identificador;
        io_queue_agregar->cola_procesos = queue_create();
        sem_wait(sem_io);
        list_add(io_queue_list, io_queue_agregar);
        sem_post(sem_io);
    }
    sem_wait(sem_io);
    list_add(io_list, io_agregar);
    sem_post(sem_io);
}

void ejecutar_io_syscall (uint32_t pid, char* id_io, uint32_t tiempo) {
    t_list *io_buscada = io_filter_by_id(id_io);
    if (list_size(io_buscada) < 1) {
        terminar_proceso(pid);
        return;
    }
    t_pcb *proceso = pcb_by_pid(cola_exec, pid);
    t_estado_metricas *metricas_exec = list_get(proceso->metricas, EXEC);
    temporal_stop(metricas_exec->MT);
    proceso->estado_actual = BLOCKED;
    t_estado_metricas *blocked = crear_metrica_estado(BLOCKED);
    list_add(proceso->metricas, blocked);
    list_add(cola_blocked, proceso);
    kernel_to_io *io_enviar = malloc(sizeof(io_enviar));
    io_enviar->pid = pid;
    io_enviar->tiempo_bloqueo = tiempo;
    t_io_queue *io_queue_buscada = io_queue_find_by_id(id_io);
    sem_wait(sem_io);
    queue_push(io_queue_buscada->cola_procesos, io_enviar);
    sem_post(sem_io);
    enviar_kernel_to_io(id_io);
}

void enviar_kernel_to_io (char* id) {
    t_list *io_buscada = io_filter_by_id(id);
    t_io *io_a_enviar = list_find(io_buscada, io_liberada);
    if (io_a_enviar == NULL) {
        return;
    }
    t_io_queue *io_queue_buscada = io_queue_find_by_id(id);
    sem_wait(sem_io);
    kernel_to_io *params = queue_pop(io_queue_buscada->cola_procesos);
    sem_post(sem_io);
    io_a_enviar->estado = true;
    io_a_enviar->proceso_ejecucion = params->pid;
    t_buffer *buffer = serializar_kernel_to_io(params);
    t_paquete *paquete = crear_paquete(IO, buffer);
    enviar_paquete(paquete, io_a_enviar->socket);
    pthread_t hilo_respuesta;
    pthread_create(&hilo_respuesta, NULL, (void*)manejar_respuesta_io, io_a_enviar);
    pthread_detach(hilo_respuesta); 
}

void manejar_respuesta_io(t_io *io_espera) {
    t_paquete *paquete = recibir_paquete(io_espera->socket);
    if (paquete->codigo_operacion != IO) {
        log_error(logger, "(%d) Codigo de operacion incorrecto para IO", io_espera->proceso_ejecucion);
        terminar_proceso(io_espera->proceso_ejecucion);
        sem_wait(sem_io);
        list_remove_element(io_list, io_espera);
        sem_post(sem_io);
        liberar_conexion(io_espera->socket);
        free(io_espera->identificador);
        free(io_espera);
        destruir_paquete(paquete);
        return;
    }
    t_pcb *pcb = pcb_by_pid(cola_blocked, io_espera->proceso_ejecucion);
    pasar_ready(pcb, list_get(pcb->metricas, BLOCKED));
    io_espera->estado = false;
    io_espera->proceso_ejecucion = -1;
    t_io_queue *cola_io = io_queue_find_by_id(io_espera->identificador);
    if (!queue_is_empty(cola_io->cola_procesos)) {
        enviar_kernel_to_io(io_espera->identificador);
    }
    return;
}

t_list *io_filter_by_id (char *id) {
    t_list *io_ret = malloc(sizeof(t_io));
    bool id_equals(void *io) {
        t_io *io_cast = (t_io*)io;
        return string_equals_ignore_case(io_cast->identificador, id);
    }   
    io_ret = list_filter(io_list, id_equals);
    return io_ret;
}

t_io_queue *io_queue_find_by_id (char *id) {
    t_io_queue *io_ret = malloc(sizeof(t_io_queue));
    bool id_equals(void *io) {
        t_io_queue *io_cast = (t_io_queue*)io_cast;
        return string_equals_ignore_case(io_cast->id, id);
    }   
    io_ret = list_find(io_queue_list, id_equals);
    return io_ret;
}

bool io_liberada(void* io) {
    t_io *io_cast = (t_io*) io;
    return !(io_cast->estado);
}

t_buffer *serializar_kernel_to_io (kernel_to_io* data) {
    t_buffer *buffer = buffer_create(sizeof(uint32_t) * 2);
    buffer_add_uint32(buffer, data->pid);
    buffer_add_uint32(buffer, data->tiempo_bloqueo);
    return buffer;
}