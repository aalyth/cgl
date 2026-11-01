#ifndef __CGL_STR_H
#define __CGL_STR_H

#include "alloc.h"
#include "optional.h"

#include <stdbool.h>
#include <stddef.h>

/* Immutable view of externally owned data. Not NUL-terminated. */
typedef struct {
	const char *data;
	size_t len;
} string_t;

#define string_lit(s) ((string_t){ .data = (s), .len = sizeof(s) - 1 })

string_t string_from(const char *);
string_t string_slice(string_t, size_t start, size_t end);
string_t string_trim(string_t);

bool string_eq(string_t, string_t);
int string_cmp(string_t, string_t);
bool string_starts_with(string_t, string_t prefix);
bool string_ends_with(string_t, string_t suffix);
bool string_contains(string_t, string_t needle);
optional(size_t) string_find(string_t, string_t needle);

/*
 * Takes the text before the next sep off the front of rest. Returns false once
 * rest is used up, so an empty field between two separators still comes back.
 *
 *	string_t rest = string_lit("a,b,c"), field;
 *	while (string_split(&rest, ',', &field))
 *		...
 */
bool string_split(string_t *rest, char sep, string_t *field);

/* hashmap/hashset utilities */
size_t string_hash(string_t);
int string_key_cmp(const void *, const void *);
size_t string_key_hash(const void *);
int cstr_key_cmp(const void *, const void *);
size_t cstr_key_hash(const void *);

struct string_builder_options {
	size_t capacity;
	const struct allocator *allocator;
};

/* Builds up text in an owned buffer, always NUL-terminated once it has data. */
struct string_builder {
	char *data;
	size_t len;
	size_t cap;
	const struct allocator *allocator;
};

int __string_builder_init(struct string_builder *, struct string_builder_options);
int __string_builder_append(struct string_builder *, string_t);

/* Returns 0, or -1 if reserving capacity failed, leaving b empty but usable. */
#define string_builder_init(b, ...) \
	__string_builder_init(b, (struct string_builder_options){ __VA_ARGS__ })

/* All of these return 0, or -1 if allocation failed, leaving b unchanged. */
int string_builder_reserve(struct string_builder *, size_t cap);
int string_builder_push(struct string_builder *, char);
__attribute__((format(printf, 2, 3))) int string_builder_appendf(struct string_builder *,
                                                                 const char *fmt, ...);

/* x is a char *, or a string_t, which may be a view into b itself. */
#define string_builder_append(b, x)    \
	__string_builder_append(       \
	        b, _Generic((x),       \
	                string_t: (x), \
	                default: string_from(_Generic((x), string_t: "", default: (x)))))

/* A view of the current contents, invalid once the builder changes. */
string_t string_builder_view(const struct string_builder *);
const char *string_builder_cstr(const struct string_builder *);
void string_builder_clear(struct string_builder *);
void string_builder_free(struct string_builder *);

#endif
