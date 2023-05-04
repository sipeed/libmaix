
#include "linux_gpio.h"

int _write_file_only(char* path, char* buf, size_t count)
{
    int fd, res;

    fd = open(path, O_WRONLY | O_NONBLOCK);
    if (fd < 0)
        return -1;

    res = write(fd, buf, count);
    if (res < 0)
    {
        close(fd);
        return -2;
    }

    res = close(fd);
    if (res < 0)
        return -3;

    return res;
}

int _read_file_only(char* path, char* buf, size_t count)
{
    int fd, res;

    fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
        return -1;

    res = read(fd, buf, count);
    if (res < 0)
    {
        close(fd);
        return -2;
    }

    res = close(fd);
    if (res < 0)
        return -3;

    return res;
}

int _get_pwm_num(char *pin)
{
    int pwm_num = 0;
    if (strlen(pin) < 3)    return -1;

    if (!strcmp(pin, "PD1") || !strcmp(pin, "PH0"))
    {
        pwm_num = 0;
    }
    else if (!strcmp(pin, "PD2") || !strcmp(pin, "PH1"))
    {
        pwm_num = 1;
    }
    else if (!strcmp(pin, "PD3") || !strcmp(pin, "PH2"))
    {
        pwm_num = 2;
    }
    else if (!strcmp(pin, "PD4") || !strcmp(pin, "PH3"))
    {
        pwm_num = 3;
    }
    else if (!strcmp(pin, "PD5") || !strcmp(pin, "PH4"))
    {
        pwm_num = 4;
    }
    else if (!strcmp(pin, "PD6") || !strcmp(pin, "PH5"))
    {
        pwm_num = 5;
    }
    else if (!strcmp(pin, "PD7") || !strcmp(pin, "PH6"))
    {
        pwm_num = 6;
    }
    else if (!strcmp(pin, "PD8") || !strcmp(pin, "PH7"))
    {
        pwm_num = 7;
    }
    else if (!strcmp(pin, "PD9") || !strcmp(pin, "PH8"))
    {
        pwm_num = 8;
    }
    else if (!strcmp(pin, "PD19") || !strcmp(pin, "PD22") || !strcmp(pin, "PH9"))
    {
        pwm_num = 9;
    }
    else
    {
        pwm_num = -1;
    }

    return pwm_num;
}

/**
 * @brief 初始化pwm
 * @details
 * @param [in] pwm_id   按键id(0 按键0,1 按键1)
 * @param [in] fre      频率
 * @param [in] duty     占空比
 * @retval
 */
void _pwm_init(char *pin, uint64_t fre, float duty)
{
    int res = 0;
    char path[100];
    char arg[20];
    int pwm_id;
    uint64_t period = 0, duty_cycle = 0;

    pwm_id = _get_pwm_num(pin);
    if (-1 == pwm_id)   return;

    snprintf(arg, sizeof(arg), "%d", pwm_id);
    res = _write_file_only("/sys/class/pwm/pwmchip0/export", arg, strlen(arg));
    if (res < 0)    {return;}

    period = 1000000000 / fre;
    snprintf(path, sizeof(path), "/sys/class/pwm/pwmchip0/pwm%d/period", pwm_id);
    snprintf(arg, sizeof(arg), "%lld", period);
    res = _write_file_only(path, arg, strlen(arg));
    if (res < 0)    {return;}

    duty = duty > 1.0 ? 1.0 : duty;
    duty = 1.0 - duty;
    duty_cycle = period * duty;
    snprintf(path, sizeof(path), "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle", pwm_id);
    snprintf(arg, sizeof(arg), "%lld", duty_cycle);
    res = _write_file_only(path, arg, strlen(arg));
    if (res < 0)    {return;}

    snprintf(path, sizeof(path), "/sys/class/pwm/pwmchip0/pwm%d/enable", pwm_id);
    res = _write_file_only(path, "1", 1);
    if (res < 0)    {return;}
}

