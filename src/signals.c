#include <minishell.h>
#include <log.h>
/**
 * @brief Handler de SIGINT (Ctrl + C)
 */
void sigint_handler(int sig) {
    (void)sig; fflush(stdout);
}

/**
 * @brief Handler de SIGCHLD (creación de un proceso hijo) con fork();
 * Marca los trabajos como DONE cuando todos sus procesos hijos han terminado.
 */
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
            if (!g_dont_nl)printf("\n");
            MSH_LOG("Trabajo [%d] '%s' finalizó\t{%d}", j->id, j->cmdline, j->pgid);
            INFO("DONE: %d", DONE);
            j->state = DONE;
        }
    }
}