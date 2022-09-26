/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __COMMON_SYS_H__
#define __COMMON_SYS_H__

#include "ax_base_type.h"

#define MAX_POOLS 5

typedef struct {
    AX_U32          nWidth;
    AX_U32          nHeight;
    AX_U32          nWidthStride;
    AX_IMG_FORMAT_E nFmt;
    AX_U32          nBlkCnt;
} COMMON_SYS_POOL_CFG_T;

typedef struct {
    AX_U8 nCamCnt;
    AX_U32 nPoolCfgCnt;
    COMMON_SYS_POOL_CFG_T* pPoolCfg;
} COMMON_SYS_ARGS_T;

AX_S32 COMMON_SYS_Init(COMMON_SYS_ARGS_T *pCommonArgs);
AX_S32 COMMON_SYS_DeInit();

#endif //__COMMON_SYS_H__

