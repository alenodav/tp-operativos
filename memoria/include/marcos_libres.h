#ifndef MARCOS_LIBRES_H_INCLUDED
#define MARCOS_LIBRES_H_INCLUDED
#include <commons/bitarray.h>

typedef struct{
    uint32_t nroMarco;
    bool estaLibre;
} t_marco_libre;

t_list* marcosLibres;

bucar_marcos_libres();
liberar_marcos_libres();
inicializar_marcos();
crear_marcos();

//¿Cuantos marcos voy a tener?
// TAM_MEMORIA/TAM_PAGINA; 

#endif