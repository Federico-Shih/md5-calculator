#ifndef SHARED_H_
#define SHARED_H_

#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <sys/wait.h>

#define MAX_RESULTS 1
#define SHARED_MEM_DIR "/shared"
#define SHARED_MEM_MAX_DIR 32

#define MAXLENGTH 128
#define HASHSIZE 32
#define MAX_PID 6 //porque el maximo pid posible es 32768

#define RESULT_FORMAT "%s  %s  %d"

typedef struct result {
  char filename[NAME_MAX];
  char hash[HASHSIZE];
  pid_t processId;
} result;

typedef struct shared_result {
  int size;
  struct result buffer[MAX_RESULTS];
  sem_t semaphore;
} shared_result;

void errorHandling(char* error);

void errorHandling(char* error){
    perror(error);
    exit(EXIT_FAILURE);
}

#endif