#ifndef MACROS_H_
#define MACROS_H_

#define INPUT_LINE_MAX 1024
#define PATH_MAX 256

#define EXIT_COMMAND_NOT_FOUND (int)-1
#define EXIT_ERROR_OPENING_FILE (int)-2
#define EXIT_ERROR_FORKING (int)-3
#define EXIT_ERROR_CREATING_PIPE (int)-4
#define EXIT_ERROR_CREATING_JOB (int)-5

#define is_external(tokens, i) (tokens->commands[i].filename != NULL)

/*------------------ COLOR MACROS ---------------------------------*/
#define printf_if_std(stream, msg) (isatty(fileno(stream)))? fprintf(stream, msg) : (void)0


#define COLOR_RESET(stream)    printf_if_std(stream, "\033[0m")
#define STYLE_BOLD(stream)     printf_if_std(stream, "\033[1m")
#define STYLE_ITALIC(stream)   printf_if_std(stream, "\033[3m")
#define STYLE_UNDERLINE(stream)   printf_if_std(stream, "\033[4m")

#define COLOR_BLACK(stream)       printf_if_std(stream, "\033[30m")
#define COLOR_RED(stream)         printf_if_std(stream, "\033[31m")
#define COLOR_GREEN(stream)       printf_if_std(stream, "\033[32m")
#define COLOR_YELLOW(stream)      printf_if_std(stream, "\033[33m")
#define COLOR_BLUE(stream)        printf_if_std(stream, "\033[34m")
#define COLOR_MAGENTA(stream)     printf_if_std(stream, "\033[35m")
#define COLOR_CYAN(stream)        printf_if_std(stream, "\033[36m")
#define COLOR_WHITE(stream)       printf_if_std(stream, "\033[37m")
#define COLOR_GREY(stream)         printf_if_std(stream, "\033[90m")


#define COLOR_BRIGHT_RED(stream)     printf_if_std(stream, "\033[91m")
#define COLOR_BRIGHT_GREEN(stream)   printf_if_std(stream, "\033[92m")
#define COLOR_BRIGHT_YELLOW(stream)  printf_if_std(stream, "\033[93m")
#define COLOR_BRIGHT_BLUE(stream)    printf_if_std(stream, "\033[94m")
#define COLOR_BRIGHT_MAGENTA(stream) printf_if_std(stream, "\033[95m")
#define COLOR_BRIGHT_CYAN(stream)    printf_if_std(stream, "\033[96m")
#define COLOR_BRIGHT_WHITE(stream)   printf_if_std(stream, "\033[97m")

#endif // MACROS_H_