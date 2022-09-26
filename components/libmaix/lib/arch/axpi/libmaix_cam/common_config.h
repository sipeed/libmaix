/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/
#ifndef _COMMON_CONFIG_H__
#define _COMMON_CONFIG_H__

#include "ax_vin_api.h"
#include "ax_mipi_api.h"

AX_MIPI_TX_ATTR_S gOs04a10MipiTxIspBypassAttr = {
    .bIspBypass     = AX_TRUE,
    .eLaneNum       = AX_MIPI_LANE_4,
    .eDataRate      = AX_MIPI_DATA_RATE_800M,   /* tx ratedata need bigger than sensor mipi */
    .eInputSrc      = AX_MIPI_TX_SRC_SNS_2,     /* default as SNS_2, modify by sensor hardware connection */
    .eDolSplitNum   = AX_MIPI_DOL_1,
    .eImgDataType   = AX_MIPI_DT_RAW10,
    .eImgVC         = AX_MIPI_VC_0,
    .nImgWidth      = 2688,
    .nImgHeight     = 1520,

    .nLaneMap[0]    = 0x00, /* lane0 as clock lane */
    .nLaneMap[1]    = 0x01, /* lane1 as clock lane */
    .nLaneMap[2]    = 0x02, /* lane2 as clock lane */
    .nLaneMap[3]    = 0x03, /* lane3 as clock lane */
    .nLaneMap[4]    = 0x04, /* lane4 as clock lane */
};

AX_MIPI_TX_ATTR_S gOs04a10MipiTxAttr = {
    .bIspBypass     = AX_FALSE,
    .eLaneNum       = AX_MIPI_LANE_4,
    .eDataRate      = AX_MIPI_DATA_RATE_2500M,
    .eInputSrc      = AX_MIPI_TX_SRC_RAW_SCALAR_2,
    .eDolSplitNum   = AX_MIPI_DOL_1,
    .eImgDataType   = AX_MIPI_DT_YUV420,
    .eImgVC         = AX_MIPI_VC_0,
    .nImgWidth      = 2688,
    .nImgHeight     = 1520,

    .nLaneMap[0]    = 0x00, /* lane0 as clock lane */
    .nLaneMap[1]    = 0x01, /* lane1 as clock lane */
    .nLaneMap[2]    = 0x02, /* lane2 as clock lane */
    .nLaneMap[3]    = 0x03, /* lane3 as clock lane */
    .nLaneMap[4]    = 0x04, /* lane4 as clock lane */
};

AX_TX_IMG_INFO_T gOs04a10TxImgInfo = {
    .bMipiTxEnable  = AX_TRUE,
    .bIspBypass     = AX_FALSE,
    .nWidth         = 2688,
    .nHeight        = 1520,
    .eImgFormat     = AX_YUV420_SEMIPLANAR,
};

AX_MIPI_RX_ATTR_S gOs04a10MipiAttr = {
    .eLaneNum = AX_MIPI_LANE_4,
    .eDataRate = AX_MIPI_DATA_RATE_80M,
    .ePhySel = AX_MIPI_RX_PHY0_SEL_LANE_0_1_2_3,
    .nLaneMap[0] = 0x00,
    .nLaneMap[1] = 0x01,
    .nLaneMap[2] = 0x03,
    .nLaneMap[3] = 0x04,
    .nLaneMap[4] = 0x02, /* clock lane */

};

AX_SNS_ATTR_T gOs04a10SnsAttr = {
    .nWidth = 2688,
    .nHeight = 1520,
    .nFrameRate = 30,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW10,
    .eSnsHcgLcg = AX_HCG_MODE,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
    .eMasterSlaveSel = AX_SNS_MASTER,
};

AX_SNS_CLK_ATTR_T gOs04a10SnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_24M,
};

AX_DEV_ATTR_T gOs04a10DevAttr = {
    .eDevWorkMode = AX_DEV_WORK_MODE_NORMAL,
    .bImgDataEnable = AX_TRUE,
    .eSnsType = AX_SNS_TYPE_MIPI,
    .tDevImgRgn = {0, 0, 2688, 1520},
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP,
    .bDolSplit = 0,
    .bHMirror = 0,
    .eBayerPattern = AX_BP_RGGB,
    .eSkipFrame = AX_SNS_SKIP_FRAME_NO_SKIP,
    .eSnsGainMode = 0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .bNonImgEnable = AX_FALSE,
    .nNonImgWidth = 2688,
    .nNonImgHeight = 1520,
    .eNonPixelFmt = AX_FORMAT_BAYER_RAW_10BPP,
    .nNonImgDT = 0x2a,/* 0x3F, */
    .nNonImgVC = 0,
    .bIspBypass = AX_FALSE,
};

