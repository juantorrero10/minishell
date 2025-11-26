#ifndef PARSER_GRAMMAR_H_
#define PARSER_GRAMMAR_H_

// tokens and ast nodes

/*---------------------- TOKENS --------------------------*/
#define TT_SEP_IDX 10    //Separator  ||, &&, ...
#define TT_GR_IDX 20     //Groupers   ), (, }, ...
#define TT_RD_IDX 30    //Redirections
#define TT_FD_IDX 40    //File descriptor
#define TT_EOL_IDX 50   // EOL
typedef enum {
    TOK_WORD=0,                 // Simple command
    TOK_PIPE=TT_SEP_IDX,        // "|""
    TOK_AND_IF,                 // "&&"
    TOK_OR_IF,                  // "||"
    TOK_SEMI,                   // ";"
    TOK_AMP,                    // "&"
    TOK_LPAREN=TT_GR_IDX,       // "("
    TOK_CMD_ST_START,           // "$("
    TOK_CMD_ST_END,             // ")" same but distinted
    TOK_RPAREN,                 // ")"
    TOK_LBRACE,                 // "{"
    TOK_RBRACE,                 // "}"
    TOK_DQ_START,               // """
    TOK_DQ_END,                 // """
    TOK_REDIR_OUT=TT_RD_IDX,    // ">"
    TOK_REDIR_OUT_APPEND,       // ">>"
    TOK_REDIR_IN,               // "<"
    TOK_REDIR_HEREDOC,          // "<<"
    TOK_REDIR_HERESTR,          // "<<<"
    TOK_REDIR_DUP_OUT,          // ">&"
    TOK_REDIR_DUP_IN,           // "<&"
    TOK_REDIR_READ_WRITE,       // "<>"
    TOK_REDIR_RHS_FD=TT_FD_IDX, // N : for N in {0, 1, 2, ...}
    TOK_REDIR_LHS_FD,           // N : for N in {0, 1, 2, ...}
    TOK_REDIR_RHS_CLOSE,        // "-"
    TOK_EOL=TT_EOL_IDX,                    // end of line
    TOK_ERROR=-1,               // Invalid token
} typeof_token;

typedef enum {
    TC_WORD,
    TC_PIPE,
    TC_CMD_SUB_START,
    TC_CMD_SUB_END,
    TC_GROUP_START,
    TC_GROUP_END,
    TC_SEP,
    TC_BG,
    TC_REDIR,
    TC_RD_ST,   // redir settings (fd)
    TC_DQ_START,
    TC_DQ_END,
    TC_ERROR
}token_cat;

typedef struct _token_generic_type{
    typeof_token type;
    char *value;    // for WORDs, strings
    int number;     // fd number (optional)
    int str_idx;
}token_t;

typedef struct _token_arr_type {
    token_t* ptr;
    size_t allocated;
    size_t occupied;
}token_arr; 

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


typedef enum {
    REDIR_FILE,     // > file, < file
    REDIR_FD,       // >&1, <&3
    REDIR_HEREDOC,  // <<EOF
    REDIR_HERESTR,  // <<< "string"
    REDIR_CLOSE
} redir_target_t;

typedef struct {
    int left_fd;          // e.g. 1 in "1>file"
    typeof_token op;      // >, >>, <, <<, <&, >&, <>, <<<
    redir_target_t kind;    // type of target
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
    size_t ncommands;
} ast_node_pipeline_t;

// They can either be a group or a subshell
typedef enum {GROUP_SUBSHELL, GROUP_GENERIC} typeof_group;
typedef struct {
    typeof_group group_type;
    ast_t* children;
    ast_node_redir_t* redirs;       // Array of redirections
    size_t nredirs;
} ast_node_group_t;

typedef struct {
    ast_t* children;
} ast_node_substitution_t;


typedef struct {
    typeof_token sep_type;          // type: {AND_IF, OR_IF, SEMI}
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



#endif // PARSER_GRAMMAR_H_