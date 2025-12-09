#ifndef COMMAND_EXECUTILS_H_
#define COMMAND_EXECUTILS_H_

int get_internal_idx(char* argv0);
size_t get_npids(ast_t* tree, bool simple);
int handle_redirection(ast_node_redir_t* rd);


#endif // COMMAND_EXECUTILS_H_