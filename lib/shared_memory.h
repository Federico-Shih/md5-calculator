#ifndef SHARED_MEMORY_H

#define SHARED_MEMORY_H
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#define CONNECTION_TIMEOUT 2
#define SHARED_MEM_SIZE 2048
#define SHARED_MEM_NAME "/shared"
#define SHARED_MEM_MAX_NAME 32
#define SHARED_SEM_NAME "/semview"
#define SHARED_SEM_NAME_CONNECTION "/semapp"

typedef struct sharedMemCDT * sharedMemADT;                                    

// Creates new sharedMemADT
sharedMemADT initSharedMem();

// Frees sharedMemADT
void freeSharedMem(sharedMemADT memory);

/*    
    Lee hasta encontrar un \n. Si encuentra un \0 significa que la memoria compartida fue cerrada.    
    Se pausa la lectura si no hay nada para leer. 
    Devuelve 1 si todavia hay datos por leer, 0 en el caso contrario. 
*/
int readSharedMem(sharedMemADT memory, char *buffer);
         
/*   
    Escribe n chars a la memoria compartida. Para terminar la lectura enviar un '\n'.
    Indica la cantidad de lineas escritas con el semaforo de escritura.   
*/
void writeSharedMem(sharedMemADT memory, const char *buffer, int n); 

/*   
    Crea una memoria compartida. Espera por CONNECTION_TIMEOUT segundos. Retorna 1 si se conecto, retorna 0 si no.
    Devuelve en memory el ADT
*/
int startSharedMem(sharedMemADT memory, const char* mem_name);

/*   Conecta con la memoria compartida. Devuelve en memory el ADT */
int connectSharedMem(sharedMemADT memory, const char* mem_name);                    

/*   Limpia la memoria compartida. Escribe \0 en la ultima linea. */
int closeSharedMem(sharedMemADT memory);                      

/*   Limpia los recursos alocados por la memoria compartida */
void disconnectSharedMem(sharedMemADT memory);
#endif