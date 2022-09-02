//#include <shared.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

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

    struct stat statbuf;
    int errnum;
    char * filenames[argc-1]; //aca guardo los relative paths de los files a procesar con md5
    int filecount = 0; //cantidad de files total a procesar
    for (int i = 1; i < argc; i++){
        errnum = stat(argv[i], &statbuf);
        if (errnum != 0)
            statErrorHandling(errnum);
            
        //si son files los agrego, si son cualquier otra cosa los descarto
        if(S_ISREG(statbuf.st_mode)){
            filenames[filecount++] == argv[i];
        }
    }
    filenames[filecount] = 0; //null terminated (podemos obviar esto depende de como recorramos el array filenames)




    int childNum = filecount/20 + 3; //algoritmo avanzado que define la cantidad de hijos a crear
    unsigned char * md5[filecount]; //donde se van a almacenar los md5 (HACER QUE SEA EN ORDEN)

    int pipedes[childNum][2][2];
    //ejemplo de uso del pipe: pipedes[5][APPREADS][READEND] refiere al fd de lectura (1) del pipe que usa la app para leer informacion del child (APPREADS) del child nro 0 (5)
    //es importante cerrar pipedes[i][APPREADS][WRITEEND] y pipedes[i][APPWRITES][READEND] desde app luego de crear el child, porque app no usara estos extremos (hacer lo analogo dentro de child)

    for (int i = 0; i < childNum; i++){
        if(pipe(pipedes[i][APPREADS]) != 0)
            pipeErrorHandling();

        if(pipe(pipedes[i][APPWRITES]) != 0)
            pipeErrorHandling();

        close(pipedes[i][APPREADS][WRITEEND]);
        close(pipedes[i][APPWRITES][READEND]);
    }

    



    //creo un file (si ya existe borro sus contenidos) y le escribo los filenames junto a su md5
    int fd = open("result.txt", O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
    if(fd < 0)
        writeErrorHandling();
    
    int aux;
    //escribo todo en el file en formato: "Filename: $FILENAME MD5: $MD5"
    for (int i = 0; i < filecount; i++){
        if(write(fd, "Filename: ", aux) == -1) writeErrorHandling();
        if(write(fd, (const void *) filenames[i], aux) == -1) writeErrorHandling();
        if(write(fd, " MD5: ", aux) == -1) writeErrorHandling();
        if(write(fd, (const void *) md5[i], aux) == -1) writeErrorHandling();
        if(write(fd, "\n", aux) == -1) writeErrorHandling();
    }

    close(fd);
    
    return 0;
}