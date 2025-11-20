#include <mshparser.h>
/*
/**
 * @param start zero indexed
 * @param end   zero indexed
 *//*
static token_arr make_arr_view(token_arr* arr, size_t start, size_t end) {
    token_arr ret = (token_arr){0};
    if (end == -1) end = arr->occupied - 1;

    //cap with real values
    start = (start > arr->occupied-1)? arr->occupied-1 : start;
    end = (end > arr->occupied-1)? arr->occupied-1 : end;

    ret.ptr = arr->ptr + start;
    ret.occupied = end - start + 1;

    ret.allocated = arr->allocated - start;
    return ret;
}

static bool arr_contains(token_arr* arr, typeof_token tt, bool __reverse_search) {
    size_t i = 0;

    if (!__reverse_search) {
        while (arr->ptr[i].type != TOK_EOL) {
            if (arr->ptr[i++].type == tt) return true;
        }
        return false;
    }
    i = arr->occupied-1;
    for (; i >= 0; i--)
    {
        if (arr->ptr[i].type == tt) return true;
    }
    return false;
    
}

*/



ast_t* ast_create_empty() {
    ast_t* ret = malloc(sizeof(ast_t));
    memset(ret, 0, sizeof(ast_t));
    ret->type = AST_INVALID; return ret;
}

static void ast_free_redir(ast_node_redir_t rd) {
    if (rd.target.fd > 999) {
        // Doesnt matter which one
        free(rd.target.delimiter);
    }
}

static void ast_free_command(ast_node_command_t* c) {
    if (!c) return;
    for (int i = 0; i < c->argc; i++)
    {
        if (c->argv[i])free(c->argv[i]);
        c->argv[i] = NULL;
    }
    if (c->filename) free(c->filename);
    c->filename = NULL;
    if (c->nredirs) {
        free(c->redirs);
    }
    free(c);
}

static void ast_free_pipeline(ast_node_pipeline_t* ppl) {
    for (int i = 0; i < ppl->ncommands; i++)
    {
        ast_free_command(ppl->elements + i);
    }
    free(ppl);
}

void ast_free(ast_t* t) {
    if (!t) return;
    switch (t->type)
    {
    case AST_COMMAND:
        ast_free_command(&t->node.cmd);
        break;
    case AST_PIPELINE:
        ast_free_pipeline(&t->node.ppl);
        break;
    case AST_REDIR:
        ast_free_redir; break;
    case AST_GROUP:
    case AST_SUBSHELL:
        ast_free(t->node.grp.children);
        free(&t->node.grp); break;
    case AST_SUBST:
        ast_free(t->node.sub.children);
        free(&t->node.sub); break;
    case AST_LIST:
        ast_free(t->node.sep.left);
        ast_free(t->node.sep.right);
        free(&t->node.sep);
    }
}