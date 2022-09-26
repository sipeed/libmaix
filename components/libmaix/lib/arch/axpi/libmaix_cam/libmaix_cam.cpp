#include "libmaix_cam.h"

#include "libmaix_cam_priv.h"

#include <cstring>

#include <opencv2/opencv.hpp>
#include "opencv2/core/types_c.h"

extern "C" {

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "ax_interpreter_external_api.h"
#include "ax_isp_api.h"
#include "ax_ivps_api.h"
#include "ax_venc_api.h"

#define COMM_ISP_PRT(fmt...)   \
do {\
    printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)

#include "common_sys.h"
#include "common_vin.h"
#include "common_cam.h"
#include "sample_resize.h"

#define SAMPLE_IVPS_GROUP_NUM (3)
#define SAMPLE_IVPS_CHN_NUM   (1)

typedef enum {
    SYS_CASE_NONE  = -1,
    SYS_CASE_SINGLE_OS04A10  = 0,
    SYS_CASE_SINGLE_GC4653   = 2,
} COMMON_SYS_CASE_E;

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10Sdr[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 8}, /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 8}, /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 5},      /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 5},
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 5},
};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleOs04a10Hdr[] = {
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_10BPP, 15}, /*vin raw10 use */
    {2688, 1520, 2688, AX_FORMAT_BAYER_RAW_16BPP, 5},  /*vin raw16 use */
    {2688, 1520, 2688, AX_YUV420_SEMIPLANAR, 5},       /*vin nv21/nv21 use */
    {1920, 1080, 1920, AX_YUV420_SEMIPLANAR, 4},
    {720, 576, 720, AX_YUV420_SEMIPLANAR, 4},
};

static COMMON_SYS_POOL_CFG_T gtSysCommPoolSingleGc4653[] = {

    {2560, 1440, 2560, AX_FORMAT_BAYER_RAW_10BPP, 5},     /*vin raw10 use */
    {2560, 1440, 2560, AX_FORMAT_BAYER_RAW_16BPP, 5},     /*vin raw16 use */
    {2560, 1440, 2560, AX_YUV420_SEMIPLANAR, 6},     /*vin nv21/nv21 use */
    {1280, 720, 1280, AX_YUV420_SEMIPLANAR, 3},
    {640, 360, 640, AX_YUV420_SEMIPLANAR, 3},

};

static CAMERA_T gCams[MAX_CAMERAS] = {0};

static AX_S32 g_isp_force_loop_exit = 0;

void *IspRun(void *args)
{
    AX_U32 i = (AX_U32)args;

    COMM_ISP_PRT("cam %d is running...\n", i);

    while (!g_isp_force_loop_exit) {
        if (gCams[i].bOpen)
            AX_ISP_Run(gCams[i].nPipeId);
    }
    return NULL;
}

