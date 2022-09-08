#include "shared_memory.h"
#include "shared.h"

// AGREGAR ERROR HANDLING

struct shared_pipe
{
    char *sharedMem;
    sem_t *viewSemaphore;
    int index;
};

int readSharedPipe(shared_pipe memory, char *buffer);

void writeSharedPipe(shared_pipe memory, const char *buffer, int n)
{
}

int startSharedPipe(shared_pipe memory, const char *mem_address)
{
    int shmFd;
    if ((shmFd = shm_open(mem_address, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) == -1)
    {
        errorHandling("shm_open");
    }

    if (ftruncate(shmFd, BUFFER_SIZE) == -1)
    {
        errorHandling("ftruncate");
    }

    memory->sharedMem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);

    close(shmFd);

    // Asigna el semaforo al ADT que el padre usa para comunicar que hay para leer.
    memory->viewSemaphore = sem_open(SHARED_SEM_DIR, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0);

    // Abre un semaforo temporal para que view pueda comunicar al padre que se conecto
    sem_t *appSem = sem_open(SHARED_SEM_DIR_CONNECTION, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 1);

    struct timespec time;
    if (clock_gettime(CLOCK_REALTIME, &time) == -1)
        return -1;

    time.tv_sec += CONNECTION_TIMEOUT;
    int initialized = sem_timedwait(appSem, &time);

    sem_close(appSem);

    if (initialized == -1)
    {
        cleanSharedPipe(memory);
    }

    return (initialized != -1);
}

int connectSharedPipe(shared_pipe memory, const char *mem_address)
{
    int shmFd;
    if ((shmFd = shm_open(mem_address, O_RDWR, S_IRUSR | S_IWUSR)) == -1)
    {
        errorHandling("connectSHM");
    }

    memory->sharedMem = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    close(shmFd);

    sem_t *appSem = sem_open(SHARED_SEM_DIR_CONNECTION, O_RDWR);

    // Avisa al padre que se conecto.
    sem_post(appSem);

    // Abre el semaforo para que se puedan conectar
    memory->viewSemaphore = sem_open(SHARED_SEM_DIR, O_RDWR);

    return 1;
}

void cleanSharedPipe(shared_pipe memory)
{
    munmap(memory->sharedMem, sizeof(struct shared_pipe));
    sem_close(memory->viewSemaphore);
}

int closeSharedPipe(shared_pipe memory)
{
    writeSharedPipe(memory, "\0", 1);
    cleanSharedPipe(memory);
    return 1;
}