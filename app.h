// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef APP_H_
#define APP_H_

#define _POSIX_C_SOURCE 200809L

#include <sys/select.h>
#include <limits.h>
#include <errno.h>

#include "lib/shared.h"
#include "lib/shared_memory.h"

#define LINE_FORMAT "Filename:%s, PID:%d, Hash:%s\n"

#define APPREADS 0  // el pipe que usara app para leer (recibir info de child)
#define APPWRITES 1 // el pipe que usara app para escribir (enviar info a child)

// pipefd[0] refiere a la punta de lectura del pipe.  pipefd[1] refiere a la punta de escritura.
#define READEND 0
#define WRITEEND 1

#define CHILD "./child"

#define FILES_PER_CHILD 20
#define MIN_CHILD 3

typedef struct result
{
  char filename[MAX_FILENAME];
  char hash[HASHSIZE + 1];
  pid_t processId;
} result;

void createChilds(int pipedes[][2][2], int childNum, int childPids[]);
void errorHandling(char *error);
int loadSet(int childNum, fd_set *selectfd, int pipedes[][2][2]);
void processFiles(int childNum, int pipedes[][2][2], int filecount, char *filenames[], int childPids[], int fd, sharedMemADT memory);
int parseArguments(int argc, char *argv[], int *filecount, char *filenames[]);
void readFromMD5(int fd, char *hash, char *filename);
void readAndProcess(int readFd, int childPid, int destFd, sharedMemADT destMemory);

#endif