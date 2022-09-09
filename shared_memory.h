#ifndef SHARED_MEMORY_H

#define SHARED_MEMORY_H

#define _BSD_SOURCE

#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#define CONNECTION_TIMEOUT 10
#define SHARED_MEM_SIZE 512
#define SHARED_MEM_NAME "/shared"
#define SHARED_MEM_MAX_NAME 32
#define SHARED_SEM_NAME "/semview"
#define SHARED_SEM_NAME_CONNECTION "/semapp"

struct shared_mem
{
    char sharedMemName[SHARED_MEM_MAX_NAME];
    char *sharedMem;
    sem_t *viewSemaphore;
    int index;
};

typedef struct shared_mem * shared_mem;                                    

/*    
    Lee hasta encontrar un '\n'. Si encuentra un '\0' significa que la memoria compartida fue cerrada.    
    Se pausa la lectura si no hay nada para leer. Indica la cantidad de lineas leidas con el semaforo de escritura.
    Devuelve 1 si todavia hay datos por leer, 0 en el caso contrario. 
*/
int readSharedMem(shared_mem memory, char *buffer);
         
/*   
    Escribe n chars a la memoria compartida. Para terminar la lectura enviar un '\n'.
    Se pausa la escritura si la lectura no ha terminado. Indica la cantidad de lineas escritas con el semaforo de escritura.   
*/
void writeSharedMem(shared_mem memory, const char *buffer, int n); 

/*   Crea una memoria compartida. Espera por CONNECTION_TIMEOUT segundos. Retorna 1 si se conecto, retorna 0 si no.  */
int startSharedMem(shared_mem memory, const char* mem_name);

/*   Conecta con la memoria compartida.  */
int connectSharedMem(shared_mem memory, const char* mem_name);                    

/*   Limpia la memoria compartida. Escribe \0 en la ultima linea. */
int closeSharedMem(shared_mem memory);                      

/*   Limpia los recursos alocados por la memoria compartida */
void cleanSharedMem(shared_mem memory);
#endif