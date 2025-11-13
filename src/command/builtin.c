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

int builtin_jobs (int c, char** v, struct file_streams fss){ 
    (void)c; (void)v;
    job_t* curr;
    job_state s; (void)s;
    int hightest, second;
    char priority = ' ';

    hightest = -1;
    second = -1;
    if (c > 1 && !strcmp(v[1], "--help")) {
        MSH_LOG_C("jobs: todo-> help message");
        return 0;
    }

    COLOR_BRIGHT_GREEN(fss.out);
    fprintf(fss.out, "jobs: ");
    COLOR_RESET(fss.out);

    if (g_bgjob_list == NULL) {
        fprintf(fss.out, "there are no jobs.\n");
        return 0;
    }
    fputc('\n', fss.out);
    curr = g_bgjob_list;
    hightest = curr->priority;
    // Buscar los dos con prioridad mas alta.
    do {
        if (curr->priority > hightest && curr->state != DONE) {
            second = hightest;
            hightest = curr->priority;
        }
        curr = curr->next;
    } while(curr);

    // Imprimir en pantalla.
    curr = g_bgjob_list;
    do {
        priority = ' ';
        s = job_get_status(curr->pgid);
        INFO("Read1: %d", curr->state);
        job_checkupdate(curr, s, curr->state, false);
        INFO("Read2: %d", curr->state);
        if (curr->priority == second)   priority = '-';
        if (curr->priority >= hightest) priority = '+';
        job_print(curr, fss.out, priority);
        curr = curr->next; 
    } while (curr != NULL);

    return 0;
    
}

static char** expand_job_args(int c, char** v, struct file_streams fss, _out_ int* err) {
    char **ret;
    int id;
    pid_t pid = -1;
    *err = 0;

    ret = malloc(sizeof(char*) * (c + 1));
    for(int i = 0; i < c; i++) {
        // Sera suficiente espacio.
        ret[i] = malloc(strlen(v[i]) + 5);
        if (v[i][0] == '%') {
            id = atoi(v[i] + 1);
            if((pid = job_get_pid(id)) == -1) {
                MSH_ERR_C("'%s' no such job", v[i]);
                *err = 1;
            }
            sprintf(ret[i], "%d", pid);
            continue;
        } strcpy(ret[i], v[i]);
    }
    ret[c] = NULL;
    return ret;
}

static void free_argv(int c, char**v) {
    for (int i = 0; i < c; i++)
    {
        free(v[i]);
    }if(v)free(v);  
}
/**
 * Sobreescritura del comando externo kill.
 */
int builtin_kill         (int c, char** v, struct file_streams fss) {
    (void)c; (void)v; (void)fss;
    char** exp_args;
    int err = 0;
    pid_t pid = 0;
    int status = 0;

    g_dont_nl = 1;
    exp_args = expand_job_args(c, v, fss, &err);
    if(!exp_args) return 127;
    if (err) {
        if (exp_args)free_argv(c, exp_args);
        return 1;
    }

    // Llamar al comando externo /usr/bin/kill
    pid = fork();
    if (!pid) {
        // Proceso hijo
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTSTP, SIG_DFL);
        dup2(fileno(fss.in), STDIN_FILENO);
        dup2(fileno(fss.out), STDOUT_FILENO);
        dup2(fileno(fss.err), STDERR_FILENO);
        setpgid(0, 0);
        execve("/usr/bin/kill", exp_args, environ);
        perror("execve->kill");
        free_argv(c, exp_args);
        exit(1);
    }
    //proceso padre
    waitpid(pid, &status, 0);
    free_argv(c, exp_args);
    g_dont_nl = 0;
    return WIFEXITED(status)? WEXITSTATUS(status) : 1;
    
}

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