#include <minishell.h>
#include <log.h>
/**
 * @brief Handler de la SIGINT que no hace nada relevante.
 */
void sigint_handler(int sig) {
    (void)sig; fflush(stdout);
}

void sigtstp_handle(int sig) {
    WARN("SIGTSTP");
    (void)sig; fflush(stdout);
    
    // Reinstalar señal.
    signal(SIGTSTP, sigtstp_handle);
}

void sigchld_handler(int sig) {
    pid_t pid;
    int status;
    bool all_done;

    (void)sig;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        job_t* j = job_get(pid);
        if (!j) continue;

        // Marcar este proceso como terminado
        for (int i = 0; i < j->nprocceses; i++) {
            if (j->pids[i] == pid)
                j->pids[i] = -1;
        }

        // 
        all_done = true;
        for (int i = 0; i < j->nprocceses; i++) {
            if (j->pids[i] != -1) {
                all_done = false;
                break;
            }
        }

        if (all_done) {
            printf("\n");
            MSH_LOG("job [%d] '%s' done\t{%d}", j->id, j->cmdline, j->pgid);
            INFO("DONE: %d", DONE);
            j->state = DONE;
        }
    }

    // Reinstalar señar, se pone por defecto en cada uso.
    signal(SIGCHLD, sigchld_handler);
}