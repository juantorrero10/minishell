#ifndef COMMAND_BUILTIN_H_
#define COMMAND_BUILTIN_H_

/**
 * Aclaracion: "builtin" -> comando interno.
 */

// Deficion de funcion de commando interno
typedef int (*builtin_function_t)(int argc, char** argv);

typedef struct _builtin_function_struct
{
    char* name;
    builtin_function_t fptr;
    
}builtin_t;

// Tabla global de punteros a funciones.
extern builtin_t g_builtin_function_table[];

// Funciones de comandos internos.
int builtin_exit (int, char**);
int builtin_chdir(int, char**);
int builtin_echo (int, char**);
int builtin_umask(int, char**);
int builtin_jobs (int, char**);
int builtin_fg   (int, char**);



#endif // COMMAND_BUILTIN_H_