AX_PIPE_ATTR_T gOs04a10PipeAttr = {
    .nWidth = 2688,
    .nHeight = 1520,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .ePipeDataSrc = AX_PIPE_SOURCE_DEV_OFFLINE,
    .eDataFlowType = AX_DATAFLOW_TYPE_NORMAL,
    .eDevSource = AX_DEV_SOURCE_SNS_0,
    .ePreOutput = AX_PRE_OUTPUT_FULL_MAIN,
};

AX_VIN_CHN_ATTR_T gOs04a10ChnAttr = {
    .tChnAttr[AX_YUV_SOURCE_ID_MAIN] = {
        .nWidth = 2688,
        .nHeight = 1520,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 2688,
        .nDepth = 3
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB1] = {
        .nWidth = 1920,
        .nHeight = 1080,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 1920,
        .nDepth = 3
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB2] = {
        .nWidth = 720,
        .nHeight = 576,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 720,
        .nDepth = 3
    },
};

/* sony imx334 sensor */
AX_MIPI_TX_ATTR_S gImx334MipiTxIspBypassAttr = {
    .bIspBypass     = AX_TRUE,
    .eLaneNum       = AX_MIPI_LANE_4,
    .eDataRate      = AX_MIPI_DATA_RATE_1000M, /* tx ratedata need bigger than sensor mipi */
    .eInputSrc      = AX_MIPI_TX_SRC_SNS_0,    /* default as SNS_0, modify by sensor hardware connection */
    .eDolSplitNum   = AX_MIPI_DOL_1,
    .eImgDataType   = AX_MIPI_DT_RAW10,
    .eImgVC         = AX_MIPI_VC_0,
    .nImgWidth      = 3840,
    .nImgHeight     = 2160,

    .nLaneMap[0]    = 0x00, /* lane0 as clock lane */
    .nLaneMap[1]    = 0x01, /* lane1 as clock lane */
    .nLaneMap[2]    = 0x02, /* lane2 as clock lane */
    .nLaneMap[3]    = 0x03, /* lane3 as clock lane */
    .nLaneMap[4]    = 0x04, /* lane4 as clock lane */

};

AX_MIPI_TX_ATTR_S gImx334MipiTxAttr = {
    .bIspBypass     = AX_FALSE,
    .eLaneNum       = AX_MIPI_LANE_4,
    .eDataRate      = AX_MIPI_DATA_RATE_2500M,
    .eInputSrc      = AX_MIPI_TX_SRC_RAW_SCALAR_2,
    .eDolSplitNum   = AX_MIPI_DOL_1,
    .eImgDataType   = AX_MIPI_DT_YUV420,
    .eImgVC         = AX_MIPI_VC_0,
    .nImgWidth      = 3840,
    .nImgHeight     = 2160,

    .nLaneMap[0]    = 0x00, /* lane0 as clock lane */
    .nLaneMap[1]    = 0x01, /* lane1 as clock lane */
    .nLaneMap[2]    = 0x02, /* lane2 as clock lane */
    .nLaneMap[3]    = 0x03, /* lane3 as clock lane */
    .nLaneMap[4]    = 0x04, /* lane4 as clock lane */

};

AX_TX_IMG_INFO_T gImx334TxImgInfo = {
    .bMipiTxEnable  = AX_TRUE,
    .bIspBypass     = AX_FALSE,
    .nWidth         = 3840,
    .nHeight        = 2160,
    .eImgFormat     = AX_YUV420_SEMIPLANAR,
};

AX_MIPI_RX_ATTR_S gImx334MipiAttr = {
    .eLaneNum = AX_MIPI_LANE_4,
    .eDataRate = AX_MIPI_DATA_RATE_80M,
    .ePhySel = AX_MIPI_RX_PHY0_SEL_LANE_0_1_2_3,
    .nLaneMap[0] = 0x00,
    .nLaneMap[1] = 0x01,
    .nLaneMap[2] = 0x03,
    .nLaneMap[3] = 0x04,
    .nLaneMap[4] = 0x02, /* clock lane */
};

AX_SNS_ATTR_T gImx334SnsAttr = {
    .nWidth = 3840,
    .nHeight = 2160,
    .nFrameRate = 30,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW10,
    .eSnsHcgLcg = AX_LCG_NOTSUPPORT_MODE,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
    .eMasterSlaveSel = AX_SNS_MASTER,
};

