#include "ax_isp_api.h"
#include "common_sys.h"
#include "common_cam.h"
#include "common_type.h"
#include "ax_buffer_tool.h"
#include "ax_global_type.h"

AX_U32 COMMON_ISP_AddToPlan(AX_POOL_FLOORPLAN_T *pPoolFloorPlan, AX_U32 nCfgCnt, AX_POOL_CONFIG_T *pPoolConfig)
{
    AX_U32 i, done = 0;
    AX_POOL_CONFIG_T *pPC;

    for (i = 0; i < nCfgCnt; i++) {
        pPC = &pPoolFloorPlan->CommPool[i];
        if (pPC->BlkSize == pPoolConfig->BlkSize) {
            pPC->BlkCnt += pPoolConfig->BlkCnt;
            done = 1;
        }
    }

    if (!done) {
        pPoolFloorPlan->CommPool[i] = *pPoolConfig;
        nCfgCnt += 1;
    }

    return nCfgCnt;
}

AX_S32 COMMON_ISP_CalcPool(COMMON_SYS_POOL_CFG_T *pPoolCfg, AX_U32 nCommPoolCnt, AX_POOL_FLOORPLAN_T *pPoolFloorPlan)
{
    AX_S32 i, nCfgCnt = 0;
    AX_POOL_CONFIG_T tPoolConfig = {
        .MetaSize  = 4 * 1024,
        .CacheMode = POOL_CACHE_MODE_NONCACHE,
        .PartitionName = "anonymous"
    };

    for (i = 0; i < nCommPoolCnt; i++) {
        tPoolConfig.BlkSize = AX_VIN_GetImgBufferSize(pPoolCfg->nHeight, pPoolCfg->nWidthStride, pPoolCfg->nFmt, AX_TRUE);
        tPoolConfig.BlkCnt  = pPoolCfg->nBlkCnt;
        nCfgCnt = COMMON_ISP_AddToPlan(pPoolFloorPlan, nCfgCnt, &tPoolConfig);
        pPoolCfg += 1;
    }

    return 0;
}

AX_S32 COMMON_SYS_Init(COMMON_SYS_ARGS_T *pCommonArgs)
{
    AX_S32 axRet = 0;
    AX_POOL_FLOORPLAN_T tPoolFloorPlan = {0};

    axRet = AX_SYS_Init();
    if (0 != axRet) {
        COMM_PRT("AX_SYS_Init failed, ret=0x%x.\n", axRet);
        return -1;
    }

    /* Release last Pool */
    axRet = AX_POOL_Exit();
    if (0 != axRet) {
        COMM_PRT("AX_POOL_Exit fail!!Error Code:0x%X\n", axRet);
    }

    /* Calc Pool BlkSize/BlkCnt */
    axRet = COMMON_ISP_CalcPool(pCommonArgs->pPoolCfg, pCommonArgs->nPoolCfgCnt, &tPoolFloorPlan);
    if (0 != axRet) {
        COMM_PRT("COMMON_ISP_CalcPool failed, ret=0x%x.\n", axRet);
        return -1;
    }

    axRet = AX_POOL_SetConfig(&tPoolFloorPlan);
    if (0 != axRet) {
        COMM_PRT("AX_POOL_SetConfig fail!Error Code:0x%X\n", axRet);
        return -1;
    } else {
        printf("AX_POOL_SetConfig success!\n");
    }

    axRet = AX_POOL_Init();
    if (0 != axRet) {
        COMM_PRT("AX_POOL_Init fail!!Error Code:0x%X\n", axRet);
        return -1;
    } else {
        COMM_PRT("AX_POOL_Init success!\n");
    }

    return 0;
}

AX_S32 COMMON_SYS_DeInit()
{
    AX_POOL_Exit();
    AX_SYS_Deinit();
    return 0;
}
