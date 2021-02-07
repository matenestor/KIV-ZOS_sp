#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "fs_api.h"
#include "fs_cache.h"


// --- SEEK

static void fs_seek_set(const int64_t offset) {
	assert(offset >= 0); // TODO delete
	fseek(filesystem, offset, SEEK_SET);
}

static void fs_seek_inodes(const uint32_t index) {
	fs_seek_set(sb.addr_inodes + (int64_t) index * sizeof(struct inode));
}

static void fs_seek_data(const uint32_t index) {
	fs_seek_set(sb.addr_data + (int64_t) index * sb.block_size);
}

// function is wrapped and only used in fs_bitmap.c
void fs_seek_bm_inode(const uint32_t index) {
	fs_seek_set(sb.addr_bm_inodes + (int64_t) index);
}

// function is wrapped and only used in fs_bitmap.c
void fs_seek_bm_data(const uint32_t index) {
	fs_seek_set(sb.addr_bm_data + (int64_t) index);
}

// --- READ

// only used in fs_common.c:init_filesystem()
size_t fs_read_superblock(struct superblock* buffer) {
	return fread(buffer, sizeof(struct superblock), 1, filesystem);
}
// function is wrapped and only used in fs_bitmap.c,
// because there are two types of bitmap, so fseek is in the wrapper function
size_t fs_read_bool(bool* buffer, const size_t count) {
	return fread(buffer, sizeof(bool), count, filesystem);
}

size_t fs_read_inode(struct inode* buffer, const size_t count, const uint32_t id) {
	fs_seek_inodes(id - 1);
	return fread(buffer, sizeof(struct inode), count, filesystem);
}

size_t fs_read_directory_item(struct directory_item* buffer,
							  const size_t count, const uint32_t id) {
	fs_seek_data(id - 1);
	return fread(buffer, sizeof(struct directory_item), count, filesystem);
}

size_t fs_read_link(uint32_t* buffer, const size_t count, const uint32_t id) {
	fs_seek_data(id - 1);
	return fread(buffer, sizeof(uint32_t), count, filesystem);
}

size_t fs_read_data(char* buffer, const size_t count, const uint32_t id) {
	fs_seek_data(id - 1);
	return fread(buffer, sizeof(char), count, filesystem);
}

// --- WRITE

// only used in format.c:init_superblock()
size_t fs_write_superblock(const struct superblock* buffer) {
	size_t ret;
	ret = fwrite(buffer, sizeof(struct superblock), 1, filesystem);
	fs_flush();
	return ret;
}
// function is wrapped and only used in fs_bitmap.c,
// because there are two types of bitmap, so fseek is in the wrapper function
size_t fs_write_bool(const bool* buffer, const size_t count) {
	static size_t ret;
	ret = fwrite(buffer, sizeof(bool), count, filesystem);
	fs_flush();
	return ret;
}

size_t fs_write_inode(const struct inode* buffer, const size_t count, const uint32_t id) {
	static size_t ret;
	fs_seek_inodes(id - 1);
	ret = fwrite(buffer, sizeof(struct inode), count, filesystem);
	fs_flush();
	return ret;
}

size_t fs_write_directory_item(const struct directory_item* buffer,
							   const size_t count, const uint32_t id) {
	static size_t ret;
	fs_seek_data(id - 1);
	ret = fwrite(buffer, sizeof(struct directory_item), count, filesystem);
	fs_flush();
	return ret;
}

size_t fs_write_link(const uint32_t* buffer, const size_t count, const uint32_t id) {
	static size_t ret;
	fs_seek_data(id - 1);
	ret = fwrite(buffer, sizeof(uint32_t), count, filesystem);
	fs_flush();
	return ret;
}

size_t fs_write_data(const char* buffer, const size_t count, const uint32_t id) {
	static size_t ret;
	fs_seek_data(id - 1);
	ret = fwrite(buffer, sizeof(char), count, filesystem);
	fs_flush();
	return ret;
}

// --- FLUSH

void fs_flush() {
	fflush(filesystem);
}

// --- SPECIFIC FUNCTIONS FOR format.c

size_t format_write_bool(const bool* buffer, const size_t count) {
	return fwrite(buffer, sizeof(bool), count, filesystem);
}

size_t format_write_inode(const struct inode* buffer, const size_t count) {
	return fwrite(buffer, sizeof(struct inode), count, filesystem);
}

size_t format_write_char(const char* buffer, const size_t count) {
	return fwrite(buffer, sizeof(char), count, filesystem);
}

size_t format_write_directory_item(const struct directory_item* buffer, const size_t count) {
	return fwrite(buffer, sizeof(struct directory_item), count, filesystem);
}
