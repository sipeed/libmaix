/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __COMMON_CAM_H__
#define __COMMON_CAM_H__

#include "ax_base_type.h"
#include "common_vin.h"
#include <pthread.h>

#define MAX_CAMERAS 4
#define MAX_FILE_NAME_CHAR_SIZE       (128)

typedef struct {
    AX_U8               bOpen;
    AX_SNS_HDR_MODE_E   eHdrMode;
    SAMPLE_SNS_TYPE_E   eSnsType;
    AX_ISP_PIPE_FRAME_SOURCE_ID_E   eSrcId;
    AX_ISP_PIPE_FRAME_SOURCE_TYPE_E eSrcType;
    AX_RAW_TYPE_E       eRawType;       /* sensor attr use */
    AX_IMG_FORMAT_E     ePixelFmt;      /* pipe/dev attr use */
    AX_MIPI_RX_DEV_E    nRxDev;
    AX_MIPI_TX_DEV_E    nTxDev;
    AX_U8               nDevId;
    AX_U8               nPipeId;
    pthread_t           tIspProcThread;
    pthread_t           tTxTransferThread;
    AX_CHAR             szTuningFileName[MAX_FILE_NAME_CHAR_SIZE];
    AX_SNS_ATTR_T       stSnsAttr;
    AX_SNS_CLK_ATTR_T       stSnsClkAttr;
    AX_DEV_ATTR_T       stDevAttr;
    AX_PIPE_ATTR_T      stPipeAttr;
    AX_VIN_CHN_ATTR_T   stChnAttr;
    AX_ISP_AE_REGFUNCS_T tAeFuncs;
    AX_ISP_AWB_REGFUNCS_T tAwbFuncs;
    AX_ISP_LSC_REGFUNCS_T tLscFuncs;
    AX_BOOL             bUser3a;
    pthread_t           tIspAFProcThread;
} CAMERA_T;

AX_S32 COMMON_CAM_Init();
AX_S32 COMMON_CAM_Deinit();

AX_S32 COMMON_CAM_Open(CAMERA_T *pCam);
AX_S32 COMMON_CAM_Close(CAMERA_T *pCam);

AX_S32 COMMON_CAM_DVP_Open(CAMERA_T *pCam);
AX_S32 COMMON_CAM_DVP_Close(CAMERA_T *pCam);

AX_S32 COMMON_CAM_MipiTx_Open(CAMERA_T *pCam);
AX_S32 COMMON_CAM_MipiTx_Close(CAMERA_T *pCam);

#endif //__COMMON_CAM_H__
