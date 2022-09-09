// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "shared.h"
#include <stdio.h>

int main(int argc, char const *argv[]) {
  int shm_fd;
  if (argc == 2) {
    shm_fd = shm_open(argv[1], O_RDWR, S_IRUSR|S_IWUSR);
    
  } else {
      char shared_mem_name[SHARED_MEM_MAX_NAME];
      char buf;
      int readres;
      int i = 0;
      for(; (readres = read(STDIN_FILENO, &buf, 1)) > 0 && buf != '\n' && buf != '\0' && buf != ' ' && i < SHARED_MEM_MAX_NAME-1; i++)
        shared_mem_name[i] = buf;

      shared_mem_name[i] = '\0';
      printf("%s", shared_mem_name);
      shm_fd = shm_open(shared_mem_name, O_RDWR, S_IRUSR|S_IWUSR);
  }
  if (shm_fd == -1) {
    perror("shm_open");
  }
    
  
  // shared_result* shared_mem = mmap(NULL, sizeof(struct shared_result), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);

  // Indica a App que la vista esta conectadad
  // sem_post(&(shared_mem->semaphore));

  // Deja tiempo a app para que espere el semaforo.
  sleep(1);
  // do {
  //   sem_wait(&(shared_mem->semaphore));
  //   for (int i = 0; i < shared_mem->size; i += 1) {
  //     printf("FILENAME=%s HASH=%s PID=%d\n", shared_mem->buffer[i].filename, shared_mem->buffer[i].hash, shared_mem->buffer[i].processId);
  //   }
  // } while (shared_mem->size != 0);

  // munmap(shared_mem, sizeof(struct shared_result));
  close(shm_fd);
  exit(0);
}
