/**
 * En este archivo estaran las functiones que se encargan de inicializar
 * lo que minishell necesita para funcionar.
 * Minishell necesita antes que nada:
 *      -  Reemplazar el "handler" del las señales SIGINT para el propio shell y los procesos hijo.
 *      -  Obtener el numero de variables del entorno del sistema -> g_num_envvars
 *      ...
 */

//Necesario para sigaction y otras definiciones que dependen de la defincion de este macro.
#define _XOPEN_SOURCE 700 
#define _DEF_BANNER

#include <minishell.h>
#include <log.h>

/*-------Iniciar variables globales--------------- */
size_t g_num_envvars = 0;
int g_last_error_code = 0;
builtin_t g_builtin_function_table[] = {
        {"exit", builtin_exit},
        {"cd", builtin_chdir},
        {"chdir", builtin_chdir},
        {"umask", builtin_umask},
        {"jobs", builtin_jobs},
        {"fg", builtin_fg},
        {NULL, NULL}
    };;


/**
 * @brief Handler de la SIGINT que no hace nada relevante.
 */
static void sigint_handler(int sig) {
    (void)sig; fflush(stdout);
}

/**
 * @brief Reemplazar el handler por defecto de la señal de Ctrl^C.
 */
static void init_install_signit_handler(void) {
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

    
}


void init_minishell(int argc, char** argv) {
    (void)argc; (void)argv;
    
    size_t idx = 0;
    
    init_install_signit_handler();

    //Obtener el numero de variables de entorno
    while(environ[idx++]) {
        g_num_envvars++;
    }
    // Imprimir banner si no estamos en debug
#ifndef __DEBUG
    printf("%s", banner);
#endif
}