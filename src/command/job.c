#include <minishell.h>
#include <log.h>

/**
 * @brief Añadir un nuevo trabajo a la lista con la prioridad mas alta.
 * @param j trabajo a añadir.
 * @return int ID del trabajo añadido.
 */
int job_add(job_t j) {
    job_t* new = malloc(sizeof(job_t));
    size_t sz = 0;
    job_t* curr, *prev;
    int highest = 0;

    //Copiar contenidos
    new->background = j.background;
    new->nprocceses = j.nprocceses;
    new->pgid = j.pgid;
    new->state = j.state;
    new->next = NULL;
    new->priority = 0;
    new->id = 1;
    

    new->cmdline = strdup(j.cmdline);

    sz = sizeof(pid_t) * j.nprocceses;
    new->pids = malloc(sz);
    memcpy(new->pids, j.pids, sz);


    if (g_bgjob_list == NULL) {
        g_bgjob_list = new;
        g_sz_jobs++;
        return 1;
    }
    // Añadir al final de la lista con prioridad mas alta.
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

// Liberar memoria de un objeto job.
static void job_free(job_t* j) {
    if(!j) return;
    if(j->cmdline)free(j->cmdline);
    if(j->pids)free(j->pids);
    free(j);
}

/**
 * @brief Eliminar el trabajo con el pgid especificado.
 * @param pgid ID de grupo del trabajo a eliminar.
 */
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

/**
 * @brief Obtener el trabajo con el pgid especificado.
 * @param pgid ID de grupo del trabajo a obtener.
 * @return job_t* puntero al trabajo o NULL si no se encuentra.
 */
job_t* job_get(pid_t pgid) {
    job_t* r = NULL;
    job_t* c = NULL;

    if (g_bgjob_list == NULL) return r;
    c = g_bgjob_list;
    do {
        for (int i = 0; i < c->nprocceses; i++)
        {
            if (c->pids[i] == pgid)return c;
        }
        c = c->next;
        
    } while (c);
    return r;
}

/**
 * @brief Obtener el trabajo con mayor prioridad.
 * @return job_t* puntero al trabajo o NULL si no hay trabajos.
 */
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

/**
 * @brief Obtener el pgid del trabajo con el id especificado.
 * @param id ID del trabajo.
 * @return pid_t pgid del trabajo o -1 si no se encuentra.
 */
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

// Convertir estado a cadena para imprimir en pantalla.
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

/**
 * @brief Imprimir un trabajo en un stream.
 * @param j trabajo a imprimir.
 * @param stream donde imprimir.
 * @param priority caracter de prioridad ('+', '-', ' ').
 * @note Al igual que en bash, los trabajos terminados se eliminan al imprimirlos.
 */
void job_print(job_t* j, FILE* stream, char priority){
    char buff[32];
    
    str_state(j->state, buff);
    if(j->state == DONE) priority = ' ';
    fprintf(stream, "[%d]%c %s\t\t%s\t{%d}\n", j->id, priority, buff, j->cmdline, j->pgid);
    if (j->state == DONE) {
        job_rm(j->pgid);
    }
}

/**
 * @brief Obtener el estado actual de un trabajo.
 * @param pgid ID de grupo del trabajo.
 * @return job_state estado del trabajo.
 * @note se lee /proc/[pgid]/stat para obtener el estado y detectar cambios.
 */
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
        if (job)job->priority = 0;
        return DONE;
    }
}

/**
 * @brief actualizar y notificar cambio de estado de un trabajo.
 * @param j trabajo a actualizar.
 * @param new nuevo estado.
 * @param old estado anterior.
 * @param notify si es true notificar el cambio de estado.
 */
void job_checkupdate(job_t* j, job_state new, job_state old, bool notify) {
    if ((int)new == -1) return;
    if (new == old) return;
    j->state = new;
    if (new == STOPPED && old == RUNNING && notify) {
        MSH_LOG("Trabajo [%d] '%s' detenido.\t{%d}", j->id, j->cmdline, j->pgid);
    }
}

/**
 * @brief Actualizar el estado de todos los trabajos en la lista.
 */
void job_update_status() {
    job_t* curr;
    job_state cs;

    if (!g_bgjob_list) return;
    curr = g_bgjob_list;
    do {
        cs = job_get_status(curr->pgid);
        //Si ya no existe el proceso pero si sigue habiendo descriptor de trabajo. 
        if ((int)cs == -1) {curr->state = DONE; job_checkupdate(curr, cs, curr->state, false);}
        else job_checkupdate(curr, cs, curr->state, true);
        
        INFO("job_checkupdate: %d -> %d", curr->state, cs);
        curr = curr->next;
    } while(curr);
}