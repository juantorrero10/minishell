#include <minishell.h>
#include <parser/public.h>

int get_internal_idx(char* argv0) {
    int idx = 0;
    builtin_t* curr = g_builtin_function_table;

    if (!argv0) return -1;

    while (curr[idx].name)
    {
        if (!strcmp(curr[idx].name, argv0)) return idx;
        idx++;
    }

    return -1;
    
}

size_t get_npids(ast_t* tree, bool simple) {
    if (!tree) return 0;
    switch (tree->type)
    {
    case AST_COMMAND:
        if (tree->node.cmd.filename || !simple) {
            return 1;
        } else return 0;
    case AST_PIPELINE:
        return tree->node.ppl.nelements;
    case AST_GROUP:
        return get_npids(tree->node.grp.children, true);
    case AST_BG:
        return get_npids(tree->node.bg.children, false);
    case AST_LIST:
        return get_npids(tree->node.sep.left, simple) + 
            get_npids(tree->node.sep.right, simple);
    default:
        return 0;
    }
}


int handle_redirection(ast_node_redir_t* rd) {
    int new_fd = 0;

    //Default mask and mode values
    int flag_mask = O_CREAT | O_TRUNC | O_WRONLY;
    int mode_mask = 0644;

    switch (rd->op)
    {
    case TOK_REDIR_IN:
        flag_mask = O_RDONLY;
        goto L1;
    case TOK_REDIR_OUT_APPEND:
        flag_mask = O_CREAT | O_APPEND | O_WRONLY;
        goto L1;
    case TOK_REDIR_READ_WRITE:
        flag_mask = O_CREAT | O_RDWR;
        goto L1;
    case TOK_REDIR_OUT:
L1:
        if (!(flag_mask & O_CREAT)) {
            mode_mask = umask(0);
            umask(mode_mask);
        }
        if ((new_fd = open(rd->target.filename, flag_mask, mode_mask)) == -1) {
            MSH_ERR("couldn't open file '%s': %s", rd->target.filename, strerror(errno));
            g_abort_execution = 1;
            return EXIT_ERROR_OPENING_FILE;
        }
        dup2(new_fd, rd->left_fd);
        close(new_fd);
        break;
    case TOK_REDIR_DUP_IN:
    case TOK_REDIR_DUP_OUT:
        if (rd->kind == REDIR_CLOSE) {
            if (close(rd->left_fd) == -1) {
                g_abort_execution = 1;
                return EXIT_ERROR_CLOSING_FD;
            }
            break;
        }
        if (dup2(rd->target.fd, rd->left_fd) == -1) {
            MSH_ERR("couldn't duplicate fds: %d -> %d: %s", rd->target.fd, rd->left_fd, strerror(errno));
            g_abort_execution = 1;
            return EXIT_ERROR_DUPING_FD;
        } break;
    //todo: herestr and heredoc
    default:
        MSH_ERR("unknown redirection type or not supported yet.");
        g_abort_execution = 1;
        break;
    }

    return 0;
}