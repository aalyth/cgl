#ifndef __CGL_LIST_H
#define __CGL_LIST_H

#include "alloc.h"
#include "common.h"
#include "optional.h"

#include <stddef.h>

struct list_options {
	const struct allocator *allocator;
};

#define __cgl_list_define(T, ty)                                            \
	struct __list_node_##ty {                                           \
		struct __list_node_##ty *prev;                              \
		struct __list_node_##ty *next;                              \
		T value;                                                    \
	};                                                                  \
                                                                            \
	struct __list_##ty {                                                \
		struct __list_node_##ty *head;                              \
		struct __list_node_##ty *tail;                              \
		size_t size;                                                \
		const struct allocator *allocator;                          \
	};                                                                  \
                                                                            \
	void __init_list_##ty(struct __list_##ty *, struct list_options);   \
	int __list_push_back_##ty(struct __list_##ty *, T);                 \
	int __list_push_front_##ty(struct __list_##ty *, T);                \
	struct __optional_##ty __list_pop_back_##ty(struct __list_##ty *);  \
	struct __optional_##ty __list_pop_front_##ty(struct __list_##ty *); \
	void __list_for_each_##ty(struct __list_##ty *, void (*)(T));       \
	void __list_free_##ty(struct __list_##ty *);

__cgl_macro_for_each_type(__cgl_list_define)

#define __cgl_list_type_case(T, ty) , T : (struct __list_##ty *)0
#define __cgl_list_init_case(T, ty) , struct __list_##ty * : __init_list_##ty
#define __cgl_list_push_back_case(T, ty) , struct __list_##ty * : __list_push_back_##ty
#define __cgl_list_push_front_case(T, ty) , struct __list_##ty * : __list_push_front_##ty
#define __cgl_list_pop_back_case(T, ty) , struct __list_##ty * : __list_pop_back_##ty
#define __cgl_list_pop_front_case(T, ty) , struct __list_##ty * : __list_pop_front_##ty
#define __cgl_list_for_each_case(T, ty) , struct __list_##ty * : __list_for_each_##ty
#define __cgl_list_free_case(T, ty) , struct __list_##ty * : __list_free_##ty

#define __cgl_list_dispatch(l, case) _Generic((l)__cgl_macro_for_each_type(case))

#define list(T)                                                                  \
	typeof(*_Generic((T) {} __cgl_macro_for_each_type(__cgl_list_type_case), \
	               default: (struct __list_v *)0))

#define list_init(l, ...) \
	__cgl_list_dispatch(l, __cgl_list_init_case)(l, (struct list_options){ __VA_ARGS__ })
#define list_push_back(l, x) __cgl_list_dispatch(l, __cgl_list_push_back_case)(l, x)
#define list_push_front(l, x) __cgl_list_dispatch(l, __cgl_list_push_front_case)(l, x)
#define list_pop_back(l) __cgl_list_dispatch(l, __cgl_list_pop_back_case)(l)
#define list_pop_front(l) __cgl_list_dispatch(l, __cgl_list_pop_front_case)(l)
#define list_for_each(l, fn) __cgl_list_dispatch(l, __cgl_list_for_each_case)(l, fn)
#define list_free(l) __cgl_list_dispatch(l, __cgl_list_free_case)(l)
#define list_size(l) ((l)->size)

#endif
