#ifndef MACROS_H_
#define MACROS_H_

#define INPUT_LINE_MAX 1024



/*------------------ COLOR MACROS ---------------------------------*/
#define COLOR_RESET(stream)    fprintf(stream, "\033[0m")
#define STYLE_BOLD(stream)     fprintf(stream, "\033[1m")
#define STYLE_ITALIC(stream)   fprintf(stream, "\033[3m")

#define COLOR_RED(stream)      fprintf(stream, "\033[31m")
#define COLOR_GREEN(stream)    fprintf(stream, "\033[32m")
#define COLOR_YELLOW(stream)   fprintf(stream, "\033[33m")
#define COLOR_BLUE(stream)     fprintf(stream, "\033[34m")
#define COLOR_MAGENTA(stream)  fprintf(stream, "\033[35m")
#define COLOR_CYAN(stream)     fprintf(stream, "\033[36m")
#define COLOR_WHITE(stream)    fprintf(stream, "\033[37m")

#define COLOR_BRIGHT_RED(stream)     fprintf(stream, "\033[91m")
#define COLOR_BRIGHT_GREEN(stream)   fprintf(stream, "\033[92m")
#define COLOR_BRIGHT_YELLOW(stream)  fprintf(stream, "\033[93m")
#define COLOR_BRIGHT_BLUE(stream)    fprintf(stream, "\033[94m")

#endif // MACROS_H_