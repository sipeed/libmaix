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
#include "common_cam.h"
#include "common_type.h"

AX_S32 COMMON_CAM_Init()
{
    AX_S32 axRet;

    /* AX ISP Init */
    axRet = AX_VIN_Init();
    if (0 != axRet) {
        COMM_PRT("AX_VIN_Init failed, ret=0x%x.\n", axRet);
        return -1;
    }

    /* mipi init */
    axRet = AX_MIPI_RX_Init();
    if (0 != axRet) {
        COMM_PRT("AX_MIPI_RX_Init failed, ret=0x%x.\n", axRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_CAM_Deinit()
{
    AX_S32 axRet = 0;

    axRet = AX_MIPI_RX_DeInit();

    axRet = AX_VIN_Deinit();
    if (0 != axRet) {
        COMM_PRT("AX_VIN_DeInit failed, ret=0x%x.\n", axRet);
        return -1 ;
    }

    return axRet;
}

AX_S32 COMMON_CAM_Open(CAMERA_T *pCam)
{
    AX_S32 axRet;
    AX_U8 nPipeId = pCam->nPipeId;
    AX_U8 nDevId = pCam->nDevId;
    AX_MIPI_RX_DEV_E nRxDev = pCam->nRxDev;
    SAMPLE_SNS_TYPE_E eSnsType = pCam->eSnsType;
    AX_VIN_SNS_DUMP_ATTR_T  tDumpAttr = {0};
    AX_PIPE_ATTR_T   tPipeAttr = {0};
    char *pFile = pCam->szTuningFileName;
    AX_VIN_DEV_BIND_PIPE_T tDevBindPipe = {0};

    tDevBindPipe.nNum = 1;
    tDevBindPipe.nPipeId[0] = nPipeId;

    /* AX vin init */
    axRet = AX_VIN_Create(nPipeId);
    if (0 != axRet) {
        COMM_PRT("AX_VIN_Create failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = COMMON_ISP_RegisterSns(nPipeId, nDevId, eSnsType);
    if (0 != axRet) {
        COMM_PRT("COMMON_ISP_RegisterSns failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_VIN_SetRunMode(nPipeId, AX_ISP_PIPELINE_NORMAL);
    if (0 != axRet) {
        printf("AX_VIN_SetRunMode failed, ret=0x%x.\n", axRet);
        return -1;
    }

    /* confige sensor attr */
    axRet = AX_VIN_SetSnsAttr(nPipeId, &pCam->stSnsAttr);
    if (0 != axRet) {
        COMM_PRT("AX_VIN_SetSnsAttr failed, nRet=0x%x.\n", axRet);
        return -1;
    }

    /* confige sensor clk */
    axRet = AX_VIN_OpenSnsClk(nPipeId, pCam->stSnsClkAttr.nSnsClkIdx, pCam->stSnsClkAttr.eSnsClkRate);
    if (0 != axRet) {
        COMM_PRT("AX_VIN_OpenSnsClk failed, nRet=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_VIN_SetDevAttr(nDevId, &pCam->stDevAttr);
    if (0 != axRet) {
        COMM_PRT("AX_VIN_SetDevAttr failed, nRet=0x%x.\n", axRet);
        return -1;
    }

    axRet = COMMON_ISP_SetMipiAttr(nRxDev, eSnsType, AX_FALSE);
    if (0 != axRet) {
        COMM_PRT("AX_MIPI_RX_SetAttr failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_VIN_SetChnAttr(nPipeId, &pCam->stChnAttr);
    if (0 != axRet) {
        COMM_PRT("AX_VIN_SetChnAttr failed, nRet = 0x%x.\n", axRet);
        return -1;
    }

    /*change hde mode*/
    axRet = AX_VIN_SetPipeAttr(nPipeId, &pCam->stPipeAttr);
    if (0 != axRet) {
        COMM_PRT("AX_VI_SetPipeAttr failed, nRet = 0x%x.\n", axRet);
        return -1;
    }

    /* config bind */
    axRet = AX_VIN_SetDevBindPipe(nDevId, &tDevBindPipe);
    if (0 != axRet) {
        COMM_PRT("AX_VIN_SetDevBindPipe failed, ret=0x%x\n", axRet);
        return -1;
    }

    axRet = AX_ISP_Open(nPipeId);
    if (0 != axRet) {
        COMM_PRT("AX_ISP_Open failed, ret=0x%x\n", axRet);
        return -1;
    }

    /* ae alg register*/
    axRet = COMMON_ISP_RegisterAeAlgLib(nPipeId, eSnsType, pCam->bUser3a, &pCam->tAeFuncs);
    if (0 != axRet) {
        COMM_PRT("RegisterAeAlgLib failed, ret=0x%x.\n", axRet);
        return -1;
    }

    /* awb alg register*/
    axRet = COMMON_ISP_RegisterAwbAlgLib(nPipeId, eSnsType, pCam->bUser3a,  &pCam->tAwbFuncs);
    if (0 != axRet) {
        COMM_PRT("RegisterAwbAlgLib failed, ret=0x%x.\n", axRet);
        return -1;
    }

    /* lsc alg register*/
    axRet = COMMON_ISP_RegisterLscAlgLib(nPipeId, eSnsType, pCam->bUser3a, &pCam->tLscFuncs);
    if (0 != axRet) {
        COMM_PRT("RegisterLscAlgLib failed, ret=0x%x.\n", axRet);
        return -1;
    }

    COMM_ISP_PRT("pFile %s \n", pFile);
    axRet = AX_ISP_LoadBinParams(nPipeId, pFile);
    if (0 != axRet) {
        COMM_PRT("AX_ISP_LoadBinParams %s will user sensor.h\n", pFile);
        //return -1;
    }

    axRet = AX_VIN_Start(nPipeId);
    if (0 != axRet) {
        COMM_PRT("AX_VIN_Start failed, ret=0x%x\n", axRet);
        return -1;
    }

    if (pCam->eSrcType != AX_PIPE_FRAME_SOURCE_TYPE_DEV) {
        axRet = AX_VIN_SetPipeFrameSource(nPipeId, pCam->eSrcId, pCam->eSrcType);
        if (0 != axRet) {
            printf("AX_VIN_SetPipeFrameSource failed, ret=0x%x.\n", axRet);
            return -1;
        }
    }

    axRet = AX_VIN_EnableDev(nDevId);
    if (0 != axRet) {
        COMM_PRT("AX_VIN_EnableDev failed, ret=0x%x.\n", axRet);
        return -1;
    }

    AX_VIN_GetPipeAttr(nPipeId, &tPipeAttr);

    if (AX_PIPE_SOURCE_DEV_OFFLINE == tPipeAttr.ePipeDataSrc) {
        tDumpAttr.bEnable = AX_TRUE;
        tDumpAttr.nDepth = 2;
        axRet = AX_VIN_SetSnsDumpAttr(nDevId, &tDumpAttr);
        if (0 != axRet) {
            COMM_ISP_PRT(" AX_VIN_SetSnsDumpAttr failed, ret=0x%x.\n", axRet);
            return -1;
        }
    }

    axRet = AX_VIN_StreamOn(nPipeId);
    if (0 != axRet) {
        COMM_PRT(" failed, ret=0x%x.\n", axRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_CAM_Close(CAMERA_T *pCam)
{
    AX_S32 axRet = 0;
    AX_U8 nPipeId = pCam->nPipeId;
    AX_U8 nDevId = pCam->nDevId;
    AX_VIN_SNS_DUMP_ATTR_T  tDumpAttr = {0};
    AX_PIPE_ATTR_T   tPipeAttr = {0};

    AX_VIN_StreamOff(nPipeId);

    AX_VIN_GetPipeAttr(nPipeId, &tPipeAttr);

    if (AX_PIPE_SOURCE_DEV_OFFLINE == tPipeAttr.ePipeDataSrc) {
        tDumpAttr.bEnable = AX_FALSE;
        axRet = AX_VIN_SetSnsDumpAttr(nDevId, &tDumpAttr);
        if (0 != axRet) {
            COMM_ISP_PRT(" AX_VIN_SetSnsDumpAttr failed, ret=0x%x.\n", axRet);
            return -1;
        }
    }

    axRet = AX_VIN_CloseSnsClk(pCam->stSnsClkAttr.nSnsClkIdx);
    if (0 != axRet) {
        COMM_PRT("AX_VIN_CloseSnsClk failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_VIN_DisableDev(nDevId);
    if (0 != axRet) {
        COMM_PRT("AX_VIN_DisableDev failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_VIN_Stop(nPipeId);
    if (0 != axRet) {
        COMM_PRT("AX_VIN_Stop failed, ret=0x%x.\n", axRet);
        return -1 ;
    }
    COMMON_ISP_UnRegisterAeAlgLib(nPipeId);
    COMMON_ISP_UnRegisterAwbAlgLib(nPipeId);
    if(pCam->bUser3a)
        COMMON_ISP_UnRegisterLscAlgLib(nPipeId);

    axRet = AX_ISP_Close(nPipeId);
    if (0 != axRet) {
        COMM_PRT("AX_ISP_Close failed, ret=0x%x.\n", axRet);
        return -1 ;
    }

    /* ISP Unregister Sensor*/
    COMMON_ISP_UnRegisterSns(nPipeId);

    AX_VIN_Destory(nPipeId);

    COMM_PRT("%s: pipe %d: exit.\n", __func__, nPipeId);
    return 0;
}


AX_S32 COMMON_CAM_DVP_Open(CAMERA_T *pCam)
{
    AX_S32 axRet;
    AX_U8 nPipeId = pCam->nPipeId;
    AX_U8 nDevId = pCam->nDevId;
    AX_MIPI_RX_DEV_E nRxDev = pCam->nRxDev;
    SAMPLE_SNS_TYPE_E eSnsType = pCam->eSnsType;
    AX_VIN_SNS_DUMP_ATTR_T  tDumpAttr = {0};
    AX_VIN_DEV_BIND_PIPE_T tDevBindPipe = {0};

    tDevBindPipe.nNum = 1;
    tDevBindPipe.nPipeId[0] = nPipeId;

    /* AX vin init */
    axRet = AX_VIN_Create(nPipeId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_Create failed, ret=0x%x.\n", axRet);
        return -1;
    }
    if (eSnsType == MIPI_YUV){
        axRet = COMMON_ISP_SetMipiAttr(nRxDev, eSnsType, AX_TRUE);
            if (0 != axRet) {
                COMM_ISP_PRT("AX_MIPI_RX_SetAttr failed, ret=0x%x.\n", axRet);
                return -1;
            }
    }

    axRet = AX_VIN_SetRunMode(nPipeId, AX_ISP_PIPELINE_NONE_NPU);
    if (0 != axRet) {
        printf("AX_VIN_SetRunMode failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_VIN_SetDevAttr(nDevId, &pCam->stDevAttr);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_SetDevAttr failed, nRet=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_VIN_SetChnAttr(nPipeId, &pCam->stChnAttr);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_SetChnAttr failed, nRet = 0x%x.\n", axRet);
        return -1;
    }

    /*change hde mode*/
    axRet = AX_VIN_SetPipeAttr(nPipeId, &pCam->stPipeAttr);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VI_SetPipeAttr failed, nRet = 0x%x.\n", axRet);
        return -1;
    }

    /* config bind */
    axRet = AX_VIN_SetDevBindPipe(nDevId, &tDevBindPipe);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_SetDevBindPipe failed, ret=0x%x\n", axRet);
        return -1;
    }

    axRet = AX_ISP_Open(nPipeId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_ISP_Open failed, ret=0x%x\n", axRet);
        return -1;
    }

    axRet = AX_VIN_Start(nPipeId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_Start failed, ret=0x%x\n", axRet);
        return -1;
    }

    axRet = AX_VIN_EnableDev(nDevId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_EnableDev failed, ret=0x%x.\n", axRet);
        return -1;
    }

    tDumpAttr.bEnable = AX_TRUE;
    tDumpAttr.nDepth = 2;
    axRet = AX_VIN_SetSnsDumpAttr(nDevId, &tDumpAttr);
    if (0 != axRet) {
        COMM_ISP_PRT(" AX_VIN_SetSnsDumpAttr failed, ret=0x%x.\n", axRet);
        return -1;
    }

    return 0;
}

AX_S32 COMMON_CAM_DVP_Close(CAMERA_T *pCam)
{
    AX_S32 axRet = 0;
    AX_U8 nPipeId = pCam->nPipeId;
    AX_U8 nDevId = pCam->nDevId;

    axRet = AX_VIN_DisableDev(nDevId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_DisableDev failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_VIN_Stop(nPipeId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_VIN_Stop failed, ret=0x%x.\n", axRet);
        return -1 ;
    }

    axRet = AX_ISP_Close(nPipeId);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_ISP_Close failed, ret=0x%x.\n", axRet);
        return -1 ;
    }

    AX_VIN_Destory(nPipeId);

    COMM_ISP_PRT("%s: exit.\n", __func__);
    return 0;
}

