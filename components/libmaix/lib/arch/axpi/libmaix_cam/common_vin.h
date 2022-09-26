/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __COMMON_ISP_H__
#define __COMMON_ISP_H__
#include <stdio.h>
#include <stdarg.h>

#include "ax_vin_api.h"
#include "ax_base_type.h"
#include "ax_interpreter_external_api.h"
#include "ax_sys_api.h"
#include "ax_mipi_api.h"
#include "ax_isp_3a_api.h"

#define MAX_SNS_NUM 2

#define AX_ALIGN_UP_SAMPLE(x, align) (((x) + ((align) - 1)) & ~((align)-1))
#define AX_ALIGN_DOWN_SAMPLE(x, align) ((x) & ~((align)-1))

#define DEF_ISP_BUF_BLOCK_NUM    (10)

typedef struct _AX_SNS_CLK_ATTR_T_ {
    AX_U8                  nSnsClkIdx;
    AX_SNS_CLK_RATE_E   eSnsClkRate;
} AX_SNS_CLK_ATTR_T;

typedef enum {
    SAMPLE_SNS_TYPE_NONE = -1,

    /*ov sensor*/
    OMNIVISION_OS04A10 = 0,
    OMNIVISION_OS04A10_MASTER = 1,
    OMNIVISION_OS04A10_SLAVE = 2,
    OMNIVISION_OS08A20 = 3,

    /*sony sensor*/
    SONY_IMX334 = 20,

    /*gc sensor*/
    GALAXYCORE_GC4653 = 30,

    /*DVP sensor*/
    SENSOR_DVP = 40,

    /*BT sensor*/
    SENSOR_BT601  = 50,
    SENSOR_BT656  = 51,
    SENSOR_BT1120 = 52,

    MIPI_YUV = 60,

    SAMPLE_SNS_TYPE_BUTT,
} SAMPLE_SNS_TYPE_E;

typedef enum {
    COMM_ISP_SUCCESS                           = 0x0,
    COMM_ISP_ERR_CODE_FAILED                   = 0x1,
    COMM_ISP_ERR_CODE_PTR_NULL,
    COMM_ISP_ERR_CODE_INVALID_ADDRESS,
    COMM_ISP_ERR_CODE_ILLEGAL_PARAMS,

} COMM_ISP_ERR_CODE_E;

typedef enum {
    COMM_ISP_RAW0 = 0,
    COMM_ISP_RAW1,
    COMM_ISP_YUV_MAIN,
    COMM_ISP_YUV_SUB1,
    COMM_ISP_YUV_SUB2,
    COMM_ISP_BUF_MAX
} COMM_ISP_BLK_SIZE_E;

