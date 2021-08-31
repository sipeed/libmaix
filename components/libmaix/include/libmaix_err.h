/**
 * libmaix_err.h, maix lib error enumerate
 * 
 * @copyright © 2020-2021 Sipeed Ltd, All rights reserved
 * @author neucrack
 * @update --2020.12.28--neucrack: create lib
 *         --
 */
#ifndef __LIBMAIX_ERR_H__
#define __LIBMAIX_ERR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    LIBMAIX_ERR_NONE   = 0,
    LIBMAIX_ERR_PARAM  = 1,
    LIBMAIX_ERR_NO_MEM = 2,
    LIBMAIX_ERR_NOT_IMPLEMENT = 3,
    LIBMAIX_ERR_NOT_READY     = 4,
    LIBMAIX_ERR_NOT_INIT      = 5,
    LIBMAIX_ERR_NOT_PERMIT    = 6,
    LIBMAIX_ERR_NOT_EXEC      = 7,
    LIBMAIX_ERR_UNKNOWN,
}libmaix_err_t;         // update this enum you MUST update `libmaix_err_strs` definition too

extern char* libmaix_err_strs[];

static inline char* libmaix_get_err_msg(libmaix_err_t err)
{
    return libmaix_err_strs[err];
}

#ifdef __cplusplus
}
#endif


#endif

