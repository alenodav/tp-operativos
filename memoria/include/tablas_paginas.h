#ifndef TABLAS_PAGINAS.H
#define TABLAS_PAGINAS.H

#include<commons/collections/list.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

//Tabla de paginas.
typedef struct{
    uint32_t nivel;     //nivel(1...n)
    t_list* entradas;   //lista de struct entrada_pagina
    uint32_t pid;       //proceso al que le pertenece esa tabla de paginas.
} tabla_paginas;

//Entrada de una tabla de paginas.
typedef struct{
    t_marco marco;                   //solo valido si es entrada de nivel final.
    tabla_paginas* tabla_siguiente;  //puntero a la siguiente tabla, solo valido si es entrada de nivel intermedio.
    bool es_final;                   //indica si es tabla final, si TRUE, se usa marco; si FALSE, se usa tabla_siguiente.         
} entrada_pagina;

//Estructura de marco en memoria
typedef struct{
    uint32_t numero_de_marco;
    bool estado;    //1 libre, 0 ocupado.
    bool modificado; //1 si el marco fue modificado, 0 si no.
} t_marco;

void inicializar_tabla_de_paginas();
void crear_tabla_de_paginas();
void crear_entradas();


#endif