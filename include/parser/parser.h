#ifndef PARSER_PARSER_H_
#define PARSER_PARSER_H_



#ifdef COMPILING_PARSER
extern int g_abort_ast;

#define is_last_token(arr, idx) (idx >= (int)arr->count - 1 || arr->ptr[idx+1].type == TOK_EOL)
#define tok_type(arr, idx) (arr->ptr[idx].type)
#define strloc(arr, idx) (arr->ptr[idx].str_idx)
#endif

// Main parser function
ast_t* parse_string(char* cmdline);

#endif // PARSER_PARSER_H_