AX_SNS_CLK_ATTR_T gImx334SnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_27M,
};

AX_DEV_ATTR_T gImx334DevAttr = {
    .eDevWorkMode = AX_DEV_WORK_MODE_NORMAL,
    .bImgDataEnable = AX_TRUE,
    .eSnsType = AX_SNS_TYPE_MIPI,
    .tDevImgRgn = {12, 10, 3840, 2160},
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP,
    .bDolSplit = 0,
    .bHMirror = 0,
    .eBayerPattern = AX_BP_RGGB,
    .eSkipFrame = AX_SNS_SKIP_FRAME_NO_SKIP,
    .eSnsGainMode = 0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .bNonImgEnable = AX_FALSE,
    .nNonImgWidth = 3840,
    .nNonImgHeight = 2160,
    .eNonPixelFmt = AX_FORMAT_BAYER_RAW_10BPP,
    .nNonImgDT = 0x2a,/* 0x3F, */
    .nNonImgVC = 0,
    .bIspBypass = AX_FALSE,
};

AX_PIPE_ATTR_T gImx334PipeAttr = {
    .nWidth = 3840,
    .nHeight = 2160,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .ePipeDataSrc = AX_PIPE_SOURCE_DEV_OFFLINE,
    .eDataFlowType = AX_DATAFLOW_TYPE_NORMAL,
    .eDevSource = AX_DEV_SOURCE_SNS_0,
    .ePreOutput = AX_PRE_OUTPUT_FULL_MAIN,
};

AX_VIN_CHN_ATTR_T gImx334ChnAttr = {
    .tChnAttr[AX_YUV_SOURCE_ID_MAIN] = {
        .nWidth = 3840,
        .nHeight = 2160,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 3840,
        .nDepth = 3
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB1] = {
        .nWidth = 1920,
        .nHeight = 1080,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 1920,
        .nDepth = 3
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB2] = {
        .nWidth = 960,
        .nHeight = 540,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 960,
        .nDepth = 3
    },
};

/* gc gc4653 sensor */
AX_MIPI_TX_ATTR_S gGc4653MipiTxIspBypassAttr = {
    .bIspBypass     = AX_TRUE,
    .eLaneNum       = AX_MIPI_LANE_2,
    .eDataRate      = AX_MIPI_DATA_RATE_800M,   /* tx ratedata need bigger than sensor mipi */
    .eInputSrc      = AX_MIPI_TX_SRC_SNS_2,     /* default as SNS_2, modify by sensor hardware connection */
    .eDolSplitNum   = AX_MIPI_DOL_1,
    .eImgDataType   = AX_MIPI_DT_RAW10,
    .eImgVC         = AX_MIPI_VC_0,
    .nImgWidth      = 2560,
    .nImgHeight     = 1440,

    .nLaneMap[0]    = 0x00, /* lane0 as clock lane */
    .nLaneMap[1]    = 0x01, /* lane1 as clock lane */
    .nLaneMap[2]    = 0x02, /* lane2 as clock lane */
    .nLaneMap[3]    = 0x03, /* lane3 as clock lane */
    .nLaneMap[4]    = 0x04, /* lane4 as clock lane */
};

AX_MIPI_TX_ATTR_S gGc4653MipiTxAttr = {
    .bIspBypass     = AX_FALSE,
    .eLaneNum       = AX_MIPI_LANE_2,
    .eDataRate      = AX_MIPI_DATA_RATE_80M,
    .eInputSrc      = AX_MIPI_TX_SRC_RAW_SCALAR_2,
    .eDolSplitNum   = AX_MIPI_DOL_1,
    .eImgDataType   = AX_MIPI_DT_YUV420,
    .eImgVC         = AX_MIPI_VC_0,
    .nImgWidth      = 2560,
    .nImgHeight     = 1440,

    .nLaneMap[0]    = 0x00, /* lane0 as clock lane */
    .nLaneMap[1]    = 0x01, /* lane1 as clock lane */
    .nLaneMap[2]    = 0x02, /* lane2 as clock lane */
    .nLaneMap[3]    = 0x03, /* lane3 as clock lane */
    .nLaneMap[4]    = 0x04, /* lane4 as clock lane */
};

AX_TX_IMG_INFO_T gGc4653TxImgInfo = {
    .bMipiTxEnable  = AX_TRUE,
    .bIspBypass     = AX_FALSE,
    .nWidth         = 2560,
    .nHeight        = 1440,
    .eImgFormat     = AX_YUV420_SEMIPLANAR,
};

