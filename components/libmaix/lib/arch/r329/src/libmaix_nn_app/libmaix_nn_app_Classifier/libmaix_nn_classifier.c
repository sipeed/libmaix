
#include "libmaix_nn_classifier.h"
#include "stdlib.h"
#include "errno.h"
#include "math.h"
#include "float.h"
#include "string.h"
#include <unistd.h>
#include "stdio.h"
#include "stdbool.h"
#include "libmaix_nn.h"
#include "libmaix_image.h"

#define SAMPLES_LENGTH  512
#define SAMPLES_LENGTH_DETECTOR 384
#define DETECTOR_SUB_RATE 16 // 224/14


typedef struct _kmeans_point_t
{
    uint32_t length;
    int32_t class;
    float center_distance;
    float* data;
} kmeans_point_t;

typedef struct 
{
    libmaix_nn_t* model;
    int class_num;
    int sample_num;
    int curr_class_num;
    int curr_sample_num;
    kmeans_point_t** center_point;
    kmeans_point_t** sample_point;
    kmeans_point_t*  predict_point;
    // bool is_detector;
    int crop_size;
    float* detector_class_distance; // length: class_num

    int samples_length;
    int input_w;
    int input_h;

    libmaix_nn_layer_t* input;
    libmaix_nn_layer_t* output;
} classifier_t;

static libmaix_err_t check_param(classifier_t* self)
{
    if(!self->model)
        return LIBMAIX_ERR_PARAM;
    return LIBMAIX_ERR_NONE;
}

static kmeans_point_t *k_point_init(uint32_t length)
{
    kmeans_point_t *k_point = (kmeans_point_t*)malloc(sizeof(kmeans_point_t));
    if(!k_point)
        return NULL;
    k_point->length = length;
    k_point->class = -1;
    k_point->center_distance = -1;
    k_point->data = (float*)malloc(sizeof(float) * k_point->length);
    if(!k_point->data)
    {
        free(k_point);
        return NULL;
    }
    memset(k_point->data, 0, sizeof(float) * k_point->length);
    return k_point;
}

static void k_point_deinit(kmeans_point_t *k_point)
{
    free(k_point->data);
    //k_point->data = NULL;
    free(k_point);
    //k_point = NULL;
}

static float get_distance(kmeans_point_t *point_a, kmeans_point_t *point_b)
{
    if(point_a->length != point_b->length)
    {
        return -1.;
    }
    float sum = 0.;
    for (uint32_t i = 0; i < point_a->length; i++)
    {
        sum += (point_a->data[i] - point_b->data[i]) * (point_a->data[i] - point_b->data[i]);
    }
    sum = sqrt(sum);
    return sum;
}

// static float get_distance2(kmeans_point_t *point_a, kmeans_point_t *point_b)
// {
//     if(point_a->length != point_b->length)
//     {
//         return -1.;
//     }
//     float sum = 0.;
//     float sum2 = 0.;
//     float sum3 = 0.;
//     uint32_t i = 0;

//     for (i = 0; i < point_a->length; i++)
//     {
        
//         sum += point_a->data[i] * point_b->data[i];
//         sum2 +=  point_a->data[i]*point_a->data[i] ;
//         sum3 +=  point_b->data[i]*point_b->data[i] ;
//     }
     
//     sum = sum/sqrt(sum2 * sum3);
//     sum = 0.5 + 0.5*sum;

//     return sum;
// }

static void k_means_cluster(classifier_t* self)
{
    for(int i = 0; i < self->curr_sample_num; i++)
    {
        float min_dis = FLT_MAX;
        float dis = 0;
        for(int j = 0; j <self->curr_class_num; j++)
        {
            dis = get_distance(self->sample_point[i], self->center_point[j]);
            if(dis < min_dis)
            {
                min_dis = dis;
                self->sample_point[i]->class = j;
            }
        }
        //printf("point[%d] in cluster %d\r\n", i, sample_point[i]->class);
    }
}

