#ifndef READ_H_
#define READ_H_
extern void (*g_prompt_push_fptr)(void);
int read_line_input(char* buff, size_t max, bool print_prompt);
size_t read_additional_line(_out_ char** ptr);
#endif // READ_H_