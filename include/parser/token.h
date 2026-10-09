#ifndef PARSER_TOKEN_H_
#define PARSER_TOKEN_H_

#define is_last_token(arr, idx) (idx >= (int)arr->count - 1 || arr->ptr[idx+1].type == TOK_EOL)
#define tok_type(arr, idx) (arr->ptr[idx].type)
#define strloc(arr, idx) (arr->ptr[idx].str_idx)

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
} token_kind;

typedef enum {
    TC_INVALID,
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
} token_category;

typedef struct _token_generic_type{
    token_kind type;
    char *value;    // for WORDs, strings
    int number;     // fd number (optional)
    int str_idx;
} token_t;

typedef struct _token_arr_type {
    token_t* ptr;
    size_t capacity;
    size_t count;
} token_arr; 

#endif // PARSER_TOKEN_H_