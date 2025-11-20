#include <mshparser.h>

int g_abort_ast = 0;

static ast_t* parse_line(token_arr* arr, const char* cmdline);

static ast_t* parse_simple_command(token_arr* arr, const char* cmdline);

static ast_t* parse_group(token_arr* arr, const char* cmdline);

static ast_node_redir_t* parse_redirections(token_arr* arr, const char* cmdline);

static ast_t* parse_list(token_arr* arr, int idx, const char* cmdline) {
    ast_t* ret = ast_create_empty();    // Defaults to AST_INVALID
    ast_node_list_t sep = (ast_node_list_t){0};
    token_arr view_left;
    token_arr view_right;

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
    ast_t* ret = ast_create_empty();    // Defaults to AST_INVALID
    ast_node_pipeline_t ppl = (ast_node_pipeline_t){0};
    token_arr view;
    char *ptr           = cmdline;
    int ppl_end;        = 0;
    int idx             = 0;
    int cmd_st          = 0;
    int cmd_end         = 0;
    // tokens allowed in a pipeline (using this to delimit the pipeline)
    // size: 6
    typeof_token allowed[] = {TOK_WORD, 
        TOK_PIPE, TOK_DQ_START, TOK_DQ_END, TOK_CMD_ST_START, TOK_CMD_ST_END};
    int cmd_sub = 0;
    int quoted = 0;

    ret->type = AST_PIPELINE;
    
    // grammar rule: if not in a cmd sub, only tokens in [allowed] are valid.
    while (arr->ptr[idx].type != TOK_EOL) 
    {
        if (arr->ptr[idx].type == TOK_CMD_ST_START) cmd_sub++;
        if (arr->ptr[idx].type == TOK_CMD_ST_END) cmd_sub--;
        if ((!type_in_list(arr->ptr[idx].type, allowed, 6)) && cmd_sub <= 0)
            {ppl_end = idx; break;}
        idx++;
    }

    // grammar rule: if last tok is pipe -> unexpected token
    // ex:        find / -type f | &grep -v "..."
    //                           ^ end
    if (arr->ptr[ppl_end].type == TOK_PIPE) {
        error_parse(ERR_UNEXP, ptr + arr->ptr[ppl_end + 1].str_idx);
        free(ret); g_abort_ast=1; return NULL;
    }

    idx = 0;
    while (idx <= ppl_end) {
        if (arr->ptr[idx].type == TOK_CMD_ST_START) cmd_sub++;
        if (arr->ptr[idx].type == TOK_CMD_ST_END) cmd_sub--;

        if (arr->ptr[idx].type == TOK_PIPE && !cmd_sub) {
            cmd_end = idx - 1;
            view = make_arr_view(arr, cmd_st, cmd_end);
            // Only allowing simple commands in pipelines.
            ppl.elements[ppl.ncommands++] = *parse_line(&view, cmdline);
        }
        idx++;
    }

    ret->node.ppl = ppl;
    return ret;
    
}



ast_t* parse_line(token_arr* arr, const char* cmdline) {
    // HIGHER LEVEL
    // 0. background flag
    // 1. parse_list
    // 3. parse_pipeline
    // 4. parse_command (simple/compound) -> group
    // 5. words         inside parse_command.
    // LOWER LEVEL
    ast_t* ret  = NULL; (void) ret;
    int found   = 0;
    token_arr view;
    
    // Reset abort signal
    g_abort_ast = 0;

    // Background flag
    if (arr->ptr[arr->occupied-2].type == TOK_AMP) {
        ret->type = AST_BG;
        view = make_arr_view(arr, 0, arr->occupied-3);
        return parse_line(&view, cmdline);
    }

    found = find_list_sep(arr, cmdline);
    if (g_abort_ast) return NULL;
    if (found >= 0) {
        return parse_list(arr, found, cmdline);
    }

    found = find_pipe(arr, cmdline);
    if (g_abort_ast) return NULL;
    if (found >= 0) {
        return parse_pipeline(arr, cmdline);
    }

    //TODO: parse compund commands
    return ret;

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