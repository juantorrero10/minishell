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

/**
 * @brief expandir variables de entorno en una cadena
 * @example "aaaa$HOME$USER $HOSTTYPE" -> "aaaa/home/{usuario}{usuario} x86_64"
 * @attention la funcion que llama a esta es responsable de liberar la memoria del buffer de salida.
 */
char* env_expand_vars(const char* fmt, _out_ size_t* length);

/**
 * @brief Expandir las variables del entorno en una linea de commandos entera.
 * @note Debido a la estructura del proyecto no se permiten variables del entorno 
 * en el primer argumento (el que dice el comando a ejecutar).
 * @returns devuelve una copia de los tokens originales y debe ser liberada
 *          tras su uso con free_tokens();
 */
tline* env_expand_wholeline(const tline* og);

#endif // ENV_H_