#include "app.h"


int main(int argc, char * argv[]){
    if(argc <= 1)
        errorHandling("Invalid number of arguments");

    setvbuf(stdin, NULL, _IONBF, 0); //turn off buffering
/*   
    int shm_fd;
    if ((shm_fd = shm_open(SHARED_MEM_DIR, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR)) == -1) {
        errorHandling("shm_open");
    }

    if (ftruncate(shm_fd, sizeof(struct shared_result)) == -1) {
        errorHandling("ftruncate");
    }


    shared_result* shared_mem = mmap(NULL, sizeof(struct shared_result), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (sem_init(&(shared_mem->semaphore), 1, 0) == -1) {
        errorHandling("sem_init");
    }
    printf("%s\n", SHARED_MEM_DIR);
    struct timespec time;
    if (clock_gettime(CLOCK_REALTIME, &time) == -1)
        return -1;

    time.tv_sec += 10;
    shared_mem->size = 1;
    int initializedView = sem_timedwait(&(shared_mem->semaphore), &time);

    if (initializedView == -1) {
        sem_post(&(shared_mem->semaphore));
    }
    return 4;
    */
    
    char* filenames[argc-1]; //null terminated names of valid arguments
    int filecount = 0; //size of filenames
    parseArguments(argc, argv, &filecount, filenames);    

    int childNum;
    if(filecount >= 3)
        childNum = filecount/20 + 3; //algoritmo avanzado que define la cantidad de hijos a crear
    else
        childNum = filecount;
    // pipedes[childNum][APPWRITES or APPREADS] represents pipe for app writing and pipe for app reading
    int pipedes[childNum][2][2];
    // childPids stores the pids of the child processes :D
    int childPids[childNum]; //creo que es innecesario
    // Create pipes and childs
    createChilds(pipedes, childNum, childPids);

    //asigna files a procesar a todos los hijos disponibles hasta que todos los files esten procesados
    processFiles(childNum, pipedes, filecount, filenames);

    // //creo un file (si ya existe borro sus contenidos) y le escribo los filenames junto a su md5
    // int fd = open("result.txt", O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
    // if(fd < 0) errorHandling("open");
    // // escribo todo en el file en formato: "Filename: $FILENAME MD5: $MD5"
    // for (int i = 0; i < filecount; i++){
    //     if(write(fd, "Filename: ", aux) == -1) writeErrorHandling();
    //     if(write(fd, (const void *) filenames[i], aux) == -1) writeErrorHandling();
    //     if(write(fd, " MD5: ", aux) == -1) writeErrorHandling();
    //     if(write(fd, (const void *) md5[i], aux) == -1) writeErrorHandling();
    //     if(write(fd, "\n", aux) == -1) writeErrorHandling();
    // }

    //cierro los fd restantes
    for (size_t itChild = 0; itChild < childNum; itChild++){
        close(pipedes[itChild][APPREADS][READEND]);
    }

/*
    close(fd);
    sem_close(&(shared_mem->semaphore));
    munmap(shared_mem, sizeof(shared_result));
    close(shm_fd);
    */
    return 0;
}

void createChilds(int pipedes[][2][2], int childNum, int childPids[]){
        pid_t pid;
        for (int i = 0; i < childNum; i++){
            if(pipe(pipedes[i][APPREADS]) != 0)
                errorHandling("pipe");
            if(pipe(pipedes[i][APPWRITES]) != 0)
                errorHandling("pipe");
        
        
            if ((pid = fork()) == -1) {
                errorHandling("fork");
            }
            // Execute child process
            if (pid == 0) {
                dup2(pipedes[i][APPWRITES][READEND], STDIN_FILENO); //child reads from STDIN
                close(pipedes[i][APPWRITES][WRITEEND]);
                dup2(pipedes[i][APPREADS][WRITEEND], STDOUT_FILENO); //child writes to STDOUT
                close(pipedes[i][APPREADS][READEND]);

                if(execl(CHILD, CHILD, (char *)NULL) == -1)
                    errorHandling("execl");
            }else{
                childPids[i] = pid;
            }
            close(pipedes[i][APPREADS][WRITEEND]);
            close(pipedes[i][APPWRITES][READEND]);
        }
}

