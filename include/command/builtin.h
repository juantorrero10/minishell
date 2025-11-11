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
extern bool g_exit_signal;

// Funciones de comandos internos.
int builtin_exit (int, char**, struct file_streams);
int builtin_chdir(int, char**, struct file_streams);
int builtin_umask(int, char**, struct file_streams);
int builtin_jobs (int, char**, struct file_streams);
int builtin_fg   (int, char**, struct file_streams);



#endif // COMMAND_BUILTIN_H_