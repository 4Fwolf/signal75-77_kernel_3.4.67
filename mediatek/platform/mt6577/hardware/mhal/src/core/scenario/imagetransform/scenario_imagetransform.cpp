/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "MT6573MDPDrv.h"
#include "isp_drv.h"
#include "mt6573_sysram.h"
#include "scenario_imagetransform.h"
#include "MediaHal.h"
#include <cutils/log.h>
#include <cutils/pmem.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define MT6573IMGTRANSDEBUG

#ifdef MT6573IMGTRANSDEBUG
#undef LOG_TAG
#define LOG_TAG "ImgTrans"
#define PLOG(...) \
        do { \
            LOGE(__VA_ARGS__); \
        } while (0)

#define PMSG(...) \
        do { \
            LOGI(__VA_ARGS__); \
        } while (0)

#define INVALID_ASSERT(x) while(x){;}
#else
#define INVALID_ASSERT(x)
#define PLOG(...) 
#define PMSG(...)
#endif

//choose a path
// 1.(RDMA0->MOUT)-> (PRZ0->MOUT1)                        ->ROTDMA0
//                                or (CRZ->IPP->(OVL)->MOUT0)    or ROTDMA1
//                                                                                    or ROTDMA2
// 2. RDMA1->PRZ1->ROTDMA3
static int RDMA0_Path(MT6573MDPDrv * pMDPObj , MT6573MDPDrv::RDMA_PARAM * a_pRDMA_Param , MT6573MDPDrv::RESZ_PARAM * a_pRESZ_Param , MT6573MDPDrv::ROTDMA_PARAM * a_pROTDMA_Param, unsigned long a_u4Tbl)
{
    MT6573MDPDrv::RDMA0_PARAM stRDMA0Param;
    MT6573MDPDrv::PRZ0_PARAM stPRZ0Param;
    MT6573MDPDrv::CRZ_IPP_PARAM stCRZParam;
    MT6573MDPDrv::IPP_PARAM stIPPParam;
    MT6573MDPDrv::ROTDMA0_PARAM stROTDMA0Param;
    MT6573MDPDrv::ROTDMA1_PARAM stROTDMA1Param;
    MT6573MDPDrv::ROTDMA2_PARAM stROTDMA2Param;

    MT6573MDPDrv::SYSRAM_INFO stRDMA0Sysram;
    MT6573MDPDrv::SYSRAM_INFO stROTDMASysram;
    stSysramParam stRDMA0SysramParam;
    stSysramParam stROTDMASysramParam;

    MT6573MDPDrv::ErrMsg retMsg = MT6573MDPDrv::MSG_OK;
    int retVal = MHAL_NO_ERROR;
    int fd = -1;

    fd = open("/dev/mt6573-SYSRAM", O_RDWR);
    if(-1 == fd)
    {
        PLOG("Open sysram driver failed");
        return MHAL_INVALID_DRIVER;
    }

    stROTDMASysramParam.u4Size = 0;
    stRDMA0SysramParam.u4Size = 0;

    memset(&stCRZParam , 0 , sizeof(MT6573MDPDrv::CRZ_IPP_PARAM));

//Config

    stRDMA0Param.pRDMA0_param = a_pRDMA_Param;
    stRDMA0Param.bToCAM = (CRZ_IPP_OVL_FLAG & a_u4Tbl ? 1 : 0);
    stRDMA0Param.bToPRZ0 = (PRZ0_FLAG & a_u4Tbl ? 1 : 0);
    stRDMA0Param.bToVRZ = 0;
    stRDMA0Param.bToJPGDMA = 0;
    retMsg = pMDPObj->Config_RDMA0(&stRDMA0Param , &stRDMA0Sysram);
    if(MT6573MDPDrv::MSG_OK != retMsg)
    {
        PLOG("Config RDMA0 failed : %d" , retMsg);
        retVal = MHAL_INVALID_PARA;
        goto Disable;
    }

    if(CRZ_IPP_OVL_FLAG & a_u4Tbl)
    {
        stIPPParam.bEnColorAdj = 0;
        stIPPParam.bEnRGBReplace = 0;
        stIPPParam.bEnColorInverse = 0;
        stIPPParam.bEnColorize = 0;
        stIPPParam.bEnSatAdj = 0;
        stIPPParam.bEnHueAdj = 0;
        stIPPParam.bEnContractBrightAdj = 0;
        stCRZParam.pCRZ_Param = a_pRESZ_Param;
        stCRZParam.pIPP_Param = &stIPPParam;
        stCRZParam.pOVL_Param = NULL;
        stCRZParam.uSource = 2;
        if(ROTDMA0_FLAG & a_u4Tbl)
        {
            stCRZParam.bToROTDMA = 1;
        }
        else if(ROTDMA1_FLAG & a_u4Tbl)
        {
            stCRZParam.bToROTDMA = 2;
        }
        else
        {
            stCRZParam.bToROTDMA = 3;
        }
        stCRZParam.bToPRZ0 = 0;
        stCRZParam.bToVRZ = 0;
        stCRZParam.bToJPGDMA = 0;
        stCRZParam.bAddOverlayToROTDMA = 0;
        stCRZParam.bAddOverlayToPRZ0 = 0;
        stCRZParam.bAddOverlayToVRZ = 0;
        stCRZParam.bAddOverlayToJPGDMA = 0;
        retMsg = pMDPObj->Config_CRZ_IPP(&stCRZParam);
        if(MT6573MDPDrv::MSG_OK != retMsg)
        {
            PLOG("Config CRZIPP failed : %d" , retMsg);
            retVal = MHAL_INVALID_PARA;
            goto Disable;
        }
    }

    if(PRZ0_FLAG & a_u4Tbl)
    {
        stPRZ0Param.pPRZ0_Param = a_pRESZ_Param;
        stPRZ0Param.bCamIn = 0;
        stPRZ0Param.bToROTDMA = 1;
        stPRZ0Param.bToVRZ = 0;
        stPRZ0Param.bSource = 2;
        retMsg = pMDPObj->Config_PRZ0(&stPRZ0Param);
        if(MT6573MDPDrv::MSG_OK != retMsg)
        {
            PLOG("Config PRZ0 failed : %d" , retMsg);
            retVal = MHAL_INVALID_PARA;
            goto Disable;
        }
    }

    if(ROTDMA0_FLAG & a_u4Tbl)
    {
        stROTDMA0Param.pROTDMA0_Param = a_pROTDMA_Param;
        stROTDMA0Param.u4InputSource = (CRZ_IPP_OVL_FLAG & a_u4Tbl ? 0 : 1);
        retMsg = pMDPObj->Config_ROTDMA0(&stROTDMA0Param , &stROTDMASysram);
        if(MT6573MDPDrv::MSG_OK != retMsg)
        {
            PLOG("Config ROTDMA0 failed : %d" , retMsg);
            retVal = MHAL_INVALID_PARA;
            goto Disable;
        }
    }

    if(ROTDMA1_FLAG & a_u4Tbl)
    {
        stROTDMA1Param.pROTDMA1_Param = a_pROTDMA_Param;
        stROTDMA1Param.u4InputSource = (CRZ_IPP_OVL_FLAG & a_u4Tbl ? 0 : 1);
        retMsg = pMDPObj->Config_ROTDMA1(&stROTDMA1Param , &stROTDMASysram);
        if(MT6573MDPDrv::MSG_OK != retMsg)
        {
            PLOG("Config ROTDMA1 failed : %d" , retMsg);
            retVal = MHAL_INVALID_PARA;
            goto Disable;
        }
    }

    if(ROTDMA2_FLAG & a_u4Tbl)
    {
        stROTDMA2Param.pROTDMA2_Param = a_pROTDMA_Param;
        stROTDMA2Param.u4InputSource = (CRZ_IPP_OVL_FLAG & a_u4Tbl ? 0 : 1);
        retMsg = pMDPObj->Config_ROTDMA2(&stROTDMA2Param , &stROTDMASysram);
        if(MT6573MDPDrv::MSG_OK != retMsg)
        {
            PLOG("Config ROTDMA2 failed : %d" , retMsg);
            retVal = MHAL_INVALID_PARA;
            goto Disable;
        }
    }

//TODO : Optimize sysram usage
//Allocate sysram

    stROTDMASysramParam.u4Alignment = stROTDMASysram.u4Alignment;
    stROTDMASysramParam.u4Size = (stROTDMASysram.u4RecommandBuffCnt*stROTDMASysram.u4SingleBuffSize);
    if(ROTDMA0_FLAG & a_u4Tbl)
    {
        stROTDMASysramParam.u4Owner = MT6573SYSRAMUSR_ROTDMA0;
    }
    else if(ROTDMA1_FLAG & a_u4Tbl)
    {
        stROTDMASysramParam.u4Owner = MT6573SYSRAMUSR_ROTDMA1;
    }
    else
    {
        stROTDMASysramParam.u4Owner = MT6573SYSRAMUSR_ROTDMA2;
    }

    if(0 < stROTDMASysramParam.u4Size)
    {
        if(ioctl(fd, MT6573MDP_X_SYSRAMALLOC , &stROTDMASysramParam))
        {
            PLOG("Allocate sysram for ROTDMA failed");
            stROTDMASysramParam.u4Size = 0;
            retVal = MHAL_INVALID_MEMORY;
            goto Disable;
        }
    }

    stRDMA0SysramParam.u4Alignment = stRDMA0Sysram.u4Alignment;
    stRDMA0SysramParam.u4Size = (stRDMA0Sysram.u4RecommandBuffCnt*stRDMA0Sysram.u4SingleBuffSize);
    stRDMA0SysramParam.u4Owner = MT6573SYSRAMUSR_RDMA0;
    if(0 < stRDMA0SysramParam.u4Size)
    {
        if(ioctl(fd, MT6573MDP_X_SYSRAMALLOC , &stRDMA0SysramParam))
        {
            stRDMA0SysramParam.u4Size = 0;
            PLOG("Allocate sysram for RDMA0 failed");
            if(0 < stROTDMASysramParam.u4Size)
            {
                ioctl(fd, MT6573MDP_S_SYSRAMFREE , &stROTDMASysramParam);
                stROTDMASysramParam.u4Size = 0;
            }
            retVal = MHAL_INVALID_MEMORY;
            goto Disable;
        }
    }

//Enable
    if(ROTDMA0_FLAG & a_u4Tbl)
    {
        retMsg = pMDPObj->Enable_ROTDMA0(stROTDMASysramParam.u4Addr , stROTDMASysram.u4RecommandBuffCnt);
        if(MT6573MDPDrv::MSG_OK != retMsg)
        {
            PLOG("Enable ROTDMA0 failed : %d" , retMsg);
            retVal = MHAL_INVALID_CTRL_CODE;
            goto Disable;
        }
    }

    if(ROTDMA1_FLAG & a_u4Tbl)
    {
        retMsg = pMDPObj->Enable_ROTDMA1(stROTDMASysramParam.u4Addr , stROTDMASysram.u4RecommandBuffCnt);
        if(MT6573MDPDrv::MSG_OK != retMsg)
        {
            PLOG("Enable ROTDMA1 failed : %d" , retMsg);
            retVal = MHAL_INVALID_CTRL_CODE;
            goto Disable;
        }
    }

    if(ROTDMA2_FLAG & a_u4Tbl)
    {
        retMsg = pMDPObj->Enable_ROTDMA2(stROTDMASysramParam.u4Addr , stROTDMASysram.u4RecommandBuffCnt);
        if(MT6573MDPDrv::MSG_OK != retMsg)
        {
            PLOG("Enable ROTDMA2 failed : %d" , retMsg);
            retVal = MHAL_INVALID_CTRL_CODE;
            goto Disable;
        }
    }

    if(CRZ_IPP_OVL_FLAG & a_u4Tbl)
    {
        retMsg = pMDPObj->Enable_CRZ_IPP();
        if(MT6573MDPDrv::MSG_OK != retMsg)
        {
            PLOG("Enable CRZIPP failed : %d" , retMsg);
            retVal = MHAL_INVALID_CTRL_CODE;
            goto Disable;
        }
    }

    if(PRZ0_FLAG & a_u4Tbl)
    {
        retMsg = pMDPObj->Enable_PRZ0();
        if(MT6573MDPDrv::MSG_OK != retMsg)
        {
            PLOG("Enable PRZ0 failed : %d" , retMsg);
            retVal = MHAL_INVALID_CTRL_CODE;
            goto Disable;
        }
    }

    retMsg = pMDPObj->Enable_RDMA0(stRDMA0SysramParam.u4Addr);
    if(MT6573MDPDrv::MSG_OK != retMsg)
    {
        PLOG("Enable RDMA0 failed : %d" , retMsg);
        retVal = MHAL_INVALID_CTRL_CODE;
        goto Disable;
    }

//Wait Done
    retMsg = pMDPObj->Wait_Done(((ROTDMA0_FLAG | ROTDMA1_FLAG | ROTDMA2_FLAG) & a_u4Tbl));
    if(MT6573MDPDrv::MSG_OK != retMsg)
    {
        PLOG("Wait done failed : %d" , retMsg);
        retVal = MHAL_INVALID_CTRL_CODE;
        goto Disable;
    }

//Disable
Disable:
    pMDPObj->Disable_RDMA0();

    if(CRZ_IPP_OVL_FLAG & a_u4Tbl)
    {
        pMDPObj->Disable_CRZ_IPP();
    }

    if(PRZ0_FLAG & a_u4Tbl)
    {
        pMDPObj->Disable_PRZ0();
    }

    if(ROTDMA0_FLAG & a_u4Tbl)
    {
        pMDPObj->Disable_ROTDMA0();
    }

    if(ROTDMA1_FLAG & a_u4Tbl)
    {
        pMDPObj->Disable_ROTDMA1();
    }

    if(ROTDMA2_FLAG & a_u4Tbl)
    {
        pMDPObj->Disable_ROTDMA2();
    }

//Free sysram
    if(0 < stRDMA0SysramParam.u4Size)
    {
        ioctl(fd, MT6573MDP_S_SYSRAMFREE , &stRDMA0SysramParam);
    }
    if(0 < stROTDMASysramParam.u4Size)
    {    
        ioctl(fd, MT6573MDP_S_SYSRAMFREE , &stROTDMASysramParam);
    }
    close(fd);

    return retVal;
}

