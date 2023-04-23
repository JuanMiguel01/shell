#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"


 command_t *parser(char *input) {
    // Dividir la línea en tokens usando espacios y saltos de línea como delimitadores
    char *token = strtok(input, " \n");
    // Variable para contar cuántas tuberías se han encontrado
    int pipe_count = 0;
    
    // Crear una lista enlazada para almacenar los comandos y sus argumentos
    command_t *head = NULL;
    command_t *tail = NULL;
    
    // Variable para almacenar el comando actualmente en proceso
    command_t *current_command = NULL;

    // Variable para almacenar el comando anterior
    command_t *previous_command = NULL;
    
    // Recorrer cada token
    while (token != NULL || token != '\0' ) {
        
        // Si el token es un #, salir del bucle
        if (strcmp(token, "#") == 0) {
            break;
        } else if (strcmp(token, "cd") == 0) {
            // Crear un nuevo comando y agregarlo a la lista enlazada
            current_command = malloc(sizeof(command_t));
            current_command->name = "cd";
            current_command->args = malloc(2 * sizeof(char *));
            current_command->num_args = 2;
            current_command->next = NULL;
            
            // Agregar el nombre del comando como primer argumento
            current_command->args[0] = "cd";
            
            // Obtener el siguiente token (que debería ser el argumento del comando cd)
            token = strtok(NULL, " \n");
            current_command->args[1] = token;
            
        } else if (strcmp(token, "<") == 0) {
            // Agregar el comando actualmente en proceso a la lista enlazada (si existe)
            if (current_command != NULL) {
                if (head == NULL) {
                    head = current_command;
                    tail = current_command;
                } else {
                    tail->next = current_command;
                    tail = current_command;
                }
                current_command = NULL;
            }
            
            // Crear un nuevo comando y agregarlo a la lista enlazada
            command_t *new_command = malloc(sizeof(command_t));
            new_command->name = "<";
            new_command->args = malloc(sizeof(char *));
            new_command->num_args = 1;
            new_command->next = NULL;
            
            // Obtener el siguiente token (que debería ser el argumento del comando <)
            token = strtok(NULL, " \n");
            new_command->args[0] = token;
            
            // Agregar el nuevo comando a la lista enlazada
            if (head == NULL) {
                head = new_command;
                tail = new_command;
            } else {
                tail->next = new_command;
                tail = new_command;
            }
        }else if (strcmp(token, ">") == 0) {
             // Agregar el comando actualmente en proceso a la lista enlazada (si existe)
             if(current_command!=NULL){
                 if(head==NULL){
                     head=current_command;
                     tail=current_command;
                 } else {
                     tail->next=current_command;
                     tail=current_command; 
                 }
                 current_command=NULL; 
             }

             // Crear un nuevo comando y agregarlo a la lista enlazada
             command_t *new_command=malloc(sizeof(command_t));
             new_command->name=">";
             new_command->args=malloc(sizeof(char *));
             new_command->num_args=1; 
             new_command->next=NULL;

             // Obtener el siguiente token (que debería ser el argumento del comando >)
             token=strtok(NULL," \n");
             new_command->args[0]=token;

             // Agregar el nuevo comando a la lista enlazada
             if(head==NULL){
                 head=new_command;
                 tail=new_command;
             } else {
                 tail->next=new_command;
                 tail=new_command;
             }
        } else if (strcmp(token, ">>") == 0) {
            // Agregar el comando actualmente en proceso a la lista enlazada (si existe)
            if (current_command != NULL) {
                if (head == NULL) {
                    head = current_command;
                    tail = current_command;
                } else {
                    tail->next = current_command;
                    tail = current_command;
                }
                current_command = NULL;
            }
            
            // Crear un nuevo comando y agregarlo a la lista enlazada
            command_t *new_command = malloc(sizeof(command_t));
            new_command->name = ">>";
            new_command->args = malloc(sizeof(char *));
            new_command->num_args = 1;
            new_command->next = NULL;
            
            // Obtener el siguiente token (que debería ser el argumento del comando >>)
            token = strtok(NULL, " \n");
            new_command->args[0] = token;
            
            // Agregar el nuevo comando a la lista enlazada
            if (head == NULL) {
                head = new_command;
                tail = new_command;
            } else {
                tail->next = new_command;
                tail = new_command;
            }
        } else if (strcmp(token, "ls") == 0) {
            // Crear un nuevo comando y agregarlo a la lista enlazada
            current_command = malloc(sizeof(command_t));
            current_command->name = "ls";
            current_command->args = malloc(sizeof(char *));
            current_command->num_args = 1;
            current_command->next = NULL;
            
            // Agregar el nombre del comando como primer argumento
            current_command->args[0] = "ls";
        }else if (strcmp(token, "|") == 0) {
            // Aumentar el contador de tuberías
            pipe_count++;
        } else if (strcmp(token, "&") == 0) {
            printf("comando & reconocido");
        } else if (strcmp(token, "jobs") == 0) {
            printf("comando jobs reconocido");
        } else if (strcmp(token, "fg") == 0) {
            printf("comando fg reconocido");
        }   else {
            // Si no es ninguno de los tokens especiales, asumir que es un argumento del comando actual
            if (current_command == NULL) {
                // Si no hay un comando actualmente en proceso, crear uno nuevo
                current_command = malloc(sizeof(command_t));
                current_command->name = token;
                current_command->args = malloc(10 * sizeof(char *));
                current_command->num_args = 1;
                current_command->next = NULL;
                
                // Agregar el nombre del comando como primer argumento
                current_command->args[0] = token;
            } else {
                // Si hay un comando actualmente en proceso, agregar el token como un argumento adicional
                current_command->args[current_command->num_args] = token;
                current_command->num_args++;
            }
        }
        
        // Obtener el siguiente token
        token = strtok(NULL, " \n");
    }
    
    // Agregar el último comando a la lista enlazada (si existe)
    if (current_command != NULL) {
        if (head == NULL) {
            head = current_command;
            tail = current_command;
        } else {
            tail->next = current_command;
            tail = current_command;
        }
    }
    
    // Recorrer la lista enlazada e imprimir cada comando y sus argumentos
    command_t *current = head;
    while (current != NULL) {
        printf("%s ", current->name);
        for (int i = 1; i < current->num_args; i++) {
            printf("%s ", current->args[i]);
        }
        printf("\n");
        current = current->next;
    }
    
    return head;
}