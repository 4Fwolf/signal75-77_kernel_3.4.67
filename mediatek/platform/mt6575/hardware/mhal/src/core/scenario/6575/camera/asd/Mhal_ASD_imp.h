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
#ifndef MHAL_ASD_IMP_H
#define MHAL_ASD_IMP_H
//----------------------------------------------------------------------------
#include "Mhal_ASD.h"
#include <cutils/xlog.h>
#include <utils/threads.h>
#include <asd_hal_base.h>
#include "isp_hal.h"
#include "aaa_hal_base.h"
#include "fd_hal_base.h"
//----------------------------------------------------------------------------
using namespace android;
//----------------------------------------------------------------------------
#define LOG_MSG(fmt, arg...)    XLOGD("[%s]"fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    XLOGW("[%s]Warning(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    XLOGE("[%s]Err(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
//----------------------------------------------------------------------------
#define MHAL_ASD_WORKING_BUF_SIZE       (160*120*2*11+200*1024)
#define MHAL_ASD_SCENE_DET_THRESHOLD    (30)
#define MHAL_ASD_SKIP_THRESHOLD         (10)
#define MHAL_ASD_FACE_AMOUNT            (15)
//----------------------------------------------------------------------------
typedef enum
{
    MHAL_ASD_STATE_UNINIT,
    MHAL_ASD_STATE_INIT,
    MHAL_ASD_STATE_ENABLE
}MHAL_ASD_STATE_ENUM;

typedef enum
{
    MHAL_ASD_THREAD_STATE_NONE,
    MHAL_ASD_THREAD_STATE_CREATE,
    MHAL_ASD_THREAD_STATE_END
}MHAL_ASD_THREAD_STATE_ENUM;

typedef enum
{
    MHAL_ASD_BUF_STATE_EMPTY,
    MHAL_ASD_BUF_STATE_READY
}MHAL_ASD_BUF_STATE_ENUM;

//----------------------------------------------------------------------------
class mHal_ASD_Imp : public mHal_ASD
{
    protected:
        mHal_ASD_Imp();
        ~mHal_ASD_Imp();
    //
    public:
        static mHal_ASD* GetInstance(void);
        virtual void    DestroyInstance(void);
        virtual MINT32  Init(void);
        virtual MINT32  Uninit(void);
        virtual MINT32  RegCallback(mHalCamObserver CB);
        virtual void    Enable(MBOOL En);
        virtual MINT32  Proc(void);
        virtual MINT32  SetFrame(MHAL_ASD_FRAME_STRUCT* pFrame);
        virtual void    DetectScene(void);
        virtual void    SendScene(void);
        virtual MHAL_ASD_STATE_ENUM GetState(void);
        virtual mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM GetScene(void);
    //
    private:
        volatile int                    mUsers;
        volatile MHAL_ASD_STATE_ENUM    mState;
        volatile MUINT32                mSceneDetCount;
        MHAL_ASD_FRAME_STRUCT           mFrame;
        IspHal*                         mpIspHal;
        Hal3ABase*                      mpHal3A;
        halFDBase*                      mpHalFD;
        halASDBase*                     mpHalASDObj;
        halIspSensorType_e              mSensorType;
        mutable Mutex                   mLockProc;
        mutable Mutex                   mLockSetFrame;    
        MUINT8*                         mpWorkingBuf;
        mHalCamObserver                 mCB;
        camera_face_metadata_m*         mpFaceResult;
        mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM             mSceneCur;
        volatile mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM    mScenePre;
};
//----------------------------------------------------------------------------
#endif

