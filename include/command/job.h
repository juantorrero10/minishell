#ifndef COMMAND_JOB_H_
#define COMMAND_JOB_H_

#include <minishell.h>

#define JOB_INVALID_ID (int)0


typedef enum {RUNNING=0, STOPPED, DONE, SLEEPING}job_state;


struct pid_array {
    int count;
    pid_t* data;
};



typedef struct _job_desc {
    int id;
    job_state state;
    bool background;
    bool pipe;
    int priority;        
    pid_t pgid;                     // ID de grupo.
    const char *cmdline;
    struct pid_array pids;          // PIDs de los procesos hijos.
    struct _job_desc* next;
    
} job_t, *job_llist;

// Lista enlazada global de trabajos en segundo plano
extern job_llist g_bgjob_list;

// Numero de trabajos en segundo plano.
extern size_t g_sz_jobs;

// Funciones para manejar trabajos en segundo plano.
int         job_add             (const job_t* j);
void        job_rm              (pid_t pgid);
job_t*      job_get             (pid_t pgid);
job_t*      job_get_plus        ();
pid_t       job_get_pid         (int id);
void        job_print           (job_t* j, FILE* stream, char priority);
job_state   job_get_status      (pid_t pgid);
pid_t       job_get_pid         (int id);
void        job_update_status   ();
void        job_checkupdate     (job_t* j, job_state new, job_state old, bool notify);
cmd_exit_t  job_wait            (job_t* j);




#endif // COMMAND_JOB_H_