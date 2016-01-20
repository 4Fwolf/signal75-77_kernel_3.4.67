/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MdpUltCam"
//
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
//
#include "MediaTypes.h"
//#include "mdp_hal_imp.h"
#include "MDP_ULT_Camio.h"
#include "MT6573MDPDrv.h"
#include "mt6573_sysram.h"
#include "jpeg_enc_hal.h"
//

#define MDP_ULT_CAM

/*******************************************************************************
*
********************************************************************************/
#define MDP_LOG(fmt, arg...)    LOGD(fmt, ##arg)
#define MDP_ERR(fmt, arg...)    LOGE("Err: %5d:, "fmt, __LINE__, ##arg)

/*******************************************************************************
*
********************************************************************************/
static MT6573MDPDrv *pMdpDrv;
static unsigned long resTable;
static MT6573MDPDrv::SYSRAM_INFO stROTDMA0Sysram;
static MT6573MDPDrv::SYSRAM_INFO stROTDMA1Sysram;
static MT6573MDPDrv::SYSRAM_INFO stROTDMA2Sysram;
static MT6573MDPDrv::SYSRAM_INFO stJpgDMASysram;
static MT6573MDPDrv::SYSRAM_INFO stRDMA0Sysram;
static MT6573MDPDrv::SYSRAM_INFO stVRZSysram;
static stSysramParam stROTDMA0SysramParam;
static stSysramParam stROTDMA1SysramParam;
static stSysramParam stROTDMA2SysramParam;
static stSysramParam stVRZSysramParam;
static stSysramParam stRDMA0SysramParam;
static stSysramParam stJpgDMASysramParam;
//
static int fdSysRam;
//

/*******************************************************************************
*
********************************************************************************/
/*
MdpHal* 
MdpHal::createInstance()
{
    return MdpHalImp::getInstance();
}
*/

