
#include "ms_asr.h"

int main()
{
    int res = 0;
    int demo_idx = 0;
    //parse opts
    char* cfg_file = NULL;

    am_args_t am_args = {};

    res = ms_asr_init("", "", &am_args, 0);

    if(res != 0) {
        printf("ms_asr_init error!\n");
        return -1;
    }

    //Main Loop
    if(1) {
        res = ms_asr_decoder_cfg(DECODER_RAW, NULL , NULL, 0);
        if(res != 0) {printf("DECODER_RAW init error!\n");goto free_decoder;};
    }

    while(1) {
        int frames = ms_asr_run(1); //1 frame default 768ms(am_xxyy_192)
        if(frames<1) {
            printf("run out\n");
            break;
        }
        // if(key_read()==1) {
        //     printf("Reset ASR state!\n");
        //     ms_asr_clear();
        // }
    }

    printf("Done~~~\n");
    //deinit resource
free_decoder:
    ms_asr_deinit();
    return res;

    // libms_unit_test();

    // {
    //     void *tmp = ms_asr_init;
    // }
    // {
    //     void *tmp = ms_asr_decoder_cfg;
    // }
}
