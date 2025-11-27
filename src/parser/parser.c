#include <mshparser.h>
#include <../log.h>

int g_abort_ast = 0;


// forward declaration.
static ast_t* parse_line(token_arr* arr, const char* cmdline, _opt_ void* locate_at);

/**
 * @brief parse redirections.
 * It only expects a view of only redirections.
 * no words, no separators...
 * @param __nredirs   [out] if NULL or *0 figure out nredirs at return it
 *                  \n[in]  *nredirs != 0 caller may have provided nredirs previouly so no need to do it again.
 * @return redir array.
 */
static ast_node_redir_t* parse_redirections(token_arr* arr, const char* cmdline, _in_out_ int* __nredirs) {
    ast_node_redir_t* redir_arr = NULL;
    ast_node_redir_t* curr_rd = NULL;
    int curr     = 0;
    int nredirs  = 0;
    int idx      = 0;
    int rd_idx   = 0;
    int end_last = 0;
    bool last    = 0;
    typeof_token bef, aft; (void)bef; (void)aft;
    char* ptr = (char*)cmdline; //Make the compiler happy

    if (!__nredirs || !*__nredirs) {
        find_redirs(arr, cmdline, &nredirs);
    } else {nredirs = *__nredirs;}

    redir_arr = malloc(sizeof(ast_node_redir_t) * nredirs);

    /**
     * Grammar for redirs.
     * format: <fd=n>, expects a file descriptor or defaults to n.
     *         <fd>  , just expects it
     *         <str+>, expects a string (+ -> string is expanded.)
     *         <cls>,  closing of fd: '-' char
     *         <filename>,  self explanatory.
     * 
     * -------------------------------------------------------------
     * Redir append:    <fd=1> '>>'  <filename>
     * Redit out:       <fd=1> '>'   <filename>
     * Redir in:        <fd=0> '<'   <filename>
     * Redir inout      <fd=0> '<>'  <filename>
     * Redir dup out:   <fd=1> '>&'  <fd> or <cls>
     * Redir dup in:    <fd=0> '<&'  <fd> or <cls>
     * Redir heredoc:   <fd=0> '<<'  <str>
     * Redit herestr:   <fd=0> '<<<' <str+>
    */

    while(!last_token(arr, idx)) {
        curr_rd = redir_arr + curr;
        //Skip until next redir token
        while(get_category(tok_type(arr, idx)) != TC_REDIR && !last_token(arr, idx)) idx++;
        if (last_token(arr, idx)) last = 1;
        bef = tok_type(arr, idx-1);
        aft = tok_type(arr, idx+1);
        switch (tok_type(arr, idx))
        {
        case TOK_REDIR_IN:
        case TOK_REDIR_READ_WRITE:
        case TOK_REDIR_OUT_APPEND:
        case TOK_REDIR_OUT:
            //Grammar check
            if (last) error_parse(ERR_EXP, "Expected a filename.");
            if ((idx > end_last && tok_type(arr, idx-1) != TOK_REDIR_LHS_FD) || (
                tok_type(arr, idx+1)  != TOK_WORD && 
                tok_type(arr, idx+1) != TOK_DQ_START && 
                tok_type(arr, idx+1) != TOK_CMD_ST_START)) {
                    error_parse(ERR_INVALID_REDIR_SYNTAX, ptr + strloc(arr, idx-1));
                    goto __redir_abort;
                }
            
            rd_idx = idx;
            curr_rd->kind = REDIR_FILE;
            get_word(arr, idx+1, &curr_rd->target.filename);
            //Skip until end of quotes
            if (tok_type(arr, idx+1) == TOK_DQ_START) {
                while(tok_type(arr, idx+1) != TOK_DQ_END) {
                    idx++;
                } 
            }
            if (tok_type(arr, idx+1) == TOK_CMD_ST_START) {
                while(tok_type(arr, idx+1) != TOK_CMD_ST_END) {
                    idx++;
                } 
            }
            break;

        case TOK_REDIR_DUP_IN:
        case TOK_REDIR_DUP_OUT:
            //Grammar check
            if (last) error_parse(ERR_EXP, "Expected a file descriptor.");
            if ((idx > end_last && (tok_type(arr, idx-1) != TOK_REDIR_LHS_FD)) ||
                (tok_type(arr, idx+1) != TOK_REDIR_RHS_FD  &&
                 tok_type(arr, idx+1) != TOK_REDIR_RHS_CLOSE)) {
                    error_parse(ERR_INVALID_REDIR_SYNTAX, ptr + strloc(arr, idx-1));
                    goto __redir_abort;
                }
            if (tok_type(arr, idx+1) == TOK_REDIR_RHS_CLOSE) {
                curr_rd->kind = REDIR_CLOSE; break;
            }
            rd_idx = idx;
            curr_rd->kind = REDIR_FD;
            curr_rd->target.fd = arr->ptr[idx+1].number;
            break;
        
        case TOK_REDIR_HEREDOC:
        case TOK_REDIR_HERESTR:
            //Grammar check
            if (last) error_parse(ERR_EXP, "Expected a string.");
            if ((idx > end_last &&
                tok_type(arr, idx-1) != TOK_REDIR_LHS_FD) || (
                tok_type(arr, idx+1)  != TOK_WORD && 
                tok_type(arr, idx+1) != TOK_DQ_START && 
                tok_type(arr, idx+1) != TOK_CMD_ST_START)) {
                    error_parse(ERR_INVALID_REDIR_SYNTAX, ptr + strloc(arr, idx-1));
                    goto __redir_abort;
                }
            rd_idx = idx;
            if (tok_type(arr, idx) == TOK_REDIR_HEREDOC)
                curr_rd->kind = REDIR_HEREDOC;
            else curr_rd->kind = REDIR_HERESTR;
            get_word(arr, idx+1, &curr_rd->target.filename);
            //Skip until end of quotes
            if (tok_type(arr, idx+1) == TOK_DQ_START) {
                while(tok_type(arr, idx+1) != TOK_DQ_END) {
                    idx++;
                } 
            }
            if (tok_type(arr, idx+1) == TOK_CMD_ST_START) {
                while(tok_type(arr, idx+1) != TOK_CMD_ST_END) {
                    idx++;
                } 
            } break;

        default:
            break;
        }

        // finish current node and go to next one
        if (rd_idx <= end_last)
            curr_rd->left_fd = redir_default_fd(tok_type(arr, rd_idx)); //default val
        else curr_rd->left_fd = arr->ptr[rd_idx-1].number;                
        curr_rd->op = tok_type(arr, rd_idx);
        idx = idx + 2;
        end_last = idx;

        curr++;
    }

    return redir_arr;

__redir_abort:
    g_abort_ast = 1;
    free(redir_arr);
    return NULL;
}

