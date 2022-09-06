#include "shared.h"
#include <errno.h>

int main(){
    struct stat statbuf;
    int errnum;
    errnum = stat("childd.c", &statbuf);
    if (errnum != 0 && errnum != ENOENT){
        printf("El error de errno es %d y el de errnum es %d\n", errno, errnum);
        errorHandling("stat");
    }
        
    else if (errnum != ENOENT && S_ISREG(statbuf.st_mode)){
        //si son files los agrego, si son cualquier otra cosa o no existen los descarto
        puts("es un file");
        }
    return 0;
}