#ifndef MMU_H_
#define MMU_H_

#include <math.h>
#include <stdint.h>
#include <utils/config.h>
#include<utils/paquete.h>
#include<utils/estructuras.h>

void recibir_t_config_to_cpu();
t_config_to_cpu *deserializar_t_config_to_cpu(t_buffer* buffer);
uint32_t numero_pagina(uint32_t direccion_logica);
uint32_t entrada_nivel_X(uint32_t direccion_logica, uint32_t nivel);
uint32_t desplazamiento(uint32_t direccion_logica);
uint32_t calcular_direccion_fisica(uint32_t direccion_logica, uint32_t pid);
uint32_t consultar_marco(uint32_t direccion_logica, uint32_t pid);
uint32_t consultar_marco_memoria(uint32_t* indices, uint32_t pid);
t_buffer* serializar_indices_tablas(uint32_t* indices, uint32_t pid);

typedef struct entrada_tlb {
    uint32_t pagina;
    uint32_t marco;
    t_temporal *tiempo_desde_ultimo_uso;
} entrada_tlb;

uint32_t buscar_en_tlb(uint32_t nro_pagina);
void agregar_a_tlb(entrada_tlb* entrada);

#endif