void processFiles(int childNum, int pipedes[][2][2], int filecount, char * filenames[]){

    int filesSent = 0; //le mande a los hijos para que procesen
    int filesReceived = 0; //los resultados que obtuve de los hijos

    //ocupo a todos los hijos
    for(int itChild = 0; itChild < childNum && filesSent < filecount; itChild++){
        printf("Enviando a child el nombre: %s de longitud %d\n", filenames[filesSent], strlen(filenames[filesSent]));
        dprintf(pipedes[itChild][APPWRITES][WRITEEND], "%s\n", filenames[filesSent]);
        filesSent++;
    }

    fd_set selectfd;
    bool processing = true;
    
    int maxfd, retval;
    
    while (processing){
        //loadSet
        maxfd = loadSet(childNum, &selectfd, pipedes);
        //SELECT: me tengo que fijar que pipedes[i][APPREADS][READEND] este listo para ser leido (porque eso significa que el child ya termino de procesar el file anterior)
        // quedan dentro de selectfd los fds que estan listos para hacer una tarea
        
        retval = select(maxfd, &selectfd, NULL, NULL, NULL);
        
        if(retval == -1){
            errorHandling("select");
        } else if(retval){ //retval es la cantidad de hijos que termino de procesar
            readChildsAndProcess(childNum, retval, &filesReceived, &filesSent, filecount, filenames, &selectfd, pipedes);
        }
        //si todos los files fueron procesados termino el loop.
        if(filesReceived >= filecount)
            processing = false;
    }
}

int loadSet(int childNum, fd_set *selectfd, int pipedes[][2][2]){
    int currentfd, maxfd = 0;
    FD_ZERO(selectfd);
    for(int itChild = 0 ; itChild < childNum; itChild++){
            currentfd = pipedes[itChild][APPREADS][READEND];
            FD_SET(currentfd, selectfd);
            if(currentfd > maxfd)
                maxfd = currentfd;
    }
    return maxfd;
}

void parseArguments(int argc, char * argv[], int * filecount, char * filenames[]){
    struct stat statbuf;
    int errnum;
    for (int i = 1; i < argc; i++){
        errnum = stat(argv[i], &statbuf);
        if (errnum != 0 && errno != ENOENT){
            errorHandling("stat");
        }
        else if (errno != ENOENT && S_ISREG(statbuf.st_mode)){
            //si son files los agrego, si son cualquier otra cosa o no existen los descarto
            filenames[(*filecount)++] = argv[i];
        }
        errno = 0;
    }

    if (*filecount == 0)
        errorHandling("No valid arguments inserted\n");
}

void readChildsAndProcess(int childNum, int fdNum, int *filesReceived, int* filesSent, int filecount, char* filenames[], fd_set *selectfd, int pipedes[][2][2]){
    char readBuf;
    int processedFds = 0;
    char pidAux[6];
    result resStruct;
    
    for(int itChild = 0; itChild < childNum; itChild++){
        if(FD_ISSET(pipedes[itChild][APPREADS][READEND], selectfd)){
            puts("im alive");
            //si filesSent < filecount -> leo del hijo que termino y le paso un proceso nuevo
            //si filesSent == filecount -> leo del hijo que termino y le paso EOF para que ese hjo muera
            readUntilWhitespace(pipedes[itChild][APPREADS][READEND], resStruct.hash, sizeof(resStruct.hash)-1);
            read(pipedes[itChild][APPREADS][READEND], &readBuf, 1);
            read(pipedes[itChild][APPREADS][READEND], &readBuf, 1);
            readUntilWhitespace(pipedes[itChild][APPREADS][READEND], resStruct.filename, sizeof(resStruct.filename)-1);puts("sigo vivo'''''''''''''''''''");
            read(pipedes[itChild][APPREADS][READEND], &readBuf, 1);puts("sigo vivo'''''''''''''''''''");
            read(pipedes[itChild][APPREADS][READEND], &readBuf, 1);puts("sigo vivo'''''''''''''''''''");
            readUntilWhitespace(pipedes[itChild][APPREADS][READEND], pidAux, sizeof(pidAux)-1);
            read(pipedes[itChild][APPREADS][READEND], &readBuf, 1);
            
            resStruct.processId = atoi(pidAux);


            printf("PARENT RECEIVED: Filename:%s, PID:%d, Hash:%s", resStruct.filename, resStruct.processId, resStruct.hash); //delete later

            (*filesReceived)++;

            if(*filesSent < filecount){
                write(pipedes[itChild][APPWRITES][WRITEEND], filenames[*filesSent], strlen(filenames[*filesSent])+1);
                (*filesSent)++;
            }else{
                //close last write pid of child pipe-> child receives EOF when reading pipe-> child ends
                close(pipedes[itChild][APPWRITES][WRITEEND]);
            }
            if(++processedFds > fdNum) //to avoid unnecessary for cycles
                break;
        }
    }
}

//returns null term string in dest of the string read from fd until a whitespace or maxlength
void readUntilWhitespace(int fd, char * dest, int maxlength){
    char buf;
    int ret;
    int it = 0;
    while((ret = read(fd, &buf, 1)) > -40 && buf != ' ' && buf != '\n' && it < maxlength-1){
        dest[it++] = buf;
    }
    dest[it+1] = 0;
    printf("EL MENSAJE LEIDO FUE %s\n", dest);
}