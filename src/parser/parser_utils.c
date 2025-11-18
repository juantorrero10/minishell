#include <mshparser.h>
#define __DEBUG
#include <../log.h>

// returns buff for convenience
const char* str_tok(typeof_token tt, char buff[])
{
    switch (tt) {
        case TOK_WORD:                 strcpy(buff, "WORD"); break;

        /* separators */
        case TOK_PIPE:                 strcpy(buff, "|"); break;
        case TOK_AND_IF:               strcpy(buff, "&&"); break;
        case TOK_OR_IF:                strcpy(buff, "||"); break;
        case TOK_SEMI:                 strcpy(buff, ";"); break;
        case TOK_AMP:                  strcpy(buff, "&"); break;

        /* groupers */
        case TOK_LPAREN:               strcpy(buff, "(  [SS]"); break;
        case TOK_CMD_ST_START:         strcpy(buff, "$( [ST]"); break;
        case TOK_CMD_ST_END:           strcpy(buff, ")  [ST]"); break;
        case TOK_RPAREN:               strcpy(buff, ")  [SS]"); break;
        case TOK_LBRACE:               strcpy(buff, "{"); break;
        case TOK_RBRACE:               strcpy(buff, "}"); break;

        /* redirections */
        case TOK_REDIR_OUT:            strcpy(buff, ">"); break;
        case TOK_REDIR_OUT_APPEND:     strcpy(buff, ">>"); break;
        case TOK_REDIR_IN:             strcpy(buff, "<"); break;
        case TOK_REDIR_HEREDOC:        strcpy(buff, "<<"); break;
        case TOK_REDIR_HERESTR:        strcpy(buff, "<<<"); break;
        case TOK_REDIR_DUP_OUT:        strcpy(buff, ">&"); break;
        case TOK_REDIR_DUP_IN:         strcpy(buff, "<&"); break;
        case TOK_REDIR_READ_WRITE:     strcpy(buff, "<>"); break;

        /* FD-prefix tokens */
        case TOK_REDIR_OUT_FD:         strcpy(buff, "FD_OUT"); break;
        case TOK_REDIR_IN_FD:          strcpy(buff, "FD_IN"); break;

        /* misc */
        case TOK_EOL:                  strcpy(buff, "EOL"); break;

        default:
        case TOK_ERROR:                strcpy(buff, "ERROR"); break;
    }
    return buff;
}

void pu_peek(token_arr* arr) {
    token_t* c;
    char buff[20];

    for (size_t i = 0; i < arr->occupied; i++)
    {
        c = arr->ptr + i;
        str_tok(c->type, buff);
        DUMP("%zu:\t%s,\t\tnum:%d\t\tval:%s", i, buff, c->number, c->value);

    }
    
}



int pu_check_balance(char* cmdline, size_t view) {
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