void libmaix_camera_module_init()
{
    COMM_ISP_PRT("ISP Sample. Build at %s %s\n", __DATE__, __TIME__);

    COMMON_SYS_ARGS_T tCommonArgs = {0};
    COMMON_SYS_CASE_E eSysCase = SYS_CASE_SINGLE_GC4653;
    AX_SNS_HDR_MODE_E eHdrMode = AX_SNS_LINEAR_MODE;
    SAMPLE_SNS_TYPE_E eSnsType = OMNIVISION_OS04A10;
    AX_S32 axRet = 0;

    if (eSysCase == SYS_CASE_SINGLE_OS04A10) {
        tCommonArgs.nCamCnt = 1;
        eSnsType = OMNIVISION_OS04A10;
        COMMON_ISP_GetSnsConfig(OMNIVISION_OS04A10, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr, &gCams[0].stChnAttr);
        if (eHdrMode == AX_SNS_LINEAR_MODE){
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10Sdr) / sizeof(gtSysCommPoolSingleOs04a10Sdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolSingleOs04a10Sdr;
        } else if(eHdrMode == AX_SNS_HDR_2X_MODE){
            tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleOs04a10Hdr) / sizeof(gtSysCommPoolSingleOs04a10Hdr[0]);
            tCommonArgs.pPoolCfg  = gtSysCommPoolSingleOs04a10Hdr;
        }
    } else if (eSysCase == SYS_CASE_SINGLE_GC4653) {
        tCommonArgs.nCamCnt = 1;
        eSnsType = GALAXYCORE_GC4653;
        tCommonArgs.nPoolCfgCnt = sizeof(gtSysCommPoolSingleGc4653) / sizeof(gtSysCommPoolSingleGc4653[0]);
        tCommonArgs.pPoolCfg  = gtSysCommPoolSingleGc4653;
        COMMON_ISP_GetSnsConfig(GALAXYCORE_GC4653, &gCams[0].stSnsAttr, &gCams[0].stSnsClkAttr, &gCams[0].stDevAttr, &gCams[0].stPipeAttr,
                                &gCams[0].stChnAttr);
    }

    for (int i = 0; i < tCommonArgs.nCamCnt; i++) {
        gCams[i].eSnsType = eSnsType;
        gCams[i].stSnsAttr.eSnsMode = eHdrMode;
        gCams[i].stDevAttr.eSnsMode = eHdrMode;
        gCams[i].stPipeAttr.eSnsMode = eHdrMode;

        if (i == 0) {
            gCams[i].nDevId = 0;
            gCams[i].nRxDev = AX_MIPI_RX_DEV_0;
            gCams[i].nPipeId = 0;
        } else if (i == 1) {
            gCams[i].nDevId = 2;
            gCams[i].nRxDev = AX_MIPI_RX_DEV_2;
            gCams[i].nPipeId = 2;
        }
    }

    AX_NPU_SDK_EX_ATTR_T sNpuAttr;
    sNpuAttr.eHardMode = AX_NPU_VIRTUAL_1_1;
    axRet = AX_NPU_SDK_EX_Init_with_attr(&sNpuAttr);
    if (0 != axRet) {
        COMM_ISP_PRT("AX_NPU_SDK_EX_Init_with_attr failed, ret=0x%x.\n", axRet);
        exit(-233);
    }

    axRet = COMMON_SYS_Init(&tCommonArgs);
    if (axRet) {
        COMM_ISP_PRT("isp sys init fail\n");
        exit(-233);
    }

    COMMON_CAM_Init();

    for (int i = 0; i < tCommonArgs.nCamCnt; i++) {
        axRet = COMMON_CAM_Open(&gCams[i]);
        if (axRet) {
            COMM_ISP_PRT("COMMON_CAM_Open failed\n", i);
            exit(-233);
        }
        gCams[i].bOpen = AX_TRUE;
        COMM_ISP_PRT("camera %d is open\n", i);
    }

    g_isp_force_loop_exit = 0;
    for (int i = 0; i < MAX_CAMERAS; i++) {
        if (gCams[i].bOpen)
            pthread_create(&gCams[i].tIspProcThread, NULL, IspRun, (AX_VOID *)i);;
    }

    return ;
}

void libmaix_camera_module_deinit()
{
    AX_S32 axRet = 0;
    g_isp_force_loop_exit = 1;
    for (int i = 0; i < MAX_CAMERAS; i++) {
        if (gCams[i].bOpen) {
            pthread_cancel(gCams[i].tIspProcThread);
            axRet = pthread_join(gCams[i].tIspProcThread, NULL);
            if (axRet < 0) {
                COMM_ISP_PRT(" isp run thread exit failed, ret=0x%x.\n", axRet);
            }
        }
    }

    for (int i = 0; i < MAX_CAMERAS; i++) {
        if (!gCams[i].bOpen)
            continue;
        COMMON_CAM_Close(&gCams[i]);
    }

    COMMON_CAM_Deinit();
    COMMON_SYS_DeInit();

    return ;
}

