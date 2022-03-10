#include "libmaix_disp.h"
#include "libmaix_disp_priv.h"
#include <string.h>

struct libmaix_disp * libmaix_disp_create(u_int8_t fbiopan)
{
    struct libmaix_disp *disp = (struct libmaix_disp*)malloc(sizeof(struct libmaix_disp));
    if(NULL == disp) {
        return NULL;
    }
    memset(disp, 0, sizeof(struct libmaix_disp));

    struct libmaix_disp_priv_t *priv = (struct libmaix_disp_priv_t *)malloc(sizeof(struct libmaix_disp_priv_t));
    if(NULL == priv) {
        free(disp);
        return NULL;
    }
    memset(priv, 0, sizeof(struct libmaix_disp_priv_t));

    disp->reserved = (void*)priv;

    if(disp_priv_init(disp) != 0) {
        // priv->fbiopan = fbiopan;
        libmaix_disp_destroy(&disp);
        return NULL;
    }

    return disp;
}

void libmaix_disp_destroy(struct libmaix_disp** disp)
{
    if(NULL == disp || *disp == NULL)
        return;

    struct libmaix_disp_priv_t *priv = (struct libmaix_disp_priv_t*)(*disp)->reserved;

    if(priv) {
        if(priv->devDeinit)
            priv->devDeinit(*disp);
        free(priv);
    }

    free(*disp);
    *disp = NULL;
}
