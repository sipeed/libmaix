#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "mdsc.h"
#define debug_line //printf("%s:%d %s %s %s \r\n", __FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__)
FILE *load_file(char *filename)
{
    FILE *fp;
    if (NULL == (fp = fopen(filename, "r")))
    {
        perror("fopen");
        return NULL;
    }
    return fp;
}

char *get_key(char *line)
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
    return key_head;
}

float *get_float_value(char *line)
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
            start++;
            *value = atof(single_value_head);
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
            *value = atof(single_value_head);
            value++;
            free(single_value_head);
        }
    }
    return value_head;
}

char *get_sting_value(char *line)
{

    char *strline = (char *)malloc(sizeof(char) * 1024);
    if(strline == NULL)
    {
        printf("malloc strine buffer is faild\n");
    }
    memcpy(strline, line, 1024);
    char *string_value = (char *)malloc(sizeof(char) * 32);
    if(string_value ==  NULL)
    {
        printf("malloc strlinr value buffer is faild\n");
    }
    memset(string_value , 0 ,32);
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
    return string_value_head;
}

void set_inputs_value(float **values, ini_info_t *ini_info) // float**指向二维数组
{
    int input_num = ini_info->input_num;
    for (int i = 0; i != ini_info->input_num; i++)
    {
        for (int j = 0; j != 3; j++)
        {
            ini_info->inputs_shape[i][j] = (int)values[i][j];
        }
        int h = (int)values[i][0];
        int w = (int)values[i][1];
        int c = (int)values[i][2];
        if (h != 1 && w != 1 && c == 3)
        {
            // mean
            ini_info->mean[i][0] = values[i][3];
            ini_info->mean[i][1] = values[i][4];
            ini_info->mean[i][2] = values[i][5];

            // norm,
            ini_info->norm[i][0] = values[i][6];
            ini_info->norm[i][1] = values[i][7];
            ini_info->norm[i][2] = values[i][8];
        }
        else if (h != 1 && w != 1 && c == 1)
        {
            // mean
            ini_info->mean[i][0] = values[i][3];
            ini_info->mean[i][1] = ini_info->mean[i][0];
            ini_info->mean[i][2] = ini_info->mean[i][0];

            // norm
            ini_info->norm[i][0] = values[i][4];
            ini_info->norm[i][1] = ini_info->norm[i][0];
            ini_info->norm[i][2] = ini_info->norm[i][0];
        }
        else if (h == 1 && w == 1 && c != 1)
        {
            // mean
            ini_info->mean[i][0] = values[i][3];
            ini_info->mean[i][1] = ini_info->mean[i][0];
            ini_info->mean[i][2] = ini_info->mean[i][0];

            // norm
            ini_info->norm[i][0] = values[i][4];
            ini_info->norm[i][1] = ini_info->norm[i][0];
            ini_info->norm[i][2] = ini_info->norm[i][0];
        }
    }
}

void set_outputs_value(float **values, ini_info_t *ini_info)
{
    for (int i = 0; i != ini_info->output_num; i++)
    {
        for (int j = 0; j != 3; j++)
        {
            ini_info->outputs_shape[i][j] = (int)values[i][j];
        }
    }
}

void set_inputs_scale(float *values, ini_info_t *ini_info)
{
    for (int i = 0; i != ini_info->input_num; i++)
    {
        ini_info->inputs_scale[i] = values[i];
    }
}

void set_outputs_scale(float *values, ini_info_t *ini_info)
{
    for (int i = 0; i != ini_info->output_num; i++)
    {
        ini_info->ouputs_scale[i] = values[i];
    }
}

