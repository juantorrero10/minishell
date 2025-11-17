#include <mshparser.h>


// Main procedure
ast_t* parse_command();

static ast_t* parse_list();
static ast_t* parse_pipeline();
static ast_t* parse_simple_command();
static ast_t* parse_redirection(ast_t*);