/**
 * @brief parse one command. 
 * It expects just a 
 * <arg0> <arg1> ... <argn> [<redirections>]
 * everything higher level should have been taken care of by now.
 * 
 * command substitutions are handled here.
 * @todo: command substitutions.
 */
static ast_t* parse_simple_command(token_arr* arr, const char* cmdline, _opt_ void* locate_at) {
    ast_node_command_t cmd = (ast_node_command_t){0};
    ast_t* ret      = NULL;
    int idx         = 0;
    int idx_cmdsub  = 0;
    int idx_redir   = 0;
    int nredirs     = 0;
    int argc        = 0;
    int first       = 0;
    //Helper numbers
    int j            = 0;
    size_t p         = 0;
    size_t l         = 0;
    size_t total_len = 0;

    char* buf;
    char* ptr       = (char*)cmdline;
    token_arr view;

    if (!locate_at) ret = ast_create_empty();
    else ret = (ast_t*)locate_at;

    cmd.argc = 0;
    cmd.nredirs = 0;   //temp
    cmd.redirs = NULL; //temp
    cmd.argv = NULL;

    idx_redir = find_redirs(arr, cmdline, &nredirs);
    if (g_abort_ast) {ast_free(ret); return NULL;}
    if (nredirs) {
        view = make_arr_view(arr, idx_redir, __INT32_MAX__);
        cmd.redirs = parse_redirections(&view, cmdline, &nredirs);
        if (g_abort_ast) {ast_free(ret); return NULL; }
        cmd.nredirs = nredirs;
    }

    ret->type = AST_COMMAND;
    //When redirections are supported, properly truncate array.
    view = make_arr_view(arr, 0, idx_redir);
    idx_cmdsub = find_cmd_sub(&view);
    if (idx_cmdsub != -1) {
        // abort because they are not supported yet.
        ast_free(ret);
        error_parse(ERR_UNEXP, ptr + strloc(arr, idx_cmdsub));
        error_clarify("Cmd subtitutions are not supported yet.");
        g_abort_ast = 1;
        free(ret);
        return NULL;
    }

    // STEP 1: Figure out how many args the command will have.
    // Count each word. If quoted start count just one until quotes end.
    while (tok_type(arr, idx) != TOK_EOL && idx < (int)arr->occupied) {
        if (tok_type(arr, idx) == TOK_DQ_START) {
            //skip until DQ_END
            while(tok_type(arr, idx) != TOK_DQ_END) idx++;
            argc++;
        }
        else if (tok_type(arr, idx) == TOK_WORD) argc++;
        // Stop when encounter a redir
        else if (get_category(tok_type(arr, idx)) == TC_REDIR || tok_type(arr, idx) == TOK_REDIR_LHS_FD) {break;}
        // Error if other thing (Im not sure if this is ever going to be true).
        else {error_parse(ERR_UNEXP, ptr + strloc(arr, idx)); if(!locate_at)free(ret); g_abort_ast = 1; return NULL;}
        idx++;
    }

    // STEP 2: make the fucking argvs
     cmd.argc = argc;
    cmd.argv = malloc(sizeof(char*) * (argc + 1)); // +1 null terminated string.
    if (!cmd.argv) { /* handle oom if you want */ }
    cmd.argv[argc] = NULL;
    idx = 0;
    argc = 0;
    while (tok_type(arr, idx) != TOK_EOL && idx < (int)arr->occupied)
    {
        if (tok_type(arr, idx) == TOK_WORD) {
            cmd.argv[argc] = strdup(arr->ptr[idx].value);
            idx++; argc++;
            continue;
        }

        if (tok_type(arr, idx) == TOK_DQ_START) {
            /* build one argument from all tokens until DQ_END (to match counting pass) */
            total_len = 0;
            j = idx + 1;
            /* compute required length */
            while (tok_type(arr, j) != TOK_DQ_END && j < (int)arr->occupied) {
                if (tok_type(arr, j) == TOK_WORD && arr->ptr[j].value)
                    total_len += strlen(arr->ptr[j].value) + 1; /* +1 for possible space */
                j++;
            }
            if (total_len == 0) {
                /* empty quoted string -> empty arg */
                cmd.argv[argc] = strdup("");
            } else {
                buf = malloc(total_len + 1);
                p = 0;
                first = 1;
                j = idx + 1;
                while (tok_type(arr, j) != TOK_DQ_END && j < (int)arr->occupied) {
                    if (tok_type(arr, j) == TOK_WORD && arr->ptr[j].value) {
                        if (!first) buf[p++] = ' ';
                        l = strlen(arr->ptr[j].value);
                        memcpy(buf + p, arr->ptr[j].value, l);
                        p += l;
                        first = 0;
                    }
                    j++;
                }
                buf[p] = '\0';
                cmd.argv[argc] = buf;
            }
            /* advance idx to token after DQ_END */
            if (tok_type(arr, j) == TOK_DQ_END) idx = j + 1;
            else idx = j; /* malformed but counting pass already validated */
            argc++;
            continue;
        }

        /* other tokens (redir, group, etc.) terminate argv list here */
        break;
    }

    //STEP 3: self explanatory
    cmd.filename = find_binary_path(cmd.argv[0]);
    ret->node.cmd = cmd;
    return ret;
}

