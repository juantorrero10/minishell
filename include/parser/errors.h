#ifndef PARSER_ERRORS_H_
#define PARSER_ERRORS_H_

#include <../macros.h>

#define ERR_LBL "syntax error"

typedef enum {
    ERR_UNEXP,
    ERR_UNTERM,
    ERR_INVALUE
} err_t; 

// Functions for displaying errors
void error_parse(err_t e, char* s);

#endif // PARSER_ERRORS_H_