#ifndef PARSER_PARSER_UTIL_H_
#define PARSER_PARSER_UTIL_H_

extern int g_abort_ast;

#define last_token(arr, idx) (idx >= (int)arr->occupied - 1 || arr->ptr[idx+1].type == TOK_EOL)
#define tok_type(arr, idx) (arr->ptr[idx].type)
#define strloc(arr, idx) (arr->ptr[idx].str_idx)

// Main parser function
ast_t* parse_string(char* cmdline);

#endif // PARSER_PARSER_UTIL_H_