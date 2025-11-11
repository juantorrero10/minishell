/**
 * Funciones de manejo de variables del entorno
 */
#include <minishell.h>

#include <log.h>

#define MAX_VARNAME_COPY 256

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

char* env_expand_vars(const char* fmt, size_t* length) {
    size_t idx = 0, out_len = 0, start = 0;
    size_t alloc_size = 1;
    char* output = calloc(1, alloc_size);
    bool paren = false;
    char varname[MAX_VARNAME_COPY];

    if (!fmt) return NULL;
    if (!output) return NULL;
    while (fmt[idx]) {
        if (fmt[idx] == '$') {
            idx++;

            paren = 0;
            start = idx;

            // Variables con parentesis -> $(USER)
            if (fmt[idx] == '(') {
                paren = true;
                idx++;
                start = idx;
                while (fmt[idx] && fmt[idx] != ')') idx++;
                if (!fmt[idx]) {
                    // No hay ')', tratar literalmente
                    output = realloc(output, alloc_size + 2);
                    output[out_len++] = '$';
                    output[out_len++] = '(';
                    output[out_len] = '\0';
                    idx = start;
                    continue;
                }
            } else {
                // si no tiene parentesis, buscar hasta caracter no alfanumero o final de cadena
                while (isalnum(fmt[idx]) || fmt[idx] == '_')
                    idx++;
            }

            size_t var_len = idx - start;
            if (paren && fmt[idx] == ')') idx++; // saltar ')'

            // Copiar variable, no dejar copiar mas de MAX_VARNAME_COPY bytes (256 si no lo he cambiado)
            memset(varname, 0, MAX_VARNAME_COPY);
            if (var_len >= MAX_VARNAME_COPY) var_len = MAX_VARNAME_COPY - 1;
            strncpy(varname, fmt + start, var_len);
            varname[var_len] = '\0';

            // Obtener variable de entorno
            size_t val_len = 0;
            char* env_value = env_get_var(varname, &val_len);
            if (!env_value) env_value = " ";

            // Concatenar
            alloc_size += val_len;
            output = realloc(output, alloc_size);
            memcpy(output + out_len, env_value, val_len);
            out_len += val_len;
            output[out_len] = '\0';
        } else {
            // Copiar literalmente si no es una variable
            alloc_size++;
            output = realloc(output, alloc_size);
            output[out_len++] = fmt[idx++];
            output[out_len] = '\0';
        }
    }

    *length = out_len;
    return output;
}


tline* env_expand_wholeline(const tline* og) {
    tline* new = (tline*) malloc(sizeof(tline));
    size_t sz = 0;
    tcommand c = {0};
    char* tmp = NULL;

    // Copiar los contenidos "estaticos"
    new->ncommands = og->ncommands;
    new->background = og->background;
    
    // Copiar las variables expandidas
    new->redirect_error = env_expand_vars(og->redirect_error, &sz);
    new->redirect_input = env_expand_vars(og->redirect_input, &sz);
    new->redirect_output = env_expand_vars(og->redirect_output, &sz);

    new->commands = (tcommand*) malloc(sizeof(tcommand) * og->ncommands);

    for (int i = 0; i < og->ncommands; i++)
    {
        c = (tcommand){0};
        new->commands[i].argc = og->commands[i].argc;

        //Copiar filename.
        new->commands[i].filename = NULL;
        new->commands[i].argv = NULL;
        if (og->commands[i].filename) {
            new->commands[i].filename = malloc(strlen(og->commands[i].filename));
            strcpy(new->commands[i].filename, og->commands[i].filename);
        }

        // Hacer la tabla de punteros (argv**) y copiar argv[0]
        if (og->commands[i].argv && og->commands[i].argv[0]) {
            sz = strlen(og->commands[i].argv[0]) + 1;
            c.argv = malloc(sizeof(char*) * og->commands[i].argc);
            c.argv[0] = malloc(sz);
            strcpy(c.argv[0], og->commands[i].argv[0]);
        }
        
        // Expandir el resto de argumentos
        for (int j = 1; j < og->commands[i].argc; j++)
        {
            tmp = env_expand_vars(
                og->commands[i].argv[j], &sz);
            
            c.argv[j] = malloc(sz + 1);
            memcpy(c.argv[j], tmp, sz + 1);
        }
        new->commands[i].argv = c.argv;
    }
    return new;
}