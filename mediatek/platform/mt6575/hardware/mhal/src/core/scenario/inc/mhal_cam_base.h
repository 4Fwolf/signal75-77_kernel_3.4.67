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
** $Log: mhal_cam_base.h $
 *
*/

#ifndef _MHAL_CAM_BASE_H
#define _MHAL_CAM_BASE_H


struct mhalCamSensorInfo_s;


/*******************************************************************************
*
********************************************************************************/
typedef enum MHalCamCmd_s {
    CAM_CMD_SET_SENSOR_DEV      = 0x1001,
    CAM_CMD_SET_SMILE_PREVIEW   = 0x1002,
    CAM_CMD_DO_PANORAMA         = 0x1003,
    CAM_CMD_CANCEL_PANORAMA     = 0x1004,
    CAM_CMD_SET_FLASHLIGHT_PARAMETER = 0x1005,
    CAM_CMD_START_MAV           = 0x1006,
    CAM_CMD_STOP_MAV            = 0x1007,
    CAM_CMD_SET_ATV_DISP        = 0x1008,  
    CAM_CMD_START_AUTORAMA      = 0x1009,
    CAM_CMD_STOP_AUTORAMA       = 0x100A,   
    CAM_CMD_START_3DSHOT        = 0x100B,
    CAM_CMD_STOP_3DSHOT         = 0x100C,    
    CAM_CMD_SET_FACE_PREVIEW    = 0x100D,  //!++ ICS FD
    CAM_CMD_SET_SHOT_MODE       = 0x100E,
    CAM_CMD_VIDEO_SNAPSHOT      = 0x100F,
    CAM_CMD_CANCEL_PICTURE      = 0x1010,
    //
    CAM_CMD_GET_SUPPORTED_SENSOR_DEV = 0x2001,
    CAM_CMD_GET_RAW_IMAGE_INFO  = 0x2002,
    CAM_CMD_GET_BS_INFO         = 0x2003,
    CAM_CMD_GET_ATV_DISP_DELAY  = 0x2005, 
    CAM_CMD_GET_BUF_SUPPORT_FMT = 0x2006, 
    CAM_CMD_GET_CAM_BUF_INFO = 0x2007, 
    CAM_CMD_GET_3A_SUPPORT_FEATURE = 0x2008, 
    CAM_CMD_CAM_MAX             = 0xFFFF
} MHalCamCmd_e;

/*******************************************************************************
*
********************************************************************************/
class mHalCamBase 
{
public:
    /////////////////////////////////////////////////////////////////////////
    //
    // searchCamera () -
    //! \brief search number of camera
    //
    /////////////////////////////////////////////////////////////////////////
    static MINT32 searchCamera(MVOID *a_pOutBuffer, MUINT32 *pBytesReturned);
    
    /////////////////////////////////////////////////////////////////////////
    //
    // createInstance () -
    //! \brief create mhal cam instance
    //
    /////////////////////////////////////////////////////////////////////////
    static mHalCamBase* createInstance();

    /////////////////////////////////////////////////////////////////////////
    //
    // destroyInstance () -
    //! \brief destroy mhal cam instance
    //
    /////////////////////////////////////////////////////////////////////////
    virtual void destroyInstance() = 0;

    // mhal_cam.cpp
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamBase () -
    //! \brief mhal cam constructor 
    //
    /////////////////////////////////////////////////////////////////////////                    
    mHalCamBase() {}

    /////////////////////////////////////////////////////////////////////////
    //
    // ~mhalCamBase () -
    //! \brief mhal cam base descontrustor 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual ~mHalCamBase() ;

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamPreviewStart () -
    //! \brief set mhal into camera preview start state .
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mHalCamPreviewStart(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamPreviewStop () -
    //! \brief set mhal into camera preview stop state .
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalCamPreviewStop() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamDoFocus () -
    //! \brief set mhal to do focus.
    //
    /////////////////////////////////////////////////////////////////////////        
    virtual MINT32 mHalCamDoFocus() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamCancelFocus () -
    //! \brief set mhal to cancel the focus 
    //
    /////////////////////////////////////////////////////////////////////////            
    virtual MINT32 mHalCamCancelFocus() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamPreCapture () -
    //! \brief Set camera to into pre capture state 
    //
    /////////////////////////////////////////////////////////////////////////            
    virtual MINT32 mHalCamPreCapture() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamCaptureStart () -
    //! \brief Set to start capture 
    //
    /////////////////////////////////////////////////////////////////////////                   
    virtual MINT32 mHalCamCaptureStart(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamSetZoom () -
    //! \brief Set camera to do digital zoom 
    //
    /////////////////////////////////////////////////////////////////////////                       
    virtual MINT32 mHalCamSetZoom(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamSet3AParameter () -
    //! \brief Set the 3A related parameter 
    //
    /////////////////////////////////////////////////////////////////////////                       
    virtual MINT32 mHalCamSet3AParameter(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamCaptureStart () -
    //! \brief init the camera capture init 
    //
    /////////////////////////////////////////////////////////////////////////                       
    virtual MINT32 mHalCamCaptureInit(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamCaptureUninit () -
    //! \brief uninit the camera capture releated setting 
    //
    /////////////////////////////////////////////////////////////////////////                       
    virtual MINT32 mHalCamCaptureUninit(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamVideoRecordStart () -
    //! \brief start to do video start
    //
    /////////////////////////////////////////////////////////////////////////                       
    virtual MINT32 mHalCamVideoRecordStart(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamVideoRecordStop () -
    //! \brief set the video  to stop recording 
    //
    /////////////////////////////////////////////////////////////////////////                       
    virtual MINT32 mHalCamVideoRecordStop(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamSetDispParameter () -
    //! \brief set the display paramter for surface fringer 
    //
    /////////////////////////////////////////////////////////////////////////                       
    virtual MINT32 mHalCamSetDispParameter(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize) {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamInit () -
    //! \brief init the mhal cam 
    //
    /////////////////////////////////////////////////////////////////////////      
    virtual MINT32 mHalCamInit(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamUninit () -
    //! \brief unint the mhal cam 
    //
    /////////////////////////////////////////////////////////////////////////                     
    virtual MINT32 mHalCamUninit() {return 0;}

    //mhal_feature.cpp
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamSetFeatureMode () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mHalCamSetFeatureMode(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamGetFeatureEnum () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mHalCamGetFeatureEnum(MVOID*const a_pOutBuf, MUINT32 const a_u4OutBufNum, MUINT32& ra_u4NumReturned) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamSendCommand () -
    //! \brief 
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mHalCamSendCommand(MINT32 cmd, MINT32 arg1 = 0, MINT32 arg2 = 0, MINT32 arg3 = 0) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamSetJpgForPreConfig () -
    //! \brief pre-config jpg for allocating M4U buffer to reduce capture time.
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mHalCamSetJpgForPreConfig(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamGetBufMemInfo () -
    //! \brief get the buffer memory info
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mHalCamGetBufMemInfo(MVOID *a_pOutBuffer) = 0; 
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamReleaseVdoFrame () -
    //! \brief to release a video frame. 
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mHalCamReleaseVdoFrame(MVOID *a_pInBuffer) {return 0;} 

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalCamQuerySensorInfo () -
    //! \brief to query sensor information. 
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 mHalCamQuerySensorInfo(MINT32 cameraId, mhalCamSensorInfo_s* pInfo);

    //
    static mhalCamSensorInfo_s mHalCamSensorInfo[4];
    //
    static MINT32 mRetFakeSubOrientation;
    //
    static MUINT32 mu4Camera3dId;
    //
protected:
    //
private:
    //
};


#endif

