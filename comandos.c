#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include "parser.h"
int cd(int argc, char **argv) {
    // Verifica si se pasó el número correcto de argumentos
    if (argc != 2) {
        printf("Uso: %s [directorio]\n", argv[0]);
        return 1;
    }

    // Intenta cambiar el directorio actual del proceso
    if (chdir(argv[1]) != 0) {
        // Si hay un error, imprime un mensaje de error
        perror("Error al cambiar de directorio");
        return 1;
    }

    // Si todo salió bien, retorna 0 para indicar éxito
    return 0;
}

char *pwd() {
    static char path[1024];

    // Obtiene la ruta del directorio actual
    if (getcwd(path, sizeof(path)) == NULL) {
        perror("Error al obtener el directorio actual");
        return NULL;
    }

    return path;
}

void ls() {
    char *path;
    DIR *dir;
    struct dirent *entry;

    // Obtiene la ruta del directorio actual utilizando la función pwd
    path = pwd();
    if (path == NULL) {
        return;
    }

    // Abre el directorio especificado
    dir = opendir(path);
    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return;
    }

    // Lee el contenido del directorio
    while ((entry = readdir(dir)) != NULL) {
        // Excluye los nombres de archivo "." y ".."
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // Imprime el nombre de cada entrada seguido de un espacio
            printf("%s ", entry->d_name);
        }
    }
    printf("\n");

    // Cierra el directorio
    closedir(dir);
}
// Función para redirigir la salida estándar a un archivo
// filename: nombre del archivo al que se quiere redirigir la salida estándar
// append: si es verdadero (1), agrega al final del archivo; si es falso (0), sobrescribe el archivo
int redirigir_salida(char *filename, int doble) {
    // Hace una copia del descriptor de archivo original de stdin
    int fd;

    // Redirige stdin desde el archivo especificado
    if ((fd = open (filename, O_WRONLY | O_CREAT | (doble ? O_APPEND : 0))) < 0)
    {
        // Si hay un error, imprime un mensaje de error
        perror("Error al redirigir stdin");
        return -1;
    }

    // Retorna la copia del descriptor de archivo original de stdin
    return dup2 (fd, STDOUT_FILENO);
}

// Función para restaurar la salida estándar a su estado original
// stdout_copy: copia del descriptor de archivo original de stdout (obtenido al llamar a redirigir_salida)
void restaurar_salida(int stdout_copy) {
    // Restaura stdout a su estado original utilizando dup2
    dup2(stdout_copy, STDOUT_FILENO);
    // Cierra el descriptor de archivo copiado
    close(stdout_copy);
}

// Función para redirigir la entrada estándar desde un archivo
// filename: nombre del archivo desde el que se quiere redirigir la entrada estándar
int redirigir_entrada(char *filename) {
    // Hace una copia del descriptor de archivo original de stdin
    int fd;

    // Redirige stdin desde el archivo especificado
    if ((fd = open (filename, O_RDONLY)) < 0)
    {
        // Si hay un error, imprime un mensaje de error
        perror("Error al redirigir stdin");
        return -1;
    }

    // Retorna la copia del descriptor de archivo original de stdin
    return dup2 (fd, STDIN_FILENO);
}

// Función para restaurar la entrada estándar a su estado original
// stdin_copy: copia del descriptor de archivo original de stdin (obtenido al llamar a redirigir_entrada)
void restaurar_entrada(int stdin_copy) {
    // Restaura stdin a su estado original utilizando dup2
    dup2(stdin_copy, STDIN_FILENO);
    // Cierra el descriptor de archivo copiado
    close(stdin_copy);
}


