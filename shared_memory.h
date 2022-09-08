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
#define BUFFER_SIZE 512
#define SHARED_MEM_DIR "/shared"
#define SHARED_MEM_MAX_DIR 32
#define SHARED_SEM_DIR "/semview"
#define SHARED_SEM_DIR_CONNECTION "/semapp"


typedef struct shared_pipe *shared_pipe;                                            

/*    
    Reads until it encounters a '\n' character. If it encounters an '\0' character, it means that the pipe was closed.    
    Pauses read if there isn't anything to read. Indicates with write semaphore amount of lines read.
    Returns 1 if there is still data to read, returns 0 when there isn't. 
*/
int readSharedPipe(shared_pipe memory, char *buffer);
         
/*   
    Writes n chars to shared memory. To signal data to read end of the pipe, send a \n character.   
    Pauses write if read hasn't finished. Indicates with read semaphore amount of lines written.
*/
void writeSharedPipe(shared_pipe memory, const char *buffer, int n); 

/*   Creates a shared pipe. Waits for 2 seconds. Returns 1 if connected, return 0 if not.  */
int startSharedPipe(shared_pipe memory, const char* mem_address);

/*   Connects to shared pipe.  */
int connectSharedPipe(shared_pipe memory, const char* mem_address);                    

/*   Cleans a shared pipe. Writes \0 at last line. */
int closeSharedPipe(shared_pipe memory);                      

/*   Cleans shared_pipe allocated resources */
void cleanSharedPipe(shared_pipe memory);
#endif