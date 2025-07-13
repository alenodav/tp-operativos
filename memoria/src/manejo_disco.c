#include<../include/manejo_disco.h>

void dump_memory(tablas_por_pid* contenido) {
    char* contenido_dump = "";
    for(int i = 0; i < contenido->cant_marcos; i++) {
        void* contenido_marco = leer_pagina_completa(contenido->marcos[i] * memoria_cfg->TAM_PAGINA);
        string_append(&contenido_dump, (char*)contenido_marco);
    }
    char * path_archivo = string_duplicate(memoria_cfg->DUMP_PATH);
    string_append_with_format(&path_archivo, "%d-%s.dmp", contenido->pid, temporal_get_string_time("%H:%M:%S:%MS"));
    FILE* archivo_dump = txt_open_for_append(path_archivo);
    txt_write_in_file(archivo_dump, contenido_dump);
    txt_close_file(archivo_dump);
    free(contenido_dump);
    free(path_archivo);
}

void suspender_proceso(tablas_por_pid* contenido) {
    char* contenido_a_swap = "";
    for(int i = 0; i < contenido->cant_marcos; i++) {
        void* contenido_marco = leer_pagina_completa(contenido->marcos[i] * memoria_cfg->TAM_PAGINA);
        string_append(&contenido_a_swap, (char*)contenido_marco);
        void* pagina_vacia = calloc(memoria_cfg->TAM_PAGINA, sizeof(void*));
        actualizar_pagina_completa(contenido->marcos[i] * memoria_cfg->TAM_PAGINA, pagina_vacia);
        liberar_marco(contenido->marcos[i]);
        free(pagina_vacia);
    }
    free(contenido->marcos);
    contenido->marcos = NULL;
    FILE* swapfile = txt_open_for_append(memoria_cfg->PATH_SWAPFILE);
    char* pid = string_itoa(contenido->pid);
    string_append(&contenido_a_swap, "\n");
    string_append(&pid, "\n");
    txt_write_in_file(swapfile, pid);
    txt_write_in_file(swapfile, contenido_a_swap);
    txt_close_file(swapfile);
    free(contenido_a_swap);
    free(pid);
}

void dessuspender_procesos (tablas_por_pid* contenido, uint32_t tamanio_proceso) {
    char* swapfile_tmp_path = memoria_cfg->PATH_SWAPFILE;
    string_append(&swapfile_tmp_path, ".tmp");
    FILE* swapfile_tmp = txt_open_for_append(swapfile_tmp_path);
    FILE* swapfile = fopen(memoria_cfg->PATH_SWAPFILE, "r");
    char* pid_s = string_itoa(contenido->pid);
    char* linea = malloc(memoria_cfg->TAM_MEMORIA);
    bool encontrado = false;

    while (fgets(linea, memoria_cfg->TAM_MEMORIA, swapfile)) {
        linea[string_length(linea) - 1] = '\0';

        if (string_equals_ignore_case(linea, pid_s)) {
            encontrado = true;
        }

    }

    fclose(swapfile);
}

