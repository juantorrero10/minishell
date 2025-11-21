#include <mshparser.h>

int g_abort_ast = 0;

// forward declaration.
static ast_t* parse_line(token_arr* arr, const char* cmdline);

/**
 * @brief parse one command. 
 * It expects just a 
 * <arg0> <arg1> ... <argn> [<redirections>]
 * everything higher level should have been taken care of by now.
 * 
 * command substitutions are handled here.
 * @todo: command substitutions.
 * @todo: redirections.
 */
static ast_t* parse_simple_command(token_arr* arr, const char* cmdline) {
    ast_node_command_t cmd = (ast_node_command_t){0};
    ast_t* ret      = ast_create_empty();
    int idx         = 0;
    int idx_redir   = 0; (void)idx_redir;
    int idx_cmdsub  = 0;
    int nredirs     = 0;
    int argc        = 0;
    char* ptr       = (char*) cmdline;
    token_arr view;

    idx_redir = find_redirs(arr, cmdline, &nredirs);
    if (g_abort_ast) {ast_free(ret); return NULL;}
    if (nredirs) {
        // abort because they are not supported yet.
        ast_free(ret);
        error_parse(-1, "Redirections are not supported yet.");
        return NULL;
    }
    cmd.argc = 0;
    cmd.nredirs = 0;   //temp
    cmd.redirs = NULL; //temp
    cmd.argv = NULL;

    ret->type = AST_COMMAND;
    //When redirections are supported, properly truncate array.
    view = make_arr_view(arr, 0, __INT32_MAX__);
    idx_cmdsub = find_cmd_sub(&view);
    if (idx_cmdsub != -1) {
        // abort because they are not supported yet.
        ast_free(ret);
        error_parse(-1, "Cmd subtitutions are not supported yet.");
        return NULL;
    }

    // STEP 1: Figure out how many args the command will have.
    // Count each word. If quoted start count just one until quotes end.
    while (arr->ptr[idx].type != TOK_EOL && idx < (int)arr->occupied) {
        if (arr->ptr[idx].type == TOK_DQ_START) {
            //skip until DQ_END
            while(arr->ptr[idx].type != TOK_DQ_END) idx++;
            argc++;
        }
        else if (arr->ptr[idx].type == TOK_WORD) argc++;
        else {g_abort_ast = 1;error_parse(ERR_UNEXP, ptr + arr->ptr[idx].str_idx-1); ast_free(ret); return NULL;}
        idx++;
    }

    // STEP 2: make the fucking argvs
    cmd.argc = argc;
    cmd.argv = malloc(sizeof(char*) * (argc + 1)); // +1 null terminated string.
    cmd.argv[argc] = NULL;
    idx = 0;
    argc = 0;
    while (arr->ptr[idx].type != TOK_EOL && idx < (int)arr->occupied)
    {
        // TODO: if cmd_sub call ...
        // view = make_arr_view( /* strip $( and ) */ );
        // cmd.argv[argc] <- char* execute_cmd_sub(parse_line(view, cmdline), cmdline);
        // idx++, argc ++

        if (arr->ptr[idx].type == TOK_WORD) cmd.argv[argc] = strdup(arr->ptr[idx].value);
        if (arr->ptr[idx].type == TOK_DQ_START) { idx++; continue; }
        if (arr->ptr[idx].type == TOK_DQ_END) { idx++; continue; }
        idx ++; argc ++;
    }

    //STEP 3: self explanatory
    cmd.filename = find_binary_path(cmd.argv[0]);
    ret->node.cmd = cmd;
    return ret;
}

//static ast_t* parse_group(token_arr* arr, const char* cmdline);

//static ast_node_redir_t* parse_redirections(token_arr* arr, const char* cmdline);

static ast_t* parse_list(token_arr* arr, int idx, const char* cmdline) {
    ast_t* ret = ast_create_empty();    // Defaults to AST_INVALID
    ast_node_list_t sep = (ast_node_list_t){0};
    token_arr view_left;
    token_arr view_right;
    int temp    = 0;

    // ; take priority.
    // bool __only_semicolon <- 1
    if ((temp = find_list_sep(arr, cmdline, 1)) != -1) {
        idx = temp;
    }

    ret->type = AST_LIST;
    sep.sep_type = arr->ptr[idx].type;
    view_left = make_arr_view(arr, 0, idx - 1);
    view_right = make_arr_view(arr, idx+1, arr->occupied - 1);
    sep.left = parse_line(&view_left, cmdline);
    sep.right = parse_line(&view_right, cmdline);
    ret->node.sep = sep;
    return ret;
}

