#ifndef PARSER_TOKENIZER_H_
#define PARSER_TOKENIZER_H_

#define WORD_BASE "-_.=/$"
#define WORD_SPCHARS ":;\\?! "

#define isokforwords(c) (isalnum(c) || strchr(WORD_BASE, c)) 
#define isokforwords_sp(c) (isokforwords(c) || strchr(WORD_SPCHARS, c))
#define isokforvars(c) (isalnum(c) || c == '_')

// Tokenizer functions for the parser module
token_arr __tokenize(char* cmdline, _out_ int* st);
void free_token_arr(token_arr* a);

/**ove noticed that the logic is wrong, vars should not be detected for now. if you do "User: $USER" it compunds to two words User and xxxxxx (being whatever the user is). It should be we continous word, any suggestions? */

#endif // PARSER_TOKENIZER_H_