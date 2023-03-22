#include "qspi_flash.h"

static QSPI_HandleTypeDef *_hqspi = NULL;

typedef struct {
    char* command_name;
    uint8_t command_byte;
    uint8_t address_length;
    uint8_t dummy_cycles_length;
} qspi_flash_command_instruction;

qspi_flash_command_instruction flash_command_instrunction_list[] = {
    {"Normal Read",                                         0x03, 3, 0},
    {"Fast Read",                                           0x0B, 3, 1},
    {"2Read (2 x I/O read)",                                0xBB, 3, 4},
    {"DREAD (1I / 2O read)",                                0x3B, 3, 8},
    {"4READ (4 x I/O read)",                                0xEB, 3, 6},
    {"QREAD (1I / 4O read)",                                0x6B, 3, 8},
    {"Page Program",                                        0x02, 3, 0},
    {"Quad Page Program",                                   0x38, 3, 0},
    {"Sector Erase",                                        0x20, 3, 0},
    {"Block Erase 32KB",                                    0x52, 3, 0},
    {"Block Erase 64KB",                                    0xD8, 3, 0},
    {"Chip Earse",                                          0x60, 0, 0},
    {"Read SFDP",                                           0x5A, 0, 1},
    {"Write Enable",                                        0x06, 0, 0},
    {"Write Disable",                                       0x04, 0, 0},
    {"Read Status Register",                                0x05, 0, 0},
    {"Read Configuration Register",                         0x15, 0, 0},
    {"Write Status Register",                               0x01, 3, 0},
    {"Suspend Program/Erase",                               0x75, 0, 0},
    {"Resume Program/Erase",                                0x7A, 0, 0},
    {"Deep Power Down",                                     0xB9, 0, 0},
    {"Set Burst Length",                                    0xC0, 0, 0},
    {"Read Identification",                                 0x9F, 0, 0},
    {"Read Electronic ID",                                  0xAB, 0, 0},
    {"Read Electronic Manufacturer & Device ID",            0x90, 0, 2},
    {"Enter Secured OTP",                                   0xB1, 0, 0},
    {"Exit Secured OTP",                                    0xC1, 0, 0},
    {"Read Security Register",                              0x2B, 0, 0},
    {"Write Security Register",                             0x2F, 0, 0},
    {"No Operation",                                        0x00, 0, 0},
    {"Reset Enable",                                        0x66, 0, 0},
    {"Reset Memory",                                        0x99, 0, 0},
};

int qspi_flash_initialize(QSPI_HandleTypeDef *hqspi)
{
    if (!hqspi) {
        DEBUG_VV("ERROR: Unreferenced qspi peripheral.\r\n");
        return 1;
    }

    _hqspi = hqspi;
    return 0;
}

int qspi_flash_deinitialize(void)
{
    _hqspi = NULL;

    return 0;
}

static int qspi_flash_write_enable(void)
{
    if (!_hqspi) {
        DEBUG_VV("ERROR: Unreferenced qspi peripheral.\r\n");
        return 1;
    }

    QSPI_CommandTypeDef qspi_flash_write_enable = {
        .Instruction = 0x06,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DummyCycles = 0,
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .DataMode = QSPI_DATA_NONE,
        .NbData = 0,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };
    
    int ret = (int)HAL_QSPI_Command(_hqspi, &qspi_flash_write_enable, HAL_MAX_DELAY);
    return ret;
}

// static int qspi_flash_write_disable(void)
// {
//     if (!_hqspi) {
//         DEBUG_VV("ERROR: Unreferenced qspi peripheral.\r\n");
//         return 1;
//     }

//     QSPI_CommandTypeDef qspi_flash_write_enable = {
//         .Instruction = 0x04,
//         .AddressMode = QSPI_ADDRESS_NONE,
//         .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
//         .DummyCycles = 0,
//         .InstructionMode = QSPI_INSTRUCTION_1_LINE,
//         .DataMode = QSPI_DATA_NONE,
//         .NbData = 0,
//         .DdrMode = QSPI_DDR_MODE_DISABLE,
//         .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
//     };
    
//     int ret = (int)HAL_QSPI_Command(_hqspi, &qspi_flash_write_enable, HAL_MAX_DELAY);
//     return ret;
// }

