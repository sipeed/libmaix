#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include "mud.h"
#include "libmaix_nn.h"
#include "libmaix_err.h"
#define MAX_LEN 5

#define LIBMAIX_DEBUG_PRINTF(fmt, ...) printf("{%s:%d}[DEBUG:%s]( " fmt " )\r\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

    char *libmaix_mud_get_key(char *line)
    {
        char *strline = (char *)malloc(sizeof(char) * 128);
        memcpy(strline, line, 128);
        char *key = (char *)malloc(sizeof(char) * 32);
        memset(key, 0, 32);
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
        key = '\0';
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

                sscanf(single_value_head, "%f", value);
                value++;
                free(single_value_head);
                tmp = strchr(start, ',');
            }
            else
            {
                while (*start != '\n')
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
                *single_value = '\0';
                sscanf(single_value_head, "%f", value);
                value++;
                free(single_value_head);
            }
        }
        return value_head;
    }

    char *libmaix_mud_get_sting_value(char *line)
    {
        char *strline = (char *)malloc(sizeof(char) * 128);
        memset(strline, 0, 128);
        if (strline == NULL)
        {
            LIBMAIX_DEBUG_PRINTF("libmaix mud src : malloc strine buffer is faild\n");
        }
        strcpy(strline, line);
        char *string_value = (char *)malloc(sizeof(char) * 128);
        if (string_value == NULL)
        {
            LIBMAIX_DEBUG_PRINTF("libmaix mud src : malloc strlinr value buffer is faild\n");
        }
        memset(string_value, 0, 128);
        char *string_value_head = string_value;
        char *start = strchr(strline, '=') + 1;
        while (*start != '\n')
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

    void libmaix_mud_set_inputs_value(float **values, mud_info *mud_info_obj) // float**指向二维数组
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

    void libmaix_mud_set_outputs_value(float **values, mud_info *mud_info_obj)
    {
        for (int i = 0; i != mud_info_obj->output_num; i++)
        {
            for (int j = 0; j != 3; j++)
            {
                mud_info_obj->outputs_shape[i][j] = (int)values[i][j];
            }
        }
    }

    void libmaix_mud_set_inputs_scale(float *values, mud_info *mud_info_obj)
    {
        for (int i = 0; i != mud_info_obj->input_num; i++)
        {
            mud_info_obj->inputs_scale[i] = values[i];
        }
    }

    void libmaix_mud_set_outputs_scale(float *values, mud_info *mud_info_obj)
    {
        for (int i = 0; i != mud_info_obj->output_num; i++)
        {
            mud_info_obj->ouputs_scale[i] = values[i];
        }
    }

    libmaix_err_t libmaix_mud_get_section(FILE *fp, char *title, mud_info *mud_info_obj)
    {
        libmaix_err_t err = LIBMAIX_ERR_NONE;
        int flag = 0;
        char string_title[64], string_lines[128];
        sprintf(string_title, "[%s]", title);
        // input
        if (0 == strcmp(title, "inputs"))
        {
            int input_num = 0;

            // input names buffer
            char **inputs_name = (char **)malloc(sizeof(char *) * MAX_LEN);
            if (inputs_name == NULL)
            {
                LIBMAIX_DEBUG_PRINTF("libmaix mud src : malloc inputs names  buffer  is faild\n");
                err = LIBMAIX_ERR_NO_MEM;
                return err;
            }
            char **inputs_name_head = inputs_name;

            // input values buffer
            float **value = (float **)malloc(sizeof(float *) * MAX_LEN);
            if (value == NULL)
            {
                LIBMAIX_DEBUG_PRINTF("libmaix mud src : malloc inputs values  buffer  is faild\n");
                err = LIBMAIX_ERR_NO_MEM;
                return err;
            }
            float **value_head = value;

            while (!feof(fp))
            {
                fgets(string_lines, 128, fp);
                if (0 == strncmp(string_title, string_lines, strlen(string_title)))
                {
                    flag = 1;
                    continue;
                }
                else if (flag == 1 && (NULL != strchr(string_lines, '=')))
                {
                    // malloc input name
                    *inputs_name = (char *)malloc(sizeof(char) * 128);
                    if (!*inputs_name)
                    {
                        LIBMAIX_DEBUG_PRINTF("libmaix mud src : malloc input name  buffer  is faild\n");
                        err = LIBMAIX_ERR_NO_MEM;
                        if (inputs_name_head)
                        {
                            for (int i = 0; i != MAX_LEN; i++)
                            {
                                char *input_name = *(inputs_name_head + i);
                                if (input_name != NULL)
                                {
                                    free(input_name);
                                    input_name = NULL;
                                }
                            }
                            free(inputs_name_head);
                            inputs_name_head = NULL;
                        }
                        return err;
                    }
                    memset(*inputs_name, 0, 128);
                    // set input name
                    char *name = libmaix_mud_get_key(string_lines);
                    if (!name)
                    {
                        LIBMAIX_DEBUG_PRINTF("libmaix mud src : get input name is faild\n");
                        err = LIBMAIX_ERR_PARAM;
                        if (inputs_name_head)
                        {
                            for (int i = 0; i != MAX_LEN; i++)
                            {
                                char *input_name = *(inputs_name_head + i);
                                if (input_name != NULL)
                                {
                                    free(input_name);
                                    input_name = NULL;
                                }
                            }
                            free(inputs_name_head);
                            inputs_name_head = NULL;
                        }
                        return err;
                    }
                    // LIBMAIX_DEBUG_PRINTF("name : %s \n", name);
                    strcpy(*inputs_name, name);
                    free(name);
                    // malloc input value
                    *value = (float *)malloc(sizeof(float) * 10);
                    if (!*value)
                    {
                        LIBMAIX_DEBUG_PRINTF("libmaix mud src : malloc input values  buffer  is faild\n");
                        err = LIBMAIX_ERR_NO_MEM;
                        if (value_head)
                        {
                            for (int i = 0; i != MAX_LEN; i++)
                            {
                                float *input_value = *(value_head + i);
                                if (input_value != NULL)
                                {
                                    free(input_value);
                                    input_value = NULL;
                                }
                            }
                            free(value_head);
                            value_head = NULL;
                        }
                        return err;
                    }
                    // get input value
                    float *tmp_value = libmaix_mud_get_float_value(string_lines);
                    if (!tmp_value)
                    {
                        LIBMAIX_DEBUG_PRINTF("libmaix mud src : get input values  is faild\n");
                        err = LIBMAIX_ERR_NO_MEM;
                        if (value_head)
                        {
                            for (int i = 0; i != MAX_LEN; i++)
                            {
                                float *input_value = *(value_head + i);
                                if (input_value != NULL)
                                {
                                    free(input_value);
                                    input_value = NULL;
                                }
                            }
                            free(value_head);
                            value_head = NULL;
                        }
                        return err;
                    }
                    memset(*value, 0, 10);
                    memcpy(*value, tmp_value, 10 * sizeof(float));
                    free(tmp_value);
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
            libmaix_mud_set_inputs_value(value_head, mud_info_obj);
            rewind(fp);
            return err;
        }
        // output
        else if (0 == strcmp(title, "outputs"))
        {
            int output_num = 0;
            // outputs names buffer
            char **outputs_name = (char **)malloc(sizeof(char *) * MAX_LEN);
            if (outputs_name == NULL)
            {
                LIBMAIX_DEBUG_PRINTF("libmaix mud src : malloc outputs names  buffer  is faild\n");
                err = LIBMAIX_ERR_NO_MEM;
                return err;
            }
            char **outputs_name_head = outputs_name;

            // outputs values buffer
            float **value = (float **)malloc(sizeof(float *) * MAX_LEN);
            if (value == NULL)
            {
                LIBMAIX_DEBUG_PRINTF("libmaix mud src : malloc outputs valuse  buffer  is faild\n");
                err = LIBMAIX_ERR_NO_MEM;
                return err;
            }
            float **value_head = value;

            while (!feof(fp))
            {
                fgets(string_lines, 128, fp);
                if (0 == strncmp(string_title, string_lines, strlen(string_title)))
                {
                    flag = 1;
                    continue;
                }
                else if (flag == 1 && (NULL != strchr(string_lines, '=')))
                {

                    // malloc output name
                    *outputs_name = (char *)malloc(sizeof(char) * 128);
                    if (!*outputs_name)
                    {
                        LIBMAIX_DEBUG_PRINTF("libmaix mud src : malloc output name  buffer  is faild\n");
                        err = LIBMAIX_ERR_NO_MEM;
                        if (outputs_name_head)
                        {
                            for (int i = 0; i != MAX_LEN; i++)
                            {
                                char *output_name = *(outputs_name_head + i);
                                if (output_name != NULL)
                                {
                                    free(output_name);
                                    output_name = NULL;
                                }
                            }
                            free(outputs_name_head);
                            outputs_name_head = NULL;
                        }
                        return err;
                    }
                    memset(*outputs_name, 0, 128);
                    // get output name
                    char *name = libmaix_mud_get_key(string_lines);
                    if (!name)
                    {
                        LIBMAIX_DEBUG_PRINTF("libmaix mud src : get input name is faild\n");
                        err = LIBMAIX_ERR_PARAM;
                        if (outputs_name_head)
                        {
                            for (int i = 0; i != MAX_LEN; i++)
                            {
                                char *output_name = *(outputs_name_head + i);
                                if (output_name != NULL)
                                {
                                    free(output_name);
                                    output_name = NULL;
                                }
                            }
                            free(outputs_name_head);
                            outputs_name_head = NULL;
                        }
                        return err;
                    }
                    strcpy(*outputs_name, name);
                    free(name);

                    // malloc output value
                    *value = (float *)malloc(sizeof(float) * 10);
                    if (!*value)
                    {
                        LIBMAIX_DEBUG_PRINTF("libmaix mud src : malloc output values  buffer  is faild\n");
                        err = LIBMAIX_ERR_NO_MEM;
                        if (value_head)
                        {
                            for (int i = 0; i != MAX_LEN; i++)
                            {
                                float *output_value = *(value_head + i);
                                if (output_value != NULL)
                                {
                                    free(output_value);
                                    output_value = NULL;
                                }
                            }
                            free(value_head);
                            value_head = NULL;
                        }
                        return err;
                    }
                    memset(*value, 0, 10);
                    // set output value
                    float *tmp_value = libmaix_mud_get_float_value(string_lines);
                    if (!tmp_value)
                    {
                        LIBMAIX_DEBUG_PRINTF("libmaix mud src : get output values  is faild\n");
                        err = LIBMAIX_ERR_NO_MEM;
                        if (value_head)
                        {
                            for (int i = 0; i != MAX_LEN; i++)
                            {
                                float *output_value = *(value_head + i);
                                if (output_value != NULL)
                                {
                                    free(output_value);
                                    output_value = NULL;
                                }
                            }
                            free(value_head);
                            value_head = NULL;
                        }
                        return err;
                    }
                    memcpy(*value, tmp_value, 10 * sizeof(float));
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
            libmaix_mud_set_outputs_value(value_head, mud_info_obj);
            rewind(fp);
            return err;
        }
        // basic extra and decoder
        else if (0 == strcmp(title, "basic"))
        {
            while (!feof(fp))
            {
                fgets(string_lines, 128, fp);

                if (0 == strncmp(string_title, string_lines, strlen(string_title)))
                {
                    flag = 1;
                    continue;
                }
                else if (flag == 1 && (NULL != strchr(string_lines, '=')))
                {
                    char *key = libmaix_mud_get_key(string_lines);
                    char *value = libmaix_mud_get_sting_value(string_lines);

                    // LIBMAIX_DEBUG_PRINTF("[basic] key %p value %p\r\n", key, value);
                    if (key)
                    {
                        if (0 == strcmp(key, "type")) // input scale
                        {
                            // value is a buffer
                            // LIBMAIX_DEBUG_PRINTF("type len:%d, type:%s\n", strlen(value), value);
                            mud_info_obj->model_type = (char *)malloc(sizeof(char) * 128);
                            if (!mud_info_obj->model_type)
                            {
                                LIBMAIX_DEBUG_PRINTF("libmaix mud src : malloc basic model type  buffer  is faild\n");
                                err = LIBMAIX_ERR_NO_MEM;
                                free(key);
                                free(value);
                                return err;
                            }
                            strcpy(mud_info_obj->model_type, value);
                        }
                        if (0 == strcmp(key, "bin"))
                        {
                            LIBMAIX_DEBUG_PRINTF("bin len :%d , bin:%s\n", strlen(value), value);
                            if (*value == '/')
                            {

                                mud_info_obj->bin_path = (char *)malloc(sizeof(char) * 128);
                                strcpy(mud_info_obj->bin_path, value);
                            }
                            else
                            {

                                char *full_path = (char *)malloc(sizeof(char) * 128);
                                memset(full_path, 0, 128);
                                char *pre = full_path;
                                char *end = strrchr(mud_info_obj->mud_file_path, '/');
                                char *start = mud_info_obj->mud_file_path;
                                while (start <= end)
                                {
                                    *pre++ = *start++;
                                }
                                *pre = '\0';
                                strcat(full_path, value);
                                mud_info_obj->bin_path = full_path;
                            }
                        }
                        if (0 == strcmp(key, "param"))
                        {
                            // LIBMAIX_DEBUG_PRINTF("param len:%d, param:%s\n", strlen(value), value);
                            if (*value == '/')
                            {
                                mud_info_obj->param_path = (char *)malloc(sizeof(char) * 128);
                                strcpy(mud_info_obj->param_path, value);
                            }
                            else
                            {
                                char *full_path = (char *)malloc(sizeof(char) * 128);
                                memset(full_path, 0, 128);
                                char *pre = full_path;
                                char *end = strrchr(mud_info_obj->mud_file_path, '/');
                                char *start = mud_info_obj->mud_file_path;
                                while (start <= end)
                                {
                                    *pre++ = *start++;
                                }
                                *pre = '\0';
                                strcat(full_path, value);
                                mud_info_obj->param_path = full_path;
                            }
                        }
                    }
                    else
                    {
                        LIBMAIX_DEBUG_PRINTF("libmaix mud src : basic get key is error\n");
                        err = LIBMAIX_ERR_PARAM;
                        return err;
                    }
                }
                else if (strspn(string_lines, " \t\n") == strlen(string_lines))
                {
                    flag = 0;
                    continue;
                }
            }
            rewind(fp);
            return err;
        }
        // sacle
        else if (0 == strcmp(title, "extra"))
        {
            while (!feof(fp))
            {
                fgets(string_lines, 128, fp);
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
                    if (key)
                    {
                        if (0 == strcmp(key, "inputs_scale")) // input scale
                        {
                            for (int i = 0; i != mud_info_obj->input_num; i++)
                            {
                                mud_info_obj->inputs_scale[i] = value[i];
                            }
                            free(key);
                            free(value);
                            key = NULL;
                            value = NULL;
                        }
                        else
                        {
                            for (int j = 0; j != mud_info_obj->output_num; j++)
                            {
                                mud_info_obj->ouputs_scale[j] = value[j];
                            }
                            free(key);
                            free(value);
                            key = NULL;
                            value = NULL;
                        }
                    }
                    else
                    {
                        LIBMAIX_DEBUG_PRINTF("libmaix mud src : extra get key is error\n");
                        err = LIBMAIX_ERR_PARAM;
                        return err;
                    }
                }
                else if (strspn(string_lines, " \t\n") == strlen(string_lines))
                {
                    flag = 0;
                    continue;
                }
            }
            rewind(fp);
            return err;
        }
    }

    libmaix_err_t libmaix_mud_read_mud_file(char *mud_path, mud_info *mud_info_obj)
    {
        libmaix_err_t err = LIBMAIX_ERR_NONE;
        FILE *fp;
        if (*mud_path == '/')
        {
            mud_info_obj->mud_file_path = (char *)malloc(sizeof(char) * 128);
            if (!mud_info_obj->mud_file_path)
            {
                LIBMAIX_DEBUG_PRINTF("libmaix mud src : init mud info object's file path is faild\n");
                err = LIBMAIX_ERR_NOT_READY;
                return err;
            }
            strcpy(mud_info_obj->mud_file_path, mud_path);
            fp = fopen(mud_info_obj->mud_file_path, "r");
            if (NULL == fp)
            {
                LIBMAIX_DEBUG_PRINTF("libmaix mud src : can't open the specifically mud file\n ");
                err = LIBMAIX_ERR_NOT_READY;
                free(mud_info_obj->mud_file_path);
                mud_info_obj->mud_file_path = NULL;
                return err;
            }
        }
        else
        {
            if ((mud_info_obj->mud_file_path = getcwd(NULL, 0)) == NULL)
            {
                LIBMAIX_DEBUG_PRINTF("libmaix mud src : getcwd error\n");
                err = LIBMAIX_ERR_NOT_READY;
                return err;
            }
            strcat(mud_info_obj->mud_file_path, "/");
            strcat(mud_info_obj->mud_file_path, mud_path);
            fp = fopen(mud_info_obj->mud_file_path, "r");
            if (NULL == fp)
            {
                LIBMAIX_DEBUG_PRINTF("libmaix mud src : can't open the specifically mud file\n");
                err = LIBMAIX_ERR_NOT_READY;
                free(mud_info_obj->mud_file_path);
                mud_info_obj->mud_file_path = NULL;
                return err;
            }
        }
        if (fp && mud_info_obj)
            libmaix_mud_get_section(fp, "basic", mud_info_obj);
        if (fp && mud_info_obj)
            libmaix_mud_get_section(fp, "inputs", mud_info_obj);
        if (fp && mud_info_obj)
            libmaix_mud_get_section(fp, "outputs", mud_info_obj);
        if (fp && mud_info_obj)
            libmaix_mud_get_section(fp, "extra", mud_info_obj);
        if (fp)
            fclose(fp);

        return err;
    }

    libmaix_nn_t *libmaix_mud_build_model(mud_info *mud_info_obj, libmaix_nn_model_path_t *path, libmaix_nn_opt_param_t *opt)
    {
        libmaix_nn_t *nn = NULL;
        libmaix_err_t err = LIBMAIX_ERR_NONE;

        if (strcmp(mud_info_obj->model_type, "aipu") == 0)
        {
            LIBMAIX_DEBUG_PRINTF("r329");
            if (strlen(mud_info_obj->bin_path) == 0)
            {
                LIBMAIX_DEBUG_PRINTF("this path is empty ! ");
                return NULL;
            }
            // input && output num && enrypt
            opt->aipu.input_num = mud_info_obj->input_num;
            opt->aipu.output_num = mud_info_obj->output_num;
            opt->aipu.encrypt = false;
            // path
            int bin_len = strlen(mud_info_obj->bin_path);
            char *bin_src = mud_info_obj->bin_path;
            char *bin_dst = (char *)malloc(bin_len + 1);
            if (bin_dst)
            {
                memcpy(bin_dst, bin_src, bin_len);
                bin_dst[bin_len] = '\0';
                path->aipu.model_path = bin_dst;
            }


            // name
            opt->aipu.input_names = (char **)malloc(sizeof(char *) * mud_info_obj->input_num);
            for (int i = 0; i != opt->aipu.input_num; i++)
            {
                int len = strlen(mud_info_obj->inputs[i]) ;
                char *src = mud_info_obj->inputs[i];
                LIBMAIX_DEBUG_PRINTF("input id:%d  len:%d  inputs:%s", i, len, src);
                char *dst = (char *)malloc(len);
                if (dst)
                {
                    strcpy(dst, src);
                    opt->aipu.input_names[i] = dst;
                }
            }
            opt->aipu.output_names = (char **)malloc(sizeof(char *) * mud_info_obj->output_num);
            for (int i = 0; i != opt->aipu.output_num; i++)
            {
                int len = strlen(mud_info_obj->outpus[i]) ;
                char *src = mud_info_obj->outpus[i];
                LIBMAIX_DEBUG_PRINTF("output id:%d  len:%d  outputs:%s", i, len, src);
                char *dst = (char *)malloc(len);
                if (dst)
                {
                    strcpy(dst, src);
                    opt->aipu.output_names[i] = dst;
                }
            }


            for (int i = 0; i != 3; i++)
            {
                opt->aipu.mean[i] = mud_info_obj->mean[0][i];
                opt->aipu.norm[i] = mud_info_obj->norm[0][i];
            }
            for (int i = 0; i != mud_info_obj->output_num; i++)
            {
                opt->aipu.scale[i] = mud_info_obj->ouputs_scale[i];
            }
        }
        else if (strcmp(mud_info_obj->model_type, "awnn") == 0)
        {
            LIBMAIX_DEBUG_PRINTF("v831");

            if (strlen(mud_info_obj->bin_path) == 0 || strlen(mud_info_obj->param_path) == 0)
            {
                LIBMAIX_DEBUG_PRINTF("this path is empty ! ");
                return NULL;
            }
            // input && output num && enrypt
            opt->awnn.input_num = mud_info_obj->input_num;
            opt->awnn.output_num = mud_info_obj->output_num;
            opt->awnn.encrypt = false;
            // path
            int bin_len = strlen(mud_info_obj->bin_path);
            char *bin_src = mud_info_obj->bin_path;
            char *bin_dst = (char *)malloc(bin_len + 1);
            if (bin_dst)
            {
                memcpy(bin_dst, bin_src, bin_len);
                bin_dst[bin_len] = '\0';
                path->awnn.bin_path = bin_dst;
            }

            int param_len = strlen(mud_info_obj->param_path);
            char *param_src = mud_info_obj->param_path;
            char *param_dst = (char *)malloc(param_len + 1);
            if (param_dst)
            {
                memcpy(param_dst, param_src, param_len);
                param_dst[param_len] = '\0';
                path->awnn.param_path = param_dst;
            }

            opt->awnn.input_names = (char **)malloc(sizeof(char *) * mud_info_obj->input_num);
            for (int i = 0; i != opt->awnn.input_num; i++)
            {
                int len = strlen(mud_info_obj->inputs[i]) ;
                char *src = mud_info_obj->inputs[i];
                LIBMAIX_DEBUG_PRINTF("input id:%d  len:%d  inputs:%s", i, len, src);
                char *dst = (char *)malloc(len);
                if (dst)
                {
                    strcpy(dst, src);
                    opt->awnn.input_names[i] = dst;
                }
            }
            opt->awnn.output_names = (char **)malloc(sizeof(char *) * mud_info_obj->output_num);
            for (int i = 0; i != opt->awnn.output_num; i++)
            {
                int len = strlen(mud_info_obj->outpus[i]) ;
                char *src = mud_info_obj->outpus[i];
                LIBMAIX_DEBUG_PRINTF("output id:%d  len:%d  outputs:%s", i, len, src);
                char *dst = (char *)malloc(len);
                if (dst)
                {
                    strcpy(dst, src);
                    opt->awnn.output_names[i] = dst;
                }
            }

            // mean & norm
            for (int i = 0; i != 3; i++)
            {
                opt->awnn.mean[i] = mud_info_obj->mean[0][i];
                opt->awnn.norm[i] = mud_info_obj->norm[0][i];
            }
            for (int i = 0; i != 3; i++)
            {
                LIBMAIX_DEBUG_PRINTF("mean%d : %f \n", i, opt->awnn.mean[i]);
                LIBMAIX_DEBUG_PRINTF("norm%d : %f \n", i, opt->awnn.norm[i]);
            }
        }
        else
        {
            // printf("mud_info_obj->model_type %s is not support \n", mud_info_obj->model_type);
            LIBMAIX_DEBUG_PRINTF("this type value is empty or the type is unsupport !\n");
            return NULL;
        }

        // nn create
        nn = libmaix_nn_create();
        if (!nn)
        {
            LIBMAIX_DEBUG_PRINTF("libmaix_nn object create fail\n");
        }
        err = nn->init(nn);

        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_DEBUG_PRINTF("libmaix_nn init fail: %s\n", libmaix_get_err_msg(err));
        }
        err = nn->load(nn, path, opt);
        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_DEBUG_PRINTF("libmaix_nn load fail: %s\n", libmaix_get_err_msg(err));
        }
        return nn;
    }

    void libmaix_mud_deinit_mud(mud_info *mud_info_obj)
    {
        if (mud_info_obj->is_init)
        {
            if (mud_info_obj->inputs)
            {
                for (int i = 0; i != mud_info_obj->input_num; i++)
                {
                    char *ptr = *(mud_info_obj->inputs + i);
                    if (ptr != NULL)
                    {
                        free(ptr);
                        ptr = NULL;
                    }
                }

                free(mud_info_obj->inputs);
                mud_info_obj->inputs = NULL;
            }

            if (mud_info_obj->outpus)
            {

                for (int i = 0; i != mud_info_obj->output_num; i++)
                {
                    char *ptr = *(mud_info_obj->outpus + i);
                    if (ptr != NULL)
                    {
                        free(ptr);
                        ptr = NULL;
                    }
                }

                free(mud_info_obj->outpus);
                mud_info_obj->outpus = NULL;
            }
            if (mud_info_obj->model_type)
            {

                free(mud_info_obj->model_type);
                mud_info_obj->model_type = NULL;
            }
            if (mud_info_obj->bin_path)
            {

                free(mud_info_obj->bin_path);
                mud_info_obj->bin_path = NULL;
            }
            if (mud_info_obj->param_path)
            {

                free(mud_info_obj->param_path);
                mud_info_obj->param_path = NULL;
            }
            if (mud_info_obj->mud_file_path)
            {
                free(mud_info_obj->mud_file_path);
                mud_info_obj->mud_file_path = NULL;
            }

            free(mud_info_obj);
            mud_info_obj = NULL;
        }
    }

    mud_info *libmaix_mud_init(char *mud_path)
    {
        libmaix_err_t err = LIBMAIX_ERR_NONE;
        if (0 != access(mud_path, 0))
        {
            LIBMAIX_DEBUG_PRINTF("libmaix mud src : the mud file path is not exist\n");
            err = LIBMAIX_ERR_NOT_PERMIT;
            return NULL;
        }

        // malloc structer
        mud_info *mud_info_obj = (mud_info *)malloc(sizeof(mud_info));
        if (!mud_info_obj)
        {
            LIBMAIX_DEBUG_PRINTF("libmaix mud src : init mud object is faild\n");
            return NULL;
        }

        err = libmaix_mud_read_mud_file(mud_path, mud_info_obj);
        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_DEBUG_PRINTF("libmaix mud src : read  mud file is faild\n");
            if (mud_info_obj)
            {
                free(mud_info_obj);
                mud_info_obj = NULL;
                return NULL;
            }
        }
        mud_info_obj->is_init = true;
        return mud_info_obj;
    }

    libmaix_nn_t *libmaix_mud_load_model(char *mud_path)
    {
        // check mud path is usable
        libmaix_err_t err = LIBMAIX_ERR_NONE;
        libmaix_nn_t *nn = NULL;

        if (0 != access(mud_path, 0))
        {
            LIBMAIX_DEBUG_PRINTF("libmaix mud src : the mud file path is not exist\n");
            err = LIBMAIX_ERR_NOT_PERMIT;
            return NULL;
        }

        // malloc structer
        mud_info *mud_info_obj = (mud_info *)malloc(sizeof(mud_info));
        if (!mud_info_obj)
        {
            LIBMAIX_DEBUG_PRINTF("libmaix mud src : init mud object is faild\n");
            return NULL;
        }

        err = libmaix_mud_read_mud_file(mud_path, mud_info_obj);
        if (err != LIBMAIX_ERR_NONE)
        {
            LIBMAIX_DEBUG_PRINTF("libmaix mud src : read  mud file is faild\n");
            if (mud_info_obj)
            {
                free(mud_info_obj);
                mud_info_obj = NULL;
                return NULL;
            }
        }
        mud_info_obj->is_init = true;
        libmaix_nn_model_path_t model_path;
        libmaix_nn_opt_param_t opt_param;
        nn = libmaix_mud_build_model(mud_info_obj, &model_path, &opt_param);
        if (!nn)
        {
            LIBMAIX_DEBUG_PRINTF("libmaix mud src : build nn object is faild\n");
            return NULL;
        }
        return nn;
    }

#ifdef __cplusplus
}
#endif
