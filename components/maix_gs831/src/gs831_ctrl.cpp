
#include "gs831_uvai.hpp"

extern "C"
{
  extern gs831_uv *gs831;

  void gs831_ctrl_load()
  {
    gs831->input_event0 = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    if (gs831->input_event0 <= 0)
    {
      perror("open /dev/input/event0 device error!\n");
      abort();
    }

    uart_t uart_dev_parm = {
        .baud = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 'n'};
    gs831->dev_ttyS = linux_uart_init((char *)"/dev/ttyS0", &uart_dev_parm);
    if (gs831->dev_ttyS < 0)
    {
      perror(" uart /dev/ttyS1 open err!");
      abort();
    }
    write(gs831->dev_ttyS, "gs831!\r\n", sizeof("gs831!\r\n"));

    gs831->timeout.tv_sec = 0;
    gs831->timeout.tv_usec = 0;
    FD_ZERO(&gs831->readfd);
    LIBMAIX_DEBUG_PRINTF("gs831_ctrl_load");
  }

  void gs831_ctrl_exit()
  {
    close(gs831->dev_ttyS);
    close(gs831->input_event0);
    LIBMAIX_DEBUG_PRINTF("gs831_ctrl_exit");
  }

  void gs831_ctrl_loop()
  {
    // CALC_FPS("gs831_ctrl_loop");

    int ret = 0;

    // serial
    FD_SET(gs831->dev_ttyS, &gs831->readfd);
    ret = select(gs831->dev_ttyS + 1, &gs831->readfd, NULL, NULL, &gs831->timeout);
    if (ret != -1 && FD_ISSET(gs831->dev_ttyS, &gs831->readfd))
    {
      char tmp[2] = {0};
      int readByte = read(gs831->dev_ttyS, &tmp, 1);
      if (readByte != -1)
      {
        printf("readByte %d %X\n", readByte, tmp);
      }
    }

    // key
    FD_SET(gs831->input_event0, &gs831->readfd);
    ret = select(gs831->input_event0 + 1, &gs831->readfd, NULL, NULL, &gs831->timeout);
    if (ret != -1 && FD_ISSET(gs831->input_event0, &gs831->readfd))
    {
      struct input_event event;
      if (read(gs831->input_event0, &event, sizeof(event)) == sizeof(event))
      {
        if ((event.type == EV_KEY) && (event.value == 0 || event.value == 1))
        {
          printf("keyEvent %d %s\n", event.code, (event.value) ? "Pressed" : "Released");
          if (event.value == 0)
          {
            gs831->exit = 1;
          }
        }
      }
    }

    LIBMAIX_DEBUG_PRINTF("gs831_ctrl_loop");
  }

}