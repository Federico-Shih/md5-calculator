// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "shared_memory.h"
#include "shared.h"

// AGREGAR ERROR HANDLING
struct sharedMemCDT
{
    char sharedMemName[SHARED_MEM_MAX_NAME];
    char *sharedMem;
    sem_t *viewSemaphore;
    int index;
};

int readSharedMem(sharedMemADT memory, char *buffer) {
    // Wait for data to be available
    sem_wait(memory->viewSemaphore);

    // i for writing buffer, end to terminate loop, no_lines to tell reader
    int i, no_lines = 0;
    for (i = 0; i < MAXLINE; i += 1) {
        char c = memory->sharedMem[memory->index + i];
        if (c == '\n' || c == '\0') {
            if (c == '\0') no_lines = 1;
            break;
        } else {
            buffer[i] = c;
        }
    }
    memory->index += i + 1;
    buffer[i] = '\0';

    return !no_lines;
}

// Consider prevent writing if child was not connected
void writeSharedMem(sharedMemADT memory, const char *buffer, int n) {
    for (int i = 0; i < n; i += 1) {
        memory->sharedMem[memory->index + i] = buffer[i];
        if (buffer[i] == '\n' || buffer[i] == '\0') {
            sem_post(memory->viewSemaphore);
        }
    }
    memory->index += n;
}

int startSharedMem(sharedMemADT memory, const char *mem_name) {
    int shmFd;
    if ((shmFd = shm_open(mem_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
        errorHandling("shm_open");
    }

    if (ftruncate(shmFd, SHARED_MEM_SIZE) == -1){
        errorHandling("ftruncate");
    }

    memory->sharedMem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    strncpy(memory->sharedMemName, mem_name, SHARED_MEM_MAX_NAME);
    memory->index = 0;
    close(shmFd);

    // Asigna el semaforo al ADT que el padre usa para comunicar que hay para leer.
    memory->viewSemaphore = sem_open(SHARED_SEM_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0);

    // Abre un semaforo temporal para que view pueda comunicar al padre que se conecto
    sem_t *appSem = sem_open(SHARED_SEM_NAME_CONNECTION, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 1);

    struct timespec time;
    if (clock_gettime(CLOCK_REALTIME, &time) == -1)
        errorHandling("clock_gettime");

    time.tv_sec += CONNECTION_TIMEOUT;
    
    int initialized = sem_timedwait(appSem, &time);

    if(initialized == -1) {
        disconnectSharedMem(memory);
        if (errno != ETIMEDOUT) //si hubo un error ejecutando sem_timedwait
            perror("sem_timedwait");
        return 0;
    }

    return 1;
}

int connectSharedMem(sharedMemADT memory, const char * mem_name) {
    int shmFd;
    if ((shmFd = shm_open(mem_name, O_RDWR, S_IRUSR | S_IWUSR)) == -1){
        errorHandling("connectSHM");
    }

    memory->sharedMem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    memory->index = 0;
    close(shmFd);

    sem_t *appSem = sem_open(SHARED_SEM_NAME_CONNECTION, O_RDWR);
    if(appSem == SEM_FAILED){
        disconnectSharedMem(memory);
        freeSharedMem(memory);
        errorHandling("sem_open");
    }
    
    // Avisa al padre que se conecto.
    if(sem_post(appSem) == -1){
        disconnectSharedMem(memory);
        freeSharedMem(memory);
        errorHandling("sem_post");
    }

    // Abre el semaforo para que se puedan conectar
    sem_t * address = sem_open(SHARED_SEM_NAME, O_RDWR);
    if(address == SEM_FAILED){
        disconnectSharedMem(memory);
        freeSharedMem(memory);
        errorHandling("sem_open");
    }
    
    memory->viewSemaphore = address;

    return 1;
}

void disconnectSharedMem(sharedMemADT memory) {
    shm_unlink(memory->sharedMemName); 
}  

int closeSharedMem(sharedMemADT memory) {
    writeSharedMem(memory, "\0", 1);
    return 1;
}

sharedMemADT initSharedMem() {
    sharedMemADT ret = malloc(sizeof(struct sharedMemCDT));
    ret->index = 0;
    return ret;
}

void freeSharedMem(sharedMemADT memory) {
    munmap(memory->sharedMem, SHARED_MEM_SIZE);
    sem_close(memory->viewSemaphore);
    free(memory);
}