static int qspi_flash_read_status_register(uint8_t* p_status_register)
{
    if (!_hqspi) {
        DEBUG_VV("ERROR: Unreferenced qspi peripheral.\r\n");
        return 1;
    }

    QSPI_CommandTypeDef qspi_flash_write_enable = {
        .Instruction = 0x05,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DummyCycles = 0,
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .DataMode = QSPI_DATA_1_LINE,
        .NbData = 1,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };
    
    int ret = (int)HAL_QSPI_Command(_hqspi, &qspi_flash_write_enable, HAL_MAX_DELAY);

    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: HAL_QSPI_Command status %d\r\n", ret);
        return ret;
    }

    uint8_t rx_buf[1] = {0x00};

    ret = HAL_QSPI_Receive(_hqspi, rx_buf, HAL_MAX_DELAY);

    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: HAL_QSPI_Receive status %d\r\n", ret);
        return ret;
    }

    DEBUG_VVV("Status Register: 0x%02X\r\n", rx_buf[0]);
    DEBUG_VVV("%s\r\n", ((rx_buf[0] & 0x80) == 0x80) ? "Status register write disabled." : "Status register write enabled.");
    DEBUG_VVV("%s\r\n", ((rx_buf[0] & 0x40) == 0x40) ? "Quad I/O mode enabled." : "Quad I/O mode disabled.");
    DEBUG_VVV("%s\r\n", ((rx_buf[0] & 0x02) == 0x02) ? "Write enabled." : "Write disabled.");
    DEBUG_VVV("%s\r\n", ((rx_buf[0] & 0x01) == 0x01) ? "Write in progress." : "Not in write operation.");
    
    if (p_status_register) {
        *p_status_register = rx_buf[0];
    }

    return ret;
}

static int qspi_flash_read_security_register(uint8_t* p_security_register)
{
    if (!_hqspi) {
        DEBUG_VV("ERROR: Unreferenced qspi peripheral.\r\n");
        return 1;
    }

    QSPI_CommandTypeDef qspi_read_security_register = {
        .Instruction = 0x2B,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DummyCycles = 0,
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .DataMode = QSPI_DATA_1_LINE,
        .NbData = 1,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };

    int ret = (int)HAL_QSPI_Command(_hqspi, &qspi_read_security_register, HAL_MAX_DELAY);

    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: HAL_QSPI_Command status %d\r\n", ret);
        return ret;
    }

    uint8_t rx_buf[1] = {0x00};

    ret = HAL_QSPI_Receive(_hqspi, rx_buf, HAL_MAX_DELAY);

    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: HAL_QSPI_Receive status %d\r\n", ret);
        return ret;
    }

    DEBUG_VVV("Status Register: 0x%02X\r\n", rx_buf[0]);
    DEBUG_VVV("%s\r\n", ((rx_buf[0] & 0x40) == 0x40) ? "Indicate erase failed." : "Normal erase succeed.");
    DEBUG_VVV("%s\r\n", ((rx_buf[0] & 0x20) == 0x20) ? "Indicate program failed." : "Normal program succeed.");
    DEBUG_VVV("%s\r\n", ((rx_buf[0] & 0x08) == 0x08) ? "Erase suspended." : "Erase is not suspended.");
    DEBUG_VVV("%s\r\n", ((rx_buf[0] & 0x04) == 0x04) ? "Program suspended." : "Program is not suspended.");
    DEBUG_VVV("%s\r\n", ((rx_buf[0] & 0x02) == 0x02) ? "Lock-down state." : "Not in the Lock-down state.");
    DEBUG_VVV("%s\r\n", ((rx_buf[0] & 0x01) == 0x01) ? "Factory lock." : "Non-factory lock.");
    
    if (p_security_register) {
        *p_security_register = rx_buf[0];
    }

    return ret;
}

int qspi_flash_read_identification(uint8_t* p_identification)
{
    if (!_hqspi) {
        DEBUG_VV("ERROR: Unreferenced qspi peripheral.\r\n");
        return 1;
    }

    QSPI_CommandTypeDef qspi_flash_id_read = {
        .Instruction = 0x9F,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DummyCycles = 0,
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .DataMode = QSPI_DATA_1_LINE,
        .NbData = 3,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };

    int ret = HAL_QSPI_Command(_hqspi, &qspi_flash_id_read, 1000);

    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: HAL_QSPI_Command status %d\r\n", ret);
        return ret;
    }

    uint8_t rx_buf[3] = {0x00, };
    ret = HAL_QSPI_Receive(_hqspi, rx_buf, 1000);

    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: HAL_QSPI_Receive status %d\r\n", ret);
        return ret;
    }

    if (p_identification) {
        for (int i = 0; i < sizeof(rx_buf) / sizeof(uint8_t); i++) {
            *p_identification = rx_buf[i];
            p_identification++;
        } 
    }

    return ret;
}

