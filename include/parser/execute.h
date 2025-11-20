#ifndef PARSER_EXECUTE_H_
#define PARSER_EXECUTE_H_

int execute_pipeline(const char* cmdline, ast_node_pipeline_t* ppl);
int execute_command(const char* cmdline, ast_node_command_t* command);

#endif // PARSER_EXECUTE_H_