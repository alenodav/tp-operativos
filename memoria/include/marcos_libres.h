#ifndef MARCOS_LIBRES_H_INCLUDED
#define MARCOS_LIBRES_H_INCLUDED
#include <commons/collections/list.h>
#include <stdint.h>

void inicializar_bitmap(int cantidad_frames);
bool hay_marcos_libres();
int obtener_marco_libre();
void liberar_marco(int marco);
void destruir_bitmap();
#endif