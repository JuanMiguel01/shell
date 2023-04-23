#include "prompt.h"
#include <stdio.h>
#include "parser.h"
#include "ejecutarPrograma.h"
int main() {
    char comando[1000];
    while (1) {
        imprimirPrompt();
        fgets(comando, 1000, stdin);
        
        if (strcmp(comando, "exit\n") == 0) {
            break;
        }
        
        // Llamar a la función parser con el comando ingresado por el usuario como parámetro
        command_t *comandos = parser(comando);
        ejecutar_programa(comandos);
    }
    return 0;
}