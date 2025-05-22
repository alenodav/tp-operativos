#include <main.h>
#include "liberacion.h"

t_log *logger;
t_dictionary *diccionario_procesos;
char *memoria_usuario;
pthread_mutex_t mutex_diccionario;

int main(int argc, char *argv[])
{
    t_config *config = crear_config("memoria");
    logger = crear_log(config, "memoria");
    log_debug(logger, "Config y Logger creados correctamente.");

    memoria_usuario = string_repeat('-', atoi(config_get_string_value(config, "TAM_MEMORIA")));
    // Inicializo diccionario de procesos y su mutex
    diccionario_procesos = dictionary_create();
    pthread_mutex_init(&mutex_diccionario, NULL);
    log_debug(logger, "Diccionario de procesos creado correctamente.");

    uint32_t fd_escucha_memoria = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"));

    // Thread para CPU;
    pthread_t thread_escucha_cpu;
    pthread_create(&thread_escucha_cpu, NULL, (void*)handshake_cpu, &fd_escucha_memoria);
    pthread_detach(thread_escucha_cpu);

    // Thread para atender a Kernel
    pthread_t thread_escucha_kernel;
    pthread_create(&thread_escucha_kernel, NULL, (void*)handshake_kernel, &fd_escucha_memoria);
    pthread_detach(thread_escucha_kernel);

    getchar();

    liberar_conexion(fd_escucha_memoria);
    config_destroy(config);
    liberar_diccionario(diccionario_procesos);
    pthread_mutex_destroy(&mutex_diccionario);
    free(memoria_usuario);
    log_info(logger, "Finalizo el proceso.");
    log_destroy(logger);

    return 0;
}

void handshake_kernel(uint32_t fd_escucha_memoria)
{
    uint32_t cliente = esperar_cliente(fd_escucha_memoria);

    char *identificador = recibir_handshake(cliente);

    if (string_equals_ignore_case(identificador, "KERNEL"))
    {
        log_debug(logger, "Handshake Kernel a Memoria OK.");
    }
    else
    {
        log_error(logger, "Handshake Kernel a Memoria error.");
        free(identificador);
        liberar_conexion(cliente);
        return;
    }

    free(identificador);
    enviar_handshake(cliente, "MEMORIA");

    // Loop para atender Kernel
    while (1)
    {
        t_paquete *paquete = recibir_paquete(cliente);
        if (paquete == NULL)
        {
            log_error(logger, "Error al recibir paquete de Kernel");
            break;
        }

        switch (paquete->codigo_operacion)
        {
        case CONSULTA_MEMORIA_PROCESO:
            if (recibir_consulta_memoria(cliente))
            {
                // Si hay espacio, esperar las instrucciones
                recibir_instrucciones(cliente, 0); // verificar si el pid viene en la estructura
            }
            break;
        case TERMINAR_PROCESO:
            break;
        default:
            log_error(logger, "Operación desconocida de Kernel");
            break;
        }

        destruir_paquete(paquete);
    }

    log_info(logger, "Finalizando conexión con Kernel");
    liberar_conexion(cliente);
    return;
}

