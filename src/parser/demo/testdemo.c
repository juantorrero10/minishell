#include <mshparser.h>
#include <test/test_ast.h>

/**
 * .c file containing a main() for testing only the parser.
 * this file will not be compiled anymore when developing
 * of this submodule ends.
 */

int main(int argc, char** argv)
{
    char out[] = "summary.txt";
    ast_t* a; (void)a;
    (void)argv;
    if (argc < 1) {return 1;}
    

    printf("parser test demo\n");
    /*
    while (1) {
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
        */
    test_start_test("cherrypick.txt", out);
    return (0);
}