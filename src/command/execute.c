#include <minishell.h>
#include <log.h>

#define MAX_PIPELINE_ELEMENTS 256

const char* g_cmdline = NULL;

struct fds {
    int in;
    int out;
    int err;
    int pipe_read;
};

static error_t execute_ast(
    const ast_t* tree,
    struct fds* fds,
    struct redir_arr* grp_redirs,
    job_t* inherited_job,
    bool background
);

static error_t execute_command(
    const ast_node_command_t* cmd,
    struct fds* fds,
    struct redir_arr* grp_redirs,
    job_t* inherited_job,
    _out_ bool* needs_to_be_waited_for
);

static error_t execute_pipeline(
    const ast_node_pipeline_t* ppl,
    struct fds* fds,
    struct redir_arr* grp_redirs,
    job_t* inherited_job
);

static error_t combine_grp_redirs(
    const struct redir_arr* r1,
    const struct redir_arr* r2,
    _out_ struct redir_arr* out
);


// wrapper around the main recursive step.
cmd_exit_t execute_line(ast_t* tree, const char* cmdline) {
    g_cmdline = cmdline;
    g_internal = 0;
    g_background = 0;
    g_abort_execution = 0;

    if (!tree || !cmdline || !(cmdline[0])) {
        return 0;
    }

    struct fds fds = {0, 1, 2, -1};
    struct redir_arr g = {0};

    return execute_ast(tree, &fds, &g, NULL, false);
}

// Main recursive step
static error_t execute_ast(
    const ast_t* tree,
    struct fds* fds,
    struct redir_arr* grp_redirs,
    job_t* inherited_job,
    bool background
) {
    switch (tree->type) {
        case AST_BG:
            INFO("exec_BG");
            return execute_ast(tree->node.bg.children, fds, grp_redirs, inherited_job, true);
        case AST_COMMAND:
        {
            INFO("exec_COMMMAND");
            pid_t pid;
            job_t new_job = {
                .background = background,
                .pipe = false,
                .cmdline = g_cmdline,
                .id = JOB_INVALID_ID,
                .next = NULL,
                .pgid = 0,
                .pids.data = &pid,
                .pids.count = 0,
                .state = RUNNING,
            };
            // Use inherited job or create a new one
            job_t* used_job = (inherited_job != NULL)? inherited_job : &new_job;
            bool wait;
            error_t e = execute_command(&(tree->node.cmd), fds, grp_redirs, used_job, &wait);
            if (e < 0) {
                return e;
            }
            g_dont_nl = 1;
            cmd_exit_t exit_code = e;
            // Only wait for the job if we are not inheriting
            if (used_job == &new_job && wait) {
                exit_code = job_wait(used_job);
            }
            g_dont_nl = 0;
            return exit_code; 
        }
        case AST_PIPELINE:
        {
            INFO("exec_PIPELINE");
            pid_t pids[MAX_PIPELINE_ELEMENTS];
            job_t job = {
                .background = background,
                .pipe = true,
                .cmdline = g_cmdline,
                .id = JOB_INVALID_ID,
                .next = NULL,
                .pgid = 0,
                .pids.data = pids,
                .pids.count = 0,
                .state = RUNNING,
            };
            return execute_pipeline(&(tree->node.ppl), fds, grp_redirs, &job);
        }
        case AST_LIST:
        {
            INFO("exec_LIST");
            error_t lhs_ret = execute_ast(tree->node.sep.left, fds, grp_redirs, inherited_job, background);
            
            separator_kind s = tree->node.sep.sep_type;
            if (
                (s == SEP_AND && lhs_ret != 0) || 
                (s == SEP_OR && lhs_ret == 0) || 
                lhs_ret < 0
            )  {
                return lhs_ret;
            }
            return execute_ast(tree->node.sep.right, fds, grp_redirs, inherited_job, background);
        }
        case AST_GROUP:
        {
            INFO("exec_GROUP");
            const ast_node_group_t* grp = &tree->node.grp;
            if (grp->group_type == GROUP_SUBSHELL) {
                pid_t pid = fork();
                if (pid < 0) {
                    MSH_ERR("Couldn't fork process: %s%s%s%s", 
                        STYLE_BOLD, COLOR_BRIGHT_RED, strerror(errno), COLOR_RESET);
                    return EXIT_ERROR_FORKING;
                }
                if (pid == 0) {
                    // increase SHLVL
                    char* shlvl = getenv("SHLVL");
                    if (!shlvl || strlen(shlvl) == 0) {
                        exit(EXIT_ERROR_INTERNAL);
                    }
                    int shllvl_int = atoi(shlvl);
                    char buff[128];
                    sprintf(buff, "%d", shllvl_int + 1);
                    setenv("SHLVL", buff, 1);
                } else {
                    int status;
                    tcsetpgrp(STDIN_FILENO, pid);
                    if (waitpid(pid, &status, 0) < 0) {
                        MSH_ERR("Error whilst executing subshell: %s%s%s%s", 
                            STYLE_BOLD, COLOR_BRIGHT_RED, strerror(errno), COLOR_RESET);
                        return EXIT_ERROR_SUBSHELL;
                    }
                    tcsetpgrp(STDIN_FILENO, getpid());
                    if (WIFEXITED(status)) {
                        return WEXITSTATUS(status);
                    } else if (WIFSIGNALED(status)) {
                        return 128 + WTERMSIG(status);
                    }
                    return WEXITSTATUS(status);
                }
            }
            // Insert group redirections
            struct redir_arr new;
            error_t err = combine_grp_redirs(
                &grp->redirs,
                grp_redirs,
                &new
            );
            if (err < 0) {
                return err;
            }
            cmd_exit_t ret = execute_ast(tree->node.grp.children, fds, &new, inherited_job, background);
            free(new.data);
            if (grp->group_type == GROUP_SUBSHELL) {
                exit(ret);
            }
            return ret;
        }
        default:
            MSH_ERR("operation not yet supported");
            return EXIT_ERROR_UNIMPLEMENTED;
    }
}