void handshake_cpu(uint32_t fd_escucha_memoria)
{
    uint32_t cliente = esperar_cliente(fd_escucha_memoria);

    char *identificador = recibir_handshake(cliente);

    if (string_equals_ignore_case(identificador, "cpu"))
    {
        log_debug(logger, "Handshake CPU a Memoria OK.");
    }
    else
    {
        log_error(logger, "Handshake CPU a Memoria error.");
        free(identificador);
        liberar_conexion(cliente);
        return;
    }

    free(identificador);
    enviar_handshake(cliente, "MEMORIA");

    bool proceso_terminado = false;
    // Loop para atender peticiones de CPU
    while (!proceso_terminado)
    {
        t_paquete *paquete = recibir_paquete(cliente);
        if (paquete == NULL)
        {
            log_error(logger, "Error al recibir paquete de CPU");
            break;
        }

        switch (paquete->codigo_operacion)
        {
        case FETCH:
            // CPU pide una instrucción
            proceso_terminado = enviar_instruccion(cliente);
            if (proceso_terminado)
            {
                log_info(logger, "Proceso terminado - instrucción EXIT enviada");
            }
            break;
        case READ_MEMORIA:
            // TODO: Implementar lectura de memoria
            cpu_read *parametros = deserializar_cpu_read(paquete->buffer);
            char* retorno = string_substring(memoria_usuario,parametros->direccion,parametros->direccion + parametros->tamanio);
            t_buffer *buffer_read = buffer_create(sizeof(uint32_t) + parametros->tamanio + 1);
            buffer_add_uint32(buffer_read, parametros->tamanio + 1);
            buffer_add_string(buffer_read, parametros->tamanio + 1, retorno);
            t_paquete *paquete_read = crear_paquete(READ_MEMORIA, buffer_read);
            enviar_paquete(paquete_read, cliente);
            break;
        case WRITE_MEMORIA:
            // TODO: Implementar escritura en memoria
            cpu_write *parametros_write = deserializar_cpu_write(paquete->buffer);
            for (int i = 0;i<string_length(parametros_write->datos);i++) {
                memoria_usuario[parametros_write->direccion + i] = parametros_write->datos[i];
            }
            break;
        default:
            log_error(logger, "Operación desconocida de CPU");
            break;
        }

        destruir_paquete(paquete);
    }

    log_info(logger, "Finalizando conexión con CPU");
    liberar_conexion(cliente);
    return;
}


bool recibir_consulta_memoria(uint32_t fd_kernel)
{
    t_paquete *paquete = recibir_paquete(fd_kernel);

    if (paquete->codigo_operacion != CONSULTA_MEMORIA_PROCESO)
    {
        log_error(logger, "Codigo de operacion incorrecto");
        return false;
    }

    // creo buffer para leer el paquete con pid y tamaño que se me envio
    t_buffer *buffer = paquete->buffer;
    uint32_t tamanio = buffer_read_uint32(buffer);

    // logueo
    log_info(logger, "Recibo consulta de espacio para tamaño %d", tamanio);

    bool hay_espacio = verificar_espacio_memoria( tamanio);

    // preparo el paquete de respuesta--           --creo buffer y le asigno tamaño del bool  que devuelve verificar_espacio_memoria
    t_paquete *respuesta = crear_paquete(CONSULTA_MEMORIA_PROCESO, buffer_create(sizeof(bool)));
    buffer_add_bool(respuesta->buffer, hay_espacio);
    enviar_paquete(respuesta, fd_kernel);

    destruir_paquete(paquete);
    destruir_paquete(respuesta);

    return hay_espacio;
}

void recibir_instrucciones(uint32_t fd_kernel, uint32_t pid)
{
    t_paquete *paquete = recibir_paquete(fd_kernel);

    if (paquete->codigo_operacion != SAVE_INSTRUCTIONS)
    {
        log_error(logger, "Codigo de operacion incorrecto para guardar las instrucciones");
        return;
    }

    kernel_to_memoria *kernelToMemoria = deserializar_kernel_to_memoria(paquete->buffer);

    log_info(logger, "Recibo archivo con instrucciones para PID %d", kernelToMemoria->pid);

    cargar_instrucciones(kernelToMemoria->archivo, kernelToMemoria->pid);

    free(kernelToMemoria->archivo);
    free(kernelToMemoria);
    destruir_paquete(paquete);
}

bool verificar_espacio_memoria(uint32_t tamanio)
{
    uint32_t tamanio_memoria_default = 4096;

    log_info(logger, "- Tamaño Memoria Total: %d - Tamaño Solicitado: %d - Espacio Disponible: %d",
             tamanio_memoria_default,
             tamanio,
             tamanio_memoria_default - tamanio);

    return tamanio_memoria_default > tamanio;
}


kernel_to_memoria *deserializar_kernel_to_memoria(t_buffer *buffer)
{
    kernel_to_memoria *data = malloc(sizeof(kernel_to_memoria));

    data->archivo_length = buffer_read_uint32(buffer);
    data->archivo = buffer_read_string(buffer, &data->archivo_length);
    data->tamanio = buffer_read_uint32(buffer);
    return data;
}


