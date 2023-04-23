#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
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
int redirigir_salida(char *filename, int append) {
    // Hace una copia del descriptor de archivo original de stdout
    printf(append);
    
    int stdout_copy = dup(STDOUT_FILENO);
     printf("entro al metodo1");
    // Redirige stdout al archivo especificado
    // Si append es verdadero (1), utiliza el modo "a" para agregar al final del archivo
    // Si append es falso (0), utiliza el modo "w" para sobrescribir el archivo
    if (freopen(filename, append ? "a" : "w", stdout) == NULL) {
        // Si hay un error, imprime un mensaje de error
        
        perror("Error al redirigir stdout");
        return -1;
    } printf("entro al metodo3");
    printf("stdout");
    // Retorna la copia del descriptor de archivo original de stdout
    return stdout_copy;
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
    int stdin_copy = dup(STDIN_FILENO);

    // Redirige stdin desde el archivo especificado
    if (freopen(filename, "r", stdin) == NULL) {
        // Si hay un error, imprime un mensaje de error
        perror("Error al redirigir stdin");
        return -1;
    }

    // Retorna la copia del descriptor de archivo original de stdin
    return stdin_copy;
}

// Función para restaurar la entrada estándar a su estado original
// stdin_copy: copia del descriptor de archivo original de stdin (obtenido al llamar a redirigir_entrada)
void restaurar_entrada(int stdin_copy) {
    // Restaura stdin a su estado original utilizando dup2
    dup2(stdin_copy, STDIN_FILENO);
    // Cierra el descriptor de archivo copiado
    close(stdin_copy);
}

void ejecutarComando(char *comando[]) {
    // Implementa aquí tu propia lógica para ejecutar el comando
    // ...
}

void ejecutarTuberia(char *comando1[], char *comando2[]) {
    int fd[2];
    pid_t pid;

    // Crea la tubería
    pipe(fd);

    // Crea un proceso hijo
    pid = fork();

    if (pid == 0) {
        // Proceso hijo
        // Cierra el descriptor de lectura de la tubería
        close(fd[0]);

        // Redirige la salida estándar hacia el extremo de escritura de la tubería
        dup2(fd[1], STDOUT_FILENO);

        // Ejecuta el primer comando usando tu propia implementación
        ejecutarComando(comando1);
    } 
    else {
        // Proceso padre
        // Espera a que el hijo termine
        wait(NULL);

        // Cierra el descriptor de escritura de la tubería
        close(fd[1]);

        // Redirige la entrada estándar hacia el extremo de lectura de la tubería
        dup2(fd[0], STDIN_FILENO);

        // Ejecuta el segundo comando usando tu propia implementación
        ejecutarComando(comando2);
    }
}