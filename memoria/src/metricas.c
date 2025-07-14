#include <../include/metricas.h>
#include <stdio.h>
#include <stdlib.h>

t_list* metricas_por_procesos;


void mostrar_metricas(uint32_t pid){
    for (int i = 0; i < list_size(metricas_por_procesos); i++) {
        t_metricas* metricas = list_get(metricas_por_procesos, i);
        if (metricas->pid == pid) {
            log_info(logger, "##PID: <%i> - Proceso Destruido - Metricas - Acc.T.Pag: <%i> - Inst.Sol: <%i> - SWAP: <%i> - Mem.Prin: <%i> - Lec.Mem <%i> - Esc.Mem <%i>", 
                        metricas->pid, metricas->accesos_tablas_paginas, metricas->instrucciones_solicitadas,
                        metricas->bajadas_a_swap, metricas->subidas_a_mp, metricas->cantidad_lecturas,
                        metricas->cantidad_escrituras);      
            break;   
        }
    
    }
}