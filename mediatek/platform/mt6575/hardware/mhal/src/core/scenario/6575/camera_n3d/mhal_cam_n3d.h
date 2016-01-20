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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
/*
** $Log: mhal_cam.h $
 *
*/
                                
#ifndef _MHAL_CAM_N3D_H
#define _MHAL_CAM_N3D_H

#include <pthread.h>
#include <semaphore.h>
#include <linux/rtpm_prio.h>
#include <utils/List.h>

#include "mhal_cam_base.h"
#include "isp_hal.h"
#include "aaa_hal_base.h"
#include <mcam_mem.h>
#include "camera_feature.h"
#include "feature_hal.h"
#include "flashlight_hal_base.h"
#include "res_mgr_hal.h"

#include "ICameraIO.h"

/*******************************************************************************
*
********************************************************************************/
#define MHAL_CAM_3A_OPTION              1       // 3A On/Off
#define MHAL_CAM_ISP_TUNING             1       // ISP Tuning On/Off

//
#define MHAL_CAM_N3D_FEATURE_OFF    0
#define MHAL_CAM_N3D_FEATURE_ON     1

using namespace android;
using namespace NSFeature;

/*******************************************************************************
*
********************************************************************************/
class IShotN3D;
/*******************************************************************************
*
********************************************************************************/
typedef enum mHalCamN3dState_enum {
    MHAL_CAM_N3D_INIT           = 0x0000,
    MHAL_CAM_N3D_IDLE           = 0x0001,
    //
    MHAL_CAM_N3D_PREVIEW_MASK   = 0x0100,
    MHAL_CAM_N3D_PREVIEW        = 0x0100,
    MHAL_CAM_N3D_FOCUS          = 0x0101,
    //
    MHAL_CAM_N3D_CAPTURE_MASK   = 0x0200,
    MHAL_CAM_N3D_PRE_CAPTURE    = 0x0200,
    MHAL_CAM_N3D_CAPTURE        = 0x0201,
    //
    MHAL_CAM_N3D_STOP           = 0x0400,
    //
    MHAL_CAM_N3D_UNINIT         = 0x0800,
    //
    MHAL_CAM_N3D_ERROR          = 0x8000,
} mHalCamN3dState_e;




/*******************************************************************************
*
********************************************************************************/
class mHalCamN3D : public mHalCamBase {
public:
    //
    static mHalCamBase* getInstance();
    //
    virtual void destroyInstance();
    //
    mHalCamN3D();
    //
    virtual ~mHalCamN3D();
    //
    virtual MINT32 mHalCamPreviewStart(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize);
    //
    virtual MINT32 mHalCamPreviewStop();
    //
    virtual MINT32 mHalCamPreCapture();
    //
    virtual MINT32 mHalCamCaptureStart(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize);
    //
    virtual MINT32 mHalCamSet3AParameter(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize);
    //
    //
    virtual MINT32 mHalCamCaptureInit(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize);
    //
    virtual MINT32 mHalCamCaptureUninit(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize);
    //
    virtual MINT32 mHalCamVideoRecordStart(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize);
    //
    virtual MINT32 mHalCamVideoRecordStop(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize);

