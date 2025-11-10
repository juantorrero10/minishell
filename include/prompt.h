#ifndef PROMPT_H_
#define PROMPT_H_

#include <minishell.h>

void prompt_print_cwd();
void prompt_print_username();
void prompt_print_last_errorcode();
void prompt_print_str(char* s) ;

/*--------------------- CUSTOMIZE SHELL PROMPT -------------------------------- */
#define PROMPT_LABEL "msh> "
#define PROMPT_PRINT() COLOR_RED(stdout);             \
                        STYLE_BOLD(stdout);     \
                    prompt_print_username();  \
                    COLOR_RESET(stdout);    \
                    prompt_print_str(" at "); \
                    COLOR_BRIGHT_BLUE(stdout);            \
                    prompt_print_cwd();      \
                    COLOR_RESET(stdout);        \
                    prompt_print_str(" ");   \
                    prompt_print_str(PROMPT_LABEL)

#endif //PROMPT_H_