int qspi_flash_program_page(uint8_t *data, uint32_t data_length, uint32_t address)
{
    if (!_hqspi) {
        DEBUG_VV("ERROR: Unreferenced qspi peripheral.\r\n");
        return 1;
    }

    if (address > 0x007FFFFF) {
        DEBUG_VV("ERROR: Accessing the address out of range.\r\n");
        return 1;
    }

    if (!data) {
        DEBUG_VV("ERROR: Unreferenced data buffer.\r\n");
        return 1;
    }

    if (data_length == 0) {
        DEBUG_VV("ERROR: Zero length data buffer.\r\n");
        return 1;
    }
    
    uint8_t status_register[1] = {0x00, };
    int ret;

    // Check the write enable (WEL) is 1
    do {
        ret = qspi_flash_write_enable();
        if (ret) {
            DEBUG_VV("ERROR: Failed to write enable command.\r\n");
            return 1;
        }

        HAL_Delay(1);

        ret = qspi_flash_read_status_register(status_register);
        if (ret) {
            DEBUG_VV("ERROR: Failed to read status register\r\n");
            return 1;
        }
    } while (!((status_register[0] & 0x02) == 0x02));

    QSPI_CommandTypeDef qspi_flash_program_page = {
        .Instruction = 0x38,
        .AddressMode = QSPI_ADDRESS_4_LINES,
        .Address = address,
        .AddressSize = QSPI_ADDRESS_24_BITS,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DummyCycles = 0,
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .DataMode = QSPI_DATA_4_LINES,
        .NbData = data_length,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };

    ret = HAL_QSPI_Command(_hqspi, &qspi_flash_program_page, HAL_MAX_DELAY);

    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: HAL_QSPI_Command status %d\r\n", ret);
        return ret;
    }

    ret = HAL_QSPI_Transmit(&hqspi, data, HAL_MAX_DELAY);

    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: HAL_QSPI_Transmit status %d\r\n", ret);
        return ret;
    }

    // Is write in Progress finished?
    do {
        ret = qspi_flash_read_status_register(status_register);
        if (ret) {
            DEBUG_VV("ERROR: Failed to read status register\r\n");
            return 1;
        }

        HAL_Delay(1);
    } while (!((status_register[0] & 0x01) == 0x00));

    ret = qspi_flash_read_security_register(status_register);
    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: Failed to read security register.\r\n");
        return ret;
    }
    
    if (((status_register[0] & 0x40) == 0x40) || ((status_register[0] & 0x20) == 0x20)) {
        DEBUG_VV("ERROR: Cannot successfully program or erase the chip.\r\n");
        return 1;
    }

    return 0;
}

int qspi_flash_erase_sector(uint32_t address)
{
    if (!_hqspi) {
        DEBUG_VV("ERROR: Unreferenced qspi peripheral.\r\n");
        return 1;
    }

    if (address > 0x007FFFFF) {
        DEBUG_VV("ERROR: Accessing the address out of range.\r\n");
        return 1;
    }
    
    uint8_t status_register[1] = {0x00, };
    int ret;

    // Check the write enable (WEL) is 1
    do {
        ret = qspi_flash_write_enable();
        if (ret) {
            DEBUG_VV("ERROR: Failed to write enable command.\r\n");
            return 1;
        }

        HAL_Delay(1);

        ret = qspi_flash_read_status_register(status_register);
        if (ret) {
            DEBUG_VV("ERROR: Failed to read status register\r\n");
            return 1;
        }
    } while (!((status_register[0] & 0x02) == 0x02));

    QSPI_CommandTypeDef qspi_flash_program_page = {
        .Instruction = 0x20,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DummyCycles = 0,
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .DataMode = QSPI_DATA_1_LINE,
        .NbData = 3,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };

    ret = HAL_QSPI_Command(_hqspi, &qspi_flash_program_page, HAL_MAX_DELAY);

    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: HAL_QSPI_Command status %d\r\n", ret);
        return ret;
    }

    uint8_t data[3] = {((address >> 16) & 0xFF), ((address >> 8) & 0xFF), (address & 0xFF)};

    ret = HAL_QSPI_Transmit(&hqspi, data, HAL_MAX_DELAY);

    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: HAL_QSPI_Transmit status %d\r\n", ret);
        return ret;
    }

    // Is write in Progress finished?
    do {
        ret = qspi_flash_read_status_register(status_register);
        if (ret) {
            DEBUG_VV("ERROR: Failed to read status register\r\n");
            return 1;
        }
        
        HAL_Delay(1);
    } while (!((status_register[0] & 0x01) == 0x00));

    ret = qspi_flash_read_security_register(status_register);
    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: Failed to read security register.\r\n");
        return ret;
    }
    
    if (((status_register[0] & 0x40) == 0x40) || ((status_register[0] & 0x20) == 0x20)) {
        DEBUG_VV("ERROR: Cannot successfully program or erase the chip.\r\n");
        return 1;
    }

    return 0;
}

