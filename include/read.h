#ifndef READ_H_
#define READ_H_

#include <minishell.h>

int read_line_input(char* buff, size_t max);
void free_tokens(tline* t);
#endif // READ_H_