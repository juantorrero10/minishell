#include <mshparser.h>
#include <test/test_ast.h>

/**
 * .c file containing a main() for testing only the parser.
 * this file will not be compiled anymore when developing
 * of this submodule ends.
 */

int main(int argc, char** argv)
{
    bool test = 1;
    char out[] = "tests/summary.txt";
    (void)out;
    ast_t* a; (void)a;
    (void)argv; (void) argc;
    char buff[1024]; (void)buff;
    char buff2[2048]; (void)buff2;
    size_t sz = 0;

    
    printf("parser test demo\n");
    while (!test) {
        printf("parserdemo> ");
        fgets(buff, 1024, stdin);
        sz = strlen(buff);
        if (buff[sz - 1] == '\n') buff[sz-- - 1] = '\0';
        if (!strcmp("exit", buff)) exit(0);
        a = parse_string(buff);
        if (a){
            test_ast_to_string(a, buff2, 2048, 1, 1);
            TOKAY("Generated AST: \n%s", buff2);
            memset(buff2, 0, 2048);
            ast_free(a);
            free(a);
        }
    }
    
    if (test) test_start_test((argc < 2)? "tests/cherrypick.txt" : argv[1], out);
    return (0);
}