/**
 * @brief parse a group/subshell
 * It expects:
 * ( ... )   [<redirections>] subshells
 * or
 * { ... ; } [<redirections>] groups
 */
static ast_t* parse_group(token_arr* arr, const char* cmdline) {
    ast_node_group_t grp = (ast_node_group_t){0};
    ast_t* ret      = ast_create_empty();
    int idx         = 0;
    int n_red       = 0; (void) n_red;
    token_arr view;
    char* ptr = (char*) cmdline;

    ret->type = AST_GROUP;
    grp.nredirs = 0;
    grp.redirs = NULL;

    //Get type of group.
    if (arr->ptr[0].type == TOK_LPAREN) grp.group_type = GROUP_SUBSHELL;
    if (arr->ptr[0].type == TOK_LBRACE) grp.group_type = GROUP_GENERIC;

    //Get group end
    //By this point is guaranteed to have a closing token.
    idx = arr->occupied - 1;
    while (get_category(tok_type(arr, idx)) != TC_GROUP_END && idx >= 0) idx--;
    idx++;
    

    // Grammar rule: {} groups need to have a ; before the closing }.
    // Not sure why but bash enforces this rule.
    if (grp.group_type == GROUP_GENERIC) {
        if (tok_type(arr, idx-2) != TOK_SEMI) {
            error_parse(ERR_EXP, "Expected ';' -> { ...  ; }");
            fprintf(stderr,      "                              HERE  ^ \n");
            g_abort_ast = 1;
            free(ret);
            return NULL;
        }
    }

    //Get redirections.
    if (!last_token(arr, idx)) {
        if (find_redirs(arr, cmdline, &n_red) == -1) {
            // Not sure if this will ever trigger but it doesnt hurt to check.
            error_parse(ERR_UNEXP, ptr + strloc(arr, idx+1));
            g_abort_ast = 1; free(ret);
            return NULL;
        }
        view = make_arr_view(arr, idx, __INT32_MAX__);
        grp.redirs = parse_redirections(&view, cmdline, &n_red);
        grp.nredirs = n_red;
    }
    //Parse children
    view = make_arr_view(arr, 1, idx-2);
    grp.children = parse_line(&view, cmdline, NULL);
    ret->node.grp = grp;
    return ret;
}

