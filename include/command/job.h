#ifndef COMMAND_JOB_H_
#define COMMAND_JOB_H_

#include <minishell.h>

/*
* DONE e INTERRUPT son distintos porque desaparecen una vez impresos.
*/
typedef enum {RUNNING=0, STOPPED, WAITING, DONE=10, INTERRUPTED}job_state;

typedef struct _job_desc {
    pid_t *pids;          // PIDs de los procesos hijos.
    pid_t pgid;           // ID de grupo.
    int id;
    job_state state;
    int nprocceses;
    int background;
    char priority;        //'+', '-' o ' '
    char *cmdline;
    struct _job_desc* next;
    
} job_t, *job_llist;

// Lista enlazada global de trabajos en segundo plano
extern job_llist g_bgjob_list;

// Numero de trabajos en segundo plano.
extern size_t g_sz_jobs;

// Trabajo actual en primer plano.
extern job_t* g_curr_fg_job;

/**
 * @brief Inserta un trabajo al final de la lista
 * @returns id del 
 */
int job_add(job_t j);

job_t* job_get(pid_t pgid);
pid_t job_get_pid(int id);

/**
 * @brief lleva un trabajo al primer plano y lo elimina de la lista.
 */
int job_fg(pid_t pgid);

/**
 * @brief lleva un trabajo al segundo plano y lo añade a la lista
 */
int job_bg(pid_t pgid);

/**
 * @brief envia señales a los trabajos.
 */
int job_stop(pid_t pgid);
int job_resume(pid_t pgid);
int job_interrupt(pid_t pid);
int job_kill(pid_t pid);

void job_print(job_t* j, FILE* stream);

job_state job_get_status(pid_t pgid);
void job_update_status();
void job_checkupdate(job_t* j, job_state new, job_state old, bool notify);




#endif // COMMAND_JOB_H_