static error_t execute_command(
    const ast_node_command_t* cmd,
    struct fds* fds,
    struct redir_arr* grp_redirs,
    job_t* job,
    _out_ bool* needs_to_be_waited_for
) {
    pid_t pid = 0;

    if (needs_to_be_waited_for) *needs_to_be_waited_for = true;

    struct file_streams fss = {
        .in = stdin, 
        .out = stdout, 
        .err = stderr
    };

    // Determine if external command needs to be overriden
    bool override = false;
    if (cmd->filename != NULL) {
        char** ow_table = g_overwrite_external;
        while (*ow_table) {
            if (!strcmp(*ow_table, cmd->argv[0])) {
                override = true;
                break;
            }
            ow_table++;
        }
    }

    // Figure out if its a builtin command
    bool is_builtin = false;
    int builtin_idx = 0;
    if (cmd->filename == NULL || override) {
        builtin_t* table = g_builtin_function_table;
        while (table->fptr != NULL) {
            if (!strcmp(cmd->argv[0], table->name)) {
                is_builtin = true;
                break;
            }
            table++;
            builtin_idx++;
        }
        if (!is_builtin) {
            MSH_ERR("unknown command %s%s%s%s%s", STYLE_BOLD, STYLE_UNDERLINE,
                COLOR_BRIGHT_RED, cmd->argv[0], COLOR_RESET);
            return EXIT_COMMAND_NOT_FOUND;
            
        }
    }

    bool must_fork = !is_builtin || job->pipe || job->background;


    if (must_fork) {
        pid = fork();
    }
    if (pid == -1) {
        MSH_ERR("Couln't not fork process: %s%s%s%s", 
            STYLE_BOLD, COLOR_BRIGHT_RED, strerror(errno), COLOR_RESET);
        return EXIT_ERROR_FORKING;
    }

    if (pid == 0) {
        /// CHILD PROCESS

        // Builtin command
        if (is_builtin) {
            int exit_code = g_builtin_function_table[builtin_idx].fptr(cmd->argc, cmd->argv, fss);
            if (must_fork) {
                exit(exit_code);
            } else {
                if (needs_to_be_waited_for) *needs_to_be_waited_for = false;
                return exit_code;
            }
        }
        INFO("isatty(stdout): %d", isatty(STDOUT_FILENO));

        // Group redirections
        if (grp_redirs->data && grp_redirs->sz > 0) {
            for (size_t i = 0; i < grp_redirs->sz; i++)
            {
                int code = eu_handle_redirection(&grp_redirs->data[i]);
                if (code < 0) exit(code);
            }
        }

        // Pipe redirections
        #define apply_and_close(src_fd, target_fd) do {\
            if (src_fd != -1 && target_fd != src_fd) {\
                dup2(src_fd, target_fd);\
                if (src_fd > STDERR_FILENO) {\
                    close(src_fd);\
                }\
            }\
        } while (0)
        apply_and_close(fds->in, STDIN_FILENO);
        apply_and_close(fds->out, STDOUT_FILENO);
        apply_and_close(fds->err, STDERR_FILENO);
        #undef apply_and_close
        if (fds->pipe_read != -1) {
            close(fds->pipe_read);
        }

        // Normal redirections
        if (cmd->nredirs && cmd->nredirs > 0) {
            for (size_t i = 0; i < cmd->nredirs; i++)
            {
                int code = eu_handle_redirection(&cmd->redirs[i]);
                if (code < 0) exit(code);
            }
        }
        // Set the PGID to itself or the pipeline pgid.
        setpgid(0, (job->pgid == 0) ? 0 : job->pgid);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        execve(cmd->filename, cmd->argv, environ);
        // Si ha ocurrido un error.
        perror(cmd->filename);
        // Terminar ejecuccion del proceso hijo.
        _exit(127);
    }
    // PARENT
    // Append pid to list 
    job->pids.data[job->pids.count++] = pid;
    if (job->pgid == 0) {
        job->pgid = pid;
    }
    setpgid(pid, job->pgid);
    return 0;
}

