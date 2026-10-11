#ifndef PROMPT_H_
#define PROMPT_H_

#include <minishell.h>

extern int g_last_error_code;

char* prompt_get();
void prompt_append_cwd(bool abrv_home);
void prompt_append_username();
void prompt_append_last_errorcode();
void prompt_append_str(char* s);
void prompt_append_hostname();
void prompt_append_git_branch();
void prompt_clear();

// COLOR 
#define PROMPT_COLOR_RESET(stream)    prompt_append_str("\001\033[0m\002")
#define PROMPT_STYLE_BOLD(stream)     prompt_append_str("\001\033[1m\002")
#define PROMPT_STYLE_ITALIC(stream)   prompt_append_str("\001\033[3m\002")
#define PROMPT_STYLE_UNDERLINE(stream)   prompt_append_str("\001\033[4m\002")

#define PROMPT_COLOR_BLACK(stream)       prompt_append_str("\001\033[30m\002")
#define PROMPT_COLOR_RED(stream)         prompt_append_str("\001\033[31m\002")
#define PROMPT_COLOR_GREEN(stream)       prompt_append_str("\001\033[32m\002")
#define PROMPT_COLOR_YELLOW(stream)      prompt_append_str("\001\033[33m\002")
#define PROMPT_COLOR_BLUE(stream)        prompt_append_str("\001\033[34m\002")
#define PROMPT_COLOR_MAGENTA(stream)     prompt_append_str("\001\033[35m\002")
#define PROMPT_COLOR_CYAN(stream)        prompt_append_str("\001\033[36m\002")
#define PROMPT_COLOR_WHITE(stream)       prompt_append_str("\001\033[37m\002")
#define PROMPT_COLOR_GREY(stream)         prompt_append_str("\001\033[90m\002")


#define PROMPT_COLOR_BRIGHT_RED(stream)     prompt_append_str("\001\033[91m\002")
#define PROMPT_COLOR_BRIGHT_GREEN(stream)   prompt_append_str("\001\033[92m\002")
#define PROMPT_COLOR_BRIGHT_YELLOW(stream)  prompt_append_str("\001\033[93m\002")
#define PROMPT_COLOR_BRIGHT_BLUE(stream)    prompt_append_str("\001\033[94m\002")
#define PROMPT_COLOR_BRIGHT_MAGENTA(stream) prompt_append_str("\001\033[95m\002")
#define PROMPT_COLOR_BRIGHT_CYAN(stream)    prompt_append_str("\001\033[96m\002")
#define PROMPT_COLOR_BRIGHT_WHITE(stream)   prompt_append_str("\001\033[97m\002")

/*--------------------- PERSONALIZAR PROMPT -------------------------------- */

#define PROMPT_LABEL "msh> "

#define LAST_ERROR() if(g_last_error_code == EXIT_SUCCESS)     \
                            PROMPT_COLOR_BRIGHT_BLUE(stdout);                         \
                        else {                                           \
                            PROMPT_COLOR_RED(stdout);                           \
                            PROMPT_STYLE_ITALIC(stdout);                        \
                            prompt_append_last_errorcode();               \
                            prompt_append_str(" ");                       \
                            PROMPT_COLOR_RESET(stdout); PROMPT_COLOR_RED(stdout);  }   \
                    prompt_append_str("» ");
                    
/**
 *  Para personalizar el prompt se debe modificar este macro con
 *  la informacion que se desea que contenga. He dejado un ejemplo 
 *  para hacerse a la idea.
 * 
 *  Para cambiar el color y estilo de la fuente use las funciones 
 *  definidas en `macros.h` para ello.
 * 
 * 
 */

#define DEFAULT_PROMPT_PUSH()                                                      \
    /* codigo de error */                                                \
    LAST_ERROR();                                                           \
                                                                             \
    /* usuario */                                                     \
    PROMPT_COLOR_MAGENTA(stdout);                                           \
    PROMPT_STYLE_BOLD(stdout);                                                     \
    prompt_append_username();                                                \
    PROMPT_COLOR_RESET(stdout);                                                    \
    prompt_append_str(" ");                                                  \
    PROMPT_COLOR_BRIGHT_CYAN(stdout);                                              \
    prompt_append_cwd(true);                                                 \
    prompt_append_str(" ");                                                  \
    PROMPT_COLOR_GREEN(stdout);                                            \
    PROMPT_STYLE_ITALIC(stdout);                                                    \
    prompt_append_git_branch();                                              \
    prompt_append_str(" ");                                                  \
    PROMPT_COLOR_RESET(stdout);                                                    \
                                                                             \
    /* Final arrow */                                                       \
    PROMPT_STYLE_BOLD(stdout);                                                     \
    prompt_append_str("msh> ");                                                 \
    PROMPT_COLOR_RESET(stdout)

/*
#define DEFAULT_PROMPT_PUSH()  LAST_ERROR();                   \
                        COLOR_BRIGHT_YELLOW(stdout);    \
                        STYLE_ITALIC(stdout);             \
                        prompt_append_username();        \
                        COLOR_RESET(stdout);            \
                        prompt_append_str(" at ");       \
                        COLOR_CYAN(stdout);      \
                        prompt_append_cwd(1);            \
                        COLOR_RESET(stdout);            \
                        prompt_append_str(" ");          \
                        prompt_append_str(PROMPT_LABEL)
*/

#endif //PROMPT_H_