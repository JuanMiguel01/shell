#ifndef PARSER_H
#define PARSER_H

typedef struct command {
    char *name;
    char **args;
    int num_args;
    struct command *next;
    struct command *previous; // Agregar un campo para el comando anterior
} command_t;


command_t *parser(char *input);

#endif