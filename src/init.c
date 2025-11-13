/**
 * En este archivo estaran las functiones que se encargan de inicializar
 * lo que minishell necesita para funcionar.
 * Minishell necesita antes que nada:
 *      -  Reemplazar el "handler" del las señales SIGINT para el propio shell y los procesos hijo.
 *      -  Reemplazar el handler de las señales SIGCHLG para notificar cuando los procesos en segundo plano terminan.
 *      -  Obtener el numero de variables del entorno del sistema -> g_num_envvars
 *      -  Cambiar la variable $SHELL por la de este programa.
 */

#define _DEF_BANNER

#include <minishell.h>
#include <log.h>

/*------- Inicializar variables globales--------------- */
size_t g_num_envvars = 0;
int g_last_error_code = 0;
int g_exit_signal = 0;
bool g_dont_nl = 0;
job_llist g_bgjob_list = NULL;
size_t g_sz_jobs;
builtin_t g_builtin_function_table[] = {
        {"exit", builtin_exit},
        {"cd", builtin_chdir},
        {"chdir", builtin_chdir},
        {"umask", builtin_umask},
        {"jobs", builtin_jobs},
        {"fg", builtin_fg},
        {"werror", builtin_whaterror},
        {"set", builtin_set},
        {"unset", builtin_unset},
        {"whaterror", builtin_whaterror},
        {"kill", builtin_kill}, // Sobreescrito.
        {NULL, NULL}
    };




/**
 * @brief Instalar señales.
 */
static void init_install_signals(void) {
    struct sigaction action;

    // Establecer la function las "flags"
    action.sa_handler = sigint_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    // Reemplazar el handle por defecto por el nuestro, que no hace nada.
    if (sigaction(SIGINT, &action, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    signal(SIGCHLD, sigchld_handler);
    signal(SIGTSTP, sigtstp_handle);

    
}

/**
 * @brief Cambiar la variable $SHELL por la de este binario.
 */
static int init_shell_env(const char *argv0) {
    char path[PATH_MAX];

    if (realpath(argv0, path) == NULL) {
#ifdef __DEBUG
        perror("realpath");
#endif
        return -1;
    }
    if (setenv("SHELL", path, 1) != 0) {
#ifdef __DEBUG
        perror("setenv");
#endif
        return -1;
    }

    OKAY("changed $SHELL to: %s", getenv("SHELL"));
    return 0;
}


void init_minishell(int argc, char** argv) {
    (void)argc;
    size_t idx = 0;
    init_install_signals();
    if (init_shell_env(argv[0]) == -1) WARN("$SHELL was no updated.");

    //Obtener el numero de variables de entorno
    while(environ[idx++]) {
        g_num_envvars++;
    }
    // Imprimir banner si no estamos en debug
#ifndef __DEBUG
    printf("%s", banner);
#endif
    INFO("TERMINAL PID: %d", getpid());
    INFO("DEBUG MODE");
}