static int SampleIvpsInit()
{
    AX_S32 s32Ret;
    AX_S32 nGrp, nChn;
    AX_IVPS_GRP_ATTR_S stGrpAttr = {0};
    AX_IVPS_PIPELINE_ATTR_S stPipelineAttr = {0};

    s32Ret = AX_IVPS_Init();
    if (0 != s32Ret)
    {
        COMM_ISP_PRT("AX_IVPS_Init failed,s32Ret:0x%x\n", s32Ret);
        return s32Ret;
    }

    stPipelineAttr.tFbInfo.PoolId = AX_INVALID_POOLID;
    stPipelineAttr.nOutChnNum = 3;

    for (nGrp = 0; nGrp < SAMPLE_IVPS_GROUP_NUM; nGrp++)
    {
        stGrpAttr.ePipeline = AX_IVPS_PIPELINE_DEFAULT;
        s32Ret = AX_IVPS_CreateGrp(nGrp, &stGrpAttr);
        if (0 != s32Ret)
        {
            COMM_ISP_PRT("AX_IVPS_CreateGrp failed,nGrp %d,s32Ret:0x%x\n", nGrp,
                           s32Ret);
            return s32Ret;
        }

        for (nChn = 0; nChn < SAMPLE_IVPS_CHN_NUM; nChn++)
        {
            if (nGrp == 0)
            {
                stPipelineAttr.tFilter[nChn + 1][0].bEnable = AX_TRUE;
                stPipelineAttr.tFilter[nChn + 1][0].tFRC.nSrcFrameRate = 30;
                stPipelineAttr.tFilter[nChn + 1][0].tFRC.nDstFrameRate = 30;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicOffsetX0 = 0;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicOffsetY0 = 0;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicWidth = 2688;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicHeight = 1520;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicStride = stPipelineAttr.tFilter[nChn + 1][0].nDstPicWidth;
                stPipelineAttr.tFilter[nChn + 1][0].nDstFrameWidth = 2688;
                stPipelineAttr.tFilter[nChn + 1][0].nDstFrameHeight = 1520;
                stPipelineAttr.tFilter[nChn + 1][0].eDstPicFormat = AX_YUV420_SEMIPLANAR;
                stPipelineAttr.tFilter[nChn + 1][0].eEngine = AX_IVPS_ENGINE_GDC;
                stPipelineAttr.tFilter[nChn + 1][0].tTdpCfg.eRotation = AX_IVPS_ROTATION_0;
                stPipelineAttr.nOutFifoDepth[nChn] = 1;
            }
            else if (nGrp == 1)
            {
                stPipelineAttr.tFilter[nChn + 1][0].bEnable = AX_TRUE;
                stPipelineAttr.tFilter[nChn + 1][0].tFRC.nSrcFrameRate = 30;
                stPipelineAttr.tFilter[nChn + 1][0].tFRC.nDstFrameRate = 30;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicOffsetX0 = 0;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicOffsetY0 = 0;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicWidth = 1920;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicHeight = 1080;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicStride = stPipelineAttr.tFilter[nChn + 1][0].nDstPicWidth;
                stPipelineAttr.tFilter[nChn + 1][0].nDstFrameWidth = 1920;
                stPipelineAttr.tFilter[nChn + 1][0].nDstFrameHeight = 1080;
                stPipelineAttr.tFilter[nChn + 1][0].eDstPicFormat = AX_YUV420_SEMIPLANAR;
                stPipelineAttr.tFilter[nChn + 1][0].eEngine = AX_IVPS_ENGINE_TDP;
                stPipelineAttr.tFilter[nChn + 1][0].tTdpCfg.eRotation = AX_IVPS_ROTATION_0;
                stPipelineAttr.nOutFifoDepth[nChn] = 1;
            }
            else
            {
                stPipelineAttr.tFilter[nChn + 1][0].bEnable = AX_TRUE;
                stPipelineAttr.tFilter[nChn + 1][0].tFRC.nSrcFrameRate = 30;
                stPipelineAttr.tFilter[nChn + 1][0].tFRC.nDstFrameRate = 30;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicOffsetX0 = 0;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicOffsetY0 = 0;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicWidth = 720;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicHeight = 576;
                stPipelineAttr.tFilter[nChn + 1][0].nDstPicStride = stPipelineAttr.tFilter[nChn + 1][0].nDstPicWidth;
                stPipelineAttr.tFilter[nChn + 1][0].nDstFrameWidth = 720;
                stPipelineAttr.tFilter[nChn + 1][0].nDstFrameHeight = 576;
                stPipelineAttr.tFilter[nChn + 1][0].eDstPicFormat = AX_YUV420_SEMIPLANAR;
                stPipelineAttr.tFilter[nChn + 1][0].eEngine = AX_IVPS_ENGINE_TDP;
                stPipelineAttr.tFilter[nChn + 1][0].tTdpCfg.eRotation = AX_IVPS_ROTATION_0;
            }

            s32Ret = AX_IVPS_SetPipelineAttr(nGrp, &stPipelineAttr);
            if (0 != s32Ret)
            {
                COMM_ISP_PRT("AX_IVPS_SetPipelineAttr failed,nGrp %d,s32Ret:0x%x\n",
                               nGrp, s32Ret);
                return s32Ret;
            }

            s32Ret = AX_IVPS_EnableChn(nGrp, nChn);
            if (0 != s32Ret)
            {
                COMM_ISP_PRT("AX_IVPS_EnableChn failed,nGrp %d,nChn %d,s32Ret:0x%x\n",
                               nGrp, nChn, s32Ret);
                return s32Ret;
            }
        }

        s32Ret = AX_IVPS_StartGrp(nGrp);
        if (0 != s32Ret)
        {
            COMM_ISP_PRT("AX_IVPS_StartGrp failed,nGrp %d,s32Ret:0x%x\n", nGrp,
                           s32Ret);
            return s32Ret;
        }
    }
    return 0;
}

