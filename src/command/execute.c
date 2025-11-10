#include <minishell.h>
#include <log.h>

/**
 * @brief Ejecutar un comando interno.
 * Los comandos internos no permiten redirecciones (por ahora).
 */
static int execute_builtin(int argc, char** argv) {
    size_t i = 0;
    int ret = EXIT_COMMAND_NOT_FOUND;

    while (g_builtin_function_table[i].fptr != NULL) {
        if (!strcmp(argv[0], g_builtin_function_table[i].name)) {
            ret = g_builtin_function_table[i].fptr(argc, argv);
            break;
        }
        i++;
    }
    return ret; 
}

/**
 * @brief ejecutar una serie de commandos.
 */
int execute_command(tline* tokens) {
    int ret = 0;

    for (int i = 0; i < tokens->ncommands; i++)
    {
        if (tokens->commands[i].filename) {
            OKAY("Extern: %s", tokens->commands[i].argv[0]);
        } else {
            ret = execute_builtin(tokens->commands[i].argc, tokens->commands[i].argv);
            if (ret == EXIT_COMMAND_NOT_FOUND) 
                printf("minishell: command not found: '%s'\n", tokens->commands[i].argv[0]);
        }
    };
    return ret;
}
