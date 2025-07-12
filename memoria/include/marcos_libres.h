#ifndef MARCOS_LIBRES_H
#define MARCOS_LIBRES_H
#include <commons/collections/list.h>
#include <stdint.h>
#include "common.h"

void inicializar_bitmap(int cantidad_frames);
bool hay_marcos_libres();

void liberar_marco(int marco);
void destruir_bitmap();
#endif