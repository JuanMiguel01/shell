#ifndef FUNCIONES_H
#define FUNCIONES_H

int cd(int argc, char **argv);
int redirigir_salida(char *filename, int append);
void restaurar_salida(int stdout_copy);
int redirigir_entrada(char *filename);
void restaurar_entrada(int stdin_copy);
void ejecutarComando(char *comando[]);
void ejecutarTuberia(char *comando1[], char *comando2[]);
void ls() ;
char *pwd() ;
#endif