AX_MIPI_RX_ATTR_S gGc4653MipiAttr = {
    .eLaneNum = AX_MIPI_LANE_2,
    .eDataRate = AX_MIPI_DATA_RATE_80M,
    .ePhySel = AX_MIPI_RX_PHY0_SEL_LANE_0_1_2_3,
    .nLaneMap[0] = 0x00,
    .nLaneMap[1] = 0x01,
    .nLaneMap[2] = 0x03,
    .nLaneMap[3] = 0x04,
    .nLaneMap[4] = 0x02, /* clock lane */
};

AX_SNS_ATTR_T gGc4653SnsAttr = {
    .nWidth = 2560,
    .nHeight = 1440,
    .nFrameRate = 30,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW10,
    .eSnsHcgLcg = AX_LCG_NOTSUPPORT_MODE,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
    .eMasterSlaveSel = AX_SNS_MASTER,
};

AX_SNS_CLK_ATTR_T gGc4653SnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_24M,
};

AX_DEV_ATTR_T gGc4653DevAttr = {
    .eDevWorkMode = AX_DEV_WORK_MODE_NORMAL,
    .bImgDataEnable = AX_TRUE,
    .eSnsType = AX_SNS_TYPE_MIPI,
    .tDevImgRgn = {0, 0, 2560, 1440},
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP,
    .bDolSplit = 0,
    .bHMirror = 0,
    .eBayerPattern = AX_BP_RGGB,
    .eSkipFrame = AX_SNS_SKIP_FRAME_NO_SKIP,
    .eSnsGainMode = 0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .bNonImgEnable = AX_FALSE,
    .nNonImgWidth = 2560,
    .nNonImgHeight = 1440,
    .eNonPixelFmt = AX_FORMAT_BAYER_RAW_10BPP,
    .nNonImgDT = 0x2a,/* 0x3F, */
    .nNonImgVC = 0,
    .bIspBypass = AX_FALSE,
};

AX_PIPE_ATTR_T gGc4653PipeAttr = {
    .nWidth = 2560,
    .nHeight = 1440,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .ePipeDataSrc = AX_PIPE_SOURCE_DEV_ONLINE,
    .eDataFlowType = AX_DATAFLOW_TYPE_MULTIPLEX,
    .eDevSource = AX_DEV_SOURCE_SNS_0,
    .ePreOutput = AX_PRE_OUTPUT_FULL_MAIN,
};

AX_VIN_CHN_ATTR_T gGc4653ChnAttr = {
    .tChnAttr[AX_YUV_SOURCE_ID_MAIN] = {
        .nWidth = 2560,
        .nHeight = 1440,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 2560,
        .nDepth = 1
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB1] = {
        .nWidth = 1280,
        .nHeight = 720,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 1280,
        .nDepth = 1
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB2] = {
        .nWidth = 640,
        .nHeight = 360,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 640,
        .nDepth = 1
    },
};

/*  ov_os08a20 sensor  */
AX_MIPI_TX_ATTR_S gOs08a20MipiTxIspBypassAttr = {
    .bIspBypass     = AX_TRUE,
    .eLaneNum       = AX_MIPI_LANE_4,
    .eDataRate      = AX_MIPI_DATA_RATE_1000M,  /* tx ratedata need bigger than sensor mipi */
    .eInputSrc      = AX_MIPI_TX_SRC_SNS_0,     /* default as SNS_0, modify by sensor hardware connection */
    .eDolSplitNum   = AX_MIPI_DOL_1,
    .eImgDataType   = AX_MIPI_DT_RAW12,
    .eImgVC         = AX_MIPI_VC_0,
    .nImgWidth      = 3840,
    .nImgHeight     = 2160,

    .nLaneMap[0]    = 0x00, /* lane0 as clock lane */
    .nLaneMap[1]    = 0x01, /* lane1 as clock lane */
    .nLaneMap[2]    = 0x02, /* lane2 as clock lane */
    .nLaneMap[3]    = 0x03, /* lane3 as clock lane */
    .nLaneMap[4]    = 0x04, /* lane4 as clock lane */
};

