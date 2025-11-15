#ifndef COMMAND_JOB_H_
#define COMMAND_JOB_H_

#include <minishell.h>


typedef enum {RUNNING=0, STOPPED, DONE}job_state;

typedef struct _job_desc {
    pid_t *pids;          // PIDs de los procesos hijos.
    pid_t pgid;           // ID de grupo.
    int id;
    job_state state;
    int nprocceses;
    int background;
    int priority;        
    char *cmdline;
    struct _job_desc* next;
    
} job_t, *job_llist;

// Lista enlazada global de trabajos en segundo plano
extern job_llist g_bgjob_list;

// Numero de trabajos en segundo plano.
extern size_t g_sz_jobs;

// Funciones para manejar trabajos en segundo plano.
int         job_add             (job_t j);
void        job_rm              (pid_t pgid);
job_t*      job_get             (pid_t pgid);
job_t*      job_get_plus        ();
pid_t       job_get_pid         (int id);
void        job_print           (job_t* j, FILE* stream, char priority);
job_state   job_get_status      (pid_t pgid);
pid_t       job_get_pid         (int id);
void        job_update_status   ();
void        job_checkupdate     (job_t* j, job_state new, job_state old, bool notify);




#endif // COMMAND_JOB_H_