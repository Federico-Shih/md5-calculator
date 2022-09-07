#ifndef APP_H_
#define APP_H_


#include "shared.h"
#include <semaphore.h>
#include <time.h>
#include <sys/select.h>

#define APPREADS 0 //el pipe que usara app para leer (recibir info de child)
#define APPWRITES 1 //el pipe que usara app para escribir (enviar info a child)

// pipefd[0] refers to the read end of the pipe.  pipefd[1] refers to the  write  end of the pipe.
#define READEND 0
#define WRITEEND 1

#define CHILD "./child"

void createChilds(int pipedes[][2][2], int childNum, int childPids[]);
void errorHandling(char* error);
void readChildsAndProcess(int childNum, int fdNum, int *filesReceived, int* filesSent, int filecount, char* filenames[], fd_set *selectfd, int pipedes[][2][2], int childPids[]);
int loadSet(int childNum, fd_set *selectfd, int pipedes[][2][2]);
void processFiles(int childNum, int pipedes[][2][2], int filecount, char * filenames[], int childPids[]);
void parseArguments(int argc, char * argv[], int * filecount, char * filenames[]);
void readUntilWhitespace(int fd, char * dest, int maxlength);

#endif