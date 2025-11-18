#ifndef PARSER_TOKENIZER_H_
#define PARSER_TOKENIZER_H_

#define isokforwords(c) (isalnum(c) || c == '_' || c == '.')

// Tokenizer functions for the parser module
token_arr tokenize(char* cmdline, _out_ size_t* arr_sz);
void free_token_arr(token_arr* a);



#endif // PARSER_TOKENIZER_H_