static ast_t* parse_pipeline(token_arr* arr, const char* cmdline) {
    ast_t* ret = ast_create_empty();    // Defaults to 
    ast_t* temp;
    ast_node_pipeline_t ppl = (ast_node_pipeline_t){0};
    token_arr view;
    char *ptr           = (char*) cmdline;
    int ppl_end         = 0;
    int idx             = 0;
    int cmd_st          = 0;
    int cmd_end         = 0;
    int n_pipes         = 0;
    // tokens allowed in a pipeline (using this to delimit the pipeline)
    // size: 6
    typeof_token allowed[] = {TOK_WORD, 
        TOK_PIPE, TOK_DQ_START, TOK_DQ_END, TOK_CMD_ST_START, TOK_CMD_ST_END};
    int cmd_sub = 0;

    ret->type = AST_PIPELINE;
    
    // grammar rule: if not in a cmd sub, only tokens in [allowed] are valid.
    while (arr->ptr[idx].type != TOK_EOL && idx < (int)arr->occupied) 
    {
        ppl_end = idx;
        if (arr->ptr[idx].type == TOK_CMD_ST_START) cmd_sub++;
        if (arr->ptr[idx].type == TOK_CMD_ST_END) cmd_sub--;
        if (arr->ptr[idx].type == TOK_PIPE)n_pipes++;
        if ((!type_in_list(arr->ptr[idx].type, allowed, 6)) && cmd_sub <= 0)
            {ppl_end = idx; break;}
        idx++;
    }

    // grammar rule: if last tok is pipe -> unexpected token
    // ex:        find / -type f | &grep -v "..."
    //                           ^ end
    if (arr->ptr[ppl_end].type == TOK_PIPE) {
        error_parse(ERR_UNEXP, ptr + arr->ptr[ppl_end + 1].str_idx-1);
        free(ret); g_abort_ast=1; return NULL;
    }

    idx = 0;
    ppl.elements = malloc(sizeof(ast_t) * (n_pipes + 1));
    while (idx <= ppl_end) {
        if (arr->ptr[idx].type == TOK_CMD_ST_START) cmd_sub++;
        if (arr->ptr[idx].type == TOK_CMD_ST_END) cmd_sub--;

        if ((arr->ptr[idx].type == TOK_PIPE && !cmd_sub) || idx == ppl_end) {
            // Supress | token if not the last one.
            cmd_end = idx - 1;
            if (idx == ppl_end) cmd_end++;
            view = make_arr_view(arr, cmd_st, cmd_end);
            // Only allowing simple commands in pipelines.
            temp = parse_line(&view, cmdline);
            ppl.elements[ppl.ncommands++] = *temp;
            cmd_st = idx + 1;
            cmd_end = cmd_st;
        }
        idx++;
    }

    ret->node.ppl = ppl;
    return ret;
    
}


/**
 * @brief parse whole token array given.
 * this is a general function to call from each recursion.
 * its cascades down from more complex to more simple.
 * @note operates under assumption of a well tokenized and balanced input.
 */
ast_t* parse_line(token_arr* arr, const char* cmdline) {
    // HIGHER LEVEL
    // 0. background flag
    // 1. parse_list
    // 3. parse_pipeline
    // 4. parse_command (simple/compound) -> group
    // 5. words         inside parse_command.
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
        bg.children = parse_line(&view, cmdline);
        ret->node.bg = bg; return ret;
    }

    found = find_list_sep(arr, cmdline, 0);
    if (g_abort_ast) return NULL;
    if (found >= 0) {
        return parse_list(arr, found, cmdline);
    }

    found = find_pipe(arr, cmdline);
    if (g_abort_ast) return NULL;
    if (found >= 0) {
        return parse_pipeline(arr, cmdline);
    }

    //TODO: groups.

    return parse_simple_command(arr, cmdline);

}

// Main procedure
ast_t* parse_command(char* cmdline) {
    int sz = 0;
    token_arr arr = {NULL, 0, 0}; (void)arr;
    char* trim_chars = "\t\r\n ";
    ast_t* result;

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

    result = parse_line(&arr, cmdline);
    if (result) ast_free(result);

clean_exit:
    free_token_arr(&arr);
    return NULL;
}

/*

static ast_t* parse_pipeline();
static ast_t* parse_simple_command();
static ast_t* parse_redirection(ast_t*);

*/