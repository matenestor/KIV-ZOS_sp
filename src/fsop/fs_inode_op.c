#include <stdint.h>

#include "fs_api.h"
#include "fs_cache.h"

#include "errors.h"
#include "logger.h"


extern int free_all_links(struct inode* inode_source);

// ---------- INODE FREE ------------------------------------------------------

/*
 * 	Free given inode by resetting its values,
 * 	freeing links and turning on its bitmap field.
 */
static int free_inode(struct inode* inode2free) {
	// free given inode
	inode2free->inode_type = Inode_type_free;
	inode2free->file_size = 0;
	free_all_links(inode2free);
	free_bitmap_field_inode(inode2free->id_inode);
	fs_write_inode(inode2free, 1, inode2free->id_inode);

	return RETURN_SUCCESS;
}

/*
 * Free file inode. If 'id_inode' is id of directory or free inode, error is set.
 */
int free_inode_file(struct inode* inode2free) {
	int ret = RETURN_FAILURE;

	if (inode2free->inode_type == Inode_type_file) {
		free_inode(inode2free);
		ret = RETURN_SUCCESS;
	}
	else if (inode2free->inode_type == Inode_type_dirc) {
		set_myerrno(Err_item_not_file);
	} else {
		set_myerrno(Err_item_not_exists);
	}
	return ret;
}

/*
 * Free file inode. If directory is not empty
 * or 'id_inode' is id of file or free inode, error is set.
 */
int free_inode_directory(struct inode* inode2free) {
	int ret = RETURN_FAILURE;

	if (inode2free->inode_type == Inode_type_dirc) {
		if (is_directory_empty(inode2free)) {
			free_inode(inode2free);
			ret = RETURN_SUCCESS;
		} else {
			set_myerrno(Err_dir_not_empty);
		}
	}
	else if (inode2free->inode_type == Inode_type_file) {
		set_myerrno(Err_item_not_directory);
	} else {
		set_myerrno(Err_item_not_exists);
	}
	return ret;
}

// ---------- INODE CREATE ----------------------------------------------------

/*
 *  Create new inode for a file. One inode from filesystem is used.
 *  Returns index number of new inode, or 'RETURN_FAILURE'.
 */
uint32_t create_inode_file(struct inode* new_inode) {
	// id of new inode, which will be used
	uint32_t id_free_inode = allocate_bitmap_field_inode();

	if (id_free_inode != FREE_LINK) {
		// cache the free inode, init it and write it
		fs_read_inode(new_inode, 1, id_free_inode);
		new_inode->inode_type = Inode_type_file;
		fs_write_inode(new_inode, 1, id_free_inode);

		log_info("New file inode created, id: [%d].", id_free_inode);
	} else {
		// no need for free_bitmap_field_inode(), because nothing
		// was allocated, if 'id_free_inode' is RETURN_FAILURE
		log_error("Unable to create new inode.");
	}

	return id_free_inode;
}

/*
 *  Create new inode for a directory. One inode and one data block from filesystem is used.
 *  Returns index number of new inode, or 'RETURN_FAILURE'.
 */
uint32_t create_inode_directory(struct inode* new_inode, const uint32_t id_parent) {
	// id of new inode, which will be initialized
	uint32_t id_free_inode = allocate_bitmap_field_inode();
	// id of new block, where first direct link will point to; equal to create_direct()
	uint32_t id_free_block = allocate_bitmap_field_data();
	// default folders in each directory
	struct directory_item new_dirs[sb.count_dir_items];

	// if there is free inode and data block for it, use it
	if (!(id_free_inode == FREE_LINK || id_free_block == FREE_LINK)) {
		fs_read_inode(new_inode, 1, id_free_inode);

		// init first block of new directory
		init_empty_dir_block(new_dirs, id_free_inode, id_parent);
		// init new inode
		new_inode->inode_type = Inode_type_dirc;
		new_inode->file_size = sb.block_size;
		new_inode->direct[0] = id_free_block;

		// write new updated inode and data block
		fs_write_inode(new_inode, 1, id_free_inode);
		fs_write_directory_item(new_dirs, sb.count_dir_items, id_free_block);

		log_info("New directory inode created, id: [%d].", id_free_inode);
	}
	else {
		// getting empty bitmap fields turned them off, so turn them on again
		if (id_free_inode != FREE_LINK) {
			free_bitmap_field_inode(id_free_inode);
			id_free_inode = FREE_LINK;
		}
		if (id_free_block != FREE_LINK) {
			free_bitmap_field_data(id_free_block);
		}

		log_error("Unable to create new inode.");
	}

	return id_free_inode;
}
