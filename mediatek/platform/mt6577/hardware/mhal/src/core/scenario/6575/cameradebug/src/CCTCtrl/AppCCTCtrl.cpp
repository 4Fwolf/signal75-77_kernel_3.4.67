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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AppCCTCtrl.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////
//! \file  AppCCTCtrl.cpp
//! \brief
/*******************************************************************************
 *
 * $Log: AppCCTCtrl.cpp $
 *
 * 12 17 2011 sam.sun
 * [ALPS00104050] [Need Patch] [Volunteer Patch]ICS4.0 mATV Camera migration
 * .
 *
 * 10 17 2011 koli.lin
 * [ALPS00030473] [Camera]
 * [Camera] 1. Fix the preview/capture image luminance different issue for CCT.
 *                2. Add a isp gain control for CCT.
 *
 *
 *******************************************************************************/

#define LOG_TAG "APPCCTCtrl"

#include <stdio.h>
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/threads.h>
#include <linux/cache.h> 

extern "C" {
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <semaphore.h>
#include "mtkfb.h"
}

#include "kd_imgsensor_define.h"
#include "../inc/AcdkTypes.h"
#include "../inc/AcdkErrCode.h"
#include "../inc/AcdkLog.h"
#include "../inc/AcdkCommon.h"
#include <mhal/inc/MediaHal.h>
#include <mhal/inc/camera.h>

#include "cct_if.h"
#include "cct_feature.h"
#include "isp_hal.h"
#include "../inc/AcdkSurfaceView.h"

#include "mHalCam4CCTl.h" 
#include "AppCCTCtrl.h"

//
#include <mhal/inc/camera.h>
using namespace NSCamera;
using namespace NSACDK; 

//#define PREVIEW_WIDTH 320 
//#define PREVIEW_HEIGHT 240
#define RGB_BUFFER_CNT 4 
#define YUV_BUFFER_CNT 4

#define CAM_MEM_SIZE     ((16 * 1024 * 1024 + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1))     // align for cache 

static AppCCTCtrl *g_pAppCCTCtrlObj = NULL; 

#define MEDIA_PATH "//data"
#define PROJECT_NAME "YUSU"

#define MEM_ALIGN_BYTE       16
#define SWAP(a, b) {int c = (a); (a) = (b); (b) = c; }

//#undef MTK_M4U_SUPPORT

/******************************************************************************
*
*******************************************************************************/
/**
 * @par Structure
 *   mhalCamCallbackParam_t
 * @par Description
 *   The parameter of camera callback function
 */
typedef struct mhalCamCallbackParam_s {
    MUINT32 type;
    void *pdata;
} mhalCamCallbackParam_t;


/*******************************************************************************
*
********************************************************************************/
static MRESULT mrPreviewStart_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    ACDK_LOGD("mrPreviewStart_Cmd() - E \n");
    if (g_pAppCCTCtrlObj == NULL) {
        ACDK_LOGE("[mrPreviewStart_Cmd] Null NuCamera Obj\n");
        return E_ACDK_CAMCTRL_NULL_OBJ;
    }
    
    MRESULT mrRet = S_ACDK_CAMCTRL_OK; 
    mrRet = g_pAppCCTCtrlObj->startPreview( NULL);
    return mrRet;
}

/*******************************************************************************
*
********************************************************************************/
static MRESULT mrPreviewStop_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    ACDK_LOGD("mrPreviewStop_Cmd () - E \n");
    if (g_pAppCCTCtrlObj == NULL) {
        ACDK_LOGE("[mrPreviewStop_Cmd] Null NuCamera Obj\n");
        return E_ACDK_CAMCTRL_NULL_OBJ;
    }

    MRESULT mrRet = S_ACDK_CAMCTRL_OK; 
    mrRet = g_pAppCCTCtrlObj->stopPreview();
    return mrRet;
}

/*******************************************************************************
*
********************************************************************************/
static MRESULT mrCaptureImg_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    MRESULT mrRet = S_ACDK_CAMCTRL_OK; 

    ACDK_LOGD(" mrCaptureImg_Cmd() - E \n");
    if (a_u4Argc != (MUINT32)2 && a_u4Argc != (MUINT32)4 && a_u4Argc != 1)
    {
        ACDK_LOGE(" Number of argument must be 1\n");
        ACDK_LOGE(" capraw <mode, 0:preview, 1:capture> <SaveRaw> <Num (Option)> <width(Optoin)> <height(Optoin)>\n");        
        return E_ACDK_CAMCTRL_BAD_ARG;
    }

    MUINT32 mode = atoi((char *)a_pprArgv[0]);
    MUINT32 rawCap = 0; 
    MUINT32 width = 0;
    MUINT32 height = 0; 
    MUINT32 capCnt = 1; 
    if (a_u4Argc == 5)
    {
        rawCap = atoi((char *)a_pprArgv[1]);  
        capCnt = atoi((char *)a_pprArgv[2]); 
        width = atoi((char *)a_pprArgv[3]); 
        height = atoi((char *)a_pprArgv[4]); 
    }
    else if (a_u4Argc == 3) {
        rawCap = atoi((char *)a_pprArgv[1]);  
        capCnt = atoi((char *)a_pprArgv[2]); 
    }
    else if (a_u4Argc == 2) {
        rawCap = atoi((char *)a_pprArgv[1]);  
    }
   
    if(g_pAppCCTCtrlObj != NULL)
    {
        ACDK_LOGD("Capture RAW Image, Cnt = %d \n", capCnt);
        ACDK_LOGD("Mode:%d\n", mode); 
        ACDK_LOGD("Width = %d, Height = %d\n", width, height); 

        for (int i = 0; i < (int)capCnt; i++) {
            ACDK_LOGD("Capture No = %d\n", i); 
            if (rawCap) {
                mrRet = g_pAppCCTCtrlObj->takePicture(mode, JPEG_TYPE | RAW_TYPE, NULL, width, height,  1);
            }
            else {
                mrRet = g_pAppCCTCtrlObj->takePicture(mode, JPEG_TYPE, NULL, width, height,  1);
            }
            mrRet = g_pAppCCTCtrlObj->startPreview( NULL);
        }
    }
    return mrRet;

}

/*******************************************************************************
*
********************************************************************************/
static MRESULT mrQuickViewImg_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    MRESULT mrRet = S_ACDK_CAMCTRL_OK; 
    ACDK_LOGD("mrQuickViewImg_Cmd() - E \n"); 
    if (a_u4Argc != (MUINT32) 1)
    {
        ACDK_LOGE("Usage:quickView <image file>\n"); 
        return E_ACDK_CAMCTRL_BAD_ARG; 
    }

    if (g_pAppCCTCtrlObj != NULL)
    {
        ACDK_LOGD("Quick View Image \n"); 
        ACDK_LOGD("File Name:%s\n", a_pprArgv[0]); 

        mrRet = g_pAppCCTCtrlObj->quickViewImg(a_pprArgv[0], NULL);   
    }

    return S_ACDK_CAMCTRL_OK;
}



/*******************************************************************************
*
********************************************************************************/
static MRESULT mrSwitchCamera_Cmd(const MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    MRESULT mrRet = S_ACDK_CAMCTRL_OK; 
    ACDK_LOGD("mrSwitchCamera_Cmd() - E \n"); 
    if (a_u4Argc != (MUINT32) 1) {
        ACDK_LOGE("switch <cam No (1:main, 2:Sub, 4:matv>\n"); 
        return E_ACDK_CAMCTRL_BAD_ARG; 
    }

    MINT32 setSrc = atoi((char*)a_pprArgv[0]); 

    if (g_pAppCCTCtrlObj != NULL)
    {
        ACDK_LOGD("mrSwitchCamera - Set Src = %x\n",setSrc); 

        mrRet = g_pAppCCTCtrlObj->setSrcDev(setSrc);

        mrRet = g_pAppCCTCtrlObj->startPreview(NULL);
    }

    return S_ACDK_CAMCTRL_OK;
}

/*******************************************************************************
*
********************************************************************************/
static Acdk_CLICmd g_pAppCCTCtrl_Cmds[] =
{
    {"prvstart",            "Preview Start",          mrPreviewStart_Cmd}, 
    {"prvstop",             "preview Stop",          mrPreviewStop_Cmd}, 
    {"cap",                   "capture image",        mrCaptureImg_Cmd},
    {"quickView",          "Quick View Image",  mrQuickViewImg_Cmd}, 
    {"switch",               "Switch Sensor",         mrSwitchCamera_Cmd}, 
    NULL_CLI_CMD, 
};

/*******************************************************************************
*
********************************************************************************/
void   
AppCCTCtrl::
destroyInstance() 
{
    delete this; 
}

/*******************************************************************************
*
********************************************************************************/
AppCCTCtrl::AppCCTCtrl () 
    :AcdkCCTCtrl()
    ,mFrameCnt(0)
    ,mPrvWidth(320)
    ,mPrvHeight(240)
    ,mPrvStartX(0)
    ,mPrvStartY(0)
    ,mOrientation(0)
    ,mImgPmemId(-1)
    ,mImageBuf(0)
    ,mImagePhyAddr(0)
    ,mSurfacePmemId(-1)
    ,mSurfaceBuf(0) 
    ,mSurfacePhyAddr(0)
    ,mSupportedSensorDev(0)
    ,mSrcDev(0)
    ,mPreviewCB(0)
    ,mEncodeLen(0)
    ,mFocusDone(0)
    ,mCapDone(0)
    ,mImgCnt(0)
    ,mSensorOrientation(0)
{
    //
    ACDK_LOGD("[AppCCTCtrl] - E \n"); 
    mAcdkSurfaceViewObj = AcdkSurfaceView::createInstance(); 
    if (mAcdkSurfaceViewObj == 0) {
        ACDK_LOGE("[AppCCTCtrl] Can not create surface view obj \n");        
    }
     
    //
#if (CCT_SUPPORT)        
    mCCTIFObj = CCTIF::createInstance();
    if (0 == mCCTIFObj) {
        ACDK_LOGE("[AppCCTCtrl] Can not create CCTIF obj \n");        
    }
#endif     
 
    //
    mmHalCam4CCTObj = new mHalCam4CCT(); 
    if (0 == mmHalCam4CCTObj) {
        ACDK_LOGE("[AppCCTCtrl] Can not get mmHalCamCCT Obj \n"); 
    }
    //
    memset(&mmHalCamParam, 0 , sizeof(mhalCamParam_t)); 
    g_pAppCCTCtrlObj = this; 
    m_pAcdkCLICmds = g_pAppCCTCtrl_Cmds;
}

