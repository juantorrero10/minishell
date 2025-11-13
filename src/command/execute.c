#include <minishell.h>
#include <log.h>

static void close_file_streams(struct file_streams streams) {
    (streams.out == stdout)? fflush(stdout) : fclose(streams.out);
    (streams.in == stdin)  ? fflush(stdin)  : fclose(streams.in);
    (streams.err == stderr)? fflush(stderr) : fclose(streams.err);
}


/**
 * @brief Ejecutar un comando interno.
 */
static int launch_builtin(int idx, int argc, char** argv, struct file_streams streams) {
    int ret = 0;
    if (idx == EXIT_COMMAND_NOT_FOUND){
        MSH_ERR("%s: command not found", argv[0]);
        return idx;
    }
    ret = g_builtin_function_table[idx].fptr(argc, argv, streams);
    return ret; 
}

static int find_builtin(char* c) {
    size_t i = 0;

    while (g_builtin_function_table[i].fptr != NULL) {
        if (!strcmp(c, g_builtin_function_table[i].name)) {
            return i;
        }
        i++;
    }
    return EXIT_COMMAND_NOT_FOUND;
}
#ifdef __DEBUG
/*
static void print_command(tline* tokens) {
    int __i = 0;
    int __j = 0;

    for (__i = 0; __i < tokens->ncommands; __i++) {
        OKAY("Command # %d", __i);
        for (__j = 0; __j < tokens->commands[__i].argc; __j++)
        {
            INFO("ARG%d: %s", __j, tokens->commands[__i].argv[__j]);
        }
        
    }

    INFO("> : %s", tokens->redirect_output);
    INFO("< : %s", tokens->redirect_input);
    INFO(">&: %s", tokens->redirect_error);
    // cd $(PWD)/src > $(HOME)/out.txt
}
*/
#endif

static int launch_external(int i, tline *tokens, struct file_streams fss, job_t* job, int* pipe_fd, int* prev_pipe) {
    pid_t pid = 0;
    int n = 0;
    
    n = tokens->ncommands;
    // Si no es el último
    if ( i < (n - 1)) {
        if (pipe(pipe_fd) == -1) {
            MSH_ERR("error creating pipe");
            perror("pipe");
            return EXIT_ERROR_CREATING_PIPE;
        }
    }
    pid = fork();
    if (pid == -1) {
        MSH_ERR("fork of '%s' failed", tokens->commands[i].argv[0]);
        perror("fork"); return EXIT_ERROR_FORKING;
    };
    /**
     * "Entrada normal": stdin o archivo
     * "Salida normal" : stdout o archivo
     */

    if (pid == 0) {
        /*--------------------- PROCESO HIJO ---------------------*/

        // Cambiar stdin por la entrada normal o la salida del proceso anterior.
        if (i == 0) {
            // Cerrar archivo para no impedir al programa abrirlo.
            if (fss.in != stdin) {dup2(fileno(fss.in), STDIN_FILENO); fclose(fss.in);}
        }
        else {dup2(*prev_pipe, STDIN_FILENO); close(*prev_pipe);}

        // Cambiar stdout por la entrada del siguiente o salida normal
        if (i == (n - 1)) {
            if (fss.out != stdout) { dup2(fileno(fss.out), STDOUT_FILENO); fclose(fss.out); }
            if (fss.err != stderr) { dup2(fileno(fss.err), STDERR_FILENO); fclose(fss.err); }
        } else 
        {
            dup2(pipe_fd[1], STDOUT_FILENO); 
            close(pipe_fd[0]);
        }


        //Ejecutar comando
        setpgid(pid, job->pgid);
        INFO("isatty(stdout): %d", isatty(STDOUT_FILENO));
        execve(tokens->commands[i].filename, tokens->commands[i].argv, environ);
        perror(tokens->commands[i].filename);
        // Terminar ejecuccion del proceso hijo.
        exit(1);

    } else {
        /*--------------------- PROCESO PADRE ---------------------*/
        job->pids[i] = pid;
        if (i == 0) job->pgid = pid;
        setpgid(pid, job->pgid);

        // Si no es el primero, cerrar la anterior
        if (*prev_pipe != -1) close(*prev_pipe);
        // Si no es el ultimo, 
        if (i < (n - 1)) {
            close(pipe_fd[1]);
            *prev_pipe = pipe_fd[0];
        }
        return 0;
    }
}

/**
 * @brief ejecutar una serie de commandos.
 */
