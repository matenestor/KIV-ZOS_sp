#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "fs_api.h"
#include "fs_cache.h"

#include "errors.h"
#include "logger.h"

extern void fs_seek_bm_inode(uint32_t);
extern void fs_seek_bm_data(uint32_t);
extern size_t fs_read_bool(bool*, size_t);
extern size_t fs_write_bool(const bool*, size_t);


static const bool bm_true = true;
static const bool bm_false = false;

static void bitmap_field_inodes_on(const int32_t index) {
	fs_seek_bm_inode(index);
	fs_write_bool(&bm_true, 1);
}

static void bitmap_field_inodes_off(const int32_t index) {
	fs_seek_bm_inode(index);
	fs_write_bool(&bm_false, 1);
}

static void bitmap_field_data_on(const int32_t index) {
	fs_seek_bm_data(index);
	fs_write_bool(&bm_true, 1);
}

static void bitmap_field_data_off(const int32_t index) {
	fs_seek_bm_data(index);
	fs_write_bool(&bm_false, 1);
}

/*
 * Function searches for empty field in given bitmap seeked with 'fs_seek' function.
 * When empty field is found, it is turned off in the bitmap with 'bitmap_field_off' function.
 */
static uint32_t get_empty_bitmap_field(void (*fs_seek_bm)(), void(*bitmap_field_off)()) {
	size_t i;
	size_t id = FREE_LINK;
	bool* bitmap = malloc(sb.block_count);

	if (bitmap) {
		// --- seek beginning of bitmap
		fs_seek_bm(0);
		fs_read_bool(bitmap, sb.block_count);

		// check cached array for a free field
		for (i = 0; i < sb.block_count; ++i) {
			if (bitmap[i]) {
				// --- turn off empty field
				bitmap_field_off(i);
				id = i + 1;
				break;
			}
		}
		free(bitmap);
	}

	return id;
}

/*
 * Wrapper function for search of empty inode bitmap field.
 */
uint32_t allocate_bitmap_field_inode() {
	uint32_t index = get_empty_bitmap_field(fs_seek_bm_inode, bitmap_field_inodes_off);
	if (index == FREE_LINK) {
		if (is_error()) {
			my_perror("System error");
		} else {
			set_myerrno(Err_inode_no_inodes);
			log_error("Out of inodes.");
		}
	} else {
		log_info("Free inode, index: [%d].", index);
	}
	return index;
}

/*
 * Wrapper function for search of empty data block bitmap field.
 */
uint32_t allocate_bitmap_field_data() {
	uint32_t index = get_empty_bitmap_field(fs_seek_bm_data, bitmap_field_data_off);
	if (index == FREE_LINK) {
		if (is_error()) {
			my_perror("System error");
		} else {
			set_myerrno(Err_block_no_blocks);
			log_error("Out of data blocks.");
		}
	} else {
		log_info("Free data block, index: [%d].", index);
	}
	return index;
}

/*
 * Wrapper function for releasing inode bitmap field.
 */
void free_bitmap_field_inode(int32_t id) {
	bitmap_field_inodes_on(id - 1);
}

/*
 * Wrapper function for releasing data block bitmap field.
 */
void free_bitmap_field_data(int32_t id) {
	bitmap_field_data_on(id - 1);
}

// --- SPECIFIC FUNCTIONS FOR format.c

void format_root_bm_off() {
	bitmap_field_inodes_off(0);
	bitmap_field_data_off(0);
}