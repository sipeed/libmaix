
#!/usr/bin/env python
# -*- coding:utf-8 -*-

import re
import struct
import time


def convert_hex(string):
    res = []
    result = []
    for item in string:
        res.append(item)
    for i in res:
        result.append(hex(i))
    return result


# def packTagDat(id, decision, x_rol, y_rol, z_rol):
#     """
#     拼接标签数据
#     :param id:
#     :param decision:
#     :param x_rol:
#     :param y_rol:
#     :param z_rol:
#     :return:
#     """
#     return struct.pack(">HffffdBB", id, decision, x_rol, y_rol, z_rol, time.time(), 0x0d, 0x0a)


# def pack(cmd, dat):
#     """
#     添加头部校验长度和指令
#     :param cmd:
#     :param dat:
#     :return:
#     """
#     dat = struct.pack(">B", cmd) + dat
#     dat = struct.pack(">b", len(dat)) + dat
#     packCrc = 0
#     for i in dat:
#         packCrc = packCrc ^ i
#     dat = struct.pack('>BB', 0x55, packCrc) + dat
#     return dat


# def checkDat(dat):
#     """
#     校验数据
#     :param dat:
#     :return:
#     """
#     cmd = 0
#     res = None
#     if len(dat) > 3:
#         if dat[0] == 0x55:
#             if dat[2] == len(dat) - 3:
#                 packCrc = 0
#                 for i in range(2, len(dat)):
#                     packCrc = packCrc ^ dat[i]

#                 if dat[1] == packCrc:
#                     cmd = dat[3]
#                     res = dat[4:len(dat) - 2]

#     return cmd, res


# def unpack(dat):
#     """
#     解包
#     :param dat:
#     :return:
#     """
#     cmd = dat[0]
#     dat = dat[1]
#     if cmd == 0:
#         return
#     elif cmd == 1:
#         time_ = struct.unpack('>d', dat)
#         print("校准时间", time_[0])
#         print("v831收到之后，换这个时间戳设置自已的时间，之后发标签数据带上时间戳")

#     elif cmd == 2:
#         id, decision, x_rol, y_rol, z_rol, time_ = struct.unpack(">Hffffd", dat)
#         print("收到标签数据", id, decision, x_rol, y_rol, z_rol, time_)


# if __name__ == '__main__':
#     dat = pack(0x01, struct.pack(">dBB", time.time(), 0x0d, 0x0a))
#     print("1. 上位机组一个校准时间的包发给V831", convert_hex(dat))
#     print(
#         '''
#         [
#         '0x55',       头
#         '0x1a',       校验
#         '0xb',        长度
#         '0x1',        指令（0x01表示校准时间）
#         '0x41', '0xd8', '0xae', '0x42', '0x9e', '0x73', '0x4e', '0xc1',        时间
#         '0xd', '0xa'] 换行
#         '''
#     )
#     unpack(checkDat(dat))

#     dat = pack(0x02, packTagDat(287, 99.99, 71.22, 22.36, 55.55))
#     print("2. v831发布标签数据", convert_hex(dat))
#     print(
#         '''
#         [
#         '0x55',       头
#         '0x48',       校验
#         '0x1d',       长度
#         '0x2',        指令(0x02表示标签数据)
#         '0x1', '0x1f',        id
#         '0x42', '0xc7', '0xfa', '0xe1',   距离
#         '0x42', '0x8e', '0x70', '0xa4',   x_rol
#         '0x41', '0xb2', '0xe1', '0x48',   y_rol
#         '0x42', '0x5e', '0x33', '0x33',   z_rol
#         '0x41', '0xd8', '0xae', '0x44', '0x51', '0xfd', '0xe9', '0xb8',   时间
#         '0xd', '0xa'] 换行
#         '''
#     )
#     unpack(checkDat(dat))

def checksum(data):
  sum = 0
  for i in range(len(data)):
      sum += data[i]
      # print(i, data[i], sum)
  return sum & 0xff

import serial
import time
import sys

ser = serial.Serial("/dev/ttyUSB0", 115200)    # 连接串口

'''
      struct apriltag_data {
        uint8_t head;
        uint8_t len;
        uint8_t retain_0;
        uint8_t retain_1;
        uint32_t tm;
        uint32_t id;
        float decision_margin;
        float center[2];
        float points[4][2];
        float rotation[3][3];
        uint8_t retain_2;
        uint8_t retain_3;
        uint8_t sum;
        uint8_t end;
      } upload_data = { 0x55, sizeof(struct apriltag_data), 0, 0, gs831_get_ms(), 0, 0, { 0, 0 }, { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } }, { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }}, 0, 0, 0, 0x0A };
'''


data, sum = b'', 0
while True:
  dat = ser.read(1)
  if dat[0] == 0x55:
    data = dat
    dat = ser.read(1) # datlen
    data += dat
    # print(len(data), data)
    dat = ser.readline()
    # print(data[1], len(dat), dat)
    if data[1] - 2 == len(dat):
      data += dat
      sum = checksum(data[:-2])
      # print(len(data), data)
      if sum == data[-2]:
        res = struct.unpack('<BBBBIIffffffffffffffffffffBBbB', data)
        # print(res[4:-4])
        print("%02.03f %02.03f %02.03f %02.03f %02.03f %02.03f %02.03f %02.03f %02.03f" % (res[17:-4]))
      continue

sys.exit(0)

while True:

  time.sleep(2)

  ser.write(b'\x86\xAB\x00\x0B\xE8\x15\x02\x00\x00\x3B\xCF')

  time.sleep(2)

  ser.write(b'\x86\xAB\x00\x0B\xE8\x15\x05\x00\x00\x3F\xCF')

  print(time.asctime())

  time.sleep(2)

  ser.write(b'\x86\xAB\x00\x0B\xE8\x15\x07\x00\x00\x41\xCF')

  print(time.asctime())
