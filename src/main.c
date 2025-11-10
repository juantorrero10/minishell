#include <minishell.h>

int main(int argc, char** argv) {
	init_minishell(argc, argv);
	

	char buff[INPUT_LINE_MAX];
	while(1) {
		read_line_input(buff, INPUT_LINE_MAX);
	}
	return EXIT_SUCCESS;
}
