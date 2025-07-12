#include<../include/manejo_disco.h>

void dump_memory(tablas_por_pid* contenido) {
    char* contenido_dump = "";
    for(int i = 0; i < contenido->cant_marcos; i++) {
        void* contenido_marco = leer_pagina_completa(contenido->marcos[i] * memoria_cfg->TAM_PAGINA);
        string_append(&contenido_dump, (char*)contenido_marco);
    }
}