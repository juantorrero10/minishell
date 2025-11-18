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
        if (t.value)a->ptr[0].value = strdup(t.value);
        if (t.value)free(t.value);
        tok->value = NULL;
        return;
    } 
    if ( a->allocated < a->occupied+1) {
        a->allocated <<= 1;
        a->ptr = realloc(a->ptr, a->allocated * sizeof(token_t));
    }
    
    a->ptr[a->occupied] = t;
    if (t.value)a->ptr[a->occupied++].value = strdup(t.value);
    if (t.value)free(t.value);
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
    const char *src = cmdline + ws;

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
    printf("new word: %s\n", ret.value);
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
token_arr tokenize(char* cmdline, _out_ size_t* arr_sz) {
    char* ptr;
    size_t len;
    size_t i = 0;
    size_t word_start = 0;
    size_t word_end = 0;
    token_t curr = {0}; (void) curr;
    char c = 0; (void)c;
    bool dquoted = 0;
    bool squoted = 0;
    token_arr r = {NULL, 0, 0};

    ptr = cmdline;
    *arr_sz = 0;
    len = strlen(cmdline);
    if (!cmdline) {arr_sz = 0; return r;}
    if (!len) {arr_sz = 0; return r;}

init:
    curr = (token_t){.type = TOK_ERROR, .number = 0, .value = NULL };
    word_end = i;
    word_start = i;

word:
    switch (ptr[i])
    {
    case 0: goto finish_word;
    case '"' : if (!squoted)dquoted ^= 1; i++ ;word_end++; goto word;
    case '\'': if (!dquoted)squoted ^= 1; i++ ;word_end++; goto word;
    
    default:
        if (isokforwords(ptr[i]) || 
        (ptr[i] == ' ' && (dquoted || squoted))) {
            word_end++;
            i++;
            goto word;
        } else {
            if (ptr[i] != ' ') {error_parse(ERR_UNEXP, ptr + i); goto __exit;}
            i++;
            goto finish_word;
        }
    }

finish_word:
    if (word_end > word_start) {
        curr = carve_word(ptr, word_start, word_end);
        push_token(&r, &curr);
        goto init;
    }
    goto __exit;

__exit:
    
    return r;
}