#include <mshparser.h>

/**
 * .c file containing a main() for testing only the parser.
 * this file will not be compiled anymore when developing
 * of this submodule ends.
 */

int main(void)
{
    char buff[1024];
    ast_t* a; (void)a;
    size_t sz;

    printf("parser test demo\n");
    while (1) {
        printf("parserdemo> ");
        fgets(buff, 1024, stdin);
        sz = strlen(buff);
        if (buff[sz - 1] == '\n') buff[sz-- - 1] = '\0';
        if (!strcmp("exit", buff)) exit(0);
        a = parse_command(buff);
    }
    return (0);
}