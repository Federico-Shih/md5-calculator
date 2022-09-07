#include "shared.h"
#include <stdio.h>

#define MD5 "md5sum"

//devuelve en buffer el filename para pasarle a md5sum como argumento. Si recibe EOF hace que el child termine.
int getFileName(char * buffer){
    char buf[1];
    bool reading = true;
    size_t i;
    for (i = 0; i < MAXLENGTH && reading; ){
        if(read(STDIN_FILENO, buf, 1) == 0){ //EOF received-> child dies
            write(STDERR_FILENO, "\nhijo muerto\n", strlen("\nhijo muerto\n"));
            return 0;
        }
        if(*buf == '\n'){ //finished reading name
            reading = false;
        }
        else{
            buffer[i] = *buf;
            i++;
        }
    }
    buffer[i] = 0;
    //fprintf(stderr, "Recibi el filename: %s\n", buffer);
    return 1;
}

// Writes with format %hash  %name\n
void printResult(int fd)
{
    char temp;

    while(read(fd, &temp, 1) != 0 && temp != '\n'){
        write(STDOUT_FILENO, &temp, 1);
    }
    write(STDOUT_FILENO, "\n", 1);
}

int main()
{

    char filename[MAXLENGTH]; // leerlo del pipe
    int pipedes[2];

    pid_t pid;

    // mientras siga recibiendo filenames desde app.c, sigue haciendo forks y llamando a md5sum
    while (getFileName(filename))
    {   
        if (pipe(pipedes) != 0)
            exit(1);
        if ((pid = fork()) < 0)
            exit(1);
        else if(pid == 0){
            fprintf(stderr, "Calling md5 of file %s\n", filename);
            dup2(pipedes[1], STDOUT_FILENO);
            close(pipedes[0]);
            execlp(MD5, MD5, filename, NULL);
        }
        close(pipedes[1]);
        wait(NULL);

        // imprimo el struct salida estandar el resultado de MD5
        printResult(pipedes[0]);
        close(pipedes[0]);
    }

    return EXIT_SUCCESS;
}