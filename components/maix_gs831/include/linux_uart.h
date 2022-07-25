#ifndef __LINUX_UART_H
#define __LINUX_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"

#define PRINF_HEX_ARR(str,buf,len)\
do{\
    char *buff = (char *)buf;\
    printf("\e[32m[%s](%d):\e[0m", str, len);\
    for (int i = 0;i < len; ++i)\
    {\
        printf("0x%.2X ", buff[i] & 0xff);\
    }\
    printf("\r\n");\
} while (0);

typedef struct{
	int baud;
	int data_bits;
	int stop_bits;
	char parity;
}uart_t;

int linux_uart_init(char* dev, void* param);
void linux_uart_deinit(int fd);
int linux_uart_read(int fd, int cnt, uint8_t* buf);
int linux_uart_write(int fd, int cnt, uint8_t* buf);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __LINUX_UART_H */
