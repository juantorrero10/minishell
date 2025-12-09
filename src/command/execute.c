#include <minishell.h>
#include <log.h>
#include <parser/public.h>

// Forward declaration
static int execute_generic(
        job_t* job, 
        ast_t* tree, 
        const char* cmdline, 
        bool force_fork, 
        int* pipe_fd, int*
        prev_pipe
    );

static int execute_simple_command(job_t* job, ast_t* tree, const char* cmdline, bool force_fork, int* pipe_fd, int* prev_pipe) {
    (void)cmdline;
    (void)force_fork;
    int internal_idx = -1;
    ast_node_command_t cmd;
    bool overwrite = 0;
    char** ow = g_overwrite_external;
    int ret = 0;
    pid_t pid = 0;
    //TEMP
    struct file_streams fss = {.in = stdin, .out = stdout, .err = stderr};

    cmd = tree->node.cmd;
    
    if (tree->type != AST_COMMAND) {
        MSH_ERR("execution failed, expected type %d, received: %d"
            , AST_COMMAND, tree->type);
        return EXIT_ERROR_UNEXPECTED_AST;
    }

    //overwrite commands
    while(*ow) { 
        if (!strcmp(cmd.argv[0], *ow++)) {
            overwrite = 1; break;
        }
    }

    //Internal commands
    if (cmd.filename == NULL || overwrite) {
        if( (internal_idx = get_internal_idx(cmd.argv[0]) ) == -1) {
            MSH_ERR("unknown command %s%s%s%s%s", STYLE_BOLD, STYLE_UNDERLINE,
                COLOR_BRIGHT_RED, tree->node.cmd.argv[0], COLOR_RESET);
            g_abort_execution = 1;
            return EXIT_COMMAND_NOT_FOUND;
        }
        g_internal = 1;
        //TEMP
        job->pids = malloc(sizeof(pid_t));
        job->nprocceses++;
        job->pids[0] = -1;
    }

    //Externals commands
    if (!g_internal || force_fork)
        pid = fork();
    if (pid == -1) {
        MSH_ERR("fork failed");
        perror("fork");
        return EXIT_ERROR_FORKING;
    }

    if (pid == 0) {
        /*--------------- CHILD -----------------*/

        //internal
        if (g_internal) {
            if (force_fork) {
                exit(g_builtin_function_table[internal_idx].fptr(cmd.argc, cmd.argv, fss));
            }
            else return g_builtin_function_table[internal_idx].fptr(cmd.argc, cmd.argv, fss);
        }

        // Pipe redirections
        if (pipe_fd && prev_pipe) {
            if (*prev_pipe != -1) {
                dup2(*prev_pipe, STDIN_FILENO);
                close(*prev_pipe);
            }
            if (!g_last_ppl_element) {
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[0]);
            }
        }

        // Redirections
        if (cmd.nredirs && !g_internal) {
            for (size_t i = 0; i < cmd.nredirs; i++)
            {
                ret = handle_redirection(&cmd.redirs[i]);
                if (g_abort_execution) exit(ret);
            }
        }

        INFO("isatty(stdout): %d", isatty(STDOUT_FILENO));
        setpgid(0, 0);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        execve(cmd.filename, cmd.argv, environ);
        // Si ha ocurrido un error.
        perror(cmd.filename);
        // Terminar ejecuccion del proceso hijo.
        _exit(127);
    } else {
        /*--------------- PARENT -----------------*/
        job->pids[job->nprocceses] = pid;
        job->nprocceses++;
        if (!job->pgid) job->pgid = pid;
        setpgid(pid, pid);

        //pipe shit
        if (pipe_fd && prev_pipe) {
            if (*prev_pipe != -1) close(*prev_pipe);
            if (!g_last_ppl_element) {
                close(pipe_fd[1]);
                *prev_pipe = pipe_fd[0];
            }
        }
    

        INFO("end_simple");
        return 0;
        
    }
}

