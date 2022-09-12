// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "lib/shared.h"
#include "child.h"

#define MD5 "md5sum"

int main()
{
    char filename[MAX_FILENAME + 1]; // leerlo del pipe
    pid_t pid;
    // mientras siga recibiendo filenames desde app.c, sigue haciendo forks y llamando a md5sum
    while (scanf("%" S(MAX_FILENAME) "s", filename) != EOF) 
    {
        if ((pid = fork()) == -1)
            exit(1);
        else if (pid == 0)
        {
            // Debido a que este filename esta recibiendo de app, no es un riesgo de seguridad
            execlp(MD5, MD5, filename, NULL);
        }
        wait(NULL);
    }

    return EXIT_SUCCESS;
}