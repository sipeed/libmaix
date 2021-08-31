#include "board.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#include <stdlib.h>
#include <string.h>



static int keys_fd;	

int board_init()
{
    keys_fd = open(BRD_KEY_DEV, O_RDONLY | O_NONBLOCK);
	if (keys_fd < 0)
	{
		return -1;
	}
    return 0;
}

int get_io(int key, int key2, int* v1, int* v2)
{
    struct input_event t;

    *v1 = -1;
    *v2 = -1;

    if (read(keys_fd, &t, sizeof(t)) == sizeof(t))
    {

        if (t.type == EV_KEY)
            if (t.value == 0 || t.value == 1)
            {
                printf("key %d %s\n", t.code, (t.value) ? "Pressed" : "Released");
                if(t.code == key)
                {
                    *v1 = t.value;
                    return t.value;
                }
                else if(t.code == key2)
                {
                    *v2 = t.value;
                    return t.value;
                }
                // if (t.code == KEY_ESC)
                //     break;
            }
    }
    return -1;
}

void board_deinit()
{
    close(keys_fd);
}

