#ifndef PARSER_AST_H_
#define PARSER_AST_H_

//AST related functions for the parser module
ast_t* ast_create_empty();
void ast_free(ast_t* t);

#endif // PARSER_AST_H_