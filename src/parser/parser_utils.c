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
        case TOK_DQ_START:             strcpy(buff, "\" (+)"); break;
        case TOK_DQ_END:               strcpy(buff, "\" (-)"); break;

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
        case TOK_REDIR_RHS_FD:         strcpy(buff, "FD_RHS"); break;
        case TOK_REDIR_LHS_FD:          strcpy(buff, "FD_LHS"); break;
        case TOK_REDIR_RHS_CLOSE:          strcpy(buff, "-"); break;

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

token_cat get_category(typeof_token tt) {    
    switch (tt)
    {
    case TOK_AMP:
        return TC_BG;
    case TOK_AND_IF:
    case TOK_OR_IF:
    case TOK_SEMI:
        return TC_SEP;
    case TOK_LPAREN:
    case TOK_LBRACE:
        return TC_GROUP_START;
    case TOK_RBRACE:
    case TOK_RPAREN:
        return TC_GROUP_END;
    case TOK_PIPE:
        return TC_PIPE;
    case TOK_CMD_ST_START:
        return TC_CMD_SUB_START;
    case TOK_CMD_ST_END:
        return TC_CMD_SUB_END;
    case TOK_REDIR_DUP_IN:
    case TOK_REDIR_DUP_OUT:
    case TOK_REDIR_IN:
    case TOK_REDIR_HEREDOC:
    case TOK_REDIR_OUT:
    case TOK_REDIR_HERESTR:
    case TOK_REDIR_READ_WRITE:
    case TOK_REDIR_OUT_APPEND:
        return TC_REDIR;
    case TOK_REDIR_LHS_FD:
    case TOK_REDIR_RHS_FD:
    case TOK_REDIR_RHS_CLOSE:
        return TC_RD_ST;
    case TOK_WORD:
        return TC_WORD;
    case TOK_DQ_START:
        return TC_DQ_START;
    case TOK_DQ_END:
        return TC_DQ_END;
    default:
        return TOK_ERROR;
    }
    
}

/**
 * @param start zero indexed
 * @param end   zero indexed
 */
token_arr make_arr_view(token_arr* arr, size_t start, size_t end) {
    token_arr ret = (token_arr){0};
    if (end == -1) end = arr->occupied - 1;

    if (start > end) {return (token_arr){NULL, 0, 0};}

    //cap with real values
    start = (start > arr->occupied-1)? arr->occupied-1 : start;
    end = (end > arr->occupied-1)? arr->occupied-1 : end;

    ret.ptr = arr->ptr + start;
    ret.occupied = end - start + 1;

    ret.allocated = arr->allocated - start;
    return ret;
}

/**
 * @return idx of token: -1 if not found, -2 if found at invalid pos.
 */
int find_list_sep(token_arr* arr, const char* cmdline) {
    int group_weight    = 0;
    int idx             = 0;
    bool found          = 0;
    token_cat tc;
    char* ptr           = cmdline;

    if (arr->occupied <= 1) return -1;

    // Search for an open separator.
    while(arr->ptr[idx].type != TOK_EOL) {
        tc = get_category(arr->ptr[idx].type);
        if (tc == TC_GROUP_START)group_weight++;
        if (tc == TC_GROUP_END)group_weight--;
        if (!group_weight && tc == TC_SEP) {found=1;break;}
        idx++;
    }

    if (!found) return -1;

    // grammar rule: no {&&, ||, ;} at the begining or end of line
    if (idx == 0 || idx >= arr->occupied-2) {
        error_parse(ERR_UNEXP, ptr + (arr->ptr[idx].str_idx));
        g_abort_ast = 1;
        return -2;
    }
    return idx;
}

/**
 * same deal as above
 */
int find_pipe(token_arr* arr, const char* cmdline) {
    int group_weight    = 0;
    int idx             = 0;
    bool found          = 0;
    token_cat tc;
    char* ptr           = cmdline;

    if (arr->occupied <= 1) return -1;

    // Not the time if there are list separators.
    if (find_list_sep(arr, cmdline) >= 0) return -1;

    // Search for an open pipe token
    while(arr->ptr[idx].type != TOK_EOL) {
        tc = get_category(arr->ptr[idx].type);
        if (tc == TC_GROUP_START)group_weight++;
        if (tc == TC_GROUP_END)group_weight--;
        if (tc == TC_CMD_SUB_START)group_weight++;
        if (tc == TC_CMD_SUB_END)group_weight--;
        if (!group_weight && tc == TC_PIPE) {found=1;break;}
        idx++;
    }

    if(!found) return -1;

    // grammar rule: no | at the begining or end of line
    if (idx == 0 || idx >= arr->occupied-1) {
        error_parse(ERR_UNEXP, ptr + (arr->ptr[idx].str_idx));
        g_abort_ast = 1;
        return -2;
    }
}

bool type_in_list(typeof_token t, typeof_token* l, size_t sz) {
    for (size_t i = 0; i < sz; i++)
    {
        if (l[i] == t) return true;
    }
    return false;
    
}