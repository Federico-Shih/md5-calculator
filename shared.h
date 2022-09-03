#ifndef SHARED_H_
#define SHARED_H_

#define _GNU_SOURCE
#define _BSD_SOURCE

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

#define MAX_RESULTS 30
#define SHARED_MEM_DIR "/shared"
#define SHARED_MEM_MAX_DIR 32

typedef struct {
  char filename[FILENAME_MAX];
  char hash[16];
  pid_t process;
} result;

typedef struct {
  int size;
  result buffer[MAX_RESULTS];
  sem_t* semaphore;
} shared_results;

#endif