/*  ov_os08a20 sensor  */
AX_MIPI_TX_ATTR_S gOs08a20MipiTxAttr = {
    .bIspBypass     = AX_FALSE,
    .eLaneNum       = AX_MIPI_LANE_4,
    .eDataRate      = AX_MIPI_DATA_RATE_2500M,
    .eInputSrc      = AX_MIPI_TX_SRC_RAW_SCALAR_2,
    .eDolSplitNum   = AX_MIPI_DOL_1,
    .eImgDataType   = AX_MIPI_DT_YUV420,
    .eImgVC         = AX_MIPI_VC_0,
    .nImgWidth      = 3840,
    .nImgHeight     = 2160,

    .nLaneMap[0]    = 0x00, /* lane0 as clock lane */
    .nLaneMap[1]    = 0x01, /* lane1 as clock lane */
    .nLaneMap[2]    = 0x02, /* lane2 as clock lane */
    .nLaneMap[3]    = 0x03, /* lane3 as clock lane */
    .nLaneMap[4]    = 0x04, /* lane4 as clock lane */
};

AX_TX_IMG_INFO_T gOs08a20TxImgInfo = {
    .bMipiTxEnable  = AX_TRUE,
    .bIspBypass     = AX_FALSE,
    .nWidth         = 3840,
    .nHeight        = 2160,
    .eImgFormat     = AX_YUV420_SEMIPLANAR,
};

AX_MIPI_RX_ATTR_S gOs08a20MipiAttr = {
    .eLaneNum = AX_MIPI_LANE_4,
    .eDataRate = AX_MIPI_DATA_RATE_80M,
    .ePhySel = AX_MIPI_RX_PHY0_SEL_LANE_0_1_2_3,
    .nLaneMap[0] = 0x00,
    .nLaneMap[1] = 0x01,
    .nLaneMap[2] = 0x03,
    .nLaneMap[3] = 0x04,
    .nLaneMap[4] = 0x02, /* clock lane */
};

AX_SNS_ATTR_T gOs08a20SnsAttr = {
    .nWidth = 3840,
    .nHeight = 2160,
    .nFrameRate = 30,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW12,
    .eSnsHcgLcg = AX_LCG_NOTSUPPORT_MODE,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
    .eMasterSlaveSel = AX_SNS_MASTER,
};

AX_SNS_CLK_ATTR_T gOs08a20SnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_24M,
};

AX_DEV_ATTR_T gOs08a20DevAttr = {
    .eDevWorkMode = AX_DEV_WORK_MODE_NORMAL,
    .bImgDataEnable = AX_TRUE,
    .eSnsType = AX_SNS_TYPE_MIPI,
    .tDevImgRgn = {0, 0, 3840, 2160},
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP,
    .bDolSplit = 0,
    .bHMirror = 0,
    .eBayerPattern = AX_BP_RGGB,
    .eSkipFrame = AX_SNS_SKIP_FRAME_NO_SKIP,
    .eSnsGainMode = 0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .bNonImgEnable = AX_FALSE,
    .nNonImgWidth = 3840,
    .nNonImgHeight = 2160,
    .eNonPixelFmt = AX_FORMAT_BAYER_RAW_10BPP,
    .nNonImgDT = 0x2a,/* 0x3F, */
    .nNonImgVC = 0,
    .bIspBypass = AX_FALSE,
};

AX_PIPE_ATTR_T gOs08a20PipeAttr = {
    .nWidth = 3840,
    .nHeight = 2160,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_10BPP,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .ePipeDataSrc = AX_PIPE_SOURCE_DEV_OFFLINE,
    .eDataFlowType = AX_DATAFLOW_TYPE_NORMAL,
    .eDevSource = AX_DEV_SOURCE_SNS_0,
    .ePreOutput = AX_PRE_OUTPUT_FULL_MAIN,
};

AX_VIN_CHN_ATTR_T gOs08a20ChnAttr = {
    .tChnAttr[AX_YUV_SOURCE_ID_MAIN] = {
        .nWidth = 3840,
        .nHeight = 2160,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 3840,
        .nDepth = 3
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB1] = {
        .nWidth = 1344,
        .nHeight = 760,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 1344,
        .nDepth = 3
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB2] = {
        .nWidth = 672,
        .nHeight = 380,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 672,
        .nDepth = 3
    },
};

/*  DVP sensor  */
AX_SNS_ATTR_T gDVPSnsAttr = {
    .nWidth = 1600,
    .nHeight = 300,
    .nFrameRate = 25,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eRawType = AX_RT_RAW8,
    .eSnsHcgLcg = AX_LCG_MODE,
    .eBayerPattern = AX_BP_RGGB,
    .bTestPatternEnable = AX_FALSE,
    .eMasterSlaveSel = AX_SNS_MASTER,
};

AX_SNS_CLK_ATTR_T gDVPSnsClkAttr = {
    .nSnsClkIdx = 0,
    .eSnsClkRate = AX_SNS_CLK_24M,
};


