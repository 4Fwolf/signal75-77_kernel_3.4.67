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


#ifndef _Mhal_SHOT_3D_H
#define _Mhal_SHOT_3D_H

/*******************************************************************************
*
*******************************************************************************/

#include <semaphore.h>
#include <utils/threads.h>
#include <utils/Errors.h>
#include <cutils/xlog.h>
#include <mcam_mem.h>

#include <mhal/inc/camera.h>
#include <cam_types.h>

#include "3DF_hal_base.h"
#include "autorama_hal_base.h"
#include "../3dlib/Mhal_featureBase.h"


class mHalCam;
class ICameraIO;
class Hal3ABase;

/*******************************************************************************
*
*******************************************************************************/

using namespace android;

class mhal_3dshot : public mhal_featureBase
{
    public:        ////
        mhal_3dshot(NSCamera::ESensorType const eSensorType);
        static mhal_3dshot*   createInstance(NSCamera::ESensorType const eSensorType);
        virtual MVOID	      destroyInstance();

        virtual MINT32        init(mHalCam* const pMhalCam, Hal3ABase* const pHal3AObj, ICameraIO* const pCameraIOObj, mHalCamMemPool* const pFDWorkingBuf);
        virtual MINT32	      mHalCamFeatureInit(mhalCamParam_t *pmhalCamParam);
        virtual MVOID         mHalCamFeatureUninit();
        virtual MINT32        mHalCamFeatureProc(MUINT32 frameIndex);
        virtual MINT32        mHalCamFeatureCompress();
        virtual MVOID         setMerge(){setState(MHAL_CAM_FEATURE_STATE_MERGE);}
        virtual MVOID         setCancel(){setState(MHAL_CAM_FEATURE_STATE_IDLE);}
        virtual MBOOL         shot3dDone(){return m3dFrameIdx == m3dFrame.frmCount? true : false;}

    public:       ////
        virtual MINT32        mHalCamFeatureAddImg();    
        virtual MINT32	      mHalCamFeatureMerge();
        virtual MINT32	      setParameters(mhalCamParam_t *pmhalCamParam);
        virtual MINT32	      ISShot(MVOID *arg1, MBOOL &shot);

    private:       ////
        NSCamera::ESensorType const meSensorType;
        hal3DFBase*           mp3dshotObj;
        MINT8*                mp3dWorkingBuf; 
        MUINT32               mJPGFrameAddr;    
        MUINT32               m3dFrameIdx;
        MTKPIPEAUTORAMA_DIRECTION_ENUM mStitchDir;
        MUINT8                outputFile[128]; 
        mutable Mutex         mLock;
		MavPipeResultInfo     mp3dResult;
		PipePano3DResultInfo  MyPano3DResultInfo;
		MavPipeResultInfo     MyMavPipeResultInfo;
		MavPipeImageInfo      ImageInfo;

        MUINT32               frmYuv_virtAddr;
        MUINT32               frmYuv_size;
        MUINT32               m3dNum;
	    mhalCamFrame_t        m3dFrame;
        MUINT32               virAddrShift;
        MINT32                mergeCnt;
	public:        ////
		MVOID                 setFrameBuf(MUINT8* memIn, MUINT32 size);

		
	/* passed from mHalCam (Current version)*/
    private:
        mHalCam*              mpMhalCam;
        Hal3ABase*            mpHal3AObj;
        ICameraIO*            mpCameraIOObj; 
        mHalCamMemPool*       mFDWorkingBuf;
        mhalCamParam_t*       mpmhalCamParam;
};


#endif