static void k_means_update_center(classifier_t* self)
{
    bool class_have_sample[self->class_num];
    memset(class_have_sample, 0, self->class_num * sizeof(bool));
    uint32_t class_num[self->class_num];
    // see all sample's class, don't set to zero if no sample belongs to one class
    for(int i = 0; i < self->curr_sample_num; i++)
    {
        class_have_sample[self->sample_point[i]->class] = true;
    }
    // set center point all zero
    for(int i = 0; i < self->class_num; i++)
    {
        if(class_have_sample[i])
        {
            memset(self->center_point[i]->data, 0, sizeof(float) * self->center_point[i]->length);
            class_num[i] = 0;
        }
    }
    // add points to the center of the corresponding category
    for(int i = 0; i < self->curr_sample_num; i++)
    {
        for(int j = 0; j < self->sample_point[i]->length; j++)
        {
            self->center_point[self->sample_point[i]->class]->data[j] += self->sample_point[i]->data[j];
        }
        class_num[self->sample_point[i]->class] += 1;
    }
    for(int i = 0; i < self->curr_class_num; i++)
    {
        if(class_have_sample[i])
        {
            for(int j = 0; j < self->center_point[i]->length; j++)
            {
                self->center_point[i]->data[j] /= class_num[i];
            }
        }
    }
}

static float k_means_get_difference(classifier_t* self)
{
    float sum = 0;
    for(int i = 0; i < self->curr_sample_num; i++)
    {
        sum += get_distance(self->center_point[self->sample_point[i]->class], self->sample_point[i]);
    }
    return sum;
}

static int k_means_train(classifier_t* self)
{
    float score1, score2; 
    uint32_t count = 0;
    // start first time cluster
    k_means_cluster(self);
    // sum of center point and sample points disstance.
    score1 = k_means_get_difference(self);
    count++;
    //printf("The 1st difference between samples and center is %.2f\r\n", score1);
    // updata center
    k_means_update_center(self);
    k_means_cluster(self);
    score2 = k_means_get_difference(self);
    count++;
    //printf("The 2nd difference between samples and center is %.2f\r\n", score2);
    while (fabs(score2 - score1) != 0)
    {
        score1 = score2;
        k_means_update_center(self);
        k_means_cluster(self);
        score2 = k_means_get_difference(self);
        count++;

        //printf("The %dth difference between samples and center is %.2f\r\n", count, score2);
    }
    //printf("The total number of cluster is: %d\r\n", count);
    
    return 0;
}

static int k_means_predict(classifier_t* self, float* min_distance)
{
    // float softmax_in[K_NUM];
    // float softmax_out[K_NUM];

    if(self->predict_point->length != self->center_point[0]->length)
    {
        //printf("predict error!\r\n");
        return -EINVAL;
    }
    *min_distance = FLT_MAX;
    float dis = 0;
    for(int i = 0; i < self->curr_class_num; i++)
    {
        dis = get_distance(self->predict_point, self->center_point[i]);
        if(dis < *min_distance)
        {
            *min_distance = dis;
            self->predict_point->class = i;
        }
        // softmax_in[i] = dis;
    }
    // softmax(softmax_in, softmax_out, K_NUM);
    // for(int i = 0; i < K_NUM; i++)
    // {
    //     printf("%d,", (uint8_t)(softmax_out[i]*100));
    // }
    // printf("\r\n");
    //printf("Predict class is %d \r\n", predict_point->class);
    return self->predict_point->class;
}

// static int arg_max(float* data, int len)
// {
//     float min = data[0];
//     int index = 0;
//     for(int i=1; i<len; ++i)
//     {
//         if(data[i] > min)
//         {
//             index = i;
//             min = data[i];
//         }
//     }
//     return index;
// }