AX_DEV_ATTR_T gDVPDevAttr = {
    .eDevWorkMode = AX_DEV_WORK_MODE_NORMAL,
    .bImgDataEnable = AX_TRUE,
    .eSnsType = AX_SNS_TYPE_DVP,
    .tDevImgRgn = {0, 0, 1600, 300},
    .ePixelFmt = AX_FORMAT_BAYER_RAW_8BPP,
    .bDolSplit = 0,
    .bHMirror = 0,
    .eBayerPattern = AX_BP_RGGB,
    .eSkipFrame = AX_SNS_SKIP_FRAME_NO_SKIP,
    .eSnsGainMode = 0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,

    .tDvpIntfAttr = {
        .eDataSeq = AX_VIN_DATA_SEQ_UYVY,
        .nComponentMask[0] = 0x00ff,
        .nComponentMask[1] = 0x00ff,
    },

    .bNonImgEnable = AX_FALSE,
    .nNonImgWidth = 1600,
    .nNonImgHeight = 300,
    .eNonPixelFmt = AX_FORMAT_BAYER_RAW_8BPP,
    .nNonImgDT = 0x2a,/* 0x3F, */
    .nNonImgVC = 0,
    .bIspBypass = AX_FALSE,
};

AX_PIPE_ATTR_T gDVPPipeAttr = {
    .nWidth = 1600,
    .nHeight = 300,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_8BPP,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .ePipeDataSrc = AX_PIPE_SOURCE_DEV_OFFLINE,
    .eDataFlowType = AX_DATAFLOW_TYPE_MULTIPLEX,
    .eDevSource = AX_DEV_SOURCE_SNS_0,
    .ePreOutput = AX_PRE_OUTPUT_FULL_MAIN,
};

AX_VIN_CHN_ATTR_T gDVPChnAttr = {
    .tChnAttr[AX_YUV_SOURCE_ID_MAIN] = {
        .nWidth = 1600,
        .nHeight = 300,
        .eImgFormat = AX_YUV422_INTERLEAVED_UYVY,
        .bEnable = 1,
        .nWidthStride = 1600,
        .nDepth = 1
    },

};

/*  BT601 */
AX_DEV_ATTR_T gBT601DevAttr = {
    .eDevWorkMode = AX_DEV_WORK_MODE_NORMAL,
    .bImgDataEnable = AX_TRUE,
    .eSnsType = AX_SNS_TYPE_BT601,
    .tDevImgRgn = {0, 0, 1920, 1080},
    .ePixelFmt = AX_YUV422_INTERLEAVED_UYVY,
    .bDolSplit = 0,
    .bHMirror = 0,
    .eBayerPattern = AX_BP_RGGB,
    .eSkipFrame = AX_SNS_SKIP_FRAME_NO_SKIP,
    .eSnsGainMode = 0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,

    .tBtIntfAttr = {
        .eScanMode = AX_VIN_SCAN_INTERLACED,
        .nComponentMask[0] = 0xFF300,
        .nComponentMask[1] = 0x003fc,
        .tSyncCfg = {
            .eVsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
            .eHsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
        },
    },

    .bNonImgEnable = AX_FALSE,
    .nNonImgWidth = 1920,
    .nNonImgHeight = 1080,
    .eNonPixelFmt = AX_YUV422_INTERLEAVED_UYVY,
    .nNonImgDT = 0,/* 0x3F, */
    .nNonImgVC = 0,
    .bIspBypass = AX_FALSE,
};


AX_PIPE_ATTR_T gBT601PipeAttr = {
    .nWidth = 1920,
    .nHeight = 1080,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_YUV422_INTERLEAVED_UYVY,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .ePipeDataSrc = AX_PIPE_SOURCE_DEV_OFFLINE,
    .eDataFlowType = AX_DATAFLOW_TYPE_NORMAL,
    .eDevSource = AX_DEV_SOURCE_SNS_0,
    .ePreOutput = AX_PRE_OUTPUT_FULL_MAIN,
};

AX_VIN_CHN_ATTR_T gBT601ChnAttr = {
    .tChnAttr[AX_YUV_SOURCE_ID_MAIN] = {
        .nWidth = 1920,
        .nHeight = 1080,
        .eImgFormat = AX_YUV422_INTERLEAVED_UYVY,
        .bEnable = 1,
        .nWidthStride = 1920,
        .nDepth = 8
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB1] = {
        .nWidth = 1280,
        .nHeight = 720,
        .eImgFormat = AX_YUV422_INTERLEAVED_UYVY,
        .bEnable = 1,
        .nWidthStride = 1280,
        .nDepth = 6
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB2] = {
        .nWidth = 672,
        .nHeight = 380,
        .eImgFormat = AX_YUV422_INTERLEAVED_UYVY,
        .bEnable = 1,
        .nWidthStride = 672,
        .nDepth = 6
    },
};

