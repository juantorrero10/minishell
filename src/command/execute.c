#include <minishell.h>
#include <log.h>
#include <parser/public.h>


static int get_internal_idx(char* argv0) {
    int idx = 0;
    builtin_t* curr = g_builtin_function_table;

    if (!argv0) return -1;

    while (curr[idx].name)
    {
        if (!strcmp(curr[idx].name, argv0)) return idx;
        idx++;
    }

    return -1;
    
}


static int execute_simple_command(job_t* job, ast_t* tree, const char* cmdline) {
    (void)cmdline;
    int internal_idx = 0;
    ast_node_command_t cmd;
    pid_t pid = 0;
    //TEMP
    struct file_streams fss = {.in = stdin, .out = stdout, .err = stderr};

    cmd = tree->node.cmd;
    
    if (tree->type != AST_COMMAND) {
        MSH_ERR("execution failed, expected type %d, received: %d"
            , AST_COMMAND, tree->type);
        return EXIT_ERROR_UNEXPECTED_AST;
    }

    //TEMP
    if(cmd.nredirs > 0) {
        MSH_LOG("not yet.");
    }
    //Internal commands
    if (cmd.filename == NULL) {
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
        return g_builtin_function_table[internal_idx].fptr(cmd.argc, cmd.argv, fss);
    }

    //Externals commands
    pid = fork();

    if (!pid) {
        /*--------------- CHILD -----------------*/

        /**
         * @todo check for redirs */

        if (cmd.nredirs) {
            //
        }
        INFO("isatty(stdout): %d", isatty(STDOUT_FILENO));
        setpgid(0, 0);
        execve(cmd.filename, cmd.argv, environ);
        // Si ha ocurrido un error.
        perror(cmd.filename);
        // Terminar ejecuccion del proceso hijo.
        _exit(127);
    } else {
        /*--------------- PARENT -----------------*/
        //TEMP
        job->pids = malloc(sizeof(pid_t));
        job->nprocceses++;
        job->pids[0] = pid;
        if (!job->pgid) job->pgid = pid;
        setpgid(pid, pid);
        return 0;
        
    }
}


static int execute_generic(job_t* job, ast_t* tree, const char* cmdline) {
    (void)cmdline;

    //////TEMP////////
    if (tree->type != AST_BG && tree->type != AST_COMMAND) {
        MSH_ERR("not yet.");
        return -1;
    }

    if (tree->type == AST_BG) {
        g_background = 1;
        return execute_generic(job, tree->node.bg.children, cmdline);
    }

    return execute_simple_command(job, tree, cmdline);
}

int execute_line(ast_t* tree, const char* cmdline) {
    job_t job = (job_t){0};
    job.cmdline = (char*)cmdline;
    int status = 0;
    int ret = 0;
    bool interrupted = 0;

    g_internal = 0;
    g_background = 0;
    g_abort_execution = 0;

    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);


    ret = execute_generic(&job, tree, cmdline);

    if (!g_internal && !g_abort_execution) {
        if (!g_background) {
            g_dont_nl = 1;
            tcsetpgrp(STDIN_FILENO, job.pgid);
            for (int i = 0; i < job.nprocceses; i++)
            {
                if (job.pids[i] == -1) continue;
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
    return ret;
}