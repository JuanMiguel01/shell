#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"
char *get_next_arg(char **input)
{
    char *arg = NULL;
    char *end = NULL;
    
    // Ignorar espacios y saltos de línea al principio
    while (**input == ' ' || **input == '\n')
    {
        (*input)++;
    }

    if (**input == '\"')
    {
        // Si el argumento comienza con una comilla doble, buscar la comilla doble final
        arg = ++(*input);
        end = strchr(*input, '\"');
        if (end != NULL)
        {
            *end = '\0';
            *input = end + 1;
        }
    }
    else
    {   
        
        // Si el argumento no comienza con una comilla doble, buscar el siguiente espacio o salto de línea
        arg = *input;
        
        //printf("El valor de arg1 es: %s\n", arg);
        end = strpbrk(*input, " \n");
        

        if (end != NULL)
        {
            *end = '\0';
            *input = end + 1;
        }
        else
        {
            *input += strlen(*input);
        }
    }
    
    return arg;
}
command_t *parse_command(char *input)
{

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
    command->background = 0;
    command->if_commands = NULL;
    command->else_commands = NULL;
    command->then_commands = NULL;
    // Agregar el nombre del comando como primer argumento

    command->args[0] = command->name;
    printf(input);
    char *arg = get_next_arg(&input);
    
    while (*arg != '\0')
    {

        if (strcmp(arg, ">") == 0 || strcmp(arg, "<") == 0 || strcmp(arg, ">>") == 0)
        {
            // Si el argumento es >, < o >>, procesarlo primero
            char *archivo = get_next_arg(&input);
            if (archivo == NULL)
            {
                perror("Archivo NULL");
            }
            else
            {
                if (strcmp(arg, "<") == 0)
                {
                    command->stdin_archivo = archivo;
                }
                if (strcmp(arg, ">") == 0)
                {
                    command->stdout_archivo = archivo;
                    command->doble = 0;
                }
                if (strcmp(arg, ">>") == 0)
                {
                    command->stdout_archivo = archivo;
                    command->doble = 1;
                }
            }
        }
        else if (strcmp(command->args[0], "if") == 0 ) {
            // Si el argumento es if
            
            command->name = command->args[0];
            
            
            break;
        } 
        else
        {
            // Si el argumento no es >, < o >>, procesarlo normalmente

            // Agregar el argumento al comando
            command->num_args++;
            command->args = realloc(command->args, command->num_args * sizeof(char *));
            command->args[command->num_args - 1] = arg;
        }

        arg = get_next_arg(&input);
    }
    if (strcmp(command->args[command->num_args - 1], "&") == 0)
    {
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
command_t *parse_commands(char *input)
{
    if (input != NULL && input[0] != '\0' && input[0] != ' ')
    {
        add_history(input);
    }
    command_t *head = NULL;
    command_t *tail = NULL;

    char *saveptr1;
    // Separar la entrada en segmentos usando los símbolos || y ;
    char *segment = strtok_r(input, ";", &saveptr1);

    while (segment != NULL)
    {
        
        char *saveptr2;
        char *saveptr3;
        // Separar el segmento en subsegmentos usando el símbolo |
        
        char *subsegment = strtok_r(segment, "|", &saveptr2);
        
        while (subsegment != NULL)
        {   
            
            // Ignorar los caracteres después de un #
            char *comment = strchr(subsegment, '#');
            if (comment != NULL)
            {
                *comment = '\0';
            }
            
            char *subsegment1 = strdup(subsegment);
            
            command_t *command = parse_command(subsegment);
            
            printf("  name: %s\n", command->name);
            printf("  args: %s\n", command->if_commands);
            
            
            if ((subsegment = strtok_r(NULL, "|", &saveptr2)) != NULL)
            {
                command->tuberia = 1;
            }
            if (strncmp(subsegment1, "if", strlen("if")) == 0)
            {
                subsegment = subsegment1 + strlen("if");
                printf("Subsegment después: %s\n", subsegment);
                subsegment1 = strdup(subsegment);
            }
            
            perror("aqu");
            perror(subsegment);
            
            
            if (strcmp(command->name, "if") == 0)
            {   
                
                // Si el comando es un comando if
                command_t *if_command = malloc(sizeof(command_t));
                if_command->next = NULL;

                command->if_commands = if_command;
                
                
                while (subsegment != NULL && strcmp(subsegment, "then") != 0)
                {   
                    // Analizar los comandos entre if y then
                        char *p = strstr(subsegment, "then");
                        
                        if (p != NULL)
                        {
                            // p apunta a la primera aparición de "else" o "end" en subsegment
                            // puedes crear una nueva cadena con los caracteres desde el principio hasta antes de p
                            size_t index = p - subsegment;
                            char *new_subsegment = malloc(index + 1);
                            strncpy(new_subsegment, subsegment, index);
                            new_subsegment[index] = '\0';
                            // new_subsegment contiene la subcadena desde el principio hasta antes de "else" o "end"
                            subsegment=new_subsegment;
                        }
                       
                    command_t *cmd = parse_command(subsegment);
                    if_command->next = cmd;
                    command->if_commands = cmd;
                    
                    subsegment = NULL;
                    
                }
                
                char *p = strstr(subsegment1, "then");
                
                if (p!=NULL)
                {   
                    subsegment = p;
                    subsegment1 = strdup(subsegment);
                }
                perror("antes del if");
                perror(subsegment);
                
                if (subsegment != NULL && strncmp(subsegment, "then", 4) == 0)
                {
                    // Si se encuentra un comando then
                    
                    command_t *then_command = malloc(sizeof(command_t));
                    then_command->next = NULL;
                    command->then_commands = then_command;

                   
                    while (subsegment != NULL && strcmp(subsegment, "else") != 0 && strcmp(subsegment, "end") != 0)
                    {
                        // Analizar los comandos entre then y else o end
                        perror("debtro de then");
                        perror(subsegment);
                        char *arg=get_next_arg(&subsegment);
                        perror(subsegment);
                        char *p1 = strstr(subsegment, "else");
                        char *p2 = strstr(subsegment, "end");
                        char *p;
                        if (p1 != NULL && p2 != NULL)
                        {
                            p = (p1 < p2) ? p1 : p2;
                        }
                        else if (p1 != NULL)
                        {
                            p = p1;
                        }
                        else
                        {
                            p = p2;
                        }
                        if (p != NULL)
                        {
                            // p apunta a la primera aparición de "else" o "end" en subsegment
                            // puedes crear una nueva cadena con los caracteres desde el principio hasta antes de p
                            size_t index = p - subsegment;
                            char *new_subsegment = malloc(index + 1);
                            strncpy(new_subsegment, subsegment, index);
                            new_subsegment[index] = '\0';
                            // new_subsegment contiene la subcadena desde el principio hasta antes de "else" o "end"
                            subsegment=new_subsegment;
                        }

                        command_t *cmd = parse_command(subsegment);
                        perror(cmd->name);
                        then_command->next = cmd;
                        command->then_commands = cmd;
                       
                        subsegment = NULL;
                    }
                    char *p = strstr(subsegment1, "else");

                    if (p != NULL)
                    {
                        subsegment = p;
                        subsegment1 = strdup(subsegment);
                    }
                    perror("antes del else");
                    perror(subsegment);

                    if (subsegment != NULL && strncmp(subsegment, "else",4) == 0)
                    {
                        // Si se encuentra un comando else
                        command_t *else_command = malloc(sizeof(command_t));
                        else_command->next = NULL;
                        command->else_commands = else_command;
                        perror("antes de sub");
                        perror(subsegment);
                        if(subsegment!=NULL){
                            perror("entro al no NUl");
                        }
                        while (subsegment != NULL && strcmp(subsegment, "end") != 0)
                        {
                            // Analizar los comandos entre else y end
                        perror("dentro del else");
                        char *arg=get_next_arg(&subsegment);
                        perror("kjfna");
                        perror(subsegment);
                        
                        char *p = strstr(subsegment, "end");
                        
                        if (p != NULL)
                        {
                            // p apunta a la primera aparición de "end" en subsegment
                            // puedes crear una nueva cadena con los caracteres desde el principio hasta antes de p
                            size_t index = p - subsegment;
                            char *new_subsegment = malloc(index + 1);
                            strncpy(new_subsegment, subsegment, index);
                            new_subsegment[index] = '\0';
                            // new_subsegment contiene la subcadena desde el principio hasta antes de "else" o "end"
                            subsegment=new_subsegment;
                        }
                            perror(subsegment);
                            command_t *cmd = parse_command(subsegment);
                            else_command->next = cmd;
                            command->else_commands = cmd;

                            subsegment = NULL;
                        }
                    }
                }
            }
            
                // Agregar el comando a la lista enlazada
                if (head == NULL)
                {
                    head = command;
                    tail = command;
                }
                else
                {
                    tail->next = command;
                    tail = command;
                }
            

            
        }

        segment = strtok_r(NULL, ";&&", &saveptr1);
    }

    // Imprimir en pantalla la lista de comandos generada
    printf("Commands:\n");

    command_t *commands = head;

    while (commands != NULL)
{  
    printf("  Command: %s\n", commands->name);
    
    printf("  Arguments:\n");
    for (int i = 0; i < commands->num_args; i++)
    {
        printf("    %s\n", commands->args[i]);
    }
    printf("\n");

    // Imprimir los comandos dentro de if_commands
    if (commands->if_commands != NULL)
    {
        printf("  If Commands:\n");
        command_t *if_commands = commands->if_commands;
        while (if_commands != NULL)
        {
            printf("    Command: %s\n", if_commands->name);
            printf("    Arguments:\n");
            for (int i = 0; i < if_commands->num_args; i++)
            {
                printf("      %s\n", if_commands->args[i]);
            }
            printf("\n");
            if_commands = if_commands->next;
        }
    }

    // Imprimir los comandos dentro de then_commands
    if (commands->then_commands != NULL)
    {
        printf("  Then Commands:\n");
        command_t *then_commands = commands->then_commands;
        while (then_commands != NULL)
        {
            printf("    Command: %s\n", then_commands->name);
            printf("    Arguments:\n");
            for (int i = 0; i < then_commands->num_args; i++)
            {
                printf("      %s\n", then_commands->args[i]);
            }
            printf("\n");
            then_commands = then_commands->next;
        }
    }

    // Imprimir los comandos dentro de else_commands
    if (commands->else_commands != NULL)
    {
        printf("  Else Commands:\n");
        command_t *else_commands = commands->else_commands;
        while (else_commands != NULL)
        {
            printf("    Command: %s\n", else_commands->name);
            printf("    Arguments:\n");
            for (int i = 0; i < else_commands->num_args; i++)
            {
                printf("      %s\n", else_commands->args[i]);
            }
            printf("\n");
            else_commands = else_commands->next;
        }
    }

    commands = commands->next;
}

    return head;
}

//;  &&  || |
