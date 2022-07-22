#ifndef __MUD_H__
#define __MUD_H__
#include "libmaix_nn.h"
#define INI_VERSION "0.1.1"
#define MAX_LEN 5
#ifdef __cplusplus
extern "C" {
#endif

typedef struct mud_info
{
    /* data */
    char**inputs;
    char **outpus;
    char *model_type;
    char *bin_path;
    char *param_path;
    char *mud_file_path;
    float mean[MAX_LEN][3];
    float norm[MAX_LEN][3];
    float inputs_scale[MAX_LEN];
    float ouputs_scale[MAX_LEN];
    int input_num;
    int output_num;
    int  outputs_shape[MAX_LEN][3];
    int inputs_shape[MAX_LEN][3];
    bool is_init;
}mud_info;

void libmaix_mud_deinit_mud(mud_info * mud_info_obj);
mud_info * libmaix_mud_load_mud(char * mud_path);
libmaix_nn_t * libmaix_mud_load_model(char *mud_path);
libmaix_nn_t* libmaix_mud_build_model(mud_info * mud_info_obj ,libmaix_nn_model_path_t * path, libmaix_nn_opt_param_t *opt);
void libmaix_mud_read_mud_file(char * mud_path ,  mud_info * mud_info_obj);
int libmaix_mud_get_section(FILE *fp, char *title, mud_info *mud_info_obj);
void libmaix_mud_set_inputs_scale(float *values, mud_info * mud_info_obj);
void libmaix_mud_set_outputs_scale(float *values, mud_info * mud_info_obj);
void libmaix_mud_set_inputs_value(float **values, mud_info * mud_info_obj);
void libmaix_mud_set_outputs_value(float **values, mud_info * mud_info_obj);
char *libmaix_mud_get_sting_value(char *line);
float *libmaix_mud_get_float_value(char *line);
char *libmaix_mud_get_key(char *line);


#ifdef __cplusplus
}
#endif
#endif