#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_

void binit();
void bexit();
void* balloc(size_t sz);
void* bcalloc(size_t nmemb, size_t sz);
void breset();

#endif // ALLOCATOR_H_