#ifndef EJECUTAR_PROGRAMA_H
#define EJECUTAR_PROGRAMA_H

#include "parser.h"
typedef struct commandlist {
    command_t *comandos;
    char* line;
    int indice;
    struct commandlist *next;
} commandlist_t;

int *ejecutar_programa(char* line, command_t *comando, int*);

#endif