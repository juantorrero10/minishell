#include <mshparser.h>
#include <../log.h>

ast_t* ast_create_empty() {
    ast_t* ret = malloc(sizeof(ast_t));
    memset(ret, 0, sizeof(ast_t));
    ret->type = AST_INVALID; return ret;
}

ast_t* ast_create_array(size_t n_trees) {
    INFO("n_trees");
    ast_t* t = malloc(sizeof(ast_t) * n_trees);
    memset(t, 0, sizeof(ast_t) * n_trees);
    for (size_t i = 0; i < n_trees; i++) t[i].type = AST_INVALID;
    return t;
}

static void ast_free_redir(ast_node_redir_t rd) {
    if (rd.target.filename) free(rd.target.filename);
    if (rd.target.string)   free(rd.target.string);
    if (rd.target.delimiter)     free(rd.target.delimiter);
}

static void ast_free_command(ast_node_command_t* c) {
    if (!c) return;
    if (c->argv) {
        for (int i = 0; i < (int)c->argc; i++) {
            if (c->argv[i]) {
                free(c->argv[i]);
                c->argv[i] = NULL;
            }
        }
        // free null termination
        free(c->argv[c->argc]);
        free(c->argv);
        c->argv = NULL;
    }
    c->argc = 0;
    if (c->filename) {
        free(c->filename);
        c->filename = NULL;
    }
    if (c->nredirs && c->redirs) {
        for (size_t i = 0; i < c->nredirs; ++i) {
            ast_free_redir(c->redirs[i]);
        }
        free(c->redirs);
        c->redirs = NULL;
        c->nredirs = 0;
    }
}

static void ast_free_pipeline(ast_node_pipeline_t* ppl) {
    ast_t* elem; (void) elem;

    for (size_t i = 0; i < ppl->ncommands; i++)
    {
        elem = ppl->elements + i;
        ast_free(ppl->elements + i);
    }
    free(ppl->elements);
}

void ast_free(ast_t* t) {
    if (!t) return;
    switch (t->type)
    {
    case AST_BG:
        if (t->node.bg.children) {
            ast_free(t->node.bg.children);
        }
        break;
    case AST_COMMAND:
        ast_free_command(&(t->node.cmd));
        break;
    case AST_PIPELINE:
        ast_free_pipeline(&(t->node.ppl));
        break;
    case AST_REDIR:
        ast_free_redir(t->node.redir);
        break;
    case AST_GROUP:
    case AST_SUBSHELL:
        if (t->node.grp.children) {
            ast_free(t->node.grp.children);
            /* don't free t->node.grp.children pointer here — caller will if it's a heap ptr */
        }
        if (t->node.grp.redirs) {
            // free array of redirs if allocated (adjust per your definitions)
            free(t->node.grp.redirs);
            t->node.grp.redirs = NULL;
            t->node.grp.nredirs = 0;
        }
        break;
    case AST_SUBST:
        if (t->node.sub.children) {
            ast_free(t->node.sub.children);
        }
        break;
    case AST_LIST:
        if (t->node.sep.left)  { ast_free(t->node.sep.left);  free(t->node.sep.left);  }
        if (t->node.sep.right) { ast_free(t->node.sep.right); free(t->node.sep.right); }
        break;
    default:
        break;
    }

}