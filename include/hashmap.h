#ifndef __CGL_HASHMAP_H
#define __CGL_HASHMAP_H

#include "alloc.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct hashmap_options {
	size_t capacity;
	const struct allocator *allocator;
};

struct __hashmap {
	uint64_t *hashes;
	unsigned char *entries;
	size_t size;
	size_t cap;
	unsigned shift;
	size_t key_size;
	size_t value_size;
	size_t value_offset;
	size_t entry_size;
	int (*cmp)(const void *, const void *);
	size_t (*hash)(const void *);
	const struct allocator *allocator;
};

int __hashmap_init(struct __hashmap *, size_t key_size, size_t key_align, size_t value_size,
                   size_t value_align, int (*cmp)(const void *, const void *),
                   size_t (*hash)(const void *), struct hashmap_options);
int __hashmap_put(struct __hashmap *, const void *key, const void *value);
void *__hashmap_get(struct __hashmap *, const void *key);
bool __hashmap_remove(struct __hashmap *, const void *key);
void __hashmap_free(struct __hashmap *);

/* A union so the type-only pointers cost nothing: they are never read or written. */
#define hashmap(K, V)                  \
	union {                        \
		struct __hashmap core; \
		K *__k;                \
		V *__v;                \
	}

/*
 * cmp and hash receive pointers to keys, so a char * key arrives as char **.
 * cmp returns 0 for equal keys. Returns 0, or -1 if reserving capacity failed,
 * leaving the map empty but usable.
 */
#define hashmap_init(m, cmp, hash, ...)                                            \
	__hashmap_init(&(m)->core, sizeof(*(m)->__k), _Alignof(typeof(*(m)->__k)), \
	               sizeof(*(m)->__v), _Alignof(typeof(*(m)->__v)), cmp, hash,  \
	               (struct hashmap_options){ __VA_ARGS__ })

/* Replaces the value if the key exists. Returns 0, or -1 if allocation failed. */
#define hashmap_put(m, k, v)                                         \
	({                                                           \
		typeof(*(m)->__k) __cgl_key = (k);                   \
		typeof(*(m)->__v) __cgl_value = (v);                 \
		__hashmap_put(&(m)->core, &__cgl_key, &__cgl_value); \
	})

/* A pointer to the value, or NULL. Any put or remove invalidates it. */
#define hashmap_get(m, k)                                                \
	({                                                               \
		typeof(*(m)->__k) __cgl_key = (k);                       \
		(typeof((m)->__v))__hashmap_get(&(m)->core, &__cgl_key); \
	})

#define hashmap_contains(m, k) (hashmap_get(m, k) != NULL)

/* Returns whether the key was there. */
#define hashmap_remove(m, k)                              \
	({                                                \
		typeof(*(m)->__k) __cgl_key = (k);        \
		__hashmap_remove(&(m)->core, &__cgl_key); \
	})

#define hashmap_size(m) ((m)->core.size)
#define hashmap_free(m) __hashmap_free(&(m)->core)

#endif
