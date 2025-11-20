#ifndef PARSER_SCANNER_H_
#define PARSER_SCANNER_H_

typedef struct s_scanner {
    char *buf;
    size_t len;
    size_t i;
    int end;
    char curr;
} scanner;

void scanner_init(scanner *s, char *input);
void scanner_next(scanner *s);
char scanner_peek(scanner *s);
int scanner_eof(scanner *s);
void scanner_adv(scanner*s , int n);

#endif // PARSER_SCANNER_H_