struct_memoria_to_cpu *parsear_linea(char *linea)
{
    char **token = string_split(linea, " ");

    struct_memoria_to_cpu *struct_memoria_to_cpu = malloc(sizeof(struct_memoria_to_cpu));
    struct_memoria_to_cpu->parametros = NULL;

    if (string_equals_ignore_case(token[0], "READ"))
    {
        struct_memoria_to_cpu->instruccion = READ;
    }
    else if (string_equals_ignore_case(token[0], "WRITE"))
    {
        struct_memoria_to_cpu->instruccion = WRITE;
    }
    else if (string_equals_ignore_case(token[0], "GOTO"))
    {
        struct_memoria_to_cpu->instruccion = GOTO;
    }
    else if (string_equals_ignore_case(token[0], "NOOP"))
    {
        struct_memoria_to_cpu->instruccion = NOOP;
    }
    else if (string_equals_ignore_case(token[0], "EXIT"))
    {
        struct_memoria_to_cpu->instruccion = EXIT;
    }
    else
    {
        log_error(logger, "Instrucción desconocida: %s\n", token[0]);
    }

    if (token[1] != NULL)
    {
        struct_memoria_to_cpu->parametros = string_new();
        for (int i = 1; token[i] != NULL; i++)
        {
            string_append_with_format(&struct_memoria_to_cpu->parametros, "%s%s", token[i], token[i + 1] ? " " : "");
        }
    }
    else
    {
        struct_memoria_to_cpu->parametros = string_duplicate("");
    }

    string_iterate_lines(token, (void*)free);
    free(token);

    return struct_memoria_to_cpu;
}

void cargar_instrucciones(char *path_archivo, uint32_t pid)
{
    FILE *archivo = fopen(path_archivo, "r");
    if (!archivo)
    {
        perror("No se pudo abrir el archivo de instrucciones");
        return;
    }

    t_list *lista_instrucciones = list_create();
    char *linea = NULL;
    size_t len = 0;

    while (getline(&linea, &len, archivo) != -1)
    {
        string_trim(&linea);
        if (string_is_empty(linea))
            continue;

        struct_memoria_to_cpu *instruccion = parsear_linea(linea);
        list_add(lista_instrucciones, instruccion);
    }

    free(linea);
    fclose(archivo);

    char *pid_str = string_itoa(pid);
    dictionary_put(diccionario_procesos, pid_str, lista_instrucciones);
    free(pid_str);
}

bool enviar_instruccion(uint32_t fd_cpu)
{
    t_paquete *paquete = recibir_paquete(fd_cpu);
    uint32_t pid = buffer_read_uint32(paquete->buffer);
    uint32_t pc = buffer_read_uint32(paquete->buffer);

    char *pid_str = string_itoa(pid);
    t_list *lista_instruccion_to_cpu = dictionary_get(diccionario_procesos, pid_str);
    free(pid_str);

    if (lista_instruccion_to_cpu == NULL || pc >= list_size(lista_instruccion_to_cpu))
    {
        log_error(logger, "PID no encontrado o PC no valido");
        destruir_paquete(paquete);
        return false;
    }

    struct_memoria_to_cpu *instruccion = list_get(lista_instruccion_to_cpu, pc);

    uint32_t tam_para = string_length(instruccion->parametros);

    t_paquete *paquete_retorno = crear_paquete(FETCH, buffer_create(0));
    buffer_add_uint8(paquete_retorno->buffer, instruccion->instruccion);
    buffer_add_string(paquete_retorno->buffer, tam_para, instruccion->parametros);

    enviar_paquete(paquete_retorno, fd_cpu);

    return instruccion->instruccion == EXIT;
}

/* void leer_configuracion(char* config){

} */


cpu_read *deserializar_cpu_read(t_buffer *data) {
    cpu_read *ret = malloc(sizeof(cpu_read));
    ret->direccion = buffer_read_uint32(data);
    ret->tamanio = buffer_read_uint32(data);
    return ret;
}

cpu_write *deserializar_cpu_write(t_buffer *data) {
    cpu_write *ret = malloc(sizeof(cpu_write));
    ret->direccion = buffer_read_uint32(data);
    ret->datos_length = buffer_read_uint32(data);
    ret->datos = buffer_read_string(data, &ret->datos_length);
    return ret;
}
