#ifndef PROMPT_H_
#define PROMPT_H_

#include <minishell.h>

extern int g_last_error_code;

void prompt_print_cwd(bool abrv_home);
void prompt_print_username();
void prompt_print_last_errorcode();
void prompt_print_str(char* s);

int prompt_get_last_errorcode();

/*--------------------- PERSONALIZAR PROMPT -------------------------------- */

#define PROMPT_LABEL "msh> "

#define LAST_ERROR() if(prompt_get_last_errorcode() == EXIT_SUCCESS)     \
                            COLOR_GREEN(stdout);                         \
                        else {                                           \
                            COLOR_RED(stdout);                           \
                            STYLE_ITALIC(stdout);                        \
                            prompt_print_last_errorcode();               \
                            prompt_print_str(" ");                       \
                            COLOR_RESET(stdout); COLOR_RED(stdout);  }   \
                    prompt_print_str("» "); 

/**
 *  Para personalizar el prompt se debe modificar este macro con
 *  la informacion que se desea que contenga. He dejado un ejemplo 
 *  para hacerse a la idea.
 * 
 *  Para cambiar el color y estilo de la fuente use las funciones 
 *  definidas en `macros.h` para ello.
 */
#define PROMPT_PRINT()  LAST_ERROR();                   \
                        COLOR_YELLOW(stdout);           \
                        STYLE_ITALIC(stdout);             \
                        prompt_print_username();        \
                        COLOR_RESET(stdout);            \
                        prompt_print_str(" at ");       \
                        COLOR_BRIGHT_BLUE(stdout);      \
                        prompt_print_cwd(1);            \
                        COLOR_RESET(stdout);            \
                        prompt_print_str(" ");          \
                        prompt_print_str(PROMPT_LABEL)

#endif //PROMPT_H_