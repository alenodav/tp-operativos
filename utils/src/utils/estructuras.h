#include<commons/collections/list.h>
#include<stdio.h>
#include<stdlib.h>

typedef struct t_io{
    char* id;
    bool estado;
    uint32 proceso_en_ejecucion;
    //pcb* cola_procesos 
} t_io;

typedef struct t_cpu{
    char* id;
    bool estado;
} t_cpu;

//hay otra estructura para cpu que no se cual es la correcta, si la de arriba o esta de abajo:

/* typedef struct t_cpu{
    uint32 pid;
    uint32 pc;
} t_cpu; */

typedef struct t_memoria{
    uint32 pid;
    t_list* lista_instrucciones;
    uint32 size_lista_instrucciones;
} t_memoria;