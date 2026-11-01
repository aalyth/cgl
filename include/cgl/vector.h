#ifndef CGL_VECTOR_H
#define CGL_VECTOR_H

#include "alloc.h"
#include "common.h"
#include "optional.h"

#include <stddef.h>

struct vector_options {
	size_t capacity;
	struct allocator *allocator;
};

#define __cgl_vector_define(T, ty)                                             \
	struct __vector_##ty {                                                 \
		T *data;                                                       \
		size_t len;                                                    \
		size_t cap;                                                    \
		struct allocator *allocator;                                   \
	};                                                                     \
                                                                               \
	int __init_vector_##ty(struct __vector_##ty *, struct vector_options); \
	int __vector_push_##ty(struct __vector_##ty *, T);                     \
	struct __optional_##ty __vector_pop_##ty(struct __vector_##ty *);      \
	void __vector_for_each_##ty(struct __vector_##ty *, void (*)(T));

__cgl_macro_for_each_type(__cgl_vector_define)

#define __cgl_vector_type_case(T, ty) , T : (struct __vector_##ty *)0
#define __cgl_vector_init_case(T, ty) , struct __vector_##ty * : __init_vector_##ty
#define __cgl_vector_push_case(T, ty) , struct __vector_##ty * : __vector_push_##ty
#define __cgl_vector_pop_case(T, ty) , struct __vector_##ty * : __vector_pop_##ty
#define __cgl_vector_for_each_case(T, ty) , struct __vector_##ty * : __vector_for_each_##ty

#define __cgl_vector_dispatch(v, case) _Generic((v)__cgl_macro_for_each_type(case))

#define vector(T)                                                                  \
	typeof(*_Generic((T) {} __cgl_macro_for_each_type(__cgl_vector_type_case), \
	               default: (struct __vector_v *)0))

#define vector_init(v, ...) \
	__cgl_vector_dispatch(v, __cgl_vector_init_case)(v, (struct vector_options){ __VA_ARGS__ })
#define vector_push(v, x) __cgl_vector_dispatch(v, __cgl_vector_push_case)(v, x)
#define vector_pop(v) __cgl_vector_dispatch(v, __cgl_vector_pop_case)(v)
#define vector_for_each(v, fn) __cgl_vector_dispatch(v, __cgl_vector_for_each_case)(v, fn)

#endif
