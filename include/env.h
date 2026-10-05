#ifndef ENV_H_
#define ENV_H_

#include <minishell.h>

// El SO almacena automaticamente todas las variables del entorno aqui.
extern char** environ;
extern size_t g_num_envvars;  // # de variables de entorno.

/**
 * @brief Expandir las variables del entorno en una linea de commandos entera.
 * @note Debido a la estructura del proyecto no se permiten variables del entorno 
 * en el primer argumento (el que dice el comando a ejecutar).
 * @returns devuelve una copia de los tokens originales y debe ser liberada
 *          tras su uso con free_tokens();
 */
void env_expand_ast(ast_t* ast);

#endif // ENV_H_