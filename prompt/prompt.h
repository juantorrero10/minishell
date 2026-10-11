#ifndef PROMPT_H_
#define PROMPT_H_

#include <stdbool.h>

extern int g_last_error_code;

char* prompt_get();
void prompt_append_cwd(bool abrv_home);
void prompt_append_username();
void prompt_append_last_errorcode();
void prompt_append_str(char* s);
void prompt_append_hostname();
void prompt_append_git_branch();
void prompt_clear();

// COLOR 
#define PROMPT_COLOR_RESET(stream)    prompt_append_str("\001\033[0m\002")
#define PROMPT_STYLE_BOLD(stream)     prompt_append_str("\001\033[1m\002")
#define PROMPT_STYLE_ITALIC(stream)   prompt_append_str("\001\033[3m\002")
#define PROMPT_STYLE_UNDERLINE(stream)   prompt_append_str("\001\033[4m\002")

#define PROMPT_COLOR_BLACK(stream)       prompt_append_str("\001\033[30m\002")
#define PROMPT_COLOR_RED(stream)         prompt_append_str("\001\033[31m\002")
#define PROMPT_COLOR_GREEN(stream)       prompt_append_str("\001\033[32m\002")
#define PROMPT_COLOR_YELLOW(stream)      prompt_append_str("\001\033[33m\002")
#define PROMPT_COLOR_BLUE(stream)        prompt_append_str("\001\033[34m\002")
#define PROMPT_COLOR_MAGENTA(stream)     prompt_append_str("\001\033[35m\002")
#define PROMPT_COLOR_CYAN(stream)        prompt_append_str("\001\033[36m\002")
#define PROMPT_COLOR_WHITE(stream)       prompt_append_str("\001\033[37m\002")
#define PROMPT_COLOR_GREY(stream)         prompt_append_str("\001\033[90m\002")


#define PROMPT_COLOR_BRIGHT_RED(stream)     prompt_append_str("\001\033[91m\002")
#define PROMPT_COLOR_BRIGHT_GREEN(stream)   prompt_append_str("\001\033[92m\002")
#define PROMPT_COLOR_BRIGHT_YELLOW(stream)  prompt_append_str("\001\033[93m\002")
#define PROMPT_COLOR_BRIGHT_BLUE(stream)    prompt_append_str("\001\033[94m\002")
#define PROMPT_COLOR_BRIGHT_MAGENTA(stream) prompt_append_str("\001\033[95m\002")
#define PROMPT_COLOR_BRIGHT_CYAN(stream)    prompt_append_str("\001\033[96m\002")
#define PROMPT_COLOR_BRIGHT_WHITE(stream)   prompt_append_str("\001\033[97m\002")

#endif //PROMPT_H_