#include <mshparser.h>

void error_parse(err_t e, char* s) {
    switch (e)
    {
    case ERR_UNTERM_BRACKET:
        LPERR("unterminated bracket >}<");
        break;
    
    case ERR_UNTERM_PAREN:
        LPERR("unterminated parenthesis >)<");
        break;

    case ERR_UNTERM_DQUOTE:
        LPERR("unterminated double-quote >\"< ");
        break;

    case ERR_UNTERM_SQUOTE:
        LPERR("unterminated single-quote >'<");
        break;

    case ERR_MISMATCHED_BRACKET:
        LPERR("incorrect order of characters (,),{,}");
        break;
    case ERR_UNEXP:
        LPERR("unexpected token: %s%s%c%s%s", STYLE_BOLD, COLOR_RED, *s, COLOR_RESET, s+1);
        break;
    default:
        LPERR("%s", s);
        break;
    }
}