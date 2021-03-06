#include <string.h>

#include "fs_api.h"
#include "inode.h"
#include "iteration_carry.h"
#include "cmd_utils.h"

#include "logger.h"
#include "errors.h"


/*
 * Remove directory from filesystem, if it is not empty.
 */
int sim_rmdir(const char* path) {
	log_info("rmdir: [%s]", path);

	char dir_path[strlen(path) + 1];
	char dir_name[STRLEN_ITEM_NAME] = {0};
	struct inode inode_parent = {0};
	struct inode inode_rmdir = {0};
	struct carry_dir_item carry = {0};

	// CONTROL

	if (strlen(path) == 0) {
		set_myerrno(Err_arg_missing);
		goto fail;
	}
	if (split_path(path, dir_path, dir_name) == RETURN_FAILURE) {
		goto fail;
	}
	if (strcmp(dir_name, ".") == 0 || strcmp(dir_name, "..") == 0) {
		set_myerrno(Err_dir_arg_invalid);
		goto fail;
	}
	// load inode to remove and its parent for removing record of the directory
	if (get_inode_wparent(&inode_rmdir, &inode_parent, path) == RETURN_FAILURE) {
		goto fail;
	}
	if (inode_rmdir.inode_type != Inode_type_dirc) {
		set_myerrno(Err_item_not_directory);
		goto fail;
	}

	// REMOVE

	if (!is_directory_empty(&inode_rmdir)) {
		set_myerrno(Err_dir_not_empty);
		goto fail;
	}

	carry.id = inode_rmdir.id_inode;
	strncpy(carry.name, dir_name, STRLEN_ITEM_NAME);

	// delete record from parent
	// if this fails, which should not, only record about the inode is deleted,
	// so it is possible to retrieve it with command 'fsck' --> lost+found/
	if (iterate_links(&inode_parent, &carry, delete_block_item) == RETURN_FAILURE) {
		goto fail;
	}
	// delete directory itself
	if (free_inode_directory(&inode_rmdir) == RETURN_FAILURE) {
		goto fail;
	}

	return RETURN_SUCCESS;

fail:
	log_warning("rmdir: unable to remove directory [%s]", path);
	return RETURN_FAILURE;
}
