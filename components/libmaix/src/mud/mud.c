#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include "mud.h"
#include "libmaix_nn.h"
#define MAX_LEN 5
#define debug_line //printf("%s:%d %s %s %s \r\n", __FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__)

char *libmaix_mud_get_key(char *line)
{
    char *strline = (char *)malloc(sizeof(char) * 1024);
    memcpy(strline, line, 1024);
    char *key = (char *)malloc(sizeof(char) * 32);
    memset(key,0,32);
    char *start, *end, *key_head;
    start = strline;
    end = strchr(strline, '=');
    key_head = key;
    while (start != end)
    {
        if (*start == ' ')
        {
            start++;
        }
        else
        {
            *key = *start;
            start++;
            key++;
        }
    }
    key =  '\0';
    free(strline);
    start = NULL;
    end = NULL;
    return key_head;
}

float *libmaix_mud_get_float_value(char *line)
{
    float *value = (float *)malloc(sizeof(float) * 10);
    float *value_head = value;
    char *start = strchr(line, '=') + 1;
    char *tmp = strchr(start, ',');
    while (*start != '\n')
    {
        char *single_value = (char *)malloc(sizeof(char) * 10);
        char *single_value_head = single_value;
        if (tmp != NULL)
        {

            for (; start != tmp; start++)
            {
                if (*start == ' ')
                    continue;
                *single_value = *start;
                single_value++;
            }
            *single_value = '\0';

            start++;

            sscanf(single_value_head , "%f", value);
            value++;
            free(single_value_head);
            tmp = strchr(start, ',');
        }
        else
        {
            while(*start != '\n')
            {
                if (*start == ' ')
                {
                    start++;
                    continue;
                }
                *single_value = *start;
                single_value++;
                start++;
            }
            * single_value = '\0';
            sscanf(single_value_head , "%f",value);
            value++;
            free(single_value_head);
        }
    }
    return value_head;
}

char *libmaix_mud_get_sting_value(char *line)
{
    char *strline = (char *)malloc(sizeof(char) * 1024);
    memset(strline,0,1024);
    if(strline == NULL)
    {
        printf("libmaix mud src : malloc strine buffer is faild\n");
        exit(0);
    }
    strcpy(strline , line);
    char *string_value = (char *)malloc(sizeof(char) * 1024);
    if(string_value ==  NULL)
    {
        printf("libmaix mud src : malloc strlinr value buffer is faild\n");
        exit(0);
    }
    memset(string_value , 0 ,1024);
    char *string_value_head = string_value;
    char *start = strchr(strline, '=') + 1;
    while (*start != '\n'  )
    {
        if (*start == ' ')
        {
            start++;
            continue;
        }
        *string_value = *start;
        string_value++;
        start++;
    }
    string_value = '\0';
    free(strline);
    return string_value_head;
}

void libmaix_mud_set_inputs_value(float **values, mud_info * mud_info_obj) // float**指向二维数组
{
    int input_num = mud_info_obj->input_num;
    for (int i = 0; i != mud_info_obj->input_num; i++)
    {
        for (int j = 0; j != 3; j++)
        {
            mud_info_obj->inputs_shape[i][j] = (int)values[i][j];
        }
        int h = (int)values[i][0];
        int w = (int)values[i][1];
        int c = (int)values[i][2];
        if (h != 1 && w != 1 && c == 3)
        {
            // mean
            mud_info_obj->mean[i][0] = values[i][3];
            mud_info_obj->mean[i][1] = values[i][4];
            mud_info_obj->mean[i][2] = values[i][5];

            // norm,
            mud_info_obj->norm[i][0] = values[i][6];
            mud_info_obj->norm[i][1] = values[i][7];
            mud_info_obj->norm[i][2] = values[i][8];
        }
        else if (h != 1 && w != 1 && c == 1)
        {
            // mean
            mud_info_obj->mean[i][0] = values[i][3];
            mud_info_obj->mean[i][1] = mud_info_obj->mean[i][0];
            mud_info_obj->mean[i][2] = mud_info_obj->mean[i][0];

            // norm
            mud_info_obj->norm[i][0] = values[i][4];
            mud_info_obj->norm[i][1] = mud_info_obj->norm[i][0];
            mud_info_obj->norm[i][2] = mud_info_obj->norm[i][0];
        }
        else if (h == 1 && w == 1 && c != 1)
        {
            // mean
            mud_info_obj->mean[i][0] = values[i][3];
            mud_info_obj->mean[i][1] = mud_info_obj->mean[i][0];
            mud_info_obj->mean[i][2] = mud_info_obj->mean[i][0];

            // norm
            mud_info_obj->norm[i][0] = values[i][4];
            mud_info_obj->norm[i][1] = mud_info_obj->norm[i][0];
            mud_info_obj->norm[i][2] = mud_info_obj->norm[i][0];
        }
    }
}

