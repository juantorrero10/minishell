#include <mshparser.h>

static int check_balance(char* cmdline, size_t view) {
    char saved = cmdline[view];
    cmdline[view] = '\0';

    const char *p = cmdline;
    int ret = 0;

    char stack[512];
    size_t top = 0;

    int in_squote = 0;
    int in_dquote = 0;

    while (*p) {
        char c = *p;

        /* ---------------- QUOTES ---------------- */

        if (!in_squote && c == '"') {
            in_dquote = !in_dquote;
            p++;
            continue;
        }
        if (!in_dquote && c == '\'') {
            in_squote = !in_squote;
            p++;
            continue;
        }

        /* inside quotes: ignore structure */
        if (in_squote || in_dquote) {
            p++;
            continue;
        }

        /* ---------------- PARENTHESES/BRACES ---------------- */

        if (c == '(' || c == '{') {
            if (top >= sizeof(stack)) {
                error_parse(ERR_NEST_TOO_DEEP, NULL);
                ret = 1;
                goto done;
            }
            stack[top++] = c;
        }
        else if (c == ')' || c == '}') {
            if (top == 0) {
                error_parse(ERR_UNTERM_PAREN, NULL);
                ret = 1;
                goto done;
            }

            char open = stack[top - 1];
            if ((c == ')' && open != '(') ||
                (c == '}' && open != '{')) {
                error_parse(ERR_MISMATCHED_BRACKET, NULL);
                ret = 1;
                goto done;
            }

            top--;
        }

        p++;
    }

    /* ---------------- AFTER SCAN: CHECK STATES ---------------- */

    if (in_dquote) {
        error_parse(ERR_UNTERM_DQUOTE, NULL);
        ret = 1;
        goto done;
    }
    if (in_squote) {
        error_parse(ERR_UNTERM_SQUOTE, NULL);
        ret = 1;
        goto done;
    }
    if (top != 0) {
        error_parse(ERR_UNTERM_PAREN, NULL);
        ret = 1;
        goto done;
    }
done:
    cmdline[view] = saved;
    return ret;
}

// Main procedure

ast_t* parse_command(char* cmdline) {
    size_t sz = 0;
    token_arr arr = {NULL, 0, 0}; (void)arr;


    if (check_balance(cmdline, strlen(cmdline)))return NULL;
    arr = tokenize(cmdline, &sz);

    free_token_arr(&arr);
    return NULL;
}

/*
static ast_t* parse_list();
static ast_t* parse_pipeline();
static ast_t* parse_simple_command();
static ast_t* parse_redirection(ast_t*);

*/