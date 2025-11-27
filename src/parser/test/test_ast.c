#include <test/test_ast.h>
#include <errno.h>

/** @attention null terminated argv */
static size_t get_argv_sz(char** argv) {
    size_t ret_sz;
    char** ptr = argv;
    int i = 0;

    while (ptr[i]) {
        
        if (ptr[i+1])
        //                           + space
            ret_sz += strlen(ptr[i]) + 1;
        else
            ret_sz += strlen(ptr[i]);
        i++;
    }

    return ret_sz;
}

/** @attention null terminated argv */
static void get_argv(char** og, char* buff, size_t sz) {
    size_t argv_sz;
    char** ptr = og;
    int i = 0;

    if (sz < (argv_sz = get_argv_sz(og))) {
        TERROR("get_argv(): buff to small (%zu < %zu)", sz, argv_sz); 
        exit(1);
    }

    while (ptr[i])
    {
        strcat(buff, ptr[i]);
        if (ptr[i+1])
            strcat(buff, " ");
        i++;
    }
    
}

/**
 * default size: 2048
 * if buffer is smaller, specify.
 */
void test_ast_to_string(ast_t* ast, char* buff, size_t buff_sz, int ind, bool do_indent) {
    const int IND_SPACES    = 4;
    int indent_lvl          = ind; // indentation level for printing
    size_t rem_sz           = 0; // remaininig buffer size
    (void) rem_sz;
    ast_node_redir_t* redirs;
    size_t nredirs;

    memset(buff, 0, buff_sz);
    
    indent_lvl *= do_indent;
    if (!buff_sz) buff_sz = 2048;
    char scratch[buff_sz];
    char scratch2[buff_sz];
    char scratch3[buff_sz];
    rem_sz = buff_sz;


    switch (ast->type) {
        case AST_COMMAND:
            goto simple_command;
        case AST_LIST:
            goto list;
        case AST_PIPELINE:
            goto pipeline;
        case AST_GROUP:
            goto group;
        case AST_BG:
            goto bg;
        default: 
            return;

    }

bg:
    concat_str("bg:\n", buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    concat_str("children:\n", buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    test_ast_to_string(ast->node.bg.children, scratch2, buff_sz, indent_lvl+1, do_indent);
    strcat(buff, scratch2);
    memset(scratch2, 0, buff_sz);
    return;

list:
    sprintf(scratch2, "sep: '%s'\n", str_tok(ast->node.sep.sep_type, scratch3));
    concat_str(scratch2, buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    memset(scratch2, 0, buff_sz);
    memset(scratch3, 0, buff_sz);
    concat_str("left:\n", buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    test_ast_to_string(ast->node.sep.left, scratch2, buff_sz, indent_lvl+1, do_indent);
    strcat(buff, scratch2);
    concat_str("right:\n", buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    test_ast_to_string(ast->node.sep.right, scratch3, buff_sz, indent_lvl+1, do_indent);
    strcat(buff, scratch3);
    memset(scratch2, 0, buff_sz);
    memset(scratch3, 0, buff_sz);
    return;

pipeline:
    concat_str("pipeline:\n", buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    sprintf(scratch2, "ncommands: %zu\n", ast->node.ppl.ncommands);
    concat_str(scratch2, buff, scratch3, indent_lvl, IND_SPACES, buff_sz);
    memset(scratch2, 0, buff_sz);
    for (size_t i = 0; i < ast->node.ppl.ncommands; i++)
    {   
        sprintf(scratch3, "%zu/%zu:\n", i+1, ast->node.ppl.ncommands);
        concat_str(scratch3, buff, scratch, indent_lvl, IND_SPACES, buff_sz);
        test_ast_to_string((ast_t*)(ast->node.ppl.elements + i), scratch2, buff_sz, indent_lvl+1 ,do_indent);
        strcat(buff, scratch2);
        memset(scratch2, 0, buff_sz);
        memset(scratch3, 0, buff_sz);
    }
    return;

group:
    concat_str("group:\n", buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    if (ast->node.grp.group_type == GROUP_GENERIC) {
        concat_str("type: GENERIC\n", buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    } else {
        concat_str("type: SUBSHELL\n", buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    }
    concat_str("children:\n", buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    test_ast_to_string(ast->node.grp.children, scratch2, buff_sz, indent_lvl+1, do_indent);
    strcat(buff, scratch2);
    memset(scratch2, 0, buff_sz);
    sprintf(scratch2, "nredirs: %zu\n", ast->node.grp.nredirs);
    concat_str(scratch2, buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    memset(scratch2, 0, buff_sz);
    if (ast->node.grp.nredirs > 0) {
        redirs = ast->node.grp.redirs;
        nredirs = ast->node.grp.nredirs;
        goto redirs;
    } else return;
    return;
    

simple_command:
    concat_str("cmd:\n", buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    sprintf(scratch2, "argc: %d\n", ast->node.cmd.argc);
    concat_str(scratch2, buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    memset(scratch2, 0, buff_sz);
    get_argv(ast->node.cmd.argv, scratch, buff_sz);
    sprintf(scratch2, "argv: \'%s\'\n", scratch);
    concat_str(scratch2, buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    memset(scratch2, 0, buff_sz);
    sprintf(scratch2, "nredirs: %zu\n", ast->node.cmd.nredirs);
    concat_str(scratch2, buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    memset(scratch2, 0, buff_sz);
    if (ast->node.cmd.nredirs > 0) {
        redirs = ast->node.cmd.redirs;
        nredirs = ast->node.cmd.nredirs;
        goto redirs;
    } else return;
redirs:
    concat_str("redirs:\n", buff, scratch, indent_lvl, IND_SPACES, buff_sz);
    indent_lvl++;
    indent_lvl *= do_indent;
    for (size_t i = 0; i < nredirs; i++) {
        ast_node_redir_t *rd = &redirs[i];

        concat_indent(buff, scratch, indent_lvl, IND_SPACES, buff_sz);

        // print left-fd and operator
        memset(scratch2, 0, buff_sz);
        sprintf(
            scratch2,
            "fd=%d op='%s' ",
            rd->left_fd,
            str_tok(rd->op, scratch3)
        );
        memset(scratch3, 0, buff_sz);
        strcat(buff, scratch2);

        // print right side depending on redir kind
        memset(scratch2, 0, buff_sz);

        switch (rd->kind) {
            case REDIR_FILE:
                sprintf(scratch2, "file=\"%s\"\n", rd->target.filename);
                break;

            case REDIR_FD:
                sprintf(scratch2, "dup-to-fd=%d\n", rd->target.fd);
                break;

            case REDIR_CLOSE:
                sprintf(scratch2, "close\n");
                break;

            case REDIR_HEREDOC:
                sprintf(scratch2, "heredoc delimiter=\"%s\"\n", rd->target.string);
                break;

            case REDIR_HERESTR:
                sprintf(scratch2, "herestring=\"%s\"\n", rd->target.string);
                break;

            default:
                sprintf(scratch2, "UNKNOWN\n");
                break;
        }

        strcat(buff, scratch2);
    }

    indent_lvl--;
    return;

}


void test_start_test(char* file_input, char* file_output_summary) {
    char __buff[2048];
    char input[1024];
    char ast[2048];
    char line[1024];
    char *line_ptr;
    char *st_lbl;
    FILE *in;
    FILE * out;
    int test_id = 0;
    int out_flag = 0;
    int __ok = 0;
    int __failed = 0;
    int __aborted = 0;
    int __total = 0;

    in = fopen(file_input, "r");
    if (!in) {TERROR("Could not open file: %s, --> %s", file_input, strerror(errno)); exit(1); }
    TOKAY("Input file: %s", file_input);

    out = fopen(file_output_summary, "w+");
    if (!in) {TERROR("Could not open file: %s, --> %s", file_output_summary, strerror(errno)); exit(1); }
    TOKAY("Output file: %s", file_output_summary);

    while ((line_ptr = fgets(line, sizeof(line), in)) != NULL) {
        memcpy(input, line, sizeof(input));
        memset(ast, 0, sizeof(ast));
        fgets(line, sizeof(line), in);
        while(!strstr(line, "--END--")) {
            strcat(ast, line);
            fgets(line, sizeof(line), in);
        }
        ASSERT_AST(input, ast, out_flag);
        
        ST_LABEL(out_flag, st_lbl)
        fprintf(stdout, "%s [%d] %s: %s\n", test_entry, test_id++, input, st_lbl);
        fflush(stdout);
        if (out_flag == 0) {
            fprintf(out, "test nº%d failed. input: %s\nasts:\n", test_id-1, input);
            fprintf(out, "%s\nversus:\n%s\n", __buff, ast);
            fprintf(out, "------------------------------------------------------------\n");
        }
        fflush(stdout);
        memset(__buff, 0, sizeof(__buff));
        memset(line, 0, sizeof(line));
        memset(input, 0, sizeof(input));
        memset(ast, 0, sizeof(ast));
    }

    fprintf(out, "succeded: %d/%d\n", __ok, __total);
    printf("%ssucceded: %s%d/%d\n", COLOR_BRIGHT_GREEN, COLOR_RESET, __ok, __total);

    fprintf(out, "failed: %d/%d\n", __failed, __total);
    printf("%sfailed: %s%d/%d\n", COLOR_RED, COLOR_RESET, __failed, __total);

    fprintf(out, "aborted: %d/%d\n", __aborted, __total);
    printf("%saborted: %s%d/%d\n", COLOR_YELLOW, COLOR_RESET, __aborted, __total);


    fclose(in); fclose(out);
    return;

}
