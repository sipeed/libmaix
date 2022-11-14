
> yolov5s models from https://github.com/AXERA-TECH/ax-models/blob/main/ax620/yolov5s.joint
> yolov5s code from https://github.com/AXERA-TECH/ax-samples/blob/main/examples/ax_yolov5s_steps.cc

## run code

```
fbon
cd /home/libmaix/examples/axpi_yolov5_cam
python3 project.py build
./dist/axpi_yolov5_cam -m /home/models/yolov5s.joint
```

## result

```
root@AXERA:/home/libmaix/examples/axpi_yolov5_cam# python3 project.py build
-- SDK_PATH:/home/libmaix
-- project name: axpi_yolov5_cam
build now
[  3%] Built target gen_exe_src
-- Update build time and version info to header  config  at: ('/home/libmaix/examples/axpi_yolov5_cam/bu)
[  3%] Built target update_build_info
[ 11%] Built target maix_cv_image
[ 81%] Built target libmaix
[ 88%] Built target main
[100%] Built target axpi_yolov5_cam
==================================
build end, time last:2.02s
==================================
root@AXERA:/home/libmaix/examples/axpi_yolov5_cam# ./dist/axpi_yolov5_cam -m /home/models/yolov5s.joint
--------------------------------------
model file : /home/models/yolov5s.joint
img_h, img_w : 640 640
[libmaix_camera_module_init]-90: ISP Sample. Build at Nov 14 2022 10:45:05
[AX_SYS_LOG] AX_SYS_Log2ConsoleThread_Start
AX_POOL_SetConfig success!
[COMMON_SYS_Init]-85: AX_POOL_Init success!
[COMMON_ISP_GetI2cDevNode]-448: get board_id = 2
[RegisterSns]-556: set sensor bus idx 0
[sample_isp][COMMON_CAM_Open][167] pFile
[COMMON_CAM_Open]-170: AX_ISP_LoadBinParams  will user sensor.h
Cannot open /sys/class/gpio/gpio496/direction.
Cannot open /sys/class/gpio/gpio496/direction.
i2c_init: i2c device is /dev/i2c-0
ISP IFE INIT done.
ISP ITP INIT done.
[libmaix_camera_module_init]-158: camera 0 is open
[IspRun]-79: cam 0 is running...
AX_IVPS_Init
[framebuffer](480,854, 32bpp)
Run-Joint Runtime version: 0.5.10
----------------------------
[INFO]: Virtual npu mode is 1_1

Tools version: d696ee2f
time cost of get cam�—img : 184
time cost of resize image : 18
time cost of get image : 202
3. init
time cost of step2-3 : 3
4.run & benchmark
run over: output len 3
time cost of run : 25
5.get bbox
time cost of generate_prop : 6
time cost of outbbox : 0
object size 1
71:  63%, [   6,  251,  316,  423], sink
time cost of bbox : 2
time cost of delete IObuffer : 54
time cost of get cam�—img : 137
time cost of resize image : 23
time cost of get image : 160
3. init
time cost of step2-3 : 3
4.run & benchmark
run over: output len 3
time cost of run : 25
5.get bbox

```
