#include <minishell.h>
#include <log.h>

/**
 * Esta funcion, solo se encarga de liberar las copias, 
 * no el que se crea con tokenize() porque la libreria ya se encarga de él.
 */
void free_tokens(tline* t) {
    if (!t) return;
    if (t->redirect_error) free(t->redirect_error);
    if (t->redirect_input) free(t->redirect_input);
    if (t->redirect_output) free(t->redirect_output);
    for (int i = 0; i < t->ncommands; i++)
    {
        for (int j = 0; j < t->commands[i].argc; j++)
        {
            if (t->commands[i].argv[j]) free(t->commands[i].argv[j]);
        }
        if (t->commands[i].argv) free(t->commands[i].argv);
        if (t->commands[i].filename) free(t->commands[i].filename);
        
    }free(t->commands);
}

/**
 * @brief Junta argumentos con entre comillas.
 * @param line Linea de comandos tokenizada.
 * Es necesario que se ejecute despues de la expansión de variables.
 */
static void join_quoted_arguments(tline* line) {
    size_t len_curr = 0;
    size_t bufflen_new = 0;
    char* buff;
    char* ptr;
    int argc;
    int new_idx = 0;
    int k = 0;

    // Iterar en cada comando
    for (int i = 0; i < line->ncommands; i++)
    {
        argc = line->commands[i].argc;
        new_idx = 0;
        // Iterar en cada argumento
        for (int j = 0; j < line->commands[i].argc; j++)
        {
            bufflen_new = 0;
            buff = NULL;
            if (line->commands[i].argv[j][0] == '\"') {
                k = j;
                //Saltar hasta el commando con el cierre de commillas.
                while (line->commands[i].argv[j][strlen(line->commands[i].argv[j]) - 1] != '\"' 
                    && j < line->commands[i].argc) {
                    j++;
                }
                // Unir los argumentos k, ..., j
                for (int l = 0; l < (j - k + 1); l++)
                {
                    bufflen_new += strlen(line->commands[i].argv[k + l]) + 1;
                }
                // Quitar las comillas.
                bufflen_new -= 2;
                buff = malloc(bufflen_new + 1);
                buff[0] = '\0';
                for (int l = 0; l < (j - k + 1); l++)
                {
                    ptr = line->commands[i].argv[k + l];
                    len_curr = strlen(ptr);
                    // Si es el último no copiar " del final
                    if(l == (j - k)) len_curr--;
                    // Si es el primero saltar " del inicio
                    if (l == 0) {ptr++; len_curr--;}
                    // Añadir espacio.
                    if (l != 0) {
                        strcat(buff, " ");
                    }
                    strncat(buff, ptr, len_curr);
                    free(line->commands[i].argv[k + l]);
                }
                // Restar al numero de argumentos
                buff[bufflen_new] = '\0';
                argc -= (j - k);
                
            }
            if (buff)line->commands[i].argv[new_idx] = buff;
            new_idx++;
        }
        // Eliminar argumentos sobrantes
        for (int j = 0; j < (line->commands[i].argc - argc); j++)
        {
            line->commands[i].argv[new_idx + j] = NULL;
        }
        
        line->commands[i].argc = argc;
        
    }
    
}

int read_line_input(char* buff, size_t max, bool print_prompt) {
    size_t buff_len = 0;
    size_t remaining = 0;
    size_t total = 0;
    tline* line = NULL;
    tline* expanded = NULL;
    char buff2[INPUT_LINE_MAX];
    char t; (void)t;
    int ret = 0;

    // Imprimir el "prompt"
    if (print_prompt) {PROMPT_PRINT();}
    fgets(buff, max, stdin);

    /** Si hay un caracter '\n' al final significa que el usuario 
    *   ha querido ejecutar este commando.
    * 
    *   Si no lo hay se puede haber producido un SIGINT o SIGTSTP y se 
    *   descarta lo que hay escrito.
    */

    buff_len = strlen(buff);
    remaining= INPUT_LINE_MAX - (buff_len + 1);
    // Ignorar lineas que no acaban en '\n' o que empiezan con '#' (comentarios)
    if (buff[buff_len - 1] == '\n' && buff[0] != '#') {
        buff[buff_len - 1] = '\0';
        buff_len--;
        total = buff_len;
        remaining++;
        
        // Permitir varias lineas con '\'
        while (buff[total - 1] == '\\' && remaining > 0) {
            if (print_prompt) {
                COLOR_GREY(stdout);
                fprintf(stdout, ">\t\t\t\t");
                COLOR_RESET(stdout);
            }
            // Eliminar la barra invertida
            buff[total - 1] = '\0';
            buff_len--; total--; remaining++;
            // Obtener siguiente linea
            fgets(buff2, remaining, stdin);
            buff_len = strlen(buff2);
            
            if (buff2[buff_len - 1] == '\n') {
                buff2[buff_len - 1] = '\0';
                buff_len--;
                strcat(buff, buff2);
                memset(buff2, 0, remaining);
                remaining -= buff_len;
                total += buff_len;
            } else {    //un SIGINT
                fputc('\n', stdout);
                return 0;
            }
        }

        // tokenizar
        line = tokenize(buff);
        if (line != NULL) {

            // expandir variables de entorno y juntar argumentos entre comillas
            expanded = env_expand_wholeline(line);
            join_quoted_arguments(expanded);

            // ejecutar commando.
            ret = execute_command(expanded, buff);
            
            // Actualizar el último codigo de error. $STATUS
            g_last_error_code = ret;
            sprintf(buff, "%d", ret);
            setenv("STATUS", buff, 1);

            free_tokens(expanded);
            free(expanded);

            // señal de salida = 1 -> salir de la shell con código ret.
            if (g_exit_signal == 1) {
                INFO("exit signal=1");
                exit(ret);
            }
            // señal de salida = 2 -> se ha cancelado el diálogo de salida.
            if (g_exit_signal == 2) {
                fputc('\n', stdout);
                g_exit_signal = 0;
                INFO("end, signal=2");
                return ret;
            }
            INFO("end");
            return ret;
        }
        return -1;
    } else {
        fputc('\n', stdout);
        return 0;
    }
}