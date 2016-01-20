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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "mHalCam"

#include <cutils/xlog.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <linux/rtpm_prio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>

#include "Mhal_MAV.h"
#include "mhal_cam.h"
#include "ICameraIO.h"
#include "MpoEncoder.h"
#include "jpeg_hal.h"
#include "mhal_misc.h"
#include "CamJpg.h"


#if (MAV_PROFILE_CAPTURE)
    MyDbgTimer DbgTmr("MAV capture");
#endif

/*******************************************************************************
*
*******************************************************************************/

static pthread_t MAVFuncThread;
static sem_t MAVSemThread, MAVmergeDone;
static mhal_MAV *mhalMAVObj;
//static MBOOL     mRelease;

#define Optimization 1  // Turning on/off needs sync with 
#define errorTest 0     // Test error handling

#if errorTest
MBOOL ErrorTest();
#endif

/*******************************************************************************
*
*******************************************************************************/
mhal_MAV*
mhal_MAV::
createInstance(NSCamera::ESensorType const eSensorType)
{
    return new mhal_MAV(eSensorType);
}


/*******************************************************************************
*
*******************************************************************************/
MVOID
mhal_MAV::
destroyInstance()
{
    MY_DBG("destroyInstance");    

    mpEISHalObj = NULL;
    mpHal3AObj = NULL;
    mpMhalCam = NULL;
    mhalMAVObj = NULL;

    delete this;
}


/*******************************************************************************
*
********************************************************************************/

mhal_MAV::mhal_MAV(NSCamera::ESensorType const eSensorType)
    : mhal_featureBase("MAV")
    , meSensorType(eSensorType)
    , mpMAVWorkingBuf(NULL)
    , mJPGFrameAddr(NULL) 
    , mRelease(false)    
    , mpEISHalObj(NULL)
    , mpHal3AObj(NULL)
    , mpMhalCam(NULL)
{
    MY_DBG("mhal_MAV constructor \n");
    
    ::memset(&mMAVFrame, 0, sizeof(mhalCamFrame_t));    
    mhalMAVObj = NULL;
    setState(MHAL_CAM_FEATURE_STATE_IDLE);
}


/*******************************************************************************
*
********************************************************************************/
MINT32 
mhal_MAV::
init(
    mHalCam* const pMhalCam, 
    Hal3ABase* const pHal3AObj,
    ICameraIO* const pCameraIOObj, 
    mHalCamMemPool* const pFDWorkingBuf
)
{
    //
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - initial In");
    #endif
    if  ( ! pMhalCam )
    {
        MY_ERR("[mHalCamInitMAV] pHal3AObj==NULL");
        return  -1;
    }
    mpMhalCam = pMhalCam;

    //
    if  ( ! pHal3AObj )
    {
        MY_ERR("[mHalCamInitMAV] pHal3AObj==NULL");
        return  -1;
    }
    mpHal3AObj = pHal3AObj;    

    //
    if  ( ! pCameraIOObj )
    {
        MY_ERR("[mHalCamInitMAV] pCameraIOObj == NULL");
        return  -1;
    }
    mpCameraIOObj = pCameraIOObj;

    //
    if  ( ! pFDWorkingBuf )
    {
        MY_ERR("[mHalCamInitMAV] pFDWorkingBuf == NULL");
        return  -1;
    }
    mFDWorkingBuf = pFDWorkingBuf;

    //
    mhalMAVObj = this;

    //
    mpMAVObj = hal3DFBase::createInstance(HAL_MAV_OBJ_NORMAL);
    if ( ! mpMAVObj )
    {
        MY_ERR("[mHalCamInitMAV] mpMAVObj==NULL \n");
        return MHAL_INVALID_MEMORY;
    }

    
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - initial out");
    #endif
    
    return MHAL_NO_ERROR;
}

/******************************************************************************
*
*******************************************************************************/
MINT32
mhal_MAV::
getPriority(int policy)
{
    int priority;
    policy = sched_getscheduler(0);
    if (policy == SCHED_OTHER) {
        // a conventional process has only the static priority
        priority = getpriority(PRIO_PROCESS, 0);
    } else {
        // a real-time process has both the static priority and real-time priority.
        struct sched_param sched_p;
        sched_getparam(0, &sched_p);
        priority = sched_p.sched_priority;
    }

    return priority;
}

