
#include "prompt.h" 
#include <stdio.h>

int main() {
    char comando[1000];
    while (1) {
        imprimirPrompt();
        fgets(comando, 1000, stdin);
        if (strcmp(comando, "exit\n") == 0) {
            break;
        }
        // Aquí puedes procesar el comando ingresado por el usuario
    }
    return 0;

}