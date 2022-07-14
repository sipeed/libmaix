
#include "xxxx_uvai.hpp"

extern "C"
{
  extern xxxx_uv *xxxx;

  void xxxx_ctrl_load()
  {
    xxxx->input_event0 = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    if (xxxx->input_event0 <= 0)
    {
      perror("open /dev/input/event0 device error!\n");
      abort();
    }

    uart_t uart_dev_parm = {
        .baud = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 'n'};
    xxxx->dev_ttyS1 = linux_uart_init((char *)"/dev/ttyS1", &uart_dev_parm);
    if (xxxx->dev_ttyS1 < 0)
    {
      perror(" uart /dev/ttyS1 open err!");
      abort();
    }
    write(xxxx->dev_ttyS1, "xxxx!\r\n", sizeof("xxxx!\r\n"));

    xxxx->timeout.tv_sec = 0;
    xxxx->timeout.tv_usec = 0;
    FD_ZERO(&xxxx->readfd);
    LIBMAIX_DEBUG_PRINTF("xxxx_ctrl_load");
  }

  void xxxx_ctrl_exit()
  {
    close(xxxx->dev_ttyS1);
    close(xxxx->input_event0);
    LIBMAIX_DEBUG_PRINTF("xxxx_ctrl_exit");
  }

  void xxxx_ctrl_loop()
  {
    // CALC_FPS("xxxx_ctrl_loop");

    int ret = 0;

    // serial
    FD_SET(xxxx->dev_ttyS1, &xxxx->readfd);
    ret = select(xxxx->dev_ttyS1 + 1, &xxxx->readfd, NULL, NULL, &xxxx->timeout);
    if (ret != -1 && FD_ISSET(xxxx->dev_ttyS1, &xxxx->readfd))
    {
      char tmp[2] = {0};
      int readByte = read(xxxx->dev_ttyS1, &tmp, 1);
      if (readByte != -1)
      {
        // printf("readByte %d %X\n", readByte, tmp);
      }
    }

    // key
    FD_SET(xxxx->input_event0, &xxxx->readfd);
    ret = select(xxxx->input_event0 + 1, &xxxx->readfd, NULL, NULL, &xxxx->timeout);
    if (ret != -1 && FD_ISSET(xxxx->input_event0, &xxxx->readfd))
    {
      struct input_event event;
      if (read(xxxx->input_event0, &event, sizeof(event)) == sizeof(event))
      {
        if ((event.type == EV_KEY) && (event.value == 0 || event.value == 1))
        {
          printf("keyEvent %d %s\n", event.code, (event.value) ? "Pressed" : "Released");
          if (event.value == 0)
          {
            xxxx->exit = 1;
          }
        }
      }
    }

    LIBMAIX_DEBUG_PRINTF("xxxx_ctrl_loop");
  }

}