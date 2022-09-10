#include "shared.h"

void errorHandling(char* error) {
    perror(error);
    exit(EXIT_FAILURE);
}
