#ifndef PARSER_PARSER_H_
#define PARSER_PARSER_H_



#ifdef COMPILING_PARSER
    extern int g_abort_ast;
#endif

// Main parser function
ast_t* parse_string(char* cmdline);

#endif // PARSER_PARSER_H_