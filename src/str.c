
#include "str.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MIN_CAP 16

string_t string_from(const char *s)
{
	return (string_t){ .data = s, .len = strlen(s) };
}

/* Bounds are clamped, so an out-of-range slice is short or empty. */
string_t string_slice(string_t s, size_t start, size_t end)
{
	if (end > s.len)
		end = s.len;
	if (start > end)
		start = end;
	return (string_t){ .data = s.data + start, .len = end - start };
}

string_t string_trim(string_t s)
{
	while (s.len && isspace((unsigned char)s.data[0])) {
		s.data++;
		s.len--;
	}
	while (s.len && isspace((unsigned char)s.data[s.len - 1]))
		s.len--;
	return s;
}

static bool same_bytes(const char *a, const char *b, size_t len)
{
	return !len || !memcmp(a, b, len);
}

bool string_eq(string_t a, string_t b)
{
	return a.len == b.len && same_bytes(a.data, b.data, a.len);
}

int string_cmp(string_t a, string_t b)
{
	size_t len = a.len < b.len ? a.len : b.len;
	int order = len ? memcmp(a.data, b.data, len) : 0;

	if (order)
		return order;
	return (a.len > b.len) - (a.len < b.len);
}

bool string_starts_with(string_t s, string_t prefix)
{
	return s.len >= prefix.len && same_bytes(s.data, prefix.data, prefix.len);
}

bool string_ends_with(string_t s, string_t suffix)
{
	return s.len >= suffix.len &&
	       same_bytes(s.data + s.len - suffix.len, suffix.data, suffix.len);
}

optional(size_t) string_find(string_t s, string_t needle)
{
	if (!needle.len)
		return optional_some((size_t)0);

	for (size_t i = 0; s.len - i >= needle.len && i < s.len; i++) {
		const char *match = memchr(s.data + i, needle.data[0], s.len - needle.len - i + 1);

		if (!match)
			break;

		i = (size_t)(match - s.data);
		if (same_bytes(match, needle.data, needle.len))
			return optional_some(i);
	}
	return optional_empty(size_t);
}

bool string_contains(string_t s, string_t needle)
{
	return optional_is_present(string_find(s, needle));
}

bool string_split(string_t *rest, char sep, string_t *field)
{
	const char *end;

	if (!rest->data)
		return false;

	end = rest->len ? memchr(rest->data, sep, rest->len) : NULL;
	if (!end) {
		*field = *rest;
		*rest = (string_t){ 0 };
		return true;
	}

	*field = (string_t){ .data = rest->data, .len = (size_t)(end - rest->data) };
	rest->len -= field->len + 1;
	rest->data = end + 1;
	return true;
}

/* FNV-1a */
size_t string_hash(string_t s)
{
	uint64_t hash = UINT64_C(14695981039346656037);

	for (size_t i = 0; i < s.len; i++) {
		hash ^= (unsigned char)s.data[i];
		hash *= UINT64_C(1099511628211);
	}
	return (size_t)hash;
}

int string_key_cmp(const void *a, const void *b)
{
	return string_cmp(*(const string_t *)a, *(const string_t *)b);
}

size_t string_key_hash(const void *key)
{
	return string_hash(*(const string_t *)key);
}

int cstr_key_cmp(const void *a, const void *b)
{
	return strcmp(*(const char *const *)a, *(const char *const *)b);
}

size_t cstr_key_hash(const void *key)
{
	return string_hash(string_from(*(const char *const *)key));
}

int __string_builder_init(struct string_builder *s, struct string_builder_options opt)
{
	*s = (struct string_builder){ .allocator = opt.allocator ?: &libc_allocator };

	return opt.capacity ? string_builder_reserve(s, opt.capacity) : 0;
}

/* cap counts characters: the terminator always has room on top of it. */
int string_builder_reserve(struct string_builder *s, size_t cap)
{
	size_t next = s->cap > SIZE_MAX / 2 ? SIZE_MAX - 1 : s->cap * 2;
	char *data;

	if (cap <= s->cap && s->data)
		return 0;
	if (cap == SIZE_MAX)
		return -1;

	if (next < cap)
		next = cap;
	if (next < MIN_CAP)
		next = MIN_CAP;

	if (s->data)
		data = allocator_realloc(s->allocator, s->data, next + 1);
	else
		data = allocator_malloc(s->allocator, next + 1);
	if (!data)
		return -1;

	if (!s->data)
		data[0] = '\0';
	s->data = data;
	s->cap = next;
	return 0;
}

int string_builder_push(struct string_builder *s, char c)
{
	return __string_builder_append(s, (string_t){ .data = &c, .len = 1 });
}

int __string_builder_append(struct string_builder *s, string_t x)
{
	bool inside = s->data && x.data >= s->data && x.data <= s->data + s->len;
	size_t offset = inside ? (size_t)(x.data - s->data) : 0;

	if (x.len > SIZE_MAX - s->len || string_builder_reserve(s, s->len + x.len))
		return -1;

	if (x.len)
		memmove(s->data + s->len, inside ? s->data + offset : x.data, x.len);
	s->len += x.len;
	s->data[s->len] = '\0';
	return 0;
}

int string_builder_appendf(struct string_builder *s, const char *fmt, ...)
{
	va_list args;
	va_list copy;
	int len;

	va_start(args, fmt);
	va_copy(copy, args);
	len = vsnprintf(NULL, 0, fmt, copy);
	va_end(copy);

	if (len < 0 || string_builder_reserve(s, s->len + (size_t)len)) {
		va_end(args);
		return -1;
	}

	vsnprintf(s->data + s->len, (size_t)len + 1, fmt, args);
	va_end(args);
	s->len += (size_t)len;
	return 0;
}

string_t string_builder_view(const struct string_builder *s)
{
	return (string_t){ .data = s->data ? s->data : "", .len = s->len };
}

const char *string_builder_cstr(const struct string_builder *s)
{
	return s->data ? s->data : "";
}

void string_builder_clear(struct string_builder *s)
{
	s->len = 0;
	if (s->data)
		s->data[0] = '\0';
}

void string_builder_free(struct string_builder *s)
{
	if (s->data)
		allocator_free(s->allocator, s->data);
	s->data = NULL;
	s->len = 0;
	s->cap = 0;
}