/*******************************************************************************
*
********************************************************************************/
MdpUltCam*
MdpUltCam::
getInstance()
{
    MDP_LOG("[MdpUltCam] getInstance \n");
    static MdpUltCam singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void   
MdpUltCam::
destroyInstance() 
{
}

/*******************************************************************************
*
********************************************************************************/
MdpUltCam::MdpUltCam()
{
    MDP_LOG("[MdpUltCam]");

    //
    pMdpDrv = new MT6573MDPDrv();
    if (pMdpDrv == NULL) {
        MDP_ERR("new MT6573MDPDrv() fail \n");
    }
    resTable = 0;
    //    
    fdSysRam = open("/dev/mt6573-SYSRAM", O_RDWR);
    if (fdSysRam < 0) {
        MDP_ERR("open /dev/mt6573-SYSRAM fail \n");
    }
    resTable = 0;
    //
    memset(&mMDPParam, 0, sizeof(halIDPParam_t));
    memset(&stROTDMA0Sysram, 0, sizeof(MT6573MDPDrv::SYSRAM_INFO));
    memset(&stROTDMA1Sysram, 0, sizeof(MT6573MDPDrv::SYSRAM_INFO));
    memset(&stROTDMA2Sysram, 0, sizeof(MT6573MDPDrv::SYSRAM_INFO));
    memset(&stJpgDMASysram, 0, sizeof(MT6573MDPDrv::SYSRAM_INFO));
    memset(&stRDMA0Sysram, 0, sizeof(MT6573MDPDrv::SYSRAM_INFO));
    memset(&stVRZSysram, 0, sizeof(MT6573MDPDrv::SYSRAM_INFO));
    memset(&stROTDMA0SysramParam, 0, sizeof(stSysramParam));
    memset(&stROTDMA1SysramParam, 0, sizeof(stSysramParam));
    memset(&stROTDMA2SysramParam, 0, sizeof(stSysramParam));
    memset(&stVRZSysramParam, 0, sizeof(stSysramParam));
    memset(&stRDMA0SysramParam, 0, sizeof(stSysramParam));
    memset(&stJpgDMASysramParam, 0, sizeof(stSysramParam));

    memset(&mDISPRingBuffer, 0, sizeof(stRingBuffer_PrivateData));
    memset(&mVDOENCRingBuffer, 0, sizeof(stRingBuffer_PrivateData));
    memset(&mFDRingBuffer, 0, sizeof(stRingBuffer_PrivateData));
    memset(&mQUICKVIEWRingBuffer, 0, sizeof(stRingBuffer_PrivateData));
    memset(&mJPEGRingBuffer, 0, sizeof(stRingBuffer_PrivateData));
    memset(&mTHUMBNAILRingBuffer, 0, sizeof(stRingBuffer_PrivateData));
}

/******************************************************************************
*
********************************************************************************/
MdpUltCam::~MdpUltCam()
{
    MDP_LOG("[~MdpHalImp]");

    // 
    if (pMdpDrv) {
        delete pMdpDrv;
    }
    pMdpDrv = NULL;
    //
    if (fdSysRam) {
       close(fdSysRam); 
    }
}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpUltCam::dumpReg()
{
    MINT32 ret = 0;
    
    MDP_LOG("[dumpReg]");
    //
    ret = pMdpDrv->Dump_Reg();
    //
    return ret;   
}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpUltCam::init()
{
    MINT32 ret = 0;
    
    MDP_LOG("[init]");

    return ret;   
}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpUltCam::uninit()
{
    MINT32 ret = 0;
    
    MDP_LOG("[uninit]");
    
    //
    return ret;   
}

/******************************************************************************
*
*******************************************************************************/
MINT32 MdpUltCam::lockResource(MUINT32 res)
{
    MINT32 ret = -1;
    MT6573MDPDrv::ErrMsg retMsg;
    stLockResParam resParam;
    //
    MDP_LOG("[lockResource] E res: 0x%x \n", res);
    //
    resParam.u4LockResTable = res;
    resParam.u4TimeOutInms = 500;
    resParam.u4IsTimeShared = 0;
    retMsg = pMdpDrv->LockResource(&resParam);
    if (MT6573MDPDrv::MSG_OK == retMsg) {
        ret = 0;
    }
    MDP_LOG("[lockResource] X ret: %d \n", ret);

    return ret;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 MdpUltCam::unlockResource(MUINT32 res)
{
    MINT32 ret = 0;
    MT6573MDPDrv::ErrMsg retMsg;

    retMsg = pMdpDrv->UnlockResource(res);
    if (MT6573MDPDrv::MSG_OK != retMsg) {
        MDP_ERR("[unlockResource] pMdpDrv->UnlockResource fail: %d, res: 0x%x \n", retMsg, res);
        ret = -1;
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpUltCam::start()
{
    MINT32 ret = 0;
    MT6573MDPDrv::ErrMsg retMsg;

    MDP_LOG("[start]");
    //
    if (JPGDMA_FLAG & resTable) {
        stJpgDMASysramParam.u4Alignment = stJpgDMASysram.u4Alignment;
        stJpgDMASysramParam.u4Size = (stJpgDMASysram.u4RecommandBuffCnt * stJpgDMASysram.u4SingleBuffSize);
        stJpgDMASysramParam.u4Owner = MT6573SYSRAMUSR_JPGDMA;
        if (0 < stJpgDMASysramParam.u4Size) {
            if (ioctl(fdSysRam, MT6573MDP_X_SYSRAMALLOC , &stJpgDMASysramParam)) {
                MDP_ERR("MT6573MDP_X_SYSRAMALLOC fail \n");
                return -1;
            }
        }
    }
    //
    if (ROTDMA0_FLAG & resTable) {
        stROTDMA0SysramParam.u4Alignment = stROTDMA0Sysram.u4Alignment;
        stROTDMA0SysramParam.u4Size = (stROTDMA0Sysram.u4RecommandBuffCnt * stROTDMA0Sysram.u4SingleBuffSize);
        stROTDMA0SysramParam.u4Owner = MT6573SYSRAMUSR_ROTDMA0;
        if (0 < stROTDMA0SysramParam.u4Size) {
            if (ioctl(fdSysRam, MT6573MDP_X_SYSRAMALLOC , &stROTDMA0SysramParam)) {
                MDP_ERR("MT6573MDP_X_SYSRAMALLOC fail \n");
                return -1;
            }
        }
    }
    //
    if (ROTDMA1_FLAG & resTable) {
        stROTDMA1SysramParam.u4Alignment = stROTDMA1Sysram.u4Alignment;
        stROTDMA1SysramParam.u4Size = (stROTDMA1Sysram.u4RecommandBuffCnt * stROTDMA1Sysram.u4SingleBuffSize);
        stROTDMA1SysramParam.u4Owner = MT6573SYSRAMUSR_ROTDMA1;
        if (0 < stROTDMA1SysramParam.u4Size) {
            if (ioctl(fdSysRam, MT6573MDP_X_SYSRAMALLOC , &stROTDMA1SysramParam)) {
                MDP_ERR("MT6573MDP_X_SYSRAMALLOC fail \n");
                return -1;
            }
        }
    }
    //
    if (ROTDMA2_FLAG & resTable) {
        stROTDMA2SysramParam.u4Alignment = stROTDMA2Sysram.u4Alignment;
        stROTDMA2SysramParam.u4Size = (stROTDMA2Sysram.u4RecommandBuffCnt * stROTDMA2Sysram.u4SingleBuffSize);
        stROTDMA2SysramParam.u4Owner = MT6573SYSRAMUSR_ROTDMA2;
        if (0 < stROTDMA2SysramParam.u4Size) {
            if (ioctl(fdSysRam, MT6573MDP_X_SYSRAMALLOC , &stROTDMA2SysramParam)) {
                MDP_ERR("MT6573MDP_X_SYSRAMALLOC fail \n");
                return -1;
            }
        }
    }
    //
    if (RDMA0_FLAG & resTable) {
        stRDMA0SysramParam.u4Alignment = stRDMA0Sysram.u4Alignment;
        stRDMA0SysramParam.u4Size = (stRDMA0Sysram.u4RecommandBuffCnt * stRDMA0Sysram.u4SingleBuffSize);
        stRDMA0SysramParam.u4Owner = MT6573SYSRAMUSR_RDMA0;
        if (RDMA0_FLAG) {
            if(0 < stRDMA0SysramParam.u4Size) {
                if (ioctl(fdSysRam, MT6573MDP_X_SYSRAMALLOC , &stRDMA0SysramParam)) {
                    MDP_ERR("MT6573MDP_X_SYSRAMALLOC fail \n");
                    return -1;
                }
            }
        }
    }
    //
    if (VRZ_FLAG & resTable) {
        stVRZSysramParam.u4Alignment = stVRZSysram.u4Alignment;
        stVRZSysramParam.u4Size = (stVRZSysram.u4RecommandBuffCnt * stVRZSysram.u4SingleBuffSize);
        stVRZSysramParam.u4Owner = MT6573SYSRAMUSR_VRZ;
        if(0 < stVRZSysramParam.u4Size) {
            if (ioctl(fdSysRam, MT6573MDP_X_SYSRAMALLOC , &stVRZSysramParam)) {
                MDP_ERR("MT6573MDP_X_SYSRAMALLOC fail \n");
                return -1;
            }
        }
    }
    //    
    if (mMDPParam.u4Mode == MDP_MODE_PRV_YUV) {
        //
        if (ROTDMA2_FLAG & resTable) {
            retMsg = pMdpDrv->Enable_ROTDMA2(stROTDMA2SysramParam.u4Addr, stROTDMA2Sysram.u4RecommandBuffCnt);
            if (MT6573MDPDrv::MSG_OK != retMsg) {
                MDP_ERR("Enable_ROTDMA2 fail : %d \n", retMsg);
                return -2;        
            }
        }
        //
        if (VRZ_FLAG & resTable) {
            retMsg = pMdpDrv->Enable_VRZ(stVRZSysramParam.u4Addr, stVRZSysram.u4RecommandBuffCnt);
            if (MT6573MDPDrv::MSG_OK != retMsg) {
                MDP_ERR("Enable_VRZ fail : %d \n", retMsg);
                return -4;        
            }
        }
    }
    //
    if (JPGDMA_FLAG & resTable) {
        retMsg = pMdpDrv->Enable_JPGDMA(stJpgDMASysramParam.u4Addr);
        if (MT6573MDPDrv::MSG_OK != retMsg) {
            MDP_ERR("Enable_JPGDMA fail : %d \n", retMsg);
            return -2;        
        }    
    }
    //
    if (ROTDMA0_FLAG & resTable) {
        retMsg = pMdpDrv->Enable_ROTDMA0(stROTDMA0SysramParam.u4Addr, stROTDMA0Sysram.u4RecommandBuffCnt);
        if (MT6573MDPDrv::MSG_OK != retMsg) {
            MDP_ERR("Enable_ROTDMA0 fail : %d \n", retMsg);
            return -3;        
        }
    }
    //
    if (ROTDMA1_FLAG & resTable) {
        retMsg = pMdpDrv->Enable_ROTDMA1(stROTDMA1SysramParam.u4Addr, stROTDMA1Sysram.u4RecommandBuffCnt);
        if (MT6573MDPDrv::MSG_OK != retMsg) {
            MDP_ERR("Enable_ROTDMA1 fail : %d \n", retMsg);
            return -3;        
        }
    }
    //
    if (PRZ0_FLAG & resTable) {
        retMsg = pMdpDrv->Enable_PRZ0();
        if (MT6573MDPDrv::MSG_OK != retMsg) {
            MDP_ERR("Enable_ROTDMA1 fail : %d \n", retMsg);
            return -3;        
        }
    }
    //
    if (CRZ_IPP_OVL_FLAG & resTable) {
        retMsg = pMdpDrv->Enable_CRZ_IPP();
        if(MT6573MDPDrv::MSG_OK != retMsg) {
            MDP_ERR("Enable_CRZ_IPP failed : %d \n" , retMsg);
            return -4;
        }
    }

    return ret;   
}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpUltCam::stop()
{
    MINT32 ret = 0;
    
    MDP_LOG("[stop]");

    //
    if (RDMA0_FLAG & resTable) {
        pMdpDrv->Disable_RDMA0();
    }
    //
    if (CRZ_IPP_OVL_FLAG & resTable) {
        pMdpDrv->Disable_CRZ_IPP();
    }
    //
    if (PRZ0_FLAG & resTable) {
        pMdpDrv->Disable_PRZ0();
    }
    //
    if (VRZ_FLAG & resTable) {
        pMdpDrv->Disable_VRZ();
    }
    //        
    if (ROTDMA0_FLAG & resTable) {
        pMdpDrv->Disable_ROTDMA0();
    }
    //
    if (ROTDMA1_FLAG & resTable) {
        pMdpDrv->Disable_ROTDMA1();
    }
    //
    if (ROTDMA2_FLAG & resTable) {
        pMdpDrv->Disable_ROTDMA2();
    }
    //
    if (JPGDMA_FLAG & resTable) {
        pMdpDrv->Disable_JPGDMA();
    }
    //
    if(0 < stJpgDMASysramParam.u4Size) {    
        ioctl(fdSysRam, MT6573MDP_S_SYSRAMFREE , &stJpgDMASysramParam);
        stJpgDMASysramParam.u4Size = 0;
    }
    // Free sysram
    if(0 < stROTDMA0SysramParam.u4Size) {    
        ioctl(fdSysRam, MT6573MDP_S_SYSRAMFREE , &stROTDMA0SysramParam);
        stROTDMA0SysramParam.u4Size = 0;
    }
    //
    if(0 < stROTDMA1SysramParam.u4Size) {    
        ioctl(fdSysRam, MT6573MDP_S_SYSRAMFREE , &stROTDMA1SysramParam);
        stROTDMA1SysramParam.u4Size = 0;
    }
    //
    if(0 < stROTDMA2SysramParam.u4Size) {    
        ioctl(fdSysRam, MT6573MDP_S_SYSRAMFREE , &stROTDMA2SysramParam);
        stROTDMA2SysramParam.u4Size = 0;
    }
    //
    if (0 < stRDMA0SysramParam.u4Size) {
        ioctl(fdSysRam, MT6573MDP_S_SYSRAMFREE , &stRDMA0SysramParam);
        stRDMA0SysramParam.u4Size = 0;
    }
    //
    if (0 < stVRZSysramParam.u4Size) {
        ioctl(fdSysRam, MT6573MDP_S_SYSRAMFREE , &stVRZSysramParam);
        stVRZSysramParam.u4Size = 0;
    }

    // Unlock resource
    ret = unlockResource(resTable);
    resTable = 0;
    
    return ret;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 MdpUltCam::calCropRect(
    rect_t rSrc,
    rect_t rDst,
    rect_t *prCrop,
    MUINT32 zoomRatio
)
{
    float defaultRatio = ((float) rSrc.w) / ((float) rSrc.h);
    float ratio = ((float) rDst.w) / ((float) rDst.h);
    rect_t rCrop;

    MDP_LOG("[calCropRegion] src: %d/%d, dst: %d/%d, zoom: %d \n", 
        rSrc.w, rSrc.h, rDst.w, rDst.h, zoomRatio); 

    if (defaultRatio < ratio) {
        // Need to reduce height
        rCrop.w = rSrc.w;
        rCrop.h = (int) rSrc.w / ratio;
    }
    else if (defaultRatio > ratio) {
        rCrop.w = (int) rSrc.h * ratio;
        rCrop.h = rSrc.h;
    }
    else {
        rCrop.w = rSrc.w;
        rCrop.h = rSrc.h;
    }
    //
    if (zoomRatio > 800) {
        MDP_LOG("[calCropRegion] Zoom Raio reach max value %d" , zoomRatio);
        zoomRatio = 800;
    }
    //
    rCrop.w = (int) (rCrop.w * 100.0 / (float) zoomRatio);   
    rCrop.h = (int) (rCrop.h * 100.0 / (float) zoomRatio);
    // make it 16x
    rCrop.w = rCrop.w >> 4 << 4;
    rCrop.h = rCrop.h >> 4 << 4;
    //
    rCrop.x = (rSrc.w - rCrop.w) / 2;
    rCrop.y = (rSrc.h - rCrop.h) / 2;
    //
    *prCrop = rCrop;

    MDP_LOG("  Crop region: %d, %d, %d, %d \n", rCrop.x, rCrop.y, rCrop.w, rCrop.h); 
    
    return 0;
}

/*******************************************************************************
*RDMA0->CRZ->IPP->OVL->ROTDMA0
*                                   ->VRZ->ROTDMA2
*Always occupied
********************************************************************************/
MINT32 MdpUltCam::setPrv(halIDPParam_t *phalIDPParam)
{
    MINT32 ret = 0;
    rect_t rSrc, rDst, rCrop;
    MINT32 isContinous = 1;
    MT6573MDPDrv::ROTDMA_PARAM stROTDMA0;
    MT6573MDPDrv::RESZ_PARAM stCRZ;
    MT6573MDPDrv::IPP_PARAM stIPP;
    MT6573MDPDrv::CRZ_IPP_PARAM stCRZIPPParam;
    MT6573MDPDrv::ROTDMA0_PARAM stROTDMA0Param;
    MT6573MDPDrv::VRZ_PARAM stVRZ;
    MT6573MDPDrv::ROTDMA_PARAM stROTDMA2;
    MT6573MDPDrv::ROTDMA2_PARAM stROTDMA2Param;
    MUINT32 ySize;
    MUINT32 uvSize;
    MUINT32 addr;
    MT6573MDPDrv::COLOR_FMT yuvFmt;
    MUINT32 yuvW, yuvH, yuvVirAddr, yuvBufCnt;
    MUINT32 isROTDMA2 = (phalIDPParam->u4RgbW | phalIDPParam->u4RgbH) > 0 ? 1 : 0;
    
    MDP_LOG("[setPrv] src: %d/%d, mtkyuv: %d/%d, yuv: %d/%d, rgb: %d/%d, rot: %d, flip: %d, %d , YUVaddr: 0x%x , MTKYUVaddr: 0x%x\n", 
        phalIDPParam->u4SrcW, phalIDPParam->u4SrcH, phalIDPParam->u4MTKYuvW, phalIDPParam->u4MTKYuvH,
        phalIDPParam->u4YuvW, phalIDPParam->u4YuvH, phalIDPParam->u4RgbW, 
        phalIDPParam->u4RgbH, phalIDPParam->u4Rotate, phalIDPParam->u4Flip, isROTDMA2 , phalIDPParam->u4YuvVirAddr , phalIDPParam->u4MTKYuvVirAddr);
    //
    if ((phalIDPParam->u4MTKYuvW == 0) || (phalIDPParam->u4MTKYuvH == 0)) {
        // YV12, another option NV21-> MT6573MDPDrv::NV21
        yuvFmt = MT6573MDPDrv::YV12_Planar;
        yuvW = phalIDPParam->u4YuvW;
        yuvH = phalIDPParam->u4YuvH;
        yuvVirAddr = phalIDPParam->u4YuvVirAddr;
        yuvBufCnt = phalIDPParam->u4YuvBufCnt;
    }
    else {
        // MtkYuv
        yuvFmt = MT6573MDPDrv::YUV420_4x4BLK;
        yuvW = phalIDPParam->u4MTKYuvW;
        yuvH = phalIDPParam->u4MTKYuvH;
        yuvVirAddr = phalIDPParam->u4MTKYuvVirAddr;
        yuvBufCnt = phalIDPParam->u4MTKYuvBufCnt;
    }
    //
    memset(&rSrc, 0, sizeof(rSrc));
    memset(&rDst, 0, sizeof(rDst));
    memset(&rCrop, 0, sizeof(rCrop));
    rSrc.w = phalIDPParam->u4SrcW;
    rSrc.h = phalIDPParam->u4SrcH;
    rDst.w = yuvW;
    rDst.h = yuvH;
    calCropRect(rSrc, rDst, &rCrop, phalIDPParam->u4ZoomRatio);
    //
    // Lock resource first
    resTable = (CRZ_IPP_OVL_FLAG | ROTDMA0_FLAG);
    if (isROTDMA2) {
        resTable |= (ROTDMA2_FLAG | VRZ_FLAG);
    }
    if (lockResource(resTable) < 0) {
        MDP_ERR("  lockResource fail \n");
        resTable = 0;
        return -1;
    }
    //
    // Config ROTDMA0 for YUV420sp
    stROTDMA0.bCamIn = 1;
    stROTDMA0.bContinuous = isContinous;
    stROTDMA0.bDithering = 0;
    // Total 8 combinations, rotate x flip (4 x 2 = 8)
    stROTDMA0.bFlip = phalIDPParam->u4Flip;
    switch (phalIDPParam->u4Rotate) {
    case 0:
        stROTDMA0.bRotate = 0;
        break;
    case 90:
        stROTDMA0.bRotate = 1;
        break;
    case 180:
        stROTDMA0.bRotate = 2;
        break;
    case 270:
        stROTDMA0.bRotate = 3;
        break;
    default: 
        stROTDMA0.bRotate = 0;
        break;
    }
    stROTDMA0.bSpecifyAlpha = 0;
    stROTDMA0.uAlpha = 255;
    stROTDMA0.eOutFMT = yuvFmt;
    // Config Buffer
    stROTDMA0.uOutBufferCnt = yuvBufCnt;
    ySize = yuvW * yuvH;
    uvSize = (yuvW * yuvH) * 5 / 4;
    addr = yuvVirAddr;

//
    stROTDMA0.u4OutYBuffAddr = addr;
    stROTDMA0.u4OutYtoUoffsetInBytes = ySize;
    stROTDMA0.u4OutYtoVoffsetInBytes = uvSize;
    stROTDMA0.u4OutYtoNextYoffsetInBytes = (ySize * 2);
//
#if 0
    for (MUINT32 i = 0; i < yuvBufCnt; i++) {
        stROTDMA0.u4OutYBuffAddr[i] = addr;
        stROTDMA0.u4OutUBuffAddr[i] = addr + ySize;
        stROTDMA0.u4OutVBuffAddr[i] = addr + uvSize;
        addr += (ySize * 2);
    }
#endif

    //
    mDISPRingBuffer.u4BufferCnt = yuvBufCnt;
    mVDOENCRingBuffer.u4BufferCnt = yuvBufCnt;

    stROTDMA0.u2ROIOffsetX = 0;
    stROTDMA0.u2ROIOffsetY = 0;
    stROTDMA0.u2ROIWidthInPixel = yuvW;
    stROTDMA0.u2ROIHeightInLine = yuvH;
    if ((stROTDMA0.bRotate & 1) == 0) {
        stROTDMA0.u2DestImgWidthInPixel = yuvW;
        stROTDMA0.u2DestImgHeightInLine = yuvH;
    }
    else {
        stROTDMA0.u2DestImgWidthInPixel = yuvH;
        stROTDMA0.u2DestImgHeightInLine = yuvW;
    }
    //
    stROTDMA0Param.pROTDMA0_Param = &stROTDMA0;
    stROTDMA0Param.u4InputSource = 0;
    //
    // Config ROTDMA2 for RGB565
    stROTDMA2 = stROTDMA0;
    stROTDMA2.eOutFMT = MT6573MDPDrv::RGB565;
    // Config Buffer
    stROTDMA2.uOutBufferCnt = phalIDPParam->u4RgbBufCnt;
    ySize = phalIDPParam->u4RgbW * phalIDPParam->u4RgbH;
    addr = phalIDPParam->u4RgbVirAddr;

    stROTDMA2.u4OutYBuffAddr = addr;
    stROTDMA2.u4OutYtoUoffsetInBytes = 0;
    stROTDMA2.u4OutYtoVoffsetInBytes = 0;
    stROTDMA2.u4OutYtoNextYoffsetInBytes = (ySize * 2);

#if 0
    for (MUINT32 i = 0; i < phalIDPParam->u4RgbBufCnt; i++) {
        stROTDMA2.u4OutYBuffAddr[i] = addr;
        addr += (ySize * 2);
    }
#endif

    mFDRingBuffer.u4BufferCnt = phalIDPParam->u4RgbBufCnt;

    //
    stROTDMA2.u2ROIOffsetX = 0;
    stROTDMA2.u2ROIOffsetY = 0;
    stROTDMA2.u2ROIWidthInPixel = phalIDPParam->u4RgbW;
    stROTDMA2.u2ROIHeightInLine = phalIDPParam->u4RgbH;
    stROTDMA2.u2DestImgWidthInPixel = phalIDPParam->u4RgbW;
    stROTDMA2.u2DestImgHeightInLine = phalIDPParam->u4RgbH;
    //
    stROTDMA2Param.pROTDMA2_Param = &stROTDMA2;
    stROTDMA2Param.u4InputSource = 2;
    //
    // Config VRZ
    stVRZ.bCamIn = 1;
    stVRZ.uSource = 2;
    stVRZ.u2SrcWidthInPixel = yuvW;
    stVRZ.u2SrcHeightInLine = yuvH;
    stVRZ.u2DestWidthInPixel = phalIDPParam->u4RgbW;
    stVRZ.u2DestHeightInLine= phalIDPParam->u4RgbH;
    stVRZ.bContinuous = isContinous;
    //
    // Config CRZ
    stCRZ.bCamIn = 1;
    stCRZ.bContinuous = isContinous;
    stCRZ.u2SrcWidthInPixel = phalIDPParam->u4SrcW;
    stCRZ.u2SrcHeightInLine = phalIDPParam->u4SrcH;
    stCRZ.u2CropOffsetXInPixel = rCrop.x;
    stCRZ.u2CropOffsetYInLine = rCrop.y;
    stCRZ.u2CropWidthInPixel = rCrop.w;
    stCRZ.u2CropHeightInLine = rCrop.h;
    stCRZ.u2DestWidthInPixel = yuvW;
    stCRZ.u2DestHeightInLine = yuvH;
    stCRZ.uDnScaleCoeff = 15;    //TODO tune parameters here
    stCRZ.uUpScaleCoeff = 8;     //TODO tune parameters here
    stCRZ.uEEHCoeff = 0;
    stCRZ.uEEVCoeff = 0;
    // Config Ipp    
    stIPP.bEnColorAdj = 0;
    stIPP.bEnRGBReplace = 0;
    stIPP.bEnColorInverse = 0;
    stIPP.bEnColorize = 0;
    stIPP.bEnSatAdj = 0;
    stIPP.bEnHueAdj = 0;
    stIPP.bEnContractBrightAdj = 0;
    //
    stCRZIPPParam.pCRZ_Param = &stCRZ;
    stCRZIPPParam.pIPP_Param = &stIPP;
    stCRZIPPParam.pOVL_Param = NULL;
    stCRZIPPParam.uSource = 0;
    stCRZIPPParam.bToROTDMA = 1;
    stCRZIPPParam.bToPRZ0 = 0;
    stCRZIPPParam.bToVRZ = isROTDMA2;
    stCRZIPPParam.bToJPGDMA = 0;
    stCRZIPPParam.bAddOverlayToROTDMA = 0;
    stCRZIPPParam.bAddOverlayToPRZ0 = 0;
    stCRZIPPParam.bAddOverlayToVRZ = 0;
    stCRZIPPParam.bAddOverlayToJPGDMA = 0;
    //
    MT6573MDPDrv::ErrMsg retMsg = pMdpDrv->Config_CRZ_IPP(&stCRZIPPParam);
    if (MT6573MDPDrv::MSG_OK != retMsg) {
        MDP_ERR("Config_CRZ_IPP fail : %d \n", retMsg);
        return -1;
    }        
    //
    retMsg = pMdpDrv->Config_ROTDMA0(&stROTDMA0Param , &stROTDMA0Sysram);
    if (MT6573MDPDrv::MSG_OK != retMsg) {
        MDP_ERR("Config_ROTDMA0 fail : %d \n", retMsg);
        return -2;
    }
    if (isROTDMA2) {
        //
        retMsg = pMdpDrv->Config_ROTDMA2(&stROTDMA2Param , &stROTDMA2Sysram);
        if (MT6573MDPDrv::MSG_OK != retMsg) {
            MDP_ERR("Config_ROTDMA2 fail : %d \n", retMsg);
            return -3;
        }
        //
        retMsg = pMdpDrv->Config_VRZ(&stVRZ, &stVRZSysram);
        if (MT6573MDPDrv::MSG_OK != retMsg) {
            MDP_ERR("Config_VRZ fail : %d \n", retMsg);
            return -4;
        }
    }

    //
    mDISPRingBuffer.u4Active = 1;
    mDISPRingBuffer.u4ReaderPosition = ((0 != mMDPParam.u4MTKYuvW) && (0 != mMDPParam.u4MTKYuvH)) ? (mMDPParam.u4MTKYuvBufCnt -1) : (mMDPParam.u4YuvBufCnt - 1);
    mVDOENCRingBuffer.u4Active = 1;
    mVDOENCRingBuffer.u4ReaderPosition = ((0 != mMDPParam.u4MTKYuvW) && (0 != mMDPParam.u4MTKYuvH)) ? (mMDPParam.u4MTKYuvBufCnt -1) : (mMDPParam.u4YuvBufCnt - 1);
    mFDRingBuffer.u4Active = 1;
    mFDRingBuffer.u4ReaderPosition = (mMDPParam.u4RgbBufCnt - 1);

    return ret;   
}

/*******************************************************************************
*RDMA0->CRZ->IPP->OVL->ROTDMA0
*          ->JPEGDMA
********************************************************************************/
MINT32 MdpUltCam::setCapJpg(halIDPParam_t *phalIDPParam)
{
    MINT32 ret = 0;
    rect_t rSrc, rDst, rCrop;
    MT6573MDPDrv::ErrMsg retMsg;
    stLockResParam resParam;
    MINT32 isContinous = 0;
    MT6573MDPDrv::RESZ_PARAM stCRZ;
    MT6573MDPDrv::IPP_PARAM stIPP;
    MT6573MDPDrv::CRZ_IPP_PARAM stCRZIPPParam;
    MT6573MDPDrv::JPGDMA_PARAM stJpgDma;
    MT6573MDPDrv::ROTDMA_PARAM stROTDMA1;
    MT6573MDPDrv::ROTDMA1_PARAM stROTDMA1Param;
    MT6573MDPDrv::RESZ_PARAM stPRZ;
    MT6573MDPDrv::PRZ0_PARAM stPRZ0Param;
    MUINT32 ySize;
    MUINT32 addr;
#if defined MDP_ULT_CAM
	MT6573MDPDrv::RDMA_PARAM stRDMA0;
    MT6573MDPDrv::RDMA0_PARAM stRDMA0Param;
#endif
    
    MDP_LOG("[setCapJpg] src: %d/%d, yuv: %d/%d \n", 
        phalIDPParam->u4SrcW, phalIDPParam->u4SrcH, phalIDPParam->u4YuvW, phalIDPParam->u4YuvH);

    memset(&rSrc, 0, sizeof(rSrc));
    memset(&rDst, 0, sizeof(rDst));
    memset(&rCrop, 0, sizeof(rCrop));
    rSrc.w = phalIDPParam->u4SrcW;
    rSrc.h = phalIDPParam->u4SrcH;
    rDst.w = phalIDPParam->u4YuvW;
    rDst.h = phalIDPParam->u4YuvH;
    calCropRect(rSrc, rDst, &rCrop, phalIDPParam->u4ZoomRatio);
    //
    // Lock resource first
    resTable = CRZ_IPP_OVL_FLAG | JPGDMA_FLAG | PRZ0_FLAG | ROTDMA1_FLAG | BRZ_FLAG;
    if (lockResource(resTable) < 0) {
        MDP_ERR("  lockResource fail \n");
        resTable = 0;
        return -1;
    }
    //
    // Config ROTDMA1 for RGB565 quickview
#if defined MDP_ULT_CAM
	stROTDMA1.bCamIn = 0;
#else
	stROTDMA1.bCamIn = 1;
#endif
    stROTDMA1.bContinuous = isContinous;
    stROTDMA1.bDithering = 0;
    stROTDMA1.bFlip = 0;
    stROTDMA1.bRotate = phalIDPParam->u4Rotate;
    stROTDMA1.bSpecifyAlpha = 0;
    stROTDMA1.uAlpha = 255;
    stROTDMA1.eOutFMT = MT6573MDPDrv::RGB565;
    // Config Buffer
    stROTDMA1.uOutBufferCnt = phalIDPParam->u4RgbBufCnt;
    ySize = phalIDPParam->u4RgbW * phalIDPParam->u4RgbH;
    addr = phalIDPParam->u4RgbVirAddr;

    stROTDMA1.u4OutYBuffAddr = addr;
    stROTDMA1.u4OutYtoUoffsetInBytes = 0;
    stROTDMA1.u4OutYtoVoffsetInBytes = 0;
    stROTDMA1.u4OutYtoNextYoffsetInBytes = (ySize * 2);
#if 0
    for (MUINT32 i = 0; i < phalIDPParam->u4RgbBufCnt; i++) {
        stROTDMA1.u4OutYBuffAddr[i] = addr;
        addr += (ySize * 2);
    }
#endif
    //
    stROTDMA1.u2ROIOffsetX = 0;
    stROTDMA1.u2ROIOffsetY = 0;
    stROTDMA1.u2ROIWidthInPixel = phalIDPParam->u4RgbW;
    stROTDMA1.u2ROIHeightInLine = phalIDPParam->u4RgbH;
    stROTDMA1.u2DestImgWidthInPixel = phalIDPParam->u4RgbW;
    stROTDMA1.u2DestImgHeightInLine = phalIDPParam->u4RgbH;
    //
    stROTDMA1Param.pROTDMA1_Param = &stROTDMA1;
    stROTDMA1Param.u4InputSource = 1;
    //
    // Config PRZ
#if defined MDP_ULT_CAM
    stPRZ.bCamIn = 0;
#else
    stPRZ.bCamIn = 1;
#endif
	stPRZ.bContinuous = isContinous;
    stPRZ.u2SrcWidthInPixel = phalIDPParam->u4YuvW;
    stPRZ.u2SrcHeightInLine = phalIDPParam->u4YuvH;
    stPRZ.u2CropOffsetXInPixel = 0;
    stPRZ.u2CropOffsetYInLine = 0;
    stPRZ.u2CropWidthInPixel = phalIDPParam->u4YuvW;
    stPRZ.u2CropHeightInLine = phalIDPParam->u4YuvH;
    stPRZ.u2DestWidthInPixel = phalIDPParam->u4RgbW;
    stPRZ.u2DestHeightInLine = phalIDPParam->u4RgbH;
    stPRZ.uDnScaleCoeff = 15;    //TODO tune parameters here
    stPRZ.uUpScaleCoeff = 8;     //TODO tune parameters here
    stPRZ.uEEHCoeff = 0;
    stPRZ.uEEVCoeff = 0;
    // Config PRZ param
    stPRZ0Param.pPRZ0_Param = &stPRZ;
#if defined MDP_ULT_CAM
    stPRZ0Param.bCamIn = 0;
#else
    stPRZ0Param.bCamIn = 1;
#endif
    stPRZ0Param.bToROTDMA = 1;
    stPRZ0Param.bToVRZ = 0;
    stPRZ0Param.bSource = 1;
    //    
    // Config CRZ
#if defined MDP_ULT_CAM
    stCRZ.bCamIn = 0;
#else
    stCRZ.bCamIn = 1;
#endif
    stCRZ.bContinuous = isContinous;
    stCRZ.u2SrcWidthInPixel = phalIDPParam->u4SrcW;
    stCRZ.u2SrcHeightInLine = phalIDPParam->u4SrcH;
    stCRZ.u2CropOffsetXInPixel = rCrop.x;
    stCRZ.u2CropOffsetYInLine = rCrop.y;
    stCRZ.u2CropWidthInPixel = rCrop.w;
    stCRZ.u2CropHeightInLine = rCrop.h;
    stCRZ.u2DestWidthInPixel = phalIDPParam->u4YuvW;
    stCRZ.u2DestHeightInLine = phalIDPParam->u4YuvH;
    stCRZ.uDnScaleCoeff = 15;    //TODO tune parameters here
    stCRZ.uUpScaleCoeff = 8;     //TODO tune parameters here
    stCRZ.uEEHCoeff = 0;
    stCRZ.uEEVCoeff = 0;
    // Config Ipp    
    stIPP.bEnColorAdj = 0;
    stIPP.bEnRGBReplace = 0;
    stIPP.bEnColorInverse = 0;
    stIPP.bEnColorize = 0;
    stIPP.bEnSatAdj = 0;
    stIPP.bEnHueAdj = 0;
    stIPP.bEnContractBrightAdj = 0;
    //
    stCRZIPPParam.pCRZ_Param = &stCRZ;
    stCRZIPPParam.pIPP_Param = &stIPP;
    stCRZIPPParam.pOVL_Param = NULL;
#if defined MDP_ULT_CAM
    stCRZIPPParam.uSource = 2;
#else
    stCRZIPPParam.uSource = 0;
#endif
    stCRZIPPParam.bToROTDMA = 0;
    stCRZIPPParam.bToPRZ0 = 1;
    stCRZIPPParam.bToVRZ = 0;
    stCRZIPPParam.bToJPGDMA = 1;
    stCRZIPPParam.bAddOverlayToROTDMA = 0;
    stCRZIPPParam.bAddOverlayToPRZ0 = 0;
    stCRZIPPParam.bAddOverlayToVRZ = 0;
    stCRZIPPParam.bAddOverlayToJPGDMA = 0;
    //
    retMsg = pMdpDrv->Config_CRZ_IPP(&stCRZIPPParam);
    if (MT6573MDPDrv::MSG_OK != retMsg) {
        MDP_ERR("Config_CRZ_IPP fail : %d \n", retMsg);
        return -1;
    }        
    //
    stJpgDma.u2ImgWidthInPixel = phalIDPParam->u4YuvW;
    stJpgDma.u2ImgHeightInLine = phalIDPParam->u4YuvH;
    stJpgDma.bContinuous = 0;
    stJpgDma.bCamIn = 1;
    stJpgDma.bSource = 0;
    stJpgDma.u2OutFMT = 422;
    //
    retMsg = pMdpDrv->Config_JPGDMA(&stJpgDma, &stJpgDMASysram);
    if (MT6573MDPDrv::MSG_OK != retMsg) {
        MDP_ERR("Config_JPGDMA fail : %d \n", retMsg);
        return -2;
    }
    //
    retMsg = pMdpDrv->Config_ROTDMA1(&stROTDMA1Param , &stROTDMA1Sysram);
    if (MT6573MDPDrv::MSG_OK != retMsg) {
        MDP_ERR("Config_ROTDMA1 fail : %d \n", retMsg);
        return -3;
    }
    //
    retMsg = pMdpDrv->Config_PRZ0(&stPRZ0Param);
    if (MT6573MDPDrv::MSG_OK != retMsg) {
        MDP_ERR("Config_PRZ0 fail : %d \n", retMsg);
        return -3;
    }
    //

#if defined MDP_ULT_CAM
    memset(&stRDMA0, 0, sizeof(stRDMA0));

    stRDMA0.eINFMT = MT6573MDPDrv::RGB565;
    stRDMA0.u2SrcWidthInPixel = rSrc.w;
    stRDMA0.u2SrcHeightInLine = rSrc.h;
    stRDMA0.u2CropWidthInPixel = rSrc.w;
    stRDMA0.u2CropHeightInLine= rSrc.h;
    stRDMA0.u2CropXOffsetInPixel = 0;
    stRDMA0.u2CropYOffsetLine = 0;
    stRDMA0.uInBufferCnt = 1;
    //TODO: RDMA source address
	//stRDMA0.u4InputYAddr = (unsigned long)fSrcAddr;
    
    memset(&stRDMA0Param, 0, sizeof(stRDMA0Param));
    stRDMA0Param.pRDMA0_param = &stRDMA0;
#endif
    
    return ret;   
}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpUltCam::setConf(halIDPParam_t *phalIDPParam)
{
    MINT32 ret = 0;

    //
    memcpy(&mMDPParam, phalIDPParam, sizeof(halIDPParam_t));
    //
    switch (phalIDPParam->u4Mode) {
    case MDP_MODE_PRV_YUV:
        ret = setPrv(phalIDPParam);
        break;
    case MDP_MODE_CAP_JPG:
        ret = setCapJpg(phalIDPParam);
        break;
    default:
        ret = -1;
        break;
    }
    if (ret < 0) {
        return ret;
    }
        
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpUltCam::dequeueBuff(halMdpOutputPort_e e_Port , halMdpBufInfo_t * a_pstBuffInfo)
{
    MT6573MDPDrv::ErrMsg retMsg;
    unsigned long u4WaitTarget;
    unsigned long u4Index , u4NextIndex;
    stRingBuffer_PrivateData * stRingBuffer;
    stTimeStamp TS;

    switch (e_Port) {
    case DISP_PORT:
        u4WaitTarget = ROTDMA0_FLAG;
        stRingBuffer = &mDISPRingBuffer;
        break;
    //
    case VDOENC_PORT:
        u4WaitTarget = ROTDMA0_FLAG;
        stRingBuffer = &mVDOENCRingBuffer;
        break;
    //
    case FD_PORT :
        u4WaitTarget = ROTDMA2_FLAG;
        stRingBuffer = &mFDRingBuffer;
        break;
    //
    case JPEG_PORT ://TODO
    case QUICKVIEW_PORT :
    case THUMBNAIL_PORT :
    default :
        return -1;
    }

    //Check if any buffer available, currently rotdma is writing u4Index
    pMdpDrv->CheckROTDMA_WriteIndex(u4WaitTarget , &u4Index);
    //Reader's next position
    u4NextIndex = ((stRingBuffer->u4ReaderPosition + 1) >= stRingBuffer->u4BufferCnt) ? 0 : (stRingBuffer->u4ReaderPosition + 1);

    if (u4NextIndex == u4Index) {
//MDP_LOG("Reader catch up RSTA:%lu,Position:%lu,WSTA:%lu" , u4Index , stRingBuffer->u4ReaderPosition , u4WSTA);
        //negative, reader catch up writer , wait till next buffer comes
        if(stRingBuffer->u4PreStat)
        {
            pMdpDrv->Wait_Done(u4WaitTarget);//Clear flag
            stRingBuffer->u4PreStat = 0;
        }
        retMsg = pMdpDrv->Wait_Done(u4WaitTarget);

        if (MT6573MDPDrv::MSG_OK != retMsg) {
            MDP_ERR("Wait done failed : %d \n", retMsg);
            return -1;
        }
        a_pstBuffInfo->hwIndex = stRingBuffer->u4ReaderPosition;
        a_pstBuffInfo->fillCnt = 1;
    }
    else {
        //positive, return immediately
        a_pstBuffInfo->hwIndex = stRingBuffer->u4ReaderPosition;
        a_pstBuffInfo->fillCnt = (u4Index > u4NextIndex ? (u4Index - u4NextIndex) :  (stRingBuffer->u4BufferCnt - u4NextIndex + u4Index));
        stRingBuffer->u4PreStat = 1;
//MDP_LOG("There are buffers RSTA:%lu,Position:%lu,u4WSTA:%lu,Cnt:%lu" , u4Index , stRingBuffer->u4ReaderPosition , u4WSTA , a_pstBuffInfo->fillCnt);
    }

    pMdpDrv->Get_ROTDMA0_TimeStamp(&TS);

    for (u4Index = 0 ; u4Index < stRingBuffer->u4BufferCnt ; u4Index += 1) {
        a_pstBuffInfo->timeStamp.timeStampS[u4Index] = TS.u4Sec[u4Index];
        a_pstBuffInfo->timeStamp.timeStampUs[u4Index] = TS.u4MicroSec[u4Index];
    }

    return 0;
}

MINT32 MdpUltCam::enqueueBuff(halMdpOutputPort_e e_Port)
{
    stRingBuffer_PrivateData * stRingBuffer;
    MT6573MDPDrv::ErrMsg retMsg;

    switch (e_Port) {
    case DISP_PORT:
        stRingBuffer = &mDISPRingBuffer;
        retMsg = pMdpDrv->Queue_ROTDMA_Buff(ROTDMA0_FLAG , stRingBuffer->u4ReaderPosition);
        break;
    //
    case VDOENC_PORT:
        stRingBuffer = &mVDOENCRingBuffer;
        retMsg = pMdpDrv->Queue_ROTDMA_Buff(ROTDMA0_FLAG , stRingBuffer->u4ReaderPosition);
        break;
    //
    case FD_PORT :
        stRingBuffer = &mFDRingBuffer;
        retMsg = pMdpDrv->Queue_ROTDMA_Buff(ROTDMA2_FLAG , stRingBuffer->u4ReaderPosition);
        break;
    //
    case JPEG_PORT ://TODO
    case QUICKVIEW_PORT :
    case THUMBNAIL_PORT :
    default :
        return -1;
    }

    if(MT6573MDPDrv::MSG_OK == retMsg)
    {
        stRingBuffer->u4ReaderPosition = (stRingBuffer->u4ReaderPosition + 1) == stRingBuffer->u4BufferCnt ? 0 : (stRingBuffer->u4ReaderPosition + 1);
    }

    return 0;    
}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpUltCam::waitDone(MINT32 mode)
{
    MINT32 ret = 0;
    MT6573MDPDrv::ErrMsg retMsg;

    //MDP_LOG("[waitDone]: %d \n", mode);

    switch (mMDPParam.u4Mode) {
    case MDP_MODE_PRV_YUV:
        // Wait ROTDMA0 first
        retMsg = pMdpDrv->Wait_Done(ROTDMA0_FLAG);
        //pMdpDrv->Dump_Reg();
        if (MT6573MDPDrv::MSG_OK != retMsg) {
            MDP_ERR("Wait done failed : %d \n", retMsg);
            return -2;        
        }
        break;
    case MDP_MODE_CAP_JPG:
        retMsg = pMdpDrv->Wait_Done(ROTDMA1_FLAG);
        //retMsg = MT6573MDPDrv::MSG_OK;
        if (MT6573MDPDrv::MSG_OK != retMsg) {
            MDP_ERR("Wait done failed : %d \n", retMsg);
            return -6;        
        }
        break;
    default:
        ret = -1;
        break;
    }

    return ret;    
}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpUltCam::sendCommand(int cmd, int *parg1, int *parg2, int *parg3)
{
    MINT32 ret = 0;
    rect_t * pRect;

    MDP_LOG("[sendCommand] cmd: 0x%x \n", cmd);
    //
    switch (cmd) {
    case CMD_SET_ZOOM_RATIO:
        pRect = (rect_t *) parg1;
        MDP_LOG("  CMD_SET_ZOOM x: %d y: %d, w: %d, h: %d \n", pRect->x, pRect->y, pRect->w, pRect->h);
        pMdpDrv->Config_CRZ_ZOOM(pRect->x, pRect->y, pRect->w, pRect->h);
        //pMdpDrv->Dump_Reg();
        break;
    default:
        ret = -1;
        break;
    }

    return ret;
}


