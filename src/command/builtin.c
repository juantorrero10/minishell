#include <minishell.h>
/**
 * En este archivo se almancenan las funciones de que manejan 
 * los commandos internos o "builtins"
 */



/**
 *  exit [code]
 */
int builtin_exit (int c, char** v, struct file_streams fss){
    int code = 0;

    if (c > 2) {
        MSH_ERR_C(fss.err, "exit: too many arguments");
        return 0;
    }
    if (c == 2) {
        if (!strcmp("--help", v[1])) {
            MSH_LOG_C(fss.out, "exit: Usage: %s [code]", v[0]);
            return code;
        }
        code = atoi(v[1]);
    }
    exit(code);
}

/**
 * cd [dir]
 */
int builtin_chdir(int c, char** v, struct file_streams fss){
    int ret = 0;

    if (c > 2) {
        MSH_ERR_C(fss.err, "cd: too many arguments");
    }
    if (c == 2) {
        if (!strcmp("--help", v[1])) {
            MSH_LOG_C(fss.out, "cd: Usage: %s [dir]", v[0]);
            return ret;
        }
        ret = chdir(v[1]);
        if (ret == -1) {
            MSH_ERR_C(fss.err, "cd: '%s' is not a directory", v[1]);
        }
    }
    return ret;
}

int builtin_umask(int c, char** v, struct file_streams fss){ (void)c; (void)v; (void)fss; return 0L;}
int builtin_jobs (int c, char** v, struct file_streams fss){ (void)c; (void)v; (void)fss; return 0L;}
int builtin_fg   (int c, char** v, struct file_streams fss){ (void)c; (void)v; (void)fss; return 0L;}