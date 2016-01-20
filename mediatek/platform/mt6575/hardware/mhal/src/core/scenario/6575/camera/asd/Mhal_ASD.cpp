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
//----------------------------------------------------------------------------
#define LOG_TAG "mHalCamASD"
//
#include <semaphore.h>
#include <utils/Log.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>
#include <mhal/inc/camera.h>
#include "Mhal_ASD_imp.h"
//----------------------------------------------------------------------------
static mHal_ASD* pmHalASDObj = NULL;
static pthread_t mHalASDHandle = NULL;
static sem_t mHalASDSemBuffer, mHalASDSemThread;
static volatile MHAL_ASD_THREAD_STATE_ENUM mHalASDThreadState = MHAL_ASD_THREAD_STATE_NONE;
static volatile MHAL_ASD_BUF_STATE_ENUM mHalASDBufState = MHAL_ASD_BUF_STATE_EMPTY;
static void* mHalASDThread(void *arg);
//----------------------------------------------------------------------------
static void* mHalASDThread(void *arg)
{
    MINT32 Ret = MHAL_NO_ERROR;
    volatile MUINT32 DetectSceneCount = 0;
    //
    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = 0;
    sched_setscheduler(0, SCHED_OTHER, &sched_p);    
    setpriority(PRIO_PROCESS, 0, PRIO_MAX);
    LOG_MSG("Policy(%d), Priority(%d)",sched_getscheduler(0),getpriority(PRIO_PROCESS, 0));
    LOG_MSG("pid(%d),ppid(%d),tid(%d)",getpid(),getppid(),gettid());
    //
    mHalASDThreadState = MHAL_ASD_THREAD_STATE_CREATE;
    mHalASDBufState = MHAL_ASD_BUF_STATE_EMPTY;
    sem_post(&mHalASDSemThread);
    //
    while(1)
    {
        sem_wait(&mHalASDSemBuffer);
        //
        if(mHalASDThreadState == MHAL_ASD_THREAD_STATE_END)
        {
            LOG_MSG("Thread is terminated!"); 
            break;
        }
        //
        if(mHalASDBufState == MHAL_ASD_BUF_STATE_READY)
        {
            if(DetectSceneCount < MHAL_ASD_SKIP_THRESHOLD)
            {
                DetectSceneCount++;
                LOG_MSG("DetectSceneCount(%d)",DetectSceneCount);
            }
            else
            {
                pmHalASDObj->DetectScene();
            }
            mHalASDBufState = MHAL_ASD_BUF_STATE_EMPTY;
        }
        else
        {
            LOG_ERR("Buf is empty!"); 
        }
        //
        pmHalASDObj->SendScene();
    }
    //
    return NULL;
}
//----------------------------------------------------------------------------
static BOOL mHalASDThreadCreate(void)
{
    MINT32   Ret;
    LOG_MSG("");
    //
    sem_init(&mHalASDSemBuffer,0,0);
    sem_init(&mHalASDSemThread,0,0);
    LOG_MSG("Policy(%d), Priority(%d)",sched_getscheduler(0),getpriority(PRIO_PROCESS, 0));
    Ret = pthread_create(&mHalASDHandle, NULL, mHalASDThread, NULL);
    sem_wait(&mHalASDSemThread);
    //
    if(Ret == 0)
    {
        LOG_MSG("Handle(0x%08X)",(int)mHalASDHandle);
        return TRUE;
    }
    else
    {
        LOG_ERR("Create fail Ret(%d)",Ret);
        return FALSE;
    }
}
//----------------------------------------------------------------------------
static void mHalASDThreadWaitEnd(void)
{
    LOG_MSG("start");
    mHalASDThreadState = MHAL_ASD_THREAD_STATE_END;
    sem_post(&mHalASDSemBuffer);
    pthread_join(mHalASDHandle, NULL);
    LOG_MSG("end");
}
//----------------------------------------------------------------------------
mHal_ASD_Imp::mHal_ASD_Imp(void)
{
    LOG_MSG("");
    //
    mState = MHAL_ASD_STATE_UNINIT;
    pmHalASDObj = this;
}
//----------------------------------------------------------------------------
mHal_ASD_Imp::~mHal_ASD_Imp(void)
{
    LOG_MSG("");
    //
}
//-----------------------------------------------------------------------------
mHal_ASD* mHal_ASD::CreateInstance(void)
{
    return mHal_ASD_Imp::GetInstance();
}
//-----------------------------------------------------------------------------
mHal_ASD* mHal_ASD_Imp::GetInstance(void)
{
    static mHal_ASD_Imp Singleton;
    //
    //LOG_MSG("");
    //
    return &Singleton;
}
//----------------------------------------------------------------------------
void mHal_ASD_Imp::DestroyInstance(void) 
{
}
//----------------------------------------------------------------------------
MINT32 mHal_ASD_Imp::Init(void)
{
    MINT32 Ret = MHAL_NO_ERROR;
    //
    Mutex::Autolock lockProc(mLockProc);
    Mutex::Autolock lockSetFrame(mLockSetFrame);
    //
    LOG_MSG("mUsers(%d)",mUsers);
    //
    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        return MHAL_NO_ERROR;
    }
    //
    if(mState != MHAL_ASD_STATE_UNINIT)
    {
        LOG_ERR("Please set info or uninit first! state(%d)",mState);
        return MHAL_INVALID_RESOURCE;
    }
    //
    mpIspHal = IspHal::createInstance();
    if(mpIspHal == NULL)
    {
        LOG_ERR("pIspHal is NULL"); 
        return MHAL_INVALID_MEMORY;
    }
    //
    Ret = mpIspHal->init();
    if(Ret < 0)
    {
        LOG_ERR("pIspHal->init err, Ret(0x%x)", Ret); 
        return Ret;
    }
    //
    Ret = mpIspHal->sendCommand(ISP_CMD_GET_SENSOR_TYPE, (int)&mSensorType);
    if(Ret < 0)
    {
        LOG_ERR("ISP_CMD_GET_SENSOR_TYPE err, Ret(0x%x)", Ret); 
        return Ret;
    }
    //
    mpWorkingBuf = (MUINT8*)malloc(MHAL_ASD_WORKING_BUF_SIZE);
    if(mpWorkingBuf == NULL)
    {
        LOG_ERR("memory is not enough"); 
        return MHAL_INVALID_RESOURCE;
    }
    //
    mHalASDHandle = NULL;
    //
    if(!mHalASDThreadCreate())
    {
        LOG_ERR("Create thread fail");
        if(mpWorkingBuf != NULL)
        {
            free(mpWorkingBuf);
            mpWorkingBuf = NULL;
        }
        return Ret;
    }
    //
    mState = MHAL_ASD_STATE_INIT;
    mSceneDetCount = 0;
    mSceneCur = mhal_ASD_DECIDER_UI_AUTO;
    mScenePre = mhal_ASD_DECIDER_UI_SCENE_NUM;
    mpFaceResult = NULL;
    android_atomic_inc(&mUsers);
    LOG_MSG("End");
    return Ret;
}
//----------------------------------------------------------------------------
MINT32 mHal_ASD_Imp::Uninit(void)
{
    MINT32 Ret = MHAL_NO_ERROR;
    //
    Mutex::Autolock lockProc(mLockProc);
    Mutex::Autolock lockSetFrame(mLockSetFrame);
    //
    LOG_MSG("mUsers(%d)",mUsers);
    //
    if(mUsers <= 0) {
        // No more users
        return MHAL_NO_ERROR;
    }
    // More than one user
    android_atomic_dec(&mUsers);
    //
    if(mUsers > 0)
    {
        return MHAL_NO_ERROR;
    }
    //
    if(mState == MHAL_ASD_STATE_UNINIT)
    {
        LOG_MSG("It has been uninited");
        return MHAL_NO_ERROR;
    }
    //
    Enable(MFALSE);
    mHalASDThreadWaitEnd();
    //
    if(mpHalASDObj != NULL)
    {
        Ret = mpHalASDObj->mHalAsdUnInit();
        if(Ret < 0)
        {
            LOG_ERR("mHalAsdUninit err, Ret(0x%x)", Ret);
            return Ret;
        }
        //
        mpHalASDObj->destroyInstance();
        mpHalASDObj = NULL;
    }
    //    
    if(mpWorkingBuf != NULL)
    {
        free(mpWorkingBuf);
        mpWorkingBuf = NULL;
    }
    //
    if(mFrame.pData != NULL)
    {
        free(mFrame.pData);
        mFrame.pData = NULL;
    }
    //
    if(mpIspHal != NULL)
    {
        mpIspHal->uninit();
        mpIspHal->destroyInstance();
        mpIspHal = NULL;
    }
    //
    if(mpHal3A != NULL)
    {
        //Hal3A does not provide multi-user protection, so we do not uninit Hal3A brefore destroyInstance.
        mpHal3A->destroyInstance();
        mpHal3A = NULL;
    }
    //
    if(mpHalFD != NULL)
    {
        //Hal3A does not provide multi-user protection, so we do not uninit and destroyInstance Hal3A.
        mpHalFD = NULL;
        //
        if(mpFaceResult != NULL)
        {
            if(mpFaceResult->faces != NULL)
            {
                free(mpFaceResult->faces);
                mpFaceResult->faces = NULL;
            }
            free(mpFaceResult);
            mpFaceResult = NULL;
        }
    }
    //
    mCB = NULL;
    mState = MHAL_ASD_STATE_UNINIT;
    mSceneDetCount = 0;
    mSceneCur = mhal_ASD_DECIDER_UI_AUTO;
    mScenePre = mhal_ASD_DECIDER_UI_SCENE_NUM;
    mpFaceResult = NULL;
    //
    LOG_MSG("End");
    return Ret;
}
//----------------------------------------------------------------------------
MINT32 mHal_ASD_Imp::RegCallback(mHalCamObserver CB)
{
    //
    if  ( ! CB )
    {
        LOG_ERR("CB is NULL"); 
        return MHAL_INVALID_PARA;
    }
    //
    mCB = CB;
    return MHAL_NO_ERROR;
}
//----------------------------------------------------------------------------
void mHal_ASD_Imp::Enable(MBOOL En)
{
    LOG_MSG("mState(%d), En(%d)",mState,En);
    //
    if(mState != MHAL_ASD_STATE_UNINIT)
    {
        if(En)
        {
            mState = MHAL_ASD_STATE_ENABLE;
        }
        else
        {
            mState = MHAL_ASD_STATE_INIT;
        }
    }
}
//----------------------------------------------------------------------------
MINT32 mHal_ASD_Imp::Proc(void)
{
    MINT32 Ret = MHAL_NO_ERROR;
    AAA_ASD_INFO_T ASDInfo;
    Hal3ASensorType_e SensorType[2] = {HAL3A_SENSOR_TYPE_RAW, HAL3A_SENSOR_TYPE_YUV};
    //
    Mutex::Autolock lock(mLockProc);
    //
    if(mState != MHAL_ASD_STATE_ENABLE)
    {
        return MHAL_NO_ERROR;
    }
    //
    LOG_MSG("Start");
    //
    if  ( ! mCB )
    {
        LOG_ERR("mCB is NULL"); 
        return MHAL_INVALID_PARA;
    }
    //
    if(mpHal3A == NULL)
    {
        mpHal3A = Hal3ABase::createInstance(SensorType[mSensorType], ISP_SENSOR_DEV_MAIN);
        if(mpHal3A == NULL)
        {
            LOG_ERR("pHal3A is NULL"); 
            return MHAL_INVALID_MEMORY;
        }
    }
    //
    Ret = mpHal3A->getASDInfo(ASDInfo);
    if(Ret < 0)
    {
        LOG_ERR("getASDInfo err, Ret(0x%x)", Ret); 
        return Ret;
    }
    //
    if(mpHalFD == NULL)
    {
        mpHalFD = halFDBase::createInstance(HAL_FD_OBJ_HW);
        if(mpHalFD == NULL)
        {
            LOG_ERR("pHal3A is NULL"); 
            return MHAL_INVALID_MEMORY;
        }
        //
        if(mpFaceResult == NULL)
        {
            mpFaceResult = (camera_face_metadata_m*)malloc(sizeof(camera_face_metadata_m));
            camera_face_m* pFaces = ( camera_face_m*)malloc(sizeof(camera_face_m)*MHAL_ASD_FACE_AMOUNT);
            mpFaceResult->faces = pFaces;
            mpFaceResult->number_of_faces = 0;
        }
    }
    //
    Ret = mpHalFD->halFDGetFaceResult(mpFaceResult,0);
    if(Ret < 0)
    {
        LOG_ERR("halFDGetFaceResult err, Ret(0x%x)", Ret); 
        return Ret;
    }
    //
    if(mpHalASDObj == NULL)
    {
        mpHalASDObj = halASDBase::createInstance(HAL_ASD_OBJ_AUTO);
        if(mpHalASDObj == NULL)
        {
            LOG_ERR("mpHalASDObj is NULL"); 
            return MHAL_INVALID_MEMORY;
        }
        //
        Ret = mpHalASDObj->mHalAsdInit((void*)&ASDInfo, mpWorkingBuf, (mSensorType==HAL3A_SENSOR_TYPE_RAW)?0:1);
        if(Ret < 0)
        {
            LOG_ERR("mHalAsdInit err, Ret(0x%x)", Ret);
            return Ret;
        }
    }
    //
    LOG_MSG("mHalAsdDecider");
    Ret = mpHalASDObj->mHalAsdDecider((void*)&ASDInfo,(void*)mpFaceResult,mSceneCur);
    if(Ret < 0)
    {
        LOG_ERR("mHalAsdDecider err, Ret(0x%x)", Ret); 
        return Ret;
    }
    //
    //SendScene();
    //
    LOG_MSG("End");
    //
    return Ret;
}
//----------------------------------------------------------------------------
MINT32 mHal_ASD_Imp::SetFrame(MHAL_ASD_FRAME_STRUCT* pFrame)
{
    Mutex::Autolock lock(mLockSetFrame);
    //
    //LOG_MSG("mHalASDBufState(%d)",mHalASDBufState);
    //
    if( mState != MHAL_ASD_STATE_ENABLE ||
        mHalASDThreadState == MHAL_ASD_THREAD_STATE_END)
    {
        return MHAL_NO_ERROR;
    }
    //
    if(pFrame == NULL)
    {
        LOG_ERR("pFrame is NULL");
        return MHAL_INVALID_MEMORY;
    }
    //
    if(pFrame->pData == NULL)
    {
        LOG_ERR("pFrame->pData is NULL");
        return MHAL_INVALID_MEMORY;
    }
    //
    if(mHalASDBufState == MHAL_ASD_BUF_STATE_EMPTY)
    {
        //copy preview data
        if(mFrame.pData == NULL)
        {
            mFrame.Width = pFrame->Width;
            mFrame.Height = pFrame->Height;
            mFrame.DataSize = mFrame.Width*mFrame.Height*2;
            mFrame.pData = (MUINT8*)malloc(mFrame.DataSize);
            if(mFrame.pData == NULL)
            {
                LOG_ERR("Memory allocate fail!");
                return MHAL_INVALID_MEMORY;
            }
            //LOG_MSG("W(%d), H(%d), S(%d), A(0x%08X)",mFrame.Width,mFrame.Height,mFrame.DataSize,(UINT32)(mFrame.pData));
        }
        LOG_MSG("Data copy");
        memcpy(mFrame.pData, pFrame->pData, mFrame.DataSize);
        mHalASDBufState = MHAL_ASD_BUF_STATE_READY;
        LOG_MSG("Data ready");
        sem_post(&mHalASDSemBuffer);
    }
    //LOG_MSG("End");
    //
    return MHAL_NO_ERROR;
}
//----------------------------------------------------------------------------
void mHal_ASD_Imp::DetectScene(void)
{
    if(mState != MHAL_ASD_STATE_ENABLE)
    {
        return;
    }
    //
    LOG_MSG("W(%d), H(%d), S(%d), A(0x%08X)",mFrame.Width,mFrame.Height,mFrame.DataSize,(UINT32)(mFrame.pData));
    mpHalASDObj->mHalAsdDoSceneDet((void*)mFrame.pData, mFrame.Width, mFrame.Height);
    LOG_MSG("End");
}
//----------------------------------------------------------------------------
void mHal_ASD_Imp::SendScene(void)
{
    mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM SceneCur;
    //
    if(mState != MHAL_ASD_STATE_ENABLE)
    {
        return;
    }
    //
    if  ( ! mCB )
    {
        LOG_ERR("CB is NULL"); 
        return;
    }
    //
    SceneCur = mSceneCur;
    LOG_MSG("SceneCur(%d), mScenePre(%d)",SceneCur,mScenePre);
    //
    mSceneDetCount++;
    if( mScenePre != SceneCur ||
        mSceneDetCount >= MHAL_ASD_SCENE_DET_THRESHOLD)
    {
        LOG_MSG("Callback");
        mCB.notify(MHAL_CAM_CB_ASD, &SceneCur);

        mSceneDetCount = 0;
        mScenePre = SceneCur;
    }
    return;
}
//----------------------------------------------------------------------------
MHAL_ASD_STATE_ENUM mHal_ASD_Imp::GetState(void)
{
    //LOG_MSG("mState(%d)",mState);
    return mState;
}
//----------------------------------------------------------------------------
mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM mHal_ASD_Imp::GetScene(void)
{
    LOG_MSG("mSceneCur(%d)",mSceneCur);
    return mSceneCur;
}
//----------------------------------------------------------------------------


