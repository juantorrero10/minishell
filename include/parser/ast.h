#ifndef PARSER_AST_H_
#define PARSER_AST_H_

//AST related functions for the parser module

// Interface for the parser module

/*-------------------------- AST -------------------------*/
typedef enum {
    AST_COMMAND,       // simple command
    AST_PIPELINE,      // a | b | c
    AST_LIST,          // a && b || c ; d
    AST_REDIR,         // command with redirects (includes the fd numbers)
    AST_SUBSHELL,      // ( command list )
    AST_SUBST,         // substitution echo "$(find / -type f)"
    AST_GROUP,         // { ... }
    AST_BG,
    AST_INVALID=-1
} ast_type_t;


typedef struct _ast_generic_type ast_t;

// We need the enumerations to have the same values as the token_kind ones.
// Given that, you can convert to them with a simple cast.
#define TT_AND_START 11       //  TOK_AND_IF idx
#define TT_REDIR_START 30     //  TOK_REDIR_OUT idx

typedef enum {
    REDIR_OUT=TT_REDIR_START,    // ">"
    REDIR_OUT_APPEND,       // ">>"
    REDIR_IN,               // "<"
    REDIR_HEREDOC,          // "<<"
    REDIR_HERESTR,          // "<<<"
    REDIR_DUP_OUT,          // ">&"
    REDIR_DUP_IN,           // "<&"
    REDIR_READ_WRITE,       // "<>"
} redir_kind;


typedef enum {
    REDIR_TARGET_FILE,     // > file, < file
    REDIR_TARGET_FD,       // >&1, <&3
    REDIR_TARGET_HEREDOC,  // <<EOF
    REDIR_TARGET_HERESTR,  // <<< "string"
    REDIR_TARGET_CLOSE     // "-"
} redir_target;

typedef struct {
    int left_fd;          // e.g. 1 in "1>file"
    redir_kind op;      // >, >>, <, <<, <&, >&, <>, <<<
    redir_target target_kind;    // type of target
    union {
        char *filename;   // for file redir
        int  fd;          // for fd duplication
        char *delimiter;  // for heredoc
        char *string;     // for <<< string
    } target;
} ast_node_redir_t;

typedef struct {
    char **argv;                    //NULL terminated
    int argc;
    char* filename;                 //NULL if internal or non-existent
    ast_node_redir_t* redirs;       // Array of redirections
    size_t nredirs;                 // N of redirs
} ast_node_command_t;

typedef struct {
    ast_t* elements;
    size_t nelements;
} ast_node_pipeline_t;

// They can either be a group or a subshell
typedef enum {GROUP_SUBSHELL, GROUP_GENERIC} group_kind;
struct redir_arr {
    ast_node_redir_t* data;
    size_t sz;
};

typedef struct {
    group_kind group_type;
    ast_t* children;
    struct redir_arr redirs;
} ast_node_group_t;

typedef struct {
    ast_t* children;
} ast_node_substitution_t;

typedef enum {SEP_AND = TT_AND_START, SEP_OR, SEP_SEMICOLON, SEP_AMP} separator_kind;
typedef struct {
    separator_kind sep_type;          // type: {AND_IF, OR_IF, SEMI}
    ast_t *left;
    ast_t *right; 
} ast_node_list_t;

typedef struct {
    ast_t* children;
} ast_node_background_t;


struct _ast_generic_type {
    ast_type_t type;
    union {
        ast_node_command_t cmd;
        ast_node_pipeline_t ppl;
        ast_node_list_t sep;
        ast_node_group_t grp;
        ast_node_substitution_t sub;
        ast_node_redir_t  redir;
        ast_node_background_t bg;
    }node;
};

void ast_free(ast_t* a);

#ifdef COMPILING_PARSER
    ast_t* ast_create_empty();
    ast_t* ast_create_array(size_t n_trees);
#endif

#endif // PARSER_AST_H_