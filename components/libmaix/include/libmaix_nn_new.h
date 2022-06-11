#ifndef __LIBMAIX_NN_NEW_H__
#define __LIBMAIX_NN_NEW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "libmaix_nn.h"

libmaix_nn_t* libmaix_nn_create_from_mud(char *path);
libmaix_nn_t* libmaix_nn_create_from_raw(libmaix_nn_opt_param_t cfg);

#ifdef __cplusplus
}
#endif

#endif
