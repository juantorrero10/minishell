#ifndef PARSER_ERRORS_H_
#define PARSER_ERRORS_H_

#include <colors.h>

#define ERR_LBL "syntax error"
#define CLR_LBL "    >>>     "
#define LPERR(MSG, ...) fprintf(stderr, STYLE_BOLD""COLOR_RED""ERR_LBL": "COLOR_RESET""MSG"\r\n", ##__VA_ARGS__)
#define LPCLR(MSG, ...) fprintf(stderr, STYLE_BOLD""COLOR_RED""CLR_LBL": "COLOR_RESET""MSG"\r\n", ##__VA_ARGS__)

typedef enum {
    ERR_UNEXP,
    ERR_EXP,
    ERR_UNTERM_DQUOTE,
    ERR_UNTERM_SQUOTE,
    ERR_UNTERM_PAREN,
    ERR_UNTERM_BRACKET,
    ERR_NEST_TOO_DEEP,
    ERR_MISMATCHED_BRACKET,
    ERR_INVALID_REDIR_SYNTAX,
    ERR_INVALUE
} err_t; 

// Functions for displaying errors
void error_parse(err_t e, char* s);
void error_clarify(char* s);

#endif // PARSER_ERRORS_H_