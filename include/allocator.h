#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_


// Bump allocator
void binit();
void bexit();
void benable_alignment(bool b);
void bset_brk(size_t b);
size_t bget_brk();
void* balloc(size_t sz);
void* bcalloc(size_t nmemb, size_t sz);
void* brealloc(void* p, size_t old, size_t new);
char* bstrndup(const char* str, size_t len);
char* bstrdup(const char* str);
void bfree(size_t s);
void breset();

#endif // ALLOCATOR_H_