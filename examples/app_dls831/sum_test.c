
#include "stdio.h"
#include "stdint.h"

int main()
{
    uint8_t data[] = {
        0xAD, 0xFA, 0x03, 0xFF, 0x7F, 0x7F, 0x7F, 
    };

    uint8_t sum = 0;
    for (int i = 2; i < sizeof(data); i++)
    {
        sum += data[i];
    }

    printf("sum = %x\r\n", sum);

    return 0;
}