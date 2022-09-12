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
    if ((shmFd = shm_open(mem_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR)) != -1)
    {
        if (ftruncate(shmFd, SHARED_MEM_SIZE) != -1)
        {
            memory->sharedMem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
            if (memory->sharedMem != (void *)-1)
            {
                close(shmFd);
                // Asigna el semaforo al ADT que el padre usa para comunicar que hay para leer.
                memory->viewSemaphore = sem_open(SEM_VIEW_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0);
                if (memory->viewSemaphore != SEM_FAILED)
                {
                    // Abre un semaforo temporal para que view pueda comunicar al padre que se conecto
                    sem_t *appSem = sem_open(SEM_APP_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0);
                    if (appSem != SEM_FAILED)
                    {
                        struct timespec time;
                        if (clock_gettime(CLOCK_REALTIME, &time) != -1)
                        {
                            time.tv_sec += CONNECTION_TIMEOUT;

                            printf("%s\n", SHARED_MEM_NAME);
                            int initialized = sem_timedwait(appSem, &time);

                            sem_close(appSem);
                            sem_unlink(SEM_APP_NAME);

                            if (initialized != -1)
                            {
                                return 1;
                            }
                            if (errno == ETIMEDOUT)
                                return 0;
                            perror("sem_timedwait");
                        }
                        else
                            perror("clock_gettime");
                        sem_close(appSem);
                        sem_unlink(SEM_APP_NAME);
                    }
                    else
                        perror("sem_open_app");
                    sem_close(memory->viewSemaphore);
                    sem_unlink(SEM_VIEW_NAME);
                }
                else
                    perror("sem_open_view");
                munmap(memory->sharedMem, SHARED_MEM_SIZE);
            }
            else
                perror("mmap");
        }
        else
        {
            perror("ftruncate");
        }
        close(shmFd);
        disconnectSharedMem(memory);
    }
    else
        perror("shm_open");
    free(memory);
    errorHandling("startSharedMem");
    return 0;
}

int connectSharedMem(sharedMemADT memory, const char *mem_name)
{
    strncpy(memory->sharedMemName, mem_name, strlen(mem_name));
    int shmFd;
    if ((shmFd = shm_open(mem_name, O_RDWR, S_IRUSR | S_IWUSR)) != -1)
    {
        memory->sharedMem = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
        if (memory->sharedMem != (void *)-1)
        {
            memory->index = 0;
            close(shmFd);

            sem_t *appSem = sem_open(SEM_APP_NAME, O_RDWR);
            if (appSem != SEM_FAILED)
            {
                // Avisa al app que se conecto.
                if (sem_post(appSem) != -1)
                {
                    sem_close(appSem);

                    // Abre el semaforo para que se puedan conectar
                    sem_t *address = sem_open(SEM_VIEW_NAME, O_RDWR);
                    if (address != SEM_FAILED)
                    {
                        memory->viewSemaphore = address;
                        return 1;
                    }
                    else
                        perror("sem_open");
                    freeSharedMem(memory);
                }
                else
                    perror("sem_post");
                sem_close(appSem);
                sem_unlink(SEM_APP_NAME);
            }
            else
                perror("sem_open");
            munmap(memory->sharedMem, SHARED_MEM_SIZE);
        }
        else
            perror("mmap");
        close(shmFd);
        disconnectSharedMem(memory);
    }
    else
        perror("shm_open-no-init-parent");
    sem_unlink(SEM_VIEW_NAME);
    free(memory);
    return 0;
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

void disconnectSharedMem(sharedMemADT memory)
{
    shm_unlink(memory->sharedMemName);
}

int closeSharedMem(sharedMemADT memory)
{
    writeSharedMem(memory, "\0", 1);
    return 1;
}

void freeSharedMem(sharedMemADT memory)
{
    munmap(memory->sharedMem, SHARED_MEM_SIZE);
    sem_close(memory->viewSemaphore);
    sem_unlink(SEM_VIEW_NAME);
    free(memory);
}