#include "shared.h"
#include <stdio.h>

int main(int argc, char* argv[]){
    if(argc < 2){
        fprintf(stderr, "incorrect agrs");
        exit(EXIT_FAILURE);
    }
    int i;
    unsigned char result[16];

    // MD5(argv[1], strlen(argv[1]), result);

    // output
    for(i = 0; i < 16; i++)
        printf("%02x", result[i]);

    printf("\n");

    return EXIT_SUCCESS;
}