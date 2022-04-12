/**
 * @file evdev_mouse.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"
#include "evdev_mouse.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#if USE_BSD_EVDEV
#include <dev/evdev/input.h>
#else
#include <linux/input.h>
#endif

/*********************
 *      DEFINES
 *********************/
#ifndef EVDEV_MOUSE_NAME
#define EVDEV_MOUSE_NAME "/dev/input/event0"
#endif

#ifndef EVDEV_MOUSE_SWAP_AXES
#define EVDEV_MOUSE_SWAP_AXES 1
#endif

#ifndef EVDEV_MOUSE_CALIBRATE
#define EVDEV_MOUSE_CALIBRATE 1
#    define EVDEV_HOR_MIN         6               /*to invert axis swap EVDEV_XXX_MIN by EVDEV_XXX_MAX*/
#    define EVDEV_HOR_MAX       590              /*"evtest" Linux tool can help to get the correct calibraion values>*/
#    define EVDEV_VER_MIN       -60
#    define EVDEV_VER_MAX      1010
#endif

// static int map(int x, int in_min, int in_max, int out_min, int out_max)
// {
//   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
// }

// data->point.x = map(evdev_root_x, evdev_hor_min, evdev_hor_max, 0, disp_hor_res);
// data->point.y = map(evdev_root_y, evdev_ver_min, evdev_ver_max, 0, disp_ver_res);

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static int map(int x, int in_min, int in_max, int out_min, int out_max);

/**********************
 *  STATIC VARIABLES
 **********************/
int evdev_fd;
int evdev_root_x;
int evdev_root_y;
int evdev_button;

int evdev_key_val;

static int evdev_hor_min = 6;
static int evdev_hor_max = 590;
static int evdev_ver_min = -60;
static int evdev_ver_max = 1010;
static int evdev_hor_invert = 0;
static int evdev_ver_invert = 0;
static int cal_para[7];
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the evdev interface
 */
void evdev_mouse_init(void)
{
#if USE_BSD_EVDEV
    evdev_fd = open(EVDEV_MOUSE_NAME, O_RDWR | O_NOCTTY);
#else
    evdev_fd = open(EVDEV_MOUSE_NAME, O_RDWR | O_NOCTTY | O_NDELAY);
#endif
    if(evdev_fd == -1) {
        perror("unable open evdev interface:");
        return;
    }

#if USE_BSD_EVDEV
    fcntl(evdev_fd, F_SETFL, O_NONBLOCK);
#else
    fcntl(evdev_fd, F_SETFL, O_ASYNC | O_NONBLOCK);
#endif

    evdev_root_x = 0;
    evdev_root_y = 0;
    evdev_key_val = 0;
    evdev_button = LV_INDEV_STATE_REL;
}
/**
 * reconfigure the device file for evdev
 * @param dev_name set the evdev device filename
 * @return true: the device file set complete
 *         false: the device file doesn't exist current system
 */
