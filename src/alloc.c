
#include <cgl/alloc.h>

#include <stdlib.h>

static void *__libc_malloc(void *, size_t);
static void *__libc_realloc(void *__unused, void *ptr, size_t);
static void __libc_free(void *, void *);

const struct allocator libc_allocator = (struct allocator){ .malloc = &__libc_malloc,
	                                                    .realloc = &__libc_realloc,
	                                                    .free = &__libc_free,
	                                                    .ctx = NULL };
static void *__libc_malloc(void *, size_t size)
{
	return malloc(size);
}

static void *__libc_realloc(void *, void *ptr, size_t size)
{
	return realloc(ptr, size);
}

static void __libc_free(void *, void *ptr)
{
	free(ptr);
}
