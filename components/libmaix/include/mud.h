#ifndef __INI_H__
#define __INI_H__
#include "libmaix_nn.h"
#define INI_VERSION "0.1.1"
#define MAX_LEN 5
#ifdef __cplusplus
extern "C" {
#endif
typedef struct ini_info
{
    /* data */
    char**inputs;
    char **outpus;
    char *model_type;
    char *bin_path;
    char *param_path;
    float mean[MAX_LEN][3];
    float norm[MAX_LEN][3];
    float inputs_scale[MAX_LEN];
    float ouputs_scale[MAX_LEN];
    int input_num;
    int output_num;
    int  outputs_shape[MAX_LEN][3];
    int inputs_shape[MAX_LEN][3];
}ini_info_t;

FILE * load_file(char *filename);
char * get_key(char * line);
float * get_float_value(char *line);
char * get_sting_value(char *line);
void set_inputs_value(float ** values, ini_info_t * ini_info);
void set_outputs_value(float **values, ini_info_t *ini_info);
void set_inputs_scale(float * values , ini_info_t * ini_info);
void set_outputs_scale(float * values , ini_info_t * ini_info);
int get_section(FILE * fp , char *title , ini_info_t * ini_info);
void read_file(char * mdsc_path , ini_info_t * ini_info_ptr);
libmaix_nn_t* load_mdsc(char * path , ini_info_t * info_ptr);
// libmaix_nn_t* build_model(ini_info_t * info_ptr);
libmaix_nn_t* build_model(ini_info_t * info_ptr ,libmaix_nn_model_path_t * path, libmaix_nn_opt_param_t *opt);
#ifdef __cplusplus
}
#endif
#endif