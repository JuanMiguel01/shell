#ifndef PARSER_H
#define PARSER_H

typedef struct command {
    char *name;
    char **args;
    char *stdin_archivo;
    char *stdout_archivo;
    int pid;
    unsigned doble : 1;
    unsigned tuberia : 1;
    unsigned background : 1; // Agregar un campo para indicar si el comando debe ejecutarse en segundo plano
    unsigned num_args;
    struct command *next;
} command_t;

command_t *parse_commands(char *input);

#endif