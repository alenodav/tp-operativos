#include "../include/manejo_memoria.h"
#include "../include/main.h"

void* crear_espacio_de_memoria(){
    void* mp = malloc(memoria_cfg->TAM_MEMORIA);
    memset(mp,0,memoria_cfg->TAM_MEMORIA);      //seteo el bloque de memoria a 0.
    return mp;
}

void* leer_de_memoria(void* memoria, uint32_t marco, uint32_t desplazamiento, uint32_t size){
    void* contenido = malloc(size); 
    memcpy(contenido, memoria + (marco*memoria_cfg->TAM_PAGINA) + desplazamiento);

    return contenido;   
}

void* escribir_en_memoria(void* memoria, void* contenido, uint32_t marco, uint32_t desplazamiento, uint32_t size){
    uint32_t offset = (marco * memoria_cfg->TAM_PAGINA) + desplazamiento; //direccion fisica dentro del bloque de memoria
    memcpy(memoria + offset, contenido, size); //copia el contenido a la memoria
}


