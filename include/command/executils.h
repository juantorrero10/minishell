#ifndef COMMAND_EXECUTILS_H_
#define COMMAND_EXECUTILS_H_

int eu_get_internal_idx(char* argv0);
size_t eu_lookahead_get_npids(ast_t* tree, bool simple);
int eu_handle_redirection(ast_node_redir_t* rd);


#endif // COMMAND_EXECUTILS_H_