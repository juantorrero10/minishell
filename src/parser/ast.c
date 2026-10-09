#include <private.h>
#include <../log.h>



ast_t* ast_create_empty() {
    ast_t* ret = malloc(sizeof(ast_t));
    memset(ret, 0, sizeof(ast_t));
    ret->type = AST_INVALID; 
    return ret;
}

ast_t* ast_create_array(size_t n_trees) {
    INFO("n_trees");
    ast_t* t = malloc(sizeof(ast_t) * n_trees);
    memset(t, 0, sizeof(ast_t) * n_trees);
    for (size_t i = 0; i < n_trees; i++) t[i].type = AST_INVALID;
    return t;
}

static void ast_free_contents(ast_t* t);

void ast_free_redir(ast_node_redir_t* rd) {
    if (rd->target_kind != REDIR_TARGET_FD && rd->target_kind != REDIR_TARGET_CLOSE) {
        free(rd->target.filename);
        rd->target.filename = NULL;
    }
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
        free(c->argv);
        c->argv = NULL;
    }
    c->argc = 0;
    if (c->nredirs && c->redirs) {
        for (size_t i = 0; i < c->nredirs; ++i) {
            ast_free_redir(&c->redirs[i]);
        }
        free(c->redirs);
        c->redirs = NULL;
        c->nredirs = 0;
    }
}

static void ast_free_pipeline(ast_node_pipeline_t* ppl) {
    ast_t* elem; (void) elem;

    for (size_t i = 0; i < ppl->nelements; i++)
    {
        elem = ppl->elements + i;
        ast_free_contents(ppl->elements + i);
    }
    free(ppl->elements);
}

static void ast_free_contents(ast_t* t) {
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
        ast_free_redir(&t->node.redir);
        break;
    case AST_GROUP:
    case AST_SUBSHELL:
        if (t->node.grp.children) {
            ast_free(t->node.grp.children);
            /* don't free t->node.grp.children pointer here — caller will if it's a heap ptr */
        }
        if (t->node.grp.redirs.data) {
            // free array of redirs if capacity (adjust per your definitions)
            for (size_t i = 0; i < t->node.grp.redirs.sz; i++)
                ast_free_redir(&t->node.grp.redirs.data[i]);
            
            free(t->node.grp.redirs.data);
            t->node.grp.redirs = (struct redir_arr){0};
        }
        break;
    case AST_SUBST:
        if (t->node.sub.children) {
            ast_free(t->node.sub.children);
        }
        break;
    case AST_LIST:
        if (t->node.sep.left)  { ast_free(t->node.sep.left);}
        if (t->node.sep.right) { ast_free(t->node.sep.right); }
        break;
    case AST_INVALID:
    default:
        break;
    }
}

void ast_free(ast_t* t) {
    ast_free_contents(t);
    free(t);
}