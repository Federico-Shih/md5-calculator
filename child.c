#include "shared.h"
#include <stdio.h>

#define MD5 "md5sum"

// devuelve en buffer el filename para pasarle a md5sum como argumento. Si recibe EOF hace que el child termine.
int getFileName(char *buffer)
{
    int i = 0;
    char buf;
    int c;
    while ((c = getchar()) != EOF)
    {
        if (c == '\n' || c == '\0' || c == ' ') {
            buffer[i] = '\0';
            return 1;
        } else {
            buffer[i++] = c;
        }
    }
    return 0;
}

// Writes with format %hash  %name  %pid
void printResult(int fd, pid_t pid)
{
    result resStruct;

    char temp;
    int itName = 0;
    int c;
    if (read(fd, resStruct.hash, HASHSIZE) == -1)
        perror("read");

    while ((c = read(fd, &temp, 1) != -1) || c != 0)
    {
        if (temp == '\0' || temp == '\n')
        {
            resStruct.filename[itName] = '\0';
            break;
        }
        else if (temp != ' ')
        {
            resStruct.filename[itName++] = temp;
        }
    }

    if (c == -1)
        errorHandling("printResult");

    resStruct.processId = pid;

    // printf("%32s %s %d", resStruct.hash, resStruct.filename, resStruct.processId);
    write(STDOUT_FILENO, &resStruct, sizeof(result));
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
        else if (pid == 0)
        {
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