void libmaix_mud_set_outputs_value(float **values, mud_info * mud_info_obj)
{
    for (int i = 0; i != mud_info_obj->output_num; i++)
    {
        for (int j = 0; j != 3; j++)
        {
            mud_info_obj->outputs_shape[i][j] = (int)values[i][j];
        }
    }
}

void libmaix_mud_set_inputs_scale(float *values, mud_info * mud_info_obj)
{
    for (int i = 0; i != mud_info_obj->input_num; i++)
    {
        mud_info_obj->inputs_scale[i] = values[i];
    }
}

void libmaix_mud_set_outputs_scale(float *values, mud_info * mud_info_obj)
{
    for (int i = 0; i != mud_info_obj->output_num; i++)
    {
        mud_info_obj->ouputs_scale[i] = values[i];
    }
}

int libmaix_mud_get_section(FILE *fp, char *title, mud_info *mud_info_obj)
{
    int flag = 0;
    char string_title[64], string_lines[1024];
    sprintf(string_title, "[%s]", title);
    // input
    if (0 == strcmp(title, "inputs"))
    {
        int input_num = 0;
        //input names buffer
        char **inputs_name = (char **)malloc(sizeof(char *) * MAX_LEN);
        if(inputs_name == NULL)
        {
            printf("libmaix mud src : malloc inputs names  buffer  is faild\n");
            exit(0);
        }
        char **inputs_name_head = inputs_name;

        //input values buffer
        float **value = (float **)malloc(sizeof(float *) * MAX_LEN);
        if(value == NULL)
        {
            printf("libmaix mud src : malloc inputs values  buffer  is faild\n");
            exit(0);
        }
        float **value_head = value;

        while (!feof(fp))
        {
            fgets(string_lines, 1024, fp);
            if (0 == strncmp(string_title, string_lines, strlen(string_title)))
            {
                flag = 1;
                continue;
            }
            else if (flag == 1 && (NULL != strchr(string_lines, '=')))
            {
                *inputs_name = (char *)malloc(sizeof(char) * 1024);
                if(! *inputs_name)
                {
                    printf("libmaix mud src : malloc input name  buffer  is faild\n");
                    exit(0);
                }
                memset( *inputs_name,0,1024);
                char * name = libmaix_mud_get_key(string_lines);
                strcpy(* inputs_name , name);
                free(name);


                *value = (float *)malloc(sizeof(float) * 10);
                if(! *value)
                {
                    printf("libmaix mud src : malloc input values  buffer  is faild\n");
                    exit(0);
                }
                *value = libmaix_mud_get_float_value(string_lines);

                inputs_name++;
                value++;
                input_num++;
            }
            else if (strspn(string_lines, "\t\n") == strlen(string_lines))
            {
                flag = 0;
                continue;
            }
        }
        mud_info_obj->input_num = input_num;
        mud_info_obj->inputs = inputs_name_head;
        libmaix_mud_set_inputs_value(value_head , mud_info_obj);
        rewind(fp);
    }
    // output
    else if (0 == strcmp(title, "outputs"))
    {
        int output_num = 0;
        //outputs names buffer
        char **outputs_name = (char **)malloc(sizeof(char *) * MAX_LEN);
        if(outputs_name == NULL)
        {
            printf("libmaix mud src : malloc outputs names  buffer  is faild\n");
            exit(0);
        }
        char **outputs_name_head = outputs_name;

        // outputs values buffer
        float **value = (float **)malloc(sizeof(float *) * MAX_LEN);
        if(value == NULL)
        {
            printf("libmaix mud src : malloc outputs valuse  buffer  is faild\n");
            exit(0);
        }
        float **value_head = value;

        while (!feof(fp))
        {
            fgets(string_lines, 1024, fp);
            if (0 == strncmp(string_title, string_lines, strlen(string_title)))
            {
                flag = 1;
                continue;
            }
            else if (flag == 1 && (NULL != strchr(string_lines, '=')))
            {
                *outputs_name = (char *)malloc(sizeof(char) *1024);
                if(! *outputs_name)
                {
                    printf("libmaix mud src : malloc output name  buffer  is faild\n");
                    exit(0);
                }
                char * name = libmaix_mud_get_key(string_lines);
                strcpy(* outputs_name , name);
                free(name);


                *value = (float *) malloc(sizeof(float)*10);
                if(! *value)
                {
                    printf("libmaix mud src : malloc output values  buffer  is faild\n");
                    exit(0);
                }
                float * tmp_value = libmaix_mud_get_float_value(string_lines);
                memset( *value , 0 ,10);
                memcpy( *value , tmp_value ,10 * sizeof(float));
                free(tmp_value);

                outputs_name++;
                value++;
                output_num++;
            }
            else if (strspn(string_lines, " \t\n") == strlen(string_lines))
            {
                flag = 0;
                continue;
            }
        }
        mud_info_obj->output_num = output_num;
        mud_info_obj->outpus = outputs_name_head;
        libmaix_mud_set_outputs_value(value_head , mud_info_obj);
        rewind(fp);
    }
    // basic extra and decoder
    else if (0 == strcmp(title, "basic"))
    {
        while (!feof(fp))
        {
            fgets(string_lines, 1024, fp);

            if (0 == strncmp(string_title, string_lines, strlen(string_title)))
            {

                flag = 1;
                continue;
            }
            else if (flag == 1 && (NULL != strchr(string_lines, '=')))
            {
                char *key = libmaix_mud_get_key(string_lines);
                char *value = libmaix_mud_get_sting_value(string_lines);

                if (0 == strcmp(key, "type")) // input scale
                {
                    // value is a buffer
                    printf("type:%s\n", value);
                    mud_info_obj->model_type = value;
                }
                if (0 == strcmp(key, "bin"))
                {
                    printf("bin:%s\n", value);
                    if(*value == '/')
                    {
                        mud_info_obj->bin_path = value;
                    }
                    else
                    {
                        char * full_path = (char *)malloc(sizeof(char) * 1024);
                        memset(full_path , 0, 1024);
                        char * pre = full_path;
                        char * end = strrchr(mud_info_obj->mud_file_path ,'/');
                        char * start = mud_info_obj->mud_file_path;
                        while( start <= end)
                        {
                            *pre ++ = *start++;
                        }
                        *pre =  '\0';
                        strcat(full_path , value);
                        mud_info_obj->bin_path = full_path;
                    }
                }
                if (0 == strcmp(key, "param"))
                {
                    printf("param:%s\n",value);
                    if(*value == '/')
                    {
                        mud_info_obj->param_path = value;
                    }
                    else
                    {
                        char * full_path = (char *)malloc(sizeof(char) * 1024);
                        char * pre = full_path;
                        char * mud_workspace = (char *)malloc(sizeof(char) * 1024);
                        memset(full_path , 0, 1024);
                        memset(mud_workspace , 0, 1024);
                        char * end = strrchr(mud_info_obj->mud_file_path ,'/');
                        char * start = mud_info_obj->mud_file_path;
                        while( start <= end)
                        {
                            *pre ++ = *start++;
                        }
                        *pre =  '\0';
                        strcat(full_path , value);
                        mud_info_obj->param_path = full_path;
                    }
                }
            }
            else if (strspn(string_lines, " \t\n") == strlen(string_lines))
            {
                flag = 0;
                continue;
            }
        }
        rewind(fp);
    }
    // sacle
    else if (0 == strcmp(title, "extra"))
    {
        while (!feof(fp))
        {
            fgets(string_lines, 1024, fp);
            if (0 == strncmp(string_title, string_lines, strlen(string_title)))
            {
                flag = 1;
                continue;
            }
            else if (flag == 1 && (NULL != strchr(string_lines, '=')))
            {
                char *key = libmaix_mud_get_key(string_lines);
                float *value = libmaix_mud_get_float_value(string_lines);

                int count = 0;
                if ( 0 == strcmp( key ,  "inputs_scale"))  // input scale
                {

                    for(int i = 0 ; i != mud_info_obj->input_num ; i++)
                    {
                        mud_info_obj->inputs_scale[i]  =value[i];
                    }
                }
                else
                {

                    for(int j = 0 ; j != mud_info_obj->output_num ; j++)
                    {
                        mud_info_obj->ouputs_scale[j] = value[j];
                    }
                }
            }
            else if (strspn(string_lines, " \t\n") == strlen(string_lines))
            {
                flag = 0;
                continue;
            }
        }
        rewind(fp);
    }
}

