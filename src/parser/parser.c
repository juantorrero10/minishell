#include <mshparser.h>





// Main procedure

ast_t* parse_command(char* cmdline) {
    int sz = 0;
    token_arr arr = {NULL, 0, 0}; (void)arr;
    char* trim_chars = "\t\r\n ";
    sz = strlen(cmdline);
    
    //trim characters
    for (int i = ((int)sz)-1; i >= 0; i--)
    {
        if (strchr(trim_chars, cmdline[i])) {
            cmdline[i] = '\0';
            sz--;
        } else break;
    }
    

    if (pu_check_balance(cmdline, strlen(cmdline)))return NULL;
    arr = tokenize(cmdline, &sz);
    if (sz) goto clean_exit;
    pu_peek(&arr);

clean_exit:
    free_token_arr(&arr);
    return NULL;
}

/*
static ast_t* parse_list();
static ast_t* parse_pipeline();
static ast_t* parse_simple_command();
static ast_t* parse_redirection(ast_t*);

*/