static AX_S32 SampleIvpsDeInit()
{
    AX_S32 s32Ret, nGrp, nChn;

    for (nGrp = 0; nGrp < SAMPLE_IVPS_GROUP_NUM; nGrp++)
    {
        s32Ret = AX_IVPS_StopGrp(nGrp);
        if (0 != s32Ret)
        {
            COMM_ISP_PRT("AX_IVPS_StopGrp failed,nGrp %d,s32Ret:0x%x\n", nGrp,
                           s32Ret);
            return s32Ret;
        }

        for (nChn = 0; nChn < SAMPLE_IVPS_CHN_NUM; nChn++)
        {
            s32Ret = AX_IVPS_DisableChn(nGrp, nChn);
            if (0 != s32Ret)
            {
                COMM_ISP_PRT(
                    "AX_IVPS_DisableChn failed,nGrp %d,nChn %d,s32Ret:0x%x\n", nGrp,
                    nChn, s32Ret);
                return s32Ret;
            }
        }

        s32Ret = AX_IVPS_DestoryGrp(nGrp);
        if (0 != s32Ret)
        {
            COMM_ISP_PRT("AX_IVPS_DestoryGrp failed,nGrp %d,s32Ret:0x%x", nGrp,
                           s32Ret);
            return s32Ret;
        }
    }

    s32Ret = AX_IVPS_Deinit();
    if (0 != s32Ret)
    {
        COMM_ISP_PRT("AX_IVPS_Deinit failed,s32Ret:0x%x\n", s32Ret);
        return s32Ret;
    }

    return 0;
}

