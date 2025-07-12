#ifndef CONEXION_H
#define CONEXION_H
#include <stdio.h>
#include <stdlib.h>
#include <commons/txt.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/error.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include <errno.h>

//Crea un socket y lo conecta a partir de una ip y un puerto
uint32_t crear_socket_cliente(char *ip, char* port);

//Libera la conexion asociada a un socket
void liberar_conexion(uint32_t socket_cliente);

//Espera y acepta una nueva conexion de un cliente desde un socket
uint32_t esperar_cliente(uint32_t socket_servidor);

//Recibe el codigo de operación a partir de un socket
uint8_t recibir_operacion(uint32_t socket_cliente);

//Inicia un servidor en el puerto especificado
uint32_t iniciar_servidor(char* puerto);

#endif