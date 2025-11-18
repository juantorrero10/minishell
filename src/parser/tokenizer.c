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

/**
 * @brief tokenize through state machine.
 */
token_arr tokenize(char* cmdline, _out_ int* st) {
    char* ptr;
    size_t len;
    size_t i = 0;
    size_t word_start = 0;
    size_t word_end = 0;
    token_t curr = {0}; (void) curr;
    char c = 0; (void)c;
    bool dquoted = 0;
    bool squoted = 0;
    bool bracketed_var = 0;
    int cmd_sub = 0;
    bool exit_s = 0;
    token_arr r = {NULL, 0, 0};
    typeof_token tt = TOK_WORD;

    ptr = cmdline;
    *st = 0;
    len = strlen(cmdline);
    if (!cmdline) {*st = 1; return r;}
    if (!len) {*st = 1; return r;}

init:
    curr = (token_t){.type = TOK_ERROR, .number = 0, .value = NULL };
    word_end = i;
    word_start = i;

word:
    switch (ptr[i])
    {
    case 0: exit_s = 1; goto finish_word;
    case '"' : if (!squoted)dquoted ^= 1; i++ ;word_end++; goto word;
    case '\'': if (!dquoted)squoted ^= 1; i++ ;word_end++; goto word;
    case '$': goto mixed_state_var_subst;
    case '{': goto mixed_state_group_var;
    case '}': 
        if (word_start < word_end) goto finish_word;
        if (bracketed_var) {
            error_parse(ERR_UNEXP, ptr + i); *st=1;i++;goto __exit;}
        else {
            curr.type = TOK_RBRACE; push_token(&r, &curr); i++;goto init;}

    case '(':
            if (word_start < word_end) goto finish_word;
            curr.type = TOK_LPAREN; 
            push_token(&r, &curr); 
            i++;goto init;

    case ')':
            if (word_start < word_end) goto finish_word;
            if (cmd_sub) {
                curr.type = TOK_CMD_ST_END; push_token(&r, &curr); i++;cmd_sub--;goto init;
            } curr.type = TOK_RPAREN; push_token(&r, &curr); i++;goto init;

    
    default:
        if (isokforwords(ptr[i]) || 
        (ptr[i] == ' ' && (dquoted || squoted))) {
            word_end++;
            i++;
            goto word;
        } else {
            if (ptr[i] != ' ') {error_parse(ERR_UNEXP, ptr + i); *st=1;goto __exit;}
            i++;
            goto finish_word;
        }
    }


mixed_state_group_var:
    //emit what word there was
    if (word_end > word_start) {
        curr = carve_word(ptr, word_start, word_end);
        push_token(&r, &curr);
    } i++; word_end = i; word_start = i;
    switch (ptr[i])
    {
    case '$':
        i++;
        bracketed_var = 1;
        word_start = i;word_end = i;
        goto var;
    
    default:
        if (isokforwords_sp(ptr[i])) {
            curr = (token_t){.type = TOK_LBRACE, .value = NULL, .number = 0};
            push_token(&r, &curr);
            goto init;
        }
        break;
    }

//maybe its a VAR or a SUBSTITUTION;
mixed_state_var_subst:
    //emit what word there was
    if (word_end > word_start) {
        curr = carve_word(ptr, word_start, word_end);
        push_token(&r, &curr);
    }
    i++; word_end = i; word_start = i;
    switch (ptr[i])
    {
    // bracketed var
    case '{':
        i++;
        bracketed_var = 1;
        word_start = i;word_end = i;
        goto var;
    // command substitution
    case '(':
        curr = (token_t){.type = TOK_CMD_ST_START, .value = NULL, .number = 0};
        push_token(&r, &curr);
        i++; cmd_sub++;
        goto init;
    default:
        if (isokforvars(ptr[i])) {word_start = i;word_end = i;goto var;}
        break;
    }

// advance until } or until non-alphanumerical (nor _) character an emit that word as a var
var:
    switch (ptr[i])
    {
    case 0: exit_s = 1; tt = TOK_VAR; goto finish_word;
    case '}':
        if (!bracketed_var) goto init;
        tt = TOK_VAR;
        bracketed_var = 0;
        i++;
        goto finish_word;
    
    default:
        if (isokforvars(ptr[i])) {
            i++;
            word_end++;
            goto var;
        } else {
            tt = TOK_VAR;
            goto finish_word;
        }
        break;
    }

finish_word:
    if (word_end > word_start) {
        curr = carve_word(ptr, word_start, word_end);
        curr.type = tt;
        tt = TOK_WORD;
        push_token(&r, &curr);
        goto init;
    }
    if (!exit_s) goto init;

__exit:
    
    return r;
}