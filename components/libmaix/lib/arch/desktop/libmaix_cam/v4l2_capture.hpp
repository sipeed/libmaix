/*
Author:Jack-Cui
Blog:http://blog.csdn.net/c406495762
Time:25 May 2017
*/
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <pthread.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <iomanip>
#include <string>

#define V4L2_PRINTF(fmt, ...) // printf("{%s:%d}[INFO:%s]( " fmt " )\r\n" , __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

using namespace std;

#define CLEAR(x) memset(&(x), 0, sizeof(x))

// #define IMAGEWIDTH 3264
// #define IMAGEHEIGHT 2448

class V4L2Capture
{
public:
  V4L2Capture(char *devName, int width, int height);
  virtual ~V4L2Capture();

  int openDevice();
  int closeDevice();
  int initDevice();
  int startCapture();
  int stopCapture();
  int freeBuffers();
  int getFrame(void **, size_t *);
  int backFrame();
  static void test();

  int capW;
  int capH;
private:
  int initBuffers();

  struct cam_buffer
  {
    void *start;
    unsigned int length;
  };
  char *devName;
  int fd_cam;
  cam_buffer *buffers;
  unsigned int n_buffers;
  int frameIndex;
};

V4L2Capture::V4L2Capture(char *devName, int width, int height)
{
  // TODO Auto-generated constructor stub
  this->devName = devName;
  this->fd_cam = -1;
  this->buffers = NULL;
  this->n_buffers = 0;
  this->frameIndex = -1;
  this->capW = width;
  this->capH = height;
}

V4L2Capture::~V4L2Capture()
{
  // TODO Auto-generated destructor stub
}

int V4L2Capture::openDevice()
{
  /*设备的打开*/
  V4L2_PRINTF("video dev : %s\n", devName);
  fd_cam = open(devName, O_RDWR);
  if (fd_cam < 0)
  {
    V4L2_PRINTF("Can't open video device");
  }
  return 0;
}

int V4L2Capture::closeDevice()
{
  if (fd_cam > 0)
  {
    int ret = 0;
    if ((ret = close(fd_cam)) < 0)
    {
      V4L2_PRINTF("Can't close video device");
    }
    return 0;
  }
  else
  {
    return -1;
  }
}

