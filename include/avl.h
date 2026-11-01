#ifndef __CGL_AVL_H
#define __CGL_AVL_H

#include "alloc.h"

#include <stdbool.h>
#include <stddef.h>

struct avl_options {
	const struct allocator *allocator;
};

struct __avl_node;

struct __avl {
	struct __avl_node *root;
	size_t size;
	size_t key_size;
	size_t value_size;
	size_t value_offset;
	int (*cmp)(const void *, const void *);
	const struct allocator *allocator;
};

void __avl_init(struct __avl *, size_t key_size, size_t key_align, size_t value_size,
                size_t value_align, int (*cmp)(const void *, const void *), struct avl_options);
int __avl_put(struct __avl *, const void *key, const void *value);
void *__avl_get(struct __avl *, const void *key);
bool __avl_remove(struct __avl *, const void *key);
void __avl_free(struct __avl *);

/* A union so the type-only pointers cost nothing: they are never read or written. */
#define avl(K, V)                  \
	union {                    \
		struct __avl core; \
		K *__k;            \
		V *__v;            \
	}

/* cmp receives pointers to two keys. */
#define avl_init(t, cmp, ...)                                                                     \
	__avl_init(&(t)->core, sizeof(*(t)->__k), _Alignof(typeof(*(t)->__k)), sizeof(*(t)->__v), \
	           _Alignof(typeof(*(t)->__v)), cmp, (struct avl_options){ __VA_ARGS__ })

/* Replaces the value if the key exists. Returns 0, or -1 if allocation failed. */
#define avl_put(t, k, v)                                         \
	({                                                       \
		typeof(*(t)->__k) __cgl_key = (k);               \
		typeof(*(t)->__v) __cgl_value = (v);             \
		__avl_put(&(t)->core, &__cgl_key, &__cgl_value); \
	})

/* A pointer to the value, or NULL. Valid until that key is removed. */
#define avl_get(t, k)                                                \
	({                                                           \
		typeof(*(t)->__k) __cgl_key = (k);                   \
		(typeof((t)->__v))__avl_get(&(t)->core, &__cgl_key); \
	})

#define avl_contains(t, k) (avl_get(t, k) != NULL)

/* Returns whether the key was there. */
#define avl_remove(t, k)                              \
	({                                            \
		typeof(*(t)->__k) __cgl_key = (k);    \
		__avl_remove(&(t)->core, &__cgl_key); \
	})

#define avl_size(t) ((t)->core.size)
#define avl_free(t) __avl_free(&(t)->core)

#endif
