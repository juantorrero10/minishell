/**
 * En este archivo se almacenan funciones para la personalizacion del "prompt".
 * El prompt es la informacion que aparece antes de donde el usuario escribe el commando.
 * Suele contener la ruta actual, el nombre usuario o la rama de git en la que se encuentra.
 * El prompt se personaliza en el archivo "prompt.h"
 */


#include <minishell.h>

/**
 * @brief Obtener cadena del nombre del usuario.
 */
static void get_username(char** out) {
    size_t len = 0;

    *out = env_get_var("USER", &len);
}

int prompt_get_last_errorcode() { return g_last_error_code;}

/**
 * @brief Imprime la ruta actual en la que se encuentra la terminal.
 * @note Esta function reemplaza la cadena /home/{usuario} con '~', al igual que bash. 
 */
void prompt_print_cwd(bool abrv_home) {
    char pwd[128];
    char home[128];
    size_t sz_home = 0;
    
    getcwd(pwd, 128);

    // Sustituir /home/{user} con ~
    if (abrv_home)
    {
        strcpy(home, env_get_var("HOME", &sz_home));

        // Si la subcadena /home/{user} esta al principio de la ruta ->
        if (strncmp(pwd, home, sz_home) == 0) {
            // Sustituir con ~
            printf("~%s", pwd + sz_home);
            return;
        }
    }
    printf("%s", pwd);
}

void prompt_print_username(void) {
    char* username;
    get_username(&username);
    printf("%s", username);
}

void prompt_print_last_errorcode() {
    printf("%d", prompt_get_last_errorcode());
}

void prompt_print_str(char* s) { printf("%s", s);}