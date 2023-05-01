
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
    if(strcmp(current->name,"help")==0){

        if (strlen(current->args[0]) > 0)
        {
            help(current->args[0]);
        }
        else
        {
           help(NULL);
        }
        
        
        return 0;
    }
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
    else if (strcmp(current->name, "true") == 0)
    {
        // Si el comando es un comando true
        return 0;
    }
    else if (strcmp(current->name, "false") == 0)
    {
        // Si el comando es un comando false
        return 1;
    }
    else if (strcmp(current->name, "if") == 0)
    {
        // Si el comando es un comando if
        int exit_status = 0;

        // Ejecutar los comandos entre if y then
        command_t *if_command = current->if_commands;
        while (if_command != NULL)
        {   
            
            
            exit_status = ejecutar_programa(line, if_command, pnumero_de_procesos);
            
            if_command = if_command->next;
            
            
        }

        // Si el valor de retorno es cero, ejecutar los comandos entre then y else o end
        if (exit_status == 0)
        {   
            command_t *then_command = current->then_commands;
            while (then_command != NULL)
            {
                ejecutar_programa(line, then_command, pnumero_de_procesos);
                then_command = then_command->next;
            }
        }
        else
        {
            // Si el valor de retorno no es cero y hay un comando else, ejecutar los comandos entre else y end
            command_t *else_command = current->else_commands;
            while (else_command != NULL)
            {   
                
                
                ejecutar_programa(line, else_command, pnumero_de_procesos);
                else_command = else_command->next;
                
                 
            }
        }
        
        return (exit_status,NULL,NULL);
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
        //fprintf(stderr, "%i\n", current->tuberia);
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
        else if (pid == 0 )
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
                return 1; // No llamar a waitpid después de ejecutar el comando history
            }
            else if (strcmp(current->name, "again") == 0)
            {

                int index = atoi(current->args[1]);

                if (index > 0 && index <= history_length)
                {
                    HIST_ENTRY *entry = history_get(history_base + history_length - index - 1);
                    
                    if (entry)
                    {
                        
                        command_t *command = parse_commands(entry->line);

                        ejecutar_programa(entry->line, command, pnumero_de_procesos);
                    }
                }
            }
            else
            {
                if (execvp(current->name, current->args) == -1)
                {
                    
                    exit(1);
                }
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
void help(char *keyword) {
    
    if (keyword == NULL) {
        
        // Imprimir lista de funcionalidades implementadas
        
        printf("%s \n", "Integrantes: Juan Miguel Pérez Martínez C312 y Amanda Noris Hernández C312");

        printf("%s\n", "Funcionalidades:");
        char *funcionalidades[] = {"basic(3 ptos)", "multipipe(1 pto)", "background(0.5 ptos)", "spaces(0.5 ptos)", "history(0.5 ptos)", "ctrl+c(0.5 ptos)", "if(1 pto)", "help(1 pto)"};
        for (int i = 0; i < 8; i++)
        {
            printf("%s\n", funcionalidades[i]);
        }

        printf("%s\n", "Consideración: El operador ; está implementado, no así los restantes de la funcionalidad chain, que tiene un valor de 0.5 puntos en total");

        printf("%s\n", "Comandos built-in:");
        char *comandos[] = {"cd", "exit", "jobs", "fg", "history", "again", "true", "false", "if", "help"};

        for (int i = 0; i < 10; i++)
        {
            printf("%s\n", comandos[i]);
        }

        printf("%s\n", "Total: 8.2 puntos");

    } else if (strcmp(keyword, "basic") == 0) {
        // Imprimir información sobre la funcionalidad "basic"
        printf("%s\n", "Las funcionalidades básicas con las que cuenta nuestro shell inclyen: la impresión de un prompt al ser ejecutado el programa,la ejecución de comandos, incluyendo entre estos el comando cd que permite el cambio de directorio, la redirección de entrada y salida estándarde comandos hacia/desde ficheros con <, >, >>, redirigir la salida estándar de un comando hacia la entrada de otro mediante el uso de tuberías|, el uso del caracter # para que se ignore todo lo escrito detrás del mismo y la terminación del programa con el comando exit. Tenemos una estructura llamada command que representa un comando. La estructura tiene varios campos para almacenar información sobre el comando, como su nombre (name), sus argumentos (args), el archivo de entrada estándar (stdin_archivo), el archivo de salida estándar (stdout_archivo), su identificador de proceso (pid), si es un comando doble (doble), si tiene una tubería (tuberia), si se ejecuta en segundo plano (background), el número de argumentos (num_args) y punteros a otros comandos relacionados (next, if_commands, then_commands, else_commands). También se muestra una declaración de una función llamada parse_commands(char *input) que toma una cadena de entrada y devuelve un puntero a una estructura command_t. El comando cd cambia el directorio actual del proceso. Primero verifica si se pasó el número correcto de argumentos (2) y si no es así, imprime un mensaje de uso y retorna 1 para indicar un error. Luego intenta cambiar el directorio actual del proceso usando la función chdir(argv[1]). Si hay un error al cambiar el directorio, se imprime un mensaje de error y se retorna 1 para indicar un error. Si todo salió bien, la función retorna 0 para indicar éxito. Nuestro código recorre los argumentos de la cadena de entrada y verifica si son >, < o >>. Si es así, obtiene el siguiente argumento (que debería ser un nombre de archivo) y lo asigna al campo correspondiente en la estructura command_t (stdin_archivo o stdout_archivo). Si el argumento es <, asigna el nombre de archivo al campo stdin_archivo. Si el argumento es if, asigna el nombre del comando y sale del bucle. Si el argumento no es ninguno de los anteriores, se agrega a la lista de argumentos del comando. Finalmente, si el último argumento es <, se establece el campo background en 1 y se elimina el último argumento. La función devuelve un puntero a la estructura command_t construida.");
        printf("%s\n", "Para ignorar todo lo que le sigue a un símbolo de #, nuestro programa primero busca el carácter # en la cadena de entrada usando la función strchr(subsegment, '#'). Si encuentra el carácter #, lo reemplaza con el carácter nulo '\0' para ignorar todo lo que sigue después del #. Luego crea una copia de la cadena de entrada modificada usando la función strdup(subsegment) y la almacena en la variable subsegment1. Finalmente, llama a la función parse_command(subsegment) para analizar la cadena de entrada y construir una estructura command_t a partir de ella. La función devuelve un puntero a la estructura command_t construida.");

    } else if (strcmp(keyword, "multipipe") == 0) {
        // Imprimir información sobre la funcionalidad "multipipe"
        printf("%s\n","La funcionalidad multipipe permite el uso de varias tuberías para la redirección de salida estándar de un comando hacia la entrada de otro en forma de cadena. Un ejemplo de esta funcionalidad es la cadena de comandos ls -l /usr/bin | grep zip | wc -l ");

    } else if (strcmp(keyword, "background") == 0) {
        // Imprimir información sobre la funcionalidad "background"
        printf("%s\n", "Nuestro shell también tiene implementada una funcionalidad de background. Mediante el comando jobs se obtienen todos los procesos que estan corriendo en el background. El comando fg <pid> envía el proceso <pid> hacia el foreground, mientras que fg (sin parámetros) envía el proceso más reciente que fue al background, hacia el foreground.Si el nombre del comando es jobs, se llama a la función jobs() y la función devuelve NULL. Si el nombre del comando es fg, se obtiene un pid (identificador de proceso) ya sea del primer argumento del comando o del primer proceso en segundo plano. Luego se llama a la función fg(pid) y la función devuelve NULL. La función jobs() recorre una lista de procesos en segundo plano (bg_processes) e imprime sus nombres . La función fg(int indice) recorre la misma lista de procesos en segundo plano y busca un proceso con un índice específico. Cuando encuentra el proceso, mueve sus comandos a la lista de procesos en ejecución (procesos_en_ejecucion) y luego lo elimina de la lista de procesos en segundo plano. Finalmente, llama a la función waitmany(current->comandos) para esperar a que los comandos del proceso terminen.");

    } else if (strcmp(keyword, "spaces") == 0) {
        // Imprimir información sobre la funcionalidad "spaces"
        printf("%s\n", "Para trabajar con mayor comodidad implementamos la funcionaidad spaces que permite que se coloquen cualquier cantidad de espacios entre comando y parámetros.");

    } else if (strcmp(keyword, "history") == 0) {
        // Imprimir información sobre la funcionalidad "history"
        printf("%s\n", "La funcionalidad history nos brinda un historial de los 10 últimos comandos ejecutados enumerados desde el más reciente(1) hasta el más antiguo(10). Además el comando again<number> ejecuta nuevamente el comando que posee ese número en el historial. ");
        printf("%s\n", "");
        printf("%s\n","Se implementa el comando history dentro de un bloque else if  en el archivo ejecutarPrograma.c que se ejecuta si el nombre del comando ingresado por el usuario es igual a “history”. Dentro del bloque else if, se utiliza la función history_list para obtener una lista de las entradas en el historial de comandos. Luego, se verifica si la lista no es NULL y, en caso afirmativo, se ejecuta un bucle while para contar el número de entradas en el historial. Después, se calcula el índice de inicio (start) para imprimir las últimas 10 entradas del historial. Si hay menos de 10 entradas en el historial, se ajusta el valor de start para que sea 0. Luego, se ejecuta un bucle for para imprimir las últimas 10 entradas del historial en orden inverso. Cada entrada se imprime con un número delante y la línea de comando correspondiente. Finalmente, la función retorna 1 para indicar que no se debe llamar a waitpid después de ejecutar el comando history.");
        printf("%s\n", "");
        printf("%s\n", "Se implementa el comando again dentro de un bloque else if que se ejecuta si el nombre del comando ingresado por el usuario es igual a “again”. Dentro del bloque else if, se utiliza la función atoi para convertir el segundo argumento del comando (current->args[1]) en un número entero (index). Luego, se verifica si index está dentro del rango válido de índices para el historial de comandos. Si index es válido, se utiliza la función history_get para obtener la entrada del historial correspondiente al índice especificado. Luego, se verifica si la entrada no es NULL y, en caso afirmativo, se ejecuta el siguiente código: Se utiliza la función parse_commands para analizar la línea de comando de la entrada del historial y crear una estructura command_t. Se llama a la función ejecutar_programa para ejecutar el comando representado por la estructura command_t.");

    } else if (strcmp(keyword, "ctrl+c") == 0) {
        // Imprimir información sobre la funcionalidad "ctrl+c"
        printf("%s\n", "El comando ctrl+c mata, por decirlo de alguna manera, al proceso que se encuentra ejecutándose. Esto lo hacemos mediante la función sigint_handler del archivo shell..c que se llama luego en el main(). Esta función maneja la señal SIGINT en un programa en C. Cuando el usuario presiona Ctrl+C, se incrementa un contador de Ctrl+C. Si hay procesos en ejecución, la función envía la señal SIGINT a todos los procesos en ejecución. Si el usuario presiona Ctrl+C más de una vez, la función envía la señal SIGKILL a todos los procesos en ejecución en lugar de SIGINT.");

    } else if (strcmp(keyword, "if") == 0) {
        // Imprimir información sobre la funcionalidad "if"
        printf("%s\n", "La funcionalidad if permite que en una sola línea se pueda realizar una operación condicional donde siempre aparecerán los operadores if then y end (el comando else es opcional en la expresión). Entre el if y el then aparece un comando (o cadena de comandos) y la expresión evalúa TRUE si el valor de retorno (exit status) es cero. Solo si la expresión del if es TRUE se evalúa el comando que está despues del then. Si aparece un else entonces se ejecuta el comando (o cadena de comandos) que aparece despues del else sólo en caso de que la expresión  condicional haya sido FALSE(exit status distinto de cero).");
        printf("%s\n", "");
        printf("%s\n", "Se implementa el comando if dentro de un bloque else if que se ejecuta si el nombre del comando ingresado por el usuario es igual a “if”. Dentro del bloque else if, se declara una variable llamada exit_status y se le asigna el valor 0. Luego, se ejecuta un bucle while para recorrer la lista enlazada de comandos entre if y then. Dentro del bucle, se llama a la función ejecutar_programa para ejecutar cada comando y se actualiza el valor de exit_status con el valor de retorno de la función. Después del bucle while, se verifica si exit_status es igual a 0. Si es así, se ejecuta otro bucle while para recorrer la lista enlazada de comandos entre then y else o end. Dentro del bucle, se llama a la función ejecutar_programa para ejecutar cada comando. Si exit_status no es igual a 0, entonces se verifica si hay un comando else. Si es así, se ejecuta otro bucle while para recorrer la lista enlazada de comandos entre else y end. Dentro del bucle, se llama a la función ejecutar_programa para ejecutar cada comando. Finalmente, la función retorna el valor de exit_status.");

    } else if (strcmp(keyword, "help") == 0) {
        // Imprimir información sobre la funcionalidad "help"
        printf("%s\n", "El comando help imprime una ayuda al usuario sobre cómo funciona el shell y cuáles son las funcionalidades que posee. La función help está implementada en el archivo ejecutarPrograma.c y recibe un argumento llamado keyword. Dentro de la función, hay una estructura condicional if que verifica si keyword es igual a NULL o no. Si keyword es igual a NULL, entonces se ejecuta el primer bloque de código dentro de la estructura condicional if. Este bloque de código imprime una lista de funcionalidades y comandos implementados en nuestro shell. Si keyword no es igual a NULL, entonces se ejecuta el segundo bloque de código dentro de la estructura condicional if. Este bloque de código verifica si keyword es igual al nombre de alguna funcionalidad y, en caso afirmativo, imprime información sobre la funcionalidad .");

    }
    
}
///////////////////////////////////////////////////////////////////
