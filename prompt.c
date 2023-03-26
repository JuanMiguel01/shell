#include<stdio.h>
#include<string.h>
#include<errno.h>


void imprimirPrompt(){
    
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
    char dirActual[1000];
    char *aux2=getcwd(dirActual,1000);
    if(aux2==NULL){
        perror("No se pudo obtener el directorio actual");
        error=1;
    }
    if(!error){
        /*coger el nombre de la carpeta actual*/
        int i=0;
        char aux;
        int ultimaBarra=0;
        while((aux=dirActual[i])!=0){
            if(aux=='/'){
                ultimaBarra=i;
            }
            i++;
        }
        char carpetaActual[1000];
        int j=0;
        for(i=ultimaBarra+1;i<strlen(dirActual);i++){
            carpetaActual[j]=dirActual[i];
            j++;
        }
        carpetaActual[j]='\0';
        /*imprimir el prompt*/
        printf("%s@%s:%s$ ",login,nombreHost,carpetaActual);

    }
    else{
        
        printf("$ ");
    }
}