#ifndef SHARED_MEMORY_H

#define SHARED_MEMORY_H

#include <semaphore.h>
#define BUFFER_SIZE 512
#define SHARED_MEM_DIR "/shared"
#define SHARED_MEM_MAX_DIR 32
#define SHARED_SEM_DIR "/semview"
#define SHARED_SEM_DIR_CONNECTION "/semapp"

typedef struct shared_result
{
    int index;
    char buffer[BUFFER_SIZE];
} shared_result;

typedef struct shared_pipe
{
    struct shared_result sharedMem;

} shared_pipe;                                                 

/*    
    Reads until it encounters a '\n' character. If it encounters an '\0' character, it means that the pipe was closed.    
    Pauses read if there isn't anything to read. Indicates with write semaphore amount of lines read.
    Returns 1 if there is still data to read, returns 0 when there isn't. 
*/
int readSharedPipe(shared_pipe *memory, char *buffer);
         
/*   
    Writes n chars to shared memory. To signal data to read end of the pipe, send a '\n' character.    
    Pauses write if read hasn't finished. Indicates with read semaphore amount of lines written.
*/
void writeSharedPipe(shared_pipe *memory, const char *buffer, int n); 

/*   Creates a shared pipe. Waits for 2 seconds. Returns 1 if connected, return 0 if not.  */
int startSharedPipe(shared_pipe *memory);  

/*   Connects to shared pipe.  */
int connectSharedPipe(shared_pipe *memory);                    

/*   Closes and unmaps a shared pipe. Writes \0 at last line. */
int closeSharedPipe(shared_pipe *memory);                      

/*   Unmaps shared pipe */
int unmapSharedPipe(shared_pipe *memory);
#endif