int qspi_flash_erase_chip(void)
{
    if (!_hqspi) {
        DEBUG_VV("ERROR: Unreferenced qspi peripheral.\r\n");
        return 1;
    }
    
    uint8_t status_register[1] = {0x00, };
    int ret;

    // Check the write enable (WEL) is 1
    do {
        ret = qspi_flash_write_enable();
        if (ret) {
            DEBUG_VV("ERROR: Failed to write enable command.\r\n");
            return 1;
        }

        HAL_Delay(1);

        ret = qspi_flash_read_status_register(status_register);
        if (ret) {
            DEBUG_VV("ERROR: Failed to read status register\r\n");
            return 1;
        }
    } while (!((status_register[0] & 0x02) == 0x02));

    QSPI_CommandTypeDef qspi_flash_program_page = {
        .Instruction = 0x60,
        .AddressMode = QSPI_ADDRESS_NONE,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE,
        .DummyCycles = 0,
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .DataMode = QSPI_DATA_NONE,
        .NbData = 0,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };

    ret = HAL_QSPI_Command(_hqspi, &qspi_flash_program_page, HAL_MAX_DELAY);

    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: HAL_QSPI_Command status %d\r\n", ret);
        return ret;
    }

    int elapsed_time = 0;
    // Is write in Progress finished?
    do {
        ret = qspi_flash_read_status_register(status_register);
        if (ret) {
            DEBUG_VV("ERROR: Failed to read status register\r\n");
            return 1;
        }
        
        HAL_Delay(1000);
        DEBUG_VV("[INFO] Erasing the data (%ds elapsed)...\r\n", ++elapsed_time);
    } while (!((status_register[0] & 0x01) == 0x00));

    ret = qspi_flash_read_security_register(status_register);
    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: Failed to read security register.\r\n");
        return ret;
    }
    
    if (((status_register[0] & 0x40) == 0x40) || ((status_register[0] & 0x20) == 0x20)) {
        DEBUG_VV("ERROR: Cannot successfully program or erase the chip.\r\n");
        return 1;
    }

    return 0;
}

int qspi_flash_read_page(uint8_t *data, uint32_t data_length, uint32_t address)
{
    if (!_hqspi) {
        DEBUG_VV("ERROR: Unreferenced qspi peripheral.\r\n");
        return 1;
    }

    if (address > 0x007FFFFF) {
        DEBUG_VV("ERROR: Accessing the address out of range.\r\n");
        return 1;
    }

    if (!data) {
        DEBUG_VV("ERROR: Unreferenced data buffer.\r\n");
        return 1;
    }

    if (data_length == 0) {
        DEBUG_VV("ERROR: Zero length data buffer.\r\n");
        return 1;
    }

    QSPI_CommandTypeDef qspi_flash_program_page = {
        .Instruction = 0xEB,
        .AddressMode = QSPI_ADDRESS_4_LINES,
        .Address = address,
        .AddressSize = QSPI_ADDRESS_24_BITS,
        .AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES,
        .AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS,
        .AlternateBytes = 0xFF,
        .DummyCycles = 4,
        .InstructionMode = QSPI_INSTRUCTION_1_LINE,
        .DataMode = QSPI_DATA_4_LINES,
        .NbData = data_length,
        .DdrMode = QSPI_DDR_MODE_DISABLE,
        .SIOOMode = QSPI_SIOO_INST_EVERY_CMD,
    };

    int ret = HAL_QSPI_Command(_hqspi, &qspi_flash_program_page, HAL_MAX_DELAY);

    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: HAL_QSPI_Command status %d\r\n", ret);
        return ret;
    }

    ret = HAL_QSPI_Receive(_hqspi, data, HAL_MAX_DELAY);

    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: HAL_QSPI_Receive status %d\r\n", ret);
        return ret;
    }

    return 0;
}