#include "../include/manejo_memoria.h"
#include "../include/main.h"

void* crear_espacio_de_memoria(){
    void* mp = malloc(memoria_cfg->TAM_MEMORIA);
    memset(mp,0,memoria_cfg->TAM_MEMORIA);      //seteo el bloque de memoria a 0.
    return mp;
}

void* leer_de_memoria(uint32_t direccion_fisica, uint32_t size, uint32_t pid){
    log_info(logger, "## PID: %d - Lectura - Dir. Física: %d - Tamaño: %d", pid, direccion_fisica, size);
    void* contenido = malloc(size); 
    sem_wait(&mutex_memoria);
    memcpy(contenido, memoria_principal->datos + direccion_fisica, size);
    sem_post(&mutex_memoria);

    return contenido;   
}

bool escribir_en_memoria(uint32_t direccion_fisica, uint32_t size, void* contenido, uint32_t pid) {//direccion fisica dentro del bloque de memoria
    log_info(logger, "## PID: %d - Escritura - Dir. Física: %d - Tamaño: %d", pid, direccion_fisica, size);
    sem_wait(&mutex_memoria);
    memcpy(memoria_principal->datos + direccion_fisica, contenido, size); //copia el contenido a la memoria
    sem_post(&mutex_memoria);
    return true;
}


