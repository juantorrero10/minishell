#include <minishell.h>
#include <log.h>

void read_line_input(char* buff, size_t max) {
    PROMPT_PRINT();
    size_t buff_len = 0;
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
        INFO("%s", buff);
        //execute_command(buff)
    } else {
        fputc('\n', stdout);
    }
    
}