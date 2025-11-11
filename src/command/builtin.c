#include <minishell.h>
#include <log.h>
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
        MSH_ERR_C("exit: too many arguments");
        return 1;
    }
    if (c == 2) {
        if (!strcmp("--help", v[1])) {
            MSH_LOG_C("exit: Usage: %s [code]", v[0]);
            return code;
        }
        code = atoi(v[1]);
    }
    g_exit_signal = 1;
    return code;
}

/**
 * cd [dir]
 */
int builtin_chdir(int c, char** v, struct file_streams fss){
    int ret = 0;

    if (c > 2) {
        MSH_ERR_C("cd: too many arguments");
        return 1;
    }
    if (c == 2) {
        if (!strcmp("--help", v[1])) {
            MSH_LOG_C("cd: Usage: %s [dir]", v[0]);
            return ret;
        }
        ret = chdir(v[1]);
        if (ret == -1) {
            MSH_ERR_C("cd: '%s' is not a directory", v[1]);
        }
    }
    return ret;
}

int builtin_set(int c, char** v, struct file_streams fss) {
    char* name;
    char* value;
    int r;

    if (c > 3) {
        MSH_ERR_C("set: too many arguments");
        return 1;
    }
    switch(c) {
    case 3:
        name = v[1];
        if (name[0] == '$') name++;
        value = v[2];
        r = setenv(name, value, 1);
        if (r == -1) r = setenv(name, value, 0);
        return r;
    case 2:
        name = v[1];
        if (name[0] == '$') name++;
        return unsetenv(name);
    default:
        MSH_LOG_C("set: Usage %s <var_name> <value>", v[0]);
        return 0;
    }
}

int builtin_unset(int c, char** v, struct file_streams fss) {
    if (c > 2) {
        MSH_ERR_C("set: too many arguments");
        return 1;
    } else if (c == 2) return builtin_set(c, v, fss);
    else MSH_LOG_C("unset: Usage %s <var_name>", v[0]);
    return 0;
}

int builtin_umask(int c, char** v, struct file_streams fss){ (void)c; (void)v; (void)fss; return 0L;}
int builtin_jobs (int c, char** v, struct file_streams fss){ (void)c; (void)v; (void)fss; return 0L;}
int builtin_fg   (int c, char** v, struct file_streams fss){ (void)c; (void)v; (void)fss; return 0L;}


int builtin_whaterror(int c, char** v, struct file_streams fss) {
    int error = g_last_error_code;
    char* s = NULL;

    if (c > 2) {
        MSH_ERR_C("whaterror: too many arguments");
        return 1;
    } else if (c == 2) {
        error = atoi(v[1]);
    }
    
    switch (error) {
        case EXIT_COMMAND_NOT_FOUND: s = "EXIT_COMMAND_NOT_FOUND";break;
        case EXIT_ERROR_FORKING: s = "EXIT_ERROR_FORKING";break;
        case EXIT_ERROR_OPENING_FILE: s = "EXIT_ERROR_OPENING_FILE";break;
        case EXIT_ERROR_CREATING_PIPE: s = "EXIT_ERROR_CREATING_PIPE";break;
        case EXIT_FAILURE: s = "EXIT_FAILURE";break;
        case EXIT_SUCCESS: s = "EXIT_SUCCESS"; break;
        default: s = "unknown";
    }
    MSH_LOG_C("%d: %s", error, s);
    return error;
}