int get_section(FILE *fp, char *title, ini_info_t *ini_info)
{
    int flag = 0;
    char string_title[64], string_lines[1024];
    sprintf(string_title, "[%s]", title);
    // input
    if (0 == strcmp(title, "inputs"))
    {

        int input_num = 0;
        char **inputs_name = (char **)malloc(sizeof(char *) * MAX_LEN);
        if(inputs_name == NULL)
        {
            printf("malloc inputs_name names  buffer  is faild\n");
        }
        char **inputs_name_head = inputs_name;
        float **value = (float **)malloc(sizeof(float *) * MAX_LEN);
        if(value == NULL)
        {
            printf("malloc input valuse  buffer  is faild\n");
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
                *inputs_name = get_key(string_lines);
                *value = get_float_value(string_lines);
                inputs_name++;
                value++;
                input_num++;
            }
            else if (strspn(string_lines, " \t\n") == strlen(string_lines))
            {

                flag = 0;
                continue;
            }
        }
        ini_info->input_num = input_num;
        ini_info->inputs = inputs_name_head;

        set_inputs_value(value_head , ini_info);
        rewind(fp);
    }
    // output
    else if (0 == strcmp(title, "outputs"))
    {

        int output_num = 0;
        char **outputs_name = (char **)malloc(sizeof(char *) * MAX_LEN);
        if(outputs_name == NULL)
        {
            printf("malloc output names  buffer  is faild\n");
        }
        char **outputs_name_head = outputs_name;
        float **value = (float **)malloc(sizeof(float *) * MAX_LEN);
        if(value == NULL)
        {
            printf("malloc output valuse  buffer  is faild\n");
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
                *outputs_name = get_key(string_lines);
                *value = get_float_value(string_lines);
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
        ini_info->output_num = output_num;
        ini_info->outpus = outputs_name_head;
        set_outputs_value(value_head , ini_info);
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
                char *key = get_key(string_lines);

                char *value = get_sting_value(string_lines);

                if (0 == strcmp(key, "type")) // input scale
                {

                    ini_info->model_type = value;
                }
                if (0 == strcmp(key, "bin"))
                {

                    ini_info->bin_path = value;
                    printf("bin len :%ld \n",strlen(value));
                }
                if (0 == strcmp(key, "param"))
                {

                    ini_info->param_path = value;
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
                char *key = get_key(string_lines);
                float *value = get_float_value(string_lines);
                debug_line;
                int count = 0;
                if ( 0 == strcmp( key ,  "inputs_scale"))  // input scale
                {
                    debug_line;
                    for(int i = 0 ; i != ini_info->input_num ; i++)
                    {
                        ini_info->inputs_scale[i]  =value[i];
                    }
                }
                else
                {
                    debug_line;
                    for(int j = 0 ; j != ini_info->output_num ; j++)
                    {
                        ini_info->ouputs_scale[j] = value[j];
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

ini_info_t read_file (char * mdsc_path)
{
    FILE *fp = load_file(mdsc_path);
    ini_info_t ini_info;
    get_section(fp , "basic", &ini_info);
    get_section(fp, "inputs", &ini_info);
    get_section(fp , "outputs", &ini_info);
    get_section(fp , "extra", &ini_info);
    return ini_info;
}

// int main(int argc, char const *argv[])
// {
//     int i = 0;
//     if (argc == 1)
//     {
//         printf("there is not ini file path to read\n'");
//         return -1;
//     }

//     char *file_path = argv[1];
//     // get information from ini file
//     FILE *fp = load_file(file_path);
//     // set a object to save  information
//     ini_info_t ini_info;
//     // section basic
//     get_section(fp , "basic", &ini_info);
//     get_section(fp, "inputs", &ini_info);
//     get_section(fp , "outputs", &ini_info);
//     debug_line;
//     get_section(fp , "extra", &ini_info);
//     printf("____________________\n");
//     printf("type :%s \n", ini_info.model_type);
//     printf("bin:%s\n", ini_info.bin_path);
//     printf("param:%s\n", ini_info.param_path);
//     printf("input num:%d\n", ini_info.input_num);
//     for(int i =0 ; i != ini_info.input_num;i++)
//     {
//         printf("this is innput%d \n ",i+1);
//         // printf("name:%s\n",ini_info.inputs[i]);
//         printf("C:%d   H:%d   W:%d\n",ini_info.inputs_shape[i][0] , ini_info.inputs_shape[i][1] , ini_info.inputs_shape[i][2]);
//         printf("norm_R: %f ,norm_G : %f  ,norm_B :%f\n",ini_info.norm[i][0],ini_info.norm[i][1],ini_info.norm[i][2]);
//         printf("mean_R: %f ,mean_G : %f  ,mean_B :%f\n",ini_info.mean[i][0],ini_info.mean[i][1],ini_info.mean[i][2]);
//         printf("input scale :%f \n",ini_info.inputs_scale[i] );

//     }
//     printf("____________________\n");
//     for(int i=0 ; i != ini_info.output_num ;i++)
//     {
//         printf("this is output%d \n ",i+1);
//         // printf("'name : %s \n", ini_info.outpus[i]);
//         printf("C:%d   H:%d   W:%d\n",ini_info.outputs_shape[i][0] , ini_info.outputs_shape[i][1] , ini_info.outputs_shape[i][2]);
//         printf("output scale :%f \n",ini_info.ouputs_scale[i]);
//     }
//     printf("____________________\n");

// }