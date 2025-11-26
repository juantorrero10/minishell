#include <mshparser.h>

void error_parse(err_t e, char* s) {
    const size_t MAX_ERROR_DISPLAY = 15;
    char* etc = "     ";

    switch (e)
    {
    case ERR_UNTERM_BRACKET:
        LPERR("unterminated bracket -> %s%s{%s ... %s%s}%s", STYLE_BOLD, COLOR_WHITE, COLOR_RESET, COLOR_GREY, STYLE_ITALIC, COLOR_RESET);
        break;
    
    case ERR_UNTERM_PAREN:
        LPERR("unterminated parenthesis -> %s%s(%s ... %s%s)%s", STYLE_BOLD, COLOR_WHITE, COLOR_RESET, COLOR_GREY, STYLE_ITALIC, COLOR_RESET);
        break;

    case ERR_UNTERM_DQUOTE:
        LPERR("unterminated double-quote -> %s%s\"%s ... %s%s\"%s", STYLE_BOLD, COLOR_WHITE, COLOR_RESET, COLOR_GREY, STYLE_ITALIC, COLOR_RESET);
        break;

    case ERR_UNTERM_SQUOTE:
        LPERR("unterminated single-quote -> %s%s\'%s ... %s%s\'%s", STYLE_BOLD, COLOR_WHITE, COLOR_RESET, COLOR_GREY, STYLE_ITALIC, COLOR_RESET);
        break;

    case ERR_MISMATCHED_BRACKET:
        LPERR("incorrect termination -> %s%s(%s ... %s%s}%s or %s%s{%s ... %s%s)%s", STYLE_BOLD, COLOR_WHITE, COLOR_RESET, COLOR_RED, 
            STYLE_ITALIC, COLOR_RESET, STYLE_BOLD, COLOR_WHITE, COLOR_RESET, COLOR_RED, 
            STYLE_ITALIC, COLOR_RESET);
        break;

    case ERR_INVALID_REDIR_SYNTAX:
        if (strlen(s) > MAX_ERROR_DISPLAY) {
            etc = " ... ";
        }
        LPERR("incorrect syntax of redirection: %s%s%s%c%s%.14s%s", STYLE_BOLD, STYLE_UNDERLINE, COLOR_RED, *s, COLOR_RESET, s+1, etc);
        break;
    case ERR_UNEXP:
        if (strlen(s) > MAX_ERROR_DISPLAY) {
                etc = " ... ";
        }
        LPERR("unexpected token: %s%s%s%c%s%.14s%s", STYLE_BOLD, STYLE_UNDERLINE, COLOR_RED, *s, COLOR_RESET, s+1, etc);
        break;
    default:
        LPERR("%s", s);
        break;
    }
}

void error_clarify(char* s) {
    LPCLR("%s", s);
}