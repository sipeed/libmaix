#ifndef __MF_BRD_H
#define __MF_BRD_H

/*****************************************************************************/
// Headers
/*****************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/*****************************************************************************/
// Enums & Macro
/*****************************************************************************/
#define BRD_IRLED_PIN			"PL3"							/* 红外引脚 */
#define BRD_WLED_PIN			"PL4"							/* 补光灯引脚 */
#define BRD_RELAY_PIN			"PE14"							/* 继电器引脚 */
#define BRD_KEY_DEV				"/dev/input/event0"				/* 按键设备路径 */
#define BRD_GPIO_DEV_PATH   	"/sys/class/gpio"				/* GPIO设备路径 */
#define BRD_UARTP_DEV			"/dev/ttyS3"					/* 协议串口设备 */
#define BRD_KEY_DEV				"/dev/input/event0"				/* 按键设备路径 */
#define BRD_IIC_DEV				"/dev/iic-0"					/* IIC设备路径 */
#define BRD_WATCHDOG_DEV		"/dev/watchdog"					/* 看门狗设备路径 */
#define BRD_SPK_FAIL_FILE		"/root/assets/fail.wav"			/* 识别失败音频文件 */
#define BRD_SPK_RECORD_OK_FILE	"/root/assets/record_ok.wav"	/* 录入成功音频文件 */
#define BRD_SPK_SUCCESS_FILE	"/root/assets/success.wav"		/* 识别成功音频文件 */
typedef enum 
{
	MF_BOARD_MF7_V		= 0,	/*7寸竖屏标案*/
	MF_BOARD_MF7_H		= 1,	/*7寸横屏标案*/
	MF_BOARD_MF2_V		= 2,	/*伟创2.8寸*/

	MF_BOARD_NONE,		/* Invalid */
}mf_brd_type_t;

typedef enum{
	KEY_RELEASE=0,
	KEY_PRESS=1,
	KEY_LONGPRESS=2,
	KEY_DBL=3
} key_state_t;


typedef struct{
	int baud;
	int data_bits;
	int stop_bits;
	char parity;
}mf_brd_uart_t;

typedef struct{
	uint32_t address;
}mf_brd_iic_t;

typedef struct{
	uint32_t mode;			/* 工作模式 */
	uint32_t speed;			/* 频率 */
	uint8_t bits;			/* 发送/接收数据位 */
}mf_brd_spi_t;


#define IOS_00 0x00
#define IOS_01 0x01
#define IOS_10 0x02
#define IOS_11 0x03

#define CLASS_0 IOS_00
#define CLASS_1 IOS_01
#define CLASS_2 IOS_10
#define CLASS_NO IOS_11

#define IO_0_MASK 0x01
#define IO_1_MASK 0x02

/*****************************************************************************/
// Functions
/*****************************************************************************/

int board_init();
int get_io(int key, int key2, int* v1, int* v2);
void board_deinit();

/*****************************************************************************/
// Vars
/*****************************************************************************/




#endif

