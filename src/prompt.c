/**
 * En este archivo se almacenan funciones para la personalizacion del "prompt".
 * El prompt es la informacion que aparece antes de donde el usuario escribe el commando.
 * Suele contener la ruta actual, el nombre usuario o la rama de git en la que se encuentra.
 * El prompt se personaliza en el archivo "prompt.h"
 */


#include <minishell.h>
#include <assert.h>

#define BUFF_SIZE 2048
#define BUFF_REMAINING 2048 - g_prompt_cur
static char g_prompt_buff[BUFF_SIZE];
static size_t g_prompt_cur = 0;

#define buff_cur ({\
    assert(g_prompt_cur < BUFF_SIZE);\
    g_prompt_buff + g_prompt_cur;\
})


#define append(fmt, ...) do {\
    int __written = snprintf(buff_cur, BUFF_REMAINING, fmt, __VA_ARGS__);\
    assert(__written > 0);\
    g_prompt_cur += __written;\
    g_prompt_buff[g_prompt_cur] = '\0';\
} while (0)

/**
 * @brief Obtener cadena del nombre del usuario.
 */
static inline void get_username(char** out) {
    *out = getenv("USER");
}

void prompt_clear() { g_prompt_cur = 0; g_prompt_buff[0] = '\0'; } 
char* prompt_get() { return g_prompt_buff; }


/**
 * @brief Imprime la ruta actual en la que se encuentra la terminal.
 * @note Esta function reemplaza la cadena /home/{usuario} con '~', al igual que bash. 
 */
void prompt_append_cwd(bool abrv_home) {
    char pwd[PATH_MAX];
    size_t sz_home = 0;
    
    getcwd(pwd, PATH_MAX);

    // Sustituir /home/{user} con ~
    if (abrv_home)
    {
        char* home = getenv("HOME");
        sz_home = strlen(home);

        // Si la subcadena /home/{user} esta al principio de la ruta ->
        if (strncmp(pwd, home, sz_home) == 0) {
            // Sustituir con ~
            append("~%s", pwd + sz_home);
            return;
        }
    }
    append("%s", pwd);
}

void prompt_append_username(void) {
    char* username;
    get_username(&username);
    append("%s", username);
}

void prompt_append_last_errorcode() {
    append("%d", g_last_error_code);
}

void prompt_append_str(char* s) { append("%s", s);}

void prompt_append_hostname() {
    char hostname[128];
    gethostname(hostname, 128);
    append("%s", hostname);
}

void prompt_append_git_branch() {
    FILE *f = popen("git rev-parse --abbrev-ref HEAD 2>/dev/null", "r");
    if (f == NULL) {
        return;
    }
    else {
        char branch[128];
        if (fgets(branch, sizeof(branch), f) != NULL) {
            // Eliminar salto de linea
            branch[strcspn(branch, "\n")] = 0;
            append("%s", branch);
        }
        pclose(f);
    }
}