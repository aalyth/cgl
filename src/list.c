
#include "list.h"

#define list_impl(T, ty)                                                                       \
	void __init_list_##ty(struct __list_##ty *l, struct list_options opt)                  \
	{                                                                                      \
		*l = (struct __list_##ty){ .allocator = opt.allocator ?: &libc_allocator };    \
	}                                                                                      \
                                                                                               \
	int __list_push_back_##ty(struct __list_##ty *l, T value)                              \
	{                                                                                      \
		struct __list_node_##ty *node = allocator_malloc(l->allocator, sizeof(*node)); \
                                                                                               \
		if (!node)                                                                     \
			return -1;                                                             \
                                                                                               \
		*node = (struct __list_node_##ty){ .prev = l->tail, .value = value };          \
		if (l->tail)                                                                   \
			l->tail->next = node;                                                  \
		else                                                                           \
			l->head = node;                                                        \
		l->tail = node;                                                                \
		l->size++;                                                                     \
		return 0;                                                                      \
	}                                                                                      \
                                                                                               \
	int __list_push_front_##ty(struct __list_##ty *l, T value)                             \
	{                                                                                      \
		struct __list_node_##ty *node = allocator_malloc(l->allocator, sizeof(*node)); \
                                                                                               \
		if (!node)                                                                     \
			return -1;                                                             \
                                                                                               \
		*node = (struct __list_node_##ty){ .next = l->head, .value = value };          \
		if (l->head)                                                                   \
			l->head->prev = node;                                                  \
		else                                                                           \
			l->tail = node;                                                        \
		l->head = node;                                                                \
		l->size++;                                                                     \
		return 0;                                                                      \
	}                                                                                      \
                                                                                               \
	struct __optional_##ty __list_pop_back_##ty(struct __list_##ty *l)                     \
	{                                                                                      \
		struct __list_node_##ty *node = l->tail;                                       \
		T value;                                                                       \
                                                                                               \
		if (!node)                                                                     \
			return __optional_none_##ty();                                         \
                                                                                               \
		value = node->value;                                                           \
		l->tail = node->prev;                                                          \
		if (l->tail)                                                                   \
			l->tail->next = NULL;                                                  \
		else                                                                           \
			l->head = NULL;                                                        \
		l->size--;                                                                     \
		allocator_free(l->allocator, node);                                            \
		return __optional_some_##ty(value);                                            \
	}                                                                                      \
                                                                                               \
	struct __optional_##ty __list_pop_front_##ty(struct __list_##ty *l)                    \
	{                                                                                      \
		struct __list_node_##ty *node = l->head;                                       \
		T value;                                                                       \
                                                                                               \
		if (!node)                                                                     \
			return __optional_none_##ty();                                         \
                                                                                               \
		value = node->value;                                                           \
		l->head = node->next;                                                          \
		if (l->head)                                                                   \
			l->head->prev = NULL;                                                  \
		else                                                                           \
			l->tail = NULL;                                                        \
		l->size--;                                                                     \
		allocator_free(l->allocator, node);                                            \
		return __optional_some_##ty(value);                                            \
	}                                                                                      \
                                                                                               \
	void __list_for_each_##ty(struct __list_##ty *l, void (*fn)(T))                        \
	{                                                                                      \
		for (struct __list_node_##ty *node = l->head; node; node = node->next)         \
			fn(node->value);                                                       \
	}                                                                                      \
                                                                                               \
	void __list_free_##ty(struct __list_##ty *l)                                           \
	{                                                                                      \
		struct __list_node_##ty *node = l->head;                                       \
                                                                                               \
		while (node) {                                                                 \
			struct __list_node_##ty *next = node->next;                            \
                                                                                               \
			allocator_free(l->allocator, node);                                    \
			node = next;                                                           \
		}                                                                              \
		l->head = NULL;                                                                \
		l->tail = NULL;                                                                \
		l->size = 0;                                                                   \
	}

__cgl_macro_for_each_type(list_impl)
