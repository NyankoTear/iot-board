#include "lfs_utils.h"

int fs_flash_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    assert(off  % cfg->read_size == 0);
    assert(size % cfg->read_size == 0);
    assert(block < cfg->block_count);
    uint32_t addr = block * cfg->block_size + off;
    
    int ret = qspi_flash_read_page((uint8_t *)buffer, size, addr);

    if (ret) {
        return -1;
    }

    return 0;
}

int fs_flash_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    assert(off  % cfg->prog_size == 0);
    assert(size % cfg->prog_size == 0);
    assert(block < cfg->block_count);  

    uint32_t addr = block * cfg->block_size + off;
    int ret = qspi_flash_program_page((uint8_t *)buffer, size, addr);

    if (ret) {
        return -1;
    }
    
    return 0;
}

int fs_flash_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    assert(block < cfg->block_count);
    
    int ret = qspi_flash_erase_sector(block * cfg->block_size);

    if (ret) {
        return -1;
    }

    return 0;
}

int fs_flash_sync(const struct lfs_config *c)
{
    return 0;
}