#include "mmu.h"
#include <utils/config.h>

t_config* config_cpu = crear_config("cpu");

uint32_t numero_pagina(uint32_t direccion_logica, uint32_t tamanio_pagina){
    return (u_int32_t)floor(direccion_logica / tamanio_pagina);
}

uint32_t entrada_nivel_X(uint32_t direccion_logica, uint32_t numero_pagina, uint32_t max_nivel, uint32_t nivel){
    uint32_t cant_entradas_tabla = config_get_int_value(config_cpu, "ENTRADAS_TLB");
    return (u_int32_t)(floor(numero_pagina / cant_entradas_tabla ^ (max_nivel-nivel))%cant_entradas_tabla);
}

uint32_t desplazamiento(uint32_t direccion_logica, uint32_t tamanio_pagina){
    return direccion_logica % tamanio_pagina;
}