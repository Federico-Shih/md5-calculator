#include "shared.h"
#include <stdio.h>

#define MD5 "md5sum"

//devuelve en buffer el filename para pasarle a md5sum como argumento. Si recibe EOF hace que el child termine.
int getFileName(char * buffer){
    char buf[1];
    bool reading = true;
    size_t i;
    for (i = 0; i < MAXLENGTH && reading; i++){
        if(read(STDIN_FILENO, buf, 1) == 0){ //EOF
            write(STDERR_FILENO, "se ha terminado la ejecucion wey\n", strlen("se ha terminado la ejecucion wey\n"));
            return 0;
        }
        if(*buf == '\n'){ //finished reading name
            write(STDERR_FILENO, "termine wachin\n", strlen("termine wachin\n"));
            reading = false;
        }
        else{
            buffer[i] = *buf;
        }
    }
    buffer[i] = 0;
    write(STDERR_FILENO, buffer, strlen(buffer));
    write(STDERR_FILENO, "\n", 1);
    return 1;
}

// Writes with format %hash  %name  %pid
void printResult(int fd, pid_t pid)
{
    char temp;

    while(read(fd, &temp, 1) != 0 && temp != '\n')
        write(STDOUT_FILENO, temp, 1);
    
    printf("  %d\n", pid);
    write(STDERR_FILENO, "el hijo escribio algo\n", sizeof("el hijo escribio algo\n"));
}

int main()
{
    pid_t thisPid = getpid(); // pid de este hijo

    char filename[MAXLENGTH]; // leerlo del pipe
    int pipedes[2];

    pid_t pid;
    int processing = true;

    // mientras siga recibiendo filenames desde app.c, sigue haciendo forks y llamando a md5sum
    while (getFileName(filename))
    {   
        if (pipe(pipedes) != 0)
            exit(1);
        if ((pid = fork()) < 0)
            exit(1);
        else if(pid == 0){
            write(STDERR_FILENO, "llamando a md5\n", sizeof("llamando a md5\n"));
            write(STDERR_FILENO, filename, strlen(filename));
            write(STDERR_FILENO, "\n", 1);
            dup2(pipedes[1], STDOUT_FILENO);
            close(pipedes[0]);
            execlp(MD5, MD5, filename, NULL);
        }
        close(pipedes[1]);
        wait(NULL);

        // imprimo el struct salida estandar el resultado de MD5
        printResult(pipedes[0], thisPid);
        close(pipedes[0]);
    }

    return EXIT_SUCCESS;
}