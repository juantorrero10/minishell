#ifndef ENV_H_
#define ENV_H_

#include <minishell.h>

// El SO almacena automaticamente todas las variables del entorno aqui.
extern char** environ;
extern size_t g_num_envvars;  // # de variables de entorno.

/**
 * @brief Obtener valor de variable del entorno a través de su nombre.
 * @param name nombre de la variable del entorno.
 * @param var_length [Salida] tamaño de la cadena de salida.
 * @return puntero a memoria al valor de la variable (dentro de environ).
 */
char* env_get_var(
    char* name, _out_ size_t* var_length);

#endif // ENV_H_