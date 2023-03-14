#ifndef __LFS_UTILS_H__
#define __LFS_UTILS_H__

#include "lfs.h"
#include "qspi_flash.h"

int fs_flash_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int fs_flash_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int fs_flash_erase(const struct lfs_config *cfg, lfs_block_t block);
int fs_flash_sync(const struct lfs_config *c);

#endif /* __LFS_UTILS_H__ */