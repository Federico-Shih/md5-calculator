#include "app.h"


int main(int argc, char * argv[]){
    if(argc <= 1)
        errorHandling("Invalid number of arguments");
    
    int shm_fd;
    if ((shm_fd = shm_open(SHARED_MEM_DIR, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR)) == -1) {
        errorHandling("shm_open");
    }

    if (ftruncate(shm_fd, sizeof(struct shared_result)) == -1) {
        errorHandling("ftruncate");
    }

/*
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

    int childNum = filecount/20 + 3; //algoritmo avanzado que define la cantidad de hijos a crear

    // pipedes[childNum][APPWRITES or APPREADS] represents pipe for app writing and pipe for app reading
    int pipedes[childNum][2][2];
    // childPids stores the pids of the child processes :D
    int childPids[childNum]; //creo que es innecesario
    // Create pipes and childs
    createChilds(pipedes, childNum, childPids);
    printf("Childs created\n");



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
                execl(CHILD, CHILD, (char *)NULL);
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
    for(int itChild = 0; itChild < childNum && filesSent < filecount; itChild++, filesSent++){
        write(pipedes[itChild][APPWRITES][WRITEEND], filenames[filesSent], strlen(filenames[filesSent]) + 1);
    }
    
    fd_set selectfd;
    bool processing = true;
    
    int maxfd, retval;
    
    while (processing){
        //loadSet
        maxfd = loadSet(childNum, &selectfd, pipedes);
        printf("PROCESSING...\n");
        //SELECT: me tengo que fijar que pipedes[i][APPREADS][READEND] este listo para ser leido (porque eso significa que el child ya termino de procesar el file anterior)
        // quedan dentro de selectfd los fds que estan listos para hacer una tarea
        
        retval = select(maxfd, &selectfd, NULL, NULL, NULL);
        printf("por ahora anda...\n");
        
        if(retval == -1){
            errorHandling("select");
        } else if(retval){ //retval es la cantidad de hijos que termino de procesar
            readChildsAndProcess(childNum, &filesReceived, &filesSent, filecount, filenames, &selectfd, pipedes);
        } else {
            printf("waiting for childs to complete md5sum");
            sleep(1);
        }
        //si todos los files fueron procesados termino el loop.
        if(filesReceived >= filecount)
            processing = false;
    }
}

int loadSet(int childNum, fd_set *selectfd, int pipedes[][2][2]){
    int currentfd, maxfd = 0;
    for(int itChild = 0 ; itChild < childNum; itChild++){
            FD_ZERO(selectfd);
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
        if (errnum != 0)
            errorHandling("stat");
            
        //si son files los agrego, si son cualquier otra cosa los descarto
        if(S_ISREG(statbuf.st_mode)){
            filenames[(*filecount)++] = argv[i];
        }
    }

    if (filecount == 0)
        errorHandling("No arguments inserted\n");
}

void readChildsAndProcess(int childNum, int *filesReceived, int* filesSent, int filecount, char* filenames[], fd_set *selectfd, int pipedes[][2][2]){
    char readbuf[1];
    char fileName[MAXLENGTH];
    char hash[32]; // Hash es definido 32 bytes
    char pid[MAXLENGTH];
    
    result resStruct;

    // DEJAR NULL TERMINATED LOS 3 CAMPOS o en hash no hace falta :)? -> opinion total
    
    for(int itChild = 0; itChild < childNum; itChild++){
        if(FD_ISSET(pipedes[itChild][APPREADS][READEND], selectfd)){
                    
            //si filesSent < filecount -> leo del hijo que termino y le paso un proceso nuevo
            //si filesSent == filecount -> leo del hijo que termino y le paso EOF para que ese hjo muera
            if (read(pipedes[itChild][APPREADS][READEND], &resStruct, sizeof(result)) != -1) {
                errorHandling("read parent");
            }

            printf("Filename:%s, PID:%d, Hash:%s", resStruct.filename, resStruct.processId, resStruct.hash); //delete later

            (*filesReceived)++;

            if(*filesSent < filecount){
                write(pipedes[itChild][APPWRITES][WRITEEND], filenames[*filesSent], strlen(filenames[*filesSent])+1);
                (*filesSent)++;
            }else{
                //close last write pid of child pipe-> child receives EOF when reading pipe-> child ends
                close(pipedes[itChild][APPWRITES][WRITEEND]);
            }
        }
    }
}