/*  BT656 */
AX_DEV_ATTR_T gBT656DevAttr = {
    .eDevWorkMode = AX_DEV_WORK_MODE_NORMAL,
    .bImgDataEnable = AX_TRUE,
    .eSnsType = AX_SNS_TYPE_BT656,
    .tDevImgRgn = {0, 0, 1920, 1080},
    .ePixelFmt = AX_YUV422_INTERLEAVED_UYVY,
    .bDolSplit = 0,
    .bHMirror = 0,
    .eBayerPattern = AX_BP_RGGB,
    .eSkipFrame = AX_SNS_SKIP_FRAME_NO_SKIP,
    .eSnsGainMode = 0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,

    .tBtIntfAttr = {
        .eScanMode = AX_VIN_SCAN_INTERLACED,
        .nComponentMask[0] = 0x003fc,
        .nComponentMask[1] = 0x003fc,
        .tSyncCfg = {
            .eVsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
            .eHsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
        },
    },

    .bNonImgEnable = AX_FALSE,
    .nNonImgWidth = 1920,
    .nNonImgHeight = 1080,
    .eNonPixelFmt = AX_YUV422_INTERLEAVED_UYVY,
    .nNonImgDT = 0,/* 0x3F, */
    .nNonImgVC = 0,
    .bIspBypass = AX_FALSE,
};


AX_PIPE_ATTR_T gBT656PipeAttr = {
    .nWidth = 1920,
    .nHeight = 1080,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_YUV422_INTERLEAVED_UYVY,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .ePipeDataSrc = AX_PIPE_SOURCE_DEV_OFFLINE,
    .eDataFlowType = AX_DATAFLOW_TYPE_NORMAL,
    .eDevSource = AX_DEV_SOURCE_SNS_0,
    .ePreOutput = AX_PRE_OUTPUT_FULL_MAIN,
};

AX_VIN_CHN_ATTR_T gBT656ChnAttr = {
    .tChnAttr[AX_YUV_SOURCE_ID_MAIN] = {
        .nWidth = 1920,
        .nHeight = 1080,
        .eImgFormat = AX_YUV422_INTERLEAVED_UYVY,
        .bEnable = 1,
        .nWidthStride = 1920,
        .nDepth = 8
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB1] = {
        .nWidth = 1280,
        .nHeight = 720,
        .eImgFormat = AX_YUV422_INTERLEAVED_UYVY,
        .bEnable = 1,
        .nWidthStride = 1280,
        .nDepth = 6
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB2] = {
        .nWidth = 672,
        .nHeight = 380,
        .eImgFormat = AX_YUV422_INTERLEAVED_UYVY,
        .bEnable = 1,
        .nWidthStride = 672,
        .nDepth = 6
    },
};

/*  BT1120 */
AX_DEV_ATTR_T gBT1120DevAttr = {
    .eDevWorkMode = AX_DEV_WORK_MODE_NORMAL,
    .bImgDataEnable = AX_TRUE,
    .eSnsType = AX_SNS_TYPE_BT1120,
    .tDevImgRgn = {0, 0, 1920, 1080},
    .ePixelFmt = AX_YUV422_INTERLEAVED_UYVY,
    .bDolSplit = 0,
    .bHMirror = 0,
    .eBayerPattern = AX_BP_RGGB,
    .eSkipFrame = AX_SNS_SKIP_FRAME_NO_SKIP,
    .eSnsGainMode = 0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,

    .tBtIntfAttr = {
        .eScanMode = AX_VIN_SCAN_INTERLACED,
        .nComponentMask[0] = 0xFF300,
        .nComponentMask[1] = 0x003fc,
        .tSyncCfg = {
            .eVsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
            .eHsyncInv = AX_VIN_SYNC_POLARITY_HIGH,
        },
    },

    .bNonImgEnable = AX_FALSE,
    .nNonImgWidth = 1920,
    .nNonImgHeight = 1080,
    .eNonPixelFmt = AX_YUV422_INTERLEAVED_UYVY,
    .nNonImgDT = 0,/* 0x3F, */
    .nNonImgVC = 0,
    .bIspBypass = AX_FALSE,
};


