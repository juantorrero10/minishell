#ifndef COMMAND_EXECUTE_H_
#define COMMAND_EXECUTE_H_

#include <colors.h>

#define MSH_LOG(MSG, ...)  fprintf(stdout, "%sminishell: %s"MSG"\r\n", COLOR_BRIGHT_BLUE, COLOR_RESET, ##__VA_ARGS__)

#define MSH_ERR(MSG, ...) fprintf(stderr, "%s%sminishell: %s"MSG"\r\n", STYLE_BOLD, COLOR_BRIGHT_RED, COLOR_RESET, ##__VA_ARGS__)

#define MSH_LOG_C(MSG, ...) fprintf(fss.out, "%sminishell: %s"MSG"\r\n", COLOR_BRIGHT_BLUE, COLOR_RESET, ##__VA_ARGS__)

#define MSH_ERR_C(MSG, ...) fprintf(fss.err, "%s%sminishell: %s"MSG"\r\n", STYLE_BOLD, COLOR_BRIGHT_RED, COLOR_RESET, ##__VA_ARGS__)

#define MSH_LOG_NN(MSG, ...)   COLOR_BRIGHT_BLUE(stdout);                               \
                            fprintf(stdout, "minishell: ");                         \
                            COLOR_RESET(stdout);                                    \
                            fprintf(stdout, MSG, ##__VA_ARGS__)


struct file_streams {
    FILE* out;
    FILE* in;
    FILE* err;
};

int execute_line(ast_t* tree, const char* cmdline);

// Flag para controlar los saltos de linea.
extern bool g_dont_nl;

// Flag -> comando actual en background
extern bool g_background; 

extern bool g_internal;

extern bool g_abort_execution;

#endif // COMMAND_EXECUTE_H_