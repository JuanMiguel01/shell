#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/history.h>
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
    
    char *arg = get_next_arg(&input);
    
    if (strcmp(command->name, "help") == 0) {
       
        if (arg != NULL && arg[0] == '<' && arg[strlen(arg) - 1] == '>') {
            // Crear una nueva cadena sin los símbolos < y >
            size_t keyword_len = strlen(arg) - 2;
            char *keyword = malloc(keyword_len + 1);
            strncpy(keyword, arg + 1, keyword_len);
            keyword[keyword_len] = '\0';
            command->args[0]=keyword;
            
        } else {
            command->args[0]=arg;
        }
        return command;
    }
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
        
        char *subsegment;
        int nopipe;
        if (strstr(input, "if") != NULL && strstr(input,"help")==NULL)
        {
            subsegment = input;
            nopipe=1;
        }
        else
        {
            subsegment = strtok_r(segment, "|", &saveptr2);
            nopipe=0;
        }
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
            
            
            
            
            if (nopipe==0 && (subsegment = strtok_r(NULL, "|", &saveptr2)) != NULL)
            {
                command->tuberia = 1;
            }
            if (strncmp(subsegment1, "if", strlen("if")) == 0)
            {
                subsegment = subsegment1 + strlen("if");
                
                subsegment1 = strdup(subsegment);
            }
            
            
            
            
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
                
                
                if (subsegment != NULL && strncmp(subsegment, "then", 4) == 0)
                {
                    // Si se encuentra un comando then
                    
                    command_t *then_command = malloc(sizeof(command_t));
                    then_command->next = NULL;
                    command->then_commands = then_command;

                   
                    while (subsegment != NULL && strcmp(subsegment, "else") != 0 && strcmp(subsegment, "end") != 0)
                    {
                            // Analizar los comandos entre then y else o end
                            
                            char *arg = get_next_arg(&subsegment);
                            
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
                                subsegment = new_subsegment;
                            }

                            command_t *cmd = parse_command(subsegment);
                            
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
                    

                    if (subsegment != NULL && strncmp(subsegment, "else", 4) == 0)
                    {
                            // Si se encuentra un comando else
                            command_t *else_command = malloc(sizeof(command_t));
                            else_command->next = NULL;
                            command->else_commands = else_command;
                            
                            
                            while (subsegment != NULL && strcmp(subsegment, "end") != 0)
                            {
                                // Analizar los comandos entre else y end
                                
                                char *arg = get_next_arg(&subsegment);
                                

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
                                    subsegment = new_subsegment;
                                }
                                
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


    command_t *commands = head;

    while (commands != NULL)
    {
        
        

        // Imprimir los comandos dentro de if_commands
        if (commands->if_commands != NULL)
        {
            
            command_t *if_commands = commands->if_commands;
            while (if_commands != NULL)
            {
               
                for (int i = 0; i < if_commands->num_args; i++)
                {
                    if (strchr(if_commands->args[i], '|') != NULL)
                    {
                            char *saveptr;
                            char *before_pipe = strtok_r(if_commands->args[i], "|", &saveptr);
                            char *after_pipe = strtok_r(NULL, "|", &saveptr);
                            command_t *new_command = malloc(sizeof(command_t));
                            
                            new_command->name = if_commands->args[i+1];
                            new_command->args = malloc(1024 * sizeof(char *));
                            
                            int num_args = 0;
                            for(int j=i+1;j<if_commands->num_args;j++)
                            {
                                new_command->args[num_args] = if_commands->args[j];
                                num_args++;
                               
                            }
                            new_command->num_args = num_args;
                            new_command->tuberia = 1;
                            if_commands->tuberia = 1;
                            // Agregar el nuevo comando a la lista enlazada
                            new_command->next = if_commands->next;
                            if_commands->next = new_command;
                            // Eliminar los argumentos a partir del símbolo "|"
                            if_commands->num_args = i;
                            break;
                    }
                    
                }
                
                if_commands = if_commands->next;
            }
        }

        // Imprimir los comandos dentro de then_commands
        if (commands->then_commands != NULL)
        {
            
            command_t *then_commands = commands->then_commands;
            while (then_commands != NULL)
            {
                
                for (int i = 0; i < then_commands->num_args; i++)
                {   
                    if (strchr(then_commands->args[i], '|') != NULL)
                    {
                            char *saveptr;
                            char *before_pipe = strtok_r(then_commands->args[i], "|", &saveptr);
                            char *after_pipe = strtok_r(NULL, "|", &saveptr);
                            command_t *new_command = malloc(sizeof(command_t));
                            
                            new_command->name = then_commands->args[i+1];
                            new_command->args = malloc(1024 * sizeof(char *));
                            
                            int num_args = 0;
                            for(int j=i+1;j<then_commands->num_args;j++)
                            {
                                new_command->args[num_args] = then_commands->args[j];
                                num_args++;
                               
                            }
                            new_command->num_args = num_args;
                            new_command->tuberia = 1;
                            then_commands->tuberia = 1;
                            // Agregar el nuevo comando a la lista enlazada
                            new_command->next = then_commands->next;
                            then_commands->next = new_command;
                            // Eliminar los argumentos a partir del símbolo "|"
                            then_commands->num_args = i;
                            break;
                    }
                    
                }
                
                then_commands = then_commands->next;
            }
        }

        // Imprimir los comandos dentro de else_commands
        if (commands->else_commands != NULL)
        {
            
            command_t *else_commands = commands->else_commands;
            while (else_commands != NULL)
            {
                
                for (int i = 0; i < else_commands->num_args; i++)
                {   
                     if (strchr(else_commands->args[i], '|') != NULL)
                    {
                            char *saveptr;
                            char *before_pipe = strtok_r(else_commands->args[i], "|", &saveptr);
                            char *after_pipe = strtok_r(NULL, "|", &saveptr);
                            command_t *new_command = malloc(sizeof(command_t));
                            
                            new_command->name = else_commands->args[i+1];
                            new_command->args = malloc(1024 * sizeof(char *));
                            
                            int num_args = 0;
                            for(int j=i+1;j<else_commands->num_args;j++)
                            {
                                new_command->args[num_args] = else_commands->args[j];
                                num_args++;
                               
                            }
                            new_command->num_args = num_args;
                            new_command->tuberia = 1;
                            else_commands->tuberia = 1;
                            // Agregar el nuevo comando a la lista enlazada
                            new_command->next = else_commands->next;
                            else_commands->next = new_command;
                            // Eliminar los argumentos a partir del símbolo "|"
                            else_commands->num_args = i;
                            break;
                    }
                    
                }
                
                else_commands = else_commands->next;
            }
        }

        commands = commands->next;
    }

    return head;
}

//;  &&  || |
