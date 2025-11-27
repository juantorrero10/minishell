#include <mshparser.h>
#define __DEBUG
#include <../log.h>

// returns buff for convenience
char* str_tok(typeof_token tt, char buff[])
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
        case TOK_REDIR_RHS_CLOSE:          strcpy(buff, "FD_CLOSE"); break;

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
        if (stack[top-1] == '(') {
        error_parse(ERR_UNTERM_PAREN, NULL);
        } else {
            error_parse(ERR_UNTERM_BRACKET, NULL);
        }
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
        return TC_ERROR;
    }
    
}

/**
 * @brief make a view of an array.
 * @example for (arr, 0, 2).
 *      arr = {a, b, c, d, EOL} .allocated = 5 .occupied = 5
 *             ^     ^
 *             S     E
 *   result = {a, b, c}         .allocated = 5 .occupied = 3 
 * @param start zero indexed
 * @param end   zero indexed
 */
token_arr make_arr_view(token_arr* arr, size_t start, size_t end) {
    token_arr ret = (token_arr){0};
    if (end == __INT32_MAX__) end = arr->occupied - 1;

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
int find_list_sep(token_arr* arr, const char* cmdline, bool __only_semicolon) {
    int group_weight    = 0;
    int idx             = 0;
    bool found          = 0;
    token_cat tc;
    char* ptr           = (char*)cmdline;

    if (arr->occupied <= 1) return -1;

    // Search for an open separator.
    while(arr->ptr[idx].type != TOK_EOL && idx < (int)arr->occupied) {
        tc = get_category(arr->ptr[idx].type);
        if (tc == TC_GROUP_START)group_weight++;
        if (tc == TC_GROUP_END)group_weight--;
        if (!group_weight && tc == TC_SEP 
            && (!__only_semicolon || arr->ptr[idx].type == TOK_SEMI)) 
            {found=1;break;}
        idx++;
    }

    if (!found) return -1;

    // grammar rule: no {&&, ||} at the begining or end of element
    if (arr->ptr[idx].type != TOK_SEMI) {
        if (idx == 0 || idx >= (int)arr->occupied-2) {
            error_parse(ERR_UNEXP, ptr + (arr->ptr[idx].str_idx-1));
            g_abort_ast = 1;
            return -2;
        }
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
    char* ptr           = (char*)cmdline;

    if (arr->occupied <= 1) return -1;

    // Not the time if there are list separators.
    if (find_list_sep(arr, cmdline, 0) >= 0) return -1;

    // Search for an open pipe token
    while(arr->ptr[idx].type != TOK_EOL && idx < (int)arr->occupied) {
        tc = get_category(arr->ptr[idx].type);
        if (tc == TC_GROUP_START)group_weight++;
        if (tc == TC_GROUP_END)group_weight--;
        if (tc == TC_CMD_SUB_START)group_weight++;
        if (tc == TC_CMD_SUB_END)group_weight--;
        if (!group_weight && tc == TC_PIPE) {found=1;break;}
        idx++;
    }

    if(!found) return -1;

    // grammar rule: no | at the begining or end of element
    if (idx == 0 || idx >= (int)arr->occupied-1) {
        error_parse(ERR_UNEXP, ptr + (arr->ptr[idx].str_idx-1));
        g_abort_ast = 1;
        return -2;
    }
    return idx;
}

/**
 * @brief find redirections in token array.
 * @param n_redirs number of redirections found in token array.
 * @return idx of first redirection in the token array provided.
 *      -1 not found,  -2 found at invalid pos.
 */
int find_redirs(token_arr* arr, const char* cmdline, _out_ int* n_redirs) {
    int group_weight    = 0;
    int idx             = 0;
    *n_redirs           = 0;
    int f; (void)f; 
    int first           = -1;
    bool found          = 0;
    token_cat tc;
    char* ptr           = (char*)cmdline;

    while(arr->ptr[idx].type != TOK_EOL && idx < (int)arr->occupied) {
        tc = get_category(arr->ptr[idx].type);
        if (tc == TC_GROUP_START)group_weight++;
        if (tc == TC_GROUP_END)group_weight--;
        if (tc == TC_CMD_SUB_START)group_weight++;
        if (tc == TC_CMD_SUB_END)group_weight--;
        if ((tc == TC_REDIR || tc == TC_RD_ST) && !group_weight) {
            if (first == -1) first = idx;
                    //Make the compiler happy
            if (tc == TC_RD_ST) idx = idx+3;
            else if (tc == TC_REDIR) idx = idx+2;
            f = (*n_redirs)++;(void)f;
            found = 1;
        } else idx++;
    }

    if (!found) return -1;

    // Only grammar enforcing that we can do here is
    // Error when fisrt redir is at last pos of the array.
    // grammar rule: redirections need at least another token at the left.
    if (arr->ptr[first+1].type == TOK_EOL || first >= (int)arr->occupied - 1) {
        error_parse(ERR_UNEXP, ptr + arr->ptr[first].str_idx-1);
        error_parse(ERR_EXP, "expected fd, filename, string or heredoc delimiter.");
        g_abort_ast = 1;
        return -2;
    }
    return first;
}

/**
 * @brief find redirections in token array.
 */
int find_cmd_sub(token_arr* arr) {
    int group_weight    = 0;
    int idx             = 0;
    bool found          = 0;
    token_cat tc;

    while(arr->ptr[idx].type != TOK_EOL && idx < (int)arr->occupied) {
        tc = get_category(arr->ptr[idx].type);
        if (tc == TC_GROUP_START)group_weight++;
        if (tc == TC_GROUP_END)group_weight--;
        if (tc == TC_CMD_SUB_START) {found=1; break;}
        idx++;
    }
    if (!found) {return -1;}

    return idx;
}

/**
 * @brief locate a binary in the disk through $PATH env var.
 * resulting string need to be freed.
 * returns NULL if non-existant.
 */
char* find_binary_path(const char* name) {
    const char* path_env    = NULL;
    char* path              = NULL;
    char* saveptr           = NULL;
    char* dir               = NULL;
    char* full              = NULL;
    size_t needed           = 0;
    size_t len_dir          = 0;
    size_t len_name         = 0;
    

    if (!name || !*name)
        return NULL;

    // If the name already contains a '/', treat it literally.
    if (strchr(name, '/')) {
        if (access(name, X_OK) == 0)
            return strdup(name);
        return NULL;
    }

    path_env = getenv("PATH");
    if (!path_env)
        return NULL;

    // Duplicate PATH because strtok modifies it
    path = strdup(path_env);
    if (!path)
        return NULL;

    dir = strtok_r(path, ":", &saveptr);

    while (dir) {
        len_dir = strlen(dir);
        len_name = strlen(name);

        // Allocate buffer for: dir + '/' + name + '\0'
        needed = len_dir + 1 + len_name + 1;
        full = malloc(needed);
        if (!full) {
            free(path);
            return NULL;
        }

        // dir/name
        strcpy(full, dir);
        full[len_dir] = '/';
        strcpy(full + len_dir + 1, name);

        // if exec.
        if (access(full, X_OK) == 0) {
            free(path);
            return full;
        }

        free(full);
        dir = strtok_r(NULL, ":", &saveptr);
    }

    free(path);
    return NULL;
}

bool type_in_list(typeof_token t, typeof_token* l, size_t sz) {
    for (size_t i = 0; i < sz; i++)
    {
        if (l[i] == t) return true;
    }
    return false;
    
}

/**
 * Search for a group.
 * true if arr given -> ( ... ) [<redirs>]
 *                   or { ... } [<redirs>]
 * false if command. (word)
 * aborts AST if anything else
 */
bool is_a_group(token_arr* arr, const char* cmdline) {
    token_cat tc = 0;
    char* ptr = (char*)cmdline; //Make the compiler happy

    tc = get_category(arr->ptr[0].type);

    if (tc == TC_GROUP_START) {
        return true;
    }
    else if (tc == TC_WORD) {
        return false;
    }

    //Grammar rule:
    //  list elements or whole lines start with word or group opening.
    else  {
        error_parse(ERR_UNEXP, ptr + arr->ptr[0].str_idx);
        error_parse(ERR_EXP, "Expected a command or a group/subshell.");
        g_abort_ast = 1;
        return false;
    }
    
}

int redir_default_fd(typeof_token rd) {
    if (get_category(rd) != TC_REDIR) return -1;
    switch (rd)
    {
    case TOK_REDIR_OUT:
    case TOK_REDIR_OUT_APPEND:
    case TOK_REDIR_DUP_OUT:
        return 1;
    default:
        return 0;
    }
}

/**
 * @brief Extract a word from a series of tokens.
 */
void get_word(token_arr* arr, int idx, char** output) {
    token_arr view = make_arr_view(arr, idx, __INT32_MAX__);
    token_arr* pview = &view;
    int i = 0;

    if (tok_type(pview, 0) == TOK_WORD) {
        *output = strdup(view.ptr[0].value);
    } // IF word is "", check for cmd subs
    else if (tok_type(pview, 0) == TOK_DQ_START) {
        while(tok_type(pview, i+1) != TOK_DQ_END) {
            i++;
            // Not allowed for now
            if (tok_type(pview, i) == TOK_CMD_ST_START) {
                error_parse(-1, "cmd substitutions are now allowed for now.");
                return;
            }
        }
        *output = strdup(view.ptr[i].value);
    } else if (tok_type(pview, 0) == TOK_CMD_ST_START) {
        error_parse(-1, "cmd substitutions are now allowed for now.");
        return;
    }

}
