
#include "comandos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include "parser.h"

static void dup2_s (int old, int new)
{
    if (old != new)
    {
        dup2 (old, new);
        close (old);
    }
}


int ejecutar_programa(command_t *comando) {
    command_t *current = comando;
    int stdout_copy = -1;
    int stdin_copy = -1;

    stdin_copy = dup (STDIN_FILENO);
    stdout_copy = dup (STDOUT_FILENO);

    while (current != NULL)
    {
        fprintf (stderr, "%i\n", current->tuberia);
        if (current->stdout_archivo != NULL) {
            // Redirigir la salida estándar al archivo especificado
            redirigir_salida (current->stdout_archivo, current->doble);
        }
        if (current->stdin_archivo != NULL) {
            // Redirigir la entrada estándar desde el archivo especificado
            redirigir_entrada (current->stdin_archivo);
        }

        int extremos [2] = { STDIN_FILENO, STDOUT_FILENO, };

        if(current->tuberia)
        {
            if (pipe (extremos) < 0)
                perror ("pipe");
        }
 
        dup2_s (extremos [1], STDOUT_FILENO);

        int pid = fork ();

        if (pid < 0)
            perror ("fork");
        else if (pid == 0)
        {
            // Llamada al método cd
            if (strcmp(current->name, "cd") == 0) {
                cd(current->num_args, current->args);
            } else {
                execvp (current->name, current->args);
            }
        } else current->pid = pid;

        dup2 (stdout_copy, STDOUT_FILENO);
        dup2_s (extremos [0], STDIN_FILENO);
        current = current->next;
    }

    dup2_s (stdout_copy, STDOUT_FILENO);
    dup2_s (stdin_copy, STDIN_FILENO);

    while (1)
    {
    startover:
        current = comando;

        while (current != NULL)
        {
            int status;
            if (current->pid > 0)
            if (waitpid (current->pid, &status, 0) < 0)
                perror ("waitpid");
            else
            {
                if (WIFEXITED (status) || WIFSIGNALED (status))
                    current->pid = -1;
                else goto startover;
            }
            current = current->next;
        }
        break;
    }
return 0;
}

//si tiene tuberia de escritura creo una tuberia 