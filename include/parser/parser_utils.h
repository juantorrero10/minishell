#ifndef PARSER_PARSER_UTILS_H_
#define PARSER_PARSER_UTILS_H_


// Unordered sort of helper functions.


void pu_peek(token_arr* arr);
int pu_check_balance(char* cmdline, size_t view);
token_cat get_category(typeof_token tt);
token_arr make_arr_view(token_arr* arr, size_t start, size_t end);
int find_list_sep(token_arr* arr, const char* cmdline, bool __only_semicolon);
int find_pipe(token_arr* arr, const char* cmdline);
int find_redirs(token_arr* arr, const char* cmdline, _out_ int* n_redirs);
int find_cmd_sub(token_arr* arr);
char* find_binary_path(const char* name);
bool is_a_group(token_arr* arr, const char* cmdline);
bool type_in_list(typeof_token t, typeof_token* l, size_t sz);
int redir_default_fd(typeof_token rd);

#endif // PARSER_PARSER_UTILS_H_