#include <minishell.h>



void get_line_input(char* buff, size_t max) {
    PROMPT_PRINT();
    fgets(buff, max, stdin);
}