
#include <cgl/hashmap.h>

#include <string.h>

#define MIN_CAP 8
#define OCCUPIED ((uint64_t)1 << 63)

static size_t round_up(size_t value, size_t align)
{
	return (value + align - 1) / align * align;
}

/* Multiplies first, so that a weak hash such as the identity still spreads out. */
static size_t slot_of(const struct __hashmap *m, uint64_t hash)
{
	return (hash * UINT64_C(0x9E3779B97F4A7C15)) >> m->shift;
}

static unsigned char *entry_at(const struct __hashmap *m, size_t i)
{
	return m->entries + i * m->entry_size;
}

static uint64_t hash_key(const struct __hashmap *m, const void *key)
{
	return (uint64_t)m->hash(key) | OCCUPIED;
}

static bool find(const struct __hashmap *m, const void *key, uint64_t hash, size_t *slot)
{
	size_t i;

	if (!m->cap)
		return false;

	i = slot_of(m, hash);
	while (m->hashes[i]) {
		if (m->hashes[i] == hash && !m->cmp(key, entry_at(m, i))) {
			*slot = i;
			return true;
		}
		i = (i + 1) & (m->cap - 1);
	}
	return false;
}

static int resize(struct __hashmap *m, size_t cap)
{
	struct __hashmap next = *m;
	unsigned bits = 0;

	if (cap > SIZE_MAX / m->entry_size || cap > SIZE_MAX / sizeof(*next.hashes))
		return -1;

	while (((size_t)1 << bits) < cap)
		bits++;

	next.cap = cap;
	next.shift = 64 - bits;
	next.hashes = allocator_malloc(m->allocator, cap * sizeof(*next.hashes));
	if (!next.hashes)
		return -1;

	next.entries = allocator_malloc(m->allocator, cap * m->entry_size);
	if (!next.entries) {
		allocator_free(m->allocator, next.hashes);
		return -1;
	}

	memset(next.hashes, 0, cap * sizeof(*next.hashes));
	for (size_t i = 0; i < m->cap; i++) {
		size_t slot;

		if (!m->hashes[i])
			continue;

		slot = slot_of(&next, m->hashes[i]);
		while (next.hashes[slot])
			slot = (slot + 1) & (cap - 1);

		next.hashes[slot] = m->hashes[i];
		memcpy(entry_at(&next, slot), entry_at(m, i), m->entry_size);
	}

	if (m->cap) {
		allocator_free(m->allocator, m->hashes);
		allocator_free(m->allocator, m->entries);
	}
	*m = next;
	return 0;
}

int __hashmap_init(struct __hashmap *m, size_t key_size, size_t key_align, size_t value_size,
                   size_t value_align, int (*cmp)(const void *, const void *),
                   size_t (*hash)(const void *), struct hashmap_options opt)
{
	size_t align = key_align > value_align ? key_align : value_align;
	size_t value_offset = round_up(key_size, value_align);
	size_t cap = MIN_CAP;

	*m = (struct __hashmap){
		.key_size = key_size,
		.value_size = value_size,
		.value_offset = value_offset,
		.entry_size = round_up(value_offset + value_size, align),
		.cmp = cmp,
		.hash = hash,
		.allocator = opt.allocator ?: &libc_allocator,
	};

	if (!opt.capacity)
		return 0;
	if (opt.capacity > SIZE_MAX / 8)
		return -1;

	while (cap / 4 * 3 < opt.capacity)
		cap *= 2;
	return resize(m, cap);
}

int __hashmap_put(struct __hashmap *m, const void *key, const void *value)
{
	uint64_t hash = hash_key(m, key);
	size_t slot;

	if (!find(m, key, hash, &slot)) {
		if (m->size >= m->cap / 4 * 3 && resize(m, m->cap ? m->cap * 2 : MIN_CAP))
			return -1;

		slot = slot_of(m, hash);
		while (m->hashes[slot])
			slot = (slot + 1) & (m->cap - 1);

		m->hashes[slot] = hash;
		memcpy(entry_at(m, slot), key, m->key_size);
		m->size++;
	}

	if (m->value_size)
		memcpy(entry_at(m, slot) + m->value_offset, value, m->value_size);
	return 0;
}

optional(void *) __hashmap_get(struct __hashmap *m, const void *key)
{
	size_t slot;

	if (!find(m, key, hash_key(m, key), &slot))
		return __optional_none_v();
	return __optional_some_v(entry_at(m, slot) + m->value_offset);
}

bool __hashmap_remove(struct __hashmap *m, const void *key)
{
	size_t mask = m->cap - 1;
	size_t hole;
	size_t i;

	if (!find(m, key, hash_key(m, key), &hole))
		return false;

	i = hole;
	for (;;) {
		size_t home;

		i = (i + 1) & mask;
		if (!m->hashes[i])
			break;

		home = slot_of(m, m->hashes[i]);
		if (((i - home) & mask) >= ((i - hole) & mask)) {
			m->hashes[hole] = m->hashes[i];
			memcpy(entry_at(m, hole), entry_at(m, i), m->entry_size);
			hole = i;
		}
	}

	m->hashes[hole] = 0;
	m->size--;
	return true;
}

void __hashmap_free(struct __hashmap *m)
{
	if (m->cap) {
		allocator_free(m->allocator, m->hashes);
		allocator_free(m->allocator, m->entries);
	}
	m->hashes = NULL;
	m->entries = NULL;
	m->size = 0;
	m->cap = 0;
}
