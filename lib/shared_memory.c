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

    return !no_lines && i < MAXLINE;
}

int writeSharedMem(sharedMemADT memory, const char *buffer, int n)
{
    if (memory->index + n >= SHARED_MEM_SIZE)
        return 0;

    // No se utilizo strcpy debido a que queremos comunicar al view que hay una linea para leer
    for (int i = 0; i < n; i += 1)
    {
        memory->sharedMem[memory->index + i] = buffer[i];
        if (buffer[i] == '\n' || buffer[i] == '\0')
        {
            if (sem_post(memory->viewSemaphore) == -1)
            {
                perror("sem_post");
                sem_close(memory->viewSemaphore);
                sem_unlink(SEM_APP_NAME);
                return 0;
            }
        }
    }
    memory->index += n;
    return 1;
}

int startSharedMem(sharedMemADT memory, const char *mem_name)
{
    strncpy(memory->sharedMemName, mem_name, SHARED_MEM_MAX_NAME);
    int shmFd;
    if ((shmFd = shm_open(mem_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR)) == -1)
    {
        freeSharedMemLevel(memory, ADT);
        return 0;
    }

    if (ftruncate(shmFd, SHARED_MEM_SIZE) == -1)
    {
        close(shmFd);
        freeSharedMemLevel(memory, ADT | DISCONNECT);
        return 0;
    }

    memory->sharedMem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    close(shmFd);
    if (memory->sharedMem == (void *)-1)
    {
        freeSharedMemLevel(memory, ADT | UNMAP | DISCONNECT);
        return 0;
    }

    // Asigna el semaforo al ADT que el padre usa para comunicar que hay para leer.
    memory->viewSemaphore = sem_open(SEM_VIEW_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0);
    if (memory->viewSemaphore == SEM_FAILED)
    {
        freeSharedMemLevel(memory, ADT | UNMAP | DISCONNECT);
        return 0;
    }

    // Abre un semaforo temporal para que view pueda comunicar al padre que se conecto
    sem_t *appSem = sem_open(SEM_APP_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0);
    if (appSem == SEM_FAILED)
    {
        freeSharedMem(memory);
        return 0;
    }

    struct timespec time;
    if (clock_gettime(CLOCK_REALTIME, &time) == -1)
    {
        sem_close(appSem);
        sem_unlink(SEM_APP_NAME);
        freeSharedMem(memory);
        return 0;
    }

    time.tv_sec += CONNECTION_TIMEOUT;

    printf("%s\n", SHARED_MEM_NAME);
    int initialized = sem_timedwait(appSem, &time);

    sem_close(appSem);
    sem_unlink(SEM_APP_NAME);

    if (initialized == -1)
    {
        if (errno != ETIMEDOUT) {
            freeSharedMem(memory);
        }
        return 0;
    }

    return 1;
}

int connectSharedMem(sharedMemADT memory, const char *mem_name)
{
    strncpy(memory->sharedMemName, mem_name, strlen(mem_name));
    int shmFd;
    if ((shmFd = shm_open(mem_name, O_RDWR, S_IRUSR | S_IWUSR)) == -1)
    {
        freeSharedMemLevel(memory, DISCONNECT | ADT);
        return 0;
    }

    memory->sharedMem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (memory->sharedMem == (void *)-1)
    {
        close(shmFd);
        freeSharedMemLevel(memory, DISCONNECT | ADT);
        return 0;
    }

    close(shmFd);

    sem_t *appSem = sem_open(SEM_APP_NAME, O_RDWR);
    if (appSem == SEM_FAILED)
    {
        freeSharedMemLevel(memory, DISCONNECT | ADT | UNMAP);
        return 0;
    }

    // Avisa al app que se conecto.
    if (sem_post(appSem) == -1)
    {
        sem_close(appSem);
        sem_unlink(SEM_APP_NAME);
        freeSharedMemLevel(memory, DISCONNECT | ADT | UNMAP);
        return 0;
    }

    sem_close(appSem);

    // Abre el semaforo para que se puedan conectar
    sem_t *address = sem_open(SEM_VIEW_NAME, O_RDWR);
    if (address == SEM_FAILED)
    {
        freeSharedMemLevel(memory, DISCONNECT | ADT | UNMAP); 
        return 0;
    }
           
    memory->viewSemaphore = address;
    return 1;
}

sharedMemADT initSharedMem()
{
    sharedMemADT ret = malloc(sizeof(struct sharedMemCDT));
    if (ret == NULL)
    { 
        perror("malloc");
    }
    else
    {
        ret->index = 0;
    }
    return ret;
}

void disconnectSharedMem(sharedMemADT memory)
{
    freeSharedMemLevel(memory, DISCONNECT);
}

int closeSharedMem(sharedMemADT memory)
{
    writeSharedMem(memory, "\0", 1);
    return 1;
}

void freeSharedMem(sharedMemADT memory)
{
    freeSharedMemLevel(memory, ADT | UNMAP | CLOSE_SEM);
}

void freeSharedMemLevel(sharedMemADT memory, int freeLevel)
{
    if ((freeLevel & CLOSE_SEM) == CLOSE_SEM)
    {
        sem_close(memory->viewSemaphore);
        sem_unlink(SEM_VIEW_NAME);
    }
    if ((freeLevel & UNMAP) == UNMAP)
    {
        munmap(memory->sharedMem, SHARED_MEM_SIZE);
    }
    if ((freeLevel & DISCONNECT) == DISCONNECT)
    {
        shm_unlink(memory->sharedMemName);
    }
    if ((freeLevel & ADT) == ADT)
    {
        free(memory);
    }

}
