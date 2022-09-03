#include "shared.h"
#include <semaphore.h>
#include <time.h>

#define APPREADS 0 //el pipe que usara app para leer (recibir info de child)
#define APPWRITES 1 //el pipe que usara app para escribir (enviar info a child)

// pipefd[0] refers to the read end of the pipe.  pipefd[1] refers to the  write  end of the pipe.
#define READEND 0
#define WRITEEND 1

void statErrorHandling(int errnum){
    //complete
    exit(1);
}

void writeErrorHandling(){
    exit(1);
}

void pipeErrorHandling(){
    //ver errno
    exit(1);
}



int main(int argc, char * argv[]){
    if(argc <= 1)
        return 0;

    int shm_fd;
    if ((shm_fd = shm_open(SHARED_MEM_DIR, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR)) == -1) {
        perror("shm_open\n");
    }

    if (ftruncate(shm_fd, sizeof(shared_results)) == -1) {
        perror("ftruncate\n");
    }

    shared_results* shared_mem = mmap(NULL, sizeof(shared_results), PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (sem_init(shared_mem->semaphore, 1, 1) == -1) {
        perror("sem_init\n");
    }

    struct timespec time = { .tv_sec = 2, .tv_nsec = 0};
    int initializedView = sem_timedwait(shared_mem->semaphore, &time);
    if (initializedView == -1) {
        sem_post(shared_mem->semaphore);
    }

    struct stat statbuf;
    int errnum;
    char* filenames[argc-1]; 
    int filecount = 0; 
    for (int i = 1; i < argc; i++){
        errnum = stat(argv[i], &statbuf);
        if (errnum != 0)
            statErrorHandling(errnum);
            
        //si son files los agrego, si son cualquier otra cosa los descarto
        if(S_ISREG(statbuf.st_mode)){
            filenames[filecount++] == argv[i];
        }
    }

    int childNum = filecount/20 + 3; //algoritmo avanzado que define la cantidad de hijos a crear

    // pipedes[childNum][APPWRITES or APPREADS] represents pipe for app writing and pipe for app reading
    int pipedes[childNum][2][2];

    // Create pipes
    for (int i = 0; i < childNum; i++){
        if(pipe(pipedes[i][APPREADS]) != 0)
            pipeErrorHandling();

        if(pipe(pipedes[i][APPWRITES]) != 0)
            pipeErrorHandling();
        
        pid_t pid;
        if ((pid = fork()) == -1) {
            perror("Fork failed\n");
        }
        // Execute child process
        if (pid == 0) {
            dup2(pipedes[i][APPWRITES][READEND], STDIN_FILENO); //child reads from STDIN
            close(pipedes[i][APPWRITES][WRITEEND]);
            dup2(pipedes[i][APPREADS][WRITEEND], STDOUT_FILENO); //child writes to STDOUT
            close(pipedes[i][APPREADS][READEND]);
            execl("./child", "./child", (char *)NULL);
        }
        close(pipedes[i][APPREADS][WRITEEND]);
        close(pipedes[i][APPWRITES][READEND]);
    }

    //creo un file (si ya existe borro sus contenidos) y le escribo los filenames junto a su md5
    int fd = open("result.txt", O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
    if(fd < 0)
        writeErrorHandling();
    
    bool processing = true;
    int files = 0; 
    while (processing) {
        for (int childs = 0; childs < childNum; childs += 1) {
            // Send files to childs
            // if files == fileCount, close child pipes
        }
        // select and write to view
    }

    
    //escribo todo en el file en formato: "Filename: $FILENAME MD5: $MD5"
    // for (int i = 0; i < filecount; i++){
    //     if(write(fd, "Filename: ", aux) == -1) writeErrorHandling();
    //     if(write(fd, (const void *) filenames[i], aux) == -1) writeErrorHandling();
    //     if(write(fd, " MD5: ", aux) == -1) writeErrorHandling();
    //     if(write(fd, (const void *) md5[i], aux) == -1) writeErrorHandling();
    //     if(write(fd, "\n", aux) == -1) writeErrorHandling();
    // }


    close(fd);
    sem_close(shared_mem->semaphore);
    munmap(shared_mem, sizeof(shared_results));
    close(shm_fd);
    return 0;
}