#include <minishell.h>
#include <log.h>
#include <assert.h>

#define ARENA_SIZE 1024 * 1024

static void* g_ptr = NULL; 
static size_t g_size = 0; 
static size_t g_brk = 0;


void binit() {
    g_ptr = malloc(ARENA_SIZE);
    assert(g_ptr != NULL);
    g_size = ARENA_SIZE;
    OKAY("Arena size is: %zu", g_size);
}

void bexit() {
    free(g_ptr);
}

void* balloc(size_t sz) {
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

void breset() {
    g_brk = 0;
}