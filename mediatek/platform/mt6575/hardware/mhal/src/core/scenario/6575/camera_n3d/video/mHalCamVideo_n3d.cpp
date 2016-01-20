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
//
#define LOG_TAG "scenario/mHalVdoN3D"
#include <utils/threads.h>
#include <sys/prctl.h>

//
#include <mhal/inc/MediaHal.h>
#include <mhal/inc/camera.h>
//
#include <mcam_log.h>
#include <mcam_profile.h>

#include "mhal_cam_n3d.h"

//
#ifdef  MHAL_CAM_FLICKER_SERVICE_SUPPORT
  #include <frameservice/IFlickerService.h>
#endif

static int MCAM_LOG_DBG = 1;  

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCamN3D::mHalCamVideoRecordStart(
    VOID *a_pInBuffer,
    MUINT32 a_u4InBufSize
)
{
    MINT32 err = MHAL_NO_ERROR;
    //
    MCAM_DBG("[mHalCamVideoRecordStart] caller:%d, \n ", gettid()); 
    //
    Mutex::Autolock _l(mLock); 
    //
#ifdef  MHAL_CAM_FLICKER_SERVICE_SUPPORT
    //  Disable auto-flicker detection when starting recording.
    if  ( NSCamera::IFrameService* pService = mvpFrameService[NSCamera::eFSID_Flicker] )
    {
        if  ( ! NSCamera::IFlickerService::FSCmd_EnableAutoDetect(pService, MFALSE).execute() )
        {
            MCAM_ERR("[mHalCamVideoRecordStart] IFlickerService::FSCmd_EnableAutoDetect(0)");
        }
    }
#endif
    //
    mHalSetResMgr(RES_MGR_HAL_MODE_VIDEO_REC_ON);  
    mIsVideoRecording = MTRUE;
    mVdoBusyFrmCnt = 0; 
    // Create a thread to do video record
    #if (MHAL_CAM_VIDEO_THREAD)
    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_RR, RTPM_PRIO_CAMERA_RECORD};
    ::pthread_create(&mVideoRecordThread, &attr, _videoRecordThread, this);
    #endif 
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCamN3D::mHalCamVideoRecordStop(
    VOID *a_pInBuffer,
    MUINT32 a_u4InBufSize
)
{
    MINT32 err = MHAL_NO_ERROR;
    //
    MCAM_DBG("[mHalCamVideoRecordStop] E, caller:%d \n", gettid());
    //
    Mutex::Autolock _l(mLock); 
    // Wait preview indicator thread to finish its job 
#if (MHAL_CAM_VIDEO_THREAD) 
    if (mVideoRecordThread != 0) {
        // signal the frame wait condition 
        mFrameInCndLock.lock(); 
        mWaitFrameCnd.broadcast(); 
        mFrameInCndLock.unlock(); 
        // do wait video record thread join, due to the the video record thread 
        // will callback to upper layer and do record stop. 
        //::pthread_join(mVideoRecordThread, NULL);
        mVideoRecordThread = 0; 
    }
#endif
    //
    mHalSetResMgr(RES_MGR_HAL_MODE_VIDEO_REC_OFF);
    //
#ifdef  MHAL_CAM_FLICKER_SERVICE_SUPPORT
    //  Restore flicker mode when stopping recording.
    if  ( NSCamera::IFrameService* pService = mvpFrameService[NSCamera::eFSID_Flicker] )
    {
        if  ( ! NSCamera::IFlickerService::FSCmd_SetFlickerMode(pService, mmHalCamParam.cam3AParam.antiBandingMode, MFALSE).execute() )
        {
            MCAM_ERR("[mHalCamVideoRecordStop] IFlickerService::FSCmd_SetFlickerMode(%d)", mmHalCamParam.cam3AParam.antiBandingMode);
        }
    }
#endif
    mIsVideoRecording = MFALSE;
    MCAM_DBG("[mHalCamVideoRecordStop] mVideoBusyFrameCnt = %d\n", mVdoBusyFrmCnt); 
#if 0 
    // There still buffer not release by upper layer, release it 
    for (int i = 0; i < mVdoBusyFrmCnt; i++) {
        ICameraIO::BuffInfo_t rBufInfo; 
        mpCameraIOObj->releaseN3DPreviewFrame(ICameraIO::ePREVIEW_VIDEO_PORT, &rBufInfo);                 
    }
#endif 
    
    MCAM_DBG("[mHalCamVideoRecordStop] X\n");
    //
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCamN3D::mHalCamReleaseVdoFrame(MVOID * a_pInBuffer)
{
    MINT32 *pIdx = (MINT32 *)a_pInBuffer; 

    android_atomic_dec(&mVdoBusyFrmCnt); 
    MCAM_DBG("[mHalCamReleaseVdoFrame] (pIdx, busyCnt, caller) = (%d, %d, %d) \n", *pIdx, mVdoBusyFrmCnt, gettid()); 

    //ICameraIO::BuffInfo_t rBufInfo; 
//   return mpCameraIOObj->releaseN3DPreviewFrame(ICameraIO::ePREVIEW_VIDEO_PORT, &rBufInfo); 
    return 0;
}

 
/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCamN3D::mHalCamVideoProc(
)
{
    MINT32 err = MHAL_NO_ERROR;
    VideoFrame rVideoFrame; 
    if (0 == getVideoFrame(rVideoFrame)) 
    {
        mhalCamTimeStampBufCBInfo rCamTimeStampBufCB(rVideoFrame.hwIndex, 
                                                                              rVideoFrame.timeStampS, 
                                                                              rVideoFrame.timeStampUs); 
        android_atomic_inc(&mVdoBusyFrmCnt); 
        MCAM_DBG1("[mHalCamVideoProc] Video CB (index, stampS, stampUs, busyCnt) = (%d, %d, %d, %d)\n", 
                                      rVideoFrame.hwIndex, rVideoFrame.timeStampS, rVideoFrame.timeStampUs, mVdoBusyFrmCnt); 

        mHalCamCBHandle(MHAL_CAM_CB_VIDEO_RECORD, &rCamTimeStampBufCB);             
    }
    return err;
}


/*******************************************************************************
*
********************************************************************************/
#if (MHAL_CAM_VIDEO_THREAD)
MVOID *
mHalCamN3D::_videoRecordThread(
    MVOID *arg
)
{
    int policy = 0, priority = 0;
    getPriority(policy, priority);    
    ::prctl(PR_SET_NAME,"_videoRecordThread",0,0,0);
    MCAM_DBG("[_videoRecordThread] create (pid,tid)=(%d,%d), (policy, priority) = (%d, %d)\n", getpid(), gettid(), policy, priority);
#if 1 
    // detach thread 
    // due to the stop record may be call by the record thread. 
    ::pthread_detach(::pthread_self());    
#endif 

    mHalCamN3D *const pmHalCam = reinterpret_cast<mHalCamN3D*>(arg); 
    if (!pmHalCam ) {
        MCAM_ERR("[_videoRecordThread] Null arg"); 
        return NULL; 
    }
    return  pmHalCam->mHalCamVideoRecordThread();
}

/*******************************************************************************
*
********************************************************************************/
MVOID*
mHalCamN3D::mHalCamVideoRecordThread(
)
{
    MCAM_DBG("[mHalCamVideoRecordThread] create (pid,tid)=(%d,%d)\n", getpid(), gettid());
    MINT32 err = MHAL_NO_ERROR; 
    mhalCamCallbackInfo_t mhalCBInfo; 
    memset(&mhalCBInfo, 0, sizeof(mhalCamCallbackInfo_t)); 
    MUINT32 frameCnt = mFrameCnt; 
    while (mHalCamGetVideoState() != MHAL_CAM_VDO_STATE_IDLE) {
        //check if the frame count is the same 
        //if no frame in, wait for next frame 
        mFrameInCndLock.lock();  
        if (frameCnt == mFrameCnt) {
            mWaitFrameCnd.wait (mFrameInCndLock); 
        }
        // 
        frameCnt = mFrameCnt; 
        mFrameInCndLock.unlock(); 
        // We process the video here 
        if (mHalCamGetVideoState() != MHAL_CAM_VDO_STATE_IDLE) {
            mhalCamParam_t *const pmhalCamParam = reinterpret_cast<mhalCamParam_t*>(&mmHalCamParam); 
            ICameraIO::BuffInfo_t rBufInfo; 
            err = mHalCamPreviewVideo(pmhalCamParam, &mhalCBInfo, &rBufInfo);        
            if (err != MHAL_NO_ERROR) {
                MCAM_ERR("[mHalCamVideoRecordThread] mHalCamPreviewVideo err:0x%x\n", err); 
            }
        }
    }
    MCAM_DBG("[mHalCamVideoRecordThread] exit (pid, tid) = (%d, %d)\n", getpid(), gettid()); 
    return NULL; 
}
#endif 


/******************************************************************************
*
*******************************************************************************/
MINT32 
mHalCamN3D::postVideoFrame(VideoFrame const &rVdoFrame) 
{
    Mutex::Autolock _l(mVdoFrameLock); 
    MCAM_DBG("[postVideoFrame] Index = %d\n", rVdoFrame.hwIndex); 
    mVdoFrameQueue.push_back(rVdoFrame); 
    return 0; 
}

/******************************************************************************
* 1. Get the video frame, 
   @ret: 0: get the frame 
           -1: empty() 
*******************************************************************************/
MINT32 
mHalCamN3D::getVideoFrame(VideoFrame &rVdoFrame) 
{
    Mutex::Autolock _l(mVdoFrameLock);
    //
    if  ( ! mVdoFrameQueue.empty() ) {
        //  If the queue is not empty, take the first command from the queue.
        rVdoFrame = *mVdoFrameQueue.begin();
        mVdoFrameQueue.erase(mVdoFrameQueue.begin());
        MCAM_DBG("[getVideoFrame] Index = %d\n", rVdoFrame.hwIndex);     
    }
    else {
        MCAM_DBG("[getVideoFrame] No video frame in \n"); 
        return -1; 
    }
    return 0;     
}