void libmaix_mud_read_mud_file(char * mud_path ,  mud_info * mud_info_obj)
{
    FILE * fp;
    if(* mud_path == '/')
    {
        mud_info_obj->mud_file_path = (char *) malloc(sizeof(char) * 1024);
        if(! mud_info_obj->mud_file_path)
        {
            printf("libmaix mud src : init mud file path is faild\n");
            exit(0);
        }
        strcpy(mud_info_obj->mud_file_path , mud_path);
        if(NULL == ( (fp = fopen(mud_info_obj->mud_file_path, "r"))))
        {
            perror("libmaix mud src : can't open the specifically mud file\n ");
            exit(0);
        }
    }
    else
    {
        if((mud_info_obj->mud_file_path = getcwd(NULL,0))==NULL){
            perror("libmaix mud src : getcwd error\n");
            exit(0);
        }
        strcat(mud_info_obj->mud_file_path , "/");
        strcat(mud_info_obj->mud_file_path,mud_path);
        if (NULL == (fp = fopen(mud_info_obj->mud_file_path, "r")))
        {
            perror("libmaix mud src : can't open the specifically mud file\n");
            exit(0);
        }
    }
    libmaix_mud_get_section(fp , "basic", mud_info_obj);
    libmaix_mud_get_section(fp, "inputs", mud_info_obj);
    libmaix_mud_get_section(fp , "outputs", mud_info_obj);
    libmaix_mud_get_section(fp , "extra", mud_info_obj);
    fclose(fp);

}