static error_t execute_pipeline(
    const ast_node_pipeline_t* ppl,
    struct fds* fds,
    struct redir_arr* grp_redirs,
    job_t* pipeline_job
) {
    struct fds new_fds = *fds;
    int pipe_fds[] = {-1, -1};

    for (size_t i = 0; i < ppl->nelements; i++)
    {
        bool is_last = i == ppl->nelements - 1;
        if (!is_last) {
            if ((pipe(pipe_fds)) == -1) {
                MSH_ERR("couldn't create pipe: %s", strerror(errno));
                return EXIT_ERROR_CREATING_PIPE;
            }
            new_fds.out = pipe_fds[1];
            new_fds.pipe_read = pipe_fds[0];
        }

        // Recurrsive call
        error_t err;
        if ((err = execute_ast(
            &ppl->elements[i], 
            &new_fds, 
            grp_redirs, 
            pipeline_job, 
            pipeline_job->background)
        ) < 0) {
            return err;
        }
        
        if (new_fds.in != fds->in) close(new_fds.in);
        if (new_fds.out != fds->out) close(new_fds.out);

        new_fds.in = pipe_fds[0];
    }
    return job_wait(pipeline_job);
    
}

static error_t combine_grp_redirs(
    const struct redir_arr* r1,
    const struct redir_arr* r2,
    _out_ struct redir_arr* out
) {
    error_t err = 0;
    out->data = NULL;
    out->sz = 0;
    if (!r1->sz && !r2->sz) {
        return 0;
    }
    ast_node_redir_t* new = calloc(r1->sz + r2->sz, sizeof(ast_node_redir_t));
    if (r1->sz && r1->data) {
        memcpy(new, r1->data, r1->sz * sizeof(ast_node_redir_t));
        for (size_t i = 0; i < r1->sz; i++) {
            ast_node_redir_t* r = &new[i];
            if (r->op == REDIR_OUT) {
                // Truncate the file here so that it only gets truncated once.
                // Otherwise it will get the truncated for every command inside a group.
                int ret = truncate(r->target.filename, 0);
                // We don't care if the file does not exist.
                if (ret != 0 && errno != ENOENT) {
                    err = EXIT_ERROR_OPENING_FILE;
                    MSH_ERR("couldn't truncate file '%s': %s", r->target.filename, strerror(errno));
                    goto __error_exit;
                }
                r->op = REDIR_OUT_APPEND;
            }
        }
    }
    if (r2->sz && r2->data) {
        memcpy(&new[r1->sz], r2->data, r2->sz * sizeof(ast_node_redir_t));
    }
    out->data = new;
    out->sz = r1->sz + r2->sz;
    return 0;

__error_exit:
    free(new);
    return err;
}