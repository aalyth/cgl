#ifndef __CGL_ALLOC_H
#define __CGL_ALLOC_H

#include <stddef.h>

struct allocator {
	void *(*malloc)(void *, size_t);
	void *(*realloc)(void *ctx, void *ptr, size_t);
	void (*free)(void *, void *);
	void *ctx;
};

#define allocator_malloc(allocator, size) (allocator)->malloc((allocator)->ctx, (size))
#define allocator_realloc(allocator, ptr, size) \
	(allocator)->realloc((allocator)->ctx, (ptr), (size))
#define allocator_free(allocator, ptr) (allocator)->free((allocator)->ctx, (ptr))

extern const struct allocator libc_allocator;

struct allocator new_arena_allocator(size_t);

#endif