//#define MT6573_PROFILEIMGTRANS_L1
static int RDMA1_Path(MT6573MDPDrv * pMDPObj , MT6573MDPDrv::RDMA_PARAM * a_pRDMA_Param , MT6573MDPDrv::RESZ_PARAM * a_pRESZ_Param , MT6573MDPDrv::ROTDMA_PARAM * a_pROTDMA_Param)
{
    MT6573MDPDrv::RDMA1_PRZ1_ROTDMA3_PARAM stParam;
    MT6573MDPDrv::SYSRAM_INFO stRDMA1Sysram;
    MT6573MDPDrv::SYSRAM_INFO stROTDMA3Sysram;
    stSysramParam stRDMA1SysramParam;
    stSysramParam stROTDMA3SysramParam;

    MT6573MDPDrv::ErrMsg retMsg = MT6573MDPDrv::MSG_OK;
    int retVal = MHAL_NO_ERROR;
    int fd = -1;

#ifdef MT6573_PROFILEIMGTRANS_L1
struct timeval t1 , t2;
gettimeofday(&t1,NULL);
#endif

    fd = open("/dev/mt6573-SYSRAM", O_RDWR);
    if(-1 == fd)
    {
        PLOG("Open sysram driver failed");
        return MHAL_INVALID_DRIVER;
    }

#ifdef MT6573_PROFILEIMGTRANS_L1
gettimeofday(&t2,NULL);
PMSG("Open sysram:%u",(t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
gettimeofday(&t1,NULL);
#endif

    stROTDMA3SysramParam.u4Size = 0;
    stRDMA1SysramParam.u4Size = 0;

//Config
    stParam.pRDMA1_param = a_pRDMA_Param;
    stParam.pPRZ1_Param = a_pRESZ_Param;
    stParam.pROTDMA3_Param = a_pROTDMA_Param;
    stParam.u4OutPut = 0;

    retMsg = pMDPObj->Config_RDMA1_PRZ1_ROTDMA3(&stParam , &stROTDMA3Sysram , &stRDMA1Sysram);
    if(MT6573MDPDrv::MSG_OK != retMsg)
    {
        PLOG("Config RDMA1 path failed");
        retVal = MHAL_INVALID_CTRL_CODE;
        goto Disable;
    }

#ifdef MT6573_PROFILEIMGTRANS_L1
gettimeofday(&t2,NULL);
PMSG("Config:%u",(t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
gettimeofday(&t1,NULL);
#endif

//Allocate sysram

//TODO : optimize buffer use
    stROTDMA3SysramParam.u4Alignment = stROTDMA3Sysram.u4Alignment;
    stROTDMA3SysramParam.u4Size = (stROTDMA3Sysram.u4RecommandBuffCnt*stROTDMA3Sysram.u4SingleBuffSize);
    stROTDMA3SysramParam.u4Owner = MT6573SYSRAMUSR_ROTDMA3;
    if(0 < stROTDMA3SysramParam.u4Size)
    {
        if(ioctl(fd, MT6573MDP_X_SYSRAMALLOC , &stROTDMA3SysramParam))
        {
            PLOG("Allocate ROTDMA3 sysram failed");
            stROTDMA3SysramParam.u4Size = 0;
            retVal = MHAL_INVALID_MEMORY;
            goto Disable;
        }
    }

//    PLOG("ROTDMA3 sysram Addr : 0x%x , Align : %lu , Owner:%d , Size : %lu" , stROTDMA3SysramParam.u4Addr , stROTDMA3SysramParam.u4Alignment , stROTDMA3SysramParam.u4Owner , stROTDMA3SysramParam.u4Size);

    stRDMA1SysramParam.u4Alignment = stRDMA1Sysram.u4Alignment;
    stRDMA1SysramParam.u4Size = (stRDMA1Sysram.u4RecommandBuffCnt*stRDMA1Sysram.u4SingleBuffSize);
    stRDMA1SysramParam.u4Owner = MT6573SYSRAMUSR_RDMA1;
    if(0 < stRDMA1SysramParam.u4Size)
    {
        if(ioctl(fd, MT6573MDP_X_SYSRAMALLOC , &stRDMA1SysramParam))
        {
            PLOG("Allocate RDMA1 sysram failed");
            stRDMA1SysramParam.u4Size = 0;
            if(0 < stROTDMA3SysramParam.u4Size)
            {
                ioctl(fd, MT6573MDP_S_SYSRAMFREE , &stROTDMA3SysramParam);
                stROTDMA3SysramParam.u4Size = 0;
            }
            close(fd);
            retVal = MHAL_INVALID_MEMORY;
            goto Disable;
        }
    }

#ifdef MT6573_PROFILEIMGTRANS_L1
gettimeofday(&t2,NULL);
PMSG("Allocate:%u",(t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
gettimeofday(&t1,NULL);
#endif

//    PLOG("RDMA1 sysram Addr : 0x%x , Align : %lu , Owner:%d , Size : %lu" , stRDMA1SysramParam.u4Addr , stRDMA1SysramParam.u4Alignment , stRDMA1SysramParam.u4Owner , stRDMA1SysramParam.u4Size);

//Enable
    retMsg = pMDPObj->Enable_RDMA1_PRZ1_ROTDMA3(stRDMA1SysramParam.u4Addr, stROTDMA3SysramParam.u4Addr , stROTDMA3Sysram.u4RecommandBuffCnt);
    if(MT6573MDPDrv::MSG_OK != retMsg)
    {
        PLOG("Enable RDMA1->PRZ1->ROTDMA3 path failed : %d" , retMsg);
        retVal = MHAL_INVALID_CTRL_CODE;
        goto Disable;
    }

#ifdef MT6573_PROFILEIMGTRANS_L1
gettimeofday(&t2,NULL);
PMSG("En:%u",(t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
gettimeofday(&t1,NULL);
#endif

//Wait Done
    retMsg = pMDPObj->Wait_Done(RDMA1_PRZ1_ROTDMA3_FLAG);
    if(MT6573MDPDrv::MSG_OK != retMsg)
    {
        PLOG("Wait RDMA1->PRZ1->ROTDMA3 path failed : %d" , retMsg);
        retVal = MHAL_INVALID_CTRL_CODE;
    }

#ifdef MT6573_PROFILEIMGTRANS_L1
gettimeofday(&t2,NULL);
PMSG("HW:%u",(t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
gettimeofday(&t1,NULL);
#endif


//Disable
Disable:
    pMDPObj->Disable_RDMA1_PRZ1_ROTDMA3();
//Free sysram
    if(0 < stRDMA1SysramParam.u4Size)
    {
        ioctl(fd, MT6573MDP_S_SYSRAMFREE , &stRDMA1SysramParam);
    }
    if(0 < stROTDMA3SysramParam.u4Size)
    {
        ioctl(fd, MT6573MDP_S_SYSRAMFREE , &stROTDMA3SysramParam);
    }
    close(fd);

#ifdef MT6573_PROFILEIMGTRANS_L1
gettimeofday(&t2,NULL);
PMSG("Disable:%u",(t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
gettimeofday(&t1,NULL);
#endif


    return retVal;
}

static inline int mHalBitblt_MDPRoute(mHalBltParam_t * bltParam , MT6573MDPDrv * MDP , unsigned long * pu4ResTbl)
{
    stLockResParam resParam;
    resParam.u4IsTimeShared = 0;
    resParam.u4TimeOutInms = 10;

#if 1
//Priority : RDMA1->PRZ1->ROTDMA3 > RDMA0->(PRZ0 > CRZ)->(ROTDMA1 > ROTDMA2 > ROTDMA0)
    //Only ROTDMA0 support planar YUV output format
    if((MHAL_FORMAT_YUV_420_SP == bltParam->dstFormat) || (MHAL_FORMAT_YUV_420 == bltParam->dstFormat) || (MHAL_FORMAT_MTK_YUV == bltParam->dstFormat))
    {
        resParam.u4LockResTable = (RDMA0_FLAG | PRZ0_FLAG | ROTDMA0_FLAG);
        if(MT6573MDPDrv::MSG_OK != MDP->LockResource(&resParam))
        {
            resParam.u4LockResTable = (RDMA0_FLAG | CRZ_IPP_OVL_FLAG | ROTDMA0_FLAG);
            if(MT6573MDPDrv::MSG_OK != MDP->LockResource(&resParam))
            {
                PMSG("Resource is busy for planar YUV output");
                return MHAL_INVALID_RESOURCE;
            }
            (*pu4ResTbl) |= (RDMA0_FLAG | CRZ_IPP_OVL_FLAG | ROTDMA0_FLAG);
        }
        else
        {
            (*pu4ResTbl) |= (RDMA0_FLAG | PRZ0_FLAG | ROTDMA0_FLAG);
        }
    }
    else
    {
        //Lock RDMA1->PRZ1->ROTDMA3 first
        resParam.u4LockResTable = RDMA1_PRZ1_ROTDMA3_FLAG;
        if(MT6573MDPDrv::MSG_OK != MDP->LockResource(&resParam))
        {
            resParam.u4LockResTable = RDMA0_FLAG;
            if(MT6573MDPDrv::MSG_OK != MDP->LockResource(&resParam))
            {
                PMSG("No RDMA resource for bitblt");
                return MHAL_INVALID_RESOURCE;
            }

            (*pu4ResTbl) |= RDMA0_FLAG;
            resParam.u4LockResTable = PRZ0_FLAG;
            if(MT6573MDPDrv::MSG_OK != MDP->LockResource(&resParam))
            {
                resParam.u4LockResTable = CRZ_IPP_OVL_FLAG;
                if(MT6573MDPDrv::MSG_OK != MDP->LockResource(&resParam))
                {
                    MDP->UnlockResource((*pu4ResTbl));
                    PMSG("No RESZ resource for bitblt");
                    return MHAL_INVALID_RESOURCE;
                }

                (*pu4ResTbl) |= CRZ_IPP_OVL_FLAG;
            }
            else
            {
                (*pu4ResTbl) |= PRZ0_FLAG;
            }

            resParam.u4LockResTable = ROTDMA1_FLAG;
            if(MT6573MDPDrv::MSG_OK != MDP->LockResource(&resParam))
            {
                resParam.u4LockResTable = ROTDMA2_FLAG;
                if(MT6573MDPDrv::MSG_OK != MDP->LockResource(&resParam))
                {
                    resParam.u4LockResTable = ROTDMA0_FLAG;
                    if(MT6573MDPDrv::MSG_OK != MDP->LockResource(&resParam))
                    {
                        MDP->UnlockResource((*pu4ResTbl));
                        PMSG("No ROTDMA resource for bitblt");
                        return MHAL_INVALID_RESOURCE;
                    }
                    (*pu4ResTbl) |= ROTDMA0_FLAG;
                }
                else
                {
                    (*pu4ResTbl) |= ROTDMA2_FLAG;
                }
            }
            else
            {
                (*pu4ResTbl) |= ROTDMA1_FLAG;
            }          

        }
        else
        {
        //RDMA1_PRZ1_ROTDMA3 is locked
            (*pu4ResTbl) |= RDMA1_PRZ1_ROTDMA3_FLAG;
        }
    }
#else
//    resParam.u4LockResTable = RDMA1_PRZ1_ROTDMA3_FLAG;
    resParam.u4LockResTable = (RDMA0_FLAG |CRZ_IPP_OVL_FLAG | ROTDMA0_FLAG);
//    resParam.u4LockResTable = (RDMA0_FLAG |PRZ0_FLAG | ROTDMA2_FLAG);

    if(MT6573MDPDrv::MSG_OK != MDP->LockResource(&resParam))
    {
        return MHAL_INVALID_RESOURCE;
    }
    (*pu4ResTbl) = resParam.u4LockResTable;
#endif

    return MHAL_NO_ERROR;
}

static int mHal_CalculateEISSetting(MUINT32 a_u4Offset , unsigned long * a_pu4Setting)
{
    //Sanity check
//    if(100 <= a_u4Offset)
    if(224 < a_u4Offset)
    {
        return -1;
    }

    if(32 > a_u4Offset)
    {
        //0~12.5
        *a_pu4Setting = 0;
    }
    else if(96 > a_u4Offset)
    {
        //12.5~37.5
        *a_pu4Setting = 1;
    }
    else if(160 > a_u4Offset)
    {
        //37.5~62.5
        *a_pu4Setting = 2;
    }
    else
    {
        //62.5~87.5
        *a_pu4Setting = 3;
    }
    	
    return 0;
}

static int mHal_ImageTransform_Exec(mHalBltParam_t *bltParam , unsigned long a_u4ResTbl , MT6573MDPDrv * MDP , unsigned long a_u4ReadCropInRDMA)
{
    MT6573MDPDrv::RDMA_PARAM stRDMA_Param;
    MT6573MDPDrv::RESZ_PARAM stRESZ_Param;
    MT6573MDPDrv::ROTDMA_PARAM stROTDMA_Param;
    unsigned long u4Offset = 0;
    unsigned long u4EISSetting;
    int ret = MHAL_NO_ERROR;

/*
PMSG("Param: srcX:%d,srcY:%d,srcW:%d,srcWStride:%d,srcH:%d,srcHStride:%d,srcAddr:%d,srcFmt:%d,dstW:%d,dstH:%d,dstAddr:%d,dstFmt:%d,Pitch:%d,Ori:%d" ,
	bltParam->srcX , bltParam->srcY , bltParam->srcW , bltParam->srcWStride , bltParam->srcH , bltParam->srcHStride , 
	bltParam->srcAddr , bltParam->srcFormat , bltParam->dstW , bltParam->dstH , bltParam->dstAddr , bltParam->dstFormat , bltParam->pitch , bltParam->orientation);
*/

    //Config
    stRDMA_Param.bContinuousHWTrigger = 0;
    stRDMA_Param.bEISEn = 0;
    stRDMA_Param.u4EISCON = 0;

    if((0 == a_u4ReadCropInRDMA) && (0 != bltParam->u4SrcOffsetXFloat) && (0 != bltParam->u4SrcOffsetYFloat))
    {
        //Calculate EIS setting
        
        if(mHal_CalculateEISSetting(bltParam->u4SrcOffsetXFloat , &u4EISSetting))
        {
     	     PLOG("Incorrect EIS offset X setting%lu!!" , (unsigned long)bltParam->u4SrcOffsetXFloat);
            return MHAL_INVALID_CTRL_CODE;
        }
        stRDMA_Param.u4EISCON = (u4EISSetting << 8);
        if(mHal_CalculateEISSetting(bltParam->u4SrcOffsetYFloat , &u4EISSetting))
        {
     	     PLOG("Incorrect EIS offset X setting%lu!!" , (unsigned long)bltParam->u4SrcOffsetXFloat);
            return MHAL_INVALID_CTRL_CODE;
        }
        stRDMA_Param.u4EISCON |= (u4EISSetting << 12);
        if(stRDMA_Param.u4EISCON)
        {
            stRDMA_Param.u4EISCON |= 0x11;
        }

        stRDMA_Param.bEISEn = 1;
    }

    switch(bltParam->srcFormat)
    {
        case MHAL_FORMAT_RGB_565 :
            stRDMA_Param.eINFMT = MT6573MDPDrv::RGB565;
            u4Offset = 0;
        break;
        case MHAL_FORMAT_RGB_888 :
            stRDMA_Param.eINFMT = MT6573MDPDrv::RGB888;
            u4Offset = 0;
        break;
        case MHAL_FORMAT_ARGB_8888 :
            stRDMA_Param.eINFMT = MT6573MDPDrv::ARGB8888;
            u4Offset = 0;
        break;
        case MHAL_FORMAT_YUV_420 :
            stRDMA_Param.eINFMT = MT6573MDPDrv::YV12_Planar;
            u4Offset = (bltParam->srcWStride*bltParam->srcHStride); 
        break;
        case MHAL_FORMAT_YUV_420_SP :
            stRDMA_Param.eINFMT = MT6573MDPDrv::NV21;
            u4Offset = (bltParam->srcWStride*bltParam->srcHStride); 
        break;
        case MHAL_FORMAT_MTK_YUV :
            stRDMA_Param.eINFMT = MT6573MDPDrv::YUV420_4x4BLK;
            u4Offset = (bltParam->srcWStride*bltParam->srcHStride);
        break;
        case MHAL_FORMAT_YUY2 :
            stRDMA_Param.eINFMT = MT6573MDPDrv::YUY2_Pack;
            u4Offset = 0;
        break;
        default :
     	     PLOG("Unsupport format 0x%x" , bltParam->srcFormat);
        break;
    }
    stRDMA_Param.u2SrcWidthInPixel = bltParam->srcWStride;
    stRDMA_Param.u2SrcHeightInLine = bltParam->srcHStride;
    stRDMA_Param.u2CropWidthInPixel = (a_u4ReadCropInRDMA ? bltParam->srcW : bltParam->srcWStride);
    stRDMA_Param.u2CropHeightInLine = (a_u4ReadCropInRDMA ? bltParam->srcH : bltParam->srcHStride);
    stRDMA_Param.u2CropXOffsetInPixel = (a_u4ReadCropInRDMA ? bltParam->srcX : 0);
    stRDMA_Param.u2CropYOffsetLine = (a_u4ReadCropInRDMA ? bltParam->srcY : 0);
    stRDMA_Param.uInBufferCnt = 1;
    stRDMA_Param.u4InputYAddr = bltParam->srcAddr;
    stRDMA_Param.u4InputYtoUoffsetInBytes = u4Offset;//TODO
    stRDMA_Param.u4InputYtoVoffsetInBytes = u4Offset + (u4Offset >> 2);//TODO
    stRDMA_Param.u4InputYtoNextYoffsetInBytes = 0;

    stRESZ_Param.bCamIn = 0;
    stRESZ_Param.bContinuous = 0;
    stRESZ_Param.u2SrcWidthInPixel = stRDMA_Param.u2CropWidthInPixel;
    stRESZ_Param.u2SrcHeightInLine = stRDMA_Param.u2CropHeightInLine;
    stRESZ_Param.u2CropOffsetXInPixel = (a_u4ReadCropInRDMA ? 0 : bltParam->srcX);
    stRESZ_Param.u2CropOffsetYInLine = (a_u4ReadCropInRDMA ? 0 : bltParam->srcY);
    stRESZ_Param.u2CropWidthInPixel = bltParam->srcW;
    stRESZ_Param.u2CropHeightInLine = bltParam->srcH;
    stRESZ_Param.u2DestWidthInPixel = (bltParam->orientation & 0x1 ? bltParam->dstH : bltParam->dstW);
    stRESZ_Param.u2DestHeightInLine = (bltParam->orientation & 0x1 ? bltParam->dstW : bltParam->dstH);
    stRESZ_Param.uDnScaleCoeff = 15;//TODO tune parameters here
    stRESZ_Param.uUpScaleCoeff = 8;//TODO tune parameters here
    stRESZ_Param.uEEHCoeff = 0;
    stRESZ_Param.uEEVCoeff = 0;

    stROTDMA_Param.bCamIn = 0;
    stROTDMA_Param.bContinuous = 0;
    stROTDMA_Param.bDithering = ((MHAL_FORMAT_RGB_565 == bltParam->dstFormat) && (MHAL_FORMAT_RGB_565 != bltParam->srcFormat) ? 1 : 0);

    stROTDMA_Param.bFlip = (MHAL_BITBLT_FLIP_H & bltParam->orientation ? 1 : 0);
    stROTDMA_Param.bRotate = (bltParam->orientation & 0x3);
    stROTDMA_Param.bSpecifyAlpha = 0;
    stROTDMA_Param.uAlpha = 255;

    switch(bltParam->dstFormat)
    {
        case MHAL_FORMAT_RGB_565 :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::RGB565;
        break;
        case MHAL_FORMAT_RGB_888 :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::RGB888;
        break;
        case MHAL_FORMAT_ARGB_8888 :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::ARGB8888;
        break;
        case MHAL_FORMAT_ABGR_8888 :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::ABGR8888;
        break;        
        case MHAL_FORMAT_YUV_420 :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::YV12_Planar;
        break;        
        case MHAL_FORMAT_YUV_420_SP :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::NV21;
        break;
        case MHAL_FORMAT_MTK_YUV :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::YUV420_4x4BLK;
        break;
        case MHAL_FORMAT_YUY2 :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::YUY2_Pack;
        break;
        case MHAL_FORMAT_UYVY :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::UYVY_Pack;
        break;
        default :
     	     PLOG("Unsupport dest format 0x%x" , bltParam->dstFormat);
        break;
    }

    stROTDMA_Param.uOutBufferCnt = 1;
    stROTDMA_Param.u4OutYBuffAddr = bltParam->dstAddr;
    stROTDMA_Param.u4OutYtoUoffsetInBytes = (bltParam->pitch*bltParam->dstH);//TODO
    stROTDMA_Param.u4OutYtoVoffsetInBytes = stROTDMA_Param.u4OutYtoUoffsetInBytes + ((bltParam->pitch*bltParam->dstH) >> 2);//TODO
    stROTDMA_Param.u4OutYtoNextYoffsetInBytes = 0;
    
    stROTDMA_Param.u2SrcROIOffsetX = 0;
    stROTDMA_Param.u2SrcROIOffsetY = 0;
    stROTDMA_Param.u2SrcWidthInPixel = stRESZ_Param.u2DestWidthInPixel;
    stROTDMA_Param.u2SrcHeightInLine = stRESZ_Param.u2DestHeightInLine;
    stROTDMA_Param.u2ROIWidthInPixel = stRESZ_Param.u2DestWidthInPixel;
    stROTDMA_Param.u2ROIHeightInLine = stRESZ_Param.u2DestHeightInLine;
    stROTDMA_Param.u2DestROIOffsetX = 0;
    stROTDMA_Param.u2DestROIOffsetY = 0;
    stROTDMA_Param.u2DestImgWidthInPixel = bltParam->pitch;
    stROTDMA_Param.u2DestImgHeightInLine = bltParam->dstH;

    //Config path -> Allocate sysram -> Wait Done -> Disable -> Free sysram
    if(a_u4ResTbl & RDMA1_PRZ1_ROTDMA3_FLAG)
    {
//        PMSG("Run RDMA1 path");
        ret = RDMA1_Path(MDP , &stRDMA_Param , &stRESZ_Param , &stROTDMA_Param);
    }
    else
    {
//        PMSG("Run RDMA0 path");
        ret = RDMA0_Path(MDP , &stRDMA_Param , &stRESZ_Param , &stROTDMA_Param , a_u4ResTbl);
    }

    return ret;
}

//#define MT6573_PROFILEIMGTRANS
int mHalBitblt(void *a_pInBuffer)
{
    int ret = MHAL_NO_ERROR;
    mHalBltParam_t *bltParam;
    mHalBltParam_t InterbltParam;
    unsigned long u4ResTbl = 0;
    unsigned long u4MultipleEntry = 0;
    unsigned long u4Residual = 0;
    unsigned char * pIntBuffAddr = NULL;
    int FD;

    bltParam = (mHalBltParam_t*)a_pInBuffer;
    MT6573MDPDrv * MDP = NULL;

//PMSG("bitblt param srcw:%u,Wstride:%u,srcH:%u,Hstride:%u,dstW:%u,dstH:%u,Pitch:%u,Orien:%u" , 
//	bltParam->srcW , bltParam->srcWStride , bltParam->srcH , bltParam->srcHStride , bltParam->dstW , bltParam->dstH , bltParam->pitch , bltParam->orientation);

#ifdef MT6573_PROFILEIMGTRANS
struct timeval t1 , t2;
gettimeofday(&t1,NULL);
#endif

//PMSG("Bitblt start!!");

    //TODO : Check parameters
    MDP = new MT6573MDPDrv();
    if(NULL == MDP)
    {
        PLOG("get MDP obj failed");
        ret = MHAL_INVALID_DRIVER;
        goto EXIT2;
    }

#ifdef MT6573_PROFILEIMGTRANS
gettimeofday(&t2,NULL);
PMSG("New:%u",(t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
gettimeofday(&t1,NULL);
#endif

    //Lock resource
    if(MHAL_NO_ERROR != mHalBitblt_MDPRoute(bltParam , MDP , &u4ResTbl))
    {
        ret = MHAL_INVALID_RESOURCE;
        goto EXIT2;
    }

#ifdef MT6573_PROFILEIMGTRANS
gettimeofday(&t2,NULL);
PMSG("Route:%u",(t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
gettimeofday(&t1,NULL);
#endif

//    PMSG("Locked resource is 0x%x" , (unsigned int)u4ResTbl);
    //SW workaround for 4x4 block YUV input
    if((MHAL_FORMAT_MTK_YUV == bltParam->srcFormat) && (bltParam->srcW > 1008))
    {
        pIntBuffAddr = (unsigned char *)pmem_alloc_sync(bltParam->srcWStride*bltParam->srcH*2 , &FD);
        if(NULL == pIntBuffAddr)
        {
            ret = MHAL_INVALID_MEMORY;
            goto EXIT1;
        }

        u4MultipleEntry = ((bltParam->srcW + 1007)/1008);
        u4Residual = (bltParam->srcW - ((u4MultipleEntry - 1)*1008));

        //Residual
        InterbltParam.srcX = (u4MultipleEntry-1)*1008;
        InterbltParam.srcY = 0;
        InterbltParam.srcW = u4Residual;
        InterbltParam.srcWStride = bltParam->srcWStride;
        InterbltParam.srcH = bltParam->srcH;
        InterbltParam.srcHStride = bltParam->srcHStride;
        InterbltParam.srcAddr = bltParam->srcAddr;
        InterbltParam.srcFormat = bltParam->srcFormat;
        InterbltParam.dstW = u4Residual;
        InterbltParam.dstH = bltParam->srcH;
        InterbltParam.dstAddr = (unsigned long)pIntBuffAddr + (bltParam->srcW - u4Residual)*2;
        InterbltParam.dstFormat = MHAL_FORMAT_YUY2;
        InterbltParam.pitch = bltParam->srcWStride;
        InterbltParam.orientation = 0;

        //Execute image transform
        for( ; u4MultipleEntry > 0 ; u4MultipleEntry -= 1)
        {
            ret = mHal_ImageTransform_Exec(&InterbltParam , u4ResTbl , MDP , 1);
            if(MHAL_NO_ERROR != ret)
            {
                goto EXIT1;
            }
            InterbltParam.srcX = (u4MultipleEntry-2)*1008;
            InterbltParam.srcW = 1008;
            InterbltParam.dstW = 1008;
            InterbltParam.dstAddr = (unsigned long)pIntBuffAddr + (u4MultipleEntry-2)*1008*2;
        }

        memcpy(&InterbltParam , bltParam , sizeof(mHalBltParam_t));
        InterbltParam.srcAddr = (MUINT32)pIntBuffAddr;
        InterbltParam.srcFormat = MHAL_FORMAT_YUY2;
        ret = mHal_ImageTransform_Exec(&InterbltParam , u4ResTbl , MDP , 0);
    }
    else
    {
        //Execute image transform
        ret = mHal_ImageTransform_Exec(bltParam , u4ResTbl , MDP , 0);
    }

#ifdef MT6573_PROFILEIMGTRANS
gettimeofday(&t2,NULL);
PMSG("HW:%u",(t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
gettimeofday(&t1,NULL);
#endif

//Save
#if 0
    FILE *fp;
    unsigned long index;
    fp = fopen("/data/inter.raw", "w");
    fwrite((void *)pIntBuffAddr , 1 , InterbltParam.srcWStride*InterbltParam.srcHStride*2 , fp);

    fclose(fp);
#endif

    //SW workaround for 4x4 block YUV input
    if((MHAL_FORMAT_MTK_YUV == bltParam->srcFormat) && (bltParam->srcW > 1008))
    {
        pmem_free((void *)pIntBuffAddr , bltParam->srcWStride*bltParam->srcH*2 , FD);
    }

EXIT1 :
//    PMSG("Unlock resource");
    MDP->UnlockResource(u4ResTbl);

//    PMSG("Bitblt done");
EXIT2 :
    delete MDP;

#ifdef MT6573_PROFILEIMGTRANS
gettimeofday(&t2,NULL);
PMSG("Delete:%u",(t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
#endif

    return ret;
}

#define VIA_CAM
int mHal_ImagePostProcess(mHalImgPostProcess_t * a_stParam)
{
    int ret = MHAL_NO_ERROR;
    MT6573MDPDrv * MDP = NULL;
    IspDrv *pIspDrv = NULL;
    stLockResParam resParam;
    MT6573MDPDrv::RDMA_PARAM stRDMA_Param;
    MT6573MDPDrv::RESZ_PARAM stRESZ_Param;
    MT6573MDPDrv::ROTDMA_PARAM stROTDMA_Param;

    MT6573MDPDrv::RDMA0_PARAM stRDMA0Param;
    MT6573MDPDrv::CRZ_IPP_PARAM stCRZParam;
    MT6573MDPDrv::IPP_PARAM stIPPParam;
    MT6573MDPDrv::ROTDMA0_PARAM stROTDMA0Param;

    MT6573MDPDrv::SYSRAM_INFO stRDMA0Sysram;
    MT6573MDPDrv::SYSRAM_INFO stROTDMASysram;
    stSysramParam stRDMA0SysramParam;
    stSysramParam stROTDMASysramParam;

    MT6573MDPDrv::ErrMsg retMsg = MT6573MDPDrv::MSG_OK;
    int retVal = MHAL_NO_ERROR;
    int fd = -1;


    unsigned long u4Offset;

    memset(&stCRZParam , 0 , sizeof(MT6573MDPDrv::CRZ_IPP_PARAM));

    MDP = new MT6573MDPDrv();
    if(NULL == MDP)
    {
        PLOG("get MDP obj failed");
        return MHAL_INVALID_DRIVER;
    }

    pIspDrv = IspDrv::createInstance();
    if (!pIspDrv) {
        PLOG("IspDrv::createInstance fail \n");
        ret = MHAL_INVALID_DRIVER;
        goto EXIT2;
    }

    if (pIspDrv->init() < 0) {
        PLOG("pIspDrv->init() fail \n");
        ret = MHAL_INVALID_DRIVER;
        goto EXIT2;
    }

    if (pIspDrv->reset() < 0) {
        PLOG("pIspDrv->reset() fail \n");
        ret = MHAL_INVALID_DRIVER;
        goto EXIT2;
    }

    resParam.u4IsTimeShared = 0;
    resParam.u4TimeOutInms = 1000;
    resParam.u4LockResTable = (RDMA0_FLAG | CRZ_IPP_OVL_FLAG | ROTDMA0_FLAG);

    if(MT6573MDPDrv::MSG_OK != MDP->LockResource(&resParam))
    {
        PLOG("Lock MDP resource failed");
        ret = MHAL_INVALID_RESOURCE;
        goto EXIT2;
    }

    //Config MDP
    stRDMA_Param.bContinuousHWTrigger = 0;
    stRDMA_Param.bEISEn = 0;
    stRDMA_Param.u4EISCON = 0;

    switch(a_stParam->srcFormat)
    {
        case MHAL_FORMAT_RGB_565 :
            stRDMA_Param.eINFMT = MT6573MDPDrv::RGB565;
            u4Offset = 0;
        break;
        case MHAL_FORMAT_RGB_888 :
            stRDMA_Param.eINFMT = MT6573MDPDrv::RGB888;
            u4Offset = 0;
        break;
        case MHAL_FORMAT_ARGB_8888 :
            stRDMA_Param.eINFMT = MT6573MDPDrv::ARGB8888;
            u4Offset = 0;
        break;
        case MHAL_FORMAT_YUV_420 :
            stRDMA_Param.eINFMT = MT6573MDPDrv::YV12_Planar;
            u4Offset = (a_stParam->srcWStride*a_stParam->srcHStride); 
        break;
        case MHAL_FORMAT_YUV_420_SP :
            stRDMA_Param.eINFMT = MT6573MDPDrv::NV21;
            u4Offset = (a_stParam->srcWStride*a_stParam->srcHStride); 
        break;
        case MHAL_FORMAT_MTK_YUV :
            stRDMA_Param.eINFMT = MT6573MDPDrv::YUV420_4x4BLK;
            u4Offset = (a_stParam->srcWStride*a_stParam->srcHStride);
        break;
        case MHAL_FORMAT_YUY2 :
            stRDMA_Param.eINFMT = MT6573MDPDrv::YUY2_Pack;
            u4Offset = 0;
        break;
        default :
     	     PLOG("Unsupport format 0x%x" , a_stParam->srcFormat);
     	     ret = MHAL_INVALID_FORMAT;
     	     goto EXIT1;
        break;
    }

    stRDMA_Param.u2SrcWidthInPixel = a_stParam->srcWStride;
    stRDMA_Param.u2SrcHeightInLine = a_stParam->srcHStride;
    stRDMA_Param.u2CropWidthInPixel = a_stParam->srcWStride;
    stRDMA_Param.u2CropHeightInLine = a_stParam->srcHStride;
    stRDMA_Param.u2CropXOffsetInPixel = 0;
    stRDMA_Param.u2CropYOffsetLine = 0;
    stRDMA_Param.uInBufferCnt = 1;
    stRDMA_Param.u4InputYAddr = a_stParam->srcAddr;
    stRDMA_Param.u4InputYtoUoffsetInBytes = u4Offset;
    stRDMA_Param.u4InputYtoVoffsetInBytes = (u4Offset + (u4Offset >> 2));
    stRDMA_Param.u4InputYtoNextYoffsetInBytes = 0;

    stRESZ_Param.bCamIn = 1;
    stRESZ_Param.bContinuous = 0;
    stRESZ_Param.u2SrcWidthInPixel = stRDMA_Param.u2CropWidthInPixel;
    stRESZ_Param.u2SrcHeightInLine = stRDMA_Param.u2CropHeightInLine;
    stRESZ_Param.u2CropOffsetXInPixel = a_stParam->srcX;
    stRESZ_Param.u2CropOffsetYInLine = a_stParam->srcY;
    stRESZ_Param.u2CropWidthInPixel = a_stParam->srcW;
    stRESZ_Param.u2CropHeightInLine = a_stParam->srcH;
    stRESZ_Param.u2DestWidthInPixel = (a_stParam->orientation & 0x1 ? a_stParam->dstH : a_stParam->dstW);
    stRESZ_Param.u2DestHeightInLine = (a_stParam->orientation & 0x1 ? a_stParam->dstW : a_stParam->dstH);
    stRESZ_Param.uDnScaleCoeff = 15;//TODO
    stRESZ_Param.uUpScaleCoeff = 8;//TODO
    stRESZ_Param.uEEHCoeff = 0;
    stRESZ_Param.uEEVCoeff = 0;

    stROTDMA_Param.bCamIn = 0;
    stROTDMA_Param.bContinuous = 0;
    stROTDMA_Param.bDithering = ((MHAL_FORMAT_RGB_565 == a_stParam->dstFormat) && (MHAL_FORMAT_RGB_565 != a_stParam->srcFormat) ? 1 : 0);
    stROTDMA_Param.bFlip = (MHAL_BITBLT_FLIP_H & a_stParam->orientation ? 1 : 0);
    stROTDMA_Param.bRotate = (a_stParam->orientation & 0x3);
    stROTDMA_Param.bSpecifyAlpha = 0;
    stROTDMA_Param.uAlpha = 255;

    switch(a_stParam->dstFormat)
    {
        case MHAL_FORMAT_RGB_565 :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::RGB565;
        break;
        case MHAL_FORMAT_RGB_888 :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::RGB888;
        break;
        case MHAL_FORMAT_ARGB_8888 :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::ARGB8888;
        break;
        case MHAL_FORMAT_ABGR_8888 :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::ABGR8888;
        break;
        case MHAL_FORMAT_YUV_420 :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::YV12_Planar;
        break;        
        case MHAL_FORMAT_YUV_420_SP :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::NV21;
        break;
        case MHAL_FORMAT_MTK_YUV :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::YUV420_4x4BLK;
        break;
        case MHAL_FORMAT_YUY2 :
            stROTDMA_Param.eOutFMT = MT6573MDPDrv::YUY2_Pack;
        break;
        default :
     	     PLOG("Unsupport dest format 0x%x" , a_stParam->dstFormat);
     	     ret = MHAL_INVALID_FORMAT;
     	     goto EXIT1;
        break;
    }

    stROTDMA_Param.uOutBufferCnt = 1;
    stROTDMA_Param.u4OutYBuffAddr = a_stParam->dstAddr;
    stROTDMA_Param.u4OutYtoUoffsetInBytes = (a_stParam->dstWStride*a_stParam->dstH);//TODO
    stROTDMA_Param.u4OutYtoVoffsetInBytes = stROTDMA_Param.u4OutYtoUoffsetInBytes + ((a_stParam->dstWStride*a_stParam->dstH) >> 2);//TODO
    stROTDMA_Param.u4OutYtoNextYoffsetInBytes = 0;

    stROTDMA_Param.u2SrcROIOffsetX = 0;
    stROTDMA_Param.u2SrcROIOffsetY = 0;
    stROTDMA_Param.u2SrcWidthInPixel = stRESZ_Param.u2DestWidthInPixel;
    stROTDMA_Param.u2SrcHeightInLine = stRESZ_Param.u2DestHeightInLine;
    stROTDMA_Param.u2ROIWidthInPixel = stRESZ_Param.u2DestWidthInPixel;
    stROTDMA_Param.u2ROIHeightInLine = stRESZ_Param.u2DestHeightInLine;
    stROTDMA_Param.u2DestROIOffsetX = a_stParam->dstX;
    stROTDMA_Param.u2DestROIOffsetY = a_stParam->dstY;
    stROTDMA_Param.u2DestImgWidthInPixel = a_stParam->dstWStride;
    stROTDMA_Param.u2DestImgHeightInLine = a_stParam->dstH;

    stRDMA0Param.pRDMA0_param = &stRDMA_Param;
    stRDMA0Param.bToCAM = 1;
    stRDMA0Param.bToPRZ0 = 0;
    stRDMA0Param.bToVRZ = 0;
    stRDMA0Param.bToJPGDMA = 0;

    stIPPParam.bEnColorAdj = 0;
    stIPPParam.bEnRGBReplace = 0;
    stIPPParam.bEnColorInverse = 0;
    stIPPParam.bEnColorize = 0;
    stIPPParam.bEnSatAdj = 0;
    stIPPParam.bEnHueAdj = 0;
    stIPPParam.bEnContractBrightAdj = 0;
    stCRZParam.pCRZ_Param = &stRESZ_Param;
    stCRZParam.pIPP_Param = &stIPPParam;
    stCRZParam.pOVL_Param = NULL;
#ifdef VIA_CAM
    stCRZParam.uSource = 1;
#else
    stCRZParam.uSource = 2;
#endif
    stCRZParam.bToROTDMA = 1;
    stCRZParam.bToPRZ0 = 0;
    stCRZParam.bToVRZ = 0;
    stCRZParam.bToJPGDMA = 0;
    stCRZParam.bAddOverlayToROTDMA = 0;
    stCRZParam.bAddOverlayToPRZ0 = 0;
    stCRZParam.bAddOverlayToVRZ = 0;
    stCRZParam.bAddOverlayToJPGDMA = 0;

    stROTDMA0Param.pROTDMA0_Param = &stROTDMA_Param;
    stROTDMA0Param.u4InputSource = 0;

    retMsg = MDP->Config_RDMA0(&stRDMA0Param , &stRDMA0Sysram);
    if(MT6573MDPDrv::MSG_OK != retMsg)
    {
        PLOG("Config RDMA0 failed : %d" , retMsg);
        ret = MHAL_INVALID_PARA;
        goto EXIT1;
    }

    retMsg = MDP->Config_CRZ_IPP(&stCRZParam);
    if(MT6573MDPDrv::MSG_OK != retMsg)
    {
        PLOG("Config CRZIPP failed : %d" , retMsg);
        ret = MHAL_INVALID_PARA;
        goto EXIT1;
    }

    retMsg = MDP->Config_ROTDMA0(&stROTDMA0Param , &stROTDMASysram);
    if(MT6573MDPDrv::MSG_OK != retMsg)
    {
        PLOG("Config ROTDMA0 failed : %d" , retMsg);
        ret = MHAL_INVALID_PARA;
        goto EXIT1;
    }

#ifdef VIA_CAM
    //Config ISP
    if (pIspDrv->setTgGrabRange(0, (stRDMA_Param.u2CropWidthInPixel - 1) , 0 , (stRDMA_Param.u2CropHeightInLine - 1)) < 0) {
        PLOG("pIspDrv->setTgGrabRange() fail \n");
        ret = MHAL_INVALID_PARA;
        goto EXIT1;
    }

    // Set camera module path
    if (pIspDrv->setCamModulePath(2, 1 , 0 , 0, OUTPUT_TYPE_FMT_YUV422 , 0 , 1) < 0) {
        PLOG("pIspDrv->setCamModulePath() fail \n");
        ret = MHAL_INVALID_PARA;
        goto EXIT1;
    }

    // Set view finder mode
    ret = pIspDrv->setViewFinderMode(0 , 0);
    if (ret < 0) {
        PLOG("pIspDrv->setViewFinderMode() fail \n");
        ret = -7;
        goto EXIT1;
    }
#endif

//Allocate sysram
    fd = open("/dev/mt6573-SYSRAM", O_RDWR);
    if(-1 == fd)
    {
        PLOG("Open sysram driver failed");
        ret = MHAL_INVALID_DRIVER;
        goto EXIT1;
    }

    stROTDMASysramParam.u4Alignment = stROTDMASysram.u4Alignment;
    stROTDMASysramParam.u4Size = (stROTDMASysram.u4RecommandBuffCnt*stROTDMASysram.u4SingleBuffSize);
    stROTDMASysramParam.u4Owner = MT6573SYSRAMUSR_ROTDMA0;

    if(0 < stROTDMASysramParam.u4Size)
    {
        if(ioctl(fd, MT6573MDP_X_SYSRAMALLOC , &stROTDMASysramParam))
        {
            PLOG("Allocate sysram for ROTDMA failed");
            close(fd);
            ret = MHAL_INVALID_MEMORY;
            goto EXIT1;
        }
    }

    stRDMA0SysramParam.u4Alignment = stRDMA0Sysram.u4Alignment;
    stRDMA0SysramParam.u4Size = (stRDMA0Sysram.u4RecommandBuffCnt*stRDMA0Sysram.u4SingleBuffSize);
    stRDMA0SysramParam.u4Owner = MT6573SYSRAMUSR_RDMA0;
    if(0 < stRDMA0SysramParam.u4Size)
    {
        if(ioctl(fd, MT6573MDP_X_SYSRAMALLOC , &stRDMA0SysramParam))
        {
            PLOG("Allocate sysram for RDMA0 failed");
            if(0 < stROTDMASysramParam.u4Size)
            {
                ioctl(fd, MT6573MDP_S_SYSRAMFREE , &stROTDMASysramParam);
            }
            close(fd);
            ret = MHAL_INVALID_MEMORY;
            goto EXIT1;
        }
    }

//Enable
    retMsg = MDP->Enable_ROTDMA0(stROTDMASysramParam.u4Addr , stROTDMASysram.u4RecommandBuffCnt);
    if(MT6573MDPDrv::MSG_OK != retMsg)
    {
        PLOG("Enable ROTDMA0 failed : %d" , retMsg);
        retVal = MHAL_INVALID_CTRL_CODE;
        goto Disable;
    }

    retMsg = MDP->Enable_CRZ_IPP();
    if(MT6573MDPDrv::MSG_OK != retMsg)
    {
        PLOG("Enable CRZIPP failed : %d" , retMsg);
        retVal = MHAL_INVALID_CTRL_CODE;
        goto Disable;
    }

#ifdef VIA_CAM
    if (pIspDrv->control(1) < 0) {
        PLOG("pIspDrv->control() fail \n");
        ret = MHAL_INVALID_CTRL_CODE;
        goto Disable;
    }
#endif

    retMsg = MDP->Enable_RDMA0(stRDMA0SysramParam.u4Addr);
    if(MT6573MDPDrv::MSG_OK != retMsg)
    {
        PLOG("Enable RDMA0 failed : %d" , retMsg);
        retVal = MHAL_INVALID_CTRL_CODE;
        goto Disable;
    }

//Wait Done
    retMsg = MDP->Wait_Done(ROTDMA0_FLAG);
    if(MT6573MDPDrv::MSG_OK != retMsg)
    {
        PLOG("Wait done failed : %d" , retMsg);
        retVal = MHAL_INVALID_CTRL_CODE;
        goto Disable;
    }

//Disable
Disable:
    MDP->Disable_RDMA0();

#ifdef VIA_CAM
    pIspDrv->control(0);
#endif

    MDP->Disable_CRZ_IPP();

    MDP->Disable_ROTDMA0();

//Free sysram
    if(0 < stRDMA0SysramParam.u4Size)
    {
        ioctl(fd, MT6573MDP_S_SYSRAMFREE , &stRDMA0SysramParam);
    }
    if(0 < stROTDMASysramParam.u4Size)
    {    
        ioctl(fd, MT6573MDP_S_SYSRAMFREE , &stROTDMASysramParam);
    }
    close(fd);

EXIT1 :
    MDP->UnlockResource(resParam.u4LockResTable);

#ifdef VIA_CAM
    if(pIspDrv)
    {
        pIspDrv->uninit();
        pIspDrv->destroyInstance();
        pIspDrv = NULL;
    }
#endif

EXIT2 :
    delete MDP;

    return ret;
}

