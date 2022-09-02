#include <shared.h>

int main(int argc, char* argv[]){
    if(argc < 2){
        fprintf(STDERR, "incorrect agrs");
        exit(EXIT_FAILURE);
    }
    int i;
    unsigned char result[MD5_DIGEST_LENGTH];

    MD5(argv[1], strlen(argv[1]), result);

    // output
    for(i = 0; i < MD5_DIGEST_LENGTH; i++)
        printf("%02x", result[i]);

    printf("\n");

    return EXIT_SUCCESS;
}