mud_info * libmaix_mud_load_mud(char *mud_path)
{
    //new a mud info object
    mud_info * mud_info_obj = (mud_info *)malloc(sizeof(mud_info));
    if(! mud_info_obj)
    {
        printf("libmaix mud src : init mud object is faild\n");
        exit(0);
    }

    // check mud path is usable
    if ( 0 != access(mud_path , 0))
    {
        printf("libmaix mud src : the mud file path is not exist\n");
        exit(0);
    }

    //read mud file
    libmaix_mud_read_mud_file(mud_path , mud_info_obj);

    mud_info_obj->is_init = true;
    return mud_info_obj;
}

libmaix_nn_t* libmaix_mud_build_model(mud_info * mud_info_obj ,libmaix_nn_model_path_t * path, libmaix_nn_opt_param_t *opt)
{
    libmaix_nn_t* nn = NULL;
    libmaix_err_t err =LIBMAIX_ERR_NONE;

    if(strcmp(mud_info_obj->model_type , "aipu") == 0)
    {
        if(strlen(mud_info_obj->bin_path) == 0)
        {
            printf("this path is empty ! \n");
        }
        //path
        path->aipu.model_path = mud_info_obj->bin_path;
        // opt
        opt->aipu.input_names = mud_info_obj->inputs;
        opt->aipu.output_names = mud_info_obj->outpus;
        opt->aipu.input_num = mud_info_obj->input_num;
        opt->aipu.output_num = mud_info_obj->output_num;
        for(int i=0 ; i !=3 ; i++ )
        {
            opt->aipu.mean[i] = mud_info_obj->mean[0][i];
            opt->aipu.norm[i] = mud_info_obj->norm[0][i];
        }
        for (int i =0 ; i != mud_info_obj->output_num ; i++)
        {
            opt->aipu.scale[i] = mud_info_obj->ouputs_scale[i];
        }

    }
    else if (strcmp(mud_info_obj->model_type , "awnn") == 0)
    {
        if(strlen(mud_info_obj->bin_path) == 0  ||  strlen(mud_info_obj->param_path)==0)
        {
            printf("this path is empty ! \n");

        }
        //input && output num && enrypt

        opt->awnn.input_num = mud_info_obj->input_num;
        opt->awnn.output_num = mud_info_obj->output_num;
        opt->awnn.encrypt = false;
        //path
        int bin_len = strlen(mud_info_obj->bin_path);
        char *bin_src = mud_info_obj->bin_path;
        char *bin_dst = (char *)malloc(bin_len +1);
        if(bin_dst)
        {
            memcpy(bin_dst, bin_src, bin_len);
            bin_dst[bin_len] = '\0';
            path->awnn.bin_path = bin_dst;
        }

        int param_len = strlen(mud_info_obj->param_path);
        char *param_src = mud_info_obj->param_path;
        char *param_dst = (char *)malloc(param_len +1);
        if(param_dst)
        {
            memcpy(param_dst, param_src, param_len);
            param_dst[param_len] = '\0';
            path->awnn.param_path = param_dst;
        }

        opt->awnn.input_names = (char **)malloc(sizeof(char*) * mud_info_obj->input_num);
        for(int i=0 ; i !=opt->awnn.input_num;i++)
        {
            int len = strlen(mud_info_obj->inputs[i])+1;
            char *src = mud_info_obj->inputs[i];
            printf("input id:%d  inputs:%s\n", i, src);
            char *dst =  (char*)malloc(len);
            if (dst) {
                strcpy(dst, src);
                opt->awnn.input_names[i] = dst;
            }
        }
        opt->awnn.output_names = (char **)malloc(sizeof(char*) *mud_info_obj->output_num);
        for(int i=0 ; i !=opt->awnn.output_num;i++)
        {
            int len = strlen(mud_info_obj->outpus[i]) +1;
            char * src = mud_info_obj->outpus[i];
            printf("output id:%d outputs:%s\n", i , src);
            char *dst = (char *)malloc(len);
            if(dst)
            {
                strcpy(dst, src);
                opt->awnn.output_names[i] =  dst;
            }
        }

        //mean & norm
        for(int i=0 ; i !=3 ; i++ )
        {
            opt->awnn.mean[i] = mud_info_obj->mean[0][i];
            opt->awnn.norm[i] = mud_info_obj->norm[0][i];
        }
        for (int i =0 ; i != 3 ; i++)
        {
            printf("mean%d : %f \n", i , opt->awnn.mean[i]);
            printf("norm%d : %f \n", i , opt->awnn.norm[i]);
        }
    }
    else
    {
        printf("this type value is empty or the type is unsupport !\n");
    }

    // nn create
    nn = libmaix_nn_create();
    if(!nn)
    {
        printf("libmaix_nn object create fail\n");
    }
    err = nn->init(nn);

    if(err != LIBMAIX_ERR_NONE)
    {
        printf("libmaix_nn init fail: %s\n", libmaix_get_err_msg(err));
    }
    err = nn->load(nn, path, opt);
    if(err != LIBMAIX_ERR_NONE)
    {
        printf("libmaix_nn load fail: %s\n", libmaix_get_err_msg(err));
    }
    return nn;
}

void libmaix_mud_deinit_mud(mud_info * mud_info_obj)
{
    if(mud_info_obj->is_init)
    {

        free(mud_info_obj->inputs);
        free(mud_info_obj->outpus);
        mud_info_obj->inputs = NULL;
        mud_info_obj->outpus = NULL;
        free(mud_info_obj->model_type);
        free(mud_info_obj->bin_path);
        if(mud_info_obj->param_path)
        {
            free(mud_info_obj->param_path);
        }
        free(mud_info_obj->mud_file_path);
    }
}

libmaix_nn_t * libmaix_mud_load_model(char *mud_path)
{
    mud_info * mud_info_obj  = libmaix_mud_load_mud(mud_path);
    libmaix_nn_model_path_t model_path;
    libmaix_nn_opt_param_t opt_param;
    libmaix_nn_t * nn  = libmaix_mud_build_model(mud_info_obj, &model_path,&opt_param);
    return nn;
}

#ifdef __cplusplus
}
#endif