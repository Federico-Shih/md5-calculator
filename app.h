#ifndef APP_H_
#define APP_H_

#define _POSIX_C_SOURCE 200809L

#include <sys/select.h>
#include <limits.h>

#include "lib/shared.h"
#include "lib/shared_memory.h"

#define APPREADS 0 //el pipe que usara app para leer (recibir info de child)
#define APPWRITES 1 //el pipe que usara app para escribir (enviar info a child)

// pipefd[0] refiere a la punta de lectura del pipe.  pipefd[1] refiere a la punta de escritura.
#define READEND 0
#define WRITEEND 1

#define CHILD "./child"

void createChilds(int pipedes[][2][2], int childNum, int childPids[]);
void errorHandling(char* error);
void readChildsAndProcess(int childNum, int fdNum, int *filesReceived, int* filesSent, int filecount, char* filenames[], fd_set *selectfd, int pipedes[][2][2], int childPids[], int fd, sharedMemADT memory);
int loadSet(int childNum, fd_set *selectfd, int pipedes[][2][2]);
void processFiles(int childNum, int pipedes[][2][2], int filecount, char * filenames[], int childPids[], int fd, sharedMemADT memory);
void parseArguments(int argc, char * argv[], int * filecount, char * filenames[]);
void readFromMD5(int fd, char * hash, int maxHash, char * filename, int maxFilename);

#endif