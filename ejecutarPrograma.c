
#include "comandos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "parser.h"

int ejecutar_programa(command_t *comando) {
    // Recorrer la lista enlazada y ejecutar primero los comandos de redirección
    command_t *current = comando;
    int stdout_copy = -1;
    int stdin_copy = -1;
    
    while (current != NULL) {
        if (strcmp(current->name, ">") == 0) {
            printf("Redirigiendo stdout al archivo %s\n", current->args[0]);
            printf("Nombre del archivo: %s\n", current->args[0]); // Agregar esta línea
            stdout_copy = redirigir_salida(current->args[0], 0);
            printf("Redirección de stdout completada\n");
        } else if (strcmp(current->name, ">>") == 0) {
            printf("Redirigiendo stdout al archivo %s en modo append\n", current->args[0]);
            // Si el comando es ">>", redirigir stdout al archivo especificado en modo append
            stdout_copy = redirigir_salida(current->args[0], 1);
            printf("Redirección de stdout completada\n");
        } else if (strcmp(current->name, "<") == 0) {
            printf("Redirigiendo stdin desde el archivo %s\n", current->args[0]);
            // Si el comando es "<", redirigir stdin desde el archivo especificado
            stdin_copy = redirigir_entrada(current->args[0]);
            printf("Redirección de stdin completada\n");
        }
        current = current->next;
    }

    // Recorrer la lista enlazada nuevamente y ejecutar los demás comandos
    current = comando;
    while (current != NULL) {
        if (strcmp(current->name, "cd") == 0) {
            // Si el comando es "cd", llamar a la función "cd"
            return cd(current->num_args, current->args);
        } else if (strcmp(current->name, "ls") == 0) {
            printf("Ejecutando el comando ls\n");
            // Si el comando es "ls", llamar a la función "ls"
            ls();
        }
        // Agregar más casos para otros comandos aquí

        current = current->next;
    }

    // Restaurar stdout y stdin a su estado original si fueron redirigidos
    if (stdout_copy != -1) {
        restaurar_salida(stdout_copy);
    }
    if (stdin_copy != -1) {
        restaurar_entrada(stdin_copy);
    }
}

void redirect_output(command_t *command) {
    // Si hay una redirección de salida, abrir el archivo especificado
    FILE *file = fopen(command->args[0], "w");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return;
    }
    
    // Redirigir la salida estándar al archivo
    int stdout_copy = dup(STDOUT_FILENO);
    dup2(fileno(file), STDOUT_FILENO);
    
    // Ejecutar el comando anterior
    command_t *previous_command = command->previous;
    if (strcmp(previous_command->name, "ls") == 0) {
        ls();
        } else if (strcmp(previous_command->name, "cd") == 0) {
        cd(previous_command->num_args, previous_command->args);
    }
    // Agregar más casos para otros comandos aquí
    
    // Restaurar la salida estándar
    dup2(stdout_copy, STDOUT_FILENO);
    close(stdout_copy);
    
    // Cerrar el archivo
    fclose(file);
}
