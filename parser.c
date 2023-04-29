#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"
char *get_next_arg(char **input) {
    char *arg = NULL;
    char *end = NULL;
    
    // Ignorar espacios y saltos de línea al principio
    while (**input == ' ' || **input == '\n') {
        (*input)++;
    }
    
    if (**input == '\"') {
        // Si el argumento comienza con una comilla doble, buscar la comilla doble final
        arg = ++(*input);
        end = strchr(*input, '\"');
        if (end != NULL) {
            *end = '\0';
            *input = end + 1;
        }
    } else {
        // Si el argumento no comienza con una comilla doble, buscar el siguiente espacio o salto de línea
        arg = *input;
        end = strpbrk(*input, " \n");
        if (end != NULL) {
            *end = '\0';
            *input = end + 1;
        } else {
            *input += strlen(*input);
        }
    }
    
    return arg;
}
command_t *parse_command(char *input) {
    
    command_t *command = malloc(sizeof(command_t));
    char *saveptr1;
    command->name = get_next_arg(&input);
    command->args = malloc(sizeof(char *));
    command->num_args = 1;
    command->next = NULL;
    command->stdin_archivo = NULL;
    command->stdout_archivo = NULL;
    command->doble = 0;
    command->tuberia = 0;
    command ->background=0;
    // Agregar el nombre del comando como primer argumento
    command->args[0] = command->name;
    
    char *arg = get_next_arg(&input);
    
    while (*arg != '\0') {

        if (strcmp(arg, ">") == 0 || strcmp(arg, "<") == 0 || strcmp(arg, ">>") == 0) {
            // Si el argumento es >, < o >>, procesarlo primero
            char* archivo = get_next_arg(&input);
            if (archivo == NULL) {
                perror("1234");
            } else {
                if (strcmp(arg, "<") == 0) { command->stdin_archivo = archivo; }
                if (strcmp(arg, ">") == 0) { command->stdout_archivo = archivo; command->doble = 0; }
                if (strcmp(arg, ">>") == 0) { command->stdout_archivo = archivo; command->doble = 1; }
            }
        }else {
            // Si el argumento no es >, < o >>, procesarlo normalmente
            
            // Agregar el argumento al comando
            command->num_args++;
            command->args = realloc(command->args, command->num_args * sizeof(char *));
            command->args[command->num_args - 1] = arg;
        }
        
        arg = get_next_arg(&input);
    }
    if (strcmp(command->args[command->num_args - 1], "&") == 0) {
        command->background = 1;
        command->args[command->num_args - 1] = NULL;
        --command->num_args;
    }
    command->num_args++;
    command->args = realloc(command->args, command->num_args * sizeof(char *));
    command->args[command->num_args - 1] = NULL;
    --command->num_args;
    
    return command;
}
command_t *parse_commands(char *input) {
    if (input != NULL && input[0] != '\0' && input[0] != ' ') {
        add_history(input);
    }
    command_t *head = NULL;
    command_t *tail = NULL;
    
    char *saveptr1;
    // Separar la entrada en segmentos usando los símbolos || y ;
    char *segment = strtok_r (input, ";", &saveptr1);
    
    while (segment != NULL) {
        char *saveptr2;
        char *saveptr3;
        // Separar el segmento en subsegmentos usando el símbolo |
        char *subsegment = strtok_r (segment, "|", &saveptr2);

        while (subsegment != NULL) {
            // Ignorar los caracteres después de un #
            char *comment = strchr(subsegment, '#');
            if (comment != NULL) {
                *comment = '\0';
            }
            
            // Llamar a parse_command en cada subsegmento
            command_t *command = parse_command(subsegment);
            
            // Activar los campos tuberia_lectura y tuberia_escritura si el separador es |
            if ((subsegment = strtok_r (NULL, "|", &saveptr2)) != NULL)
            {
                command->tuberia = 1;
            }
            
            
            
            // Agregar el comando a la lista enlazada
            if (head == NULL)
            {
                head = command;
                tail = command;
            } else {
                tail->next = command;
                tail = command;
            }
        }
        
        segment = strtok_r(NULL, ";&&", &saveptr1);
    }
   
    // Imprimir en pantalla la lista de comandos generada
    printf("Commands:\n");
    
    command_t *commands = head;
    
    while (commands != NULL) {
        printf("  Command: %s\n", commands->name);
        printf("  Arguments:\n");
        for (int i = 0; i < commands->num_args; i++) {
            printf("    %s\n", commands->args[i]);
        }
        printf("\n");
        
        commands = commands->next;
    }
    
    return head;
}

//;  &&  || |
