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
//----------------------------------------------------------------------------
#define LOG_TAG "VideoSnapshot"
//
#include <semaphore.h>
#include <utils/Log.h>
#include <sys/time.h>
#include <linux/cache.h>
#include <sys/resource.h>
#include <pthread.h>
#include <mhal/inc/camera.h>
#include <MediaTypes.h>
#include <camexif/CamExif.h>
#include <jpeg_hal.h>
#include "VideoSnapshot_imp.h"
//----------------------------------------------------------------------------
static VideoSnapshotImp* pVideoSnapshotObj = NULL;
static pthread_t VideoSnapshotHandle = NULL;
static sem_t VideoSnapshotSemThread,VideoSnapshotSemJpg;
static volatile VIDEO_SNAPSHOT_THREAD_STATE_ENUM VideoSnapshotThreadState = VIDEO_SNAPSHOT_THREAD_STATE_NONE;
static void* VideoSnapshotThread(void *arg);
//----------------------------------------------------------------------------
static void* VideoSnapshotThread(void *arg)
{
    MINT32 Ret = MHAL_NO_ERROR;
    //
    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = 0;
    sched_setscheduler(0, SCHED_OTHER, &sched_p);    
    setpriority(PRIO_PROCESS, 0, PRIO_MIN);
    LOG_MSG("[VideoSnapshotThread]Policy(%d), Priority(%d)",
            sched_getscheduler(0),
            getpriority(PRIO_PROCESS,
            0));
    LOG_MSG("[VideoSnapshotThread]pid(%d),ppid(%d),tid(%d)",
            getpid(),
            getppid(),
            gettid());
    //
    VideoSnapshotThreadState = VIDEO_SNAPSHOT_THREAD_STATE_CREATE;
    sem_post(&VideoSnapshotSemThread);
    //
    while(1)
    {
        sem_wait(&VideoSnapshotSemJpg);
        //
        if(VideoSnapshotThreadState == VIDEO_SNAPSHOT_THREAD_STATE_END)
        {
            LOG_MSG("[VideoSnapshotThread]Thread is terminated!"); 
            break;
        }
        else
        {
            if(pVideoSnapshotObj)
            {
                pVideoSnapshotObj->ProcessImg();
            }
            else
            {
                LOG_MSG("[VideoSnapshotThread]pVideoSnapshotObj is NULL"); 
            }
        }
    }
    //
    return NULL;
}
//----------------------------------------------------------------------------
static BOOL VideoSnapshotThreadCreate(void)
{
    MINT32   Ret;
    LOG_MSG("[VideoSnapshotThreadCreate]");
    //
    sem_init(&VideoSnapshotSemThread,0,0);
    sem_init(&VideoSnapshotSemJpg,0,0);
    LOG_MSG("[VideoSnapshotThreadCreate]Policy(%d), Priority(%d)",
            sched_getscheduler(0),
            getpriority(PRIO_PROCESS, 0));
    Ret = pthread_create(&VideoSnapshotHandle, NULL, VideoSnapshotThread, NULL);
    sem_wait(&VideoSnapshotSemThread);
    //
    if(Ret == 0)
    {
        LOG_MSG("[VideoSnapshotThreadCreate]Handle(0x%08X)",(int)VideoSnapshotHandle);
        return TRUE;
    }
    else
    {
        LOG_ERR("[VideoSnapshotThreadCreate]Create fail Ret(%d)",Ret);
        return FALSE;
    }
}
//----------------------------------------------------------------------------
static void VideoSnapshotThreadWaitEnd(void)
{
    LOG_MSG("[VideoSnapshotThreadWaitEnd]E");
    VideoSnapshotThreadState = VIDEO_SNAPSHOT_THREAD_STATE_END;
    sem_post(&VideoSnapshotSemJpg);
    pthread_join(VideoSnapshotHandle, NULL);
    LOG_MSG("[VideoSnapshotThreadWaitEnd]X");
}
//----------------------------------------------------------------------------
VideoSnapshotImp::VideoSnapshotImp(void)
{
    LOG_MSG("[VideoSnapshotImp]");
    //
    pVideoSnapshotObj = this;
    mBufVirAddr = 0;
    mFrameVirAddr = 0;
    mJpgSize = 0;
    mRotation = 0;
    mIsFirstFrame = MTRUE;
    mIsConfig = MFALSE;
    mIsTakePic = MFALSE;
    mpCameraIOObj = NULL;
}
//----------------------------------------------------------------------------
VideoSnapshotImp::~VideoSnapshotImp(void)
{
    LOG_MSG("[~VideoSnapshotImp]");
    //
}
//-----------------------------------------------------------------------------
VideoSnapshot* VideoSnapshot::CreateInstance(void)
{
    return VideoSnapshotImp::GetInstance();
}
//-----------------------------------------------------------------------------
VideoSnapshot* VideoSnapshotImp::GetInstance(void)
{
    static VideoSnapshotImp Singleton;
    //
    //LOG_MSG("");
    //
    return &Singleton;
}
//----------------------------------------------------------------------------
void VideoSnapshotImp::DestroyInstance(void) 
{
}
//----------------------------------------------------------------------------
MINT32 VideoSnapshotImp::Init(void)
{
    MINT32 Ret = MHAL_NO_ERROR;
    //
    Mutex::Autolock lock(mLock);
    //
    LOG_MSG("[Init]mUsers(%d) E",mUsers);
    //
    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        return MHAL_NO_ERROR;
    }
    //
    mIsConfig = MFALSE;
    mIsTakePic = MFALSE;
    VideoSnapshotThreadCreate();
    //
    android_atomic_inc(&mUsers);
    LOG_MSG("[Init]X");
    return Ret;
}
//----------------------------------------------------------------------------
MINT32 VideoSnapshotImp::Config(VIDEO_SNAPSHOT_CONFIG_STRUCT* pConfig)
{
    MINT32 Ret = MHAL_NO_ERROR;
    MUINT32 BufSize;
    mHalRegisterLoopMemory_t RegMem;
    //
    LOG_MSG("[Config]mUsers(%d) E",mUsers);
    //
    if(mUsers > 0)
    {
        if( pConfig->Width == 0 ||
            pConfig->Height == 0)
        {
            LOG_ERR("[Config]Width(%d) or Height(%d) is 0.",
                    pConfig->Width,
                    pConfig->Height);
        }
        else
        {
            LOG_MSG("[Config]Width(%d),Height(%d)",
                    pConfig->Width,
                    pConfig->Height);
            memcpy((UINT8*)&mConfig,(UINT8*)pConfig,sizeof(VIDEO_SNAPSHOT_CONFIG_STRUCT));
            //
            BufSize = mConfig.Width*mConfig.Height*2+VIDEO_SNAPSHOT_BUF_OFST_VSS;
            mBufVirAddr = (MUINT32)(MUINT8*)memalign(L1_CACHE_BYTES, BufSize);
            if(mBufVirAddr == 0)
            {
                LOG_ERR("[Config]mBufVirAddr is 0, BufSize(%d)",BufSize);
                Ret = MHAL_INVALID_MEMORY;
                goto EXIT;
            }
            else
            {
                LOG_MSG("[Config]mBufVirAddr(0x%08X), BufSize(%d)",
                        mBufVirAddr,
                        BufSize);
            }
            //
            if(mConfig.Width <= VIDEO_SNAPSHOT_SKIA_WIDTH)
            {
                RegMem.mem_type     = MHAL_MEM_TYPE_OUTPUT;
                RegMem.addr         = GetAddr(VIDEO_SNAPSHOT_BUF_TYPE_VSS);
                RegMem.buffer_size  = mConfig.Width*mConfig.Height*2;
                RegMem.mhal_color   = MHAL_FORMAT_RGB_565;
                RegMem.img_size     = mHalMdpSize(mConfig.Width, mConfig.Height);
                RegMem.img_roi      = mHalMdpRect(0, 0, mConfig.Width, mConfig.Height);
                RegMem.rotate       = 0;
                //
                if(mHalMdp_RegisterLoopMemory(MHAL_MLM_CLIENT_PVCPY, &RegMem, &mRegMemObj) < 0)
                {
                    LOG_ERR("[Config]mHalMdp_RegisterLoopMemory(MHAL_MLM_CLIENT_PVCPY) fails");
                    Ret = MHAL_INVALID_RESOURCE;
                    goto EXIT;
                }
            }
            //
            mpCameraIOObj = ICameraIO::createInstance();
            //
            mIsFirstFrame = MTRUE;
            mIsConfig = MTRUE;
        }
    }
    //
    EXIT:
    LOG_MSG("[Config]X");
    return Ret;
}
//----------------------------------------------------------------------------
MINT32 VideoSnapshotImp::Release(void)
{
    MINT32 Ret = MHAL_NO_ERROR;
    //
    LOG_MSG("[Release]mIsConfig(%d) E",mIsConfig);
    if(mIsConfig)
    {
        if(mConfig.Width <= VIDEO_SNAPSHOT_SKIA_WIDTH)
        {
            if(!mIsFirstFrame)
            {
                LOG_MSG("[Release]enqueueBuff FD_PORT");
                mpCameraIOObj->releasePreviewFrame(
                    ICameraIO::ePREVIEW_FD_PORT,
                    &mBufInfo);
            }
            //
            mHalMdp_UnRegisterLoopMemory(
                MHAL_MLM_CLIENT_PVCPY,
                &mRegMemObj);
        }
        //
        if(mBufVirAddr != 0)
        {
            free((MUINT8*)mBufVirAddr);
            mBufVirAddr = 0;
        }
        //
        mIsConfig = MFALSE;
    }
    LOG_MSG("[Release]X");
    return Ret;
}
//----------------------------------------------------------------------------
MINT32 VideoSnapshotImp::Uninit(void)
{
    MINT32 Ret = MHAL_NO_ERROR;
    //
    Mutex::Autolock lock(mLock);
    //
    LOG_MSG("[Uninit]mUsers(%d) E",mUsers);
    //
    if(mUsers <= 0)
    {
        // No more users
        Ret = MHAL_NO_ERROR;
        goto EXIT;
    }
    // More than one user
    android_atomic_dec(&mUsers);
    //
    if(mUsers > 0)
    {
        Ret = MHAL_NO_ERROR;
        goto EXIT;
    }
    //
    VideoSnapshotThreadWaitEnd();
    //
    if(mBufVirAddr != 0)
    {
        free((MUINT8*)mBufVirAddr);
        mBufVirAddr = 0;
    }
    //
    if(mpCameraIOObj != NULL)
    {
        mpCameraIOObj->destroyInstance();
        mpCameraIOObj = NULL;
    }
    //
    mIsConfig = MFALSE;
    mIsTakePic = MFALSE;
    //
    EXIT:
    LOG_MSG("[Uninit]X");
    return Ret;
}
//----------------------------------------------------------------------------
MINT32 VideoSnapshotImp::TakePic(MUINT32 Rotation)
{
    MINT32 Ret = MHAL_NO_ERROR;
    //
    Mutex::Autolock lock(mLock);
    //
    LOG_MSG("[TakePic]Rotation(%d) E",Rotation);
    //
    if(!mIsConfig)
    {
        LOG_ERR("[TakePic]Please Config first!");
        Ret = MHAL_INVALID_RESOURCE;
        goto EXIT;
    }
    //
    if(mIsTakePic)
    {
        LOG_ERR("[TakePic]The previous TakePic() is not finish!");
        Ret = MHAL_INVALID_RESOURCE;
        goto EXIT;
    }
    mIsTakePic = MTRUE;
    mRotation = Rotation;
    //
    if(mConfig.Width <= VIDEO_SNAPSHOT_SKIA_WIDTH)
    {
        sem_post(&VideoSnapshotSemJpg);
    }
    //
    EXIT:
    LOG_MSG("[TakePic]X");
    return Ret;
}
//----------------------------------------------------------------------------
MINT32 VideoSnapshotImp::SetFrame(MUINT32 BufAddr)
{
    MINT32 Ret = MHAL_NO_ERROR;
    //
    if(!mIsConfig)
    {
        LOG_ERR("[ProcessImg]Please Config first!");
        Ret = MHAL_INVALID_RESOURCE;
        return Ret;
    }
    //
    if(BufAddr == 0)
    {
        LOG_ERR("[ProcessImg]BufAddr is 0");
        Ret = MHAL_INVALID_PARA;
        return Ret; 
    }
    //
    if( mIsTakePic &&
        (mFrameVirAddr == 0) &&
        (mConfig.Width > VIDEO_SNAPSHOT_SKIA_WIDTH))
    {
        LOG_MSG("[SetFrame]mIsTakePic(%d),BufAddr(0x%08X)",mIsTakePic,BufAddr);
        mFrameVirAddr = BufAddr;
        sem_post(&VideoSnapshotSemJpg);
    }
    //
    return Ret;
}
//----------------------------------------------------------------------------
MINT32 VideoSnapshotImp::ProcessImg(void)
{
    MINT32 Ret = MHAL_NO_ERROR;
    mhalVSSJpgEncParam_t VSSJpgEncParam;
    JpgEncHal JpgEnc;
    //
    LOG_MSG("[ProcessImg]E");
    //
    if(!mIsConfig)
    {
        LOG_ERR("[ProcessImg]Please Config first!");
        Ret = MHAL_INVALID_RESOURCE;
        return Ret;
    }
    //
    if(mConfig.Width <= VIDEO_SNAPSHOT_SKIA_WIDTH)
    {
        if(mIsFirstFrame)
        {
            LOG_MSG("[ProcessImg]First getPreviewFrame");
            mpCameraIOObj->getPreviewFrame(
                ICameraIO::ePREVIEW_FD_PORT,
                &mBufInfo);
            mIsFirstFrame = MFALSE;
        }
        LOG_MSG("[ProcessImg]releasePreviewFrame");
        mpCameraIOObj->releasePreviewFrame(
            ICameraIO::ePREVIEW_FD_PORT,
            &mBufInfo);
        LOG_MSG("[ProcessImg]getPreviewFrame");
        mpCameraIOObj->getPreviewFrame(
            ICameraIO::ePREVIEW_FD_PORT,
            &mBufInfo);
        LOG_MSG("[ProcessImg]Done");
    }
    //
    #if VIDEO_SNAPSHOT_HW_JPG_SUPPORT
    if(mConfig.Width > VIDEO_SNAPSHOT_SKIA_WIDTH)
    {
        JpgEnc.setSrcWidth(mConfig.Width);
        JpgEnc.setSrcHeight(mConfig.Height);
        JpgEnc.setDstWidth(mConfig.Width);
        JpgEnc.setDstHeight(mConfig.Height);
        JpgEnc.setQuality(85);
        JpgEnc.setSrcFormat(JpgEncHal::kYV12_Planar_Format);
        JpgEnc.setEncFormat(JpgEncHal::kYUV_422_Format);
        JpgEnc.setSrcAddr((void*)GetAddr(VIDEO_SNAPSHOT_BUF_TYPE_VSS));
        JpgEnc.setDstAddr((void*)GetAddr(VIDEO_SNAPSHOT_BUF_TYPE_JPG));
        JpgEnc.setDstSize(mConfig.Width*mConfig.Height*2);
        JpgEnc.enableSOI(1);
        //FIXME, 
        //JpgEnc.setWaitResTime(500);
        //
        if(!JpgEnc.lock())
        {
            LOG_ERR("[ProcessImg]JpgEnc.lock() failed");
            goto EXIT;
        }
        //
        if(!JpgEnc.start(&mJpgSize))
        {
            LOG_ERR("[ProcessImg]JpgEnc.start() failed");
        }
        //
        JpgEnc.unlock();
    }
    #endif
    //
    LOG_MSG("[ProcessImg]JPG ENC");
    VSSJpgEncParam.u4ImgWidth       = mConfig.Width;
    VSSJpgEncParam.u4ImgHeight      = mConfig.Height;
    VSSJpgEncParam.u4ImgYuvVirAddr  = mFrameVirAddr;
    VSSJpgEncParam.u4ImgRgbVirAddr  = GetAddr(VIDEO_SNAPSHOT_BUF_TYPE_VSS);
    VSSJpgEncParam.u4ImgJpgVirAddr  = GetAddr(VIDEO_SNAPSHOT_BUF_TYPE_JPG);
    VSSJpgEncParam.u4ImgJpgSize     = 0;
    mConfig.CB.notify(
        MHAL_CAM_CB_VSS_JPG_ENC,
        &VSSJpgEncParam);
    mJpgSize = VSSJpgEncParam.u4ImgJpgSize;
    //
    LOG_MSG("[ProcessImg]mJpgSize(%d)",mJpgSize);
    //
    AddExif();
    mConfig.CB.notify(
        MHAL_CAM_CB_VSS_JPG,
        (MUINT8*)GetAddr(VIDEO_SNAPSHOT_BUF_TYPE_EXIF),
        mJpgSize);
    //
    EXIT:
    mIsTakePic = MFALSE;
    mJpgSize = 0;
    mRotation = 0;
    mFrameVirAddr = 0;
    LOG_MSG("[ProcessImg]X");
    //
    return Ret;
}
//----------------------------------------------------------------------------
MUINT32 VideoSnapshotImp::GetAddr(VIDEO_SNAPSHOT_BUF_TYPE_ENUM BufType)
{
    MUINT32 Addr = 0;
    //
    switch(BufType)
    {
        case VIDEO_SNAPSHOT_BUF_TYPE_EXIF:
        {
            Addr = (mBufVirAddr+VIDEO_SNAPSHOT_BUF_OFST_EXIF);
            break;
        }
        case VIDEO_SNAPSHOT_BUF_TYPE_JPG:
        {
            Addr = (mBufVirAddr+VIDEO_SNAPSHOT_BUF_OFST_JPG);
            break;
        }
        case VIDEO_SNAPSHOT_BUF_TYPE_VSS:
        {
            Addr = (mBufVirAddr+VIDEO_SNAPSHOT_BUF_OFST_VSS);
            break;
        }
    }
    //
    LOG_MSG("[GetAddr]BufType(%d),Addr(0x%08X)",BufType,Addr);
    return Addr;
}
//----------------------------------------------------------------------------
MINT32 VideoSnapshotImp::AddExif(void)
{
    MINT32 Ret = MHAL_NO_ERROR;
    MUINT32 ExifHeaderSize = 0;
    //
    LOG_MSG("[AddExif]E");
    //
    if(!mIsConfig)
    {
        LOG_ERR("[AddExif]Please Config first!");
        Ret = MHAL_INVALID_RESOURCE;
        return Ret;
    }
    //
    mConfig.pCamParam->camExifParam.orientation = mRotation;
    //
    CamExif camexif(mConfig.SensorType,mConfig.DeviceId);
    camexif.init(
        CamExifParam(
            mConfig.pCamParam->camExifParam,
            mConfig.pCamParam->cam3AParam,
            100), 
        mConfig.pHal3AObj);
    camexif.makeExifApp1(
        mConfig.Width,
        mConfig.Height,
        0,
        (MUINT8*)GetAddr(VIDEO_SNAPSHOT_BUF_TYPE_EXIF),
        &ExifHeaderSize);
    camexif.uninit();
    LOG_MSG("[AddExif]ExifHeaderSize1(%d)",ExifHeaderSize);
    mJpgSize -= VIDEO_SNAPSHOT_JPG_SOI_LEN;
    memcpy(
        (MUINT8*)(GetAddr(VIDEO_SNAPSHOT_BUF_TYPE_EXIF)+ExifHeaderSize),
        (MUINT8*)(GetAddr(VIDEO_SNAPSHOT_BUF_TYPE_JPG)+VIDEO_SNAPSHOT_JPG_SOI_LEN),
        mJpgSize);
    mJpgSize += ExifHeaderSize;
    LOG_MSG("[AddExif]mJpgSize(%d) X",mJpgSize);
    //
    return Ret;
}


