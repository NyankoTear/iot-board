#include "cli.h"

#define CHAR_INPUT_LIMIT 1024

static char inputs[CHAR_INPUT_LIMIT] = {0x00, };
static cli_input cli_buf;
static p_cli_input p_cli_buf = NULL;

uint8_t uart_buf[1] = {0x00};
UART_HandleTypeDef* _huart = NULL;

p_cli_input ring_buf_initialize(UART_HandleTypeDef* huart)
{
    if (!huart) {
        DEBUG_VV("ERROR: Undefined refecence `UART_HahdleTypeDef`.\r\n");
        return NULL;
    }
    _huart = huart;
    setvbuf(stdout, NULL, _IONBF, 0);

    p_cli_buf = &cli_buf;
    p_cli_buf->tail = -1;
    p_cli_buf->head = 0;
    p_cli_buf->size = 0;
    p_cli_buf->buf = inputs;
    p_cli_buf->capacity = CHAR_INPUT_LIMIT;

    HAL_StatusTypeDef ret = HAL_UART_Receive_IT(_huart, uart_buf, 1);
    if (ret != HAL_OK) {
        DEBUG_VV("ERROR: Failed to invoke HAL_UART_Receive_IT(), status: %d\r\n", ret);
        return NULL;
    }

    return p_cli_buf;
}

void ring_buf_deinitialize(void)
{
    _huart = NULL;
    p_cli_buf->buf = NULL;
    p_cli_buf = NULL;
}

static int is_ring_buf_empty(p_cli_input q) {
    if (q == NULL) {
        return -1;
    } else if (q->size == 0) {
        return 1;
    } else {
        return 0;
    }
}

static int is_ring_buf_full(p_cli_input q) {
    if (q == NULL) {
        return -1;
    } else if (q->size == q->capacity) {
        return 1;
    } else {
        return 0;
    }
}

static int ring_buf_queue(p_cli_input q, const char data) {
    if (q == NULL) {
        return -1;
    }

    if (is_ring_buf_full(q)) {
        return 0;
    }

    q->tail = (q->tail + 1) % q->capacity;
    q->buf[q->tail] = data;
    q->size++;

    return 1;
}

static char ring_buf_dequeue(p_cli_input q) {
    if (q == NULL) {
        return 0;
    }

    if (is_ring_buf_empty(q)) {
        return 0;
    }

    char data = q->buf[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->size--;

    return data;
}

static int ring_buf_clear(p_cli_input q) {
    if (q == NULL) {
        return 0;
    }

    p_cli_buf->tail = -1;
    p_cli_buf->head = 0;
    p_cli_buf->size = 0;

    return 1;
}

static int32_t ring_buf_get_size(p_cli_input q) {
    if (q == NULL) {
        return -1;
    }

    return q->size;
} 

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
    HAL_GPIO_TogglePin(USER_LED1_GPIO_Port, USER_LED1_Pin);
    
    if (p_cli_buf) {
        ring_buf_queue(p_cli_buf, uart_buf[0]);
        printf("%c", uart_buf[0]);
        if (uart_buf[0] == 0x0D) {
            printf("\r\necho: ");
            int32_t len = ring_buf_get_size(p_cli_buf);
            for (int i = 0; i < len; i++) {
                char data = ring_buf_dequeue(p_cli_buf);
                printf("%c", data);
            }
            printf("\r\n");
            ring_buf_clear(p_cli_buf);
        }
    }
    HAL_UART_Receive_IT(_huart, uart_buf, 1);
}