/**
 * @file evdev_mouse.h
 *
 */

#ifndef EVDEV_MOUSE_H
#define EVDEV_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lv_conf.h"
#include "lv_drv_conf.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum
{
    EVDEV_TYPE_HOR_MIN = 0,
    EVDEV_TYPE_HOR_MAX,
    EVDEV_TYPE_VER_MIN,
    EVDEV_TYPE_VER_MAX,
    EVDEV_TYPE_VER_INVERT,
    EVDEV_TYPE_HOR_INVERT,
    EVDEV_TYPE_GET_X,
    EVDEV_TYPE_GET_Y,
    EVDEV_TYPE_SET_CALIBRATE_PAR        // 参数类型必须是int par[7];
}evdev_param_type_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the evdev
 */
void evdev_mouse_init(void);
/**
 * reconfigure the device file for evdev
 * @param dev_name set the evdev device filename
 * @return true: the device file set complete
 *         false: the device file doesn't exist current system
 */
bool evdev_mouse_set_file(char* dev_name);
/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 * @return false: because the points are not buffered, so no more data to be read
 */
bool evdev_mouse_read(lv_indev_drv_t * drv, lv_indev_data_t * data);

/**
 * Set params of the evdev
 * @param type type of param 
 * @param param param
 * @return 
 */
void evdev_mouse_ctrl(evdev_param_type_t type, void *param);

/**
 * Get evdev fd
 * @return evdev_fd
*/
int evdev_get_fd(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* EVDEV_MOUSE_H */
