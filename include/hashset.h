#ifndef __CGL_HASHSET_H
#define __CGL_HASHSET_H

#include "hashmap.h"

/* A hashmap whose values take up no space. */
#define hashset(T)                     \
	union {                        \
		struct __hashmap core; \
		T *__t;                \
	}

#define hashset_init(s, cmp, hash, ...)                                                       \
	__hashmap_init(&(s)->core, sizeof(*(s)->__t), _Alignof(typeof(*(s)->__t)), 0, 1, cmp, \
	               hash, (struct hashmap_options){ __VA_ARGS__ })

/* Returns 0, including if already there, or -1 if allocation failed. */
#define hashset_put(s, x)                                    \
	({                                                   \
		typeof(*(s)->__t) __cgl_key = (x);           \
		__hashmap_put(&(s)->core, &__cgl_key, NULL); \
	})

#define hashset_contains(s, x)                                 \
	({                                                     \
		typeof(*(s)->__t) __cgl_key = (x);             \
		__hashmap_get(&(s)->core, &__cgl_key) != NULL; \
	})

/* Returns whether the element was there. */
#define hashset_remove(s, x)                              \
	({                                                \
		typeof(*(s)->__t) __cgl_key = (x);        \
		__hashmap_remove(&(s)->core, &__cgl_key); \
	})

#define hashset_size(s) ((s)->core.size)
#define hashset_free(s) __hashmap_free(&(s)->core)

#endif
