/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include "ax_vin_api.h"
#include "ax_isp_api.h"
#include "ax_mipi_api.h"
#include "ax_sensor_struct.h"
#include "ax_isp_3a_api.h"
#include "ax_buffer_tool.h"
#include "common_cam.h"
#include "common_type.h"
#include "common_config.h"

/* Function declaration for sensor handle */
extern AX_SENSOR_REGISTER_FUNC_T gSnsos04a10Obj;
// extern AX_SENSOR_REGISTER_FUNC_T gSnsimx334Obj;
extern AX_SENSOR_REGISTER_FUNC_T gSnsgc4653Obj;
// extern AX_SENSOR_REGISTER_FUNC_T gSnsos08a20Obj;
extern AX_SENSOR_REGISTER_FUNC_T gSnsos04a10SlaveObj;
extern AX_SENSOR_REGISTER_FUNC_T gSnsos04a10MasterObj;

static AX_IMG_FORMAT_E raw_fmt_2_comm_fmt(AX_RAW_TYPE_E eRawType)
{
    AX_IMG_FORMAT_E img_format = AX_FORMAT_INVALID;

    switch (eRawType) {
    case AX_RT_RAW8:
        img_format = AX_FORMAT_BAYER_RAW_8BPP;
        break;
    case AX_RT_RAW10:
        img_format = AX_FORMAT_BAYER_RAW_10BPP;
        break;
    case AX_RT_RAW12:
        img_format = AX_FORMAT_BAYER_RAW_12BPP;
        break;
    case AX_RT_RAW14:
        img_format = AX_FORMAT_BAYER_RAW_14BPP;
        break;
    case AX_RT_RAW16:
        img_format = AX_FORMAT_BAYER_RAW_16BPP;
        break;
    default:
        COMM_ISP_PRT("comm not support this data type: %d\n", eRawType);
        img_format = AX_FORMAT_BAYER_RAW_10BPP;
        break;
    }

    return img_format;
}

