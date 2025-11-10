#include <minishell.h>
#include <log.h>

int main(int argc, char** argv) {
	init_minishell(argc, argv);
	
	size_t len;
	char *out_buff = env_get_var("HOME", &len);
	if (out_buff)OKAY("out_buff: %s", out_buff);

	char buff[INPUT_LINE_MAX];
	while(1) {
		read_line_input(buff, INPUT_LINE_MAX);
		if (!strcmp("exit", buff)) break;
	}
	return 0;
}
