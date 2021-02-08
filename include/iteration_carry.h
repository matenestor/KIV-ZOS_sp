#ifndef ITERATION_CARRY_H
#define ITERATION_CARRY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "inode.h"

// ITERABLE functions are called as callback functions
// from iterate_links() during iteration over links in inode
#define ITERABLE(fn) bool fn(const uint32_t* links, const size_t links_count, void* p_carry)

int iterate_links(const struct inode* inode_source, void* carry, bool (*callback)());

struct carry_directory_item {
	uint32_t id;
	char name[STRLEN_ITEM_NAME];
};

ITERABLE(search_block_inode_id);
ITERABLE(search_block_inode_name);
ITERABLE(add_block_item);
ITERABLE(delete_block_item);
ITERABLE(has_common_directories);
ITERABLE(list_items);

#endif
