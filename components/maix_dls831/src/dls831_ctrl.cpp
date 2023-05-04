
#include "dls831_uvai.hpp"

extern "C"
{
  extern dls831_uv *dls831;

  #include <termios.h>
  #include <unistd.h>


  void dls831_ctrl_load()
  {
    dls831->input_event0 = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    if (dls831->input_event0 <= 0)
    {
      perror("open /dev/input/event0 device error!\n");
      abort();
    }

    int ret = 0;
    uart_t uart_dev_parm = {
    .baud = 115200,
    .data_bits = 8,
    .stop_bits = 1,
    .parity = 'n'};
    dls831->dev_ttyS1 = linux_uart_init((char *)"/dev/ttyS1", &uart_dev_parm);
    if (dls831->dev_ttyS1 < 0)
    {
      perror(" uart /dev/ttyS1 open err!");
      abort();
    }
    FD_ZERO(&dls831->readfd);
    write(dls831->dev_ttyS1, "dls831!\r\n", sizeof("dls831!\r\n"));
    dls831->timeout.tv_sec = 0;
    dls831->timeout.tv_usec = 0;

    dls831->sensor_time = dls831_get_ms();


    LIBMAIX_DEBUG_PRINTF("dls831_ctrl_load");
  }

  void dls831_ctrl_exit()
  {
    close(dls831->dev_ttyS1);
    LIBMAIX_DEBUG_PRINTF("dls831_ctrl_exit");
  }

  void dls831_ctrl_loop()
  {
    // CALC_FPS("dls831_ctrl_loop");

    // int ret = 0;

    // ret = dls831_get_ms() - dls831->sensor_time;
    // if (ret > 10000) // soft-keep
    // {
    //   dls831->exit = 1;
    //   system("reboot");
    //   exit(0);
    // }
    // else
    // {
    //   // system(string_format("echo %d > /tmp/sync", ret).c_str());
    // }

    int ret = 0;

    // serial
    FD_SET(dls831->dev_ttyS1, &dls831->readfd);
    ret = select(dls831->dev_ttyS1 + 1, &dls831->readfd, NULL, NULL, &dls831->timeout);
    if (ret != -1 && FD_ISSET(dls831->dev_ttyS1, &dls831->readfd))
    {
      char tmp[2] = {0};
      int readByte = read(dls831->dev_ttyS1, &tmp, 1);
      if (readByte != -1)
      {
        printf("readByte %d %X\n", readByte, tmp);
      }
    }

    // key
    FD_SET(dls831->input_event0, &dls831->readfd);
    ret = select(dls831->input_event0 + 1, &dls831->readfd, NULL, NULL, &dls831->timeout);
    if (ret != -1 && FD_ISSET(dls831->input_event0, &dls831->readfd))
    {
      struct input_event event;
      if (read(dls831->input_event0, &event, sizeof(event)) == sizeof(event))
      {
        if ((event.type == EV_KEY) && (event.value == 0 || event.value == 1))
        {
          printf("keyEvent %d %s\n", event.code, (event.value) ? "Pressed" : "Released");
          if (event.value == 0)
          {
            dls831->exit = 1;
          }
        }
      }
    }

    LIBMAIX_DEBUG_PRINTF("dls831_ctrl_loop");
  }
}