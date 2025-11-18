#include <mshparser.h>


/**
 * @note exponential allocation (20*2^n)
 */
static void push_token(token_arr* a, token_t* tok) {
    const size_t INITIAL_ALLOC_SIZE = 20;
    token_t t = *tok;

    if (!a->allocated && !a->ptr) {
        a->ptr = malloc(sizeof(token_t)*INITIAL_ALLOC_SIZE);
        *a->ptr = t;
        a->allocated = INITIAL_ALLOC_SIZE; a->occupied = 1;
        if (t.value)a->ptr[0].value = t.value;
        return;
    } 
    if ( a->allocated < a->occupied+1) {
        a->allocated <<= 1;
        a->ptr = realloc(a->ptr, a->allocated * sizeof(token_t));
    }
    
    a->ptr[a->occupied] = t;
    if (t.value)a->ptr[a->occupied].value = t.value;
    a->occupied++;
    tok->value = NULL;
}
/*
static int chrcount(char *s, char c, int __restrict_view) {
    int ret = 0, i = 0;

    if (i == -1) i = __INT32_MAX__;
    while(*s && (i < __restrict_view)) {if (*s++ == c) ret++; i++;}
    return ret;
}
*/
/** 
 * @brief Copy a word into a token given a view of an array
 * single or double quotes are dissmised.
 * */
static token_t carve_word(char *cmdline, size_t ws, size_t we) {
    token_t ret = { .type = TOK_WORD, .number = 0, .value = NULL };

    bool dquoted = 0;
    bool squoted = 0;

    size_t len = we - ws;
    char *src = cmdline + ws;

    // worst case: same length, no quotes removed
    char *buf = malloc(len + 1);
    size_t w = 0;

    for (size_t i = 0; i < len; i++) {
        char c = src[i];


        if (c == '"' && !squoted) {
            dquoted ^= 1;
            continue;               // DO NOT copy quote
        }
        if (c == '\'' && !dquoted) {
            squoted ^= 1;
            continue;               // DO NOT copy quote
        }

        // normal char, always copy
        buf[w++] = c;
    }

    buf[w] = '\0';
    ret.value = buf;
    return ret;
}


void free_token_arr(token_arr* a) {

    for (size_t i = 0; i < a->occupied;i++){
        if (a->ptr[i].value)free(a->ptr[i].value);
    }
    free(a->ptr);
    a->allocated = 0;
    a->occupied = 0;
    a->ptr = NULL;
}

token_arr tokenize(char *cmdline, _out_ int *st)
{
    scanner s;
    token_arr r = {NULL, 0, 0};
    token_t curr = {0};

    size_t word_start = 0;
    size_t word_end = 0;
    size_t word_len;
    typeof_token tt = TOK_WORD;

    int dquoted = 0;
    int squoted = 0;
    int bracketed_var = 0;
    int cmd_sub = 0;

    int exit_s = 0;

    bool dollar = false;
    *st = 0;
    if (!cmdline) { *st = 1; return r; }
    scanner_init(&s, cmdline);

init:
    curr = (token_t){ .type = TOK_ERROR, .number = 0, .value = NULL };
    word_start = s.i;
    word_end   = s.i;
    tt = TOK_WORD;

word:
    // ' everything is allowed but its litteral.
    if (squoted) {
    if (s.curr == '\'') {
        squoted = 0;
        scanner_next(&s);
        word_end = s.i;
        goto word;
    }
    scanner_next(&s);
    word_end = s.i;
    goto word;
}

    // " everything is allowed but it expands
    if (dquoted) {
        if (s.curr == '"') {
            dquoted = 0;
            scanner_next(&s);
            word_end = s.i;
            goto word;
        }
    }

    if (scanner_eof(&s)) {
        exit_s = 1;
        goto finish_word;
    }

    switch (s.curr)
    {
    case '"':
        if (!squoted) dquoted ^= 1;
        scanner_next(&s);
        word_end = s.i;
        goto word;

    case '\'':
        if (!dquoted) squoted ^= 1;
        scanner_next(&s);
        word_end = s.i;
        goto word;

    case '$':
        dollar = 1; scanner_next(&s); word_end = s.i; goto word;

    // If quoted add to the word
    case '{':
        if (dquoted || squoted) {
            scanner_next(&s);
            word_end = s.i;
            goto word;
        }

        if (dollar == 1) {
            scanner_next(&s); word_end = s.i;
            dollar = 0;
            bracketed_var = 1;
            goto word;
        } else {
            // If theres a word previously
            if (word_end > word_start)
                goto finish_word;
            curr = (token_t){0};
            curr.type = TOK_LBRACE;
            push_token(&r, &curr);
            scanner_next(&s);
            goto init;
        }

    case '}':
        if (dquoted || squoted) {
            scanner_next(&s);
            word_end = s.i;
            goto word;
        }

        if (bracketed_var) {
            scanner_next(&s); word_end = s.i;
            bracketed_var = 0;
            goto word;
        }
        if (word_end > word_start)
            goto finish_word;
        
        curr = (token_t){0};
        curr.type = TOK_RBRACE;
        push_token(&r, &curr);
        scanner_next(&s);
        goto init;
    // Subshells are ignored in "" and ''
    case '(':
        if ((dquoted && !dollar) || squoted) {
            scanner_next(&s);
            word_end = s.i;
            dollar = 0;
            goto word;
        } 
        // If theres a word previously
        if (word_end > word_start) {
            if (dollar) word_end--;
            goto finish_word;
        }
        curr = (token_t){0};
        if (dollar) {curr.type = TOK_CMD_ST_START; cmd_sub++;}
        else curr.type = TOK_LPAREN;
        dollar = 0;
        push_token(&r, &curr);
        scanner_next(&s);
        goto init;

    case ')':
        if (word_end > word_start)
            goto finish_word;

        if (cmd_sub) {
            curr = (token_t){0};
            curr.type = TOK_CMD_ST_END;
            push_token(&r, &curr);
            cmd_sub--;
            scanner_next(&s);
            goto init;
        }
        curr = (token_t){0};
        curr.type = TOK_RPAREN;
        push_token(&r, &curr);
        scanner_next(&s);
        goto init;

    case ' ':
        if (dquoted || squoted) {
            /* space inside quotes */
            scanner_next(&s);
            word_end = s.i;
            goto word;
        }
        /* unquoted space ends word */
        scanner_next(&s);
        goto finish_word;

    default:
        if (isokforwords(s.curr)) {
            scanner_next(&s);
            word_end = s.i;
            goto word;
        } else if (dquoted) {
            scanner_next(&s);
            word_end = s.i;
            goto word;
        }

        error_parse(ERR_UNEXP, cmdline + s.i);
        *st = 1;
        goto __exit;
    }

/* ------ finishing words ------ */

finish_word:
    if (word_end > word_start) {
        curr = carve_word(cmdline, word_start, word_end);
        curr.type = tt;
        word_len = strlen(curr.value);
        if(word_len)push_token(&r, &curr);
        else if (curr.value) free(curr.value);
        goto init;
    }

    if (!exit_s)
        goto init;

__exit:
    return r;
}
