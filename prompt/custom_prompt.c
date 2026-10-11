#include "prompt.h"

// the regular bash prompt
void prompt_push() {
    PROMPT_STYLE_BOLD();
    PROMPT_COLOR_BRIGHT_GREEN();
    prompt_append_username();
    prompt_append_str("@");
    prompt_append_hostname();
    PROMPT_COLOR_RESET();
    prompt_append_str(":");
    PROMPT_STYLE_BOLD();
    PROMPT_COLOR_BLUE();
    prompt_append_cwd(1);
    PROMPT_COLOR_RESET();
    prompt_append_str("$ ");
}
