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
                                
#ifndef _MHAL_CAM_H
#define _MHAL_CAM_H

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
#define MHAL_CAM_FD_OPTION              1       // FD On/Off
#define MHAL_CAM_ISP_TUNING             1       // ISP Tuning On/Off
#define MHAL_CAM_EIS_OPTION             1       // EIS on/off
#define MHAL_CAM_MAV_OPTION             1       // MAV on/off
#define MHAL_CAM_AUTORAMA_OPTION        1       // AUTORAMA on/off
#define MHAL_CAM_DRAW_OPTION            1       // For LCM and Sensor relation
#define MHAL_CAM_ASD_OPTION             1       //Auto Scene Detect
#define MHAL_CAM_FRAME_SERVICE_SUPPORT  1       //frame service support on/off
#define MHAL_CAM_3DSHOT_OPTION          1       // 3D shot on/off
#define MHAL_CAM_VIDEO_THREAD           0       // Video thread on
#define MHAL_CAM_VIDEO_SNAPSHOT         1       // Video snapshot
 
#if MHAL_CAM_EIS_OPTION 
#include "eis_hal_base.h"
#endif

#if (MHAL_CAM_ASD_OPTION)
#include "./asd/Mhal_ASD.h"
#endif

#if MHAL_CAM_VIDEO_SNAPSHOT
#include "./videosnapshot/VideoSnapshot.h"
#endif

#if (MHAL_CAM_FRAME_SERVICE_SUPPORT)
  #include <frameservice/IFrameService.h>
#endif

//
#define MHAL_CAM_FEATURE_OFF    0
#define MHAL_CAM_FEATURE_ON     1

using namespace android;
using namespace NSFeature;

/*******************************************************************************
*
********************************************************************************/
class IShot;
class mhal_featureBase;
class mHalFDProvider;
class mHalCamFD;
/*******************************************************************************
*
********************************************************************************/
typedef enum mHalCamState_enum {
    MHAL_CAM_INIT           = 0x0000,
    MHAL_CAM_IDLE           = 0x0001,
    //
    MHAL_CAM_PREVIEW_MASK   = 0x0100,
    MHAL_CAM_PREVIEW        = 0x0100,
    MHAL_CAM_FOCUS          = 0x0101,
    MHAL_CAM_ZOOM           = 0x0102,
    //
    MHAL_CAM_CAPTURE_MASK   = 0x0200,
    MHAL_CAM_PRE_CAPTURE    = 0x0200,
    MHAL_CAM_CAPTURE        = 0x0201,
    MHAL_CAM_PANORAMA       = 0x0202,
    //
    MHAL_CAM_STOP           = 0x0400,
    //
    MHAL_CAM_UNINIT         = 0x0800,
    //
    MHAL_CAM_ERROR          = 0x8000,
} mHalCamState_e;




/*******************************************************************************
*
********************************************************************************/
class mHalCam : public mHalCamBase {
public:
    //
    static mHalCamBase* getInstance();
    //
    virtual void destroyInstance();
    //
    mHalCam();
    //
    virtual ~mHalCam();
    //
    virtual MINT32 mHalCamPreviewStart(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize);
    //
    virtual MINT32 mHalCamPreviewStop();
    //
    virtual MINT32 mHalCamDoFocus();
    //
    virtual MINT32 mHalCamCancelFocus();
    //
    virtual MINT32 mHalCamPreCapture();
    //
    virtual MINT32 mHalCamCaptureStart(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize);
    //
    virtual MINT32 mHalCamSetZoom(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize);
    //
    virtual MINT32 mHalCamSet3AParameter(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize);
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
    mHalCamState_e mHalCamGetState();
    //
    virtual MINT32 mHalCamPreviewProc();
    //
    virtual MINT32 mHalCamFDProc();
    //
    virtual MINT32 mHalCamCaptureProc();
    //
    virtual MINT32 mHalCamPanoProc();
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
    MVOID mHalCamSetState(mHalCamState_e newState);
    //
    MVOID mHalCamWaitState(mHalCamState_e waitState);
    //
    MINT32 mHalCamStop();
    //
    MINT32 mHalCam3AInit(MUINT32 const u4CamMode, mhalCam3AParam_t const &prCam3AParam, MUINT32 u4IsZSDMode);
    //
    MINT32 mHalCam3AProc();
    //
    MINT32 mHalCamSet3AParams(mhalCam3AParam_t const *p3AParamNew, mhalCam3AParam_t const *p3AParamOld);
    //
    MINT32 mHalCamWaitPreviewStable();
    //
    MINT32 mHalCamVideoProc();
    //
    MINT32 mHalCamSetPreviewZoom(MUINT32 zoomVal);

