#include<stdio.h>
#include<string.h>
#include<errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/readline.h>
char *Prompt(){
    
    int error=0;
    /*primero coger el login del usuario y si falla aumentar la variable error*/
    char *login=getenv("USER");
    if(login==NULL){
        printf("No se pudo obtener el login del usuario");
        error=1;
    }
    
    /*coger el nombre del host actual */
    char nombreHost[256];
    int aux=gethostname(nombreHost,256);
    if(aux==-1){
        perror("No se pudo obtener el host del usuario");
        error=1;
    }
    /*coger el nombre del directorio actual*/
    char dirActual[1024];
    char *aux2=getcwd(dirActual,1000);
    if(aux2==NULL){
        perror("No se pudo obtener el directorio actual");
        error=1;
    }
    if(!error){
        
        char result[1024];
        sprintf(result, "%s@%s~%s$ ", login, nombreHost, dirActual);
        return readline(result);
       
    }
    else{
        
        return readline(NULL);
    }
}