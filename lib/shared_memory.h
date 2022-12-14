// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef SHARED_MEMORY_H

#define SHARED_MEMORY_H
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h>    /* For O_* constants */
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#define CONNECTION_TIMEOUT 2
#define SHARED_MEM_SIZE 65536
#define SHARED_MEM_NAME "/shared"
#define SHARED_MEM_MAX_NAME 32
#define SEM_VIEW_NAME "/semview"
#define SEM_APP_NAME "/semapp"

typedef struct sharedMemCDT *sharedMemADT;

enum FREE_LEVEL { ADT = 1, UNMAP = 2, CLOSE_SEM = 4, DISCONNECT = 8 };

// Creates new sharedMemADT
sharedMemADT initSharedMem();

/* 
    Limpia ciertos lados de la memoria dependiendo de nivel de limpieza queres.
    Para limpiar multiples del mismo nivel, hacer un bitwise or de los niveles.
    Ej: ADT | UNMAP.
*/
void freeSharedMemLevel(sharedMemADT memory, int freeLevel);

// Limpia sharedMemADT
void freeSharedMem(sharedMemADT memory);

/*
    Lee hasta encontrar un \n. Si encuentra un \0 significa que la memoria compartida fue cerrada.
    Se pausa la lectura si no hay nada para leer.
    Devuelve 1 si todavia hay datos por leer, 0 en el caso contrario.
*/
int readSharedMem(sharedMemADT memory, char *buffer);

/*
    Escribe n chars a la memoria compartida. Para avisar que hay nueva linea a la lectura enviar un '\n'.
    Indica la cantidad de lineas escritas con el semaforo de escritura.
*/
int writeSharedMem(sharedMemADT memory, const char *buffer, int n);

/*
    Crea una memoria compartida. Espera por CONNECTION_TIMEOUT segundos. Retorna 1 si se conecto, retorna 0 si no o si fallo.
    Devuelve en memory el ADT. Imprime la direccion del shared memory a stdout.
*/
int startSharedMem(sharedMemADT memory, const char *mem_name);

/*   Conecta con la memoria compartida. Devuelve en memory el ADT. */
int connectSharedMem(sharedMemADT memory, const char *mem_name);

/*   Escribe \0 en la ultima linea. */
int closeSharedMem(sharedMemADT memory);

/*   Borra el nombre del shared memory del filesystem*/
void disconnectSharedMem(sharedMemADT memory);
#endif