#include <minishell.h>

/**
 * @brief Get username through getlogin()
 * if it fails use getuid() 
 */
static void get_username(char** out) {
    *out = getlogin();

    if (!*out) {
        struct passwd *pw = getpwuid(getuid());
        if (pw)
            *out = pw->pw_name;
    }
    if (!*out) strcpy(*out, "unknown");
}

void prompt_print_cwd() {
    char buff[128];
    getcwd(buff, 128);

    // Substitude /home/{user} with ~
    char buff2[128];
    strcpy(buff2, "/home/");
    char* username; get_username(&username);
    strcat(buff2, username);

    // If /home/{user} substring is at the start of the string.
    if (strncmp(buff, buff2, strlen(buff2)) == 0) {
        // Print ~ + rest of path
        printf("~%s", buff + strlen(buff2));
    } else {
        printf("%s", buff);
    }
}

void prompt_print_username(void) {
    char* username;
    get_username(&username);
    printf("%s", username);
}

void prompt_print_last_errorcode() {
    printf("%ld", 0L);
}

void prompt_print_str(char* s) { printf("%s", s);}