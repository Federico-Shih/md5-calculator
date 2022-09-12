// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "app.h"

int main(int argc, char *argv[])
{
    if (argc <= 1)
        return 0;

    if (setvbuf(stdout, NULL, _IONBF, 0) != 0)
        errorHandling("setvbuf"); // apaga el buffer

    char *filenames[argc - 1]; // archivos a procesar
    int filecount = 0;         // cantidad de archivos a procesar

    int parsed = parseArguments(argc, argv, &filecount, filenames);

    if (parsed)
    {
        int childNum = filecount / FILES_PER_CHILD + MIN_CHILD;

        // pipedes[childNum][APPWRITES o APPREADS] representa el pipe en el que escribe o lee app
        int pipedes[childNum][2][2];
        int childPids[childNum];

        // crea pipes e hijos
        createChilds(pipedes, childNum, childPids);

        // creacion del archivo en donde se guardaran los resultados
        int fd = open("results.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd < 0)
        {
            errorHandling("open");
        }

        sharedMemADT memory = initSharedMem();
        startSharedMem(memory, SHARED_MEM_NAME);

        // asigna files a procesar a todos los hijos disponibles hasta que todos los files esten procesados
        processFiles(childNum, pipedes, filecount, filenames, childPids, fd, memory);

        // espero a que terminen todos los hijos
        while (wait(NULL) > 0)
            ;

        // cierro los fd restantes
        for (size_t itChild = 0; itChild < childNum; itChild++)
        {
            close(pipedes[itChild][APPREADS][READEND]);
        }
        close(fd);

        closeSharedMem(memory);
        disconnectSharedMem(memory);
        freeSharedMem(memory);
    }
    
    return 0;
}

void createChilds(int pipedes[][2][2], int childNum, int childPids[])
{
    pid_t pid;
    for (int i = 0; i < childNum; i++)
    {
        if (pipe(pipedes[i][APPREADS]) != 0)
            errorHandling("pipe");
        if (pipe(pipedes[i][APPWRITES]) != 0)
            errorHandling("pipe");

        if ((pid = fork()) == -1)
            errorHandling("fork");

        // Ejecucion del proceso hijo
        if (pid == 0)
        {
            dup2(pipedes[i][APPWRITES][READEND], STDIN_FILENO); // hijo lee de STDIN
            close(pipedes[i][APPWRITES][WRITEEND]);
            dup2(pipedes[i][APPREADS][WRITEEND], STDOUT_FILENO); // hijo escribe a STDOUT
            close(pipedes[i][APPREADS][READEND]);

            if (execl(CHILD, CHILD, NULL) == -1)
                errorHandling("execl");
        }
        else
        {
            childPids[i] = pid;
        }
        close(pipedes[i][APPREADS][WRITEEND]);
        close(pipedes[i][APPWRITES][READEND]);
    }
}

void processFiles(int childNum, int pipedes[][2][2], int filecount, char *filenames[], int childPids[], int fd, sharedMemADT memory)
{
    int filesSent = 0;     // le mande a los hijos para que procesen
    int filesReceived = 0; // los resultados que obtuve de los hijos

    fd_set selectfd;
    bool processing = true;

    // ocupo a todos los hijos
    for (int itChild = 0; itChild < childNum && filesSent < filecount; itChild++)
    {
        dprintf(pipedes[itChild][APPWRITES][WRITEEND], "%s\n", filenames[filesSent]);
        filesSent++;
    }

    while (processing)
    {
        int maxfd = loadSet(childNum, &selectfd, pipedes);
        int retval = select(maxfd + 1, &selectfd, NULL, NULL, NULL);

        if (retval == -1)
        {
            freeSharedMem(memory);
            errorHandling("select");
        }

        int processedChilds = 0;
        bool running = true;

        // iteracion por todos los hijos para saber cuales estan disponibles para trabajar
        for (int itChild = 0; itChild < childNum && running; itChild++)
        {
            if (FD_ISSET(pipedes[itChild][APPREADS][READEND], &selectfd))
            {
                readAndProcess(pipedes[itChild][APPREADS][READEND], childPids[itChild], fd, memory);

                filesReceived++;

                // enviar nueva tanda de filenames
                if (filesSent < filecount)
                {
                    dprintf(pipedes[itChild][APPWRITES][WRITEEND], "%s\n", filenames[filesSent]);
                    filesSent++;
                }
                if (++processedChilds == retval) // para evitar ciclos innecesarios
                    running = false;
            }
        }

        if (filesReceived == filecount)
        {
            for (int itChild = 0; itChild < childNum; itChild++)
            {
                // se cierra totalmente este pipe-> el hijo recibe EOF al leer-> el hijo interpreta que tiene que morir
                if (close(pipedes[itChild][APPWRITES][WRITEEND]) != 0)
                {
                    freeSharedMem(memory);
                    errorHandling("close");
                }
            }
            processing = false;
        }
    }
}

int loadSet(int childNum, fd_set *selectfd, int pipedes[][2][2])
{
    int maxfd = 0;
    FD_ZERO(selectfd);
    for (int itChild = 0; itChild < childNum; itChild++)
    {
        int currentfd = pipedes[itChild][APPREADS][READEND];
        FD_SET(currentfd, selectfd);
        if (currentfd > maxfd)
            maxfd = currentfd;
    }
    return maxfd;
}

int parseArguments(int argc, char *argv[], int *filecount, char *filenames[])
{
    struct stat statbuf;

    for (int i = 1; i < argc; i++)
    {
        if (stat(argv[i], &statbuf) != 0 && errno != ENOENT)
        {
            errorHandling("stat");
        }
        else if (errno != ENOENT && S_ISREG(statbuf.st_mode))
        {
            // si son files los agrego, si son cualquier otra cosa o no existen los descarto
            filenames[(*filecount)++] = argv[i];
        }
        errno = 0; // si se ingresa un argumento no valido se debe resetear errno a 0
    }

    if (*filecount == 0)
        return 0;
    return 1;
}

void readFromMD5(int fd, char *hash, int maxHash, char *filename, int maxFilename)
{
    char buf;
    int i;
    for (i = 0; i < maxHash && read(fd, &buf, 1) > 0 && buf != ' '; i++)
    {
        hash[i] = buf;
    }
    hash[i] = 0;
    read(fd, &buf, 1); // para sacar los espacios innecesarios
    read(fd, &buf, 1);
    for (i = 0; i < maxFilename && read(fd, &buf, 1) > 0 && buf != '\n'; i++)
    {
        filename[i] = buf;
    }
    filename[i] = 0;
}

void readAndProcess(int readFd, int childPid, int destFd, sharedMemADT destMemory)
{
    result resStruct;
    readFromMD5(readFd, resStruct.hash, HASHSIZE, resStruct.filename, MAX_FILENAME);
    resStruct.processId = childPid;

    char buffer[MAXLINE];
    int n = sprintf(buffer, LINE_FORMAT, resStruct.filename, resStruct.processId, resStruct.hash);

    dprintf(destFd, "%s", buffer);

    writeSharedMem(destMemory, buffer, n);
}