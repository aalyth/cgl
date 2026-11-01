
#include "vector.h"

#define vector_impl(T, ty)                                                                \
	int __init_vector_##ty(struct __vector_##ty *v, struct vector_options opt)        \
	{                                                                                 \
		const size_t cap = opt.capacity;                                          \
                                                                                          \
		*v = (struct __vector_##ty){ 0 };                                         \
		v->allocator = opt.allocator ?: &libc_allocator;                          \
		if (!cap)                                                                 \
			return 0;                                                         \
		if (cap > SIZE_MAX / sizeof(T))                                           \
			return -1;                                                        \
                                                                                          \
		v->data = allocator_malloc(v->allocator, cap * sizeof(T));                \
		if (!v->data)                                                             \
			return -1;                                                        \
                                                                                          \
		v->cap = cap;                                                             \
		return 0;                                                                 \
	}                                                                                 \
                                                                                          \
	static int __vector_grow_##ty(struct __vector_##ty *v)                            \
	{                                                                                 \
		const size_t max = SIZE_MAX / sizeof(T);                                  \
		size_t cap = v->cap ? v->cap * 2 : 8;                                     \
		T *data;                                                                  \
                                                                                          \
		if (v->cap > max / 2)                                                     \
			cap = max;                                                        \
		if (cap == v->cap)                                                        \
			return -1;                                                        \
                                                                                          \
		if (v->data)                                                              \
			data = allocator_realloc(v->allocator, v->data, cap * sizeof(T)); \
		else                                                                      \
			data = allocator_malloc(v->allocator, cap * sizeof(T));           \
		if (!data)                                                                \
			return -1;                                                        \
                                                                                          \
		v->data = data;                                                           \
		v->cap = cap;                                                             \
		return 0;                                                                 \
	}                                                                                 \
                                                                                          \
	int __vector_push_##ty(struct __vector_##ty *v, T x)                              \
	{                                                                                 \
		if (v->len == v->cap && __vector_grow_##ty(v))                            \
			return -1;                                                        \
		v->data[v->len++] = x;                                                    \
		return 0;                                                                 \
	}                                                                                 \
                                                                                          \
	struct __optional_##ty __vector_pop_##ty(struct __vector_##ty *v)                 \
	{                                                                                 \
		if (!v->len)                                                              \
			return __optional_none_##ty();                                    \
                                                                                          \
		return __optional_some_##ty(v->data[--v->len]);                           \
	}                                                                                 \
                                                                                          \
	void __vector_for_each_##ty(struct __vector_##ty *v, void (*fn)(T))               \
	{                                                                                 \
		for (size_t i = 0; i < v->len; i++)                                       \
			fn(v->data[i]);                                                   \
	}

__cgl_macro_for_each_type(vector_impl)