    //
    MVOID mHalCamShowParam(mhalCamParam_t *pmhalCamParam);
    //
    MINT32 mHalCamGetCaptureInfo(mhalCamRawImageInfo_t *prawImageInfo, MINT32 mode);
    //
    MINT32 mHalCamDoPanorama(mhalCamParam_t *pmhalCamParam);
    //
    MINT32 mHalCamCancelPanorama(mhalCamParam_t *pmhalCamParam);
    //
    MINT32 mHalCamSetIspMode(MINT32 ispCamMode, MINT32 ispOperationMode);
    //
    MINT32 mHalCamSetFlashlightParameter(MINT32 *pParam);
    //
    void mHalSetResMgr(MUINT32 const mode);

    //    
    virtual void mHalCamFocusThread(MINT32 enable);
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
    MINT32    configFDPort(ICameraIO::ImgCfg_t* fdCfg,          
                           ICameraIO::MemInfo_t* fdMemCfg       
                           ); 
    //
    MINT32    configVideoPort(mhalCamParam_t* const pmhalCamParam, 
                              ICameraIO::ImgCfg_t* pVideoCfg,              
                              ICameraIO::MemInfo_t* pVideoMemCfg           
                              ); 
    //
    MINT32    configDispPort(mhalCamParam_t* pmhalCamParam, 
                             ICameraIO::ImgCfg_t* pDispCfg, 
                             ICameraIO::MemInfo_t* pDispMemCfg
                             ); 
    //
    MINT32    configZSDPort(mhalCamParam_t* pmhalCamParam, 
                             ICameraIO::ImgCfg_t* pZSDCfg, 
                            ICameraIO::MemInfo_t* pZSDMemCfg,
                            MINT32 pZSDPass
                             );
    //
    #if (MHAL_CAM_VIDEO_SNAPSHOT)
    MINT32    configVSSFDPort(  ICameraIO::ImgCfg_t* fdCfg,          
                                    ICameraIO::MemInfo_t* fdMemCfg       
                                    );
    #endif
    //
    MINT32    setEISConfig(MUINT32 const ispOutW, 
                           MUINT32 const ispOutH, 
                           MUINT32 const targetW, 
                           MUINT32 const targetH
                           ); 
    //
    MINT32    mHalCamPreviewZoomProc();
    //
    static MINT32 mHalCamIOCB(MINT32 msgType, MINT32 arg1, MINT32 arg2, MVOID *user); 
    //
    MINT32    mHalCamEISProc(MINT32 *pGmvX, MINT32 *pGmvY);    
    //
    MINT32    mHalCamFreeZSDMem(void);
    //
    MINT32    mHalCamZsdWaitBuff(MUINT32 onFlash);


protected:  ////
    NSFeature::ECamRole_T meFeatureCamRole;
    Hal3ABase *mpHal3AObj;
    IspHal *mpIspHalObj;
    ICameraIO *mpCameraIOObj; 
    
    halFLASHLIGHTBase *mpFlashlightObj;
    #if MHAL_CAM_EIS_OPTION
    EisHalBase *mpEISHalObj;  
    #endif    

    #if MHAL_CAM_ASD_OPTION
    mHal_ASD *mpHalASDObj;
    #endif
    //
    #if MHAL_CAM_VIDEO_SNAPSHOT
    VideoSnapshot*  mpVideoSnapshot;
    #endif
    //
    mhal_featureBase *mhal3DObj;
    //
    // YUV sensor add Global variable
    halIspSensorType_e meSensorType;
    //
    MINT32 mAFBestPos;
    volatile MBOOL  mIsVideoRecording;

    MINT32 mCameraId;

    //
    volatile mHalCamState_e mCameraState;
    mhalCamParam_t mmHalCamParam;
    MUINT32 mAaaWaitVDNum;
    mHalCamState_e mAaaState;
    MUINT32 mIsFocusCBDone;
    mhalCamRawImageInfo_t mmHalRawImageInfo;
    //
    BOOL mIsAtvDisp;
    ResMgrHal* mResMgrHalObj;
    //    
    mHalCamMemPool *mVdoWorkingBuf;
    mHalCamMemPool *mFDWorkingBuf;  // currently keep it in mHalCam class 
    mHalCamMemPool *mZSDWorkingBuf;
    MUINT32 mCamDumpOPT; 
    //
#if (MHAL_CAM_FRAME_SERVICE_SUPPORT)
    NSCamera::IFrameService*    mvpFrameService[NSCamera::eFSID_NUM];
#endif
    //
    IShot*          mpShot;
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
    List <MUINT32> mZoomQueue; 
    //! 3A part 
    MUINT32 mEISSrcW; 
    MUINT32 mEISSrcH; 
    //
    MUINT32 mDynamicFrameCnt;
    MBOOL   mIsDynamicFpsStable;
    //

public:
	//!++ ICS FD
	#if (MHAL_CAM_FD_OPTION)
    mHalCamFD* mfdObj;
    mHalFDProvider *mfdProvider;
	#endif
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

