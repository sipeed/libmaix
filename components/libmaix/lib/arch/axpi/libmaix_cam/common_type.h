/**********************************************************************************
 *
 * Copyright (c) 2019-2020 Beijing AXera Technology Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Beijing AXera Technology Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Beijing AXera Technology Co., Ltd.
 *
 **********************************************************************************/

#ifndef __COMMON_TYPE_H__
#define __COMMON_TYPE_H__

#define COMM_PRT(fmt...)   \
do {\
    printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
    printf(fmt);\
}while(0)


#endif //__COMMON_TYPE_H__