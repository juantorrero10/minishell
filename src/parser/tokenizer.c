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

    ret.value = strndup(cmdline + ws, we - ws);
    ret.str_idx = ws;
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

//Mini st for redirections;
static typeof_token scan_redir_type(const char* s, _out_ int* skip_chars) {
    const int __restrict_view = 3;
    typeof_token t = TOK_ERROR;
    char* ptr;
    char* allowed = "><&";
    char c;
    size_t sz;

    //trim space and numbers.
    ptr = strndup(s, __restrict_view);
    *skip_chars = 0;
    sz = strlen(ptr);
    for (int i = ((int)sz)-1; i >= 0; i--)
    {
        c = ptr[i];
        if (!strchr(allowed, c)) {
            ptr[i] = '\0';
        } else break;
    }
    if (!strcmp(ptr, ">")) {
        t = TOK_REDIR_OUT; *skip_chars = 1;
    } else if (!strcmp("<", ptr)) {
        t = TOK_REDIR_IN; *skip_chars = 1;
    } else if (!strcmp(">>", ptr)) {
        t = TOK_REDIR_OUT_APPEND; *skip_chars = 2;
    } else if (!strcmp("<<", ptr)) {
        t = TOK_REDIR_HEREDOC; *skip_chars = 2;
    } else if (!strcmp(">&", ptr) || !strcmp("&>", ptr)) {
        t = TOK_REDIR_DUP_OUT; *skip_chars = 2;
    } else if (!strcmp("<&", ptr) || !strcmp("&<", ptr)) {
        t = TOK_REDIR_DUP_IN; *skip_chars = 2;
    } else if (!strcmp("<<<", ptr)) {t = TOK_REDIR_HERESTR; *skip_chars = 3;}

    free (ptr);
    return t;
}

static size_t isnum(const char* s) {
    char* buf;
    const int __restrict_view = 3; //max fd: 999
    size_t sz, sz2; (void)sz2;

    buf = strndup(s, __restrict_view);
    sz = strlen(buf); sz2 = sz;
    if (!isdigit(buf[0])) return 0;
    //trim chars
    for (int i = ((int)sz)-1; i >= 0; i--){
        if (!isdigit(buf[i])) {buf[i] = '\0';sz2--;}
    }
    return sz2;
}

static void pile_push(char* pile, int* pile_top, char c) {
    int pt = 0;

    pt = *pile_top;
    pile[pt] = c;
    *pile_top = ++pt;
}

static char pile_pop(char* pile, int* pile_top) {
    int pt = 0;

    pt = *pile_top;
    *pile_top = --pt;
    return pile[pt];
}

/**
 * @brief tokenizer state machine:
 */
