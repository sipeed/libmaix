
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "string.h"
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>

#include <linux/input.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

extern "C" {

    #include "opts.h"
    #define DR_WAV_IMPLEMENTATION
    #include "dr_wav.h"
    #include "benchmark.h"
    #include "lm_benchmark.h"
    #include "ms_asr.h"

    extern void decoder_kws_test(void);
    extern void am_infer_test(void);
    extern void mic_test(void);

    #define DBG_LINE()  //printf("### L%d\n", __LINE__)
    #define DBG(format, ...) printf(format, __VA_ARGS__)

    static int l_bpp = 24;
    volatile int exit_flag = 0;


    static int tick = 0;
    void my_rawcb(void* data, int len)  //data struct: pnyp_t data[len][BEAM_CNT]
    {
        pnyp_t* res = (pnyp_t*)data;
        printf("===================================\n");
        for(int t=0; t<len; t++) {
            pnyp_t* pp = res+BEAM_CNT*t;
            if(1){
                printf("T=%04d ====:", tick);
                tick += 1;
                for(int i=0; i < 3; i++) { //BEAM_CNT
                    printf("  %4d %-6s: %.3f;", pp[i].idx, am_vocab[pp[i].idx], ((float)(pp[i].p)));
                }
            }
            printf("\n");
        }
        printf("####\n");
        return;
    }

    /*
    int fb_display(unsigned char *rgbbuff, unsigned char * alpha,
                unsigned int x_size, unsigned int y_size,
                unsigned int x_pan, unsigned int y_pan,
                unsigned int x_offs, unsigned int y_offs);
    */
    //DIGIT_LOG_LEN
    #define DIG_H 32
    void my_digitcb(void* data, int len)
    {
        char* digit_res = (char*) data;
        char digit_res1[32];
        memset(digit_res1, ' ', 32);
        digit_res1[32-1]=0;
        memcpy(digit_res1, digit_res, strlen(digit_res));
        printf("digit_res1: %s\n", digit_res);

        return;
    }


    void my_lvcsrcb(void* data, int len)
    {
        char* words = ((char**)data)[0];
        char* pnys = ((char**)data)[1];
        printf("PNYS: %s\nHANS: %s\n", pnys, words);
        return ;
    }

    char* my_kw_tbl[3]={
        (char*)"xiao3 ai4 tong2 xue2",
        (char*)"tian1 mao1 jing1 ling2",
        (char*)"tian1 qi4 zen3 me yang4"
    };
    char* my_kw_str[3]={
        (char*)"XIAO AI TONG XU",
        (char*)"TIAN MAO JING L",
        (char*)"TIAN QI ZEN ME"
    };
    float my_kw_gate[3] = {0.1, 0.1, 0.1};
    void my_kwscb(void* data, int len)
    {
        float* p = (float*) data;
        float maxp = -1;
        int   maxi = 0;
        for(int i=0; i<len; i++){
            printf("\tkw%d: %.3f;", i, p[i]);
            if(p[i] > maxp){
                maxp = p[i];
                maxi = i;
            }
        }
        printf("\n");
        return ;
    }


    void my_handler(int s){
        printf("Caught signal %d\n",s);
        exit_flag = 1;
        return;
    }


    void set_dig(int flag)
    {
        size_t decoder_args[10];
        int res = 0;
        if(flag){ //init
            decoder_args[0] = 640;  //blank_t ms
            res = ms_asr_decoder_cfg(DECODER_DIG, my_digitcb , &decoder_args, 1);
            if(res != 0) {printf("DECODER_DIG init error!\n");};
        } else {
            ms_asr_decoder_cfg(DECODER_DIG, NULL , NULL, 0);
        }
        return;
    }

    void set_kws(int flag)
    {
        size_t decoder_args[10];
        int res = 0;
        if(flag){ //init
            decoder_args[0] = (size_t)my_kw_tbl;
            decoder_args[1] = (size_t)my_kw_gate;
            decoder_args[2] = 3;
            decoder_args[3] = 1;  //auto similar //自动去除音调 近音处理
            printf("qqqq");
            res = ms_asr_decoder_cfg(DECODER_KWS, my_kwscb , &decoder_args, 3);printf("aaaa");
            if(res != 0) {printf("DECODER_KWS init error!\n");goto out1;};
            char* similar_pnys0[1] = {(char*)"xiang3"};    //每个最多注册10个近音词
            ms_asr_kws_reg_similar((char*)"xiao3", similar_pnys0, 1);
            char* similar_pnys1[3] = {(char*)"xin1", (char*)"ting1", (char*)"jin1"};
            ms_asr_kws_reg_similar((char*)"jing1", similar_pnys1, 3);
        } else {
            ms_asr_decoder_cfg(DECODER_KWS, NULL , NULL, 0);
        }
    out1:
        return;
    }

    void set_lvcsr(int flag)
    {
        size_t decoder_args[10];
        int res = 0;
        if(flag){ //init

            decoder_args[0] = (size_t)opts.sfst_name;
            decoder_args[1] = (size_t)opts.sym_name;
            decoder_args[2] = (size_t)opts.phones_txt;
            decoder_args[3] = (size_t)opts.words_txt;
            memcpy(&decoder_args[4], &(opts.beam), sizeof(float));
            memcpy(&decoder_args[5], &(opts.bg_prob), sizeof(float));
            memcpy(&decoder_args[6], &(opts.scale), sizeof(float));
            decoder_args[7] = (size_t)opts.is_mmap;//printf("#\n");
            res = ms_asr_decoder_cfg(DECODER_LVCSR, my_lvcsrcb , &decoder_args, 8);
            if(res != 0) {printf("DECODER_LVCSR init error!\n");};
        } else {
            ms_asr_decoder_cfg(DECODER_LVCSR, NULL , NULL, 0);
        }
        return;
    }

    opts_t opts;

    int libms_unit_test()
    {
        printf("==============================================\n");
        printf("MaixSense R329 ASR Demo\n");
        printf("Developed by Sipeed.   \n");
        printf("Email: support@sipeed.com\n");
        printf("Usage: ./maix_asr cfg_file\n");
        printf("config file contains key:value every line\n");
        printf("==============================================\n\n");

        signal(SIGINT, my_handler);

        int res = 0;
        int demo_idx = 0;
        //parse opts
        char* cfg_file = NULL;
        // if(argc < 2) {
        // 	printf("argc=%d error\n", argc);
        // 	return -1;
        // }
        // cfg_file = (char*)argv[1];
        // if(parse_opts(cfg_file) != 0) {
        //     printf("parse_opts failed! check your cfg file\n");
        //     return -1;
        // }

        if(opts.testpath != NULL){
            // benchmark(opts.testpath, opts.testpny, opts.testhan);
            return 0;
        }
        if(opts.testlm != NULL){
            // lm_benchmark(opts.testlm);
            return 0;
        }

        //mic_test();return 0;

        //��ʼ��asr lib
        am_args_t am_args = {opts.model_name, opts.model_in_len, opts.strip_l, opts.strip_r, opts.phone_type, opts.agc};
        int dbg_flag = opts.dbg_micraw*DBG_MICRAW + opts.dbg_mic*DBG_MIC + \
            opts.dbg_strip*DBG_STRIP + opts.dbg_lvcsr*DBG_LVCSR +\
            opts.dbgt_pp*DBGT_PP + opts.dbgt_am*DBGT_AM +\
            opts.dbgt_kws*DBGT_KWS + opts.dbgt_wfst*DBGT_WFST;
        printf("dbg_flag = 0x%x\n", dbg_flag);
        res = ms_asr_init(opts.device_type, opts.device_name, &am_args, dbg_flag);

        if(res != 0) {
            printf("ms_asr_init error!\n");
            return -1;
        }

        //am_infer_test();
        //decoder_kws_test();
        //return 0;

        //Main Loop
        if(opts.do_raw) {
            res = ms_asr_decoder_cfg(DECODER_RAW, my_rawcb , NULL, 0);
            if(res != 0) {printf("DECODER_RAW init error!\n");goto free_decoder;};
        }

        if(opts.do_dig) set_dig(1);
        if(opts.do_kws) set_kws(1);
        if(opts.do_lvcsr) set_lvcsr(1);
        //
        while(!exit_flag) {
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
    }
    // int libms_unit_test() {
    //     return -1;
    // }
}