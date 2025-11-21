#include <mshparser.h>


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
}

static void ast_free_pipeline(ast_node_pipeline_t* ppl) {
    for (size_t i = 0; i < ppl->ncommands; i++)
    {
        ast_free(ppl->elements + i);
    }
}

void ast_free(ast_t* t) {
    if (!t) return;
    switch (t->type)
    {
    case AST_BG:
        ast_free(t->node.bg.children);
        break;
    case AST_COMMAND:
        ast_free_command(&(t->node.cmd));
        break;
    case AST_PIPELINE:
        ast_free_pipeline(&(t->node.ppl));
        break;
    case AST_REDIR:
        ast_free_redir(t->node.redir); break;
    case AST_GROUP:
    case AST_SUBSHELL:
        ast_free(t->node.grp.children);
        break;
    case AST_SUBST:
        ast_free(t->node.sub.children);
        break;
    case AST_LIST:
        ast_free(t->node.sep.left);
        ast_free(t->node.sep.right);
        break;
    default:
        break;
    }
    //free(t);
}