token_arr tokenize(char *cmdline, _out_ int *st)
{
    scanner s;
    token_arr r = {NULL, 0, 0};
    token_t curr = {0};
    int n=0; char* temp;

    // a pile/stack for the sole purpose of diferenciating
    // between closing cmd subst -> $( ...    > ) <------
    // and closing subshells     -> (  ...    > ) <------
    char pile[100];
    int pile_top = 0;
    char pc;

    size_t word_start = 0;
    size_t word_end = 0;
    size_t word_len;
    typeof_token tt = TOK_WORD;

    int dquoted = 0;
    int squoted = 0;
    int bracketed_var = 0;
    int cmd_sub = 0;

    int exit_s = 0;

    int dq_weight = 0;

    bool dollar = false;
    bool after_redir = false;
    *st = 0;

    if (!cmdline) { *st = 1; return r; }
    scanner_init(&s, cmdline);
    memset(pile, 0, 100);

init:
    after_redir = 0;
    curr = (token_t){ .type = TOK_ERROR, .number = 0, .value = NULL };
    curr.str_idx = s.i;
    word_start = s.i;
    word_end   = s.i;
    tt = TOK_WORD;

word:
    // ' everything is allowed and its not expanded
    if (squoted) {
        if (s.curr == '\'' && !cmd_sub) {
            squoted = 0;
            scanner_next(&s);
            word_end = s.i;
            goto word;
        }
        scanner_next(&s);
        word_end = s.i;
        goto word;
    }

    if (scanner_eof(&s)) {
        exit_s = 1;
        goto finish_word;
    }

    // detect redirections
    
    if ((curr.type = scan_redir_type(s.buf + s.i, &curr.number)) != TOK_ERROR) {
        scanner_adv(&s, curr.number); curr.number = 0;
        curr.str_idx = s.i;
        push_token(&r, &curr);
        after_redir = 1;
        curr = (token_t){0};word_start = s.i;word_end = s.i;tt = TOK_WORD;
    }

    switch (s.curr)
    {
    case '"':
        if (word_end > word_start) goto finish_word;
        if (!squoted) {dquoted ^= 1;
        curr = (token_t){0};
        curr.type = (dquoted)? TOK_DQ_START : TOK_DQ_END;
        curr.str_idx = s.i;
        push_token(&r, &curr);
        scanner_next(&s);
        goto init;
    }
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
        if (dquoted) {
            scanner_next(&s); word_end = s.i;
            dollar = 0;
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
            curr.str_idx = s.i;
            push_token(&r, &curr);
            scanner_next(&s);
            goto init;
        }

    case '}':
        if (dquoted) {
            scanner_next(&s); word_end = s.i;
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
        curr.str_idx = s.i;
        push_token(&r, &curr);
        scanner_next(&s);
        goto init;
    // Subshells are ignored in "" and ''
    case '(':
        
        // If theres a word previously
        if (!dquoted || dollar) {
            if (word_end > word_start) {
                if (dollar) word_end--;
                goto finish_word;
            }
        }
        if (dquoted && dollar)dq_weight++;

        curr = (token_t){0};
        pc = 'S';
        if (dollar) {curr.type = TOK_CMD_ST_START; dquoted=0; cmd_sub++;pc='C';}
        else curr.type = TOK_LPAREN;
        pile_push(pile, &pile_top, pc);
        dollar = 0;
        curr.str_idx = s.i;
        push_token(&r, &curr);
        scanner_next(&s);
        goto init;

    case ')':
        if (dquoted) {
            if (cmd_sub) {dq_weight--; cmd_sub--;}
            scanner_next(&s); word_end = s.i;
            bracketed_var = 0;
            goto word;
        }

        if (word_end > word_start)
            goto finish_word;
        pc = pile_pop(pile, &pile_top);
        if (pc == 'C') {
            curr = (token_t){0};
            curr.type = TOK_CMD_ST_END;
            curr.str_idx = s.i;
            push_token(&r, &curr);
            cmd_sub--;
            scanner_next(&s);
            if(dq_weight) dquoted = 1;
            goto init;
        }
        curr = (token_t){0};
        curr.type = TOK_RPAREN;
        curr.str_idx = s.i;
        push_token(&r, &curr);
        scanner_next(&s);
        goto init;

    case ' ':
        if (after_redir) {
            while(s.curr == ' ') scanner_next(&s);
            goto word;
        }
        if (dquoted || squoted) {
            /* space inside quotes */
            scanner_next(&s);
            word_end = s.i;
            goto word;
        }
        /* unquoted space ends word */
        scanner_next(&s);
        goto finish_word;

    case '&':
        if (dquoted) {
            scanner_next(&s);
            word_end = s.i;
            goto word;
        }
        if (word_end > word_start)
            goto finish_word;
        
        curr = (token_t){0};
        scanner_next(&s);
        if (scanner_eof(&s)) {
            curr.type = TOK_AMP;
            curr.str_idx = s.i;
            push_token(&r, &curr);
            goto init;
        } else if (s.curr == '&') {
            curr.type = TOK_AND_IF;
            curr.str_idx = s.i;
            push_token(&r, &curr);
            scanner_next(&s);
            goto init;
        } else {error_parse(ERR_UNEXP, s.buf + s.i);*st=1;goto __exit;}
        
    case '|':
        if (dquoted) {
            scanner_next(&s);
            word_end = s.i;
            goto word;
        }
        if (word_end > word_start)
            goto finish_word;
        
        scanner_next(&s);
        curr = (token_t){0};
        if (s.curr == '|') {
            curr.type = TOK_OR_IF;
            curr.str_idx = s.i;
            push_token(&r, &curr);
            scanner_next(&s);
            goto init;
        }
        curr.type = TOK_PIPE;
        curr.str_idx = s.i;
        push_token(&r, &curr);
        goto init;

    case ';':
        if (dquoted) {
            scanner_next(&s);
            word_end = s.i;
            goto word;
        }
        if (word_end > word_start)
            goto finish_word;
        curr = (token_t){0};
        curr.type = TOK_SEMI;
        curr.str_idx = s.i;
        push_token(&r, &curr);
        scanner_next(&s);
        goto init;

    default:
        if (after_redir && s.curr == '-') {
            curr = (token_t){0};
            curr.type = TOK_REDIR_RHS_CLOSE;
            curr.str_idx = s.i;
            push_token(&r, &curr);
            scanner_next(&s);
            goto init;
        }
        if ((n = isnum(s.buf + s.i)) > 0) {
            temp = s.buf + s.i;
            scanner_adv(&s, n);
            word_end = s.i;
            if (after_redir) {
                
                temp = strndup(temp, (int)n);
                curr.number = atoi(temp);
                curr.type = TOK_REDIR_RHS_FD;
                curr.str_idx = s.i;
                push_token(&r, &curr);
                free(temp);
                goto init;
            }
            if (scan_redir_type(s.buf + s.i, &curr.number) != TOK_ERROR) {
                curr = (token_t){0};
                temp = strndup(temp, (int)n);
                curr.number = atoi(temp);
                curr.type = TOK_REDIR_LHS_FD;
                curr.str_idx = s.i;
                push_token(&r, &curr);
                free(temp);
                goto init;
            }
            goto word;
        }
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
        curr.str_idx = s.i;
        if(word_len)push_token(&r, &curr);
        else if (curr.value) free(curr.value);
        goto init;
    }

    if (!exit_s)
        goto init;

__exit:
    curr = (token_t){0};
    curr.type = TOK_EOL;
    curr.str_idx = s.i;
    push_token(&r, &curr);
    return r;
}
