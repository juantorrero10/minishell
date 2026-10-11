#include <minishell.h>
#include <log.h>

#include <parser/public.h>


static bool g_print_prompt = true;

size_t read_additional_line(_out_ char** ptr) {
    if (!ptr) return 0;
    if (g_print_prompt) {
        prompt_clear();
        PROMPT_COLOR_GREY();
        prompt_append_str("> ");
        PROMPT_COLOR_RESET();
    }
    *ptr = readline(prompt_get());
    return strlen(*ptr);
}

int read_line_input(char* buff, size_t max, bool print_prompt) {
    size_t line_len = 0;
    size_t total = 0;
    int ret = 0;
    
    g_print_prompt = print_prompt;
    // Imprimir el "prompt"
    if (print_prompt) {
        prompt_clear();
        if (g_prompt_push_fptr) {
            g_prompt_push_fptr();
        } else {
            DEFAULT_PROMPT_PUSH();
        }
    }
    char* line = readline(prompt_get());
    
    // EOF
    if (!line) {
        rl_on_new_line();
        exit(g_last_error_code);
    }

    // Ignorar lineas que que empiezan con '#' (comentarios)
    if (line[0] == '#') {
        return 0;
    }
    total = strlen(line);
    strcpy(buff, line);
    free(line);
    
    // Permitir varias lineas con '\'
    size_t i = 0;
    while (i < total) {
        char* s = strstr(buff + i, "\\\n");
        if (!s) break;
        i = (size_t)((uintptr_t)s - (uintptr_t)buff);
        memmove(s, s + 2, total - i - 2 + 1); // +1 copy Null termination as well
        total -= 2;
    }
    while (total != 0 && buff[total - 1] == '\\') {
        // Eliminar la barra invertida
        buff[total - 1] = '\0';
        line_len--; total--;

        // Obtener siguiente linea
        char* add_line;
        size_t add_len = read_additional_line(&add_line);

        // EOF
        if (!add_line) {
            rl_on_new_line();
        }
        
        if (total + add_len < max) {
            strcat(buff, add_line);
            total += add_len;
        } else {
            MSH_ERR("Input line exceeds maximum length (%zu)", max);
            free(add_line);
            return -1;
        }
        free(add_line);
    }

    add_history(buff);

    // tokenizar
    //line = tokenize(buff);
    ast_t* a = parse_string(buff);
    if (a != NULL) {

        // ejecutar commando.
        ret = execute_line(a, buff);
        
        // Actualizar el último codigo de error. $STATUS
        g_last_error_code = ret;
        setenv("last", buff, 1);
        sprintf(buff, "%d", ret);
        setenv("status", buff, 1);
        

        // señal de salida = 1 -> salir de la shell con código ret.
        if (g_exit_signal == 1) {
            INFO("exit signal=1");
	        bexit();
            exit(ret);
        }
        // señal de salida = 2 -> se ha cancelado el diálogo de salida.
        if (g_exit_signal == 2) {
            fputc('\n', stdout);
            g_exit_signal = 0;
            INFO("end, signal=2");
            return ret;
        }
        INFO("end");
        return ret;
    }
    return -1;
}