int execute_command(tline* tokens, const char* cmdline) {
    /** 
     * Aqui estan los commandos que son externos pero se sobreescriben con un comando interno.
     * La razon por la que se sobreescriben es para integrarlos mejor con la terminal.
     * (ver su funcion builtin para mas detalles)
     * 
     * @note solo se puede sobreescribir el primer comando de la cadena.
    */
    char* command_overwrite[] = {"kill", NULL};
    bool overwriten = 0;

    int builtin_idx = -1;
    struct file_streams fss = {.out = stdout, .in = stdin, .err = stderr};
    int ret = 0, status;
    int pipe_fd[2];
    int prev_pipe_fd = -1;
    bool last_internal = 0;
    bool not_interrupted = 0;
    job_t job = {.nprocceses = tokens->ncommands, 
        .background = tokens->background, .pids = NULL};
    // -----------------------------------------------------------
    job.pids = malloc(sizeof(pid_t) * job.nprocceses);
    job.cmdline = malloc(strlen(cmdline) + 1);
    strcpy(job.cmdline, cmdline);

    // Ignorar estas señales durante la ejecucion.
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    // Restablecer SIGSTOP
    signal(SIGTSTP, SIG_DFL);

    // Determinar si es necesario sobreescribir
    while (command_overwrite[ret]) {
        if (!strcmp(tokens->commands[0].argv[0], command_overwrite[ret++]))
            overwriten = 1;
    }
    ret = 0;

    for (int i = 0; i < tokens->ncommands; i++)
    {
        last_internal = 0;
        // Si es el primero, permitir redirecciones de stdin
        if (i == 0 && tokens->redirect_input) {
            fss.in= fopen(tokens->redirect_input, "r");
            if (!fss.in) {
                MSH_ERR("%s: %s", tokens->redirect_input, strerror(errno));
                ret = EXIT_ERROR_OPENING_FILE; break;
            }
        }

        // si es el ultimo permitir redirecciones de stderr y stdout
        if (i == (tokens->ncommands - 1)) {
            if (tokens->redirect_output) {
                fss.out = fopen(tokens->redirect_output, "a+");
                if (!fss.out) {
                    MSH_ERR("%s: %s", tokens->redirect_output, strerror(errno));
                    ret = EXIT_ERROR_OPENING_FILE; break;
                }
            }
            if (tokens->redirect_error) {
                fss.err = fopen(tokens->redirect_error, "a+");
                if (!fss.err) {
                    MSH_ERR("%s: %s", tokens->redirect_error, strerror(errno));
                    ret = EXIT_ERROR_OPENING_FILE; break;
                }
            }
        }

        if (is_external(tokens, i) && !overwriten)
            ret = launch_external(i, tokens, fss, &job, pipe_fd, &prev_pipe_fd);
        else {
            last_internal = 1;
            builtin_idx = find_builtin(tokens->commands[i].argv[0]);
            if (builtin_idx == EXIT_COMMAND_NOT_FOUND) {
                MSH_ERR("%s: command not found", tokens->commands[i].argv[0]);
                ret = EXIT_COMMAND_NOT_FOUND; break;
            }
            ret = launch_builtin(builtin_idx, 
                tokens->commands[i].argc, tokens->commands[i].argv, fss);
        }
        
    };
    /*_--------------------------- FINAL DEL BUCLE PRINCIPAL --------------------------------*/
    if (!tokens->background) {
        g_dont_nl = 1;
        tcsetpgrp(STDIN_FILENO, job.pgid);
        for (int i = 0; i < tokens->ncommands; i++)
        {
            waitpid(job.pids[i], &status, WUNTRACED);
            INFO("st: %d: stopped?: %d", status, WIFSTOPPED(status));
            if (WIFSTOPPED(status)) {
                tcsetpgrp(STDIN_FILENO, getpid());
                job.state = STOPPED;
                job.background = 1;
                job.id = job_add(job);  // Añadir trabajo a la lista
                fputc('\n', stdout);
                MSH_LOG("new job: [%d] %d (Stopped)", job.id, job.pgid);
                ret = 0;
                not_interrupted = 1;
                signal(SIGTSTP, sigtstp_handle);
                break;
            }
        }  if (!last_internal && !not_interrupted)ret = WEXITSTATUS(status);
        tcsetpgrp(STDIN_FILENO, getpid());
        g_dont_nl = 0;

        
    } else {
        job.state = RUNNING;
        job.background = 1;
        if (job.pgid) {
            job.id = job_add(job);
        } else if (!job.pgid) {
            MSH_ERR_C("couldn't create '%s' job", cmdline);
            return EXIT_ERROR_CREATING_JOB;
        }
        MSH_LOG("new job: [%d] %d", job.id, job.pgid);
    }

    job_update_status();
    close_file_streams(fss);
    free(job.pids);
    free(job.cmdline);
    signal(SIGTSTP, sigtstp_handle);
    INFO("END execute_command");
    return ret;
}
