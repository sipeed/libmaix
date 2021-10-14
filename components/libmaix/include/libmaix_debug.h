/**
 * maix lib, debug header
 * 
 * @copyright © 2020-2021 Sipeed Ltd, All rights reserved
 * @author neucrack
 * @update --2021.1.7--neucrack: create lib
 *         --
 */

#ifndef __LIBMAIX_DEBUG_H__
#define __LIBMAIX_DEBUG_H__

#include "libmaix_err.h"
#include "assert.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DEBUG
    #define DEBUG 0
#endif

#if DEBUG // from makefile environment variable
    #define LIBMAIX_INFO_PRINTF(fmt, ...) printf("{%s:%d}[INFO:%s]( " fmt " )\r\n" , __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
    #define LIBMAIX_DEBUG_PRINTF(fmt, ...) printf("{%s:%d}[DEBUG:%s]( " fmt " )\r\n" , __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
    #define LIBMAIX_INFO_PRINTF(fmt, ...) printf("{%s:%d}[INFO:%s]( " fmt " )\r\n" , __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
    #define LIBMAIX_DEBUG_PRINTF(fmt, ...)
#endif

#define LIBMAIX_ERROR_PRINTF(fmt, ...) do{ \
        printf("[ERROR] libmaix: "); \
        printf(fmt, ##__VA_ARGS__); \
    }while(0)
#define LIBMAIX_IMAGE_ERROR(err_num) LIBMAIX_ERROR_PRINTF("%s\n", libmaix_get_err_msg(err_num))

#ifdef __cplusplus
}
#endif

#endif

