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

#define LOG_TAG "mHalMain"
//
#include "MediaHal.h"
#include <mhal/inc/camera/ioctl.h>
//
#include "mhal_main.h"
#include "mhal_cam_base.h"

/******************************************************************************
*
*******************************************************************************/
#include <cutils/xlog.h>
#define MY_LOGV(fmt, arg...)        XLOGV(fmt, ##arg)
#define MY_LOGD(fmt, arg...)        XLOGD(fmt, ##arg)
#define MY_LOGI(fmt, arg...)        XLOGI(fmt, ##arg)
#define MY_LOGW(fmt, arg...)        XLOGW(fmt, ##arg)
#define MY_LOGE(fmt, arg...)        XLOGE(fmt" (%s){#%d:%s}", ##arg, __FUNCTION__, __LINE__, __FILE__)
//
//  ASSERT
#define MY_LOGA(...) \
    do { \
        MY_LOGE("[Assert] "__VA_ARGS__); \
        MY_LOGE("(%s){#%d:%s}", __FUNCTION__, __LINE__, __FILE__); \
        while(1) { ::usleep(500 * 1000); } \
    } while (0)


/******************************************************************************
*
*******************************************************************************/
int
mHalCamIoctl(
    void*           fd, 
    unsigned int    uCtrlCode, 
    void*           pvInBuf, 
    unsigned int    uInBufSize, 
    void*           pvOutBuf, 
    unsigned int    uOutBufSize, 
    unsigned int*   puBytesReturned
)
{
    MINT32 err = MHAL_NO_ERROR;
    //
    //
    mHalOpenInfo*const pOpen = reinterpret_cast<mHalOpenInfo*>(fd);
    if  ( NULL == pOpen )
    {
        MY_LOGA("NULL file descriptor for IO Code(%x)", uCtrlCode);
        return  -1;
    }
    //
    //--------------------------------------------------------------------------
    //  io codes with the following cases are handled here.
    //
    //  * pmHalCam is unneeded
    //  * pmHalCam is created
    //  * pmHalCam is destroyed
    //
    //--------------------------------------------------------------------------
    switch  (uCtrlCode)
    {
    case MHAL_IOCTL_SEARCH_CAMERA:
        err = mHalCamBase::searchCamera(pvOutBuf, puBytesReturned);
        return  err;
    //
    case MHAL_IOCTL_INIT_CAMERA:
        pOpen->priv = mHalCamBase::createInstance();
        if  ( ! pOpen->priv ) {
            err = MHAL_UNKNOWN_ERROR;
            return  err;
        }
        err = reinterpret_cast<mHalCamBase*>(pOpen->priv)->mHalCamInit(pvInBuf, uInBufSize);
        return  err;
    //
    case MHAL_IOCTL_UNINIT_CAMERA:
        if  ( mHalCamBase*const pmHalCam = reinterpret_cast<mHalCamBase*>(pOpen->priv) )
        {
            err = pmHalCam->mHalCamUninit();
            pmHalCam->destroyInstance();
        }
        pOpen->priv = NULL;
        return  err;
    //
    default:
        break;
    }
    //
    //
    //--------------------------------------------------------------------------
    //  io codes with the following cases are handled here.
    //
    //  * pmHalCam is needed
    //
    //--------------------------------------------------------------------------
    mHalCamBase*const pmHalCam = reinterpret_cast<mHalCamBase*>(pOpen->priv);
    if  ( NULL == pmHalCam )
    {
        MY_LOGA("NULL pmHalCam for IO Code(%x)", uCtrlCode);
        return  -1;
    }
    //
    switch (uCtrlCode)
    {
    case MHAL_IOCTL_PREVIEW_START:
        err = pmHalCam->mHalCamPreviewStart(pvInBuf, uInBufSize);
        break;
    //
    case MHAL_IOCTL_PREVIEW_STOP:
        err = pmHalCam->mHalCamPreviewStop();
        break;
    //
    case MHAL_IOCTL_DO_FOCUS:
        err = pmHalCam->mHalCamDoFocus();
        break;
    //
    case MHAL_IOCTL_CANCEL_FOCUS:
        err = pmHalCam->mHalCamCancelFocus();
        break;
    //
    case MHAL_IOCTL_PRE_CAPTURE:
        err = pmHalCam->mHalCamPreCapture();
        break;
    //
    case MHAL_IOCTL_CAPTURE_START:
        err = pmHalCam->mHalCamCaptureStart(pvInBuf, uInBufSize);
        break;
    //
    case MHAL_IOCTL_CAPTURE_CANCEL:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_CANCEL_PICTURE, (MINT32)pvInBuf, uInBufSize);
        break;
    //
    case MHAL_IOCTL_SET_SHOT_MODE:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_SET_SHOT_MODE, (unsigned int)pvInBuf);
        break;
    //
    case MHAL_IOCTL_CAPTURE_INIT:
        err = pmHalCam->mHalCamCaptureInit(pvInBuf, uInBufSize);
        break;
    //
    case MHAL_IOCTL_CAPTURE_UNINIT:
        err = pmHalCam->mHalCamCaptureUninit(pvInBuf, uInBufSize);
        break;
    //
    case MHAL_IOCTL_SET_ZOOM:
        err = pmHalCam->mHalCamSetZoom(pvInBuf, uInBufSize);
        break;
    //
    case MHAL_IOCTL_SET_CAM_3A_PARAMETER:
        err = pmHalCam->mHalCamSet3AParameter(pvInBuf, uInBufSize);
        break;
    //
    case MHAL_IOCTL_SET_CAM_FEATURE_MODE:
        err = pmHalCam->mHalCamSetFeatureMode(pvInBuf, uInBufSize);
        break;
    //
    case MHAL_IOCTL_GET_CAM_FEATURE_ENUM:
        {
            MUINT32 u4NumReturned = 0;
            MUINT32 const u4OutBufNum = uOutBufSize;
            err = pmHalCam->mHalCamGetFeatureEnum(pvOutBuf, u4OutBufNum, u4NumReturned);
            if  ( MHAL_NO_ERROR == err && puBytesReturned )
                *puBytesReturned = u4NumReturned;
        }
        break;
    //
    case MHAL_IOCTL_VIDEO_START_RECORD:
        err = pmHalCam->mHalCamVideoRecordStart(pvInBuf, uInBufSize);
        break;
    //
    case MHAL_IOCTL_VIDEO_STOP_RECORD:
        err = pmHalCam->mHalCamVideoRecordStop(pvInBuf, uInBufSize);
        break;
    //
    case MHAL_IOCTL_SET_CAM_DISP_PARAMETER:
        err = pmHalCam->mHalCamSetDispParameter(pvInBuf, uInBufSize);
        break;
    //
    case MHAL_IOCTL_DO_PANORAMA:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_DO_PANORAMA, (MINT32) pvInBuf);
        break;
    //
    case MHAL_IOCTL_CANCEL_PANORAMA:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_CANCEL_PANORAMA, (MINT32) pvInBuf);
        break;
    //
    case MHAL_IOCTL_SET_FLASHLIGHT_PARAMETER:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_SET_FLASHLIGHT_PARAMETER, (MINT32) pvInBuf);
        break;
    //
    case MHAL_IOCTL_GET_RAW_IMAGE_INFO:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_GET_RAW_IMAGE_INFO, (MINT32) pvOutBuf, *(MINT32*)pvInBuf);
        break;
    //
    case MHAL_IOCTL_GET_BS_INFO:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_GET_BS_INFO, (MINT32) pvOutBuf);
        break;
    //
    case MHAL_IOCTL_START_SD_PREVIEW:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_SET_SMILE_PREVIEW, 1);
        break;
    //        
    case MHAL_IOCTL_CANCEL_SD_PREVIEW:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_SET_SMILE_PREVIEW, 0);
        break;
    //
    case MHAL_IOCTL_START_MAV:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_START_MAV, (MINT32) pvInBuf);
        break;
    //
    case MHAL_IOCTL_STOP_MAV:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_STOP_MAV, (MINT32) pvInBuf);
        break;
    //
    case MHAL_IOCTL_START_AUTORAMA:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_START_AUTORAMA , (MINT32) pvInBuf);
        break;
    //
    case MHAL_IOCTL_STOP_AUTORAMA:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_STOP_AUTORAMA, (MINT32) pvInBuf);
        break; 
    //
    case MHAL_IOCTL_START_3DSHOT:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_START_3DSHOT , (MINT32) pvInBuf);
        break;
    //
    case MHAL_IOCTL_STOP_3DSHOT:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_STOP_3DSHOT, (MINT32) pvInBuf);
        break; 
    case MHAL_IOCTL_START_FACE_DETECTION:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_SET_FACE_PREVIEW, 1);
        break;
    //
    case MHAL_IOCTL_STOP_FACE_DETECTION:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_SET_FACE_PREVIEW, 0);
        break;
    //    
    case MHAL_IOCTL_SET_ATV_DISP:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_SET_ATV_DISP, 1);
        break;
    //
    case MHAL_IOCTL_GET_ATV_DISP_DELAY:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_GET_ATV_DISP_DELAY, (MINT32) pvOutBuf);
        break;
    //
    case MHAL_IOCTL_GET_BUF_SUPPORT_FORMAT:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_GET_BUF_SUPPORT_FMT, (MINT32)pvOutBuf); 
        break; 
    //
    case MHAL_IOCTL_GET_CAM_BUF_MEM_INFO: 
        err = pmHalCam->mHalCamGetBufMemInfo(pvOutBuf); 
        break; 
    //
    case MAHL_IOCTL_RELEASE_VDO_FRAME:
        err = pmHalCam->mHalCamReleaseVdoFrame(pvInBuf); 
        break;         
    //
     case MHAL_IOCTL_GET_3A_SUPPORT_FEATURE:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_GET_3A_SUPPORT_FEATURE, (MINT32)pvOutBuf); 
        break; 
    //
     case MHAL_IOCTL_VIDEO_SNAPSHOT:
        err = pmHalCam->mHalCamSendCommand(CAM_CMD_VIDEO_SNAPSHOT, (MINT32)pvInBuf); 
        break;
    //
    case MHAL_IOCTL_QUERY_SENSOR_INFO:
        err = pmHalCam->mHalCamQuerySensorInfo((MINT32)pvInBuf, (mhalCamSensorInfo_s*)pvOutBuf);
        break;
    //
    default:
        MY_LOGA("unknown IO Code(%x)", uCtrlCode);
        break;
    }

    return err;
}

