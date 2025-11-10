#include <minishell.h>
#include <log.h>

int main() {
	char buff[INPUT_LINE_MAX];
	while(1) {
		get_line_input(buff, INPUT_LINE_MAX);
		INFO("%s", buff);
		if (strcmp("exit", buff)) break;
	}
	return 0;
}
