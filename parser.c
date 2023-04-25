#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"

command_t *parse_command(char *input) {
    command_t *command = malloc(sizeof(command_t));
    char *saveptr1;
    command->name = strtok_r(input, " \n", &saveptr1);
    command->args = malloc(sizeof(char *));
    command->num_args = 1;
    command->next = NULL;
    command->stdin_archivo = NULL;
    command->stdout_archivo = NULL;
    command->doble = 0;
    command->tuberia = 0;
    
    // Agregar el nombre del comando como primer argumento
    command->args[0] = command->name;
    
    char *arg = strtok_r(NULL, " \n", &saveptr1);
    
    while (arg != NULL) {
        if (strcmp(arg, ">") == 0 || strcmp(arg, "<") == 0 || strcmp(arg, ">>") == 0) {
            // Si el argumento es >, < o >>, procesarlo primero
            char* archivo = strtok_r(NULL, " \n", &saveptr1);
            if (archivo == NULL) {
                perror("1234");
            } else {
                if (strcmp(arg, "<") == 0) { command->stdin_archivo = archivo; }
                if (strcmp(arg, ">") == 0) { command->stdout_archivo = archivo; command->doble = 0; }
                if (strcmp(arg, ">>") == 0) { command->stdout_archivo = archivo; command->doble = 1; }
            }
        } else {
            // Si el argumento no es >, < o >>, procesarlo normalmente
            
            // Agregar el argumento al comando
            command->num_args++;
            command->args = realloc(command->args, command->num_args * sizeof(char *));
            command->args[command->num_args - 1] = arg;
        }
        
        arg = strtok_r(NULL, " \n", &saveptr1);
    }

    command->num_args++;
    command->args = realloc(command->args, command->num_args * sizeof(char *));
    command->args[command->num_args - 1] = NULL;
    --command->num_args;
    
    return command;
}
command_t *parse_commands(char *input) {
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
