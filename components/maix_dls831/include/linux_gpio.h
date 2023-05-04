
#ifndef __LINUX_GPIO_H
#define __LINUX_GPIO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void _pwm_init(char *pin, uint64_t fre, float duty);

void _pwm_set_duty(char * pin, float duty);

void _pwm_deinit(char *pin);

/*
_pwm_init("PH8", 50000, 0.1);
sleep(1);
_pwm_set_duty("PH8", 0.6);
sleep(1);
_pwm_set_duty("PH8", 0.3);
sleep(1);
_pwm_deinit("PH8");
*/

void* _gpio_init(char* pin, int mode, int state);

void _gpio_deinit(char* pin);

void _gpio_read(char* pin, int* state);

void _gpio_write(char* pin, int state);

/*
// _gpio_init("PH7", 0, 0);
// for (int i = 0; i < 30; i++)
// {
//     int val = 0;
//     _gpio_read("PH7", &val);
//     printf("%d\r\n", val);
//     sleep(1);
// }
// _gpio_deinit("PH7");
// return;
*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __LINUX_GPIO_H */
