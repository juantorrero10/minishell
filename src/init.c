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
bool g_background = 0;
bool g_internal = 0;
bool g_abort_execution = 0;
bool g_last_ppl_element = 0;
job_llist g_bgjob_list = NULL;
size_t g_sz_jobs = 0;
char* g_overwrite_external[] = {"kill", NULL};
builtin_t g_builtin_function_table[] = {
        {"exit", builtin_exit},
        {"cd", builtin_chdir},
        {"chdir", builtin_chdir},
        {"umask", builtin_umask},
        {"jobs", builtin_jobs},
        {"fg", builtin_fg},
        {"set", builtin_set},
        {"unset", builtin_unset},
        {"kill", builtin_kill}, // Sobreescrito.
        {"getpid", builtin_getpid},
        {NULL, NULL}
    };




/**
 * @brief Instalar señales.
 */
static void init_install_signals(void) {
    struct sigaction action1;
    struct sigaction action2;

    // Establecer la function las "flags"
    action1.sa_handler = sigint_handler;
    action2.sa_handler = sigchld_handler;
    sigemptyset(&action1.sa_mask);
    sigemptyset(&action2.sa_mask);
    action1.sa_flags = 0;
    action2.sa_flags = 0;

    // Reemplazar el handle por defecto por el nuestro, que no hace nada.
    if (sigaction(SIGINT, &action1, NULL) == -1) {
        perror("sigaction, SIGINT");
        exit(1);
    }
    if (sigaction(SIGCHLD, &action2, NULL) == -1) {
        perror("sigaction, SIGCHLD");
        exit(1);
    }
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    
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
    if (argc == 1 || strcmp("-s", argv[1]))printf("%s", banner);
#endif
    INFO("TERMINAL PID: %d", getpid());
    WARN("DEBUG MODE");
}