AX_S32 COMMON_ISP_SetMipiAttr(AX_U8 devId, SAMPLE_SNS_TYPE_E eSnsType, AX_BOOL bMaster)
{
    AX_MIPI_RX_ATTR_S tMipiAttr = {0};
    AX_S32 nRet = 0;

    switch (eSnsType) {
    case OMNIVISION_OS04A10:
    case OMNIVISION_OS04A10_MASTER:
    case OMNIVISION_OS04A10_SLAVE:
        memcpy(&tMipiAttr, &gOs04a10MipiAttr, sizeof(AX_MIPI_RX_ATTR_S));
        break;
    case SONY_IMX334:
        memcpy(&tMipiAttr, &gImx334MipiAttr, sizeof(AX_MIPI_RX_ATTR_S));
        break;
    case GALAXYCORE_GC4653:
        memcpy(&tMipiAttr, &gGc4653MipiAttr, sizeof(AX_MIPI_RX_ATTR_S));
        break;
    case OMNIVISION_OS08A20:
        memcpy(&tMipiAttr, &gOs08a20MipiAttr, sizeof(AX_MIPI_RX_ATTR_S));
        break;
    case MIPI_YUV:
        memcpy(&tMipiAttr, &gMIPI_YUVMipiAttr, sizeof(AX_MIPI_RX_ATTR_S));
        break;
    default:
        memcpy(&tMipiAttr, &gOs04a10MipiAttr, sizeof(AX_MIPI_RX_ATTR_S));
        break;
    }

    nRet = AX_MIPI_RX_Reset(devId);
    if (0 != nRet) {
        COMM_PRT("failed, ret=0x%x.\n", nRet);
        return -1;
    }

    if (devId == 0) {
        tMipiAttr.ePhySel = AX_MIPI_RX_PHY0_SEL_LANE_0_1_2_3;
    } else if (devId == 1) {
        tMipiAttr.ePhySel = AX_MIPI_RX_PHY1_SEL_LANE_0_1_2_3;
    } else if (devId == 2) {
        if (bMaster == AX_TRUE) {
            tMipiAttr.ePhySel = AX_MIPI_RX_PHY1_SEL_LANE_0_1_2_3;
        } else {
            tMipiAttr.ePhySel = AX_MIPI_RX_PHY2_SEL_LANE_0_1_2_3;
        }
    } else {
        COMM_PRT("devId = %d ePhySel is not supported.\n", devId);
        return -1;
    }

    nRet = AX_MIPI_RX_SetAttr(devId, &tMipiAttr);
    if (0 != nRet) {
        COMM_PRT("AX_MIPI_RX_SetAttr failed, ret=0x%x.\n", nRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_ISP_SetSnsAttr(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType, AX_RAW_TYPE_E eRawType, AX_SNS_HDR_MODE_E eHdrMode)
{
    AX_SNS_ATTR_T tSnsAttr = {0};
    AX_S32 nRet = 0;

    switch (eSnsType) {
    case OMNIVISION_OS04A10:
    case OMNIVISION_OS04A10_MASTER:
    case OMNIVISION_OS04A10_SLAVE:
        memcpy(&tSnsAttr, &gOs04a10SnsAttr, sizeof(AX_SNS_ATTR_T));
        break;
    case SONY_IMX334:
        memcpy(&tSnsAttr, &gImx334SnsAttr, sizeof(AX_SNS_ATTR_T));
        break;
    case GALAXYCORE_GC4653:
        memcpy(&tSnsAttr, &gGc4653SnsAttr, sizeof(AX_SNS_ATTR_T));
        break;
    case OMNIVISION_OS08A20:
        memcpy(&tSnsAttr, &gOs08a20SnsAttr, sizeof(AX_SNS_ATTR_T));
        break;
    case SENSOR_DVP:
        memcpy(&tSnsAttr, &gDVPSnsAttr, sizeof(AX_SNS_ATTR_T));
        break;
    default:
        memcpy(&tSnsAttr, &gOs04a10SnsAttr, sizeof(AX_SNS_ATTR_T));
        break;
    }

    tSnsAttr.eRawType = eRawType;
    tSnsAttr.eSnsMode = eHdrMode;

    nRet = AX_VIN_SetSnsAttr(pipe, &tSnsAttr);
    if (0 != nRet) {
        COMM_PRT("AX_VIN_SetSnsAttr failed, nRet=0x%x.\n", nRet);
        return -1;
    }

    return 0;
}


AX_S32 COMMON_ISP_GetSnsConfig(SAMPLE_SNS_TYPE_E eSnsType, AX_SNS_ATTR_T *ptSnsAttr, AX_SNS_CLK_ATTR_T *ptSnsClkAttr, AX_DEV_ATTR_T *pDevAttr,
                               AX_PIPE_ATTR_T *pPipeAttr, AX_VIN_CHN_ATTR_T *pChnAttr)
{
    switch (eSnsType) {
    case OMNIVISION_OS04A10:
    case OMNIVISION_OS04A10_MASTER:
    case OMNIVISION_OS04A10_SLAVE:
        memcpy(ptSnsAttr, &gOs04a10SnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gOs04a10SnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gOs04a10DevAttr, sizeof(AX_DEV_ATTR_T));
        memcpy(pPipeAttr, &gOs04a10PipeAttr, sizeof(AX_PIPE_ATTR_T));
        memcpy(pChnAttr, &gOs04a10ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SONY_IMX334:
        memcpy(ptSnsAttr, &gImx334SnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gImx334SnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gImx334DevAttr, sizeof(AX_DEV_ATTR_T));
        memcpy(pPipeAttr, &gImx334PipeAttr, sizeof(AX_PIPE_ATTR_T));
        memcpy(pChnAttr, &gImx334ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case GALAXYCORE_GC4653:
        memcpy(ptSnsAttr, &gGc4653SnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gGc4653SnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gGc4653DevAttr, sizeof(AX_DEV_ATTR_T));
        memcpy(pPipeAttr, &gGc4653PipeAttr, sizeof(AX_PIPE_ATTR_T));
        memcpy(pChnAttr, &gGc4653ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case OMNIVISION_OS08A20:
        memcpy(ptSnsAttr, &gOs08a20SnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gOs08a20SnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gOs08a20DevAttr, sizeof(AX_DEV_ATTR_T));
        memcpy(pPipeAttr, &gOs08a20PipeAttr, sizeof(AX_PIPE_ATTR_T));
        memcpy(pChnAttr, &gOs08a20ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SENSOR_DVP:
        memcpy(ptSnsAttr, &gDVPSnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gDVPSnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gDVPDevAttr, sizeof(AX_DEV_ATTR_T));
        memcpy(pPipeAttr, &gDVPPipeAttr, sizeof(AX_PIPE_ATTR_T));
        memcpy(pChnAttr, &gDVPChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SENSOR_BT601:
        memcpy(pDevAttr, &gBT601DevAttr, sizeof(AX_DEV_ATTR_T));
        memcpy(pPipeAttr, &gBT601PipeAttr, sizeof(AX_PIPE_ATTR_T));
        memcpy(pChnAttr, &gBT601ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SENSOR_BT656:
        memcpy(pDevAttr, &gBT656DevAttr, sizeof(AX_DEV_ATTR_T));
        memcpy(pPipeAttr, &gBT656PipeAttr, sizeof(AX_PIPE_ATTR_T));
        memcpy(pChnAttr, &gBT656ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SENSOR_BT1120:
        memcpy(pDevAttr, &gBT1120DevAttr, sizeof(AX_DEV_ATTR_T));
        memcpy(pPipeAttr, &gBT1120PipeAttr, sizeof(AX_PIPE_ATTR_T));
        memcpy(pChnAttr, &gBT1120ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case MIPI_YUV:
        memcpy(pDevAttr, &gMIPI_YUVDevAttr, sizeof(AX_DEV_ATTR_T));
        memcpy(pPipeAttr, &gMIPI_YUVPipeAttr, sizeof(AX_PIPE_ATTR_T));
        memcpy(pChnAttr, &gMIPI_YUVChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    default:
        memcpy(ptSnsAttr, &gOs04a10SnsAttr, sizeof(AX_SNS_ATTR_T));
        memcpy(ptSnsClkAttr, &gOs04a10SnsClkAttr, sizeof(AX_SNS_CLK_ATTR_T));
        memcpy(pDevAttr, &gOs04a10DevAttr, sizeof(AX_DEV_ATTR_T));
        memcpy(pPipeAttr, &gOs04a10PipeAttr, sizeof(AX_PIPE_ATTR_T));
        memcpy(pChnAttr, &gOs04a10ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    }

    return 0;
}

AX_S32 COMMON_ISP_SetDevAttr(AX_U8 devId, SAMPLE_SNS_TYPE_E eSnsType, AX_IMG_FORMAT_E ePixelFmt,
                             AX_SNS_HDR_MODE_E eHdrMode)
{
    AX_DEV_ATTR_T tDevAttr = {0};
    AX_S32 nRet = 0;

    switch (eSnsType) {
    case OMNIVISION_OS04A10:
    case OMNIVISION_OS04A10_MASTER:
    case OMNIVISION_OS04A10_SLAVE:
        memcpy(&tDevAttr, &gOs04a10DevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    case SONY_IMX334:
        memcpy(&tDevAttr, &gImx334DevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    case GALAXYCORE_GC4653:
        memcpy(&tDevAttr, &gGc4653DevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    case OMNIVISION_OS08A20:
        memcpy(&tDevAttr, &gOs08a20DevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    case SENSOR_DVP:
        memcpy(&tDevAttr, &gDVPDevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    case SENSOR_BT601:
        memcpy(&tDevAttr, &gBT601DevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    case SENSOR_BT656:
        memcpy(&tDevAttr, &gBT656DevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    case SENSOR_BT1120:
        memcpy(&tDevAttr, &gBT1120DevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    case MIPI_YUV:
        memcpy(&tDevAttr, &gMIPI_YUVDevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    default:
        memcpy(&tDevAttr, &gOs04a10DevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    }

    tDevAttr.ePixelFmt = ePixelFmt;
    tDevAttr.eSnsMode = eHdrMode;

    nRet = AX_VIN_SetDevAttr(devId, &tDevAttr);
    if (0 != nRet) {
        COMM_PRT("AX_VIN_SetDevAttr failed, nRet=0x%x.\n", nRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_ISP_SetPipeAttr(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType, AX_IMG_FORMAT_E ePixelFmt,
                              AX_SNS_HDR_MODE_E eHdrMode)
{
    AX_PIPE_ATTR_T tPipeAttr = {0};
    AX_S32 nRet = 0;

    switch (eSnsType) {
    case OMNIVISION_OS04A10:
    case OMNIVISION_OS04A10_MASTER:
    case OMNIVISION_OS04A10_SLAVE:
        memcpy(&tPipeAttr, &gOs04a10PipeAttr, sizeof(AX_PIPE_ATTR_T));
        break;
    case SONY_IMX334:
        memcpy(&tPipeAttr, &gImx334PipeAttr, sizeof(AX_PIPE_ATTR_T));
        break;
    case GALAXYCORE_GC4653:
        memcpy(&tPipeAttr, &gGc4653PipeAttr, sizeof(AX_PIPE_ATTR_T));
        break;
    case OMNIVISION_OS08A20:
        memcpy(&tPipeAttr, &gOs08a20PipeAttr, sizeof(AX_PIPE_ATTR_T));
        break;
    case SENSOR_DVP:
        memcpy(&tPipeAttr, &gDVPPipeAttr, sizeof(AX_PIPE_ATTR_T));
        break;
    case SENSOR_BT601:
        memcpy(&tPipeAttr, &gBT601PipeAttr, sizeof(AX_PIPE_ATTR_T));
        break;
    case SENSOR_BT656:
        memcpy(&tPipeAttr, &gBT656PipeAttr, sizeof(AX_PIPE_ATTR_T));
        break;
    case SENSOR_BT1120:
        memcpy(&tPipeAttr, &gBT1120PipeAttr, sizeof(AX_PIPE_ATTR_T));
        break;
    case MIPI_YUV:
        memcpy(&tPipeAttr, &gMIPI_YUVPipeAttr, sizeof(AX_PIPE_ATTR_T));
        break;
    default:
        memcpy(&tPipeAttr, &gOs04a10PipeAttr, sizeof(AX_PIPE_ATTR_T));
        break;
    }

    tPipeAttr.eSnsMode = eHdrMode; /* AX_SNS_HDR_MODE_E */
    tPipeAttr.ePixelFmt = ePixelFmt;

    /*change hde mode*/
    nRet = AX_VIN_SetPipeAttr(pipe, &tPipeAttr);
    if (0 != nRet) {
        COMM_PRT("AX_VI_SetPipeAttr failed, nRet = 0x%x.\n", nRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_ISP_SetChnAttr(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType)
{
    AX_VIN_CHN_ATTR_T tChnAttr = {0};
    AX_S32 nRet = 0;

    switch (eSnsType) {
    case OMNIVISION_OS04A10:
    case OMNIVISION_OS04A10_MASTER:
    case OMNIVISION_OS04A10_SLAVE:
        memcpy(&tChnAttr, &gOs04a10ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SONY_IMX334:
        memcpy(&tChnAttr, &gImx334ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case GALAXYCORE_GC4653:
        memcpy(&tChnAttr, &gGc4653ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case OMNIVISION_OS08A20:
        memcpy(&tChnAttr, &gOs08a20ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SENSOR_DVP:
        memcpy(&tChnAttr, &gDVPChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SENSOR_BT601:
        memcpy(&tChnAttr, &gBT601ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SENSOR_BT656:
        memcpy(&tChnAttr, &gBT656ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case SENSOR_BT1120:
        memcpy(&tChnAttr, &gBT1120ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    case MIPI_YUV:
        memcpy(&tChnAttr, &gMIPI_YUVChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    default:
        memcpy(&tChnAttr, &gOs04a10ChnAttr, sizeof(AX_VIN_CHN_ATTR_T));
        break;
    }

    nRet = AX_VIN_SetChnAttr(pipe, &tChnAttr);
    if (0 != nRet) {
        COMM_PRT("AX_VIN_SetChnAttr failed, nRet = 0x%x.\n", nRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_ISP_GetSnsAttr(AX_U8 pipe, AX_SNS_ATTR_T *ptSnsAttr)
{
    AX_S32 nRet = 0;

    nRet = AX_VIN_GetSnsAttr(pipe, ptSnsAttr);
    if (0 != nRet) {
        COMM_PRT("AX_VIN_GetSnsAttr failed, nRet=0x%x.\n", nRet);
        return -1;
    }

    return 0;
}

AX_S32 raw_file_write(char *pfile, char *ptr, int size)
{
    FILE *pFile = NULL;
    AX_S32 nRet = 0;
    AX_S32 w_size = 0;

    pFile = fopen(pfile, "wb");
    if (pFile) {
        COMM_PRT("[%s] is writing...\n", pfile);
        w_size = fwrite(ptr, 1, size, pFile);
        if (w_size == size) {
            COMM_PRT("[%s] wirte raw file success.\n", pfile);
        } else {
            COMM_PRT("[%s] wirte raw file fail, w_size:%d, size:%d\n", pfile, w_size, size);
            nRet = -1;
        }
        fclose(pFile);
    }
    return nRet;
}

/* TODO user need config device node number */
static AX_S8 COMMON_ISP_GetI2cDevNode(AX_U8 nDevId)
{
    AX_S8 nBusNum = 0;
    AX_U8 board_id = 0;
    FILE *pFile = NULL;
    AX_CHAR id[10] = {0};

    pFile = fopen("/sys/devices/platform/hwinfo/board_id", "r");
    if (pFile) {
        fread(&id[0], 10, 1, pFile);
        fclose(pFile);
    } else {
        COMM_PRT("fopen /sys/devices/platform/hwinfo/board_id failed!!!\n");
    }

    board_id = atoi(id);
    if (0 == strncmp("F", id, 1)) {
       board_id = 15;
    }

    COMM_PRT("get board_id = %d\n", board_id);

    if (0 == board_id || 1 == board_id) {
        if (0 == nDevId || 1 == nDevId) {
            nBusNum = 0;
        } else {
            nBusNum = 1;
        }
    } else if (2 == board_id || 3 == board_id || 15 == board_id) {
        if (0 == nDevId) {
            nBusNum = 0;
        } else if (1 == nDevId) {
            nBusNum = 1;
        } else {
            nBusNum = 6;
        }
    } else {
        COMM_PRT("get board id failed, board_id = %d\n", board_id);
        return -1;
    }

    return nBusNum;
}

AX_SENSOR_REGISTER_FUNC_T *COMMON_ISP_GetSnsObj(SAMPLE_SNS_TYPE_E eSnsType)
{
    AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl = NULL;

    switch (eSnsType) {
    case OMNIVISION_OS04A10:
        ptSnsHdl = &gSnsos04a10Obj;
        break;
    case OMNIVISION_OS04A10_MASTER:
        ptSnsHdl = &gSnsos04a10MasterObj;
        break;
    case OMNIVISION_OS04A10_SLAVE:
        ptSnsHdl = &gSnsos04a10SlaveObj;
        break;
    // case SONY_IMX334:
    //     ptSnsHdl = &gSnsimx334Obj;
    //     break;
    case GALAXYCORE_GC4653:
        ptSnsHdl = &gSnsgc4653Obj;
        break;
    // case OMNIVISION_OS08A20:
    //     ptSnsHdl = &gSnsos08a20Obj;
    //     break;
    default:
        ptSnsHdl = &gSnsos04a10Obj;
        break;
    }

    return ptSnsHdl;
}

static AX_SNS_CONNECT_TYPE_E COMMON_ISP_GetSnsBusType(SAMPLE_SNS_TYPE_E eSnsType)
{
    AX_SNS_CONNECT_TYPE_E enBusType;

    switch (eSnsType) {
    case OMNIVISION_OS04A10:
    case OMNIVISION_OS04A10_MASTER:
    case OMNIVISION_OS04A10_SLAVE:
    case SONY_IMX334:
    case GALAXYCORE_GC4653:
    case OMNIVISION_OS08A20:
    case SENSOR_DVP:
    case SENSOR_BT601:
    case SENSOR_BT656:
    case SENSOR_BT1120:
    case MIPI_YUV:
    case SAMPLE_SNS_TYPE_BUTT:
        enBusType = ISP_SNS_CONNECT_I2C_TYPE;
        break;
    default:
        enBusType = ISP_SNS_CONNECT_I2C_TYPE;
        break;
    }

    return enBusType;
}

static AX_S32 RegisterSns(AX_U8 pipe, AX_U8 nDevId, AX_SNS_CONNECT_TYPE_E eBusType, AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl)
{
    AX_S32 axRet = 0;
    AX_SNS_COMMBUS_T tSnsBusInfo = {0};;

    /* ISP Register Sensor */
    axRet = AX_VIN_RegisterSensor(pipe, ptSnsHdl);
    if (axRet) {
        COMM_PRT("AX_ISP Register Sensor Failed, ret=0x%x.\n", axRet);
        return axRet;
    }

    /* confige i2c/spi dev id */
    if (ISP_SNS_CONNECT_I2C_TYPE == eBusType) {
        tSnsBusInfo.I2cDev = COMMON_ISP_GetI2cDevNode(nDevId);
    } else {
        tSnsBusInfo.SpiDev.bit4SpiDev = COMMON_ISP_GetI2cDevNode(nDevId);
        tSnsBusInfo.SpiDev.bit4SpiCs  = 0;
    }

    if (NULL != ptSnsHdl->pfn_sensor_set_bus_info) {
        axRet = ptSnsHdl->pfn_sensor_set_bus_info(pipe, tSnsBusInfo);
        if (0 != axRet) {
            COMM_PRT("set sensor bus info failed with %#x!\n", axRet);
            return axRet;
        }
        COMM_PRT("set sensor bus idx %d\n", tSnsBusInfo.I2cDev);
    } else {
        COMM_PRT("not support set sensor bus info!\n");
        return -1;
    }

    return 0;
}

AX_S32 COMMON_ISP_RegisterSns(AX_U8 pipe, AX_U8 nDevId, SAMPLE_SNS_TYPE_E eSnsType)
{
    AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl = NULL;
    AX_SNS_CONNECT_TYPE_E eBusType;

    /* AX ISP get sensor config */
    ptSnsHdl = COMMON_ISP_GetSnsObj(eSnsType);
    if (NULL == ptSnsHdl) {
        COMM_PRT("AX_ISP Get Sensor Object Failed!\n");
        return -1;
    }

    /* confige i2c/spi dev id */
    eBusType = COMMON_ISP_GetSnsBusType(eSnsType);

    return RegisterSns(pipe, nDevId, eBusType, ptSnsHdl);
}

AX_S32 COMMON_ISP_UnRegisterSns(AX_U8 pipe)
{
    AX_S32 axRet = 0;

    axRet = AX_VIN_UnRegisterSensor(pipe);
    if (axRet) {
        COMM_PRT("AX_ISP Unregister Sensor Failed, ret=0x%x.\n", axRet);
        return axRet;
    }

    return axRet;
}

static AX_S32 RegisterAeAlgLib(AX_U8 pipe, AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl, AX_BOOL bUser3a,
                               AX_ISP_AE_REGFUNCS_T *pAeFuncs)
{
    AX_S32 axRet = 0;
    AX_ISP_AE_REGFUNCS_T tAeFuncs = {0};
    if (!bUser3a) {
        tAeFuncs.pfnAe_Init = AX_ISP_ALG_AeInit;
        tAeFuncs.pfnAe_Exit = AX_ISP_ALG_AeDeInit;
        tAeFuncs.pfnAe_Run  = AX_ISP_ALG_AeRun;
        /* Register the sensor driven interface TO the AE library */
        axRet = AX_ISP_ALG_AeRegisterSensor(pipe, ptSnsHdl);
        if (axRet) {
            COMM_PRT("AX_ISP Register Sensor Failed, ret=0x%x.\n", axRet);
            return axRet;
        }
    } else {
        tAeFuncs.pfnAe_Init = pAeFuncs->pfnAe_Init;
        tAeFuncs.pfnAe_Exit = pAeFuncs->pfnAe_Exit;
        tAeFuncs.pfnAe_Run  = pAeFuncs->pfnAe_Run;
    }

    /* Register ae alg */
    axRet = AX_ISP_RegisterAeLibCallback(pipe, &tAeFuncs);
    if (axRet) {
        COMM_PRT("AX_ISP Register ae callback Failed, ret=0x%x.\n", axRet);
        return axRet;
    }

    return 0;
}

AX_S32 COMMON_ISP_RegisterAeAlgLib(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType, AX_BOOL bUser3a,
                                   AX_ISP_AE_REGFUNCS_T *pAeFuncs)
{
    AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl = NULL;

    /* 3a get sensor config */
    ptSnsHdl = COMMON_ISP_GetSnsObj(eSnsType);
    if (NULL == ptSnsHdl) {
        COMM_PRT("AX_ISP Get Sensor Object Failed!\n");
        return -1;
    }

    return RegisterAeAlgLib(pipe, ptSnsHdl, bUser3a, pAeFuncs);
}

AX_S32 COMMON_ISP_UnRegisterAeAlgLib(AX_U8 pipe)
{
    AX_S32 axRet = 0;

    axRet = AX_ISP_ALG_AeUnRegisterSensor(pipe);
    if (axRet) {
        COMM_PRT("AX_ISP ae un register sensor Failed, ret=0x%x.\n", axRet);
        return axRet;
    }

    axRet = AX_ISP_UnRegisterAeLibCallback(pipe);
    if (axRet) {
        COMM_PRT("AX_ISP Unregister Sensor Failed, ret=0x%x.\n", axRet);
        return axRet;
    }

    return axRet;
}

static AX_S32 RegisterAwbAlgLib(AX_U8 pipe, AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl, AX_BOOL bUser3a,
                                AX_ISP_AWB_REGFUNCS_T *pAwbFuncs)
{
    AX_S32 axRet = 0;
    AX_ISP_AWB_REGFUNCS_T tAwbFuncs = {0};
    if (!bUser3a) {
        tAwbFuncs.pfnAwb_Init = AX_ISP_ALG_AwbInit;
        tAwbFuncs.pfnAwb_Exit = AX_ISP_ALG_AwbDeInit;
        tAwbFuncs.pfnAwb_Run  = AX_ISP_ALG_AwbRun;
    } else {
        tAwbFuncs.pfnAwb_Init = pAwbFuncs->pfnAwb_Init;
        tAwbFuncs.pfnAwb_Exit = pAwbFuncs->pfnAwb_Exit;
        tAwbFuncs.pfnAwb_Run  = pAwbFuncs->pfnAwb_Run;
    }

    /* Register awb alg */
    axRet = AX_ISP_RegisterAwbLibCallback(pipe, &tAwbFuncs);
    if (axRet) {
        COMM_PRT("AX_ISP Register awb callback Failed, ret=0x%x.\n", axRet);
        return axRet;
    }

    return 0;
}

AX_S32 COMMON_ISP_RegisterAwbAlgLib(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType, AX_BOOL bUser3a,
                                    AX_ISP_AWB_REGFUNCS_T *pAwbFuncs)
{
    AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl = NULL;

    /* 3a get sensor config */
    ptSnsHdl = COMMON_ISP_GetSnsObj(eSnsType);
    if (NULL == ptSnsHdl) {
        COMM_PRT("AX_ISP Get Sensor Object Failed!\n");
        return -1;
    }

    return RegisterAwbAlgLib(pipe, ptSnsHdl, bUser3a, pAwbFuncs);
}

AX_S32 COMMON_ISP_UnRegisterAwbAlgLib(AX_U8 pipe)
{
    AX_S32 axRet = 0;

    axRet = AX_ISP_UnRegisterAwbLibCallback(pipe);
    if (axRet) {
        COMM_PRT("AX_ISP Unregister Sensor Failed, ret=0x%x.\n", axRet);
        return axRet;
    }

    return axRet;
}

static AX_S32 RegisterLscAlgLib(AX_U8 pipe, AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl, AX_BOOL bUser3a,
                                AX_ISP_LSC_REGFUNCS_T *pLscFuncs)
{
    AX_S32 axRet = 0;
    if (!bUser3a)
        return 0;

    /* Register Lsc alg */
    axRet = AX_ISP_RegisterLscLibCallback(pipe, pLscFuncs);
    if (axRet) {
        COMM_PRT("AX_ISP Register Lsc callback Failed, ret=0x%x.\n", axRet);
        return axRet;
    }

    return axRet;
}

AX_S32 COMMON_ISP_RegisterLscAlgLib(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType, AX_BOOL bUser3a,
                                    AX_ISP_LSC_REGFUNCS_T *pLscFuncs)
{
    AX_SENSOR_REGISTER_FUNC_T *ptSnsHdl = NULL;
    /* 3a get sensor config */
    ptSnsHdl = COMMON_ISP_GetSnsObj(eSnsType);
    if (NULL == ptSnsHdl) {
        COMM_PRT("AX_ISP Get Sensor Object Failed!\n");
        return -1;
    }

    return RegisterLscAlgLib(pipe, ptSnsHdl, bUser3a, pLscFuncs);
}

AX_S32 COMMON_ISP_UnRegisterLscAlgLib(AX_U8 pipe)
{
    AX_S32 axRet = 0;

    axRet = AX_ISP_UnRegisterLscLibCallback(pipe);
    if (axRet) {
        COMM_PRT("AX_ISP Unregister Sensor Failed, ret=0x%x.\n", axRet);
        return axRet;
    }

    return axRet;
}

AX_S32 SampleCommPoolInit(AX_SNS_HDR_MODE_E eSnsMode, SAMPLE_SNS_TYPE_E eSnsType, AX_RAW_TYPE_E raw_type,
                          AX_POOL_FLOORPLAN_T *pPoolFloorPlan, AX_U32 gDumpFrame, AX_U8 eRunMode)
{
    AX_S32 sRet = 0;
    AX_PIPE_ATTR_T *pPipeAttr = NULL;
    AX_VIN_CHN_ATTR_T *pChnAttr = NULL;
    AX_DEV_ATTR_T *pDevAttr = NULL;
    // AX_IMG_FORMAT_E eImgFormat = AX_FORMAT_INVALID;

    switch (eSnsType) {
    case OMNIVISION_OS04A10:
    case OMNIVISION_OS04A10_MASTER:
    case OMNIVISION_OS04A10_SLAVE:
        pPipeAttr = &gOs04a10PipeAttr;
        pChnAttr = &gOs04a10ChnAttr;
        pDevAttr = &gOs04a10DevAttr;
        break;
    case SONY_IMX334:
        pPipeAttr = &gImx334PipeAttr;
        pChnAttr = &gImx334ChnAttr;
        pDevAttr = &gImx334DevAttr;
        break;
    case GALAXYCORE_GC4653:
        pPipeAttr = &gGc4653PipeAttr;
        pChnAttr = &gGc4653ChnAttr;
        pDevAttr = &gGc4653DevAttr;
        break;
    case OMNIVISION_OS08A20:
        pPipeAttr = &gOs08a20PipeAttr;
        pChnAttr = &gOs08a20ChnAttr;
        pDevAttr = &gOs08a20DevAttr;
        break;
    case SENSOR_DVP:
        pPipeAttr = &gDVPPipeAttr;
        pChnAttr = &gDVPChnAttr;
        pDevAttr = &gDVPDevAttr;
        break;
    case SENSOR_BT601:
        pPipeAttr = &gBT601PipeAttr;
        pChnAttr = &gBT601ChnAttr;
        pDevAttr = &gBT601DevAttr;
        break;
    case SENSOR_BT656:
        pPipeAttr = &gBT656PipeAttr;
        pChnAttr = &gBT656ChnAttr;
        pDevAttr = &gBT656DevAttr;
        break;
    case SENSOR_BT1120:
        pPipeAttr = &gBT1120PipeAttr;
        pChnAttr = &gBT1120ChnAttr;
        pDevAttr = &gBT1120DevAttr;
        break;
    case MIPI_YUV:
        pPipeAttr = &gMIPI_YUVPipeAttr;
        pChnAttr = &gMIPI_YUVChnAttr;
        pDevAttr = &gMIPI_YUVDevAttr;
        break;
    default:
        pPipeAttr = &gOs04a10PipeAttr;
        pChnAttr = &gOs04a10ChnAttr;
        pDevAttr = &gOs04a10DevAttr;
        break;
    }

    sRet = AX_POOL_Exit();
    if (sRet) {
        COMM_ISP_PRT("AX_POOL_Exit fail!!Error Code:0x%X\n", sRet);
    }
    memset(pPoolFloorPlan, 0, sizeof(AX_POOL_FLOORPLAN_T));
    pPoolFloorPlan->CommPool[COMM_ISP_RAW0].MetaSize   = 10 * 1024;
    if (eRunMode == 2) {
        pPoolFloorPlan->CommPool[COMM_ISP_RAW0].BlkSize   = AX_VIN_GetImgBufferSize(pDevAttr->tDevImgRgn.nHeight,
                pDevAttr->tDevImgRgn.nWidth, AX_FORMAT_BAYER_RAW_14BPP, AX_TRUE);
    } else {
        pPoolFloorPlan->CommPool[COMM_ISP_RAW0].BlkSize   = AX_VIN_GetImgBufferSize(pDevAttr->tDevImgRgn.nHeight,
                pDevAttr->tDevImgRgn.nWidth, raw_fmt_2_comm_fmt(raw_type), AX_TRUE);
    }

    if (gDumpFrame)
        pPoolFloorPlan->CommPool[COMM_ISP_RAW0].BlkCnt = 40 + gDumpFrame;
    else
        pPoolFloorPlan->CommPool[COMM_ISP_RAW0].BlkCnt = 40;
    pPoolFloorPlan->CommPool[COMM_ISP_RAW0].CacheMode = POOL_CACHE_MODE_NONCACHE;

    //PartitionName default is anonymous
    memset(pPoolFloorPlan->CommPool[COMM_ISP_RAW0].PartitionName, 0,
           sizeof(pPoolFloorPlan->CommPool[COMM_ISP_RAW0].PartitionName));
    strcpy((char *)pPoolFloorPlan->CommPool[COMM_ISP_RAW0].PartitionName, "anonymous");

    pPoolFloorPlan->CommPool[COMM_ISP_RAW1].MetaSize   = 10 * 1024;
    pPoolFloorPlan->CommPool[COMM_ISP_RAW1].BlkSize    = AX_VIN_GetImgBufferSize(pPipeAttr->nHeight,
            pPipeAttr->nWidth, AX_FORMAT_BAYER_RAW_16BPP, AX_TRUE);
    pPoolFloorPlan->CommPool[COMM_ISP_RAW1].BlkCnt = 5;
    pPoolFloorPlan->CommPool[COMM_ISP_RAW1].CacheMode = POOL_CACHE_MODE_NONCACHE;

    //PartitionName default is anonymous
    memset(pPoolFloorPlan->CommPool[COMM_ISP_RAW1].PartitionName, 0,
           sizeof(pPoolFloorPlan->CommPool[COMM_ISP_RAW1].PartitionName));
    strcpy((char *)pPoolFloorPlan->CommPool[COMM_ISP_RAW1].PartitionName, "anonymous");

    for (int i = COMM_ISP_YUV_MAIN; i <= COMM_ISP_YUV_SUB2; i++) {
        pPoolFloorPlan->CommPool[i].MetaSize   = 10 * 1024;
        pPoolFloorPlan->CommPool[i].BlkSize    = AX_VIN_GetImgBufferSize(pChnAttr->tChnAttr[i - COMM_ISP_YUV_MAIN].nHeight,
                pChnAttr->tChnAttr[i - COMM_ISP_YUV_MAIN].nWidthStride, AX_YUV420_SEMIPLANAR, AX_TRUE);

        pPoolFloorPlan->CommPool[i].BlkCnt = pChnAttr->tChnAttr[i - COMM_ISP_YUV_MAIN].nDepth;
        pPoolFloorPlan->CommPool[i].CacheMode = POOL_CACHE_MODE_NONCACHE;
        //PartitionName default is anonymous
        memset(pPoolFloorPlan->CommPool[i].PartitionName, 0, sizeof(pPoolFloorPlan->CommPool[i].PartitionName));
        strcpy((char *)pPoolFloorPlan->CommPool[i].PartitionName, "anonymous");
    }

    sRet = AX_POOL_SetConfig(pPoolFloorPlan);
    if (sRet) {
        COMM_ISP_PRT("AX_POOL_SetConfig fail!Error Code:0x%X\n", sRet);
        return -1;
    } else {
        printf("AX_POOL_SetConfig success!\n");
    }

    sRet = AX_POOL_Init();
    if (sRet) {
        COMM_ISP_PRT("AX_POOL_Init fail!!Error Code:0x%X\n", sRet);
        return -1;
    } else {
        COMM_ISP_PRT("AX_POOL_Init success!\n");
    }
    return sRet;
}


/************* Use only when mipi tx ************/
AX_S32 COMMON_ISP_InitTx()
{
    AX_S32 axRet;

    /* mipi tx init */
    axRet = AX_MIPI_TX_Init();
    if (0 != axRet) {
        COMM_PRT(" failed, ret=0x%x.\n", axRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_ISP_DeinitTx()
{
    AX_S32 axRet = 0;

    /* mipi tx deinit */
    axRet = AX_MIPI_TX_DeInit();

    return axRet;
}

AX_S32 COMMON_ISP_SetMipiTxAttr(AX_U8 nMipiTxDevId, SAMPLE_SNS_TYPE_E eSnsType, AX_SNS_HDR_MODE_E  eHdrMode,
                                AX_BOOL bIspBypass)
{
    AX_MIPI_TX_ATTR_S tMipiTxAttr = {0};
    AX_S32 nRet = 0;

    switch (eSnsType) {
    case OMNIVISION_OS04A10:
    case OMNIVISION_OS04A10_MASTER:
    case OMNIVISION_OS04A10_SLAVE:
        if (bIspBypass) {
            memcpy(&tMipiTxAttr, &gOs04a10MipiTxIspBypassAttr, sizeof(AX_MIPI_TX_ATTR_S));
        } else {
            memcpy(&tMipiTxAttr, &gOs04a10MipiTxAttr, sizeof(AX_MIPI_TX_ATTR_S));
        }
        break;
    case SONY_IMX334:
        if (bIspBypass) {
            memcpy(&tMipiTxAttr, &gImx334MipiTxIspBypassAttr, sizeof(AX_MIPI_TX_ATTR_S));
        } else {
            memcpy(&tMipiTxAttr, &gImx334MipiTxAttr, sizeof(AX_MIPI_TX_ATTR_S));
        }
        break;
    case GALAXYCORE_GC4653:
        if (bIspBypass) {
            memcpy(&tMipiTxAttr, &gGc4653MipiTxIspBypassAttr, sizeof(AX_MIPI_TX_ATTR_S));
        } else {
            memcpy(&tMipiTxAttr, &gGc4653MipiTxAttr, sizeof(AX_MIPI_TX_ATTR_S));
        }
        break;
    case OMNIVISION_OS08A20:
        if (bIspBypass) {
            memcpy(&tMipiTxAttr, &gOs08a20MipiTxIspBypassAttr, sizeof(AX_MIPI_TX_ATTR_S));
            if (eHdrMode == AX_SNS_HDR_2X_MODE) {
                tMipiTxAttr.eImgDataType = AX_MIPI_DT_RAW10;
                tMipiTxAttr.eDataRate = AX_MIPI_DATA_RATE_1500M;
            }
        } else {
            memcpy(&tMipiTxAttr, &gOs08a20MipiTxAttr, sizeof(AX_MIPI_TX_ATTR_S));
        }
        break;
    default:
        if (bIspBypass) {
            memcpy(&tMipiTxAttr, &gOs04a10MipiTxIspBypassAttr, sizeof(AX_MIPI_TX_ATTR_S));
        } else {
            memcpy(&tMipiTxAttr, &gOs04a10MipiTxAttr, sizeof(AX_MIPI_TX_ATTR_S));
        }
        break;
    }

    if ((bIspBypass) && (eHdrMode == AX_SNS_HDR_2X_MODE)) {
        tMipiTxAttr.eDolSplitNum = 2;
    }

    nRet = AX_MIPI_TX_Reset(nMipiTxDevId);
    if (0 != nRet) {
        COMM_PRT(" failed, ret=0x%x.\n", nRet);
        return -1;
    }

    nRet = AX_MIPI_TX_SetAttr(nMipiTxDevId, &tMipiTxAttr);
    if (0 != nRet) {
        COMM_PRT(" failed, ret=0x%x.\n", nRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_ISP_OpenTx(AX_U8 nTxDevId, SAMPLE_SNS_TYPE_E eSnsType, AX_BOOL bIspBypass)
{
    AX_S32 nRet = 0;
    AX_TX_IMG_INFO_T tTxImgInfo = {0};

    switch (eSnsType) {
    case OMNIVISION_OS04A10:
    case OMNIVISION_OS04A10_MASTER:
    case OMNIVISION_OS04A10_SLAVE:
        memcpy(&tTxImgInfo, &gOs04a10TxImgInfo, sizeof(AX_TX_IMG_INFO_T));
        if (bIspBypass == AX_TRUE) {
            tTxImgInfo.bIspBypass = AX_TRUE;
            tTxImgInfo.eImgFormat = AX_FORMAT_BAYER_RAW_10BPP;
        }
        break;
    case SONY_IMX334:
        memcpy(&tTxImgInfo, &gImx334TxImgInfo, sizeof(AX_TX_IMG_INFO_T));
        if (bIspBypass == AX_TRUE) {
            tTxImgInfo.bIspBypass = AX_TRUE;
            tTxImgInfo.eImgFormat = AX_FORMAT_BAYER_RAW_10BPP;
        }
        break;
    case GALAXYCORE_GC4653:
        memcpy(&tTxImgInfo, &gGc4653TxImgInfo, sizeof(AX_TX_IMG_INFO_T));
        if (bIspBypass == AX_TRUE) {
            tTxImgInfo.bIspBypass = AX_TRUE;
            tTxImgInfo.eImgFormat = AX_FORMAT_BAYER_RAW_10BPP;
        }
        break;
    case OMNIVISION_OS08A20:
        memcpy(&tTxImgInfo, &gOs08a20TxImgInfo, sizeof(AX_TX_IMG_INFO_T));
        if (bIspBypass == AX_TRUE) {
            tTxImgInfo.bIspBypass = AX_TRUE;
            tTxImgInfo.eImgFormat = AX_FORMAT_BAYER_RAW_12BPP;
        }
        break;
    default:
        memcpy(&tTxImgInfo, &gOs04a10TxImgInfo, sizeof(AX_TX_IMG_INFO_T));
        if (bIspBypass == AX_TRUE) {
            tTxImgInfo.bIspBypass = AX_TRUE;
            tTxImgInfo.eImgFormat = AX_FORMAT_BAYER_RAW_10BPP;
        }
        break;
    }

    nRet = AX_MIPI_TX_Start(nTxDevId);
    if (0 != nRet) {
        COMM_PRT(" failed, ret=0x%x.\n", nRet);
        return -1;
    }

    nRet = AX_VIN_TxOpen(nTxDevId, &tTxImgInfo);
    if (0 != nRet) {
        COMM_PRT(" failed, ret=0x%x.\n", nRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_ISP_CloseTx(AX_U8 nTxDevId)
{
    AX_S32 nRet = 0;

    nRet = AX_VIN_TxClose(nTxDevId);
    if (0 != nRet) {
        COMM_PRT(" failed, ret=0x%x.\n", nRet);
        return -1;
    }

    nRet = AX_MIPI_TX_Stop(nTxDevId);
    if (0 != nRet) {
        COMM_PRT(" failed, ret=0x%x.\n", nRet);
        return -1;
    }

    return 0;
}


AX_S32 COMMON_ISP_SetDevAttrEx(AX_U8 devId, SAMPLE_SNS_TYPE_E eSnsType, AX_IMG_FORMAT_E ePixelFmt,
                               AX_SNS_HDR_MODE_E eHdrMode, AX_ISP_DEV_WORK_MODE_E eWorkMode, AX_BOOL bImgEnable, AX_BOOL bNonImgEnable,
                               AX_BOOL bIspBypass)
{
    AX_S32 nRet = 0;
    AX_DEV_ATTR_T tDevAttr = {0};

    switch (eSnsType) {
    case OMNIVISION_OS04A10:
    case OMNIVISION_OS04A10_MASTER:
    case OMNIVISION_OS04A10_SLAVE:
        memcpy(&tDevAttr, &gOs04a10DevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    case SONY_IMX334:
        memcpy(&tDevAttr, &gImx334DevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    case GALAXYCORE_GC4653:
        memcpy(&tDevAttr, &gGc4653DevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    case OMNIVISION_OS08A20:
        memcpy(&tDevAttr, &gOs08a20DevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    default:
        memcpy(&tDevAttr, &gOs04a10DevAttr, sizeof(AX_DEV_ATTR_T));
        break;
    }

    tDevAttr.ePixelFmt = ePixelFmt;
    tDevAttr.eNonPixelFmt = ePixelFmt;
    tDevAttr.eSnsMode = eHdrMode;
    tDevAttr.eDevWorkMode = eWorkMode;
    tDevAttr.bImgDataEnable = bImgEnable;
    tDevAttr.bNonImgEnable = bNonImgEnable;
    tDevAttr.bIspBypass = bIspBypass;

    COMM_PRT("DEV ePixelFmt=%d, eSnsMode=%d, eDevWorkMode=%d, bImgDataEnable=%d, eNonPixelFmt=%d, bIspBypass:%d\n",
             tDevAttr.ePixelFmt, tDevAttr.eSnsMode, tDevAttr.eDevWorkMode, tDevAttr.bImgDataEnable, tDevAttr.eNonPixelFmt,
             tDevAttr.bIspBypass);

    nRet = AX_VIN_SetDevAttr(devId, &tDevAttr);
    if (0 != nRet) {
        COMM_PRT("AX_VIN_SetDevAttr failed, nRet=0x%x.\n", nRet);
        return -1;
    }

    return 0;
}