AX_PIPE_ATTR_T gBT1120PipeAttr = {
    .nWidth = 1920,
    .nHeight = 1080,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_YUV422_INTERLEAVED_UYVY,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .ePipeDataSrc = AX_PIPE_SOURCE_DEV_OFFLINE,
    .eDataFlowType = AX_DATAFLOW_TYPE_NORMAL,
    .eDevSource = AX_DEV_SOURCE_SNS_0,
    .ePreOutput = AX_PRE_OUTPUT_FULL_MAIN,
};

AX_VIN_CHN_ATTR_T gBT1120ChnAttr = {
    .tChnAttr[AX_YUV_SOURCE_ID_MAIN] = {
        .nWidth = 1920,
        .nHeight = 1080,
        .eImgFormat = AX_YUV422_INTERLEAVED_UYVY,
        .bEnable = 1,
        .nWidthStride = 1920,
        .nDepth = 8
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB1] = {
        .nWidth = 1280,
        .nHeight = 720,
        .eImgFormat = AX_YUV422_INTERLEAVED_UYVY,
        .bEnable = 1,
        .nWidthStride = 1280,
        .nDepth = 6
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB2] = {
        .nWidth = 672,
        .nHeight = 380,
        .eImgFormat = AX_YUV422_INTERLEAVED_UYVY,
        .bEnable = 1,
        .nWidthStride = 672,
        .nDepth = 6
    },
};

/*  MIPI_YUV sensor  */

AX_MIPI_RX_ATTR_S gMIPI_YUVMipiAttr = {
    .eLaneNum = AX_MIPI_LANE_4,
    .eDataRate = AX_MIPI_DATA_RATE_80M,
    .ePhySel = AX_MIPI_RX_PHY0_SEL_LANE_0_1_2_3,
    .nLaneMap[0] = 0x00,
    .nLaneMap[1] = 0x01,
    .nLaneMap[2] = 0x03,
    .nLaneMap[3] = 0x04,
    .nLaneMap[4] = 0x02, /* clock lane */
};

AX_DEV_ATTR_T gMIPI_YUVDevAttr = {
    .eDevWorkMode = AX_DEV_WORK_MODE_NORMAL,
    .bImgDataEnable = AX_TRUE,
    .eSnsType = AX_SNS_TYPE_MIPI_YUV,
    .tDevImgRgn = {0, 0, 3840, 2160},
    .ePixelFmt = AX_YUV422_INTERLEAVED_UYVY,
    .bDolSplit = 0,
    .bHMirror = 0,
    .eBayerPattern = AX_BP_RGGB,
    .eSkipFrame = AX_SNS_SKIP_FRAME_NO_SKIP,
    .eSnsGainMode = 0,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .eSnsOutputMode = AX_SNS_NORMAL,
    .bNonImgEnable = AX_FALSE,
    .nNonImgWidth = 3840 * 2,
    .nNonImgHeight = 2160,
    .eNonPixelFmt = AX_FORMAT_BAYER_RAW_8BPP,
    .nNonImgDT = 0x1E,/* 0x3F, */
    .nNonImgVC = 0,
    .bIspBypass = AX_FALSE,
};

AX_PIPE_ATTR_T gMIPI_YUVPipeAttr = {
    .nWidth = 3840,
    .nHeight = 2160,
    .eBayerPattern = AX_BP_RGGB,
    .ePixelFmt = AX_FORMAT_BAYER_RAW_8BPP,
    .eSnsMode = AX_SNS_LINEAR_MODE,
    .ePipeDataSrc = AX_PIPE_SOURCE_DEV_OFFLINE,
    .eDataFlowType = AX_DATAFLOW_TYPE_NORMAL,
    .eDevSource = AX_DEV_SOURCE_SNS_0,
    .ePreOutput = AX_PRE_OUTPUT_FULL_MAIN,
};

AX_VIN_CHN_ATTR_T gMIPI_YUVChnAttr = {
    .tChnAttr[AX_YUV_SOURCE_ID_MAIN] = {
        .nWidth = 3840,
        .nHeight = 2160,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 3840,
        .nDepth = 3
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB1] = {
        .nWidth = 1344,
        .nHeight = 760,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 1344,
        .nDepth = 3
    },
    .tChnAttr[AX_YUV_SOURCE_ID_SUB2] = {
        .nWidth = 672,
        .nHeight = 380,
        .eImgFormat = AX_YUV420_SEMIPLANAR,
        .bEnable = 1,
        .nWidthStride = 672,
        .nDepth = 3
    },
};

#endif //_COMMON_CONFIG_H__
