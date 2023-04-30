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
    unsigned background : 1;
    unsigned num_args;
    struct command *next;
    struct command *if_commands; // Agregar un campo para almacenar los comandos entre if y then
    struct command *then_commands; // Agregar un campo para almacenar los comandos entre then y else o end
    struct command *else_commands; // Agregar un campo para almacenar los comandos entre else y end
} command_t;

command_t *parse_commands(char *input);

#endif