static ast_t* parse_list(token_arr* arr, int idx, const char* cmdline) {
    ast_t* ret = ast_create_empty();    // Defaults to AST_INVALID
    ast_node_list_t sep = (ast_node_list_t){0};
    token_arr view_left;
    token_arr view_right;
    bool semi = false;
    bool semi_end = false;
    int temp    = 0;

    // ; take priority.
    // bool __only_semicolon <- 1
    if ((temp = find_list_sep(arr, cmdline, 1)) != -1) {
        idx = temp;
        semi = true;
    }

    //Especial case ; at the end of a element -> no separation.
    if (semi) {
        if (arr->ptr[arr->occupied-1].type == TOK_EOL && 
            arr->ptr[arr->occupied-2].type == TOK_SEMI) {
            temp = 3; semi_end = 1;
        } else if (arr->ptr[arr->occupied-1].type == TOK_SEMI) {
            temp = 2; semi_end = 1;
        }
        if (semi_end) {
            view_left = make_arr_view(arr, 0, arr->occupied-temp);
            free(ret);
            return parse_line(&view_left, cmdline, NULL);
        }
    }

    ret->type = AST_LIST;
    sep.sep_type = tok_type(arr, idx);
    view_left = make_arr_view(arr, 0, idx - 1);
    view_right = make_arr_view(arr, idx+1, arr->occupied - 1);
    sep.left = parse_line(&view_left, cmdline, NULL);
    sep.right = parse_line(&view_right, cmdline, NULL);
    ret->node.sep = sep;
    return ret;
}

static ast_t* parse_pipeline(token_arr* arr, const char* cmdline) {
    ast_t* ret = ast_create_empty();    // Defaults to 
    ast_t* temp;
    ast_node_pipeline_t ppl = (ast_node_pipeline_t){0};
    token_arr view;
    token_cat tc;
    char *ptr           = (char*) cmdline;
    int ppl_end         = 0;
    int idx             = 0;
    int cmd_st          = 0;
    int cmd_end         = 0;
    int n_pipes         = 0;
    // tokens allowed in a pipeline (using this to delimit the pipeline) + redirs
    // size: 6
    typeof_token allowed[] = {TOK_WORD, 
        TOK_PIPE, TOK_DQ_START, TOK_DQ_END, TOK_CMD_ST_START, TOK_CMD_ST_END};
    int cmd_sub = 0;

    ret->type = AST_PIPELINE;
    
    // grammar rule: if not in a cmd sub, only tokens in [allowed] are valid.
    while (!last_token(arr, idx-1)) 
    {
        tc = get_category(tok_type(arr, idx));
        ppl_end = idx;
        if (tok_type(arr, idx) == TOK_CMD_ST_START) cmd_sub++;
        if (tok_type(arr, idx) == TOK_CMD_ST_END) cmd_sub--;
        if (tok_type(arr, idx) == TOK_PIPE)n_pipes++;
        if (
            (!type_in_list(tok_type(arr, idx), allowed, 6)) && cmd_sub <= 0
            && (tc != TC_REDIR && tc != TC_RD_ST))
            {ppl_end = idx; break;}
        idx++;
    }

    // grammar rule: if last tok is pipe -> unexpected token
    // ex:        find / -type f | &grep -v "..."
    //                           ^ end
    if (!type_in_list(tok_type(arr, ppl_end), allowed, 6)) {
        error_parse(ERR_UNEXP, ptr + strloc(arr, ppl_end));
        error_clarify("only simple commands are allowed inside a pipeline for now.");
        free(ret); g_abort_ast = 1; return NULL;
    }

    idx = 0;
    ppl.elements = ast_create_array(n_pipes + 1);
    while (idx <= ppl_end) {
        if (tok_type(arr, idx) == TOK_CMD_ST_START) cmd_sub++;
        if (tok_type(arr, idx) == TOK_CMD_ST_END) cmd_sub--;

        if ((tok_type(arr, idx) == TOK_PIPE && !cmd_sub) || idx == ppl_end) {
            // Supress | token if not the last one.
            cmd_end = idx - 1;
            if (idx == ppl_end) cmd_end++;
            view = make_arr_view(arr, cmd_st, cmd_end);
            // Only allowing simple commands in pipelines.
            temp = parse_line(&view, cmdline, (ppl.elements + ppl.ncommands++));
            if (g_abort_ast || !temp) {
                if (ppl.elements)free(ppl.elements);
                goto abort_ppl;

            }
            cmd_st = idx + 1;
            cmd_end = cmd_st;
        }
        idx++;
    }

    ret->node.ppl = ppl;
    return ret;

abort_ppl:
    if (ret) { ast_free(ret); free(ret); }
    g_abort_ast = 1;
    return NULL;
    
}


