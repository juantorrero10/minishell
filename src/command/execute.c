#include <minishell.h>
#include <log.h>

static void close_file_streams(struct file_streams streams) {
    (streams.out == stdout)? fflush(stdout) : fclose(streams.out);
    (streams.in == stdin)  ? fflush(stdin)  : fclose(streams.in);
    (streams.err == stderr)? fflush(stderr) : fclose(streams.err);
}


/**
 * @brief Ejecutar un comando interno.
 * Los comandos internos no permiten redirecciones (por ahora).
 */
static int execute_builtin(int argc, char** argv, struct file_streams streams) {
    size_t i = 0;
    int ret = EXIT_COMMAND_NOT_FOUND;

    while (g_builtin_function_table[i].fptr != NULL) {
        if (!strcmp(argv[0], g_builtin_function_table[i].name)) {
            ret = g_builtin_function_table[i].fptr(argc, argv, streams);
            return ret;
        }
        i++;
    }
    MSH_ERR("%s: command not found", argv[0]);
    return ret; 
}

/**
 * @brief ejecutar una serie de commandos.
 */
int execute_command(tline* tokens) {
    char* exp = NULL;
    size_t s = 0;
    struct file_streams fss = {.out = stdout, .in = stdin, .err = stderr};
    int ret = 0;

    for (int i = 0; i < tokens->ncommands; i++)
    {
        // Si es el primero, permitir redirecciones de stdin
        if (i == 0 && tokens->redirect_input) {
            fss.in = fopen(tokens->redirect_input, "r+");
            if (!fss.in) {
                MSH_ERR("%s: %s", tokens->redirect_input, strerror(errno));
            }
        }

        if (i == (tokens->ncommands - 1)) {
            if (tokens->redirect_output) {
                fss.out = fopen(tokens->redirect_output, "a+");
                if (!fss.out) {
                    MSH_ERR("%s: %s", tokens->redirect_output, strerror(errno));
                    break;
                }
            }
            if (tokens->redirect_error) {
                fss.err = fopen(tokens->redirect_error, "a+");
                if (!fss.err) {
                    MSH_ERR("%s: %s", tokens->redirect_error, strerror(errno));
                    break;
                }
            }
        }


        if (tokens->commands[i].filename) {
            OKAY("Extern: %s", tokens->commands[i].argv[0]);
            if (tokens->commands[i].argc > 1) {
                exp = env_expand_vars(tokens->commands[i].argv[1], &s);
                OKAY("ARG: %s", exp);
                free(exp);
            }
        } else {
            ret = execute_builtin(tokens->commands[i].argc, tokens->commands[i].argv, fss);
        }
        
    };
    close_file_streams(fss);
    return ret;
}