libmaix_err_t libmaix_classifier_init(void** obj, libmaix_nn_t* model, int samples_length, int input_w, int input_h, int class_num, int sample_num)
{
    if(*obj != NULL)
        free(*obj);
    *obj = malloc(sizeof(classifier_t));
    if(!*obj)
        return LIBMAIX_ERR_NO_MEM;
    memset(*obj, 0, sizeof(classifier_t));
    classifier_t* self = (classifier_t*)*obj;
    self->samples_length = samples_length;
    self->model = model;
    self->class_num = class_num;
    self->sample_num = sample_num;
    self->input_w = input_w;
    self->input_h = input_h;
    libmaix_err_t ret = check_param(self);
    if(ret != LIBMAIX_ERR_NONE){
        goto err;
    }
    self->center_point = (kmeans_point_t**)malloc(sizeof(kmeans_point_t *) * self->class_num);
    if(!self->center_point){
        ret = LIBMAIX_ERR_NO_MEM;
        goto err;
    }
    self->sample_point = (kmeans_point_t**)malloc(sizeof(kmeans_point_t*) * self->sample_num);
    if(!self->sample_point){
        ret = LIBMAIX_ERR_NO_MEM;
        free(self->center_point);
        goto err;
    }
    // if(self->is_detector)
    // {
    //     self->detector_class_distance = (float*)malloc(sizeof(float) * self->class_num);
    //     if(!self->detector_class_distance)
    //     {
    //         ret = -ENOMEM;
    //         free(self->sample_point);
    //         free(self->center_point);
    //         goto err;
    //     }
    // }
    memset(self->center_point, 0, sizeof(kmeans_point_t*) * self->class_num);
    memset(self->sample_point, 0, sizeof(kmeans_point_t*) * self->sample_num);
    // init center point
    for(int i = 0; i < self->class_num; i++)
    {
        self->center_point[i] = k_point_init(self->samples_length);
        if(!self->center_point[i]){
            ret = LIBMAIX_ERR_NO_MEM;
            goto err1;
        }
        self->center_point[i]->class = i;
        self->center_point[i]->center_distance = -1;
    }
    // init predict point
    self->predict_point = k_point_init(self->samples_length);
    if(!self->predict_point){
        ret = LIBMAIX_ERR_NO_MEM;
        goto err1;
    }

    // input output temp var
    self->input = (libmaix_nn_layer_t*)malloc(sizeof(libmaix_nn_layer_t));
    if(!self->input)
    {
        ret = LIBMAIX_ERR_NO_MEM;
        goto err1;
    }
    void* buff = malloc(input_w * input_h * 3);
    if(!buff)
    {
        ret = LIBMAIX_ERR_NO_MEM;
        goto err1;
    }
    self->input->c = 3;
    self->input->w = input_w;
    self->input->h = input_h;
    self->input->need_quantization = true;
    self->input->layout = LIBMAIX_NN_LAYOUT_HWC;
    self->input->dtype = LIBMAIX_NN_DTYPE_UINT8;
    self->input->data = NULL;
    self->input->buff_quantization = buff;

    self->output = (libmaix_nn_layer_t*)malloc(sizeof(libmaix_nn_layer_t));
    if(!self->output)
    {
        ret = LIBMAIX_ERR_NO_MEM;
        goto err1;
    }
    float* out_buff = malloc(samples_length * sizeof(float));
    if(!out_buff)
    {
        ret = LIBMAIX_ERR_NO_MEM;
        goto err1;
    }
    self->output->c = samples_length;
    self->output->w = 1;
    self->output->h = 1;
    self->output->need_quantization = false;
    self->output->layout = LIBMAIX_NN_LAYOUT_CHW;
    self->output->dtype = LIBMAIX_NN_DTYPE_FLOAT;
    self->output->data = out_buff;
    self->output->buff_quantization = NULL;

    return LIBMAIX_ERR_NONE;
err1:
    if(self->output)
    {
        if(self->output->data)
            free(self->output->data);
        free(self->output);
    }
    if(self->input)
    {
        if(self->input->buff_quantization)
            free(self->input->buff_quantization);
        free(self->input);
    }
    for(int i=0; i < self->class_num; ++i){
        if(self->center_point[i])
            k_point_deinit(self->center_point[i]);
    }
    for(int i=0; i < self->sample_num; ++i){
        if(self->sample_point[i])
            k_point_deinit(self->sample_point[i]);
    }
    free(self->center_point);
    free(self->sample_point);
err:
    free(*obj);
    *obj = NULL;
    return ret;
}


