#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include <getopt.h>


/*coger el comando de la pantalla statica para al terminar de leer no perderla*/

char *leer(){
    static char comando[1000];
    char *aux;
    aux=fgets(comando,1000,stdin);
    if(aux==NULL){
        printf("exit");
        exit(0);
    }
    else return aux;
    
} 
