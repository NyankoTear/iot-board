#ifndef __QSPI_FLASH_H__
#define __QSPI_FLASH_H__

#include "quadspi.h"

int qspi_flash_initialize(QSPI_HandleTypeDef *hqspi);
int qspi_flash_deinitialize(void);
int qspi_flash_read_identification(uint8_t* p_identification);
int qspi_flash_program_page(uint8_t *data, uint32_t data_length, uint32_t address);
int qspi_flash_erase_sector(uint32_t address);
int qspi_flash_erase_chip(void);
int qspi_flash_read_page(uint8_t *data, uint32_t data_length, uint32_t address);

#endif /* __QSPI_FLASH_H__ */