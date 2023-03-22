#ifndef __CLI_H__
#define __CLI_H__

#include <string.h>
#include "usart.h"

typedef struct {
    uint32_t head;
    uint32_t tail;
    int32_t size;
    int32_t capacity;
    char* buf;
} cli_input;

typedef cli_input* p_cli_input;

p_cli_input ring_buf_initialize(UART_HandleTypeDef* huart);
void ring_buf_deinitialize(void);
int dummy_handler(int argc, char* argv[]);

#endif /* __CLI_H__ */