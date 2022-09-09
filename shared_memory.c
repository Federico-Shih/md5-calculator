// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "shared_memory.h"
#include "shared.h"

// AGREGAR ERROR HANDLING

# define CLOCK_REALTIME			0
extern int clock_gettime (clockid_t __clock_id, struct timespec *__tp) __THROW;



int readSharedMem(shared_mem memory, char *buffer) {
    return 0;
}

void writeSharedMem(shared_mem memory, const char *buffer, int n) {

    memcpy(memory->sharedMem, buffer, n);
}

int startSharedMem(shared_mem memory, const char *mem_name) {
    int shmFd;
    if ((shmFd = shm_open(mem_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
        errorHandling("shm_open");
    }

    if (ftruncate(shmFd, SHARED_MEM_SIZE) == -1){
        errorHandling("ftruncate");
    }

    memory->sharedMem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    strncpy(memory->sharedMemName, mem_name, SHARED_MEM_MAX_NAME);
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

    if (sem_close(appSem) != -1)
        errorHandling("sem_close");

    if(initialized == 0)
        return 0;

    closeSharedMem(memory);

    if (errno != ETIMEDOUT) //si hubo un error ejecutando sem_timedwait
        perror("sem_timedwait");

    return -1;
}

int connectSharedMem(shared_mem memory, const char * mem_name) {
    int shmFd;
    if ((shmFd = shm_open(mem_name, O_RDWR, S_IRUSR | S_IWUSR)) == -1){
        errorHandling("connectSHM");
    }

    memory->sharedMem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    close(shmFd);

    sem_t *appSem = sem_open(SHARED_SEM_NAME_CONNECTION, O_RDWR);
    if(appSem == SEM_FAILED){
        closeSharedMem(memory);
        errorHandling("sem_open");
    }

    // Avisa al padre que se conecto.
    if(sem_post(appSem)!= -1){
        closeSharedMem(memory);
        errorHandling("sem_post");
    }

    // Abre el semaforo para que se puedan conectar
    sem_t * address = sem_open(SHARED_SEM_NAME, O_RDWR);
    if(address == SEM_FAILED){
        closeSharedMem(memory);
        errorHandling("sem_open");
    }
    
    memory->viewSemaphore = address;

    return 1;
}

void cleanSharedMem(shared_mem memory) {
    munmap(memory->sharedMem, sizeof(struct shared_mem));
    sem_close(memory->viewSemaphore);
    shm_unlink(memory->sharedMemName); 
}  

int closeSharedMem(shared_mem memory) {
    writeSharedMem(memory, "\0", 1);
    cleanSharedMem(memory);
    return 1;
}