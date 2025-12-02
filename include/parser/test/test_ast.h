#ifndef PARSER_TEST_TEST_AST_H_
#define PARSER_TEST_TEST_AST_H_

#include <mshparser.h>
#include <../colors.h>



void test_ast_to_string(ast_t* ast, char* buff, size_t buff_sz, int ind, bool do_indent);
void test_start_test(char* file_input, char* file_output_summary);


#define test_lbl "[TEST]: "
#define test_entry COLOR_GREY"> "COLOR_RESET

#define TINFO(MSG, ...) fprintf(stdout, COLOR_BRIGHT_CYAN""test_lbl""COLOR_RESET""MSG"\r\n", ##__VA_ARGS__);
#define TOKAY(MSG, ...) fprintf(stdout, COLOR_BRIGHT_GREEN""test_lbl""COLOR_RESET""MSG"\r\n", ##__VA_ARGS__);
#define TWARN(MSG, ...) fprintf(stderr, COLOR_YELLOW""test_lbl""COLOR_RESET""MSG"\r\n", ##__VA_ARGS__);
#define TERROR(MSG, ...) fprintf(stderr, COLOR_BRIGHT_RED""STYLE_BOLD""test_lbl""COLOR_RESET""MSG"\r\n", ##__VA_ARGS__);



#define concat_indent(buff, scratch, ind, n_spaces, sz) for (int cc__i = 0; cc__i < n_spaces * ind; cc__i++) {\
    scratch[cc__i] = ' ';                            \
} scratch[n_spaces * ind] = '\0';                       \
strcat(buff, scratch);                                  \
memset(scratch, 0, sz)                                 


#define concat_str(str, buff, scratch, ind, n_spaces, sz) concat_indent(buff, scratch, ind, n_spaces, sz); \
strcat(buff, str)                               \


#define ASSERT_AST(input_str, output_str, out_flag) ast_t* a = parse_string(input_str); \
                                                    if (a) { \
                                                        test_ast_to_string(a, __buff, sizeof(output_str), 0, 0); \
                                                        out_flag = (strstr(__buff, output_str))? 1 : 0; \
                                                            ast_free(a); free(a); } \
                                                    else { out_flag = 2; }


#define ST_LABEL(out_flag, st) switch (out_flag)   \
{\
case 0: \
    st = COLOR_RED"[FAILED]"COLOR_RESET; \
    __failed++; __total++;\
    break;\
case 1:\
    st = COLOR_BRIGHT_GREEN"[OK]"COLOR_RESET;\
    __ok++; __total++;\
    break;\
case 2:\
    st = COLOR_YELLOW"[ABORTED]"COLOR_RESET;\
    __aborted++; __total++;\
    break;\
default:\
    st = COLOR_GREY"[???]"COLOR_RESET;\
    __total++;\
    break;\
}

#endif // PARSER_TEST_TEST_AST_H_