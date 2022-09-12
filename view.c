// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "lib/shared.h"
#include "lib/shared_memory.h"


int main(int argc, char const *argv[])
{
  char *mem_dir;
  if (argc == 2)
  {
    mem_dir = strdup(argv[1]); // warning de pvs, cambio de strcpy a strdup por posible error de buffer overflow.
  }
  else
  {
    mem_dir = malloc((SHARED_MEM_MAX_NAME + 1) * sizeof(char));
    scanf("%32s", mem_dir);
  }
  
  sharedMemADT memory = initSharedMem();
  int connected = connectSharedMem(memory, mem_dir);
  if (connected) {
    free(mem_dir); // free de malloc o free de strdup, depende en cual entre (strdup hace un malloc)

    char buffer[MAXLINE];

    while (readSharedMem(memory, buffer))
    {
      printf("%s\n", buffer);
    }

    freeSharedMem(memory);
  }
  exit(!connected);
}
