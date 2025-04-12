#include "config.h"

t_config* crear_config(char* nombre){
    char* nombre_completo = string_from_format("%s.config", nombre);
    t_config *config = config_create(nombre_completo);
    if(config == NULL) {
        abort();
    }
    free(nombre_completo);
    return config;
}

