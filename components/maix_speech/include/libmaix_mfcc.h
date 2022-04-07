
#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include "fvad.h"
#include <alsa/asoundlib.h>

typedef enum
{
    dls_state_error = -1,
    dls_state_idle,
    dls_state_ready,
    dls_state_study,
    dls_state_work,
    dls_state_finish,
    dls_state_destroy,
} dls_state;

typedef struct _dls_mfcc
{
    int id;
    snd_pcm_t *handle;
    dls_state state;
    pthread_t _thread;
    Fvad *vad;
} dls_mfcc;

int unit_test_mfcc();

#ifdef __cplusplus
}
#endif
