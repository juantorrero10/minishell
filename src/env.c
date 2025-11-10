/**
 * Funciones de las variables del sistema como $PATH.
 */
#include <minishell.h>
#include <log.h>

char* env_get_var(
    char* name, _out_ size_t* var_length) {

    char* curr_var = NULL;
    *var_length = 0;
    size_t name_sz = 0;
    char* eq = NULL;
    char* ret = NULL;

    for (size_t i = 0; i < g_num_envvars; i++)
    {
        curr_var = environ[i];
        eq = strchr(curr_var, '=');
        name_sz = (size_t)(eq - curr_var);
        curr_var[name_sz] = '\0';
        if (!strcmp(curr_var, name)) {
            curr_var[name_sz] = '=';
            ret = curr_var + name_sz + 1;
            *var_length = strlen(ret);
            return ret;
        }
        curr_var[name_sz] = '=';
        
    }
    if (!ret)WARN("'%s' was not found", name);
    return ret;
}