/*******************************************************************************
*
********************************************************************************/
AppCCTCtrl::~AppCCTCtrl()
{
    ACDK_LOGD("[~AppCCTCtrl] - E \n"); 
    g_pAppCCTCtrlObj = NULL; 

    //
    if (mAcdkSurfaceViewObj) {
        mAcdkSurfaceViewObj->destroyInstance(); 
        mAcdkSurfaceViewObj = 0; 
    }     
    
    if (mAcdkSurfaceViewObj) {
        mAcdkSurfaceViewObj->destroyInstance(); 
    }
#if (CCT_SUPPORT)    
    if (mCCTIFObj) {
        mCCTIFObj->destroyInstance(); 
    }
#endif    

    delete mmHalCam4CCTObj; 
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::init()
{
    ACDK_LOGD("[init] - E \n"); 
    MUINT32 width =0, height =0;   
    int alignSize = 0; 
    int sensorFound = 0; 
    MINT32 sensorType = MHAL_CAM_SENSOR_DEV_MAIN;
 
 #if (CCT_SUPPORT)    
    if (NULL == mCCTIFObj) {
        ACDK_LOGE("[init] Null CCTIF Obj \n"); 
        return ACDKCCTCTRL_NULL_OBJ; 
    }
#endif     

    if (NULL == mAcdkSurfaceViewObj) {
        ACDK_LOGE("[init] Null Surface View Obj \n"); 
        return ACDKCCTCTRL_NULL_OBJ; 
    }
    
    if (NULL == mmHalCam4CCTObj) {
        ACDK_LOGE("[init] Null mmHalCam4CCT Obj \n"); 
        return ACDKCCTCTRL_NULL_OBJ; 
    }

    MINT32 err; 
    //    
    err = mAcdkSurfaceViewObj->init(); 
    if (err != 0) {
        ACDK_LOGE("[init] Faile to init surface view err = 0x%x\n", err); 
        return ACDKCCTCTRL_API_FAIL;
    }
    //
    ACDK_LOGD("[init] [HAL_getNumberOfCameras] \n");
    mhalCamSensorInfo_t camSensorInfo[8];
    uint32_t retSize = 0;
    int num = 0;
    //
    err = mHalCamBase::searchCamera(&camSensorInfo[0], &retSize);
    if (err != 0) {
        ACDK_LOGE("[init] HAL_getNumberOfCameras Fail err = 0x%x\n", err); 
        goto Exit; 
    }

    num = retSize / sizeof(camSensorInfo[0]);
    ACDK_LOGD("[init] sensor num = %d\n", num); 
    switch (mSrcDev) {
        case 0:               //main 
            sensorType = MHAL_CAM_SENSOR_DEV_MAIN; 
            mSensorVFlip = 0; 
            mSensorHFlip = 0; 
            break; 
        case 1:              //sub 
            sensorType = MHAL_CAM_SENSOR_DEV_SUB; 
            mSensorVFlip = 0; 
            mSensorHFlip = 1; 
            break;  
        case 2:              //atv 
            sensorType = MHAL_CAM_SENSOR_DEV_ATV; 
            mSensorVFlip = 0; 
            mSensorHFlip = 0; 
            break;  
        default:
            ACDK_LOGE("wrong mSrcDev = %d\n", mSrcDev);
            goto Exit; 
    }
    // check search sensor result.
    for (int i = 0; i < num; i++) {
        if (camSensorInfo[i].devType == sensorType) {
            mSrcDev = i; 
            sensorFound = 1; 
            mSensorOrientation = camSensorInfo[i].orientation; 
            break; 
        }
    }
    if (0 == sensorFound) {
        ACDK_LOGE("[init] searchSensor Fail \n"); 
        goto Exit; 
    }
    //
    mAcdkSurfaceViewObj->getSurfaceInfo(width, height, mOrientation); 
    calcPreviewWin(width, height, mPrvStartX, mPrvStartY, mPrvWidth, mPrvHeight); 
    ACDK_LOGD("[init] orientation = %d, mSensorOrientation = %d\n", mOrientation, mSensorOrientation); 
    ACDK_LOGD("[init] prvStartX = %d, prvStartY = %d\n", mPrvStartX, mPrvStartY); 
    ACDK_LOGD("[init] prvWidth = %d, prvHeight = %d\n" ,mPrvWidth, mPrvHeight); 
    // calc the final orientation
    // FIXME: Zaikuo has reverted for 180 (180 only) at overlay 
    // Cases with LCM: 180 (ALPS00239191/ALPS108567)
    if (mOrientation == 180) {
        mOrientation = 0;
    }
    if (sensorType == MHAL_CAM_SENSOR_DEV_SUB) {
        mOrientation =  (mOrientation - mSensorOrientation + 360) % 360; 
    }
    else {
        mOrientation =  (mOrientation + mSensorOrientation) % 360; 
    }
    ACDK_LOGD("[init] mOrientation = %d\n", mOrientation); 
    //  
    err = mmHalCam4CCTObj->mHalCamInit((MINT32*)mSrcDev, sizeof(MINT32));
    if (err != 0) {
        ACDK_LOGE("[init] mHalCamInit Fail = 0x%x\n", err); 
        goto Exit; 
    }

    //
#if defined(MTK_M4U_SUPPORT)
    ACDK_LOGD("[init] - Use M4U Driver, using virtual memory\n"); 
    mImageBuf = (MUINT8 *) memalign (L1_CACHE_BYTES, CAM_MEM_SIZE); 
    mImagePhyAddr = (MUINT32)mImageBuf; 
    //
    alignSize = (mPrvWidth * mPrvHeight * 2 * OVERLAY_BUFFER_CNT + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1); 
    mSurfaceBuf = (MUINT8 *) memalign (L1_CACHE_BYTES, alignSize);
    mSurfacePhyAddr = (MUINT32)mSurfaceBuf; 
#else 
    ACDK_LOGD("[init] - Not Use M4U Driver, using physical memory\n"); 
    mImageBuf = (MUINT8 *)pmem_alloc_sync(CAM_MEM_SIZE , &mImgPmemId);       
    mImagePhyAddr = (MUINT32) pmem_get_phys(mImgPmemId);
    //
    mSurfaceBuf = (MUINT8 *) pmem_alloc (mPrvWidth * mPrvHeight * 2 * OVERLAY_BUFFER_CNT , &mSurfacePmemId);  
    mSurfacePhyAddr = (MUINT32)pmem_get_phys(mSurfacePmemId); 
#endif 
    ACDK_LOGD("mImageBuf virtAddr = 0x%x, phy Addr = 0x%x \n", (MUINT32)mImageBuf, mImagePhyAddr); 
    ACDK_LOGD("mSurfaceBuf virtAddr = 0x%x, phy Addr = 0x%x \n", (MUINT32)mSurfaceBuf, mSurfacePhyAddr); 

    if (NULL == mImageBuf) {
        ACDK_LOGE("[init]- Get the memory fail\n"); 
        goto Exit;
    }
    memset(mImageBuf, 0, CAM_MEM_SIZE);
    
#if defined(MTK_M4U_SUPPORT) 
    for (int i = 0; i < OVERLAY_BUFFER_CNT; i++) {
        ACDK_LOGD("[init] registerBuffer addr = 0x%x, size = %d\n", (MUINT32)mSurfaceBuf + i * mPrvWidth * mPrvHeight * 2, 
                                                                     mPrvWidth * mPrvHeight * 2); 
        err = mAcdkSurfaceViewObj->registerBuffer((MUINT32)mSurfaceBuf + i * mPrvWidth * mPrvHeight * 2, 
                                                              mPrvWidth * mPrvHeight * 2);
        if (err != 0) {
            ACDK_LOGE("[init] mAcdkSurfaceViewObj->registerBuffer() fail err = 0x%x \n", err); 
            goto Exit; 
        }        
    }
#endif 
    //
#if (CCT_SUPPORT)    
    err = mCCTIFObj->init(mmHalCam4CCTObj->getIspHalObj(), sensorType);
    if (err != 0) {
        ACDK_LOGE("[init] mCCTIFObj->init() fail err = 0x%x \n", err); 
        goto Exit; 
    }
#endif     

    return ACDKCCTCTRL_NO_ERROR; 
Exit:
    mAcdkSurfaceViewObj->uninit(); 
    return ACDKCCTCTRL_API_FAIL; 
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::calcPreviewWin(
                   MUINT32 const surfaceWidth, 
                   MUINT32 const surfaceHeight,
                   MUINT32 &x, 
                   MUINT32 &y, 
                   MUINT32 &width, 
                   MUINT32 &height
)
{
    // decide preview size && offset 
    // the screen scan direction has a angle's shift to camera sensor 
    int tempW = surfaceWidth, tempH = surfaceHeight;
    int offset = 0; 
    int degree = (mOrientation + mSensorOrientation) % 360; 
    if (degree == 90 || degree== 270) { //Sensor need to rotate 
        tempW = surfaceHeight; 
        tempH = surfaceWidth; 
        }
    if (tempW > tempH) {
        width = (tempH / 3 * 4) & (~0xf); 
        height = (width / 4 * 3) & (~0xf); 
    }
    else {
        height = (tempW / 4 * 3) & (~0xf); 
        width = (height / 3 * 4) & (~0xf); 
        }

    x = (tempW - width) / 2; 
    y = (tempH - height) / 2; 
    if (degree == 90 || degree== 270) { //Sensor need to rotate
        SWAP(x, y); 
    }

    return ACDKCCTCTRL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::uninit()
{
    ACDK_LOGD("[uninit] - E\n"); 
       
    //
    MINT32 err; 

#if (CCT_SUPPORT)    
    err = mCCTIFObj->uninit();
    if (err != 0) {
        ACDK_LOGE("[uninit] mCCTIFObj->uninit() err = 0x%x \n", err);
    }    
#endif     
   
    err = mmHalCam4CCTObj->mHalCamUninit();
    if (err != 0) {
        ACDK_LOGE("[uninit] mHalCamUninit err = 0x%x \n", err);         
    }    
#if defined(MTK_M4U_SUPPORT) 
    for (int i = 0; i < OVERLAY_BUFFER_CNT; i++) {
        ACDK_LOGD("[uninit] unregisterBuffer addr = 0x%x\n", 
                                   (MUINT32)mSurfaceBuf + i * mPrvWidth * mPrvHeight * 2);
        err = mAcdkSurfaceViewObj->unRegisterBuffer((MUINT32)mSurfaceBuf + i * mPrvWidth * mPrvHeight * 2); 
        if (err != 0) {
            ACDK_LOGE("[uninit] mAcdkSurfaceViewObj->registerBuffer() fail err = 0x%x \n", err); 
        }        
    }
#endif 

    err = mAcdkSurfaceViewObj->uninit(); 
    if (err != 0) {
        ACDK_LOGE("[uninit] Faile to uninit surface view err = 0x%x\n", err); 
    }
    
    //free pmem     
#if defined(MTK_M4U_SUPPORT)
    if (mImageBuf) {
        ACDK_LOGD("[uninit] - mImageBuf E\n"); 
        free(mImageBuf); 
    }
    if (mSurfaceBuf) {
        ACDK_LOGD("[uninit] - mSurfaceBuf E\n"); 
        free(mSurfaceBuf); 
    }
#else 
    if (mImgPmemId > 0) {
        pmem_free(mImageBuf, CAM_MEM_SIZE, mImgPmemId); 
        mImgPmemId = -1;
        mImageBuf = NULL;
    }
    //
    if (mSurfacePmemId > 0) {
        pmem_free(mSurfaceBuf, mPrvWidth * mPrvHeight * 2 * OVERLAY_BUFFER_CNT, mSurfacePmemId); 
        mSurfacePmemId = -1;
        mSurfaceBuf = NULL;
    }     
#endif
   
    ACDK_LOGD("[uninit] - X \n"); 

    return ACDKCCTCTRL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MRESULT AppCCTCtrl::startPreview(Func_CB prvCb) 
{
    ACDK_LOGD("[startPreview] - E!\n");
    MUINT32 yuvsize;
    MINT32 err; 

#if 0
    if( m_eCameraState== CAMERA_PREVIEW)
    {
        return S_ACDK_CAMCTRL_OK; 
    }
#endif 

    memset(&mmHalCamParam, 0, sizeof(mhalCamParam_t));

    //yuv buffer 
    mmHalCamParam.frmYuv.w = mPrvWidth;
    mmHalCamParam.frmYuv.h = mPrvHeight;
    mmHalCamParam.frmYuv.frmSize = mPrvWidth * mPrvHeight * 3 / 2;    //NV 21 
    mmHalCamParam.frmYuv.frmCount = YUV_BUFFER_CNT;
    mmHalCamParam.frmYuv.bufSize = (mPrvWidth * mPrvHeight * 3/2) * YUV_BUFFER_CNT;
    mmHalCamParam.frmYuv.virtAddr = (MUINT32)mImageBuf;
    mmHalCamParam.frmYuv.phyAddr = mImagePhyAddr;
    mmHalCamParam.frmYuv.frmFormat = ePIXEL_FORMAT_I420;
    yuvsize = mmHalCamParam.frmYuv.bufSize;
    //
    mmHalCamParam.frmRgb.w = 0;
    mmHalCamParam.frmRgb.h = 0;
    mmHalCamParam.frmRgb.frmSize = mPrvWidth * mPrvHeight * 2;   //rgb565 
    mmHalCamParam.frmRgb.frmCount = RGB_BUFFER_CNT;
    mmHalCamParam.frmRgb.bufSize = (mPrvWidth * mPrvHeight * 2) * RGB_BUFFER_CNT;
    mmHalCamParam.frmRgb.virtAddr = mmHalCamParam.frmYuv.virtAddr + yuvsize;
    mmHalCamParam.frmRgb.phyAddr = mmHalCamParam.frmYuv.phyAddr + yuvsize;
    mmHalCamParam.mhalObserver = mHalCamObserver(cameraCallback, this);
    mmHalCamParam.shotObserver = mHalCamObserver(cameraCallback, this);
    //
    mmHalCamParam.cam3AParam.prvFps = 30; 
    mmHalCamParam.u4FpsMode = 0; 
    mmHalCamParam.u4CamMode = 0;   // if this mode is 1, it could not be used to capture
    
    // 3A parameter 
    mmHalCamParam.cam3AParam.strobeMode =  2;
    mmHalCamParam.cam3AParam.antiBandingMode = 1; 
    mmHalCamParam.cam3AParam.afMode = 1; 
    mmHalCamParam.cam3AParam.aeMode = 1; 
    mmHalCamParam.cam3AParam.afMeterMode = 1; 
    mmHalCamParam.cam3AParam.aeMeterMode = 0;
    mmHalCamParam.cam3AParam.aeExpMode = 0; 
    mmHalCamParam.cam3AParam.awbMode = 1;
    mmHalCamParam.cam3AParam.isoSpeedMode = 0; 
    mmHalCamParam.cam3AParam.hueMode = 1;                  //middle  
    mmHalCamParam.cam3AParam.brightnessMode = 1;           //middle 
    mmHalCamParam.cam3AParam.contrastMode = 1; 
    mmHalCamParam.cam3AParam.edgeMode = 1;
    mmHalCamParam.u4ZoomVal = 100;	
    //
    mmHalCamParam.u4Rotate = 0;
    //
    mmHalCamParam.u4CamMode = MHAL_CAM_MODE_MPREVIEW;
    
    // set to meta mode
    mmHalCamParam.u4CamIspMode = ISP_OPER_MODE_META;

    ACDK_LOGD("[mrStartPreview] yuv.W:%d\n",  mmHalCamParam.frmYuv.w);
    ACDK_LOGD("[mrStartPreview] yuv.H:%d\n",  mmHalCamParam.frmYuv.h);
    ACDK_LOGD("[mrStartPreview] yuv.frmSize:%d\n",  mmHalCamParam.frmYuv.frmSize);
    ACDK_LOGD("[mrStartPreview] yuv.frmCount:%d\n",  mmHalCamParam.frmYuv.frmCount);
    ACDK_LOGD("[mrStartPreview] yuv.bufSize:%d\n",  mmHalCamParam.frmYuv.bufSize);
    ACDK_LOGD("[mrStartPreview] yuv.virtAddr:0x%x\n",  mmHalCamParam.frmYuv.virtAddr);
    ACDK_LOGD("[mrStartPreview] yuv.phyAddr:0x%x\n",  mmHalCamParam.frmYuv.phyAddr);
    ACDK_LOGD("[mrStartPreview] ISP Operation mode %d\n",  mmHalCamParam.u4CamIspMode);
     
    err = mmHalCam4CCTObj->mHalCamPreviewStart(&mmHalCamParam, sizeof(mhalCamParam_t));

    if (err != MHAL_NO_ERROR)
    {
        ACDK_LOGE("[mrStartPreview] MHAL_IOCTL_PREVIEW_START\n");  
        return ACDKCCTCTRL_API_FAIL; 
    }

    mPreviewCB = prvCb; 
    
    return ACDKCCTCTRL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::stopPreview()
{
    MRESULT mrRet = S_ACDK_CAMCTRL_OK;
#if 0  
    if (m_eCameraState == CAMERA_IDLE)
    {
        return S_ACDK_CAMCTRL_OK; 
    }
#endif 

    MINT32 err = MHAL_NO_ERROR; 

    err = mmHalCam4CCTObj->mHalCamPreviewStop();
    if (err != 0) {
        ACDK_LOGE("mrStopPreview Fail\n"); 
        return ACDKCCTCTRL_API_FAIL; 
    }
    
    //m_eCameraState = CAMERA_IDLE; 
   
    return ACDKCCTCTRL_NO_ERROR; 
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::waitFocusDone()
{
    //Do Focus first 
    MINT32 err = 0; 
    ACDK_LOGD("[waitFocusDone] E \n"); 
    mFocusDone = 0; 
    err = mmHalCam4CCTObj->mHalCamDoFocus(); 
    if (err != 0) {
        ACDK_LOGE("[waitFocusDone] Do Focus Fail err = 0x%x\n", err); 
        return ACDKCCTCTRL_API_FAIL; 
    }

    ACDK_LOGD("[waitFocusDone] Wait wile focus done \n"); 
#if 1   //fixme
    int waitCnt = 1000; 
    while (!mFocusDone && waitCnt > 0) {
        usleep(1000);    
        waitCnt--; 
    }
#endif 
    return ACDKCCTCTRL_NO_ERROR; 
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::handleCapCB(void *param)
{
    if (param == NULL){
        ACDK_LOGE("[handleCapCB] param is NULL \n"); 
        return ACDKCCTCTRL_NULL_OBJ; 
    }
    
    mhalCamCallbackParam_t *pcbParam = (mhalCamCallbackParam_t *) param;   
    mEncodeLen = *(UINT32 *) pcbParam->pdata;
    
    ACDK_LOGD("Type:%d\n", pcbParam->type);
    ACDK_LOGD("JPEG File Size:%d\n", mEncodeLen); 

    mCapDone = TRUE; 
    return ACDKCCTCTRL_NO_ERROR; 
}

/*******************************************************************************
*
********************************************************************************/    
MINT32 AppCCTCtrl::takePicture(
                MUINT32 const mode, 
                MUINT32 const imgType, 
                Func_CB const capCb, 
                MUINT32 const width, 
                MUINT32 const height,  
                MINT32 const isSaveImg 
)
{
    UINT32 virtAddr, phyAddr, size;
    INT32 capW = 0, capH = 0;
    INT32 i4QVRawW, i4QVRawH;
    INT32 thumbW, thumbH;
    INT32 estimateJpegSize;
    ACDK_SENSOR_INFO_STRUCT sensorInfo;
    MINT32 sensorScenarioId;
    UINT32 retLen = 0; 
    MINT32 colorOrder = 0; 
    MINT32 err = 0;
    MINT32 u4AECapturemode;
    IspHal *pIspHalObj = mmHalCam4CCTObj->getIspHalObj(); 

    if (0 == pIspHalObj ) {
        ACDK_LOGE("Null ISP Hal OBj \n"); 
        return ACDKCCTCTRL_NULL_OBJ; 
    }
    //
    if (width != 0 && height != 0) {
        capW = width; 
        capH = height; 
    }
    else {
        if (PREVIEW_MODE == mode) {
            pIspHalObj->sendCommand(ISP_CMD_GET_SENSOR_PRV_RANGE, (int) &capW, (int) &capH);                    
        }
        else {
            pIspHalObj->sendCommand(ISP_CMD_GET_SENSOR_FULL_RANGE, (int) &capW, (int) &capH);         
        }
    }    
    ACDK_LOGD("[takePicture] capW = %d, capH = %d\n", capW, capH); 
   
    // 
    if (mode == PREVIEW_MODE) {
        sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
    }
    else if (mode == CAPTURE_MODE) {
        sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
    }   
    
    //
   memset(&sensorInfo, 0, sizeof(ACDK_SENSOR_INFO_STRUCT));
#if (CCT_SUPPORT)        
    mCCTIFObj->sensorCCTFeatureControl(
                 ACDK_CCT_OP_GET_SENSOR_INFO, 
                 (MUINT8*)&sensorScenarioId, 
                 sizeof(MINT32), 
                 (MUINT8*)&sensorInfo, 
                 sizeof(ACDK_SENSOR_INFO_STRUCT), 
                 &retLen
                 );

    //
    colorOrder = (eCOLOR_ORDER)sensorInfo.SensorOutputDataFormat; 
#endif     
    ACDK_LOGD("[takePicture] Sensor Color Order = %d\n", sensorInfo.SensorOutputDataFormat);
    
    waitFocusDone(); 

    ACDK_LOGD("[takePicture] QQ waitFocusDone function end \n");

    // Do pre capture before stop preview, for 3A
    err = mmHalCam4CCTObj->mHalCamPreCapture();     
    if (err != 0) {
        ACDK_LOGE("[takePicture] mHalCamPreCapture fail err = 0x%x\n", err);
        return ACDKCCTCTRL_API_FAIL;
    }

    //
    err = stopPreview(); 
	
    ACDK_LOGD("[takePicture] stopPreview function end \n");
	
    if (err != 0) {
        ACDK_LOGE("[takePicture] stopPreview fail\n"); 
        return ACDKCCTCTRL_API_FAIL; 
    }

    //
    mCCTIFObj->aaaCCTFeatureControl(
                 ACDK_CCT_OP_AE_CAPTURE_MODE, 
                 (MUINT8*)&sensorScenarioId, 
                 sizeof(MINT32), 
                 (MUINT8*)&u4AECapturemode, 
                 sizeof(MINT32), 
                 &retLen
                 );
  
    estimateJpegSize = CAM_MEM_SIZE; //3 * 1024 * 1024 / 2;  // 2 MB for jpeg size 
    
    i4QVRawW = mPrvWidth;       //m_u4PrvWidth;
    i4QVRawH = mPrvHeight;        //m_u4PrvHeight; 
    
    thumbW = 160; 
    thumbH = 120; 
        
    virtAddr = (UINT32)mImageBuf; 
    phyAddr = (UINT32)mImagePhyAddr; 
    memset(&mmHalCamParam, 0, sizeof(mhalCamParam_t));
    
    mmHalCamParam.frmQv.w = i4QVRawW;
    mmHalCamParam.frmQv.h = i4QVRawH;
    mmHalCamParam.frmQv.frmSize = i4QVRawW * i4QVRawH * 4;
    mmHalCamParam.frmQv.frmCount = 1;
    mmHalCamParam.frmQv.bufSize = mmHalCamParam.frmQv.frmSize;
    mmHalCamParam.frmQv.frmFormat = ePIXEL_FORMAT_I420;//ePIXEL_FORMAT_NV21;//ePIXEL_FORMAT_YV12;//ePIXEL_FORMAT_RGBA8888; //QQ
    mmHalCamParam.u4Rotate = 0; 

    if (imgType & RAW_TYPE) {
        mmHalCamParam.u4IsDumpRaw = 1;
        strcpy((char *)mmHalCamParam.u1FileName, "/data/raw.raw");
    }
    
    mmHalCamParam.mhalObserver = mHalCamObserver(cameraCallback, this);
    mmHalCamParam.shotObserver = mHalCamObserver(cameraCallback, this);

    // RGB Buffer allocation
    size = (mmHalCamParam.frmQv.w * mmHalCamParam.frmQv.h * 4);     // 1 RGB565 buffer
    mmHalCamParam.frmQv.virtAddr = virtAddr + CAM_MEM_SIZE - size;
    mmHalCamParam.frmQv.phyAddr = phyAddr + CAM_MEM_SIZE - size;
//    phyAddr += size;
//    virtAddr += size;
    // JPEG Buffer
    if(PREVIEW_MODE == mode)
        mmHalCamParam.u4CapPreFlag = 1;
    else if(CAPTURE_MODE == mode)
        mmHalCamParam.u4CapPreFlag = 0;
    mmHalCamParam.frmJpg.w = capW & (~0xf);
    mmHalCamParam.frmJpg.h = capH & (~0xf);
    mmHalCamParam.frmJpg.frmSize = estimateJpegSize;
    mmHalCamParam.frmJpg.frmCount = 1;
    mmHalCamParam.frmJpg.bufSize = estimateJpegSize;
    mmHalCamParam.frmJpg.virtAddr = virtAddr ;
    mmHalCamParam.frmJpg.phyAddr = phyAddr;
    
    mmHalCamParam.u4ThumbW = thumbW;
    mmHalCamParam.u4ThumbH = thumbH;
    mmHalCamParam.u4ZoomVal = 100;
    // Other
    mmHalCamParam.u4JpgQValue = 85;//95;

    // Lock resource
#if 0 
    MHalLockParam_t lockParam;
    lockParam.mode = MHAL_MODE_CAM_CAPTURE;
    lockParam.waitMilliSec = 1000;
    lockParam.waitMode = MHAL_MODE_BITBLT;
    status  = mHalIoCtrl(MHAL_IOCTL_LOCK_RESOURCE, &lockParam, sizeof(lockParam), NULL, 0, NULL);
  
    if (status != MHAL_NO_ERROR) {
        // can't lock resource
        ACDK_LOGE("  takePicture, lock resource fail \n");
        return ACDKCCTCTRL_API_FAIL;
    }
#endif 

    // set to meta mode
    mmHalCamParam.u4CamIspMode = ISP_OPER_MODE_META;

    //
    mhalCamBufMemInfo_t mhalCamBufMemInfo;
    ::memset(&mhalCamBufMemInfo, 0, sizeof(mhalCamBufMemInfo));
    mhalCamBufMemInfo.bufID = MHAL_CAM_BUF_CAPTURE;
    mhalCamBufMemInfo.frmW = mPrvWidth;
    mhalCamBufMemInfo.frmH = mPrvHeight;
    err = mmHalCam4CCTObj->mHalCamGetBufMemInfo(&mhalCamBufMemInfo);
    if (err != 0) {
        ACDK_LOGE(" [takePicture] mHalCamGetBufMemInfo fail err = 0x%x\n", err);
        return ACDKCCTCTRL_API_FAIL;
    }

    err = mmHalCam4CCTObj->mHalCamCaptureInit(&mmHalCamParam, sizeof(mhalCamParam_t)); 
    if (err != 0) {
        ACDK_LOGE(" [takePicture] mHalCamCaptureInit fail err = 0x%x\n", err);
        return ACDKCCTCTRL_API_FAIL;
    }    

    err = mmHalCam4CCTObj->mHalCamCaptureStart( &mmHalCamParam, sizeof(mhalCamParam_t)); 
    if (err != MHAL_NO_ERROR) {
        ACDK_LOGE(" [takePicture] mHalCamCaptureStart fail err = 0x%x\n", err);
        return ACDKCCTCTRL_API_FAIL;
    }

    //
    mCapDone= FALSE; 
    //wait for mhal capture down 
    while (!mCapDone) {
        usleep(1000); 
    }
    ACDK_LOGD("[takePicture] QQ capture done \n");
    ACDK_LOGD("[takePicture] QQ capture done type %d\n",imgType);
    ACDK_LOGD("[takePicture] QQ capture done callback %d\n",capCb);	
    //call out the JPEG buffer 
    if ((imgType & JPEG_TYPE) && capCb != NULL)
    {
        ImageBufInfo rImgBufInfo; 
        memset(&rImgBufInfo, 0, sizeof(ImageBufInfo)); 
        rImgBufInfo.eImgType = JPEG_TYPE; 
        rImgBufInfo.rCapBufInfo.pucImgBuf = (MUINT8 *)mmHalCamParam.frmJpg.virtAddr; 
        rImgBufInfo.rCapBufInfo.u2ImgXRes = mmHalCamParam.frmJpg.w; 
        rImgBufInfo.rCapBufInfo.u2ImgYRes = mmHalCamParam.frmJpg.h; 
        rImgBufInfo.rCapBufInfo.u4ImgSize = mEncodeLen; 
		    ACDK_LOGD("[takePicture] QQ capCb called ? \n");
        capCb(&rImgBufInfo); 
    }
    
    //call out the RAW buffer 
    MINT32 paddW = 0, paddH = 0; 
    pIspHalObj->sendCommand(ISP_CMD_GET_RAW_DUMMY_RANGE, (int) &paddW, (int) &paddH);
	ACDK_LOGD("[takePicture] QQ ISP_CMD_GET_RAW_DUMMY_RANGE done \n");
    if ((imgType & RAW_TYPE) && capCb != NULL) {

        ImageBufInfo rImgBufInfo;         
        memset(&rImgBufInfo, 0, sizeof (rImgBufInfo)); 
        rImgBufInfo.eImgType = RAW_TYPE; 
        rImgBufInfo.rRawBufInfo.u4ImgSize =  (capW + paddW) * ( capH + paddH) * 4 /3;
        rImgBufInfo.rRawBufInfo.rRawImgInfo.u2Width  =  capW + paddW ; 
        rImgBufInfo.rRawBufInfo.rRawImgInfo.u2Height = capH + paddH; 
        rImgBufInfo.rRawBufInfo.rRawImgInfo.uBitDepth = 10; 
        rImgBufInfo.rRawBufInfo.bPacked  = TRUE;            
        rImgBufInfo.rRawBufInfo.rRawImgInfo.eColorOrder = (eCOLOR_ORDER)colorOrder; 

        MUINT8 *pRawBuf = (MUINT8 *)malloc (rImgBufInfo.rRawBufInfo.u4ImgSize * sizeof(UCHAR)); 

        FILE *fp = fopen((char *)mmHalCamParam.u1FileName, "rb"); 

        if (NULL == fp || NULL == pRawBuf) {
            ACDK_LOGE("Fail to open RAW image \n"); 
        }
        else {
            fread(pRawBuf, rImgBufInfo.rRawBufInfo.u4ImgSize, 1, fp); 
            fclose(fp); 
            rImgBufInfo.rRawBufInfo.pucRawBuf = pRawBuf;             
            capCb(&rImgBufInfo); 
        }
        if (pRawBuf != NULL) {
            free(pRawBuf); 
        }
    }

	ACDK_LOGD("[takePicture]  QQ fopen done \n");

    //save image 
    if (isSaveImg) {
       //save raw / bmq type 
       if(imgType & RAW_TYPE ) {                
            MUINT8 *pRawBuf = (MUINT8 *)malloc (  ( capW + paddW) * ( capH + paddH) * 4 /3 * sizeof(UCHAR)); 
            FILE *fp = fopen((char *)mmHalCamParam.u1FileName, "rb"); 

            if (NULL == fp || NULL == pRawBuf) {
                ACDK_LOGE("Fail to open RAW image \n"); 
            }
            else {
                fread(pRawBuf, ( capW + paddW ) * ( capH + paddH) * 4 /3, 1, fp); 
                fclose(fp); 
                //Save RAW Image 
               RAWBufInfo rRawBufInfo;             
               rRawBufInfo.pucRawBuf = pRawBuf; 
               //rRawBufInfo.rRawImgInfo.eColorOrder= (eCOLOR_ORDER)0; //m_rSensorInfo.eColorOrder;
               rRawBufInfo.rRawImgInfo.u2Width = capW + paddW; 
               rRawBufInfo.rRawImgInfo.u2Height =  capH + paddH; 
               rRawBufInfo.u4ImgSize =( capW + paddW ) * ( capH + paddH) * 4 /3 ; 
               rRawBufInfo.rRawImgInfo.uBitDepth = 10; 
               rRawBufInfo.bPacked = TRUE;   
               rRawBufInfo.rRawImgInfo.eColorOrder = (eCOLOR_ORDER)colorOrder; 

               if (imgType & RAW_TYPE) {
                    err = saveRAWImg(rRawBufInfo);
                    if (err != S_ACDK_CAMCTRL_OK) {
                        ACDK_LOGE("[takePicture] Save Raw File fail err = 0x%x\n", err); 
                   }        
               }
           }

           if (pRawBuf != NULL) {
                free(pRawBuf); 
          }
     }
	   
	 ACDK_LOGD("[takePicture] QQ save image  done \n");

     //save jpeg file 
     if (imgType & JPEG_TYPE) {
         err = saveJPEGImg((MUINT8*)mmHalCamParam.frmJpg.virtAddr, mEncodeLen);
         if (err != 0) {
                 ACDK_LOGE("[takePicture] Save JPEG File fail err = 0x%x\n", err); 
         }        
      }    
    }   
    ACDK_LOGD("[takePicture] QQ save jpeg  done \n");
		 
    mImgCnt++; 

    ACDK_LOGD("[takePicture] QQ mHalCamCaptureUninit done \n");

    err = mmHalCam4CCTObj->mHalCamCaptureUninit(NULL, 0);
	ACDK_LOGD("[takePicture] QQ mHalCamCaptureUninit done \n");
		
    if (err != 0) {
        ACDK_LOGE("[takePicture],mHalCamCaptureUninit, err = 0x%x \n", err);
        return ACDKCCTCTRL_API_FAIL;
    }

    //startPreview(mPreviewCB); 
    return ACDKCCTCTRL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::getFrameCnt(MUINT32 &frameCnt) 
{
    frameCnt = mFrameCnt; 
    return ACDKCCTCTRL_NO_ERROR; 
}

/*******************************************************************************
*
********************************************************************************/
MRESULT AppCCTCtrl::quickViewImg(MUINT8 *pInSrcFile, Func_CB quickViewCB)
{
    MINT32 err = 0; 
    FILE *fp = 0;
    unsigned char *src_buf = NULL; 
    ACDK_LOGD("[quickViewImg] E \n");

    unsigned int size = mmHalCamParam.frmQv.w * mmHalCamParam.frmQv.h * 4; 
    fp = fopen((char *)"/data/ftmqv.raw", "rb"); 
    if(fp == NULL) {
        ACDK_LOGE("[mrQuickViewImg] Can't open src file:%s\n", pInSrcFile);  
        goto Deinit;
    }

    src_buf = (unsigned char *) malloc (size * sizeof(unsigned char)); 
    if (src_buf == NULL) {
        ACDK_LOGD("No Enough Memory \n"); 
        fclose(fp); 
        goto Deinit; 
    }
    fread(src_buf, 1, size, fp); 
    fclose(fp); 
    
    mAcdkSurfaceViewObj->resetLayer(0); 

    err = mAcdkSurfaceViewObj->setOverlayInfo(
                               0, 
                               mPrvStartX, 
                               mPrvStartY,
                               mPrvWidth, 
                               mPrvHeight,
                               mSurfacePhyAddr,  
                               (MUINT32)mSurfaceBuf,
                               mOrientation
                               );                      
    if (err != 0) {
        ACDK_LOGE("[handlePreviewCB] setOverlayInfo fail err = 0x%x\n", err); 
        return ACDKCCTCTRL_API_FAIL; 
    }

    //! FIXME, the mOrientation should depend on screen and sensor 
    err = mAcdkSurfaceViewObj->setOverlayBuf(
                               0,
                               (MUINT8*)src_buf, 
                               (UINT32)MHAL_FORMAT_ABGR_8888, 
                               mmHalCamParam.frmQv.w, 
                               mmHalCamParam.frmQv.h,
                               mOrientation, 
                               mSensorHFlip, 
                               mSensorVFlip
                               );


    mAcdkSurfaceViewObj->refresh(); 


Deinit:
    if (src_buf != NULL) {
        free(src_buf); 
    }

    if (quickViewCB != NULL) {
            quickViewCB(NULL);
    }
    return S_ACDK_CAMCTRL_OK;
}

#if 0
MRESULT AppCCTCtrl::quickViewImg(MUINT8 *pInSrcFile, Func_CB quickViewCB)
{
    MINT32 err = 0; 
    FILE *fp = 0;
    ACDK_LOGD("[quickViewImg] E \n");
    if (pInSrcFile == NULL) {
        ACDK_LOGE("Input src File is NULL "); 
        return ACDKCCTCTRL_API_FAIL; 
    }

    int lock_mode, out_size, i;
    bool m_result = true;
    unsigned char ch;
    unsigned char *src_buf = NULL; 
    unsigned int addr; 
    MHAL_JPEG_DEC_INFO_OUT outInfo;
    MHAL_JPEG_DEC_START_IN inParams;
    MHAL_JPEG_DEC_RESULT_ENUM decResult;    

    // lock jpeg decoder
    MHalLockParam_t modeParam;
    unsigned char *dst_addr = NULL;
    modeParam.mode = MHAL_MODE_JPEG_DECODE;
    modeParam.waitMilliSec = 0;
    modeParam.waitMode = 0;
    if(MHAL_NO_ERROR != mHalIoCtrl(MHAL_IOCTL_LOCK_RESOURCE, 
                                  (void *)&modeParam, sizeof(modeParam), 
                                  NULL, 0, NULL))
    {
        ACDK_LOGE("[mrQuickViewImg]MHAL Lock failed!!!\n");
        return ACDKCCTCTRL_API_FAIL;
    }    


    unsigned int len = 0; 
    fp = fopen((char *)pInSrcFile, "rb"); 
    if(fp == NULL) {
        ACDK_LOGE("[mrQuickViewImg] Can't open src file:%s\n", pInSrcFile);  
        goto Deinit;
    }

    fseek (fp, 0, SEEK_END); 
    len = ftell(fp); 
    fseek (fp, 0, SEEK_SET); 

    src_buf = (unsigned char *) malloc (len * sizeof(unsigned char)); 
    if (src_buf == NULL) {
        ACDK_LOGD("No Enough Memory \n"); 
        fclose(fp); 
        goto Deinit; 
    }
    fread(src_buf, 1, len, fp); 
    fclose(fp); 
    
    //parse file 
    if (MHAL_NO_ERROR != mHalIoCtrl(MHAL_IOCTL_JPEG_DEC_PARSER, (void *)src_buf, len, 
                                    NULL, 0, NULL)) {
        ACDK_LOGE("[mrQuickViewImg] ERROR : mHalIoCtrl() - MHAL_IOCTL_JPEG_DEC_PARSER\n"); 
        goto Deinit;
    }
      
   
    // get file dimension
    if(MHAL_NO_ERROR != mHalIoCtrl(MHAL_IOCTL_JPEG_DEC_GET_INFO, NULL, 0, 
                                   (void *)&outInfo, sizeof(outInfo), NULL))
    {
        ACDK_LOGE("[mrQuickViewImg] ERROR : mHalIoCtrl() - Jpeg Decoder get info fail\n");
        goto Deinit;
    }

    out_size =  outInfo.srcWidth * outInfo.srcHeight;
    out_size *= 3;

    //unsigned char dst_addr = (unsigned char *)malloc (out_size * sizeof(unsigned char)); 
    dst_addr = (unsigned char *)malloc (mPrvWidth * mPrvHeight * 2  * sizeof(unsigned char)); 

    inParams.timeout = 1000; // 1 secs;
    inParams.dstWidth = mPrvWidth;//outInfo.srcWidth;
    inParams.dstHeight = mPrvHeight; //outInfo.srcHeight;
    inParams.dstVirAddr = mImageBuf;
    inParams.dstPhysAddr = NULL;
    inParams.dstFormat = JPEG_OUT_FORMAT_RGB565;
    inParams.srcBuffer = NULL;
 
    // start decode
    if(MHAL_NO_ERROR != mHalIoCtrl(MHAL_IOCTL_JPEG_DEC_START, 
                                  (void *)&inParams, sizeof(inParams), 
                                  NULL, 0, NULL)) {
        ACDK_LOGE("[mrQuickViewImg] ERROR : mHalIoCtrl() - JPEG Decoder Start\n");
        goto Deinit;
    }

#if 0
    fp = fopen("/data/test.raw", "wb"); 
    fwrite(dst_addr, 1, m_u4PrvWidth * m_u4PrvHeight * 2, fp); 
    fclose(fp);         
#endif 

    mAcdkSurfaceViewObj->resetLayer(0); 

    err = mAcdkSurfaceViewObj->setOverlayInfo(
                               0, 
                               mPrvStartX, 
                               mPrvStartY,
                               mPrvWidth, 
                               mPrvHeight,
                               mSurfacePhyAddr,  
                               (MUINT32)mSurfaceBuf
                               );                      
    if (err != 0) {
        ACDK_LOGE("[handlePreviewCB] setOverlayInfo fail err = 0x%x\n", err); 
        return ACDKCCTCTRL_API_FAIL; 
    }

    //addr = (unsigned int)pmem_get_phys(mImgPmemId); 
    addr = (unsigned int)mImageBuf;

    //! FIXME, the mOrientation should depend on screen and sensor 
    err = mAcdkSurfaceViewObj->setOverlayBuf(
                               0,
                               (MUINT8*)addr, 
                               (UINT32)MHAL_FORMAT_RGB_565, 
                               mPrvWidth, 
                               mPrvHeight,
                               mOrientation
                               );


    mAcdkSurfaceViewObj->refresh(); 

    if (quickViewCB != NULL) {
            quickViewCB(NULL);
    }
       
Deinit:
    modeParam.mode = MHAL_MODE_JPEG_DECODE;
    modeParam.waitMilliSec = 0;
    modeParam.waitMode = 0;
    if(MHAL_NO_ERROR != mHalIoCtrl(MHAL_IOCTL_UNLOCK_RESOURCE, 
                                  (void *)&modeParam, sizeof(modeParam), 
                                   NULL, 0, NULL))
    {
        ACDK_LOGE("[mrQuickViewImg]ERROR : mHalIoCtrl() - JPEG Decoder UnLock\n");
    }

    if(fp != NULL)
    {
        fclose(fp);
    }

    if (dst_addr != NULL)
    {
        free(dst_addr); 
    }
    return S_ACDK_CAMCTRL_OK;
}
#endif 

    
/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::setSrcDev(MINT32 srcDev)
{
    ACDK_LOGD("[setSrcDev] - E \n"); 

    //
#if 0
    if (m_eCameraState != CAMERA_IDLE) {
        mrStopPreview(); 
    }
#endif 

    MINT32 err = 0; 
    //
    ACDK_LOGD("[setSrcDev], new %d, old: %d \n", srcDev, mSrcDev);
    mSrcDev = srcDev;

#if 0
    if ((srcDev) != 0 && (srcDev != mSrcDev)) {
        err = mmHalCam4CCTObj->mHalCamSendCommand(CAM_CMD_SET_SENSOR_DEV, mSrcDev);    
        if (err != 0) {
            ACDK_LOGE("[setSrcDev] CAM_CMD_SET_SENSOR_DEV fail err = 0x%x\n", err); 
        }
    }  
#endif   
   
    return ACDKCCTCTRL_NO_ERROR;
}


/******************************************************************************
*
*******************************************************************************/
void
AppCCTCtrl::
cameraCallback(void* param)
{
    mHalCamCBInfo*const pCBInfo = reinterpret_cast<mHalCamCBInfo*>(param);
    if  ( ! pCBInfo ) {
        ACDK_LOGE("[AppCCTCtrl::cameraCallback] NULL pCBInfo");
        return;
    }
    //
    AppCCTCtrl*const _This = reinterpret_cast<AppCCTCtrl*>(pCBInfo->mCookie);
    if  ( ! _This ) {
        ACDK_LOGE("[AppCCTCtrl::cameraCallback] NULL AppCCTCtrl");
        return;
    }
    //
    mhalCamCallbackParam_t mHalCamCbParam;
    mHalCamCbParam.type  = pCBInfo->mType;
    mHalCamCbParam.pdata = pCBInfo->mpData;
//[TODO]    pCBInfo->mDataSize;
    //
    _This->dispatchCallback(&mHalCamCbParam);
}


/*******************************************************************************
*
********************************************************************************/
void AppCCTCtrl::dispatchCallback(void *param)
{
    mhalCamCallbackParam_t *pcbParam = (mhalCamCallbackParam_t *) param;

    switch (pcbParam->type) {
        case MHAL_CAM_CB_PREVIEW:            
            handlePreviewCB(pcbParam);
            break; 
        case MHAL_CAM_CB_AF:
            ACDK_LOGD("[cameraCallback] AF Callback \n"); 
            handleAFCB(pcbParam); 
            break;         
        case MHAL_CAM_CB_JPEG:
            ACDK_LOGD("[cameraCallback] JPEG Callback \n"); 
            handleCapCB(pcbParam); 
            break; 
        case MHAL_CAM_CB_RAW: 
            handleQVCB(pcbParam); 
            break; 
            
    }

}

/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::handleQVCB(void *param)
{
    ACDK_LOGD("[handleQVCB] - E \n"); 
    char szFileName[256]; 

    sprintf(szFileName, "%s//ftmqv.raw" , MEDIA_PATH);
     
    FILE *pFp = fopen(szFileName, "wb");
 
    if (NULL == pFp ) {
        ACDK_LOGD("Can't open file to save Image\n"); 
        return E_ACDK_CAMCTRL_OPEN_FAIL; 
    }

    MINT32 i4WriteCnt = fwrite((MUINT8*)mmHalCamParam.frmQv.virtAddr, 1, mmHalCamParam.frmQv.w * mmHalCamParam.frmQv.h * 4 , pFp);

    ACDK_LOGD("Save image file name:%s, w: %d, h: %d\n", szFileName, mmHalCamParam.frmQv.w, mmHalCamParam.frmQv.h); 

    fclose(pFp);

    ACDK_LOGD("handleQVCB() - X \n"); 
    return S_ACDK_CAMCTRL_OK;        
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::handleAFCB(void *param)
{
    mFocusDone = 1; 
    return ACDKCCTCTRL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::handlePreviewCB(void *param)
{
    //ACDK_LOGD("[previewCallback] E\n"); 
    UINT8 * pphybufin = NULL;
    UINT8 * pphybufout = NULL;
    UINT32 displayNo;
    mhalCamCallbackParam_t *pcbParam = (mhalCamCallbackParam_t *) param;
    mhalCamBufCBInfo *pcbInfo = (mhalCamBufCBInfo *) pcbParam->pdata;  
   
    displayNo = pcbInfo->u4BufIndex;
    mFrameCnt++;

#if 0
    ACDK_LOGD("[vPreviewCallback]frmYuv.w:%d\n",  mmHalCamParam.frmYuv.w);
    ACDK_LOGD("[vPreviewCallback]frmYuv.y:%d\n",  mmHalCamParam.frmYuv.h);
    ACDK_LOGD("[vPreviewCallback]frmYuv.frmCount:%d\n",  mmHalCamParam.frmYuv.frmCount);
    ACDK_LOGD("[vPreviewCallback]frmYuv.frmSize:%d\n",  mmHalCamParam.frmYuv.frmSize);
    ACDK_LOGD("[vPreviewCallback]frmYuv.bufSize:%d\n",  mmHalCamParam.frmYuv.bufSize);
    ACDK_LOGD("[vPreviewCallback]frmYuv.virtAddr:0x%x\n",  mmHalCamParam.frmYuv.virtAddr);
    ACDK_LOGD("[vPreviewCallback]frmYuv.phyAddr:0x%x\n",  mmHalCamParam.frmYuv.phyAddr);
    ACDK_LOGD("[vPreviewCallback]displayNo:%d \n", displayNo);
#endif

#if defined(MTK_M4U_SUPPORT)
    pphybufin = (UINT8 *)(mmHalCamParam.frmYuv.virtAddr + displayNo * mmHalCamParam.frmYuv.frmSize);
#else
        pphybufin = (UINT8 *)(mmHalCamParam.frmYuv.phyAddr + displayNo * mmHalCamParam.frmYuv.frmSize);
#endif
    MINT32 fbNo = mAcdkSurfaceViewObj->getFBNo(); 

    MINT32 err; 
#if 0    
    err = mAcdkSurfaceViewObj->setOverlayInfo(
                               0, 
                               mPrvStartX, 
                               mPrvStartY,
                               mPrvWidth, 
                               mPrvHeight,
                               mSurfacePhyAddr + (mPrvWidth * mPrvHeight * 2) * (fbNo), 
                               (MUINT32)mSurfaceBuf + (mPrvWidth * mPrvHeight * 2) * (fbNo)
                               );                      
#else                               
#if 0 
    err = mAcdkSurfaceViewObj->setOverlayInfo(
                               0, 
                               mPrvStartX, 
                               mPrvStartY,
                               mPrvWidth, 
                               mPrvHeight,
                               mmHalCamParam.frmYuv.phyAddr,                                    //mSurfacePhyAddr + (mPrvWidth * mPrvHeight * 2)  , 
                               (MUINT32)mmHalCamParam.frmYuv.virtAddr                       //mSurfaceBuf + (mPrvWidth * mPrvHeight * 2) 
                               );                      
#else 
    err = mAcdkSurfaceViewObj->setOverlayInfo(
                               0, 
                               mPrvStartX, 
                               mPrvStartY,
                               mPrvWidth, 
                               mPrvHeight,
                               mSurfacePhyAddr + (mPrvWidth * mPrvHeight * 2) * (fbNo), 
                               (MUINT32)mSurfaceBuf + (mPrvWidth * mPrvHeight * 2) * (fbNo), 
                               mOrientation
                              );                      
#endif 
                              
#endif                                
    if (err != 0) {
        ACDK_LOGE("[handlePreviewCB] setOverlayInfo fail err = 0x%x\n", err); 
        return ACDKCCTCTRL_API_FAIL; 
    }

#if 1 
    //! FIXME, the mOrientation should depend on screen and sensor 
    err = mAcdkSurfaceViewObj->setOverlayBuf(
                               0,
                               pphybufin, 
                               (UINT32)(UINT32)MHAL_FORMAT_YUV_420,
                               mmHalCamParam.frmYuv.w, 
                               mmHalCamParam.frmYuv.h,
                               mOrientation, 
                               mSensorHFlip, 
                               mSensorVFlip
                               );
#endif                                

    //
    if (err != 0) {
        ACDK_LOGE("[handlePreviewCB] setOverlayBuf fail err = 0x%x\n", err); 
        return ACDKCCTCTRL_API_FAIL; 
    }
    
#if 1     
    //
    err = mAcdkSurfaceViewObj->refresh();  
    if (err != 0) {
        ACDK_LOGE("[handlePreviewCB] refresh fail err = 0x%x\n", err); 
        return ACDKCCTCTRL_API_FAIL; 
    }
#endif 
    //
#if 0 
    if (mFrameCnt == 30) {
        FILE *fp = fopen("//data//preview.raw", "a+b"); 
        fwrite((char *)mmHalCamParam.frmYuv.virtAddr , 1, mmHalCamParam.frmYuv.w* mmHalCamParam.frmYuv.h *2, fp); 
        fclose(fp); 
    }
#endif 
		//sendcommand(1,NULL,0,NULL,0,NULL);
    return ACDKCCTCTRL_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::saveJPEGImg(MUINT8 *a_pBuf,  MUINT32 a_u4Size)
{
    ACDK_LOGD("[saveJPEGImg] - E \n"); 
    char szFileName[256]; 

    sprintf(szFileName, "%s//%04d_%s.jpg" , MEDIA_PATH, mImgCnt, PROJECT_NAME);
     
    FILE *pFp = fopen(szFileName, "wb");
 
    if (NULL == pFp ) {
        ACDK_LOGD("Can't open file to save Image\n"); 
        return E_ACDK_CAMCTRL_OPEN_FAIL; 
    }

    MINT32 i4WriteCnt = fwrite(a_pBuf, 1, a_u4Size , pFp);

    ACDK_LOGD("Save image file name:%s\n", szFileName); 

    fclose(pFp);

    ACDK_LOGD("mrSaveJPEGImg() - X \n"); 
    return S_ACDK_CAMCTRL_OK;     
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::saveRAWImg(const RAWBufInfo &a_pRawBufInfo)
{
    ACDK_LOGD("[saveRAWImg] - E \n"); 
    char szFileName[256]; 

    MUINT8 uBayerStart = 0; 

    sprintf(szFileName, "%s//%04d_%s_%dx%d_%d_%d.raw" , MEDIA_PATH, 
                                                          mImgCnt,
                                                          PROJECT_NAME,
                                                          a_pRawBufInfo.rRawImgInfo.u2Width, 
                                                          a_pRawBufInfo.rRawImgInfo.u2Height, 
                                                          a_pRawBufInfo.rRawImgInfo.uBitDepth, 
                                                          (MINT32)a_pRawBufInfo.rRawImgInfo.eColorOrder);

    MUINT32 *pu4SrcBuf = (MUINT32 *) a_pRawBufInfo.pucRawBuf; 
    MUINT8 *pucBuf = (MUINT8 *) malloc (a_pRawBufInfo.rRawImgInfo.u2Width * a_pRawBufInfo.rRawImgInfo.u2Height  * 2 * sizeof(MUINT8)); 
    MUINT16 *pu2DestBuf = (MUINT16 *)pucBuf;

    if (a_pRawBufInfo.bPacked == TRUE) {
        while (pu2DestBuf < (MUINT16 *)pucBuf + a_pRawBufInfo.rRawImgInfo.u2Width * a_pRawBufInfo.rRawImgInfo.u2Height) {
            MUINT32 u4Pixel = *(pu4SrcBuf++);
            *(pu2DestBuf++) = (MUINT16)(u4Pixel & 0x03FF);
            *(pu2DestBuf++) = (MUINT16)((u4Pixel >> 10) & 0x03FF); 
            *(pu2DestBuf++) = (MUINT16)((u4Pixel >> 20) & 0x03FF);
        }
    }
    else {
        memcpy(pu2DestBuf, a_pRawBufInfo.pucRawBuf, a_pRawBufInfo.rRawImgInfo.u2Width * a_pRawBufInfo.rRawImgInfo.u2Height  * 2);
    }
    
    FILE *pFp = fopen(szFileName, "wb");
 
    if (NULL == pFp ) {
        ACDK_LOGD("Can't open file to save Image\n"); 
        return E_ACDK_CAMCTRL_OPEN_FAIL; 
    }

    MINT32 i4WriteCnt = fwrite(pucBuf, 1, a_pRawBufInfo.rRawImgInfo.u2Width * a_pRawBufInfo.rRawImgInfo.u2Height  * 2 , pFp);		

    ACDK_LOGD("Save image file name:%s\n", szFileName); 

    fclose(pFp);
    free(pucBuf);

    ACDK_LOGD("mrSaveRAWImg() - X \n"); 
    return S_ACDK_CAMCTRL_OK;     
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AppCCTCtrl::sendcommand(
                MUINT32 const a_u4Ioctl,
                MUINT8 *puParaIn,
                MUINT32 const u4ParaInLen,
                MUINT8 *puParaOut,
                MUINT32 const u4ParaOutLen,
                MUINT32 *pu4RealParaOutLen
)
{ 
    MINT32 err = 0; 
#if 0
//ACDK_LOGD("[ACDK_CCT_OP_FLASH_CONTROL]: a_u4Ioctl, ACDK_CCT_OP_FLASH_CONTROL, ACDK_CCT_OP_FLASH_GET_INFO: %x, %x\n", a_u4Ioctl, ACDK_CCT_OP_FLASH_GET_INFO, ACDK_CCT_OP_FLASH_CONTROL);

if (a_u4Ioctl == ACDK_CCT_OP_FLASH_CONTROL)
	{
    ACDK_LOGD("[ACDK_CCT_OP_FLASH_CONTROL]\n");
    //ACDK_FLASH_CONTROL *pflashCtrl = (ACDK_FLASH_CONTROL *)a_pflashCtrl;

	m_pStrobeDrvObj = StrobeDrv::createInstance();
	err = m_pStrobeDrvObj->init(1);
	if (err < 0)
	    ACDK_LOGE("init error");
	if (m_pStrobeDrvObj) {
        m_pStrobeDrvObj->setState(0); //preview state
    }

	if (m_pStrobeDrvObj->getFlashlightType() == StrobeDrv::FLASHLIGHT_NONE) {
        m_pStrobeDrvObj->destroyInstance();
		m_pStrobeDrvObj = NULL;
		ACDK_LOGE("m_pStrobeDrvObj = NULL");
	}

    for (int i = 0; i < 1; i++) {
        //ACDK_LOGD("[FLASH A]: %d\n",i);
        err = m_pStrobeDrvObj->setLevel(20);
        //ACDK_LOGE("setlevel error: %d\n", err);
        //ACDK_LOGD("[FLASH B]");
        m_pStrobeDrvObj->setFire(1);
        //ACDK_LOGD("[FLASH C");
        usleep(1000);
        m_pStrobeDrvObj->setFire(0);
        //ACDK_LOGD("[FLASH D");
        //usleep(1000);
    }
    if (m_pStrobeDrvObj) {
        m_pStrobeDrvObj->destroyInstance();
		    m_pStrobeDrvObj = NULL;
    }
}
#endif 

#if (CCT_SUPPORT)        	
    if (a_u4Ioctl >= CCT_ISP_FEATURE_START && a_u4Ioctl < CCT_ISP_FEATURE_START + MAX_SUPPORT_CMD) {
        err = mCCTIFObj->ispCCTFeatureControl(a_u4Ioctl
                                                , puParaIn
                                                , u4ParaInLen
                                                , puParaOut
                                                , u4ParaOutLen
                                                , pu4RealParaOutLen);
    }
    else if (a_u4Ioctl >= CCT_SENSOR_FEATURE_START && a_u4Ioctl < CCT_SENSOR_FEATURE_START + MAX_SUPPORT_CMD) {
        err = mCCTIFObj->sensorCCTFeatureControl(a_u4Ioctl
                                                   , puParaIn
                                                   , u4ParaInLen
                                                   , puParaOut
                                                   , u4ParaOutLen
                                                   , pu4RealParaOutLen);      
    }
    else if (a_u4Ioctl >= CCT_NVRAM_FEATURE_START && a_u4Ioctl < CCT_NVRAM_FEATURE_START + MAX_SUPPORT_CMD) {
        err = mCCTIFObj->nvramCCTFeatureControl(a_u4Ioctl
                                                    , puParaIn
                                                    , u4ParaInLen
                                                    , puParaOut
                                                    , u4ParaOutLen
                                                    , pu4RealParaOutLen);       
    }
    else if (a_u4Ioctl >= CCT_3A_FEATURE_START && a_u4Ioctl < CCT_3A_FEATURE_START + MAX_SUPPORT_CMD)  {
        err = mCCTIFObj->aaaCCTFeatureControl(a_u4Ioctl
                                                  , puParaIn
                                                  , u4ParaInLen
                                                  , puParaOut
                                                  , u4ParaOutLen
                                                  , pu4RealParaOutLen);           
    }   
#endif     
    if (a_u4Ioctl == ACDK_CCT_FEATURE_QUICK_VIEW_IMAGE){
        err = quickViewImg((MUINT8*)puParaIn, (Func_CB)puParaOut);
    }
    else if (a_u4Ioctl == ACDK_CCT_FEATURE_RESET_LAYER_BUFFER) {
        err = mAcdkSurfaceViewObj->resetLayer(0); 
    }
    else if (a_u4Ioctl == ACDK_CCT_FEATURE_SET_SRC_DEV) {
        MUINT8 *pSrcDev = (MUINT8*)puParaIn;
        err = setSrcDev((MINT32)(*pSrcDev));
    }
    else if (a_u4Ioctl == ACDK_CCT_FEATURE_SET_OPERATION_MODE) {
        err = mmHalCam4CCTObj->mHalCamSetOperationMode(reinterpret_cast<MUINT32>(puParaIn));
    }
    else if (a_u4Ioctl == ACDK_CCT_OP_PREVIEW_LCD_START) {
        PACDK_CCT_CAMERA_PREVIEW_STRUCT pCamPreview = (ACDK_CCT_CAMERA_PREVIEW_STRUCT *)puParaIn; 
        ACDK_LOGD("CCTOPPreviewStart \n"); 
        ACDK_LOGD("Preview Width:%d\n", pCamPreview->u2PreviewWidth); 
        ACDK_LOGD("Preview Height:%d\n", pCamPreview->u2PreviewHeight); 
        err = startPreview(pCamPreview->fpPrvCB);             
    }
    else if (a_u4Ioctl == ACDK_CCT_OP_PREVIEW_LCD_STOP) {
        ACDK_LOGD("stopPreview \n");  
        err = stopPreview();
    }
    else if (a_u4Ioctl == ACDK_CCT_OP_SINGLE_SHOT_CAPTURE_EX) {
        PACDK_CCT_STILL_CAPTURE_STRUCT pStillCapConfig = (ACDK_CCT_STILL_CAPTURE_STRUCT *)puParaIn; 

        ACDK_LOGD("CCTOPCap Start \n");
        ACDK_LOGD("Mode:%d\n", pStillCapConfig->eCameraMode); 
        ACDK_LOGD("Format:%d\n", pStillCapConfig->eOutputFormat); 
        ACDK_LOGD("Width:%d\n", pStillCapConfig->u2JPEGEncWidth); 
        ACDK_LOGD("Height:%d\n", pStillCapConfig->u2JPEGEncHeight); 
		ACDK_LOGD("Callback:%d\n", pStillCapConfig->fpCapCB); 
		
        if (pStillCapConfig->eOutputFormat == OUTPUT_JPEG) {
           err =  takePicture (pStillCapConfig->eCameraMode,
                               JPEG_TYPE, 
                               pStillCapConfig->fpCapCB,
                               pStillCapConfig->u2JPEGEncWidth,
                               pStillCapConfig->u2JPEGEncHeight
                              );
		           ACDK_LOGD("CCTOPCap OUTPUT_JPEG \n");
        }
        else if (pStillCapConfig->eOutputFormat == OUTPUT_EXT_RAW_10BITS){
            err = takePicture(pStillCapConfig->eCameraMode, 
                                RAW_TYPE, 
                                pStillCapConfig->fpCapCB
                               );    
			        ACDK_LOGD("CCTOPCap OUTPUT_EXT_RAW_10BITS \n");
        }
        else if (pStillCapConfig->eOutputFormat == JPEG_TYPE) {
           err =  takePicture (pStillCapConfig->eCameraMode,
                               JPEG_TYPE, 
                               pStillCapConfig->fpCapCB,
                               pStillCapConfig->u2JPEGEncWidth,
                               pStillCapConfig->u2JPEGEncHeight
                              );
		           ACDK_LOGD("CCTOPCap OUTPUT_JPEG \n");
        }		
        else {
            ACDK_LOGE("No Support Format \n"); 
            err = ACDKCCTCTRL_API_FAIL; 
        }   
		
        ACDK_LOGD("CCTOPCap Done \n");
    }
    else if (a_u4Ioctl == ACDK_CCT_OP_MULTI_SHOT_CAPTURE_EX) {
        PACDK_CCT_MULTI_SHOT_CAPTURE_STRUCT pMultiCapConfig = (ACDK_CCT_MULTI_SHOT_CAPTURE_STRUCT *)puParaIn; 

        ACDK_LOGD("CCTOPCap Start \n")
        ACDK_LOGD("Mode:%d\n", pMultiCapConfig->eCameraMode); 
        ACDK_LOGD("Format:%d\n", pMultiCapConfig->eOutputFormat); 
        ACDK_LOGD("Width:%d\n", pMultiCapConfig->u2JPEGEncWidth); 
        ACDK_LOGD("Height:%d\n", pMultiCapConfig->u2JPEGEncHeight); 
        ACDK_LOGD("Count:%d\n", pMultiCapConfig->u4CapCount); 
    
        MINT32 err = 0;
        for (UINT32 i = 0 ; i < pMultiCapConfig->u4CapCount; i++) {
            if (pMultiCapConfig->eOutputFormat == OUTPUT_JPEG) {
               err =  takePicture( pMultiCapConfig->eCameraMode,
                                   JPEG_TYPE, 
                                   pMultiCapConfig->fpCapCB
                                 );

            }
            else if (pMultiCapConfig->eOutputFormat == OUTPUT_EXT_RAW_10BITS){
                err = takePicture(pMultiCapConfig->eCameraMode, 
                                  RAW_TYPE, 
                                  pMultiCapConfig->fpCapCB
                                 );
            }
            else {
                ACDK_LOGE("No Support Format \n"); 
                err = ACDKCCTCTRL_API_FAIL; 
            }
        }
    }
    else if (a_u4Ioctl == ACDK_CCT_OP_SWITCH_CAMERA) {
        MUINT32 *pSrcDev = reinterpret_cast<MUINT32 *>(puParaIn);

        ACDK_LOGD("Switch Camera: Set Src = %d\n", *pSrcDev);

        if ((*pSrcDev == 0) || (*pSrcDev == 1)) {

            err = stopPreview();
            if (err != 0) {
                ACDK_LOGE("[stopPreview] err = 0x%x\n", err);
            }

            err = uninit();
            if (err != 0) {
                ACDK_LOGE("[uninit] err = 0x%x\n", err);
            }

            err = setSrcDev(*pSrcDev);
            if (err != 0) {
                ACDK_LOGE("[setSrcDev] err = 0x%x\n", err);
            }

            err = init();
            if (err != 0) {
                ACDK_LOGE("[init] err = 0x%x\n", err);
            }

            err = startPreview(NULL);
            if (err != 0) {
                ACDK_LOGE("[startPreview] err = 0x%x\n", err);
            }
        }
    }

    return err; 
} 