static int execute_pipeline(job_t* job, ast_t* tree, const char* cmdline) {
    size_t nelements;
    int pipe_fd[2]      = {-1, -1};
    int ret             = 0;
    int prev_pipe       = -1;

    (void)job;
    (void)cmdline;
    (void)tree;

    nelements = tree->node.ppl.nelements;
    g_last_ppl_element = 0;

    // Note that this point pipelines are guaranteed to have at least 2 elements.
    for (size_t i = 0; i < nelements; i++)
    {
        if (i == nelements - 1) {g_last_ppl_element = 1;}
        else {
            if (pipe(pipe_fd) == -1) {
                MSH_ERR("couldn't create pipe: %s", strerror(errno));
                g_abort_execution = 1;
                return EXIT_ERROR_CREATING_PIPE;
            }
        }

        if (tree->node.ppl.elements[i].type == AST_COMMAND) {
            ret = execute_generic(job, &tree->node.ppl.elements[i], cmdline, true, pipe_fd, &prev_pipe);
        } else if (tree->node.ppl.elements[i].type == AST_GROUP) {
            ret = execute_generic(job, &tree->node.ppl.elements[i], cmdline, true, pipe_fd, &prev_pipe);
        } else {MSH_ERR("parsing went wrong"); g_abort_execution = 1; return EXIT_ERROR_UNEXPECTED_AST;}
        
    }
    
    return ret;
}


static int execute_generic(
        job_t* job, 
        ast_t* tree, 
        const char* cmdline, 
        bool force_fork, 
        int* pipe_fd, int*
        prev_pipe
    ) {
    (void)cmdline;

    //////TEMP////////
    if (tree->type != AST_BG 
        && tree->type != AST_COMMAND 
        && tree->type != AST_PIPELINE
        ) {
        MSH_ERR("not yet.");
        return -1;
    }
    

    if (tree->type == AST_BG) {
        g_background = 1;
        INFO("end_generic, bg");
        return execute_generic(job, tree->node.bg.children, cmdline, false, NULL, NULL);
    }

    else if (tree->type == AST_PIPELINE) {
        INFO("end_generic, pipeline");
        return execute_pipeline(job, tree, cmdline);
    }

    INFO("end_generic, simple");
    return execute_simple_command(job, tree, cmdline, force_fork, pipe_fd, prev_pipe);
    
}

int execute_line(ast_t* tree, const char* cmdline) {
    job_t job = (job_t){0};
    job.cmdline = (char*)cmdline;
    size_t npids = 0;
    int status = 0;
    int ret = 0;
    bool interrupted = 0;

    g_internal = 0;
    g_background = 0;
    g_abort_execution = 0;

    job.pids = malloc((npids = get_npids(tree, false)) * sizeof(pid_t));
    INFO("NPIDS: %zu", npids);
    ret = execute_generic(&job, tree, cmdline, false, NULL, NULL);
    SEP();
    OKAY("Exec returned: %d", ret);
    INFO("PGID: %d", job.pgid);
    INFO("PIDS: ");
#ifdef __DEBUG
    for (int i = 0; i < job.nprocceses; i++)
    {
        LOG("%d", job.pids[i]);
        if (i != job.nprocceses - 1) LOG(", ");
    } NL();
#endif
    if (!g_internal && !g_abort_execution) {
        if (!g_background) {
            g_dont_nl = 1;
            tcsetpgrp(STDIN_FILENO, job.pgid);
            for (int i = 0; i < job.nprocceses; i++)
            {
                if (job.pids[i] == -1) continue; //Skip internals
                waitpid(job.pids[i], &status, WUNTRACED);
                INFO("st: %d: stopped?: %d", status, WIFSTOPPED(status));

                if (WIFSTOPPED(status)) {
                    job.state = STOPPED;
                    job.background = 1;
                    job.id = job_add(job);
                    fputc('\n', stdout);
                    MSH_LOG("new job: [%d] %d (Stopped)", job.id, job.pgid);
                    ret = 0;
                    interrupted = 1;
                    break;
                }
            }
            if (!interrupted)ret = WEXITSTATUS(status);
            tcsetpgrp(STDIN_FILENO, getpid());
            g_dont_nl = 0;
            signal(SIGTTOU, SIG_IGN);
            signal(SIGTTIN, SIG_IGN);
            signal(SIGTSTP, SIG_IGN);
        } else {
            job.state = RUNNING;
            job.background = 1;
            if (job.pgid) {
                job.id = job_add(job);
            } else if (!job.pgid) {
                MSH_ERR("couldn't create '%s' job", cmdline);
                return EXIT_ERROR_CREATING_JOB;
            }
            MSH_LOG("new job: [%d] %d", job.id, job.pgid);
        }
    }
    job_update_status();
    free(job.pids);
    INFO("end_line");
    SEP();
    return ret;
}