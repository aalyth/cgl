#ifndef __CGL_STR_H
#define __CGL_STR_H

#include "alloc.h"
#include "optional.h"

#include <stdbool.h>
#include <stddef.h>

/* A view of bytes owned by someone else. Not NUL-terminated. */
struct str {
	const char *data;
	size_t len;
};

#define str_lit(s) ((struct str){ .data = (s), .len = sizeof(s) - 1 })

struct str str_from(const char *);
struct str str_slice(struct str, size_t start, size_t end);
struct str str_trim(struct str);

bool str_eq(struct str, struct str);
int str_cmp(struct str, struct str);
bool str_starts_with(struct str, struct str prefix);
bool str_ends_with(struct str, struct str suffix);
bool str_contains(struct str, struct str needle);
optional(size_t) str_find(struct str, struct str needle);

/*
 * Takes the text before the next sep off the front of rest. Returns false once
 * rest is used up, so an empty field between two separators still comes back.
 *
 *	struct str rest = str_lit("a,b,c"), field;
 *	while (str_split(&rest, ',', &field))
 *		...
 */
bool str_split(struct str *rest, char sep, struct str *field);

/* For hashmap, hashset and avl: str_key_* for struct str keys, cstr_key_* for char *. */
size_t str_hash(struct str);
int str_key_cmp(const void *, const void *);
size_t str_key_hash(const void *);
int cstr_key_cmp(const void *, const void *);
size_t cstr_key_hash(const void *);

struct string_options {
	size_t capacity;
	const struct allocator *allocator;
};

/* An owned, growable buffer, always NUL-terminated once it has data. */
struct string {
	char *data;
	size_t len;
	size_t cap;
	const struct allocator *allocator;
};

int __string_init(struct string *, struct string_options);
int __string_append(struct string *, struct str);

/* Returns 0, or -1 if reserving capacity failed, leaving s empty but usable. */
#define string_init(s, ...) __string_init(s, (struct string_options){ __VA_ARGS__ })

/* All of these return 0, or -1 if allocation failed, leaving s unchanged. */
int string_reserve(struct string *, size_t cap);
int string_push(struct string *, char);
__attribute__((format(printf, 2, 3))) int string_appendf(struct string *, const char *fmt, ...);

/* x is a char *, or a struct str, which may be a view into s itself. */
#define string_append(s, x)                      \
	__string_append(s, _Generic((x),         \
	                        struct str: (x), \
	                        default: str_from(_Generic((x), struct str: "", default: (x)))))

struct str string_view(const struct string *);
const char *string_cstr(const struct string *);
void string_clear(struct string *);
void string_free(struct string *);

#endif