static AX_S32 SampleLinkInit()
{
    AX_S32 i;

    /*
      VIN --> IVPS
  (ModId   GrpId   ChnId) | (ModId   GrpId   ChnId)
  --------------------------------------------------
  (VIN        0       2) -> (IVPS     2       0)
  (VIN        0       1) -> (IVPS     1       0)
  (VIN        0       0) -> (IVPS     0       0)
  (IVPS       2       0)
  (IVPS       1       0)
  (IVPS       0       0)
  */

    for (i = 0; i < 3; i++)
    {
        AX_MOD_INFO_S srcMod, dstMod;
        srcMod.enModId = AX_ID_VIN;
        srcMod.s32GrpId = 0;
        srcMod.s32ChnId = i;

        dstMod.enModId = AX_ID_IVPS;
        dstMod.s32GrpId = i;
        dstMod.s32ChnId = 0;
        AX_SYS_Link(&srcMod, &dstMod);
    }

    return 0;
}

static AX_S32 SampleLinkDeInit()
{
    AX_S32 i;

    /*
      VIN --> IVPS
  (ModId   GrpId   ChnId) | (ModId   GrpId   ChnId)
  --------------------------------------------------
  (VIN        0       2) -> (IVPS     2       0)
  (VIN        0       1) -> (IVPS     1       0)
  (VIN        0       0) -> (IVPS     0       0)
  (IVPS       2       0)
  (IVPS       1       0)
  (IVPS       0       0)
  */

    for (i = 0; i < 3; i++)
    {
        AX_MOD_INFO_S srcMod, dstMod;
        srcMod.enModId = AX_ID_VIN;
        srcMod.s32GrpId = 0;
        srcMod.s32ChnId = i;

        dstMod.enModId = AX_ID_IVPS;
        dstMod.s32GrpId = i;
        dstMod.s32ChnId = 0;
        AX_SYS_UnLink(&srcMod, &dstMod);
    }

    return 0;
}

int vi_init_capture(struct libmaix_cam *cam)
{
    struct libmaix_cam_priv_t *priv = (libmaix_cam_priv_t*)cam->reserved;

    if (priv->vi_dev == 0) {
        SampleIvpsInit();

        // COMM_ISP_PRT("priv->vi_h %d priv->vi_w %d\r\n, ", priv->vi_h, priv->vi_w);

        priv->nv12_resize_helper = new ax_crop_resize_nv12();
        priv->nv12_resize_helper->init(
            1080,
            1920,
            priv->vi_h,
            priv->vi_w,
            AX_NPU_MODEL_TYPE_1_1_1);

        priv->ivps_out_data = malloc(1080 * 1920 * 3 / 2);

        // memset(&priv->nv12_resize_input_frame, 0, sizeof(priv->nv12_resize_input_frame));
        // priv->nv12_resize_input_frame.u32PicStride[0] = priv->vi_w; // 16 align
        // priv->nv12_resize_input_frame.u32Width = priv->vi_w;
        // priv->nv12_resize_input_frame.u32Height = priv->vi_h;
        // priv->nv12_resize_input_frame.enImgFormat = AX_YUV420_SEMIPLANAR;
        // priv->nv12_resize_input_frame_blkid = AX_POOL_GetBlock(AX_INVALID_POOLID, priv->vi_h * priv->vi_w * 3 / 2, nullptr);
        // priv->nv12_resize_input_frame.u64PhyAddr[0] = AX_POOL_Handle2PhysAddr(priv->nv12_resize_input_frame_blkid);
        // if (!priv->nv12_resize_input_frame.u64PhyAddr[0])
        // {
        //     COMM_ISP_PRT("AX_POOL_Handle2PhysAddr fail!\n");
        //     return -1;
        // }
        // priv->nv12_resize_input_frame.u64PhyAddr[1] = priv->nv12_resize_input_frame.u64PhyAddr[0] + priv->vi_w * priv->vi_w;
        // priv->nv12_resize_input_frame.u64VirAddr[0] = (AX_U64)AX_POOL_GetBlockVirAddr(priv->nv12_resize_input_frame_blkid);

        // if (!priv->nv12_resize_input_frame.u64VirAddr[0])
        // {
        //     COMM_ISP_PRT("AX_POOL_Handle2PhysAddr fail!\n");
        //     return -1;
        // }

        // priv->nv12_resize_input_frame.u64VirAddr[1] = (AX_ULONG)AX_SYS_Mmap(priv->nv12_resize_input_frame.u64PhyAddr[1],
        //                                                                             priv->vi_w * priv->vi_h * 1 / 2);

        // if (!priv->nv12_resize_input_frame.u64VirAddr[1])
        // {
        //     COMM_ISP_PRT("AX_SYS_Mmap fail!\n");
        //     return -1;
        // }

        // COMM_ISP_PRT("BufPoolBlockAddrGet success PhyAddr:%llx VirAddr:%llx \n", priv->nv12_resize_input_frame.u64PhyAddr[0], priv->nv12_resize_input_frame.u64VirAddr[0]);


        priv->vi_img = NULL;
        priv->inited = 1;
    }
    return 0;
}

