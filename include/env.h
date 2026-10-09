#ifndef ENV_H_
#define ENV_H_

#include <minishell.h>

// El SO almacena automaticamente todas las variables del entorno aqui.
extern char** environ;
extern size_t g_num_envvars;  // # de variables de entorno.

struct env_growable_string {
    char* data;
    size_t len;
    size_t cap;
};

/**
 * @brief Expands string into a newly allocated buffer or inside the provided one.
 * @param og Original string.
 * @param buff [Optional] if !NULL the new string (expanded or not) will be appended
 * inside the buffer
 * @return 
 *  Expanded string or NULL if no expansion was needed. 
 *  If a buffer was provided, it will return buff->data.
 * @note New string needs to be freed if no buffer provided.
 */
char* env_expand_string(
    const char* og, 
    _opt_ struct env_growable_string* buff
);

/**
 * @brief Expand the enviroment vars in the strings of the redirections
 */
void env_expand_redirs(ast_node_redir_t* redirs, size_t nredirs);

/**
 * @brief Expandir las variables del entorno en una linea de commandos entera.
 * @note Debido a la estructura del proyecto no se permiten variables del entorno 
 * en el primer argumento (el que dice el comando a ejecutar).
 * @returns devuelve una copia de los tokens originales y debe ser liberada
 *          tras su uso con free_tokens();
 */
void env_expand_ast(ast_t* ast);

#endif // ENV_H_