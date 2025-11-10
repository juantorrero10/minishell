#ifndef PROMPT_H_
#define PROMPT_H_

#include <minishell.h>

void prompt_print_cwd(bool abrv_home);
void prompt_print_username();
void prompt_print_last_errorcode();
void prompt_print_str(char* s) ;

/*--------------------- PERSONALIZAR PROMPT -------------------------------- */

#define PROMPT_LABEL "msh> "

/**
 *  Para personalizar el prompt se debe modificar este macro con
 *  la informacion que se desea que contenga. He dejado un ejemplo 
 *  para hacerse a la idea.
 * 
 *  Para cambiar el color y estilo de la fuente use las funciones 
 *  definidas en `macros.h` para ello.
 */
#define PROMPT_PRINT()  COLOR_RED(stdout);              \
                        STYLE_BOLD(stdout);             \
                        prompt_print_username();        \
                        COLOR_RESET(stdout);            \
                        prompt_print_str(" at ");       \
                        COLOR_BRIGHT_BLUE(stdout);      \
                        prompt_print_cwd(1);            \
                        COLOR_RESET(stdout);            \
                        prompt_print_str(" ");          \
                        prompt_print_str(PROMPT_LABEL)

#endif //PROMPT_H_