#include "shared.h"
#include <stdio.h>

int main(int argc, char const *argv[])
{
  int shm_fd;
  if (argc == 2) 
  {
      shm_fd = shm_open(argv[1], O_RDWR, S_IRUSR|S_IWUSR);
  } else 
  {
      char shared_mem_name[SHARED_MEM_MAX_DIR];
      int i = 0;
      while (read(STDIN_FILENO, &(shared_mem_name[i]), 1) != -1) {
        if (shared_mem_name[i] == '\n' || shared_mem_name[i] == '\0' || shared_mem_name[i] == ' ') {
          break;
        } 
        i += 1;
      }
      shared_mem_name[i] = '\0';
      printf("WHY\n");
      printf("%s", shared_mem_name);
      shm_fd = shm_open(shared_mem_name, O_RDWR, S_IRUSR|S_IWUSR);
  }
  if (shm_fd == -1) perror("view shmopen\n");
  
  shared_result* shared_mem = mmap(NULL, sizeof(struct shared_result), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);

  // Indicate to App that view is connected
  sem_post(&(shared_mem->semaphore));

  // Let app time to wait the semaphore.
  sleep(1);
  do {
    sem_wait(&(shared_mem->semaphore));
    for (int i = 0; i < shared_mem->size; i += 1) {
      printf("FILENAME=%s HASH=%s PID=%d\n", shared_mem->buffer[i].filename, shared_mem->buffer[i].hash, shared_mem->buffer[i].process);
    }
  } while (shared_mem->size != 0);

  munmap(shared_mem, sizeof(struct shared_result));
  close(shm_fd);
  exit(0);
}
