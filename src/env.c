/**
 * Funciones de manejo de variables del entorno
 */
#include <minishell.h>

#include <log.h>

#define MAX_VARNAME_COPY 256

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/**
 * @brief Helper to check if a character is valid inside an environment variable name.
 */
static inline int is_var_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

static size_t expansion_pass(const char* og, char* new, bool fill_buff) {
    char varname[MAX_VARNAME_COPY];
    size_t idx = 0;
    size_t var_end = 0;
    size_t new_idx = 0;
    bool in_brackets;
    bool dollar_encountered = false;

    // First pass: determine the allocation size
    while (og[idx]) {
        in_brackets = false;
        if (og[idx] == '$') {
            dollar_encountered = true;
            idx++;
            if (og[idx] == '{') {
                in_brackets = true;
                idx++;
            }
            var_end = idx;
            while (og[var_end] && is_var_char(og[var_end])) var_end++;
            size_t copy_sz = var_end - idx;
            memcpy(varname, &og[idx], MIN(MAX_VARNAME_COPY - 1, copy_sz));
            varname[copy_sz] = '\0';
            char* var_value = getenv(varname);
            if (var_value) {
                if (fill_buff) {
                    size_t val_sz = strlen(var_value);
                    memcpy(&new[new_idx], var_value, val_sz);
                }
                new_idx += strlen(var_value);
            }
            idx = var_end + ((in_brackets && og[var_end] == '}')? 1 : 0);
            continue;
        }
        if (fill_buff) {
            new[new_idx] = og[idx];
        }
        idx++;
        new_idx++;
    }
    if (fill_buff) {
        new[new_idx] = '\0';
    }
    if (!dollar_encountered) {
        return 0;
    }
    return ++new_idx;
}  

/**
 * @brief Expands string into a newly allocated buffer or inside the provided one.
 * @param og Original string.
 * @param buff [Optional] if !NULL the new string (expanded or not) will be appended
 * inside the buffer
 * @return 
 *  Expanded string or NULL if no expansion was needed. 
 *  If a buffer was provided, it will return buff->data.
 * @note New string needs to be freed if no buffer provided.
 */
char* env_expand_string(
    const char* og, 
    _opt_ struct env_growable_string* buff
) {
    if (!og || !(og[0])) {
        return NULL;
    }

    if (!buff) {
        /**
         * In parset/tokenizer.c::carve_word()
         * An extra byte is alloced to determine if the string needs to be expanded
         */
        size_t len = strlen(og);
        bool needs_expansion = (bool)(*(unsigned char*)(og + len + 1));
        if (!needs_expansion) return NULL;
    }
    
    // First pass: figure out allocation size
    size_t new_sz = expansion_pass(og, NULL, false);

    if (!buff && new_sz == 0) {
        return NULL;
    }


    if (buff) {
        bool just_concat = false;
        if (new_sz == 0) {
            just_concat = true;
            new_sz = strlen(og);
        }
        size_t new_len = new_sz + buff->len;
        if (new_len >= buff->cap) {
            balloc(buff->cap);
            buff->cap *= 2;
        }
        if (!just_concat) {
            expansion_pass(og, buff->data + buff->len, true);
            buff->len = new_len - 1;
        } else {
            memcpy(buff->data + buff->len, og, new_sz);
            buff->len += new_sz;
        }
        return buff->data;
    }
    char* new = balloc(new_sz);
    expansion_pass(og, new, true);
    return new;
}

/**
 * @brief Expand the enviroment vars in the strings of the redirections
 */
void env_expand_redirs(ast_node_redir_t* redirs, size_t nredirs) {
    if (!redirs || nredirs == 0) return;
    for (size_t i = 0; i < nredirs; i++)
    {
        redir_target t = redirs[i].target_kind;
        if (t == REDIR_TARGET_FILE || t == REDIR_TARGET_HERESTR) {
            char* new_string = env_expand_string(redirs[i].target.filename, NULL);
            if (new_string == NULL || new_string == 0) {
                continue;
            }
            // Replace string
            redirs[i].target.filename = new_string;
        }
    }
}

/**
 * @deprecated Env vars are expanded individually the moment they are needed.
 * @brief Recursive expansion of enviroment variables of all found strings of an AST.
 * @param ast AST to be expanded.
 */
void env_expand_ast(ast_t* ast) {
    char* new_string = NULL;

    if (!ast) {
        return;
    }
    switch (ast->type) {
        case AST_COMMAND:
            for (int i = 0; i < ast->node.cmd.argc; i++)
            {
                new_string = env_expand_string(ast->node.cmd.argv[i], NULL);
                if (new_string == NULL) {
                    continue;
                }
                // Replace string
                free(ast->node.cmd.argv[i]);
                ast->node.cmd.argv[i] = new_string;
            }
            env_expand_redirs(ast->node.cmd.redirs, ast->node.cmd.nredirs);
            break;
        case AST_BG:
            env_expand_ast(ast->node.bg.children);
            break;
        case AST_GROUP:
            env_expand_ast(ast->node.grp.children);
            env_expand_redirs(ast->node.grp.redirs.data, ast->node.grp.redirs.sz);
            break;
        case AST_LIST:
            env_expand_ast(ast->node.sep.left);
            env_expand_ast(ast->node.sep.right);
            break;
        case AST_PIPELINE:
            ast_t* elems = ast->node.ppl.elements; 
            if (!elems) break;
            for (size_t i = 0; i < ast->node.ppl.nelements; i++)
            {
                env_expand_ast(&elems[i]);
            }
            break;
        case AST_SUBSHELL:
        case AST_SUBST:
            env_expand_ast(ast->node.sub.children);
            break;
        default:
            break;
    }
}