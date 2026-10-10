#include <minishell.h>
#include <log.h>
#include <assert.h>

#define ARENA_SIZE 1024 * 1024
// align allocations to 2 << ALIGN
#define ALIGN 3
#define ALIGN_TO(var, align) var += ((2 << (align)) - ((var) & ((2 << (align)) - 1))) & ((2 << align) - 1);

static void* g_ptr = NULL; 
static size_t g_size = 0; 
static size_t g_brk = 0;
static bool g_align = true;


void binit() {
    g_ptr = malloc(ARENA_SIZE);
    assert(g_ptr != NULL);
    g_size = ARENA_SIZE;
    OKAY("Arena size is: %zu", g_size);
}

void bexit() {
    free(g_ptr);
}

size_t bget_brk() {
    return g_brk;
}

void bset_brk(size_t b) {
    g_brk = b;
}

void benable_alignment(bool b) {
    g_align = b;
}

void* balloc(size_t sz) {
    if (g_align) {
        ALIGN_TO(sz, ALIGN);
        ALIGN_TO(g_brk, ALIGN);
    }
    void* ptr = g_ptr + g_brk;
    g_brk += sz;
    if (g_brk >= g_size) {
        fprintf(stderr, "Out of memory, command was too large.\n");
        assert(g_brk < g_size);
    }
    return ptr;
}

void* bcalloc(size_t nmemb, size_t sz) {
    size_t alloc = nmemb * sz;
    void* p = balloc(alloc);
    memset(p, 0, alloc);
    return p;
}

void* brealloc(void* p, size_t old, size_t new) {
    void* new_p = balloc(new);
    memcpy(new_p, p, old);
    return new_p;
}

char* bstrndup(const char* str, size_t len) {
    if (!str) {
        return NULL;
    }
    char* s = balloc(len + 1);
    memcpy(s, str, len);
    s[len] = '\0';
    return s;
}

char* bstrdup(const char* str) {
    return bstrndup(str, strlen(str));
}


void bfree(size_t s) {
    assert(g_brk >= s);
    g_brk -= s;
}

void breset() {
    g_brk = 0;
}