bool evdev_mouse_set_file(char* dev_name)
{ 
     if(evdev_fd != -1) {
        close(evdev_fd);
     }
#if USE_BSD_EVDEV
     evdev_fd = open(dev_name, O_RDWR | O_NOCTTY);
#else
     evdev_fd = open(dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
#endif

     if(evdev_fd == -1) {
        perror("unable open evdev interface:");
        return false;
     }

#if USE_BSD_EVDEV
     fcntl(evdev_fd, F_SETFL, O_NONBLOCK);
#else
     fcntl(evdev_fd, F_SETFL, O_ASYNC | O_NONBLOCK);
#endif

     evdev_root_x = 0;
     evdev_root_y = 0;
     evdev_key_val = 0;
     evdev_button = LV_INDEV_STATE_REL;

     return true;
}
/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 * @return false: because the points are not buffered, so no more data to be read
 */
bool evdev_mouse_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
    struct input_event in;
    lv_coord_t disp_hor_res = lv_disp_get_hor_res(drv->disp);
    lv_coord_t disp_ver_res = lv_disp_get_ver_res(drv->disp);

    while(read(evdev_fd, &in, sizeof(struct input_event)) > 0) {
        if(in.type == EV_REL) {
            if(in.code == REL_X)
				#if EVDEV_MOUSE_SWAP_AXES
					evdev_root_y += in.value;
				#else
					evdev_root_x += in.value;
				#endif
            else if(in.code == REL_Y)
				#if EVDEV_MOUSE_SWAP_AXES
					evdev_root_x += in.value;
				#else
					evdev_root_y += in.value;
				#endif
        } else if(in.type == EV_ABS) {
            if(in.code == ABS_X)
				#if EVDEV_MOUSE_SWAP_AXES
					evdev_root_y = (int)((float)in.value);// / 65535.0f * disp_ver_res);
				#else
          evdev_root_x = (int)((float)in.value);// / 65535.0f * disp_hor_res);
				#endif
            else if(in.code == ABS_Y)
				#if EVDEV_MOUSE_SWAP_AXES
					evdev_root_x = (int)((float)in.value);// / 65535.0f * disp_hor_res);
				#else
					evdev_root_y = (int)((float)in.value);// / 65535.0f * disp_ver_res);
				#endif
            else if(in.code == ABS_MT_POSITION_X)
                                #if EVDEV_MOUSE_SWAP_AXES
                                        evdev_root_y = in.value;
                                #else
                                        evdev_root_x = in.value;
                                #endif
            else if(in.code == ABS_MT_POSITION_Y)
                                #if EVDEV_MOUSE_SWAP_AXES
                                        evdev_root_x = in.value;
                                #else
                                        evdev_root_y = in.value;
                                #endif
        } else if(in.type == EV_KEY) {
            if(in.code == BTN_MOUSE || in.code == BTN_TOUCH) {
                if(in.value == 0)
                    evdev_button = LV_INDEV_STATE_REL;
                else if(in.value == 1)
                    evdev_button = LV_INDEV_STATE_PR;
            } else if(drv->type == LV_INDEV_TYPE_KEYPAD) {
		data->state = (in.value) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
		switch(in.code) {
			case KEY_BACKSPACE:
				data->key = LV_KEY_BACKSPACE;
				break;
			case KEY_ENTER:
				data->key = LV_KEY_ENTER;
				break;
			case KEY_UP:
				data->key = LV_KEY_UP;
				break;
			case KEY_LEFT:
				data->key = LV_KEY_PREV;
				break;
			case KEY_RIGHT:
				data->key = LV_KEY_NEXT;
				break;
			case KEY_DOWN:
				data->key = LV_KEY_DOWN;
				break;
			default:
				data->key = 0;
				break;
		}
		evdev_key_val = data->key;
		evdev_button = data->state;
		return false;
	    }
        }

#if EVDEV_MOUSE_CALIBRATE
    data->point.x = (cal_para[1] * evdev_root_x + cal_para[2] * evdev_root_y + cal_para[0]) >> 16;
    data->point.y = (cal_para[4] * evdev_root_x + cal_para[5] * evdev_root_y + cal_para[3]) >> 16;
    // printf("EVDEV x: %d y: %d(%d)\r\n", evdev_root_x, evdev_root_y, evdev_ver_max - evdev_root_y);
    // printf("CALIBRATE x: %d y: %d\r\n", data->point.x, data->point.y);
#else
    data->point.x = evdev_root_x;
    data->point.y = evdev_root_y;
#endif
    }

    if(drv->type == LV_INDEV_TYPE_KEYPAD) {
        /* No data retrieved */
        data->key = evdev_key_val;
	data->state = evdev_button;
	return false;
    }
    if(drv->type != LV_INDEV_TYPE_POINTER)
        return false;
    /*Store the collected data*/

    data->state = evdev_button;

    if(data->point.x < 0)
      data->point.x = 0;
    if(data->point.y < 0)
      data->point.y = 0;
    if(data->point.x >= disp_hor_res)
      data->point.x = disp_hor_res - 1;
    if(data->point.y >= disp_ver_res)
      data->point.y = disp_ver_res - 1;

    return false;
}


/**
 * set evdev params
*/
void evdev_mouse_ctrl(evdev_param_type_t type, void *param)
{
    switch (type)
    {
    case EVDEV_TYPE_HOR_MIN:
        evdev_hor_min = (int)param;
        break;
    case EVDEV_TYPE_HOR_MAX:
        evdev_hor_max = (int)param;
        break;
    case EVDEV_TYPE_VER_MIN:
        evdev_ver_min = (int)param;
        break;
    case EVDEV_TYPE_VER_MAX:
        evdev_ver_max = (int)param;
        break;
    case EVDEV_TYPE_VER_INVERT:
        evdev_ver_invert = (int)param;
        break;
    case EVDEV_TYPE_HOR_INVERT:
        evdev_hor_invert = (int)param;
        break;
    case EVDEV_TYPE_GET_X:
    {
        int *x = (int *)param;
        *x = evdev_root_x;
        break;
    }
    case EVDEV_TYPE_GET_Y:
    {
        int *y = (int *)param;
        *y = evdev_root_y;
        break;
    }
    case EVDEV_TYPE_SET_CALIBRATE_PAR:
    {
        int *para = (int *)param;
        memcpy(cal_para, para, 7 * sizeof(int));
        break;
    }
    default:break;
    }
}

/**
 * get evdev fd
*/
int evdev_get_fd(void)
{
    return evdev_fd;
}
/**********************
 *   STATIC FUNCTIONS
 **********************/
static int map(int x, int in_min, int in_max, int out_min, int out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