/**
 * @brief parse whole token array given.
 * this is a general function to call from each recursion.
 * its cascades down from more complex to more simple.
 * @note operates under assumption of a well tokenized and balanced input.
 */
ast_t* parse_line(token_arr* arr, const char* cmdline, _opt_ void* locate_at) {
    // HIGHER LEVEL
    // 0. background flag
    // 1. parse_list
    // 3. parse_pipeline
    // 4  parse_group    << redirections
    // 5. parse_command  << redirections
    // 6. words         expansions... after ASTs
    // LOWER LEVEL
    ast_t* ret  = NULL; (void) ret;
    ast_node_background_t bg = (ast_node_background_t){0}; (void)bg;
    int found   = 0;
    token_arr view;
    

    // Reset abort signal
    g_abort_ast = 0;

    if (arr->occupied == 0) return NULL;

    // Background flag
    if (arr->ptr[arr->occupied-2].type == TOK_AMP) {
        ret = ast_create_empty();
        ret->type = AST_BG;
        view = make_arr_view(arr, 0, arr->occupied-3);
        bg.children = parse_line(&view, cmdline, locate_at);
        ret->node.bg = bg; return ret;
    }

    /*-------------------- LISTS -------------------*/
    found = find_list_sep(arr, cmdline, 0);
    if (g_abort_ast) return NULL;
    if (found >= 0) {
        return parse_list(arr, found, cmdline);
    }

    /*-------------------- PIPELINES -------------------*/
    found = find_pipe(arr, cmdline);
    if (g_abort_ast) return NULL;
    if (found >= 0) {
        return parse_pipeline(arr, cmdline);
    }

    /*----------------- GROUPS/SUBSHELLS -------------------*/
    if (is_a_group(arr, cmdline)) {
        return parse_group(arr, cmdline);
    } else if (g_abort_ast) {
        return NULL;
    }

    /*-------------------- COMMANDS -------------------*/
    return parse_simple_command(arr, cmdline, locate_at);

}

// Main procedure
ast_t* parse_string(char* cmdline) {
    int sz = 0;
    token_arr arr = {NULL, 0, 0}; (void)arr;
    char* trim_chars = "\t\r\n ";
    ast_t* result = NULL;

    sz = strlen(cmdline);
    INFO("received: %s", cmdline);
    //trim characters
    for (int i = ((int)sz)-1; i >= 0; i--)
    {
        if (strchr(trim_chars, cmdline[i])) {
            cmdline[i] = '\0';
            sz--;
        } else break;
    }
    

    if (pu_check_balance(cmdline, strlen(cmdline)))return NULL;
    arr = __tokenize(cmdline, &sz);
    if (sz) goto clean_exit;
    pu_peek(&arr);

    result = parse_line(&arr, cmdline, NULL);
    if (!result) {
        WARN("parser aborted");
    }

clean_exit:
    free_token_arr(&arr);
    return result;
}