int vi_deinit_capture(struct libmaix_cam *cam)
{
    struct libmaix_cam_priv_t *priv = (libmaix_cam_priv_t*)cam->reserved;
    if (priv->inited)
    {
        priv->inited = 0;

        SampleIvpsDeInit();

        // auto pixel_size = priv->vi_w * priv->vi_h;
        // auto ret0 = AX_SYS_Munmap((void*)priv->nv12_resize_input_frame.u64VirAddr[0], pixel_size);
        // auto ret1 = AX_SYS_Munmap((void*)priv->nv12_resize_input_frame.u64VirAddr[1], pixel_size / 2);
        // if (ret0 || ret1)
        // {
        //     COMM_ISP_PRT("AX_SYS_Munmap failed, ret0=%x , ret1 = %x \n", ret0, ret1);
        // }

        // int ret = AX_POOL_ReleaseBlock(priv->nv12_resize_input_frame_blkid);
        // if (ret)
        // {
        //     COMM_ISP_PRT("IVPS Release BlkId fail, ret=0x%x\n", ret);
        // }

        delete priv->nv12_resize_helper;
        free(priv->ivps_out_data);

    }
    return priv->inited;
}

libmaix_err_t vi_start_capture(struct libmaix_cam *cam)
{
    struct libmaix_cam_priv_t *priv = (libmaix_cam_priv_t*)cam->reserved;
    if (priv->inited)
    {
        cam->fram_size = (cam->width * cam->height * 3);
        // if (priv->vi_w < priv->vcap->capW) priv->vi_x = (priv->vcap->capW - priv->vi_w) / 2;
        // if (priv->vi_h < priv->vcap->capH) priv->vi_y = (priv->vcap->capH - priv->vi_h) / 2;

        SampleLinkInit();

        return LIBMAIX_ERR_NONE;
    }
    return LIBMAIX_ERR_NOT_READY;
}

