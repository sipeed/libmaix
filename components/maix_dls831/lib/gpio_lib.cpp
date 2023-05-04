/*
 * gpio_lib.c
 *
 * Copyright 2013 Stefan Mavrodiev <support@olimex.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>

#include "gpio_lib.hpp"

unsigned int SUNXI_PIO_BASE = 0;
static volatile long int *gpio_map = NULL;

int sunxi_gpio_init(void)
{
  int fd;
  unsigned int addr_start, addr_offset;
  unsigned int PageSize, PageMask;

  fd = open("/dev/mem", O_RDWR);
  if (fd < 0)
  {
    return SETUP_DEVMEM_FAIL;
  }

  PageSize = sysconf(_SC_PAGESIZE);
  PageMask = (~(PageSize - 1));

  addr_start = SW_PORTC_IO_BASE & PageMask;
  addr_offset = SW_PORTC_IO_BASE & ~PageMask;

  gpio_map = (long int *)mmap(0, PageSize * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, addr_start);
  if (gpio_map == MAP_FAILED)
  {
    return SETUP_MMAP_FAIL;
  }

  SUNXI_PIO_BASE = (unsigned int)gpio_map;
  SUNXI_PIO_BASE += addr_offset;

  close(fd);
  return SETUP_OK;
}

int sunxi_gpio_set_cfgpin(unsigned int pin, unsigned int val)
{

  unsigned int cfg;
  unsigned int bank = GPIO_BANK(pin);
  unsigned int index = GPIO_CFG_INDEX(pin);
  unsigned int offset = GPIO_CFG_OFFSET(pin);

  if (SUNXI_PIO_BASE == 0)
  {
    return -1;
  }

  struct sunxi_gpio *pio =
      &((struct sunxi_gpio_reg *)SUNXI_PIO_BASE)->gpio_bank[bank];

  cfg = *(&pio->cfg[0] + index);
  cfg &= ~(0xf << offset);
  cfg |= val << offset;

  *(&pio->cfg[0] + index) = cfg;

  return 0;
}

int sunxi_gpio_get_cfgpin(unsigned int pin)
{

  unsigned int cfg;
  unsigned int bank = GPIO_BANK(pin);
  unsigned int index = GPIO_CFG_INDEX(pin);
  unsigned int offset = GPIO_CFG_OFFSET(pin);
  if (SUNXI_PIO_BASE == 0)
  {
    return -1;
  }
  struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *)SUNXI_PIO_BASE)->gpio_bank[bank];
  cfg = *(&pio->cfg[0] + index);
  cfg >>= offset;
  return (cfg & 0xf);
}
int sunxi_gpio_output(unsigned int pin, unsigned int val)
{

  unsigned int bank = GPIO_BANK(pin);
  unsigned int num = GPIO_NUM(pin);

  if (SUNXI_PIO_BASE == 0)
  {
    return -1;
  }
  struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *)SUNXI_PIO_BASE)->gpio_bank[bank];

  if (val)
    *(&pio->dat) |= 1 << num;
  else
    *(&pio->dat) &= ~(1 << num);

  return 0;
}

int sunxi_gpio_input(unsigned int pin)
{

  unsigned int dat;
  unsigned int bank = GPIO_BANK(pin);
  unsigned int num = GPIO_NUM(pin);

  if (SUNXI_PIO_BASE == 0)
  {
    return -1;
  }

  struct sunxi_gpio *pio = &((struct sunxi_gpio_reg *)SUNXI_PIO_BASE)->gpio_bank[bank];

  dat = *(&pio->dat);
  dat >>= num;

  return (dat & 0x1);
}
void sunxi_gpio_cleanup(void)
{
  unsigned int PageSize;
  if (gpio_map == NULL)
    return;

  PageSize = sysconf(_SC_PAGESIZE);
  munmap((void *)gpio_map, PageSize * 2);
}

int unit_test_main()
{
  printf("main\r\n");

  sunxi_gpio_init();

  sunxi_gpio_set_cfgpin(238, 0x001); // PH14 OUPUT

  sunxi_gpio_output(238, 1);
  printf("1\r\n");

  usleep(1000*1000);

  sunxi_gpio_output(238, 0);
  printf("0\r\n");

  usleep(1000*1000);

  sunxi_gpio_output(238, 1);
  printf("1\r\n");

  sunxi_gpio_cleanup();
}
