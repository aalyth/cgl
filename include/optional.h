#ifndef CGL_OPTIONAL_H
#define CGL_OPTIONAL_H

#include "common.h"

#include <assert.h>
#include <stddef.h>

#define __cgl_optional_define(T, ty)                                                \
	struct __optional_##ty {                                                    \
		T value;                                                            \
		bool present;                                                       \
	};                                                                          \
                                                                                    \
	static inline struct __optional_##ty __optional_some_##ty(T value)          \
	{                                                                           \
		return (struct __optional_##ty){ .value = value, .present = true }; \
	}                                                                           \
                                                                                    \
	static inline struct __optional_##ty __optional_none_##ty(void)             \
	{                                                                           \
		return (struct __optional_##ty){ .present = false };                \
	}                                                                           \
                                                                                    \
	static inline bool __optional_is_present_##ty(struct __optional_##ty o)     \
	{                                                                           \
		return o.present;                                                   \
	}

__cgl_macro_for_each_primitive(__cgl_optional_define)

/* Same size as a pointer: NULL is the empty state, so a NULL value can't be held. */
struct __optional_v {
	void *value;
};

static inline struct __optional_v __optional_some_v(void *value)
{
	return (struct __optional_v){ .value = value };
}

static inline struct __optional_v __optional_none_v(void)
{
	return (struct __optional_v){ .value = NULL };
}

static inline bool __optional_is_present_v(struct __optional_v o)
{
	return o.value != NULL;
}

#define __cgl_optional_type_case(T, ty) , T : (struct __optional_##ty *)0
#define __cgl_optional_some_case(T, ty) , T : __optional_some_##ty
#define __cgl_optional_present_case(T, ty) , struct __optional_##ty : __optional_is_present_##ty

#define __cgl_optional_of(x)                                                     \
	typeof(*_Generic((x)__cgl_macro_for_each_type(__cgl_optional_type_case), \
	               default: (struct __optional_v *)0))

#define optional(T) __cgl_optional_of((T){})

#define optional_some(x)                                                 \
	_Generic((x)__cgl_macro_for_each_type(__cgl_optional_some_case), \
	        default: __optional_some_v)(x)
#define optional_empty(T) ((optional(T)){})

#define optional_is_present(o) \
	_Generic((o)__cgl_macro_for_each_type(__cgl_optional_present_case))(o)
#define optional_is_missing(o) (!optional_is_present(o))

#define optional_unwrap(o)                            \
	({                                            \
		__auto_type __cgl_o = (o);            \
		assert(optional_is_present(__cgl_o)); \
		__cgl_o.value;                        \
	})

#define optional_unwrap_or_else(o, fn)                               \
	({                                                           \
		__auto_type __cgl_o = (o);                           \
		optional_is_present(__cgl_o) ? __cgl_o.value : fn(); \
	})

#define optional_map(o, fn)                                                               \
	({                                                                                \
		__auto_type __cgl_o = (o);                                                \
		optional_is_present(__cgl_o) ? optional_some(fn(__cgl_o.value)) :         \
		                               optional_empty(typeof(fn(__cgl_o.value))); \
	})

#define optional_flat_map(o, fn)                                                                  \
	({                                                                                        \
		__auto_type __cgl_o = (o);                                                        \
		optional_is_present(__cgl_o) ? fn(__cgl_o.value) : (typeof(fn(__cgl_o.value))){}; \
	})

#define optional_filter(o, fn)                                                             \
	({                                                                                 \
		__auto_type __cgl_o = (o);                                                 \
		(optional_is_present(__cgl_o) && fn(__cgl_o.value)) ? __cgl_o :            \
		                                                      (typeof(__cgl_o)){}; \
	})

#endif
