// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef SHARED_H_
#define SHARED_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_RESULTS 1
#define SHARED_MEM_DIR "/shared"
#define SHARED_MEM_MAX_NAME 32

// https://stackoverflow.com/questions/65174086/is-there-any-difference-between-filename-max-and-path-max-in-c
#define MAX_FILENAME 1024
#define HASHSIZE 32
#define MAX_PID 6 // porque el maximo pid posible es 32768

#define MAXLINE (MAX_FILENAME + HASHSIZE + MAX_PID + 24)

// MACRO UTILIZADO PARA CONCATENAR STRINGS EN PREPROCESAMIENTO https://stackoverflow.com/questions/25410690/scanf-variable-length-specifier
#define S_(x) #x
#define S(x) S_(x)


void errorHandling(char *error);

#endif