    virtual MINT32 mHalCamInit(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize);
    //
    virtual MINT32 mHalCamUninit();
    //
    mHalCamN3dState_e mHalCamGetState();
    //
    virtual MINT32 mHalCamPreviewProc();
    //
    virtual MINT32 mHalCamCaptureProc();
    //
    virtual MINT32 mHalCamSetFeatureMode(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize);
    //
    virtual MINT32 mHalCamGetFeatureEnum(MVOID*const a_pOutBuf, MUINT32 const a_u4OutBufNum, MUINT32& ra_u4NumReturned);
    //
    virtual MINT32 mHalCamSendCommand(MINT32 cmd, MINT32 arg1 = 0, MINT32 arg2 = 0, MINT32 arg3 = 0);
    //
    virtual MINT32 mHalCamGetBufMemInfo(MVOID *a_pOutBuffer); 
    //
    virtual MINT32 mHalCamReleaseVdoFrame(MVOID * a_pInBuffer); 
    //
    MINT32 mHalCam3ACtrl(MINT32 isAEEnable, MINT32 isAWBEnable, MINT32 isAFEnable);
    //
protected:
    //
//private:
    //
    MVOID mHalCamSetState(mHalCamN3dState_e newState);
    //
    MVOID mHalCamWaitState(mHalCamN3dState_e waitState);
    //
    MINT32 mHalCamStop();
    //
    MINT32 mHalCam3AInit(MUINT32 const u4CamMode, mhalCam3AParam_t const &prCam3AParam);
    //
    MINT32 mHalCam3AProc();
    //
    MINT32 mHalCamSet3AParams(mhalCam3AParam_t const *p3AParamNew, mhalCam3AParam_t const *p3AParamOld);
    //
    MINT32 mHalCamWaitPreviewStable();
    //
    MINT32 mHalCamVideoProc();
    //
    MVOID mHalCamShowParam(mhalCamParam_t *pmhalCamParam);
    //
    MINT32 mHalCamGetCaptureInfo(mhalCamRawImageInfo_t *prawImageInfo, MINT32 mode);
    //
    MINT32 mHalCamSetIspMode(MINT32 ispCamMode, MINT32 ispOperationMode);
    //
    MINT32 mHalCamSetFlashlightParameter(MINT32 *pParam);
    //
    void mHalSetResMgr(MUINT32 const mode);
    //
public:
    MVOID  mHalCamCBHandle(MUINT32 type, MVOID *pdata, MUINT32 const u4DataSize = 0);
    //
private:
    //
    static MVOID * _videoRecordThread(MVOID *arg); 
    //
#if (MHAL_CAM_VIDEO_THREAD)    
        MVOID* mHalCamVideoRecordThread();  
#endif 	

protected: 
    //
    MINT32    configDispPort(mhalCamParam_t* pmhalCamParam, 
                             ICameraIO::ImgCfg_t* pDispCfg, 
                             ICameraIO::MemInfo_t* pDispMemCfg
                             ); 
    //

    
protected:  ////
    NSFeature::ECamRole_T meFeatureCamRole;
    Hal3ABase *mpHal3AObj;
    IspHal *mpIspHalObj;
    ICameraIO *mpCameraIOObj;

    halFLASHLIGHTBase *mpFlashlightObj;
    //
    // YUV sensor add Global variable
    halIspSensorType_e meSensorType;
    //
    MINT32 mAFBestPos;
    volatile MBOOL  mIsVideoRecording;

    MINT32 mCameraId;

    //
    volatile mHalCamN3dState_e mCameraState;
    mhalCamParam_t mmHalCamParam;
    MUINT32 mAaaWaitVDNum;
    mHalCamN3dState_e mAaaState;
    MUINT32 mIsFocusCBDone;
    mhalCamRawImageInfo_t mmHalRawImageInfo;
    //
    ResMgrHal* mResMgrHalObj;
    //
    //mHalCamMemPool *mVdoWorkingBuf;
    MUINT32 mCamDumpOPT; 
    //
    IShotN3D*          mpShot;
    MINT32 volatile mVdoBusyFrmCnt;
    //
    mutable Mutex mLock;
    // 

public:
    // Video frame info 
    struct VideoFrame {        
        MUINT32 hwIndex; 
        MUINT32 fillCnt; 
        MUINT32 bufAddr; 
        MUINT32 timeStampS; 
        MUINT32 timeStampUs; 
    
        VideoFrame() : hwIndex(0), fillCnt(0), bufAddr(0), timeStampS(0), timeStampUs(0) {}
    };
    typedef List<VideoFrame>   VideoFrameQueue_t ;    
private: 
    mutable Mutex mVdoFrameLock; 
    VideoFrameQueue_t       mVdoFrameQueue;    //  Video Frame Queue 
    MINT32 postVideoFrame(VideoFrame const &rVdoFrame); 
    MINT32 getVideoFrame(VideoFrame &rVdoFrame); 
        
#if (MHAL_CAM_VIDEO_THREAD)    
        pthread_t mVideoRecordThread;
#endif 

private: 
    volatile MUINT32 mFrameCnt;

public:

    MINT32 mPreAfStatus;
    MINT32 mCurAfStatus;
    // AF callback data
    struct AfCallBackData
    {
        MUINT32 isFocused;
        MUINT32 afSize;
        AF_WIN_RESULT_T winResult;
    };
    
    //!--
//
};


#endif

