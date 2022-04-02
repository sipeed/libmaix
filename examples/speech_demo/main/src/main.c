
#include <time.h>
#include <signal.h>

#include "opts.h"
#include "ms_asr.h"

extern void decoder_kws_test(void);
extern void am_infer_test(void);
extern void mic_test(void);

#define DBG_LINE()  //printf("### L%d\n", __LINE__)
#define DBG(format, ...) printf(format, __VA_ARGS__)

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

//int font_draw(uint8_t* fb, int lcd_w, int lcd_h, int char_size, int x_oft, int y_oft, int c_color, int bg_color, char* str);
/*int fb_display(unsigned char *rgbbuff, unsigned char * alpha,
               unsigned int x_size, unsigned int y_size,
               unsigned int x_pan, unsigned int y_pan,
               unsigned int x_offs, unsigned int y_offs) */
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
    for(int i=0; i<len; i++){
        printf("\tkw%d: %.3f;", i, p[i]);
        if(p[i] > maxp){
            maxp = p[i];
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

        opts.sfst_name = "/root/test_files/lmS/lg_6m.sfst";
        opts.sym_name = "/root/test_files/lmS/lg_6m.sym";
        opts.phones_txt = "/root/test_files/lmS/phones.bin";
        opts.words_txt = "/root/test_files/lmS/words_utf.bin";
        opts.beam = 8.0;
        opts.bg_prob = 10.0;
        opts.scale = 0.5;

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

int main(int argc, char const* argv[])
{
	printf("MaixSense V831 ASR Demo\n");

    signal(SIGINT, my_handler);

    int res = 0;

    opts.device_type = DEVICE_MIC;
    opts.device_name = "hw:0,0";
    opts.model_name = "/root/test_files/cnnctc/cnnctc_3332_192";
    opts.model_in_len = 192;
    opts.strip_l = 6;
    opts.strip_r = 6;
    opts.phone_type = CN_PNYTONE;
    opts.agc = 1;

    am_args_t am_args = {opts.model_name, opts.model_in_len, opts.strip_l, opts.strip_r, opts.phone_type, opts.agc};
    res = ms_asr_init(opts.device_type, opts.device_name, &am_args, 0x00);
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

 #if 1

    opts.do_dig = 1;
    opts.do_kws = 1;
    opts.do_lvcsr = 1;

    if(opts.do_dig) set_dig(1);
    if(opts.do_kws) set_kws(1);
    if(opts.do_lvcsr) set_lvcsr(1);
    //
    int last = time(NULL);
    while(!exit_flag) {
        int frames = ms_asr_run(1); //1 frame default 768ms(am_xxyy_192)
        if(frames<1) {
            printf("run out\n");
            break;
        }
        int now = time(NULL);
        if (now > (last + 6)) {
            last = now;
            ms_asr_clear();
        }
        // if(key_read()==1) {
        //     printf("Reset ASR state!\n");
        //     lcd_clear1();
        //     ms_asr_clear();
        // }
    }
#else
    // lcd_clear();
    set_dig(1);
    while(!exit_flag) {
        int frames = ms_asr_run(1); //1 frame default 768ms(am_xxyy_192)֡
        if(frames<1) {
            printf("run out\n");
            break;
        }
        if(key_read()==1) {
            printf("Change ASR state!\n");
            demo_idx+=1;
            demo_idx = demo_idx%3;printf("000");
            if(demo_idx==0){
                set_dig(1); set_kws(0); set_lvcsr(0);
            } else if(demo_idx==1){
                set_dig(0); set_kws(1); set_lvcsr(0);
            }else{
                set_dig(0); set_kws(0); set_lvcsr(1);
            }
        }
    }
#endif
    printf("Done~~~\n");
    //deinit resource
free_decoder:
    ms_asr_deinit();
	return res;
}
