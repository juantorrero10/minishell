#include <minishell.h>

int main(int argc, char** argv) {
	bool print_prompt = true;
	init_minishell(argc, argv);
	if (argc > 1 && !strcmp(argv[1], "-s")) print_prompt = false;

	char buff[INPUT_LINE_MAX];
	while(1) {
		read_line_input(buff, INPUT_LINE_MAX, print_prompt);
	}
	return EXIT_SUCCESS;
}
