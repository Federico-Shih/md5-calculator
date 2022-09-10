// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "lib/shared.h"
#include "lib/shared_memory.h"
#include <stdio.h>

int main(int argc, char const *argv[]) {
  char* mem_dir;
  if (argc == 2) {
    mem_dir = strdup(argv[1]);  // warning de pvs, cambio de strcpy a strdup por posible error de buffer overflow.
    
  } else {
      
      char buf;
      int i = 0;
      mem_dir = malloc(SHARED_MEM_MAX_NAME);
      if(mem_dir == NULL){
        errorHandling("Could not retrieve shared memory name");      // warning de pvs, cambio de string creado fijo por malloc
      }
      else{
        for(int i = 0; ( read(STDIN_FILENO, &buf, 1) > 0) && buf != '\n' && buf != '\0' && buf != ' ' && i < SHARED_MEM_MAX_NAME-1; i++)
          mem_dir[i] = buf;

        mem_dir[i] = '\0';
      }
  }
  sharedMemADT memory = initSharedMem();
  connectSharedMem(memory, mem_dir);
  free(mem_dir);            // free de malloc o free de strdup, depende en cual entre (strdup hace un malloc)

  char buffer[MAXLINE];

  while (readSharedMem(memory, buffer)) {
    printf("%s\n", buffer);
  }
                                           
  disconnectSharedMem(memory);
  freeSharedMem(memory);
  exit(0);
}