libmaix_err_t libmaix_classifier_add_class_img(void* obj, libmaix_image_t* img, int* idx)
{
    libmaix_err_t err;
    if(!obj)
        return LIBMAIX_ERR_PARAM;
    if(!img->data || img->mode < 3)
        return LIBMAIX_ERR_PARAM;
    classifier_t* self = (classifier_t*)obj;
    if( ((*idx >= 0) && *idx >= self->class_num) || ((*idx < 0) && self->curr_class_num >= self->class_num))
        return LIBMAIX_ERR_PARAM;
    if(self->input_w != img->width || self->input_h != img->height)
        return LIBMAIX_ERR_PARAM;
    self->input->data =(uint8_t *) img->data;
    err = self->model->forward(self->model, self->input, self->output);
    if(err != LIBMAIX_ERR_NONE) return err;
    float *features = self->output->data;
    // if(self->is_detector)
    // {
    //     // detector_calc_feature_average(features, self->crop_size, -1, -1, -1, -1, NULL);
    // }
    // copy features to point
    if(*idx >= 0)
    {
        memcpy(self->center_point[*idx]->data, features, sizeof(float) * self->center_point[*idx]->length);
        self->center_point[*idx]->center_distance = 0;
        if(*idx >= self->curr_class_num)
        {
            self->curr_class_num = *idx + 1;
        }
    }
    else
    {
        memcpy(self->center_point[self->curr_class_num]->data, features, sizeof(float) * self->center_point[self->curr_class_num]->length);
        self->center_point[self->curr_class_num]->center_distance = 0;
        self->curr_class_num++;
        *idx = self->curr_class_num - 1;
    }
    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_classifier_rm_class_img(void* obj)
{
    if(!obj)
        return LIBMAIX_ERR_PARAM;
    classifier_t* self = (classifier_t*)obj;
    if(self->curr_class_num == 0 && self->center_point[self->curr_class_num] == NULL)
        return LIBMAIX_ERR_NOT_PERMIT;
    
    k_point_deinit(self->center_point[self->curr_class_num]);
    self->center_point[self->curr_class_num] = NULL;
    if(self->curr_class_num > 0)
    {
        self->curr_class_num --;
    }
    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_classifier_add_sample_img(void* obj, libmaix_image_t* img, int* idx)
{
    libmaix_err_t err;

    if(!obj)
        return LIBMAIX_ERR_PARAM;
    if(!img->data || img->mode < 3)
        return LIBMAIX_ERR_PARAM;
    if(!idx)
        return LIBMAIX_ERR_PARAM;
    if(*idx>=0)
        return LIBMAIX_ERR_NOT_IMPLEMENT;

    classifier_t* self = (classifier_t*)obj;
    if(self->curr_sample_num >= self->sample_num)
        return LIBMAIX_ERR_NOT_PERMIT;
    if(self->input_w != img->width || self->input_h != img->height)
        return LIBMAIX_ERR_PARAM;

    self->sample_point[self->curr_sample_num] = k_point_init(self->samples_length);
    if(!self->sample_point[self->curr_sample_num])
        return LIBMAIX_ERR_NO_MEM;

    self->input->data =(uint8_t *) img->data;
    err = self->model->forward(self->model, self->input, self->output);
    if(err != LIBMAIX_ERR_NONE) return err;
    
    float *features = self->output->data;

    // if(self->is_detector)
    // {
    //     // detector_calc_feature_average(features, self->crop_size, -1, -1, -1, -1, NULL);
    // }
    memcpy(self->sample_point[self->curr_sample_num]->data, features, sizeof(float) * self->sample_point[self->curr_sample_num]->length);
    self->sample_point[self->curr_sample_num]->center_distance = -2;
    *idx = self->curr_sample_num;
    self->curr_sample_num ++;
    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_classifier_rm_sample_img(void* obj)
{
    if(!obj)
        return LIBMAIX_ERR_PARAM;
    classifier_t* self = (classifier_t*)obj;
    if(self->curr_sample_num == 0 && self->sample_point[self->curr_sample_num] == NULL)
        return LIBMAIX_ERR_NOT_PERMIT;

    k_point_deinit(self->sample_point[self->curr_sample_num]);
    self->sample_point[self->curr_sample_num] = NULL;
    if(self->curr_sample_num > 0)
    {
        self->curr_sample_num --;
    }
    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_classifier_del(void** obj)
{
    if(!*obj)
        return LIBMAIX_ERR_PARAM;
    classifier_t* self = (classifier_t*)*obj;
    if(self->center_point){
        for(int i=0; i < self->class_num; ++i){
            if(self->center_point[i])
            {
                k_point_deinit(self->center_point[i]);
            }
        }
        free(self->center_point);
    }
    if(self->sample_point){
        for(int i=0; i < self->sample_num; ++i){
            if(self->sample_point[i])
            {
                k_point_deinit(self->sample_point[i]);
            }
        }
        free(self->sample_point);
    }
    if(self->predict_point)
    {
        k_point_deinit(self->predict_point);
    }
    // if(self->is_detector)
    // {
    //     free(self->detector_class_distance);
    // }
    free(*obj);
    *obj = NULL;
    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_classifier_train(void* obj)
{
    int i = 0;
    if(!obj)
        return LIBMAIX_ERR_PARAM;
    classifier_t* self = (classifier_t*)obj;
    for(i = 0; i < self->class_num; i++)
    {
        if(self->center_point[i]->center_distance < 0)
        {
            return LIBMAIX_ERR_NOT_PERMIT;
        }
    }
    if(self->curr_class_num==0 || self->curr_sample_num==0)
        return LIBMAIX_ERR_NOT_PERMIT;
    // check samples
    for(i = 0; i < self->curr_sample_num; i++)
    {
        if(self->sample_point[i]->center_distance == -1)
        {
            return LIBMAIX_ERR_NOT_PERMIT;
        }
    }

    k_means_train(self);


    // free sample points
    for(i = 0; i < self->curr_sample_num; i++)
    {
        k_point_deinit(self->sample_point[i]);
        self->sample_point[i] = NULL;
    }
    self->curr_sample_num = 0;
    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_classifier_predict(void* obj, libmaix_image_t* img, int* idx, float* min_distance)
{
    int ret = 0;
    libmaix_err_t err;

    if(!obj)
        return LIBMAIX_ERR_PARAM;
    if(!img->data || img->mode < 3)
        return LIBMAIX_ERR_PARAM;
    classifier_t* self = (classifier_t*)obj;
    *idx = -1;

    if(self->input_w != img->width || self->input_h != img->height)
        return LIBMAIX_ERR_PARAM;

    self->input->data = img->data;
    err = self->model->forward(self->model, self->input, self->output);
    if(err != LIBMAIX_ERR_NONE) return err;

    float *features = self->output->data;

    // if(self->is_detector)
    // {
    //     //detect position
    //     // ret = detect_position(self, features, p_x, p_y, p_w, p_h, min_distance);
    //     return ret;
    // }
    //copy feature to predict point
    memcpy(self->predict_point->data, features, sizeof(float) * self->predict_point->length);

    

    ret = k_means_predict(self, min_distance);
    if(ret < 0)
    {
        return LIBMAIX_ERR_UNKNOWN;
    }
    else if(ret > self->curr_class_num)
    {
        return LIBMAIX_ERR_UNKNOWN;
    }
    *idx = ret;
    return LIBMAIX_ERR_NONE;
}

typedef struct{
    uint16_t class_num;
    uint16_t sample_num;
    kmeans_point_t** center_point;
} classifier_saved_model_t;

FILE* vfs_internal_open(const char* path, const char* mode)
{
    FILE* fp = fopen(path, mode);
	if (fp == NULL)
	{
		fprintf(stderr, "fopen %s failed\n", path);
		return NULL;
	}
    return fp;
}
int vfs_internal_write(FILE* fd, void* data, int len)
{
    int nw = fwrite(data, 1, len, fd);
    return nw;
}
int vfs_internal_read(FILE* fd, void* data, int len)
{
    int nr = fread(data, 1, len, fd);
    return nr;
}
void vfs_internal_close(FILE* fd)
{
    fclose(fd);
}

// | 2B        | 2B         | 4B             | 4B      |   4B    | kmeans_point_t  | ... | kmeans_point_t |
// | class_num | sample_num | samples_length | input_w | input_h |                 |     |                |
libmaix_err_t libmaix_classifier_save(void* obj, const char* path)
{
    int ret = 0, w_len = 0;
    if(!obj)
        return LIBMAIX_ERR_PARAM;
    classifier_t* self = (classifier_t*)obj;
    FILE* f = vfs_internal_open(path, "wb");
    if(!f){
        return LIBMAIX_ERR_PARAM;
    }
    classifier_saved_model_t model;
    model.class_num = self->curr_class_num;
    model.sample_num = self->sample_num;
    model.center_point = self->center_point;
    w_len = sizeof(model.class_num) + sizeof(model.sample_num);
    ret = vfs_internal_write(f, &model.class_num, w_len);
    if(ret < 0 || ret!=w_len){
        vfs_internal_close(f);
        return LIBMAIX_ERR_NOT_EXEC;
    }
    w_len = sizeof(self->samples_length) + sizeof(self->input_w) + sizeof(self->input_h);
    ret = vfs_internal_write(f, &self->samples_length, w_len);
    if(ret < 0 || ret!=w_len){
        vfs_internal_close(f);
        return LIBMAIX_ERR_NOT_EXEC;
    }
    for(int i=0; i<model.class_num; ++i){
        w_len = sizeof(kmeans_point_t) - sizeof(float*);
        ret = vfs_internal_write(f, model.center_point[i], w_len);
        if(ret < 0 || ret!=w_len){
            vfs_internal_close(f);
            return LIBMAIX_ERR_NOT_EXEC;
        }
        w_len = self->samples_length * sizeof(float);
        ret = vfs_internal_write(f, model.center_point[i]->data, w_len);
        if(ret < 0 || ret!=w_len){
            vfs_internal_close(f);
            return LIBMAIX_ERR_NOT_EXEC;
        }
    }
    vfs_internal_close(f);
    return LIBMAIX_ERR_NONE;
}

libmaix_err_t libmaix_classifier_load(void** obj, const char* path, libmaix_nn_t* kmodel, int* feature_length, int* input_w, int* input_h, int* class_num, int* sample_num)
{
    if(*obj){
        libmaix_classifier_del(obj);
    }
    int ret = 0, r_len = 0;
    classifier_t temp;

    FILE* f = vfs_internal_open(path, "rb");
    if(!f){
        return LIBMAIX_ERR_PARAM;
    }
    classifier_saved_model_t model;
    r_len = sizeof(model.class_num) + sizeof(model.sample_num);
    ret = vfs_internal_read(f, &model.class_num, r_len);
    if(ret < 0 || ret!=r_len){
        vfs_internal_close(f);
        return LIBMAIX_ERR_NOT_EXEC;
    }
    r_len = sizeof(temp.samples_length) + sizeof(temp.input_w) + sizeof(temp.input_h);
    ret = vfs_internal_read(f, &temp.samples_length, r_len);
    if(ret < 0 || ret!=r_len){
        vfs_internal_close(f);
        return LIBMAIX_ERR_NOT_EXEC;
    }
    // init obj
    if( *sample_num <= 0)
    {
        *sample_num = model.sample_num;
    }
    if(*class_num <= 0)
    {
        *class_num = model.class_num;
    }
    *input_w = temp.input_w;
    *input_h = temp.input_h;
    *feature_length = temp.samples_length;
    ret = libmaix_classifier_init(obj, kmodel, temp.samples_length, temp.input_w, temp.input_h, *class_num, *sample_num);
    if(ret < 0){
        return LIBMAIX_ERR_NOT_EXEC;
    }
    classifier_t* self = (classifier_t*)*obj;
    model.center_point = self->center_point;
    for(int i=0; i<model.class_num; ++i){
        r_len = sizeof(kmeans_point_t) - sizeof(float*);
        ret = vfs_internal_read(f, model.center_point[i], r_len);
        if(ret < 0 || ret!=r_len){
            vfs_internal_close(f);
            return LIBMAIX_ERR_NOT_EXEC;
        }
        r_len = self->samples_length * sizeof(float);
        ret = vfs_internal_read(f, model.center_point[i]->data, r_len);
        if(ret < 0 || ret!=r_len){
            vfs_internal_close(f);
            return LIBMAIX_ERR_NOT_EXEC;
        }
    }
    self->model = kmodel;
    self->class_num = *class_num;
    self->curr_class_num = *class_num;
    self->sample_num = *sample_num;
    self->curr_sample_num = 0;
    vfs_internal_close(f);
    return LIBMAIX_ERR_NONE;
}

