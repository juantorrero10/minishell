#include <minishell.h>
#include <log.h>

int read_line_input(char* buff, size_t max) {
    size_t buff_len = 0;
    tline* line = NULL;
    int ret = 0;

    PROMPT_PRINT();
    fgets(buff, max, stdin);

    /** Si hay un caracter '\n' al final significa que el usuario 
    *   ha querido ejecutar este commando.
    * 
    *   Si no lo hay se puede haber producido un SIGINT y se debe 
    *   descartar lo que hay escrito.
    */

    buff_len = strlen(buff);
    if (buff[buff_len - 1] == '\n') {
        buff[buff_len-- - 1] = '\0';

        line = tokenize(buff);
        if (line != NULL) {
            ret = execute_command(line);
            g_last_error_code = ret;
            return ret;
        }
        return -1;
    } else {
        fputc('\n', stdout);
        return 0;
    }
    
}