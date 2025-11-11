#ifndef COMMAND_EXECUTE_H_
#define COMMAND_EXECUTE_H_


#define MSH_LOG(MSG, ...)   COLOR_BRIGHT_BLUE(stdout);                               \
                            fprintf(stdout, "minishell: ");                         \
                            COLOR_RESET(stdout);                                    \
                            fprintf(stdout, MSG"\r\n", ##__VA_ARGS__)

#define MSH_ERR(MSG, ...) COLOR_RED(stderr);                                        \
                            fprintf(stderr, "minishell: ");                         \
                            COLOR_RESET(stderr);                                    \
                            fprintf(stderr, MSG"\r\n", ##__VA_ARGS__)

#define MSH_LOG_C(MSG, ...) COLOR_BRIGHT_BLUE(fss.out);                                                                      \
                            fprintf(fss.out, "minishell: ");                         \
                            COLOR_RESET(fss.out);                                    \
                            fprintf(fss.out, MSG"\r\n", ##__VA_ARGS__)

#define MSH_ERR_C(MSG, ...) COLOR_RED(fss.err);                                                             \
                            fprintf(fss.err, "minishell: ");                         \
                            COLOR_RESET(fss.err);                                    \
                            fprintf(fss.err, MSG"\r\n", ##__VA_ARGS__)

typedef struct job {
    pid_t *pids;          // PIDs de los procesos hijos.
    pid_t pgid;           // ID de grupo.
    int nprocceses;
    int background;
    char *cmdline;
} job_t;

struct file_streams {
    FILE* out;
    FILE* in;
    FILE* err;
};

int execute_command(tline* tokens, const char* cmdline);

#endif // COMMAND_EXECUTE_H_