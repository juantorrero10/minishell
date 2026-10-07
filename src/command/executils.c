#include <minishell.h>
#include <parser/public.h>

#include <sys/mman.h>

int eu_get_internal_idx(char* argv0) {
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

size_t eu_lookahead_get_npids(ast_t* tree, bool simple) {
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
        return eu_lookahead_get_npids(tree->node.grp.children, true);
    case AST_BG:
        return eu_lookahead_get_npids(tree->node.bg.children, false);
    case AST_LIST:
        return eu_lookahead_get_npids(tree->node.sep.left, simple) + 
            eu_lookahead_get_npids(tree->node.sep.right, simple);
    default:
        return 0;
    }
}

int create_buff_fd(const char *content) {
    int fd = memfd_create("minishell_heredoc", 0);
    if (fd == -1) return -1;

    if (content && content[0] != '\0') {
        write(fd, content, strlen(content));
    }

    lseek(fd, 0, SEEK_SET);

    return fd;
}


int eu_handle_redirection(ast_node_redir_t* rd) {
    int new_fd = 0;

    //Default mask and mode values
    int flag_mask = O_CREAT | O_TRUNC | O_WRONLY;
    int mode_mask = 0644;

    switch (rd->op)
    {
    case REDIR_IN:
        flag_mask = O_RDONLY;
        goto L1;
    case REDIR_OUT_APPEND:
        flag_mask = O_CREAT | O_APPEND | O_WRONLY;
        goto L1;
    case REDIR_READ_WRITE:
        flag_mask = O_CREAT | O_RDWR;
        goto L1;
    case REDIR_OUT:
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
    case REDIR_DUP_IN:
    case REDIR_DUP_OUT:
        if (rd->target_kind == REDIR_TARGET_CLOSE) {
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
    case REDIR_HERESTR:
        new_fd = create_buff_fd(rd->target.string);
        if (new_fd < 0) {
            MSH_ERR("couldn't create the herestr: %s", strerror(errno));
            g_abort_execution = 1;
            return EXIT_ERROR_HERESTR;
        }
        dup2(new_fd, 0);
        close(new_fd);
        break;
    case REDIR_HEREDOC:
        char* delim = rd->target.delimiter;
        char* buffer = malloc(sizeof(char) * INPUT_LINE_MAX);
        size_t curr = 0;
        size_t cap = INPUT_LINE_MAX;
        char line_buff[INPUT_LINE_MAX];
        
        while(1) {
            bool free_buff = false;
            M_COLOR_GREY(stdout);
            fprintf(stdout, ">  ");
            M_COLOR_RESET(stdout);
            fgets(line_buff, INPUT_LINE_MAX, stdin);

            size_t sz = strlen(line_buff);
            char old_last = line_buff[sz - 1];
            line_buff[sz - 1] = '\0';
            if (!strcmp(delim, line_buff)) {
                break;
            }
            line_buff[sz - 1] = old_last;
            char* src = env_expand_string(line_buff);
            if (!src) {
                src = line_buff;
            } else {
                free_buff = true;
                sz = strlen(src);
            }
            if (curr + sz >= cap) {
                cap *= 2;
                buffer = realloc(buffer, sizeof(char) * cap);
            }
            memcpy(&buffer[curr], src, sz);
            curr += sz;
            if (free_buff) {
                free(src);
            }
        }
        buffer[curr] = '\0';
        new_fd = create_buff_fd(buffer);
        if (new_fd < 0) {
            MSH_ERR("couldn't create the heredoc: %s", strerror(errno));
            g_abort_execution = 1;
            return EXIT_ERROR_HERESTR;
        }
        dup2(new_fd, 0);
        close(new_fd);
        free(buffer);
        break;
    default:
        MSH_ERR("unknown redirection type or not supported yet.");
        g_abort_execution = 1;
        break;
    }

    return 0;
}