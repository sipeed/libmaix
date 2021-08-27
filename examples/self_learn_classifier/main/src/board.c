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

int get_io(int key)
{
    struct input_event t;

    if (read(keys_fd, &t, sizeof(t)) == sizeof(t))
    {

        if (t.type == EV_KEY)
            if (t.value == 0 || t.value == 1)
            {
                printf("key %d %s\n", t.code, (t.value) ? "Pressed" : "Released");
                if(t.code == key)
                    return t.value;
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

