#ifndef READ_H_
#define READ_H_

#include <minishell.h>

extern struct termios g_oldattr;

int read_line_input(char* buff, size_t max, bool);
void free_tokens(tline* t);
#endif // READ_H_