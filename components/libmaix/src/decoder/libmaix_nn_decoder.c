

#ifdef __cplusplus
extern "C" {
#endif


#include "libmaix_nn_decoder.h"
#include "stdlib.h"
#include "string.h"

libmaix_nn_decoder_t* libmaix_nn_decoder_creat(libmaic_nn_decoder_init_func_t init_func, libmaic_nn_decoder_deinit_func_t deinit_func,
                                                libmaic_nn_decoder_decode_func_t decode_func)
{
    libmaix_nn_decoder_t* obj = (libmaix_nn_decoder_t*)malloc(sizeof(libmaix_nn_decoder_t));
    if(!obj)
    {
        return NULL;
    }
    memset(obj, 0, sizeof(libmaix_nn_decoder_t)); //init space
    obj->init = init_func;
    obj->deinit = deinit_func;
    obj->decode = decode_func;

    return obj;
}

void libmaix_nn_decoder_destroy(libmaix_nn_decoder_t** obj)
{
    if(*obj)
    {
        (*obj)->deinit(*obj);
        free(*obj);
        *obj = NULL;
    }
}

#ifdef __cplusplus
}
#endif
