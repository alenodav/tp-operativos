#include "mmu.h"


uint32_t tamanio_pagina;
uint32_t cant_entradas_tabla;
uint32_t cant_niveles;
uint32_t fd_memoria;

t_config* config_cpu;

void recibir_t_config_to_cpu() {
    t_paquete* paquete = recibir_paquete(fd_memoria);
    t_config_to_cpu *config_to_cpu = deserializar_t_config_to_cpu(paquete->buffer);
    tamanio_pagina = config_to_cpu->tam_paginas;
    cant_niveles = config_to_cpu->cantidad_niveles;
    cant_entradas_tabla = config_to_cpu->cant_entradas;
    free(config_to_cpu);
    destruir_paquete(paquete);
}

t_config_to_cpu *deserializar_t_config_to_cpu(t_buffer* buffer) {
    t_config_to_cpu *retorno = malloc(sizeof(t_config_to_cpu));
    retorno->cantidad_niveles = buffer_read_uint32(buffer);
    retorno->tam_paginas = buffer_read_uint32(buffer);
    retorno->cant_entradas = buffer_read_uint32(buffer);
    return retorno;
}

uint32_t numero_pagina(uint32_t direccion_logica){
    return (uint32_t)floor(direccion_logica / tamanio_pagina);
}

uint32_t entrada_nivel_X(uint32_t numero_pagina, uint32_t nivel){
    return (uint32_t)(floor(numero_pagina / cant_entradas_tabla ^ (cant_niveles-nivel))%cant_entradas_tabla);
}

uint32_t desplazamiento(uint32_t direccion_logica){
    return direccion_logica % tamanio_pagina;
}

uint32_t calcular_direccion_fisica(uint32_t direccion_logica, uint32_t pid) {
    uint32_t nro_marco = consultar_marco(direccion_logica, pid);
    uint32_t offset = desplazamiento(direccion_logica);
    return nro_marco * tamanio_pagina + offset;
}

uint32_t consultar_marco(uint32_t direccion_logica, uint32_t pid) {
    uint32_t nro_marco = -1;
    uint32_t nro_pagina = numero_pagina(direccion_logica); 
    uint32_t *indices = calloc(cant_niveles, sizeof(uint32_t));
    for (int i = 0; i < cant_niveles; i++) {
        indices[i] = entrada_nivel_X(nro_pagina, i + 1);
    }
    nro_marco = consultar_marco_memoria(indices, pid);
    free(indices);
    return nro_marco;
}

uint32_t consultar_marco_memoria(uint32_t* indices, uint32_t pid) {
    t_buffer* buffer = serializar_indices_tablas(indices, pid);
    t_paquete* paquete = crear_paquete(CONSULTA_MARCO, buffer);
    enviar_paquete(paquete, fd_memoria);

    t_paquete* respuesta = recibir_paquete(fd_memoria);
    uint32_t marco = buffer_read_uint32(paquete->buffer);
    destruir_paquete(respuesta);
    return marco;
}

t_buffer* serializar_indices_tablas(uint32_t* indices, uint32_t pid) {
    t_buffer* buffer = buffer_create(sizeof(uint32_t) * (cant_niveles + 1));
    buffer_add_uint32(buffer, pid);
    for(int i = 0; i < cant_niveles; i++) {
        buffer_add_uint32(buffer, indices[i]);
    }
    return buffer;
}