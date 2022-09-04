#include "shared.h"
#include <stdio.h>

#define MD5 "md5sum"
#define MAXLENGTH 128

//devuelve en buffer el filename para pasarle a md5 como argumento. Si recibe EOF hace que el child termine.
int getFileName(char * buffer){
    size_t i;
    char buf[1];
    bool reading = true;
    for (size_t i = 0; i < MAXLENGTH && reading; i++){
        if(read(STDIN_FILENO, buf, 1) == 0) //EOF
            return -1;

        if(*buf == 0) //finished reading name
            reading = false;
        buffer[i] = *buf;
    }
    return 0;
}

void printResult(int fd){
    char buf[1];
    while(read(fd, buf, 1) != 0)
        printf("%s", buf);
    printf("\n");
}

int main(int argc, char* argv[]){

    char filename[MAXLENGTH]; //leerlo del pipe
    int pipedes[2];
    
    pid_t pid;
    int processing = true;
    
    //mientras siga recibiendo filenames desde app.c, sigue haciendo forks y llamando a md5sum
    while(getFileName(filename) == 0){
        if(pipe(pipedes) != 0)
            exit(1);
        if((pid = fork()) < 0)
            exit(1);
        else if(pid == 0){
            dup2(pipedes[1], STDOUT_FILENO);
            close(pipedes[0]);
            execlp(MD5, MD5, filename, NULL);
        }
        close(pipedes[1]);
        //imprimo en salida estandar el resultado de MD5
        printResult(pipedes[0]);
        
        close(pipedes[0]);
    }

    printf("child ended\n");

    return EXIT_SUCCESS;
}