#ifndef PARSER_PARSER_UTILS_H_
#define PARSER_PARSER_UTILS_H_


// Unordered sort of helper functions.


void pu_peek(token_arr* arr);
int pu_check_balance(char* cmdline, size_t view);
token_cat get_category(typeof_token tt);
token_arr make_arr_view(token_arr* arr, size_t start, size_t end);
int find_list_sep(token_arr* arr, const char* cmdline);
int find_pipe(token_arr* arr, const char* cmdline);
bool type_in_list(typeof_token t, typeof_token* l, size_t sz);

#endif // PARSER_PARSER_UTILS_H_