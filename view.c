// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "lib/shared.h"
#include "lib/shared_memory.h"
#include <stdio.h>

int main(int argc, char const *argv[]) {
  char mem_dir[SHARED_MEM_MAX_NAME];
  if (argc == 2) {
    strcpy(mem_dir, argv[1]);
  } else {
      char buf;
      int readres;
      int i = 0;
      for(; (readres = read(STDIN_FILENO, &buf, 1)) > 0 && buf != '\n' && buf != '\0' && buf != ' ' && i < SHARED_MEM_MAX_NAME-1; i++)
        mem_dir[i] = buf;

      mem_dir[i] = '\0';
  }
  sharedMemADT memory = initSharedMem();
  connectSharedMem(memory, mem_dir);

  sleep(1);
  // do {
  //   sem_wait(&(shared_mem->semaphore));
  //   for (int i = 0; i < shared_mem->size; i += 1) {
  //     printf("FILENAME=%s HASH=%s PID=%d\n", shared_mem->buffer[i].filename, shared_mem->buffer[i].hash, shared_mem->buffer[i].processId);
  //   }
  // } while (shared_mem->size != 0);

  // munmap(shared_mem, sizeof(struct shared_result));
  disconnectSharedMem(memory);
  freeSharedMem(memory);
  exit(0);
}