libmaix_err_t vi_priv_capture_image(struct libmaix_cam *cam, struct libmaix_image **img)
{
    struct libmaix_cam_priv_t *priv = (libmaix_cam_priv_t*)cam->reserved;
    if (priv->vi_img == NULL) {
      priv->vi_img = libmaix_image_create(priv->vi_w, priv->vi_h, LIBMAIX_IMAGE_MODE_RGB888, LIBMAIX_IMAGE_LAYOUT_HWC, NULL, true);
      if(!priv->vi_img) return LIBMAIX_ERR_NO_MEM;
    }

    if (priv->inited) {
        AX_VIDEO_FRAME_S video_frame_s = {0};
        memset(&video_frame_s, 0, sizeof(AX_VIDEO_FRAME_S));
        AX_S32 ret = AX_IVPS_GetChnFrame(1, 0, &video_frame_s, 0);
        if (AX_SUCCESS == ret)
        {
        //    COMM_ISP_PRT("(%dx%d)%dB [%#x] PhyAddr:%llx PhyAddr_uv:%llx \n",
        //               video_frame_s.u32Height, video_frame_s.u32Width,
        //               video_frame_s.u32FrameSize, video_frame_s.enImgFormat,
        //               video_frame_s.u64PhyAddr[0], video_frame_s.u64PhyAddr[1]);

            // AX_IVPS_ASPECT_RATIO_S tAspectRatio;
            // tAspectRatio.eMode = AX_IVPS_ASPECT_RATIO_AUTO;
            // tAspectRatio.eAligns[0] = AX_IVPS_ASPECT_RATIO_HORIZONTAL_CENTER;
            // tAspectRatio.eAligns[1] = AX_IVPS_ASPECT_RATIO_VERTICAL_CENTER;
            // tAspectRatio.nBgColor = 0x000000;
            // AX_IVPS_CropResize(&video_frame_s, &priv->nv12_resize_input_frame, &tAspectRatio);
            // auto pixel_size = priv->vi_w * priv->vi_h;
            // memcpy(priv->vi_img->data, (void*)priv->nv12_resize_input_frame.u64VirAddr[0], pixel_size);
            // memcpy(priv->vi_img->data + pixel_size, (void*)priv->nv12_resize_input_frame.u64VirAddr[1], pixel_size / 2);

            int pixel_size = (int)video_frame_s.u32Width * (int)video_frame_s.u32Height;
            video_frame_s.u64VirAddr[0] = (AX_ULONG)AX_SYS_Mmap(video_frame_s.u64PhyAddr[0], pixel_size);
            video_frame_s.u64VirAddr[1] = (AX_ULONG)AX_SYS_Mmap(video_frame_s.u64PhyAddr[1], pixel_size / 2);
            memcpy(priv->ivps_out_data, (void*)video_frame_s.u64VirAddr[0], pixel_size);
            memcpy((void*)((char*)priv->ivps_out_data + pixel_size), (void*)video_frame_s.u64VirAddr[1],
                   pixel_size / 2);
{
            // cv::Mat input(video_frame_s.u32Height * 3 / 2, video_frame_s.u32Width,
            //                 CV_8UC1, priv->ivps_out_data);
            // cv::Mat rgb(video_frame_s.u32Height, video_frame_s.u32Width, CV_8UC3);
            // cv::cvtColor(input, rgb, cv::COLOR_YUV2RGB_NV12);
            // cv::imwrite("src.jpg", rgb);
}
            priv->nv12_resize_helper->run_crop_resize_nv12(priv->ivps_out_data);
            auto ret0 = AX_SYS_Munmap((void*)video_frame_s.u64VirAddr[0], pixel_size);
            auto ret1 = AX_SYS_Munmap((void*)video_frame_s.u64VirAddr[1], pixel_size / 2);

            AX_IVPS_ReleaseChnFrame(1, 0, &video_frame_s);

            if (ret0 || ret1)
            {
                COMM_ISP_PRT("AX_SYS_Munmap failed, ret0=%x , ret1 = %x \n", ret0, ret1);
            }

            cv::Mat input(priv->vi_h * 3 / 2, priv->vi_w, CV_8UC1, priv->nv12_resize_helper->m_output_image->pVir);
            cv::Mat rgb(priv->vi_h, priv->vi_w, CV_8UC3, priv->vi_img->data);
            cv::cvtColor(input, rgb, cv::COLOR_YUV2RGB_NV12);
{
            // cv::imwrite("dst.jpg", rgb);
}

            *img = priv->vi_img;
            return LIBMAIX_ERR_NONE;
        }
        else
        {
            // COMM_ISP_PRT("AX_IVPS_GetChnFrame fail 0x%X\r\n", ret);
            AX_IVPS_ReleaseChnFrame(1, 0, &video_frame_s);
            usleep(40 * 1000);
            return LIBMAIX_ERR_NOT_READY;
        }
    }
    return LIBMAIX_ERR_NOT_READY;
}

