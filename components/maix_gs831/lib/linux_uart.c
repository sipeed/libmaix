#include "linux_uart.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <linux/serial.h>


static int _get_baud(int baud)
{
    switch (baud)
    {
    case 9600:return B9600;
    case 19200:return B19200;
    case 38400:return B38400;
    case 57600:return B57600;
    case 115200:return B115200;
    case 230400:return B230400;
    case 460800:return B460800;
    case 500000:return B500000;
    case 576000:return B576000;
    case 921600:return B921600;
#ifdef B1000000
    case 1000000:return B1000000;
#endif
#ifdef B1152000
    case 1152000:return B1152000;
#endif
#ifdef B1500000
    case 1500000:return B1500000;
#endif
#ifdef B2000000
    case 2000000:return B2000000;
#endif
#ifdef B2500000
    case 2500000:return B2500000;
#endif
#ifdef B3000000
    case 3000000:return B3000000;
#endif
#ifdef B3500000
    case 3500000:return B3500000;
#endif
#ifdef B4000000
    case 4000000:return B4000000;
#endif
    default:return -1;
    }
}


static void clear_custom_speed_flag(int _fd)
{
    struct serial_struct ss;
    if (ioctl(_fd, TIOCGSERIAL, &ss) < 0) {
        // return silently as some devices do not support TIOCGSERIAL
        return;
    }

    if ((ss.flags & ASYNC_SPD_MASK) != ASYNC_SPD_CUST)
        return;

    ss.flags &= ~ASYNC_SPD_MASK;

    if (ioctl(_fd, TIOCSSERIAL, &ss) < 0) {
        perror("TIOCSSERIAL failed");
        exit(1);
    }
}

/**
 * @brief 初始化uart
 * @note
 * @param [in] dev    设备名
 * @param [in] param  参数
 * @retval
 */
int linux_uart_init(char* dev, void* param)
{
    int fd;

    uart_t* cfg = (uart_t *)param;

    int baud = _get_baud(cfg->baud);
    int data_bits = cfg->data_bits, stop_bits = cfg->stop_bits;
    char parity = cfg->parity;

    fd = open(dev, O_RDWR | O_NONBLOCK | O_NOCTTY);
    // fd = open(dev, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        return fd;
    }

    struct termios opt;
    memset(&opt, 0, sizeof(opt));

    /* 忽略modem，使能读模式 */
    opt.c_cflag |= CLOCAL | CREAD;

    /* 设置波特率 */
    opt.c_cflag |= baud;

    /* 设置数据位 */
    switch (data_bits)
    {
    case 7:
        opt.c_cflag |= CS7;
        break;
    case 8:
        opt.c_cflag |= CS8;
        break;
    default:break;
    }

    /* 设置奇偶校验位 */
    switch (parity)
    {
    case 'N':
    case 'n':
        opt.c_iflag &= ~INPCK;
        opt.c_cflag &= ~PARENB;
        break;
    case 'O':
    case 'o':
        opt.c_iflag |= (INPCK | ISTRIP);
        opt.c_cflag |= (PARODD | PARENB);
        break;
    case 'E':
    case 'e':
        opt.c_iflag |= (INPCK | ISTRIP);
        opt.c_cflag |= PARENB;
        opt.c_cflag &= ~PARODD;
        break;
    default:break;
    }

    /* 设置停止位 */
    switch (stop_bits)
    {
    case 1:
        opt.c_cflag &= ~CSTOPB;
        break;
    case 2:
        opt.c_cflag |= CSTOPB;
        break;
    default:break;
    }

    /* 设置流控制 */
    opt.c_cflag &= ~CRTSCTS;

    /* 最小字节数与等待时间 */
    opt.c_cc[VMIN] = 1;
    opt.c_cc[VTIME] = 0;

    /* 刷新串口，更新配置 */
    tcflush(fd, TCIOFLUSH);
    tcsetattr(fd, TCSANOW, &opt);

    clear_custom_speed_flag(fd);

    return fd;
}

void linux_uart_deinit(int fd)
{
    int res;

    res = close(fd);
    if (res < 0)
        fprintf(stderr, "uart close fd(%d) err:%s\n", fd, strerror(errno));
    else
        fd = -1;
}


