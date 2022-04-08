
#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include "fvad.h"
#include <alsa/asoundlib.h>

//Die Anzahl der Werte in jedem Rahmen
#define N 512

//Die Anzahl der Mel-Filter
#define N_FILTER 26

//Die Anzahl der MFCC-Merkmale
#define N_MFCC N_FILTER

/*
* Struktur, die einen Rahmen darstellt.
* Die Werte im Rahmen sind die Intensitaeten der Darstellung der Funktion im Frequenzbereich.
* (double) magnitudes	Die Intensitaeten der Fourier-transformierten Werte.
*/
typedef
    struct
    {
        double magnitudes[N];
    } frame;

/*
* Struktur, die die MFCC-Merkmale eines Rahmens aufnimmt.
* (double) features	Die errechneten MFCC-Merkmale
*/
typedef
    struct
    {
        double features[N_MFCC];
    } mfcc_frame;

//Ueberschneidung der Rahmen
#define OVERLAP (N / 8)

typedef
    struct _word
    {
        mfcc_frame *frames;
        int n, id;
        struct _word *next;
    } word;

typedef enum
{
    dls_state_error = -1,
    dls_state_init,
    dls_state_idle,
    dls_state_ready,
    dls_state_work,
    dls_state_finish,
    dls_state_result,
    dls_state_save,
    dls_state_destroy,
} dls_state;

typedef struct _dls_mfcc
{
    snd_pcm_t *handle;
    dls_state state;
    pthread_t _thread;
    pthread_attr_t tattr;
    void *stack;
    Fvad *vad;
    frame *frames;
    int frame_n, frame_id;
    float frame_min, frame_max;
    mfcc_frame *mfcc_frames;
    word *words;
    int words_size;
} dls_mfcc;

int unit_test_mfcc();

#ifdef __cplusplus
}
#endif
