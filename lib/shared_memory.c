// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "shared_memory.h"
#include "shared.h"


struct sharedMemCDT
{
    char sharedMemName[SHARED_MEM_MAX_NAME];
    char *sharedMem;
    sem_t *viewSemaphore;
    int index;
};

int readSharedMem(sharedMemADT memory, char *buffer)
{
    // Espera a que hayan datos disponibles
    sem_wait(memory->viewSemaphore);

    // i para escribir el buffer, no_lines indica a la funcion lectora(readSharedMem) cuando dejar de leer
    int i, no_lines = 0;
    for (i = 0; i < MAXLINE; i += 1)
    {
        char c = memory->sharedMem[memory->index + i];
        if (c == '\n' || c == '\0')
        {
            if (c == '\0')
                no_lines = 1;
            break;
        }
        else
        {
            buffer[i] = c;
        }
    }
    memory->index += i + 1;
    buffer[i] = '\0';

    return !no_lines;
}

// Considerar escritura preventiva si el hijo no se conecto
int writeSharedMem(sharedMemADT memory, const char *buffer, int n)
{
    if (memory->index + n >= SHARED_MEM_SIZE)
        return 0;

    for (int i = 0; i < n; i += 1)
    {
        memory->sharedMem[memory->index + i] = buffer[i];
        if (buffer[i] == '\n' || buffer[i] == '\0')
        {
            sem_post(memory->viewSemaphore);
        }
    }
    memory->index += n;
    return 1;
}

int startSharedMem(sharedMemADT memory, const char *mem_name)
{
    strncpy(memory->sharedMemName, mem_name, SHARED_MEM_MAX_NAME);
    int shmFd;
    if ((shmFd = shm_open(mem_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) == -1)
    {
        free(memory);
        errorHandling("shm_open");
        return 0;
    }

    if (ftruncate(shmFd, SHARED_MEM_SIZE) == -1)
    {
        close(shmFd);
        disconnectSharedMem(memory);
        errorHandling("ftruncate");
        return 0;
    }

    memory->sharedMem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    memory->index = 0;
    close(shmFd);

    // Asigna el semaforo al ADT que el padre usa para comunicar que hay para leer.
    memory->viewSemaphore = sem_open(SEM_VIEW_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0);
    if (memory->viewSemaphore == SEM_FAILED)
    {
        disconnectSharedMem(memory);
        munmap(memory->sharedMem, SHARED_MEM_SIZE);
        free(memory);
        errorHandling("sem_open");
        return 0;
    }

    // Abre un semaforo temporal para que view pueda comunicar al padre que se conecto
    sem_t *appSem = sem_open(SEM_APP_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0);
    if (appSem == SEM_FAILED)
    {
        disconnectSharedMem(memory);
        munmap(memory->sharedMem, SHARED_MEM_SIZE);
        free(memory);
        errorHandling("sem_open");
        return 0;
    }

    struct timespec time;
    if (clock_gettime(CLOCK_REALTIME, &time) == -1)
    {
        disconnectSharedMem(memory);
        munmap(memory->sharedMem, SHARED_MEM_SIZE);
        free(memory);
        sem_close(appSem);
        sem_unlink(SEM_APP_NAME);
        errorHandling("clock_gettime");
        return 0;
    }

    time.tv_sec += CONNECTION_TIMEOUT;

    fprintf(stderr, "Esperando conexion de hijo\n");
    printf("%s\n", SHARED_MEM_NAME);
    int initialized = sem_timedwait(appSem, &time);

    sem_close(appSem);
    sem_unlink(SEM_APP_NAME);

    if (initialized == -1)
    {
        disconnectSharedMem(memory);
        if (errno != ETIMEDOUT) // si hubo un error ejecutando sem_timedwait
            perror("sem_timedwait");
        return 0;
    }
    fprintf(stderr, "Se conecto el hijo\n");
    return 1;
}

int connectSharedMem(sharedMemADT memory, const char *mem_name)
{
    strncpy(memory->sharedMemName, mem_name, SHARED_MEM_MAX_NAME);
    int shmFd;
    if ((shmFd = shm_open(mem_name, O_RDWR, S_IRUSR | S_IWUSR)) == -1)
    {
        sem_unlink(SEM_VIEW_NAME);
        free(memory);
        errorHandling("shm_open");
        return 0;
    }

    memory->sharedMem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    memory->index = 0;
    close(shmFd);

    sem_t *appSem = sem_open(SEM_APP_NAME, O_RDWR);
    if (appSem == SEM_FAILED)
    {
        fprintf(stderr, "NO SE ENCONTRO EL SEMAFORO PADRE\n");
        sem_unlink(SEM_VIEW_NAME);
        disconnectSharedMem(memory);
        munmap(memory->sharedMem, SHARED_MEM_SIZE);
        free(memory);
        errorHandling("sem_open");
        return 0;
    }

    // Avisa al app que se conecto.
    if (sem_post(appSem) == -1)
    {
        sem_unlink(SEM_VIEW_NAME);
        disconnectSharedMem(memory);
        munmap(memory->sharedMem, SHARED_MEM_SIZE);
        sem_close(appSem);
        sem_unlink(SEM_APP_NAME);
        free(memory);
        errorHandling("sem_post");
        return 0;
    }
    sem_close(appSem);

    // Abre el semaforo para que se puedan conectar
    sem_t *address = sem_open(SEM_VIEW_NAME, O_RDWR);
    if (address == SEM_FAILED)
    {
        sem_unlink(SEM_VIEW_NAME);
        disconnectSharedMem(memory);
        freeSharedMem(memory);
        errorHandling("sem_open");
        return 0;
    }
    fprintf(stderr, "Hijo conectado\n");
    memory->viewSemaphore = address;
    return 1;
}

void disconnectSharedMem(sharedMemADT memory)
{
    shm_unlink(memory->sharedMemName);
}

int closeSharedMem(sharedMemADT memory)
{
    writeSharedMem(memory, "\0", 1);
    return 1;
}

sharedMemADT initSharedMem()
{
    sharedMemADT ret = malloc(sizeof(struct sharedMemCDT));
    if (ret == NULL)
    { // warning de pvs, posible desreferenceo de null
        errorHandling("malloc");
    }
    else
    {
        ret->index = 0;
    }
    return ret;
}

void freeSharedMem(sharedMemADT memory)
{
    munmap(memory->sharedMem, SHARED_MEM_SIZE);
    sem_close(memory->viewSemaphore);
    sem_unlink(SEM_VIEW_NAME);
    free(memory);
}