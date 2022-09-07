#include "shared.h"
#include <stdio.h>

#define MD5 "md5sum"

//devuelve en buffer el filename para pasarle a md5sum como argumento. Si recibe EOF hace que el child termine.
int getFileName(char * buffer){
    char buf[1];
    bool reading = true;
    size_t i;
    for (i = 0; i < MAXLENGTH && reading; ){
        if(read(STDIN_FILENO, buf, 1) == 0){ //si se recibe EOF -> el hijo muere
            return 0;
        }
        if(*buf == '\n'){ //se termino de leer el nombre
            reading = false;
        }
        else{
            buffer[i] = *buf;
            i++;
        }
    }
    buffer[i] = 0;
    return 1;
}

int main()
{

    char filename[MAXLENGTH]; // leerlo del pipe

    pid_t pid;

    // mientras siga recibiendo filenames desde app.c, sigue haciendo forks y llamando a md5sum
    while (getFileName(filename))
    {   
        if ((pid = fork()) < 0)
            exit(1);
        else if(pid == 0){
            execlp(MD5, MD5, filename, NULL);
        }
        wait(NULL);
    }

    return EXIT_SUCCESS;
}