void _pwm_deinit(char *pin)
{
    int res = 0;
    char path[100];
    char arg[20];
    int pwm_id;

    pwm_id = _get_pwm_num(pin);
    if (-1 == pwm_id)   return;

    snprintf(path, sizeof(path), "/sys/class/pwm/pwmchip0/pwm%d/enable", pwm_id);
    res = _write_file_only(path, "0", 1);
    if (res < 0)    {return;}

    snprintf(arg, sizeof(arg), "%d", pwm_id);
    res = _write_file_only("/sys/class/pwm/pwmchip0/unexport", arg, strlen(arg));
    if (res < 0)    {return;}
}

void _pwm_set_duty(char * pin, float duty)
{
    int res = 0;
    char path[100];
    char arg[20];
    int pwm_id;
    uint64_t period = 0, duty_cycle = 0;

    pwm_id = _get_pwm_num(pin);
    if (-1 == pwm_id)   return;

    snprintf(path, sizeof(path), "/sys/class/pwm/pwmchip0/pwm%d/period", pwm_id);
    res = _read_file_only(path, arg, sizeof(arg));
    if (res < 0)    {return;}
    period = atoi(arg);

    duty = 1.0 - duty;
    duty_cycle = period * duty;
    snprintf(arg, sizeof(arg), "%lld", duty_cycle);
    snprintf(path, sizeof(path), "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle", pwm_id);
    res = _write_file_only(path, arg, strlen(arg));
    if (res < 0)    {return;}
}

/**
 * @brief 将字符引脚编号转换为数字引脚编号
 * @param [in] pin   字符引脚编号,格式必须为PAxx~PHxx，且xx的范围为0~31
 * @retval 返回数字引脚编号，如果为-1，则执行失败
 */
int _get_gpio_num(char* pin)
{
	if (strlen(pin) < 3)
		return -1;
	char c;
	int group_id, group_offset, gpio_num;

	c = pin[0];
	if (c != 'p' && c != 'P')
	{
		return -1;
	}

	c = pin[1];
	if ('a' <= c && c <= 'z')			// 限制a~z
	{
		group_id = c - 'a';
	}
	else if ('A' <= c && c <= 'Z')	    // 限制A~Z
	{
		group_id = c - 'A';
	}
	else
	{
		return -1;
	}

	group_offset = atoi(pin + 2);
	if (group_offset > 31)
	{
		return -1;
	}

	gpio_num = (group_id << 5) + group_offset;
	return gpio_num;
}

/**
 * @brief 向export文件注册gpio
 * @param [in] gpio   引脚编号
 * @retval 返回0，成功，小于0，失败
 */
int gpio_export(uint32_t gpio)
{
    int len, res;
    char buf[10];

    len = snprintf(buf, sizeof(buf), "%d", gpio);
	if (len > sizeof(buf))
		return -1;

    res = _write_file_only("/sys/class/gpio" "/export", buf, len);
    if (res < 0)
	{
		perror("gpio_export");
	}

    return res;
}

/**
 * @brief 向unexport文件取消注册gpio
 * @param [in] gpio   引脚编号
 * @retval 返回0，成功，小于0，失败
 */
int gpio_unexport(uint32_t gpio)
{
    int len, res;
    char buf[10];

    len = snprintf(buf, sizeof(buf), "%d", gpio);
	if (len > sizeof(buf))
		return -1;

    res = _write_file_only("/sys/class/gpio" "/unexport", buf, len);
    if (res < 0)
	{
		perror("gpio unexport");
	}

    return res;
}

/**
 * @brief 设置gpio输入/输出模式
 * @param [in] gpio   引脚编号
 * @param [in] mode   引脚模式,取值如下:
 *  				0,输入模式
 *  				1,输出模式，默认低电平
 *  				2,输出模式，默认低电平
 *  				3,输出模式，默认高电平
 * @retval 返回0，成功，小于0，失败
 */
