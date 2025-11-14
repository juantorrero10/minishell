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

int read_line_input(char* buff, size_t max) {
    size_t buff_len = 0;
    size_t remaining = 0;
    size_t total = 0;
    tline* line = NULL;
    tline* expanded = NULL;
    char buff2[INPUT_LINE_MAX];
    char t; (void)t;
    int ret = 0;

    // Imprimir el "prompt"
    PROMPT_PRINT();
    fgets(buff, max, stdin);

    /** Si hay un caracter '\n' al final significa que el usuario 
    *   ha querido ejecutar este commando.
    * 
    *   Si no lo hay se puede haber producido un SIGINT y se debe 
    *   descartar lo que hay escrito.
    */

    buff_len = strlen(buff);
    remaining= INPUT_LINE_MAX - (buff_len + 1);
    if (buff[buff_len - 1] == '\n') {
        buff[buff_len - 1] = '\0';
        buff_len--;
        total = buff_len;
        remaining++;
        // Permitir varias lineas con '\'
        while (buff[total - 1] == '\\' && remaining > 0) {
            COLOR_GREY(stdout);
            fprintf(stdout, ">\t\t\t\t");
            COLOR_RESET(stdout);
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

        line = tokenize(buff);
        if (line != NULL) {
            expanded = env_expand_wholeline(line);
            ret = execute_command(expanded, buff);
            // Actualizar el último codigo de error.
            g_last_error_code = ret;

            free_tokens(expanded);
            free(expanded);
            if (g_exit_signal == 1) {
                INFO("exit signal=1");
                exit(ret);
            }
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