#include "prompt.h"
#include <stdio.h>
#include "parser.h"

#include "ejecutarPrograma.h"
int main() {
    char *comando;
    while (1) {
        
        comando=Prompt();
        
        if (strcmp(comando, "exit\n") == 0) {
            break;
        }
        
        // Llamar a la función parser con el comando ingresado por el usuario como parámetro
        command_t *comandos = parse_commands(comando);
        ejecutar_programa(comandos);
    }
    return 0;
}