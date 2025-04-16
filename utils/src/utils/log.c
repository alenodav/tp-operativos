#include "log.h"

t_log* crear_log(t_config* config, char* nombre){
    t_log_level log_level = log_level_from_string(config_get_string_value(config, "LOG_LEVEL"));
    if (log_level == -1) {
        abort();
    }
    char* nombre_string = string_duplicate(nombre);
    char* nombre_completo = string_from_format("%s.log", nombre);
    string_to_upper(nombre_string);

    t_log *log = log_create(nombre_completo, nombre_string, true, log_level);
    if (log == NULL) {
        abort();
    }
    free(nombre_completo);
    free(nombre_string);
    return log;
}