#include <minishell.h>
#include <log.h>

int job_add(job_t j) {
    job_t* new = malloc(sizeof(job_t));
    size_t sz = 0;
    job_t* curr, *prev;
    int highest = 0;

    //----  copiar contenidos. ----
    new->background = j.background;
    new->nprocceses = j.nprocceses;
    new->pgid = j.pgid;
    new->state = j.state;
    new->next = NULL;
    new->priority = 0;
    new->id = 1;
    
    sz = strlen(j.cmdline) + 1;
    new->cmdline = malloc(sz);
    memcpy(new->cmdline, j.cmdline, sz);

    sz = sizeof(pid_t) * j.nprocceses;
    new->pids = malloc(sz);
    memcpy(new->pids, j.pids, sz);


    if (g_bgjob_list == NULL) {
        g_bgjob_list = new;
        g_sz_jobs++;
        return 1;
    }
    curr = g_bgjob_list;
    prev = NULL;
    do {
        if (curr->priority > highest) highest = curr->priority;
        prev = curr;
        curr = curr->next;
    } while (curr);
    prev->next = new;
    new->next = NULL;
    new->id = prev->id + 1;
    new->priority = highest + 1;
    g_sz_jobs++;
    return new->id;
}

static void job_free(job_t* j) {
    if(!j) return;
    if(j->cmdline)free(j->cmdline);
    if(j->pids)free(j->pids);
    free(j);
}

void job_rm(pid_t pgid) {
    job_t* curr, *next;
    
    if (g_bgjob_list == NULL) return;
    curr = g_bgjob_list;
    if (curr->pgid == pgid) {
        g_bgjob_list = curr->next;
        job_free(curr);
        g_sz_jobs--;
        return;
    }
    next = curr->next;
    if (!next) return;
    do {
        if (next->pgid == pgid) break;
        curr = curr->next;
        next = curr->next;
    } while(next);
    if (next) {
        curr->next = next->next;
        g_sz_jobs--;
        job_free(next);
    }
}

job_t* job_get(pid_t pgid) {
    job_t* r = NULL;
    job_t* c = NULL;

    if (g_bgjob_list == NULL) return r;
    c = g_bgjob_list;
    for (int i = 0; i < c->nprocceses; i++)
    {
        if (c->pids[i] == pgid) {
            return c;
        }
    }
    
    while(c->next != NULL) {
        c = c->next;
        for (int i = 0; i < c->nprocceses; i++)
        {
            if (c->pids[i] == pgid) {
                return c;
            }
        }
    }
    return r;
}

job_t* job_get_plus() {
    job_t* curr = NULL;
    job_t* highest = NULL;

    if (!g_bgjob_list) return NULL;
    curr = g_bgjob_list;
    highest = curr;
    do {
        if (curr->priority > highest->priority) highest = curr;
        curr = curr->next;
    } while(curr);
    return highest;
}

int job_fg(pid_t pgid) {return pgid;}

pid_t job_get_pid(int id) {
    job_t* curr;
    pid_t ret = -1;

    if (!g_bgjob_list)return ret;
    curr = g_bgjob_list;
    do {
        if (curr->id == id) return curr->pgid;
        curr = curr->next;
    } while (curr);
    return ret;
}

int job_stop(pid_t pgid) {return pgid;}
int job_resume(pid_t pgid) {return pgid;}
int job_interrupt(pid_t pgid) {return pgid;}
int job_kill(pid_t pgid){return pgid;}


static void str_state(job_state s, char* buff) {
    switch (s)
    {
    case RUNNING:       strcpy(buff, "Running");break;
    case STOPPED:       strcpy(buff, "Stopped");break;
    case DONE:          strcpy(buff, "Done");break;
    default:
        strcpy(buff, "Unknown");break;
    }
}

void job_print(job_t* j, FILE* stream, char priority){
    char buff[32];
    
    str_state(j->state, buff);
    if(j->state == DONE) priority = ' ';
    fprintf(stream, "[%d]%c  %s\t\t%s\t{%d}\n", j->id, priority, buff, j->cmdline, j->pgid);
    if (j->state >= 10) {
        job_rm(j->pgid);
    }
}

job_state job_get_status(pid_t pgid) {
    FILE *f;
    char s[20];
    char buff[256];
    char* ptr;
    char state;
    size_t idx = 0;
    size_t sz = 0;
    job_t* job;

    sprintf(s, "/proc/%d/stat", pgid);
    f = fopen(s, "r");
    if (!f) {
        
        ERROR("Couldn't get <%d>'s status: %s", pgid, strerror(errno));
        return -1;
    }
    memset(buff, 0, (size_t)256);
    fgets(buff, 256, f);
    if ((sz = strlen(buff)) == 0) {return -1;}

    ptr = strchr(buff, ' ');
    idx = (size_t)(ptr - buff);
    ptr = strchr(buff + idx + 1, ' ');
    idx = (size_t)(ptr - buff);
    state = buff[idx + 1];
    switch (state)
    {
    case 'S':
    case 'R':
        return RUNNING;
    case 'T':
        return STOPPED;
    default:
        job = job_get(pgid);
        job->priority = 0;
        return DONE;
    }
}

void job_checkupdate(job_t* j, job_state new, job_state old, bool notify) {
    if ((int)new == -1) return;
    if (new == old) return;
    j->state = new;
    if (new == STOPPED && old == RUNNING && notify) {
        MSH_LOG("Job [%d] '%s' stopped\t{%d}", j->id, j->cmdline, j->pgid);
    }
}

void job_update_status() {
    job_t* curr;
    job_state cs;

    if (!g_bgjob_list) return;
    curr = g_bgjob_list;
    do {
        cs = job_get_status(curr->pgid);
        job_checkupdate(curr, cs, curr->state, true);
        curr = curr->next;
    } while(curr);
}