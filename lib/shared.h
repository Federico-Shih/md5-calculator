// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef SHARED_H_
#define SHARED_H_

#define _GNU_SOURCE

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
#define SHARED_MEM_MAX_NAME 32

#define MAXLENGTH 128
#define HASHSIZE 32
#define MAX_PID 6 //porque el maximo pid posible es 32768

#define LINE_FORMAT "Filename:%s, PID:%d, Hash:%s\n"

#define MAXLINE (MAXLENGTH + HASHSIZE + MAX_PID + 24)

typedef struct result {
  char filename[MAXLENGTH];
  char hash[HASHSIZE+1];
  pid_t processId;
} result;


void errorHandling(char* error);


#endif