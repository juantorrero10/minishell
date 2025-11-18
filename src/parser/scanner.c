#include <mshparser.h>

void scanner_init(scanner *s, char *input) {
    s->buf = input;
    s->len = strlen(input);
    s->i = 0;
    s->end = (s->len == 0);
    s->curr = (s->end ? 0 : s->buf[0]);
}

void scanner_next(scanner *s) {
    if (s->end) return;

    s->i++;
    if (s->i >= s->len) {
        s->end = 1;
        s->curr = 0;
    } else {
        s->curr = s->buf[s->i];
    }
}

char scanner_peek(scanner *s) {
    if (s->end) return 0;
    return s->curr;
}

int scanner_eof(scanner *s) {
    return s->end;
}