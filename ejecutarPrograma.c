
#include "comandos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include "parser.h"
#include "ejecutarPrograma.h"
#include <readline/history.h>

static void dup2_s(int old, int new)
{
    if (old != new)
    {
        dup2(old, new);
        close(old);
    }
}

static void waitmany(command_t *command)
{
    command_t *current;
    while (1)
    {
    startover:
        current = command;

        while (current != NULL)
        {
            int status;
            if (current->pid > 0)
                if (waitpid(current->pid, &status, 0) < 0)
                    perror("waitpid");
                else
                {
                    if (WIFEXITED(status) || WIFSIGNALED(status))
                        current->pid = -1;
                    else
                        goto startover;
                }
            current = current->next;
        }
        break;
    }
}

commandlist_t *bg_processes = NULL;
extern pid_t *procesos_en_ejecucion;
int *ejecutar_programa(char *line, command_t *comando, int *pnumero_de_procesos)
{

    command_t *current = comando;
    command_t *next;
    if (strcmp(current->name, "jobs") == 0)
    {
        jobs();
        return NULL;
    }
    else if (strcmp(current->name, "fg") == 0)
    {
        int pid;
        if (current->num_args == 1 && bg_processes != NULL)
        {
            pid = bg_processes->indice;
        }
        else
        {
            pid = atoi(current->args[1]);
        }
        fg(pid);
        return NULL;
    }
    int stdout_copy = -1;
    int stdin_copy = -1;

    stdin_copy = dup(STDIN_FILENO);
    stdout_copy = dup(STDOUT_FILENO);

    pid_t *procesos_en_ejecucion = NULL;
    int numero_de_procesos = 0;
    while (current != NULL)
    {
        next = current->next;
        fprintf(stderr, "%i\n", current->tuberia);
        if (current->stdout_archivo != NULL)
        {
            // Redirigir la salida estándar al archivo especificado
            redirigir_salida(current->stdout_archivo, current->doble);
        }
        if (current->stdin_archivo != NULL)
        {
            // Redirigir la entrada estándar desde el archivo especificado
            redirigir_entrada(current->stdin_archivo);
        }

        int extremos[2] = {
            STDIN_FILENO,
            STDOUT_FILENO,
        };

        if (current->tuberia)
        {
            if (pipe(extremos) < 0)
                perror("pipe");
        }

        dup2_s(extremos[1], STDOUT_FILENO);

        int pid = fork();

        if (pid < 0)
            perror("fork");
        else if (pid == 0)
        {

            // Llamada al método cd
            if (strcmp(current->name, "cd") == 0)
            {
                cd(current->num_args, current->args);
            }
            else if (strcmp(current->name, "history") == 0)
            {
                // Implementar el comando history aquí
                HIST_ENTRY **list = history_list();
                if (list)
                {
                    int i = 0;
                    while (list[i])
                    {
                        i++;
                    }
                    int start = i - 10;
                    if (start < 0)
                    {
                        start = 0;
                    }
                    int numerador = 1;
                    for (int j = i - 1; j >= start; j--)
                    {
                        printf("%d: %s\n", numerador++, list[j]->line);
                        if (j == start)
                        {
                            numerador = 1;
                        }
                    }
                }
                return 0; // No llamar a waitpid después de ejecutar el comando history
            }
            else if (strcmp(current->name, "again") == 0)
            {

                int index = atoi(current->args[1]);

                if (index > 0 && index <= history_length)
                {
                    HIST_ENTRY *entry = history_get(history_base + history_length - index - 1);
                    printf("%s\n", entry->line);
                    if (entry)
                    {
                        printf("%s\n", entry->line);
                        command_t *command = parse_commands(entry->line);

                        ejecutar_programa(entry->line, command, pnumero_de_procesos);
                    }
                }
            }
            else
            {
                execvp(current->name, current->args);
            }
        }
        else
        {
            if (!current->background)
            {
                current->pid = pid;
                numero_de_procesos++;
                procesos_en_ejecucion = realloc(procesos_en_ejecucion, numero_de_procesos * sizeof(pid_t));
                procesos_en_ejecucion[numero_de_procesos - 1] = pid;
            }
        }

        dup2(stdout_copy, STDOUT_FILENO);
        dup2_s(extremos[0], STDIN_FILENO);
        current = next;
    }

    dup2_s(stdout_copy, STDOUT_FILENO);
    dup2_s(stdin_copy, STDIN_FILENO);
    if (comando->background)
    {
        commandlist_t *new_commandlist = malloc(sizeof(commandlist_t));
        if (new_commandlist == NULL)
        {
            perror("comandlist NULL");
        }
        else
        {
            static int next_indice = 0;
            new_commandlist->comandos = comando;
            new_commandlist->indice = ++next_indice;
            new_commandlist->line = line;
            new_commandlist->next = bg_processes;
            bg_processes = new_commandlist;
            *pnumero_de_procesos = 0;
            return NULL;
        }
    }
    waitmany(comando);
    *pnumero_de_procesos = pnumero_de_procesos;
    return procesos_en_ejecucion;
}

// si tiene tuberia de escritura creo una tuberia
void jobs()
{
    commandlist_t *current = bg_processes;
    while (current != NULL)
    {
        printf("[%d] %s\n", current->indice, current->line);
        current = current->next;
    }
}
void fg(int indice)
{
    commandlist_t *current = bg_processes;
    commandlist_t *prev = NULL;
    while (current != NULL)
    {
        if (current->indice == indice)
        {
            // move processes from current->comandos to procesos_en_ejecucion
            command_t *cmd = current->comandos;
            procesos_en_ejecucion = NULL;
            int i = 0;
            while (cmd != NULL)
            {
                procesos_en_ejecucion = realloc(procesos_en_ejecucion, (i + 1) * sizeof(pid_t));
                procesos_en_ejecucion[i] = cmd->pid;
                i++;
                cmd = cmd->next;
            }

            // remove current from bg_processes list
            if (prev == NULL)
            {
                bg_processes = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            
            waitmany(current->comandos);
            break;
        }
        prev = current;
        current = current->next;
    }
}