#ifndef COMM_ISP_PRT
#define COMM_ISP_PRT(fmt...)   \
do {\
    printf("[sample_isp][%s][%d] ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)
#endif



/* macro definition: Function returns an error */
#define COMM_ISP_CHECK_PTR_VALID(h)                                                      \
do {                                                                            \
    if (AX_NULL == (h)) {                                                          \
        COMM_ISP_PRT("[CHECK_PTR_VALID] error: pointer is null!\n");              \
        return COMM_ISP_ERR_CODE_PTR_NULL;                                             \
    }                                                                           \
} while(0)

#define COMM_ISP_CHECK_VALUE_RANGE_STRING(string, val, min, max)                                                      \
do {                                                                                                \
    if ((val) < (min) || (val) > (max)) {                                                           \
        COMM_ISP_PRT("[CHECK_VALUE_RANGE_VALID]:error: [%s]:%d beyond the range:(%d, %u)\n",       \
                    (string), (val), (min), (max));                                                 \
        return COMM_ISP_ERR_CODE_ILLEGAL_PARAMS;                                                            \
    }                                                                                               \
} while(0)

#define COMM_ISP_CHECK_VALUE_RANGE_VALID(val, min, max)                                                      \
do {                                                                                                \
    if ((val) < (min) || (val) > (max)) {                                                           \
        COMM_ISP_PRT("[CHECK_VALUE_RANGE_VALID] error: value:%d beyond the range:(%d, %d)\n",         \
                    (val), (min), (max));                                                 \
        return COMM_ISP_ERR_CODE_ILLEGAL_PARAMS;                                                            \
    }                                                                                               \
} while(0)



#ifdef __cplusplus
extern "C"
{
#endif

AX_S32 COMMON_ISP_RegisterSns(AX_U8 pipe, AX_U8 nDevId, SAMPLE_SNS_TYPE_E eSnsType);
AX_S32 COMMON_ISP_UnRegisterSns(AX_U8 pipe);
AX_S32 COMMON_ISP_RegisterAeAlgLib(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType, AX_BOOL bUser3a,
                                   AX_ISP_AE_REGFUNCS_T *pAeFuncs);
AX_S32 COMMON_ISP_UnRegisterAeAlgLib(AX_U8 pipe);

AX_S32 COMMON_ISP_RegisterAwbAlgLib(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType, AX_BOOL bUser3a,
                                    AX_ISP_AWB_REGFUNCS_T *pAwbFuncs);
AX_S32 COMMON_ISP_UnRegisterAwbAlgLib(AX_U8 pipe);
AX_S32 COMMON_ISP_RegisterLscAlgLib(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType, AX_BOOL bUser3a,
                                    AX_ISP_LSC_REGFUNCS_T *pLscFuncs);
AX_S32 COMMON_ISP_UnRegisterLscAlgLib(AX_U8 pipe);

AX_SENSOR_REGISTER_FUNC_T *COMMON_ISP_GetSnsObj(SAMPLE_SNS_TYPE_E eSnsType);

AX_S32 COMMON_ISP_SetMipiAttr(AX_U8 devId, SAMPLE_SNS_TYPE_E eSnsType, AX_BOOL bMaster);
AX_S32 COMMON_ISP_SetSnsAttr(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType, AX_RAW_TYPE_E eRawType,
                             AX_SNS_HDR_MODE_E eHdrMode);
AX_S32 COMMON_ISP_SetDevAttr(AX_U8 devId, SAMPLE_SNS_TYPE_E eSnsType, AX_IMG_FORMAT_E ePixelFmt,
                             AX_SNS_HDR_MODE_E eHdrMode);
AX_S32 COMMON_ISP_SetPipeAttr(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType, AX_IMG_FORMAT_E ePixelFmt,
                              AX_SNS_HDR_MODE_E eHdrMode);
AX_S32 COMMON_ISP_GetSnsAttr(AX_U8 pipe, AX_SNS_ATTR_T *ptSnsAttr);
AX_S32 COMMON_ISP_SetChnAttr(AX_U8 pipe, SAMPLE_SNS_TYPE_E eSnsType);

AX_S32 COMMON_ISP_InitTx();
AX_S32 COMMON_ISP_DeinitTx();

AX_S32 COMMON_ISP_SetMipiTxAttr(AX_U8 nMipiTxDevId, SAMPLE_SNS_TYPE_E eSnsType, AX_SNS_HDR_MODE_E eHdrMode, AX_BOOL bIspBypass);
AX_S32 COMMON_ISP_OpenTx(AX_U8 nTxDevId, SAMPLE_SNS_TYPE_E eSnsType, AX_BOOL bIspBypass);
AX_S32 COMMON_ISP_CloseTx(AX_U8 nTxDevId);
AX_S32 COMMON_ISP_SetDevAttrEx(AX_U8 devId, SAMPLE_SNS_TYPE_E eSnsType, AX_IMG_FORMAT_E ePixelFmt,
                               AX_SNS_HDR_MODE_E eHdrMode, AX_ISP_DEV_WORK_MODE_E eWorkMode, AX_BOOL bImgEnable, AX_BOOL bNonImgEnable,
                               AX_BOOL bIspBypass);

AX_S32 raw_file_write(char *pfile, char *ptr, int size);


AX_S32 SampleCommPoolInit(AX_SNS_HDR_MODE_E eSnsMode, SAMPLE_SNS_TYPE_E eSnsType, AX_RAW_TYPE_E raw_type,
                          AX_POOL_FLOORPLAN_T *pPoolFloorPlan, AX_U32 gDumpFrame, AX_U8 eRunMode);

AX_S32 COMMON_ISP_GetSnsConfig(SAMPLE_SNS_TYPE_E eSnsType, AX_SNS_ATTR_T *ptSnsAttr, AX_SNS_CLK_ATTR_T *ptSnsClkAttr, AX_DEV_ATTR_T *pDevAttr,
                               AX_PIPE_ATTR_T *pPipeAttr, AX_VIN_CHN_ATTR_T *pChnAttr);

#ifdef __cplusplus
}
#endif

#endif