int gpio_set_dir(uint32_t gpio, uint32_t mode)
{
    int len, res;
    char path[40];

    len = snprintf(path, sizeof(path), "/sys/class/gpio"  "/gpio%d/direction", gpio);
	if (len > sizeof(path))
		return -1;

    mode = mode > 3 ? 3 : mode;
    switch(mode)
    {
        case 0:res = _write_file_only(path, "in", sizeof("in"));break;
        case 1:res = _write_file_only(path, "out", sizeof("out"));break;
        case 2:res = _write_file_only(path, "low", sizeof("low"));break;
        case 3:res = _write_file_only(path, "high", sizeof("high"));break;
        default:break;
    }

    return res;
}

/**
 * @brief 设置gpio电平
 * @param [in] gpio   引脚编号
 * @param [in] value  引脚电平(1,高电平;0,低电平)
 * @retval 返回0，成功，小于0，失败
 */
int gpio_set_value(uint32_t gpio, uint32_t value)
{
    int len, res;
    char path[40];

    len = snprintf(path, sizeof(path), "/sys/class/gpio"  "/gpio%d/value", gpio);
	if (len > sizeof(path))
		return -1;

	if (value)
		res = _write_file_only(path, "1", 2);
	else
		res = _write_file_only(path, "0", 2);

    return res;
}

/**
 * @brief 读取gpio电平
 * @param [in] 	gpio   引脚编号
 * @param [out] value  引脚电平(1,高电平;0,低电平)
 * @retval 返回0，成功，小于0，失败
 */
int gpio_get_value(uint32_t gpio, uint32_t *value)
{
    int len, res;
    char path[40], state;

    len = snprintf(path, sizeof(path), "/sys/class/gpio"  "/gpio%d/value", gpio);
	if (len > sizeof(path))
		return -1;

	res = _read_file_only(path, &state, 1);
	if (res < 0)
		return res;

	*value = state == '0' ? 0 : 1;

    return res;
}

/**
 * @brief 初始化gpio
 * @note 还没有限制io_num的范围，需要注意
 * @param [in] dev      设备名
 * @param [in] mode     模式
 * @param [in] state    初始状态值
 * @retval 0 成功 <0 失败
 */
void* _gpio_init(char* pin, int mode, int state)
{
    int res = -1;

	int io_num = _get_gpio_num(pin);
	if (io_num < 0)
	{
		return (void *)res;
	}

    /* 向export文件注册一个gpio */
    res = gpio_export(io_num);
	if (res < 0)    return (void *)res;

	/* 设置gpio方向 */
	res = gpio_set_dir(io_num, mode);
	if (res < 0)    return (void *)res;

	if (mode > 0)
	{
		/* 设置gpio电平 */
		res = gpio_set_value(io_num, state);
		if (res < 0)    return (void *)res;

	}


    return (void*)res;
}

/**
 * @brief 初始化gpio
 * @note 还没有限制io_num的范围，需要注意
 * @param [in] handle      设备名
 * @retval
 */
void _gpio_deinit(char* pin)
{
    int res;

	int io_num = _get_gpio_num(pin);
	if (io_num < 0) return;
	res = gpio_unexport(io_num);
	if (res < 0) return;
}

/**
 * @brief 读gpio电平，只能在输入模式下调用
 * @note
 * @param [in]  handle  句柄，用来传入文件描述符
 * @param [out] state 状态值
 * @retval
 */
void _gpio_read(char* pin, int* state)
{
	int io_num = _get_gpio_num(pin);
	if (io_num < 0) return;

	gpio_get_value(io_num, (uint32_t *)state);

}

/**
 * @brief 写gpio电平，只能在输出模式下调用
 * @note
 * @param [in] handle  句柄，用来传入文件描述符
 * @param [in] state   状态值
 * @retval
 */
void _gpio_write(char* pin, int state)
{
	int io_num = _get_gpio_num(pin);
	if (io_num < 0) return;

	gpio_set_value(io_num, state);
}