/******************************************************************************
*
*******************************************************************************/
MVOID 
mhal_MAV::
setPriority(int policy, int priority)
{
    struct sched_param sched_p;

    sched_getparam(0, &sched_p);
    if (policy == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        sched_setscheduler(0, policy, &sched_p);
        setpriority(PRIO_PROCESS, 0, priority); //  Note: "priority" is nice value.
        XLOGD("[setPriority] tid(%d) policy(SCHED_OTHER:%d) priority(%d)", gettid(), policy, priority);
    } else {
        sched_p.sched_priority = priority;      //  Note: "priority" is real-time priority.
        sched_setscheduler(0, policy, &sched_p);
        XLOGD("[setPriority] tid(%d) policy(Real-Time:%d) priority(%d)", gettid(), policy, priority);
    }
}


/*******************************************************************************
*
********************************************************************************/
MVOID* 
mhal_MAV::
MAVthreadFunc(void *arg)
{
    XLOGD("[MAV][MAVthreadFunc]");
	
    // set thread name
    ::prctl(PR_SET_NAME, "MAVthread", 0, 0, 0);

    // revise thread priority
    int priority;
    setPriority(SCHED_OTHER, -20);
    priority = getPriority(SCHED_OTHER);
    XLOGD("[getPriority] %d Schedule NORMAL priority %d!\n", gettid(), priority);

    // loop for thread until access uninit state
    while(!mhalMAVObj->mRelease)
    {
        XLOGD("[MAV][MAVthreadFunc]: wait thread");
        int SemValue;
        sem_getvalue(&MAVSemThread, &SemValue);
        XLOGD("Semaphone value: %d", SemValue);
        sem_wait(&MAVSemThread);
        MINT32 err = mhalMAVObj->mHalCamFeatureAddImg();       
        if (err != MHAL_NO_ERROR && mhalMAVObj->getState() != MHAL_CAM_FEATURE_STATE_UNINIT) {
            mhalMAVObj->ErrHandling(ERR_RESET);
        }
      
        XLOGD("[MAV][MAVthreadFunc]: after do merge");
    }
    
    if (mhalMAVObj->mpMAVObj) {
        mhalMAVObj->mpMAVObj->mHal3dfUninit();
        mhalMAVObj->mpMAVObj->destroyInstance();
        mhalMAVObj->mpMAVObj = NULL;
    }
    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_MAV::
setParameters(
    mhalCamParam_t *pmhalCamParam
)
{
    MY_DBG("[setParameters]");
    MINT32 err = MHAL_NO_ERROR;
    
    MAVnum = pmhalCamParam->u4BusrtNo;
    strcpy((char *) SaveFileName, (char *)pmhalCamParam->u1FileName);
    
    mMAVFrame.w = pmhalCamParam->frmYuv.w;
    mMAVFrame.h = pmhalCamParam->frmYuv.h;
    mMAVFrame.frmSize = mMAVFrame.w * mMAVFrame.h * 3 / 2;  // YV12 now
    mMAVFrame.frmCount = pmhalCamParam->u4BusrtNo;    // Maximum is 15
    mMAVFrame.bufSize = mMAVFrame.frmSize * mMAVFrame.frmCount;
    mMAVFrame.virtAddr = (MUINT32) malloc(mMAVFrame.bufSize);
    if ( mMAVFrame.virtAddr == 0)
    {
        err = MHAL_INVALID_MEMORY;
        MY_ERR("mMAVFrame.virtAddr == 0");
        return err;
    }
    frmYuv_virtAddr = pmhalCamParam->frmYuv.virtAddr;
    frmYuv_size = pmhalCamParam->frmYuv.frmSize;
    mpmhalCamParam = pmhalCamParam;

    mMAVaddImgIdx = 0;
    mMAVFrameIdx = 0;

    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 
mhal_MAV::
mHalCamFeatureInit(
    mhalCamParam_t *pmhalCamParam
)
{
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureInit in");
    #endif
    
    MY_DBG("[mHalCamMAVInit]");
    MINT32 err = MHAL_NO_ERROR;
    
    // (1) Set global variable
    err = setParameters(pmhalCamParam);
    if ( err != MHAL_NO_ERROR)
        return err;
    
    MY_DBG("[mHalCamMAVInit] %d/%d, %d", pmhalCamParam->frmYuv.w, pmhalCamParam->frmYuv.h, MAVnum);

    // (2) Create thread
    #if(Optimization)
    MY_DBG("[init] pthread create");
    setState(MHAL_CAM_FEATURE_STATE_IDLE);  // When going to MAVthreadFunc, state can't be 'UNINIT'
    sem_init(&MAVSemThread, 0, 0);
    sem_init(&MAVmergeDone, 0, 0);

    // (2.1) set "NORMAL" priority for MAV thread with high priority   
    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, RTPM_PRIO_CAMERA_COMPRESS};
    pthread_create(&MAVFuncThread, &attr, MAVthreadFunc, NULL);  
    #endif
    
    // (3) Create working buffer and JPEG buffer
    MINT32 initBufSize = mMAVFrame.w * mMAVFrame.h * 4 * 20;
    MINT32 motionBufSize = 320 * 240 * 3 * 2;
    MINT32 warpBufSize = mMAVFrame.w * mMAVFrame.h * 2 + 2048;
    MINT32 workingSize = initBufSize + motionBufSize + warpBufSize;

    mpMAVWorkingBuf = (MINT8 *) malloc(workingSize);
    if (mpMAVWorkingBuf == NULL) {
        MY_ERR("[mHalCamInitMAV] mpMAVWorkingBuf == NULL");
        //mHalCamFeatureUninit();
        return MHAL_INVALID_MEMORY;
    }

    //mJPGFrameAddr = (MUINT32) malloc(mMAVFrame.bufSize + mMAVFrame.frmSize);
    MUINT32 jpegBufsize = ((mMAVFrame.bufSize + mMAVFrame.frmSize) + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    mJPGFrameAddr = (MUINT32)memalign (L1_CACHE_BYTES, jpegBufsize);
    MY_DBG("bufsize: %d, virtAddr: %x,  L1_bytes: %d", jpegBufsize, mJPGFrameAddr, L1_CACHE_BYTES);
    if (mJPGFrameAddr == 0) {
        MY_ERR("[mHalCamInitMAV] mMAVFrame.virtAddr == NULL");
        //mHalCamFeatureUninit();
        return MHAL_INVALID_MEMORY;
    }

    // (4) Driver initialization
    err = mpMAVObj->mHal3dfInit(mpMAVWorkingBuf, mpMAVWorkingBuf + initBufSize, mpMAVWorkingBuf + initBufSize + motionBufSize, NULL);
    if ( err < 0 ) {
        MY_ERR("mpMAVObj->mHalMavinit() Err");
        return MHAL_INVALID_MEMORY;
    }

    // Start Capturing
    setState(MHAL_CAM_FEATURE_STATE_CAPTURE);
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureInit out");
    #endif
    return err;
}



/*******************************************************************************
*
********************************************************************************/
MVOID 
mhal_MAV::
mHalCamFeatureUninit()
{
    /* Mutex at 2 plases: Uninit() and Proc() */
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureUninit in");
    #endif
    Mutex::Autolock lock(mLockUninit);
    //
    MY_DBG("[mHalCamMAVUninit] \n");
    //
    if (mRelease) { 
        MY_DBG("already uninit, return!");
        return;
    }
    /* Stop preview thread go into Proc() after leaving Uninit()*/
    setState(MHAL_CAM_FEATURE_STATE_UNINIT);
    
    //
#if (Optimization)
    MY_DBG("[mHalCamMAVUninit]: wait MAV thead");
    
    /* Use flag instead of state machine to avoid being revised unexpectedly */
    mRelease = true; 
    
    /* Join MAV thread */
    sem_post(&MAVSemThread);
    pthread_join(MAVFuncThread, NULL);
    
    MY_DBG("[mHalCamMAVUninit]: got MAV thread");  
#endif

    /* Free all memory allocated in mhal_MAV.cpp */
    if (mpMAVWorkingBuf) {
        free(mpMAVWorkingBuf);
        mpMAVWorkingBuf = NULL;
    }

    if (mMAVFrame.virtAddr) {
        free((UINT8 *) mMAVFrame.virtAddr);
        mMAVFrame.virtAddr = 0;
    }

    if (mJPGFrameAddr) {
        free((UINT8 *) mJPGFrameAddr);
        mJPGFrameAddr = 0;
    }
    
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureUninit out");
    #endif
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
mhal_MAV::
ISShot(MVOID *arg1, MBOOL &shot)
{
    MINT32 err = MHAL_NO_ERROR;
    ICameraIO::BuffInfo_t pBuffInfo; 

    // Get FD frame    
    MY_DBG("start to get Preview");        
    err = mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_FD_PORT, &pBuffInfo);  
    if ( err != MHAL_NO_ERROR ) {
        MY_WARN("getPreviewFrame error");
        return err;
    }
    
    err = mpMAVObj->mHal3dfDoMotion((MUINT32 *)(mFDWorkingBuf->getVirtAddr()+ pBuffInfo.hwIndex * 320 * 240 *2), (MUINT32 *) arg1);
    if ( err != MHAL_NO_ERROR ) {
        MY_WARN("Do motion error");
        return err;
    }    
    
    err = mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_FD_PORT, &pBuffInfo);      
    if ( err != MHAL_NO_ERROR ) {
        MY_WARN("releasePreviewFrame error");
        return err;
    }  
    
    #if 0
    mHalMiscDumpToFile((char*) "/sdcard/DCIM/Camera/fd.raw", (UINT8*)mFDWorkingBuf->getVirtAddr(), mFDWorkingBuf->getPoolSize()); 
    #endif 
 
    
    shot = ((MAVMotionResultInfo*)arg1)->ReadyToShot > 0 ? true : false; 
    MY_DBG("isshot : %d", shot);
    
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_MAV::
mHalCamFeatureProc(
    MUINT32 frameIndex
)
{
    /*  Mutex at 2 plases: Uninit() and Proc() 
     *
     *  To Avoid the following scenario:
     *  Preview thread is inside Proc(). 
     *  AP thread is inside Uninit too, but unfortunalty has deleted some memory.
     */
    Mutex::Autolock lock(mLockUninit);

    MY_DBG("[mHalCamMAVProc]");
    MINT32 err = MHAL_NO_ERROR;
    MBOOL isShot = false;

    //
    if (getState() == MHAL_CAM_FEATURE_STATE_CAPTURE) {
    
        MAVMotionResultInfo mavResult;
        err = ISShot(&mavResult, isShot);
        
        if (isShot) {
            if (mMAVFrameIdx < MAVnum) {
                MY_DBG("[MAV]Num %d", mMAVFrameIdx);
                
                // Copy and convert preview frame to YV12 ( currently )
                MUINT8 *pbufIn = (MUINT8 *) (frmYuv_virtAddr + (frmYuv_size * frameIndex));
                MUINT8 *pbufOut = (MUINT8 *) (mMAVFrame.virtAddr + (mMAVFrame.frmSize * mMAVFrameIdx));
                memcpy(pbufOut, pbufIn, mMAVFrame.frmSize);

                mpMAVResult.ImageInfo[mMAVFrameIdx].MotionValue[0] = mavResult.MV_X;
                mpMAVResult.ImageInfo[mMAVFrameIdx].MotionValue[1] = mavResult.MV_Y;

                MY_DBG("mavResult.MV_X: %d, %d", mavResult.MV_X, mavResult.MV_Y);     
                #if 0
                // for test
                char sourceFiles[80];
                sprintf(sourceFiles, "%s%d.raw", "/sdcard/DCIM/Camera/getpreview", mMAVFrameIdx);
                mHalMiscDumpToFile((char *) sourceFiles, pbufOut , mMAVFrame.frmSize);
                #endif
                
                mpMhalCam->mHalCamCBHandle(MHAL_CAM_CB_MAV, NULL);
                mMAVFrameIdx++; 
                
                #if (Optimization)
                sem_post(&MAVSemThread);
                #else
                if (mMAVFrameIdx == MAVnum){
                    mHalCamFeatureMerge();
                }
                #endif
                if(mMAVFrameIdx >= MAVnum)
                     setState(MHAL_CAM_FEATURE_STATE_MERGE);
            }
            else {
                MY_DBG("[MAV]Num %d", mMAVFrameIdx);
                mMAVFrameIdx++; 
                setState(MHAL_CAM_FEATURE_STATE_MERGE);
            }
        }
    }  
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_MAV::mHalCamFeatureAddImg()
{
    MINT32 err = MHAL_NO_ERROR;
    
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureAddImg in");
    #endif
    
    if (mMAVaddImgIdx >= MAVnum || getState() == MHAL_CAM_FEATURE_STATE_UNINIT){
        return err;
    }

    MY_DBG("mHalCamMAVAddImg(): %d", mMAVaddImgIdx);

	MavPipeImageInfo ImageInfo;
    ImageInfo.ImgAddr = mMAVFrame.virtAddr + (mMAVFrame.frmSize * mMAVaddImgIdx);
    ImageInfo.Width = mMAVFrame.w;
    ImageInfo.Height = mMAVFrame.h;
    ImageInfo.ControlFlow = 0;
    
    ImageInfo.MotionValue[0] = mpMAVResult.ImageInfo[mMAVaddImgIdx].MotionValue[0];
    ImageInfo.MotionValue[1] = mpMAVResult.ImageInfo[mMAVaddImgIdx].MotionValue[1];

    mpMAVResult.ImageInfo[mMAVaddImgIdx].Width = mMAVFrame.w;
    mpMAVResult.ImageInfo[mMAVaddImgIdx].Height = mMAVFrame.h;
    mpMAVResult.ImageInfo[mMAVaddImgIdx].ImgAddr = ImageInfo.ImgAddr;

    MY_DBG("ImgAddr 0x%x, Width %d, Height %d, Motion: %d %d", 
             ImageInfo.ImgAddr, ImageInfo.Width, ImageInfo.Height,
             ImageInfo.MotionValue[0], ImageInfo.MotionValue[1]);

    err = mpMAVObj->mHal3dfAddImg((MavPipeImageInfo*)&ImageInfo);
    if (err != MHAL_NO_ERROR) {
        MY_ERR("mHal3dfAddImg Err");
        return err;
    }
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureAddImg Done");
    #endif
    
    mMAVaddImgIdx++;

    // Do merge
    if (mMAVaddImgIdx == MAVnum)
    {     
        err = mHalCamFeatureMerge();
        if (err != MHAL_NO_ERROR) return err;
        
        // Post semaphore to confirm MAV can be uninited
        setState(MHAL_CAM_FEATURE_STATE_MERGE_DONE);
        sem_post(&MAVmergeDone);
    }
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureAddImg out");
    #endif
    MY_DBG("mHalCamMAVAddImg X");
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_MAV::
mHalCamFeatureMerge()
{
    MY_DBG("mHalCamMAVdoMerge");
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureMerge in");
    #endif
    MINT32 err = MHAL_NO_ERROR;

#if (Optimization)

    MavPipeImageInfo ImageInfo;
    MUINT32 result;
    
    MY_DBG("mHalMavMerge");
    err = mpMAVObj->mHal3dfMerge((MUINT32*)&mpMAVResult);
    if ( err < 0 ) return err;
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureMerge mHal3dfMerge");
    #endif

    MY_DBG("mHalMavGetResult");
    err = mpMAVObj->mHal3dfGetResult(result);
    if ( err < 0 ) return err;
    MY_DBG("mHalMavGetResult result %d",result);
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureMerge mHal3dfGetResult");
    #endif

    ImageInfo.ImgAddr = mMAVFrame.virtAddr;
    err = mpMAVObj->mHal3dfWarp((MavPipeImageInfo*)&ImageInfo,(MUINT32*)&mpMAVResult, MAVnum);
    if ( err < 0 ) return err;  
    MY_DBG("mHalMavGetResult done");  
    
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureMerge mHal3dfWarp");
    #endif                 
#else
    // Do Add image
    MavPipeImageInfo ImageInfo;
    MUINT32 result;
    
    for(UINT8 i = 0; i < mMAVFrame.frmCount; i++)
    {
        ImageInfo.ImgAddr = mMAVFrame.virtAddr + (mMAVFrame.frmSize * i);
        ImageInfo.Width = mMAVFrame.w;
        ImageInfo.Height = mMAVFrame.h;
        mpMAVResult.ImageInfo[i].Width = mMAVFrame.w;
        mpMAVResult.ImageInfo[i].Height = mMAVFrame.h;
        mpMAVResult.ImageInfo[i].ImgAddr = ImageInfo.ImgAddr;
        ImageInfo.MotionValue[0] = mpMAVResult.ImageInfo[i].MotionValue[0];
        ImageInfo.MotionValue[1] = mpMAVResult.ImageInfo[i].MotionValue[1];
        err = mpMAVObj->mHal3dfAddImg((MavPipeImageInfo*)&ImageInfo);
        if ( err < 0 ) return err;
    }

    // Do Merge
    MY_DBG("mHalMavMerge");
    err = mpMAVObj->mHal3dfMerge((MUINT32*)&mpMAVResult);
    if ( err < 0 ) return err;
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureMerge mHal3dfMerge");
    #endif
    
    MY_DBG("mHalMavGetResult");
    err = mpMAVObj->mHal3dfGetResult(result);
    if ( err < 0 ) return err;
    MY_DBG("mHalMavGetResult result %d",result);
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureMerge mHal3dfGetResult");
    #endif

    ImageInfo.ImgAddr = mMAVFrame.virtAddr;
    err = mpMAVObj->mHal3dfWarp((MavPipeImageInfo*)&ImageInfo,(MUINT32*)&mpMAVResult, MAVnum);
    if ( err < 0 ) return err;
    MY_DBG("mHalMavGetResult done");
    
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureMerge mHal3dfWarp");
    #endif
#endif

#if 0
    char sourceFiles[80];
    for (UINT8 i = 0; i < mMAVFrame.frmCount; i++)
    {
        sprintf(sourceFiles, "%s%d.raw", "/sdcard/DCIM/Camera/afterwarp", i);
        mHalMiscDumpToFile((char *) sourceFiles, (MUINT8 *)(mMAVFrame.virtAddr + (mMAVFrame.frmSize * i)), mMAVFrame.frmSize);
    }
#endif
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_MAV::
mHalCamFeatureCompress()
{
    MY_DBG("[mHalCamFeatureCompress]");

    MINT32 err = MHAL_NO_ERROR;
    
    // (1) confirm merge is done; so mutex is not necessary
    #if (Optimization)
    sem_wait(&MAVmergeDone);  
    MY_DBG("get MAVmergeDone semaphore");
    #endif
    Mutex::Autolock lock(mLockUninit);
    
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureCompress in");
    #endif

    // (2) Encode jpeg
    CamJpg camjpg(mpHal3AObj, mpMhalCam, mpmhalCamParam, meSensorType);

    FrameParam src(
        mpMAVResult.ClipWidth,
        mpMAVResult.ClipHeight,
        mMAVFrame.frmSize,
        0, //don't care
        JpgEncHal::kYV12_Planar_Format,
        mMAVFrame.virtAddr
    );
        
    FrameParam dst(
        mpMAVResult.ClipWidth,
        mpMAVResult.ClipHeight,
        0, // don't care
        mMAVFrame.frmSize,
        0, // don't care
        mJPGFrameAddr
    );
        
        
    MPImageInfo * pMPImageInfo = new MPImageInfo[MAVnum];
    
    for (MUINT8 i = 0; i < MAVnum; i++) {
        MY_DBG("NUM: %d", i);
        MUINT32 jpgSize;
        MUINT32 shiftSize = i * mMAVFrame.frmSize;
        //
        jpgSize = camjpg.doJpg(i == 0? 1 : 0, src, dst, shiftSize, shiftSize);
        pMPImageInfo[i].imageBuf = (char*)(mJPGFrameAddr + shiftSize);
        pMPImageInfo[i].imageSize = jpgSize ;
        pMPImageInfo[i].sourceType = SOURCE_TYPE_BUF;  
        // 
        #if 0
        char sourceFiles[80];
        sprintf(sourceFiles, "%s%d.jpg", "/sdcard/DCIM/Camera/test", i);
        mHalMiscDumpToFile((char *) sourceFiles, (MUINT8 *)pMPImageInfo[i].imageBuf, jpgSize);
        #endif
    }
    

    // (3) create MPO file
    //err = camjpg.createMPO(pMPImageInfo, MAVnum, (char*)SaveFileName);
    //if (err < 0) {
    //    MY_DBG("createMPO fails"); 
    //    return err;
    //}
    
    // (3) encode MPO to memory
    MUINT8 *mpoBuffer = NULL;
    MUINT32 mpoSize = 0;
    queryMpoSize(pMPImageInfo, MAVnum, MTK_TYPE_MAV, mpoSize);
    mpoBuffer = (MUINT8*)malloc(mpoSize);
    if(!mpoBuffer) {
        MY_DBG("alloc mpoBuffer fail");
        goto lbExit;
    }
    if(!createMPOInMemory(pMPImageInfo, MAVnum, MTK_TYPE_MAV, mpoBuffer)) {
        MY_DBG("createMPOInMemory fail");
        goto lbExit;
    }
    captureDoneCallback(MHAL_CAM_CB_MAV_ORI_DATA
                                , 2
                                , (MINT32)mpoBuffer
                                , (MINT32)mpoSize
                                );
   
    #if (MAV_PROFILE_CAPTURE)
        DbgTmr.print("MAVProfiling - mHalCamFeatureCompress out");
    #endif

lbExit:    
    setState(MHAL_CAM_FEATURE_STATE_IDLE);          
     
    return err;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
mhal_MAV::
createMPOInMemory(MPImageInfo * pMPImageInfo, MUINT32 num, MUINT32 MPOType, MUINT8* mpoBuffer)
{
    MINT32 err = NO_ERROR;
    MBOOL ok;
    MpoEncoder* mpoEncoder = new MpoEncoder();
    if (mpoEncoder) {
        ok = mpoEncoder->setJpegSources(TYPE_Disparity, pMPImageInfo, num);

        if (!ok) {
            MY_DBG("  mpoEncoder->setJpegSources fail \n");
            err = 1;
            goto mHalCamMAVMakeMPO_EXIT;
        }

        if(!mpoBuffer) {
            MY_DBG("  malloc fail\n");
            err = 1;
            goto mHalCamMAVMakeMPO_EXIT;
        }

        ok = mpoEncoder->encodeToMemory(mpoBuffer, MPOType);        

        if (!ok) {
            MY_DBG("  mpoEncoder->encode fail \n");
            err = 1;
            goto mHalCamMAVMakeMPO_EXIT;
        }

        MY_DBG("[createMPOInMemory] Done\n");
    }
    else
    {
        MY_DBG("new MpoEncoder() fail");
        return false;
    }

mHalCamMAVMakeMPO_EXIT:
    delete mpoEncoder;
    if(err!=NO_ERROR)
        return false;
    else
        return true;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
mhal_MAV::
queryMpoSize(MPImageInfo * pMPImageInfo, MUINT32 num, MUINT32 MPOType, MUINT32 &mpoSize)
{
    MINT32 err = NO_ERROR;
    MBOOL ok;
    MpoEncoder* mpoEncoder = new MpoEncoder();
    if (mpoEncoder) {
        ok = mpoEncoder->setJpegSources(TYPE_Disparity, pMPImageInfo, num);

        if (!ok) {
            MY_DBG("  mpoEncoder->setJpegSources fail \n");
            err = 1;
            goto mHalCamMAVMakeMPO_EXIT;
        }

        mpoSize = mpoEncoder->getBufferSize();

        MY_DBG("mpoSize %d", mpoSize);
    }
    else
    {
        MY_DBG("new MpoEncoder() fail");
        return false;
    }

mHalCamMAVMakeMPO_EXIT:
    delete mpoEncoder;
    if(err!=NO_ERROR)
        return false;
    else
        return true;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
mhal_MAV::
captureDoneCallback(int32_t message, int32_t id, int32_t bufferAddr, int32_t bufferSize)
{
    MY_DBG("+");
    bool ret = true;

    // for debug
	  //char value[PROPERTY_VALUE_MAX] = {'\0'};
	  //property_get("mediatek.previewfeature.dump", value, "1");
	  //bool dump = atoi(value);
    //if(dump) {
    //    mHalMiscDumpToFile("/sdcard/MAVFinal.mpo", (MUINT8*)bufferAddr, (MUINT32)bufferSize);
    //}
    
    mpMhalCam->mHalCamCBHandle(message, (void*)bufferAddr, bufferSize); 

    MY_DBG("-");
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
mhal_MAV::
ErrHandling(MINT32 errMsg)
{
    // sth wrong happend. 
    // call back error message to AP --> unlock UI
    // ap call cancel down to here
    MY_DBG("Error handling occurring");
    setState(MHAL_CAM_FEATURE_STATE_IDLE);
    mpMhalCam->mHalCamCBHandle(MHAL_CAM_CB_ERR, &errMsg);
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mhal_MAV::getState() const {

    Mutex::Autolock lock(mLock);
    return eState; 
}

/*******************************************************************************
*
*********************************************************************************/

MVOID
mhal_MAV::setState(MUINT32 state) 
{
    Mutex::Autolock lock(mLock);
    
    eState = (state_e)state; 
    MY_DBG("set state: %d", eState);
}

/*******************************************************************************
*
********************************************************************************/
#if errorTest
MBOOL ErrorTest()
{
    MINT32 n = rand()%10; 
    if ( n < 1) {  // 10% hits
        MY_DBG("ErrorTest hits!");
        return true;   
    }
    return false;
}
#endif

