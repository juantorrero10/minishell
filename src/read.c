#include <minishell.h>
#include <log.h>

#include <parser/public.h>

int read_line_input(char* buff, size_t max, bool print_prompt) {
    size_t buff_len = 0;
    size_t remaining = 0;
    size_t total = 0;
    char buff2[INPUT_LINE_MAX];
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
                M_COLOR_GREY(stdout);
                fprintf(stdout, ">\t\t\t\t");
                M_COLOR_RESET(stdout);
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
        //line = tokenize(buff);
        ast_t* a = parse_string(buff);
        if (a != NULL) {

            // expandir variables de entorno y juntar argumentos entre comillas
            // expanded = env_expand_wholeline(line);
            // join_quoted_arguments(expanded);

            // ejecutar commando.
            ret = execute_line(a, buff);
            
            // Actualizar el último codigo de error. $STATUS
            g_last_error_code = ret;
            setenv("PREV", buff, 1);
            sprintf(buff, "%d", ret);
            setenv("STATUS", buff, 1);
            

            ast_free(a);

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