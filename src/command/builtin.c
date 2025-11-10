#include <minishell.h>
/**
 * En este archivo se almancenan las funciones de que manejan 
 * los commandos internos o "builtins"
 */



/**
 *  exit [code]
 */
int builtin_exit (int c, char** v){
    int code = 0;

    if (c > 2) {
        printf("minishell: exit: too many arguments\n");
        return 0;
    }
    if (c == 2) {
        if (!strcmp("--help", v[1])) {
            printf("minishell: exit: Usage: %s [code]\n", v[0]);
            return code;
        }
        code = atoi(v[1]);
    }
    exit(code);
}

/**
 * cd [dir]
 */
int builtin_chdir(int c, char** v){
    int ret = 0;

    if (c > 2) {
        printf("minishell: cd: too many arguments\n");
    }
    if (c == 2) {
        if (!strcmp("--help", v[1])) {
            printf("minishell: cd: Usage: %s [dir]\n", v[0]);
            return ret;
        }
        ret = chdir(v[1]);
        if (ret == -1) printf("minishell: cd: '%s' is not a directory\n", v[1]);
    }
    return ret;
}
int builtin_echo (int c, char** v){ (void)c; (void)v; return 0L;}
int builtin_umask(int c, char** v){ (void)c; (void)v; return 0L;}
int builtin_jobs (int c, char** v){ (void)c; (void)v; return 0L;}
int builtin_fg   (int c, char** v){ (void)c; (void)v; return 0L;}