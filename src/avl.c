
#include "avl.h"

#include <string.h>

struct __avl_node {
	struct __avl_node *left;
	struct __avl_node *right;
	int height;
	_Alignas(max_align_t) unsigned char data[];
};

static size_t round_up(size_t value, size_t align)
{
	return (value + align - 1) / align * align;
}

static int height(const struct __avl_node *node)
{
	return node ? node->height : 0;
}

static void update_height(struct __avl_node *node)
{
	int left = height(node->left);
	int right = height(node->right);

	node->height = (left > right ? left : right) + 1;
}

static struct __avl_node *rotate_right(struct __avl_node *node)
{
	struct __avl_node *pivot = node->left;

	node->left = pivot->right;
	pivot->right = node;
	update_height(node);
	update_height(pivot);
	return pivot;
}

static struct __avl_node *rotate_left(struct __avl_node *node)
{
	struct __avl_node *pivot = node->right;

	node->right = pivot->left;
	pivot->left = node;
	update_height(node);
	update_height(pivot);
	return pivot;
}

static struct __avl_node *rebalance(struct __avl_node *node)
{
	int balance = height(node->left) - height(node->right);

	update_height(node);
	if (balance > 1) {
		if (height(node->left->left) < height(node->left->right))
			node->left = rotate_left(node->left);
		return rotate_right(node);
	}
	if (balance < -1) {
		if (height(node->right->right) < height(node->right->left))
			node->right = rotate_right(node->right);
		return rotate_left(node);
	}
	return node;
}

static struct __avl_node *insert(struct __avl *t, struct __avl_node *node, const void *key,
                                 const void *value, int *status)
{
	int order;

	if (!node) {
		node = allocator_malloc(t->allocator,
		                        sizeof(*node) + t->value_offset + t->value_size);
		if (!node) {
			*status = -1;
			return NULL;
		}

		*node = (struct __avl_node){ .height = 1 };
		memcpy(node->data, key, t->key_size);
		memcpy(node->data + t->value_offset, value, t->value_size);
		t->size++;
		return node;
	}

	order = t->cmp(key, node->data);
	if (!order) {
		memcpy(node->data + t->value_offset, value, t->value_size);
		return node;
	}

	if (order < 0)
		node->left = insert(t, node->left, key, value, status);
	else
		node->right = insert(t, node->right, key, value, status);
	return rebalance(node);
}

static struct __avl_node *take_min(struct __avl_node *node, struct __avl_node **min)
{
	if (!node->left) {
		*min = node;
		return node->right;
	}

	node->left = take_min(node->left, min);
	return rebalance(node);
}

/* Relinks nodes rather than copying data, so pointers to other values stay valid. */
static struct __avl_node *erase(struct __avl *t, struct __avl_node *node, const void *key,
                                bool *found)
{
	int order;

	if (!node)
		return NULL;

	order = t->cmp(key, node->data);
	if (order < 0) {
		node->left = erase(t, node->left, key, found);
	} else if (order > 0) {
		node->right = erase(t, node->right, key, found);
	} else {
		struct __avl_node *left = node->left;
		struct __avl_node *right = node->right;

		*found = true;
		allocator_free(t->allocator, node);
		t->size--;

		if (!left)
			return right;
		if (!right)
			return left;

		right = take_min(right, &node);
		node->left = left;
		node->right = right;
	}
	return rebalance(node);
}

static void destroy(struct __avl *t, struct __avl_node *node)
{
	if (!node)
		return;

	destroy(t, node->left);
	destroy(t, node->right);
	allocator_free(t->allocator, node);
}

void __avl_init(struct __avl *t, size_t key_size, size_t key_align, size_t value_size,
                size_t value_align, int (*cmp)(const void *, const void *), struct avl_options opt)
{
	(void)key_align;
	*t = (struct __avl){
		.key_size = key_size,
		.value_size = value_size,
		.value_offset = round_up(key_size, value_align),
		.cmp = cmp,
		.allocator = opt.allocator ?: &libc_allocator,
	};
}

int __avl_put(struct __avl *t, const void *key, const void *value)
{
	int status = 0;
	struct __avl_node *root = insert(t, t->root, key, value, &status);

	/* A failed insert into an empty tree returns NULL, which is the same root. */
	t->root = root;
	return status;
}

void *__avl_get(struct __avl *t, const void *key)
{
	struct __avl_node *node = t->root;

	while (node) {
		int order = t->cmp(key, node->data);

		if (!order)
			return node->data + t->value_offset;
		node = order < 0 ? node->left : node->right;
	}
	return NULL;
}

bool __avl_remove(struct __avl *t, const void *key)
{
	bool found = false;

	t->root = erase(t, t->root, key, &found);
	return found;
}

void __avl_free(struct __avl *t)
{
	destroy(t, t->root);
	t->root = NULL;
	t->size = 0;
}