int V4L2Capture::initDevice()
{
  int ret;
  struct v4l2_capability cam_cap;  //显示设备信息
  struct v4l2_cropcap cam_cropcap; //设置摄像头的捕捉能力
  struct v4l2_fmtdesc cam_fmtdesc; //查询所有支持的格式：VIDIOC_ENUM_FMT
  struct v4l2_crop cam_crop;       //图像的缩放
  struct v4l2_format cam_format;   //设置摄像头的视频制式、帧格式等

  /* 使用IOCTL命令VIDIOC_QUERYCAP，获取摄像头的基本信息*/
  ret = ioctl(fd_cam, VIDIOC_QUERYCAP, &cam_cap);
  if (ret < 0)
  {
    V4L2_PRINTF("Can't get device information: VIDIOCGCAP");
  }
  V4L2_PRINTF(
      "Driver Name:%s\nCard Name:%s\nBus info:%s\nDriver Version:%u.%u.%u\n",
      cam_cap.driver, cam_cap.card, cam_cap.bus_info,
      (cam_cap.version >> 16) & 0XFF, (cam_cap.version >> 8) & 0XFF,
      cam_cap.version & 0XFF);

  /* 使用IOCTL命令VIDIOC_ENUM_FMT，获取摄像头所有支持的格式*/
  cam_fmtdesc.index = 0;
  cam_fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  V4L2_PRINTF("Support format:\n");
  while (ioctl(fd_cam, VIDIOC_ENUM_FMT, &cam_fmtdesc) != -1)
  {
    V4L2_PRINTF("\t%d.%s\n", cam_fmtdesc.index + 1, cam_fmtdesc.description);
    cam_fmtdesc.index++;
  }

  /* 使用IOCTL命令VIDIOC_CROPCAP，获取摄像头的捕捉能力*/
  cam_cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (0 == ioctl(fd_cam, VIDIOC_CROPCAP, &cam_cropcap))
  {
    V4L2_PRINTF("Default rec:\n\tleft:%d\n\ttop:%d\n\twidth:%d\n\theight:%d\n",
           cam_cropcap.defrect.left, cam_cropcap.defrect.top,
           cam_cropcap.defrect.width, cam_cropcap.defrect.height);
    /* 使用IOCTL命令VIDIOC_S_CROP，获取摄像头的窗口取景参数*/
    cam_crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    cam_crop.c = cam_cropcap.defrect; //默认取景窗口大小
    if (-1 == ioctl(fd_cam, VIDIOC_S_CROP, &cam_crop))
    {
      //V4L2_PRINTF("Can't set crop para\n");
    }
  }
  else
  {
    V4L2_PRINTF("Can't set cropcap para\n");
  }

  /* 使用IOCTL命令VIDIOC_S_FMT，设置摄像头帧信息*/
  cam_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  cam_format.fmt.pix.width = capW;// cam_cropcap.defrect.width;
  cam_format.fmt.pix.height = capH;// cam_cropcap.defrect.height;
  cam_format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; //要和摄像头支持的类型对应 V4L2_PIX_FMT_MJPEG or V4L2_PIX_FMT_YUYV
  cam_format.fmt.pix.field = V4L2_FIELD_INTERLACED;
  ret = ioctl(fd_cam, VIDIOC_S_FMT, &cam_format);
  if (ret < 0)
  {
    V4L2_PRINTF("Can't set frame information");
  }
  /* 使用IOCTL命令VIDIOC_G_FMT，获取摄像头帧信息*/
  cam_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ret = ioctl(fd_cam, VIDIOC_G_FMT, &cam_format);
  if (ret < 0)
  {
    V4L2_PRINTF("Can't get frame information");
  }
  
  printf("[v4l2] Current data format information:\n\twidth:%d\n\theight:%d\n\tpixelformat:%X\n",
         cam_format.fmt.pix.width, cam_format.fmt.pix.height, cam_format.fmt.pix.pixelformat);
  capW = cam_format.fmt.pix.width;// cam_cropcap.defrect.width;
  capH = cam_format.fmt.pix.height;// cam_cropcap.defrect.height;
  ret = initBuffers();
  if (ret < 0)
  {
    V4L2_PRINTF("Buffers init error");
    //exit(-1);
  }
  // {
  //   struct v4l2_control ctrl;
  //   // V4L2_CID_HFLIP V4L2_CID_VFLIP 水平或者垂直翻转
  //   ctrl.id = V4L2_CID_HFLIP;
  //   ctrl.value = 0; // 0 或者 1 翻转或者不翻转
  //   ioctl(fd_cam, VIDIOC_S_CTRL, &ctrl);
  // }
  // {
  //   struct v4l2_control ctrl;
  //   // V4L2_CID_HFLIP V4L2_CID_VFLIP 水平或者垂直翻转
  //   ctrl.id = V4L2_CID_VFLIP;
  //   ctrl.value = 0; // 0 或者 1 翻转或者不翻转
  //   ioctl(fd_cam, VIDIOC_S_CTRL, &ctrl);
  // }
  return 0;
}

int V4L2Capture::initBuffers()
{
  int ret;
  /* 使用IOCTL命令VIDIOC_REQBUFS，申请帧缓冲*/
  struct v4l2_requestbuffers req;
  CLEAR(req);
  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  ret = ioctl(fd_cam, VIDIOC_REQBUFS, &req);
  if (ret < 0)
  {
    V4L2_PRINTF("Request frame buffers failed");
  }
  if (req.count < 2)
  {
    V4L2_PRINTF("Request frame buffers while insufficient buffer memory");
  }
  buffers = (struct cam_buffer *)calloc(req.count, sizeof(*buffers));
  if (!buffers)
  {
    V4L2_PRINTF("Out of memory");
  }
  for (n_buffers = 0; n_buffers < req.count; n_buffers++)
  {
    struct v4l2_buffer buf;
    CLEAR(buf);
    // 查询序号为n_buffers 的缓冲区，得到其起始物理地址和大小
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n_buffers;
    ret = ioctl(fd_cam, VIDIOC_QUERYBUF, &buf);
    if (ret < 0)
    {
      V4L2_PRINTF("VIDIOC_QUERYBUF %d failed\n", n_buffers);
      return -1;
    }
    buffers[n_buffers].length = buf.length;
    //V4L2_PRINTF("buf.length= %d\n",buf.length);
    // 映射内存
    buffers[n_buffers].start = mmap(
        NULL, // start anywhere
        buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd_cam,
        buf.m.offset);
    if (MAP_FAILED == buffers[n_buffers].start)
    {
      V4L2_PRINTF("mmap buffer%d failed\n", n_buffers);
      return -1;
    }
  }
  return 0;
}

