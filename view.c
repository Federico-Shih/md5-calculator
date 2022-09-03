#include "shared.h"
#include <stdio.h>

int main(int argc, char const *argv[])
{
  int shm_fd;
  if (argc == 2) 
  {
      shm_fd = shm_open(argv[2], O_RDWR, S_IRUSR|S_IWUSR);
  } else 
  {
      char shared_mem_name[SHARED_MEM_MAX_DIR];
      size_t n;
      if (getline(&shared_mem_name, &n, stdin) == -1) {
        perror("getline\n");
      }
      shm_fd = shm_open(shared_mem_name, O_RDWR, S_IRUSR|S_IWUSR);
  }
  if (shm_fd == -1) perror("view shmopen\n");
  
  shared_results* shared_mem = mmap(NULL, sizeof(shared_results), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
  
  // Indicate to App that view is connected
  sem_post(shared_mem->semaphore);

  do {
    sem_wait(shared_mem->semaphore);
    for (int i = 0; i < shared_mem->size; i += 1) {
      printf("FILENAME=%s HASH=%s PID=%d\n", shared_mem->buffer[i].filename, shared_mem->buffer[i].hash, shared_mem->buffer[i].process);
    }
  } while (shared_mem->size != 0);

  munmap(shared_mem, sizeof(shared_results));
  close(shm_fd);
  exit(0);
}
