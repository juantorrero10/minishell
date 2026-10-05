#ifndef COMMAND_BUILTIN_H_
#define COMMAND_BUILTIN_H_

/**
 * Aclaracion: "builtin" -> comando interno.
 */

// Deficion de funcion de commando interno
typedef int (*builtin_function_t)(int argc, char** argv, struct file_streams fss);

typedef struct _builtin_function_struct
{
    char* name;
    builtin_function_t fptr;
    
}builtin_t;

// Tabla global de punteros a funciones.
extern builtin_t g_builtin_function_table[];

// Señal de salida
extern int g_exit_signal;

// Funciones de comandos internos.
#define def_builtin(name) int name(int argc, char** argv, struct file_streams fss)
def_builtin(builtin_exit);
def_builtin(builtin_chdir);
def_builtin(builtin_umask);
def_builtin(builtin_jobs);
def_builtin(builtin_fg);
def_builtin(builtin_set);
def_builtin(builtin_unset);
def_builtin(builtin_kill);
def_builtin(builtin_getpid);



#endif // COMMAND_BUILTIN_H_