int V4L2Capture::startCapture()
{
  unsigned int i;
  for (i = 0; i < n_buffers; i++)
  {
    struct v4l2_buffer buf;
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if (-1 == ioctl(fd_cam, VIDIOC_QBUF, &buf))
    {
      V4L2_PRINTF("VIDIOC_QBUF buffer%d failed\n", i);
      return -1;
    }
  }
  enum v4l2_buf_type type;
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == ioctl(fd_cam, VIDIOC_STREAMON, &type))
  {
    V4L2_PRINTF("VIDIOC_STREAMON error");
    return -1;
  }
  return 0;
}

int V4L2Capture::stopCapture()
{
  enum v4l2_buf_type type;
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == ioctl(fd_cam, VIDIOC_STREAMOFF, &type))
  {
    V4L2_PRINTF("VIDIOC_STREAMOFF error\n");
    return -1;
  }
  return 0;
}

int V4L2Capture::freeBuffers()
{
  unsigned int i;
  for (i = 0; i < n_buffers; ++i)
  {
    if (-1 == munmap(buffers[i].start, buffers[i].length))
    {
      V4L2_PRINTF("munmap buffer%d failed\n", i);
      return -1;
    }
  }
  free(buffers);
  return 0;
}

int V4L2Capture::getFrame(void **frame_buf, size_t *len)
{
  struct v4l2_buffer queue_buf;
  CLEAR(queue_buf);
  queue_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  queue_buf.memory = V4L2_MEMORY_MMAP;
  if (-1 == ioctl(fd_cam, VIDIOC_DQBUF, &queue_buf))
  {
    // V4L2_PRINTF("VIDIOC_DQBUF error\n");
    return -1;
  }
  *frame_buf = buffers[queue_buf.index].start;
  *len = buffers[queue_buf.index].length;
  frameIndex = queue_buf.index;
  return 0;
}

int V4L2Capture::backFrame()
{
  if (frameIndex != -1)
  {
    struct v4l2_buffer queue_buf;
    CLEAR(queue_buf);
    queue_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queue_buf.memory = V4L2_MEMORY_MMAP;
    queue_buf.index = frameIndex;
    if (-1 == ioctl(fd_cam, VIDIOC_QBUF, &queue_buf))
    {
      V4L2_PRINTF("VIDIOC_QBUF error\n");
      return -1;
    }
    return 0;
  }
  return -1;
}

#ifdef unit_test_v4l2_capture

int v4l2_capture()
{
  unsigned char *yuv422frame = NULL;
  unsigned long yuvframeSize = 0, w = 0, h = 0;

  string videoDev = "/dev/video0";
  V4L2Capture *vcap = new V4L2Capture(const_cast<char *>(videoDev.c_str()),
                                      w, h);
  vcap->openDevice();
  vcap->initDevice();
  vcap->startCapture();

  for (int i = 0; i < 5; i++)
  {

    vcap->getFrame((void **)&yuv422frame, (size_t *)&yuvframeSize);

    V4L2_PRINTF("yuv422frame\r\n");

    FILE *fp = NULL;

    fp = fopen("./test.yuv", "w+");

    fwrite(yuv422frame, yuvframeSize, 1, fp);

    fclose(fp);

    vcap->backFrame();
  }

  vcap->freeBuffers();
  vcap->closeDevice();
}

#endif