libmaix_err_t vi_priv_capture(struct libmaix_cam *cam, unsigned char *buf)
{
    struct libmaix_cam_priv_t *priv = (libmaix_cam_priv_t*)cam->reserved;
    if (priv->inited) {
        ;
    }
    return LIBMAIX_ERR_NOT_READY;
}

int cam_priv_init(struct libmaix_cam *cam)
{
    struct libmaix_cam_priv_t *priv = (struct libmaix_cam_priv_t*)cam->reserved;

    if(NULL == priv) {
        fprintf(stderr, "cam: priv is NULL\n");
        return -1;
    }

    // sipeed v833 0 1 to mpp, 2 3 to dvp csi1 sensor use camerademo(v4l2)
    // sipeed r329 only have v4l2

    priv->devInit = vi_init_capture;
    priv->devDeinit = vi_deinit_capture;

    cam->start_capture = vi_start_capture;
    cam->capture = vi_priv_capture;
    cam->capture_image = vi_priv_capture_image;

    return priv->devInit(cam);
}


struct libmaix_cam * libmaix_cam_create(int n, int w, int h, int m, int f)
{
    struct libmaix_cam *cam = (struct libmaix_cam*)malloc(sizeof(struct libmaix_cam));
    if(NULL == cam) {
        return NULL;
    }

    cam->width = w;
    cam->height = h;

    struct libmaix_cam_priv_t *priv = (struct libmaix_cam_priv_t *)malloc(sizeof(struct libmaix_cam_priv_t));
    if(NULL == priv) {
        free(cam);
        return NULL;
    }

    memset(priv, 0, sizeof(struct libmaix_cam_priv_t));
    cam->reserved = (void*)priv;

    priv->vi_dev = n;
    priv->vi_x = 0;
    priv->vi_y = 0;
    priv->vi_w = w;
    priv->vi_h = h;
    priv->vi_m = m;
    priv->vi_f = f;
    priv->inited = 0;

    if(cam_priv_init(cam) != 0) {
        libmaix_cam_destroy(&cam);
        return NULL;
    }

    return cam;
}

void libmaix_cam_destroy(struct libmaix_cam **cam)
{
    if(NULL == cam || NULL == *cam)
        return;

    struct libmaix_cam_priv_t *priv = (struct libmaix_cam_priv_t*)(*cam)->reserved;

    if(priv) {
        if(priv->vi_img != NULL) {
            libmaix_image_destroy(&priv->vi_img);
        }
        if(priv->devDeinit) {
            priv->devDeinit(*cam);
        }
        free(priv);
    }

    free(*cam);
    *cam = NULL;
}

int libmaix_cam_unit_test(char * tmp)
{
  std::cout << time(NULL) << std::endl;
  cv::Mat frame;
  std::cout << time(NULL) << std::endl;
  cv::VideoCapture cap;
  std::cout << time(NULL) << std::endl;
  cap.open(0);
  std::cout << time(NULL) << std::endl;
  if (frame.empty())
    std::cout << "frame.empty()" << std::endl; // Ran out of film
  std::cout << time(NULL) << std::endl;

  while (1) {
    cap >> frame;
    if (!frame.empty()) {
      std::cout << time(NULL) << std::endl;
    }
    // fps 25
  }

  return 0;
}

// void rgb888_to_rgb565(uint8_t * rgb888, uint16_t width, uint16_t height, uint16_t * rgb565)
// {
//     cv::Mat tmp(width, height, CV_8UC3, (void*)rgb888);
//     // cv::imwrite("tmp.jpg", tmp);
//     cv::Mat img;
//     cv::cvtColor(tmp, img, cv::COLOR_RGB2BGR565);
//     memcpy(rgb565, img.data, width * height * 2);
// }

}
