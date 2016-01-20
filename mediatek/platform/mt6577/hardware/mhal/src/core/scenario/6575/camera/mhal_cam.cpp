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

#define LOG_TAG "scenario/mHalCam"
#include <utils/Errors.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <utils/threads.h>
//
#include <mhal/inc/MediaHal.h>
#include <mhal/inc/camera.h>
#include <scenario_types.h>
#include <cam_types.h>
#include <content/IContentManager.h>
#include "mhal_misc.h"
//
#include <mcam_log.h>
#include <mcam_profile.h>
#include "mhal_cam.h"
#include "kd_camera_feature.h"
#include "camera_custom_if.h"
//
#include "cam_port.h"
#include "ICameraIO.h"
#include "shot/IShot.h"
#include "shot/Shotbase.h"
#include "shot/NormalShot.h"
#include "shot/IBestShot.h"
#include "shot/BurstShot.h"
#include "shot/ContinuousShot.h"
#include "CameraProfile.h"
//
#ifdef  MHAL_CAM_FLICKER_SERVICE_SUPPORT
  #include <frameservice/IFlickerService.h>
#endif
//
#ifdef  MTK_CAM_HDR_SUPPORT
  #include "hdr/IHdr.h"
#endif
#ifdef  MTK_CAM_FACEBEAUTY_SUPPORT
#include "facebeauty/IFacebeauty.h"
#endif
//

#ifdef  MTK_MAV_SUPPORT
#include "mav/Mhal_MAV.h"
#endif
//
#ifdef  MTK_AUTORAMA_SUPPORT
#include "autorama/Mhal_Autorama.h"
#endif

//
#include "fd/FDProvider.h"
#include "fd/mHalCamFD.h"

//
#include "mhal_interface.h"             //For Mt6575_mHalBitblt

//
//#include <asm/arch/mt6573_sync_write.h>
/*******************************************************************************
*
********************************************************************************/
#define SWAP(a, b) {int c = (a); (a) = (b); (b) = c; }
#define ROUND_TO_2X(x) ((x ) & (~0x1))

// Capture
#define MHAL_CAM_RAW_BUF_SIZE       (0x1000000)
#define MHAL_CAM_THUMB_ADDR_OFFSET  (64 * 1024)
#define MHAL_CAM_JPEG_ADDR_OFFSET   (128 * 1024)
#define MHAL_CAM_HW_JPEG_HEADER_OFFSET   (1 * 1024)

//



//Buffer Cnt
#define PREVIEW_BUFFER_CNT              8
#define VIDEO_BUFFER_CNT                8
#define ATV_BUFFER_CNT                  12
#define FD_BUFFER_CNT                   2
#define ZSD_BUFFER_CNT                  1
#define VSS_BUFFER_CNT                  1
//
#define PREVIEW_DUMP_NO                 30

//m4u video
#if defined(MT6575)
#define USE_VIDEO_M4U
#endif

//
static pthread_t threadHMainHigh;
static pthread_t threadHMainNormal;
static sem_t semMainHigh, semMainHighBack, semMainHighEnd;
static sem_t semFDthread, semFDthreadEnd;
//
static mHalCam* pmHalCamObj = NULL;
//
static int MCAM_LOG_DBG = 0;
//

/*******************************************************************************
* Map ISP Sensor Type to NSCamera Sensor Type
********************************************************************************/
static
NSCamera::ESensorType
mapSensorType(MUINT32 const u4ISPSensorType)
{
    NSCamera::ESensorType eSensorType = NSCamera::eSensorType_Unknown;
    switch  (u4ISPSensorType)
    {
    case ISP_SENSOR_TYPE_RAW:
        eSensorType = NSCamera::eSensorType_RAW;
        break;
    case ISP_SENSOR_TYPE_YUV:
        eSensorType = NSCamera::eSensorType_YUV;
        break;
    default:
        eSensorType = NSCamera::eSensorType_Unknown;
        break;
    }
    return  eSensorType;
}

/*******************************************************************************
* Map ISP Device Type to NSCamera Device ID
********************************************************************************/
static
NSCamera::EDeviceId
mapDeviceID(MUINT32 const u4ISPDeviceType)
{
    NSCamera::EDeviceId eDeviceId = NSCamera::eDevId_Unknown;
    switch  (u4ISPDeviceType)
    {
    case ISP_SENSOR_DEV_MAIN:
        eDeviceId = NSCamera::eDevId_ImgSensor0;
        break;
    case ISP_SENSOR_DEV_SUB:
        eDeviceId = NSCamera::eDevId_ImgSensor1;
        break;
    case ISP_SENSOR_DEV_ATV:
        eDeviceId = NSCamera::eDevId_AtvSensor;
        break;
    default:
        eDeviceId = NSCamera::eDevId_Unknown;
        break;
    }
    return  eDeviceId;
}

/*******************************************************************************
*  Debug FPS, to debug the frame rate per second
*  @checkFPS ()
*     in: the fps criteria
********************************************************************************/
class DebugFPS
{
    char const*const mName;
    MINT32 mFPS;
    nsecs_t mStartTime, mEndTime;
public:
    DebugFPS(char const* const name):mName(name), mFPS(0), mStartTime(systemTime()), mEndTime(systemTime()) {}
    ~DebugFPS() {}
    void checkFPS(int const fps)
    {
        mFPS++;
        mEndTime = systemTime();
        nsecs_t diff = mEndTime - mStartTime;
        if (diff > ms2ns(990)) {
            if (mFPS != fps) {
                MCAM_DBG("[%s]Current fps: %d \n", mName, mFPS);
            }
            mFPS = 0;
            mStartTime = mEndTime;
        }
    }
};

/*******************************************************************************
*
********************************************************************************/
MVOID mHalCam::mHalSetResMgr(MUINT32 const mode)
{
    //
    RES_MGR_HAL_MODE_STRUCT    rModeInfo;
    rModeInfo.Mode = (RES_MGR_HAL_MODE_ENUM)mode;
    rModeInfo.ModeSub = RES_MGR_HAL_MODE_SUB_NONE;
    //
    if (mHalCamSensorInfo[mCameraId].devType == ISP_SENSOR_DEV_ATV)
    {
        rModeInfo.Dev = RES_MGR_HAL_DEV_ATV;
    }
    else
    {
        if(mmHalCamParam.u4CamMode == MHAL_CAM_MODE_VT)
        {
            rModeInfo.Dev = RES_MGR_HAL_DEV_VT;
        }
        else
        {
            rModeInfo.Dev = RES_MGR_HAL_DEV_CAM;
        }
    }

    MCAM_DBG("[mHalSetResMgr]Mode=%d, ModeSub=%d, Dev=%d",rModeInfo.Mode, rModeInfo.ModeSub, rModeInfo.Dev);

    if(!(mResMgrHalObj->SetMode(&rModeInfo)))
    {
        MCAM_ERR("[mHalSetResMgr]Resource check fail.");
    }
}

//FIXME, add for ldvt build fail
/*******************************************************************************
*
********************************************************************************/
UINT32 mHalMiscDumpToFile(
    char *fname,
    UINT8 *pbuf,
    UINT32 size
)
{
    int nw, cnt = 0;
    UINT32 written = 0;

    MCAM_DBG("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        MCAM_ERR("failed to create file [%s]: %s", fname, strerror(errno));
        return MHAL_UNKNOWN_ERROR;
    }

    LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd, pbuf + written, size - written);
        if (nw < 0) {
            MCAM_ERR("failed to write to file [%s]: %s", fname, strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamGetBufMemInfo(MVOID *a_pOutBuffer)
{
    using namespace NSCamera;
    mhalCamBufMemInfo_t *pCamBufMemInfo = (mhalCamBufMemInfo_t*) a_pOutBuffer;

    switch (pCamBufMemInfo->bufID) {
        case MHAL_CAM_BUF_PREVIEW:
            pCamBufMemInfo->camBufCount = PREVIEW_BUFFER_CNT;
            pCamBufMemInfo->camBufFmt   = ePIXEL_FORMAT_I420;
            pCamBufMemInfo->camBufSize = pCamBufMemInfo->frmW * pCamBufMemInfo->frmH * 3 / 2;
            pCamBufMemInfo->camMemType = MHAL_CAM_ASHMEM_TYPE;
            break;
        case MHAL_CAM_BUF_PREVIEW_ATV:
            pCamBufMemInfo->camBufCount = ATV_BUFFER_CNT;
            pCamBufMemInfo->camBufFmt   = ePIXEL_FORMAT_I420;
            pCamBufMemInfo->camBufSize = pCamBufMemInfo->frmW * pCamBufMemInfo->frmH * 3 / 2;
            pCamBufMemInfo->camMemType = MHAL_CAM_ASHMEM_TYPE;
            break;
        case MHAL_CAM_BUF_VIDEO:
            pCamBufMemInfo->camBufCount = VIDEO_BUFFER_CNT;
            pCamBufMemInfo->camBufFmt   = ePIXEL_FORMAT_I420;
            pCamBufMemInfo->camBufSize = pCamBufMemInfo->frmW * pCamBufMemInfo->frmH * 3 / 2;
            pCamBufMemInfo->camMemType = MHAL_CAM_ASHMEM_TYPE;
            break;
        case MHAL_CAM_BUF_VIDEO_BS:
            pCamBufMemInfo->camBufCount = 16;
            pCamBufMemInfo->camBufFmt   = ePIXEL_FORMAT_UNKNOWN;
            pCamBufMemInfo->camBufSize = 1024 * 1024;
#ifdef USE_VIDEO_M4U
            pCamBufMemInfo->camMemType = MHAL_CAM_ASHMEM_TYPE;
#else
            pCamBufMemInfo->camMemType = MHAL_CAM_PMEM_TYPE;
#endif
            break;
        case MHAL_CAM_BUF_CAPTURE:
            pCamBufMemInfo->camBufCount = 1;
            pCamBufMemInfo->camBufFmt   = ePIXEL_FORMAT_UNKNOWN;
            pCamBufMemInfo->camMemType = MHAL_CAM_ASHMEM_TYPE;
            {
                MINT32 err = 0;
                halISPRawImageInfo_t ispRawInfo;
                MUINT32 capBufSize = 0;
                //
                err = mpIspHalObj->sendCommand(ISP_CMD_GET_RAW_INFO, (int) &ispRawInfo);
                if (0 > err) {
                    MCAM_ERR("[mHalCamGetBufMemInfo]<MHAL_CAM_BUF_CAPTURE> ISP_CMD_GET_RAW_INFO - err(%x)", err);
                    return err;
                }
                MUINT32 const u4CapSize = MHAL_CAM_JPEG_ADDR_OFFSET + (2 * pCamBufMemInfo->frmW * pCamBufMemInfo->frmH) + MHAL_CAM_HW_JPEG_HEADER_OFFSET;
                MUINT32 const u4RawSize = MHAL_CAM_JPEG_ADDR_OFFSET + (2 * ispRawInfo.u4Width * ispRawInfo.u4Height) + MHAL_CAM_HW_JPEG_HEADER_OFFSET;
                capBufSize = (u4CapSize >= u4RawSize) ? u4CapSize : u4RawSize;
                capBufSize = (capBufSize + (L1_CACHE_BYTES)) & ~(L1_CACHE_BYTES-1);
                pCamBufMemInfo->camBufSize = capBufSize;
                MCAM_DBG("[mHalCamGetBufMemInfo]<MHAL_CAM_BUF_CAPTURE> L1_CACHE_BYTES(%d) mu4CaptureBufSize(%d)", L1_CACHE_BYTES, capBufSize);
            }
            break;
        case MHAL_CAM_BUF_POSTVIEW:
            pCamBufMemInfo->camBufCount = 1;
            //pCamBufMemInfo->camBufFmt   = ePIXEL_FORMAT_RGBA8888;
            //pCamBufMemInfo->camBufSize = pCamBufMemInfo->frmW * pCamBufMemInfo->frmH * 4;   //
            pCamBufMemInfo->camBufFmt = ePIXEL_FORMAT_I420;
            pCamBufMemInfo->camBufSize = pCamBufMemInfo->frmW * pCamBufMemInfo->frmH * 3 / 2;   //
            pCamBufMemInfo->camMemType = MHAL_CAM_ASHMEM_TYPE;
            break;
        default:
            MCAM_ASSERT(0, "[mHalCamGetBufInfo] unKnow Buffer ID \n");
            break;
    }
    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
mHalCam::mHalCam()
    : mHalCamBase()
    , meFeatureCamRole(NSFeature::ECamRole_Main)
    , mpHal3AObj(NULL)
    , mpIspHalObj(NULL)
    //, mpMdpHalObj(NULL)
    , mpCameraIOObj(NULL)
    , mpFlashlightObj(NULL)
#if (MHAL_CAM_ASD_OPTION)
    , mpHalASDObj(NULL)
#endif
#if (MHAL_CAM_VIDEO_SNAPSHOT)
    ,mpVideoSnapshot(NULL)
#endif
    , mhal3DObj(NULL)
    , mIsVideoRecording(MFALSE)
    , mCameraState(MHAL_CAM_IDLE)
    , mAaaWaitVDNum(0)
    , mAaaState(MHAL_CAM_IDLE)
    , mVdoWorkingBuf(NULL)
    , mFDWorkingBuf(NULL) //!-- ICS FD
    , mCamDumpOPT(0)
    , mpShot(NULL)
    , mVdoBusyFrmCnt(0)
    , mLock()
#if (MHAL_CAM_VIDEO_THREAD)
    , mVdoFrameLock()
    , mVdoFrameQueue()
    , mVideoRecordThread(0)
#endif
    , mFrameCnt(0)
    , mZoomQueue()
    , mEISSrcW(0)
    , mEISSrcH(0)
    , mDynamicFrameCnt(0)
    , mIsDynamicFpsStable(MTRUE)

{
    //mCameraState = MHAL_CAM_IDLE;
    //mAaaState = MHAL_CAM_IDLE;
    mAFBestPos = 0;
    //
    meSensorType = ISP_SENSOR_TYPE_RAW;
    ::memset(&mmHalCamParam, 0 , sizeof(mhalCamParam_t));
    ::memset(&mmHalRawImageInfo, 0, sizeof(mhalCamRawImageInfo_t));
    pmHalCamObj = this;
    mResMgrHalObj = ResMgrHal::CreateInstance();
    mIsAtvDisp = MFALSE;
    mZSDWorkingBuf = NULL;

    mPreAfStatus = AF_MARK_NONE;
    mCurAfStatus = AF_MARK_NONE;

#if (MHAL_CAM_FRAME_SERVICE_SUPPORT)
    ::memset(mvpFrameService, 0, sizeof(mvpFrameService));
#endif
}

/*******************************************************************************
*
********************************************************************************/
mHalCam::~mHalCam()
{
    pmHalCamObj = NULL;
    mHalCamFreeZSDMem();

    if(mResMgrHalObj != NULL)
    {
        mResMgrHalObj->DestroyInstance();
        mResMgrHalObj = NULL;
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID
mHalCam::mHalCamSetState(
    mHalCamState_e newState
)
{
    Mutex::Autolock lock(mLock);

    MCAM_DBG("[mHalCamSetState] Now: 0x%04x, Next: 0x%04x \n", mCameraState, newState);
    //
    if (newState == MHAL_CAM_ERROR) {
        goto MHAL_CAM_SET_STATE_EXIT;
    }
    //
    switch (mCameraState) {
    case MHAL_CAM_INIT:
        switch (newState) {
        case MHAL_CAM_IDLE:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_INIT \n");
            break;
        }
        break;
    //
    case MHAL_CAM_IDLE:
        switch (newState) {
        case MHAL_CAM_IDLE:
        case MHAL_CAM_PREVIEW:
        case MHAL_CAM_CAPTURE:
        case MHAL_CAM_UNINIT:
        case MHAL_CAM_PANORAMA:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_IDLE \n");
            break;
        }
        break;
    //
    case MHAL_CAM_PREVIEW:
        switch (newState) {
        case MHAL_CAM_PREVIEW:
        case MHAL_CAM_FOCUS:
        case MHAL_CAM_ZOOM:
        case MHAL_CAM_STOP:
        case MHAL_CAM_PRE_CAPTURE:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_PREVIEW \n");
            break;
        }
        break;
    //
    case MHAL_CAM_FOCUS:
        switch (newState) {
        case MHAL_CAM_PREVIEW:
        case MHAL_CAM_PRE_CAPTURE:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_FOCUS \n");
            break;
        }
        break;
    //
    case MHAL_CAM_ZOOM:
        switch (newState) {
        case MHAL_CAM_PREVIEW:
        case MHAL_CAM_ZOOM:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_ZOOM \n");
            break;
        }
        break;
    //
    case MHAL_CAM_PRE_CAPTURE:
        switch (newState) {
        case MHAL_CAM_CAPTURE:
        case MHAL_CAM_STOP:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_PRE_CAPTURE \n");
            break;
        }
        break;
    //
    case MHAL_CAM_CAPTURE:
        switch (newState) {
        case MHAL_CAM_IDLE:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_CAPTURE \n");
            break;
        }
        break;
    //
    case MHAL_CAM_PANORAMA:
        switch (newState) {
        case MHAL_CAM_IDLE:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_PANORAMA \n");
            break;
        }
        break;
    //
    case MHAL_CAM_STOP:
        switch (newState) {
        case MHAL_CAM_IDLE:
            break;
        case MHAL_CAM_STOP:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_STOP \n");
            break;
        }
        break;
    //
    case MHAL_CAM_ERROR:
        switch (newState) {
        case MHAL_CAM_STOP:
        case MHAL_CAM_UNINIT:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_STOP \n");
            break;
        }
        break;
    //
    default:
        MCAM_ASSERT(0, "Unknown state \n");
        break;
    }
    //
MHAL_CAM_SET_STATE_EXIT:

    mCameraState = newState;
}

/*******************************************************************************
*
********************************************************************************/
mHalCamState_e
mHalCam::mHalCamGetState(
)
{
    Mutex::Autolock _l(mLock);
    return mCameraState;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
mHalCam::mHalCamWaitState(
    mHalCamState_e waitState
)
{
    volatile mHalCamState_e state;
    state = mCameraState;
    while (state != waitState) {
        usleep(1000);
        state = mCameraState;
    }
}

/*******************************************************************************
*
********************************************************************************/
static mHalCamObserver g_mhalObserver;

MVOID
mHalCam::
mHalCamCBHandle(MUINT32 type, MVOID *pdata, MUINT32 const u4DataSize /*= 0*/)
{
    MCAM_ASSERT(!!g_mhalObserver, "callback is NULL \n");

    // Callback to upper layer
    g_mhalObserver.notify(type, pdata, u4DataSize);
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamSetIspMode(
    MINT32 ispCamMode,
    MINT32 ispOperationMode
)
{
    MINT32 ret = 0;

    MCAM_DBG("[mHalCamSetIspMode] ispCamMode: %d, ispOperationMode: %d \n", ispCamMode, ispOperationMode);
    //
    ret = mpIspHalObj->sendCommand(ISP_CMD_SET_OPERATION_MODE, ispOperationMode);
    if (ret < 0) {
        MCAM_ERR("  ISP_CMD_SET_OPERATION_MODE err(%x)", ret);
        return ret;
    }
    //
    ret = mpIspHalObj->sendCommand(ISP_CMD_SET_CAM_MODE, ispCamMode);
    if (ret < 0) {
        MCAM_ERR("  ISP_CMD_SET_CAM_MODE err(%x)", ret);
        return ret;
    }
    //
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamStop(
)
{
    MINT32 err = MHAL_NO_ERROR;

    CamTimer camTmr("mHalCamStop", MHAL_CAM_GENERIC_PROFILE_OPT);
    //
    err = mpCameraIOObj->stop();
    if (err < 0) {
        MCAM_ERR("[mHalCamStop] mpCameraIOObj->stop() fail \n");
        return err;
    }
    // FIX ME, should exist here ?
    if (mpFlashlightObj)
    {
        mpHal3AObj->set3AZSDMode(0);    // cotta-- initialize
        mpFlashlightObj->mHalFlashlightSetFire(0);
    }
    //
    camTmr.printTime();

    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamSetPreviewZoom(
    MUINT32 zoomVal
)
{
    MINT32 err = MHAL_NO_ERROR;

    MUINT32 sensorW = 0, sensorH = 0;  // check whether use capture frame for video 1080p or not
    err = mpIspHalObj->sendCommand(ISP_CMD_GET_SENSOR_PRV_RANGE, (MINT32)&sensorW, (MINT32)&sensorH);

    if( (1920 == mmHalCamParam.frmYuv.w && 1088 == mmHalCamParam.frmYuv.h)
        && (sensorW < mmHalCamParam.frmYuv.w || sensorH < mmHalCamParam.frmYuv.h)
        && (zoomVal != 100))
    {
        MCAM_DBG("For Video 1080p, not support zoom");
        return err;
    }

    mhalCamParam_t* pmhalCamParam = &mmHalCamParam;
    ICameraIO::Rect_t rcCrop;
    ICameraIO::Rect_t rcDest(0, 0, pmhalCamParam->frmYuv.w, pmhalCamParam->frmYuv.h);
    MUINT32 pZSDpass;

    pZSDpass = pmhalCamParam->camZsdParam.u4ZsdEnable;
    if (pZSDpass){
        err = mpCameraIOObj->setZSDPreviewZoom(zoomVal, &rcDest, &rcCrop, pZSDpass);
        pmhalCamParam->camZsdParam.u4ZsdZoomWidth = rcCrop.w;
        pmhalCamParam->camZsdParam.u4ZsdZoomHeigth = rcCrop.h;

        if ( rcCrop.w * rcCrop.h > pmhalCamParam->camZsdParam.u4ZsdWidth * pmhalCamParam->camZsdParam.u4ZsdHeight )
        {
            // Means CRZ downscale, fix output to ZSD default size
            pmhalCamParam->camZsdParam.u4ZsdZoomWidth = pmhalCamParam->camZsdParam.u4ZsdWidth;
            pmhalCamParam->camZsdParam.u4ZsdZoomHeigth = pmhalCamParam->camZsdParam.u4ZsdHeight;

        }

        MCAM_DBG("[mHalCamPreviewZoom][ZSD]: u4ZsdZoomWidth.w:%d,u4ZsdZoomHeigth.h%d \n",rcCrop.w,rcCrop.h);
    }
    else{
        err = mpCameraIOObj->setPreviewZoom(zoomVal, &rcDest, &rcCrop);
    }
    //
    #if (MHAL_CAM_ISP_TUNING)
    err = mpIspHalObj->sendCommand(ISP_CMD_SET_ZOOM_RATIO, (int) zoomVal);
    MCAM_ASSERT(err >= 0, "ISP_CMD_SET_ZOOM_RATIO \n");
    #endif
    //
    err = mpHal3AObj->setEZoomInfo(rcCrop.x, rcCrop.y, rcCrop.w, rcCrop.h);
    MCAM_ASSERT(err >= 0, "setEZoomInfo \n");
    //
#ifdef  MHAL_CAM_FLICKER_SERVICE_SUPPORT
    if  ( NSCamera::IFrameService* pService = mvpFrameService[NSCamera::eFSID_Flicker] )
    {
        using namespace NSCamera;
        if  ( ! IFlickerService::FSCmd_SetWinInfo(pService, rcCrop.w, rcCrop.h).execute() )
        {
            MCAM_ERR("[mHalCamPreviewZoom] IFlickerService::FSCmd_SetWinInfo");
            err = -1;
        }
    }
#endif
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamWaitPreviewStable(
)
{
    AutoCPTLog cptlog(Event_mHalCamWaitPreviewStable);
    MINT32 err = MHAL_NO_ERROR;
    MUINT32 delayFrame = 0;
    MUINT32 cnt = 0;
    MCAM_DBG("[mHalCamWaitPreviewStable] \n");

    //! Wait for statistic data valid
    //! For raw sensor, the 3A statistic need 2 frame
    //! for stable.
    MCAM_DBG("[mHalCamWaitPreviewStable] delay 2 frames for 3A Statistic Stable\n");
    while (cnt < 2 && (mHalCamGetState() & MHAL_CAM_PREVIEW_MASK)) {
        err = mpCameraIOObj->dropPreviewFrame();
        if (err < 0) {
            MCAM_ERR("[mHalCamWaitPreviewStable] dropFrame() fail \n");
            return err;
        }
        cnt++;
    }

    //
    //! Get the sensor stable frame count.
    //! Yuv sensor need some frames to be stable when init
    mhalCamParam_t *pmhalCamParam = (mhalCamParam_t*) &mmHalCamParam;
    if (pmhalCamParam->u4CamMode != MHAL_CAM_MODE_MVIDEO) {
        mpIspHalObj->sendCommand(ISP_CMD_GET_SENSOR_DELAY_FRAME_CNT, 0, (int)&delayFrame);
    }
    else {
        mpIspHalObj->sendCommand(ISP_CMD_GET_SENSOR_DELAY_FRAME_CNT, 1, (int)&delayFrame);
    }

    MCAM_DBG("[mHalCamWaitPreviewStable] delayFrame: %d for sensor stable\n", delayFrame);
    //DEBUG
    #if (MHAL_CAM_MAV_TEST_MODE)
    delayFrame = 40;
    #endif
    //
    //
    cnt = 0;
    while (cnt < delayFrame  && (mHalCamGetState() & MHAL_CAM_PREVIEW_MASK)) {
        #if (MHAL_CAM_3A_OPTION)
        err = mHalCam3AProc();
        //MCAM_ASSERT(err >= 0, "mHalCam3AProc err \n");
        #endif
        err = mpCameraIOObj->dropPreviewFrame();
        if (err < 0) {
            MCAM_ERR("[mHalCamWaitPreviewStable] dropFrame() fail \n");
            return err;
        }
        cnt++;
    }
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamPreviewProc(
)
{
    AutoCPTLog cptlog(Event_mHalCamPreviewProc);

    MINT32 err = MHAL_NO_ERROR;
    mFrameCnt = 0;
    //FIXME, may cause race condition w/ other thread
    mhalCamParam_t *pmhalCamParam = &mmHalCamParam;
    ICameraIO::BuffInfo_t rDispBufInfo;
    MUINT32 currentZoom;
    MUINT32 IsZSDFlash = 0;
    MUINT32 IsZSDFlashDone = 0;
	MUINT32 IsStrobe = 0;
    MUINT32 IsZSDOff = 0;

    DebugFPS debugFps("preview");
    mhalCamTimeStampBufCBInfo mhalCamDispBufCb(0);

    MCAM_DBG("[mHalCamPreviewProc] E \n");
    //
    currentZoom = pmhalCamParam->u4ZoomVal;
    //
    CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "wait preview stable");
    if (pmhalCamParam->camZsdParam.u4ZsdSkipPrev != 1) {
        mHalCamWaitPreviewStable();
    }
    else{
        //workaround for MDP 1st frame can't do zoom
        if ((pmhalCamParam->camZsdParam.u4ZsdEnable == 0x1) && (currentZoom !=0)){
            mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
            for (MUINT32 i = 0; i < rDispBufInfo.fillCnt; i++) {
                mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
            }
            if (pmhalCamParam->camZsdParam.u4ZsdEnable == 0x1) {
                ICameraIO::BuffInfo_t rVdoBufInfo;
                mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_VIDEO_PORT, &rVdoBufInfo);
                for (MUINT32 i = 0; i < rVdoBufInfo.fillCnt; i++) {
                    mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_VIDEO_PORT, &rVdoBufInfo);
                }
            }
        }
    }

    //
    while (mHalCamGetState() & MHAL_CAM_PREVIEW_MASK) {
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "preview while in");
        CamTimer camTmr("mHalCamPreviewProc", MHAL_CAM_PRV_PROFILE_OPT);
        // 0. to resolve matv desense issue. only work when matv mode
        if (mIsAtvDisp) {
            CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "wait ATV vSync");
            if (mHalCamGetState() & MHAL_CAM_PREVIEW_MASK) { // ? why this ?
                err = mpIspHalObj->waitDone(ISP_HAL_IRQ_CLEAR_WAIT|ISP_HAL_IRQ_VSYNC);
                CPTLog(Event_getmAtvVSync, CPTFlagPulse);
                mhalCamTimeStampBufCBInfo mhalATVCBInfo(0);
                mHalCamCBHandle(MHAL_CAM_CB_ATV_DISP, &mhalATVCBInfo);
            }
        }
        // 1. ZSD mode process
        if (pmhalCamParam->camZsdParam.u4ZsdEnable == 0x1) {
            CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "ZSD mode process");
            ICameraIO::BuffInfo_t rVdoBufInfo;
            mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_VIDEO_PORT, &rVdoBufInfo);
            //for ALPS00235420
            //if ((mHalCamGetState() & MHAL_CAM_PREVIEW_MASK)||mmHalCamParam.cam3AParam.strobeMode != FLASHLIGHT_FORCE_OFF)
            {
                mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_VIDEO_PORT, &rVdoBufInfo);
            }
        }
        // 2. De-queue display buffer
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "De-queue display buffer");
        err = mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
        CPTLog(Event_getPreviewFrame, CPTFlagPulse);
        camTmr.printTime("getPreviewFrame (disp)");
        MCAM_DBG1("[mHalCamPreviewProc]  DISP hwIndex: %d, fillCnt: %d, timeS = %d, timeUs = %d\n",
                               rDispBufInfo.hwIndex, rDispBufInfo.fillCnt, rDispBufInfo.timeStampS, rDispBufInfo.timeStampUs);

        // 3. Update to display buffer No and callback to SF
        if (pmhalCamParam->camZsdParam.u4ZsdEnable == 0x1) {
            if (rDispBufInfo.fillCnt > 1){
                mhalCamDispBufCb.u4BufIndex = rDispBufInfo.hwIndex + (rDispBufInfo.fillCnt-1);
                if (mhalCamDispBufCb.u4BufIndex >= PREVIEW_BUFFER_CNT){
                    mhalCamDispBufCb.u4BufIndex -= PREVIEW_BUFFER_CNT;
                }
                for (int i=0;i< (int)rDispBufInfo.fillCnt;i++){
                    mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
                    mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
                }
                MCAM_DBG1("[mHalCamPreviewProc][ZSD]Skip Disp buff to idx: %d\n", mhalCamDispBufCb.u4BufIndex);
                mhalCamDispBufCb.u4TimStampS = rDispBufInfo.timeStampS;
                mhalCamDispBufCb.u4TimStampUs= rDispBufInfo.timeStampUs;
            }
            else{
                mhalCamDispBufCb.u4BufIndex = rDispBufInfo.hwIndex;
                mhalCamDispBufCb.u4TimStampS = rDispBufInfo.timeStampS;
                mhalCamDispBufCb.u4TimStampUs= rDispBufInfo.timeStampUs;
            }
        }
        else{
            mhalCamDispBufCb.u4BufIndex = rDispBufInfo.hwIndex;
            mhalCamDispBufCb.u4TimStampS = rDispBufInfo.timeStampS;
            mhalCamDispBufCb.u4TimStampUs= rDispBufInfo.timeStampUs;
        }
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "callback dispbuf to SF");
        mHalCamCBHandle(MHAL_CAM_CB_PREVIEW, &mhalCamDispBufCb);
        camTmr.printTime("MHAL_CAM_CB_PREVIEW");
        debugFps.checkFPS(30);

        // 4. Video Process
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "Video Process");
        //if (!pmhalCamParam->camZsdParam.u4ZsdEnable)
        if (1)
        {
            if(MTRUE ==  mIsVideoRecording ) {
                VideoFrame rVdoFrame;
                rVdoFrame.hwIndex = rDispBufInfo.hwIndex;
                rVdoFrame.fillCnt = rDispBufInfo.fillCnt;
                rVdoFrame.bufAddr = rDispBufInfo.bufAddr;
                rVdoFrame.timeStampS = rDispBufInfo.timeStampS;
                rVdoFrame.timeStampUs = rDispBufInfo.timeStampUs;
                if (mVdoBusyFrmCnt < PREVIEW_BUFFER_CNT) {
                    #if (MHAL_CAM_VIDEO_SNAPSHOT)
                    if(mpVideoSnapshot)
                    {
                        if(mmHalCamParam.frmYuv.w > VIDEO_SNAPSHOT_SKIA_WIDTH)
                        {
                            mpVideoSnapshot->SetFrame(mmHalCamParam.frmYuv.virtAddr+((mmHalCamParam.frmYuv.w)*(mmHalCamParam.frmYuv.h)*3/2)*rDispBufInfo.hwIndex);
                        }
                    }
                    #endif
                    postVideoFrame(rVdoFrame);
                    mHalCamVideoProc();
                    // after post the video buffer, check if the busy count is full,
                    // only release video buffer when the buffer is not full
                    while (mVdoBusyFrmCnt == PREVIEW_BUFFER_CNT && MTRUE ==  mIsVideoRecording ) {
                        MCAM_WARN("[mHalCamPreviewProc] OMX is busy, buffer is drop, busy cnt:%d\n", mVdoBusyFrmCnt);
                        usleep(33000);
                    }
                    err = mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
                }
            }
            else {
                err = mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
            }
        }
        else {
            err = mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
        }

        // 5. 3A Process
        //    do the AWB, AE, AF proceess at each frame in
        #if (MHAL_CAM_3A_OPTION)
        {
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "3A process");
        err = mHalCam3AProc();
        }
        #endif

        // 6. Zoom Process
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "Zoom process");
        err = mHalCamPreviewZoomProc();
        if (err != MHAL_NO_ERROR) {
            MCAM_WARN("[mHalCamPreviewProc] mHalCamPreviewZoomProc err:0x%x\n", err);
        }

        // 7. EIS
#if (MHAL_CAM_EIS_OPTION)
        {
            CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "EIS process");
            MINT32  gmvX = 0, gmvY = 0;
            mHalCamEISProc(&gmvX, &gmvY);
        }
#endif

        // 8. Frame service,
        //    It is only for flicker currently.
#if (MHAL_CAM_FRAME_SERVICE_SUPPORT)
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "FrameService process");
        for (MUINT32 i = 0; i < NSCamera::eFSID_NUM; i++)
        {
            if  ( NULL != mvpFrameService[i] )
            {
                mvpFrameService[i]->onFrameUpdated();
            }
        }
#endif

        // 8. MAV process
        #if (MHAL_CAM_MAV_OPTION)
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "MAV process");
        if ( mhal3DObj && mhal3DObj->getFrame() )
        {
            err = mhal3DObj->mHalCamFeatureProc(rDispBufInfo.hwIndex);
            if (err != MHAL_NO_ERROR)
            {
                MCAM_ERR("mHalCamFeatureProc err");
                MINT32 cbMsg = ERR_RESET;
                mHalCamCBHandle(MHAL_CAM_CB_ERR, &cbMsg);
            }
        }
        #endif

        // 9. Auto Scene detection procss
        #if (MHAL_CAM_ASD_OPTION)
        if(mpHalASDObj)
        {
            CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "ASD procss");
            mpHalASDObj->Proc();
        }
        #endif

        // 10. Notify the frame is coming
        mFrameCnt++;
        MCAM_DBG1("[mHalCamPreviewProc] Brocast the preview frame in frameCnt:%d E \n", mFrameCnt);

        if (mCamDumpOPT) {
            if (mFrameCnt == PREVIEW_DUMP_NO) {
                CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "dump process");
                ::mHalMiscDumpToFile((char*) "//sdcard/prv.raw", (UINT8*)pmhalCamParam->frmYuv.virtAddr, pmhalCamParam->frmYuv.frmSize);
            }
        }
        camTmr.printTime("mHalCamPreviewProc");
    }
    MCAM_DBG("[mHalCamPreviewProc]  exit while \n");
    //
    CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "3A setFlashActive");
    mpHal3AObj->setFlashActive(MFALSE);
    while (mHalCamGetState() == MHAL_CAM_PRE_CAPTURE) {
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "preCapture while in");
/*
        if ((!IsZSDFlashDone)&&(pmhalCamParam->camZsdParam.u4ZsdEnable)&&(pmhalCamParam->u4ShotMode==MHAL_CAM_CAP_MODE_NORMAL)) {
            //if (mmHalCamParam.cam3AParam.strobeMode != FLASHLIGHT_FORCE_OFF){
            CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "strobe on for zsd");
            if (mpHal3AObj->isStrobeOn()){
                IsZSDFlash = 1;
                IsZSDFlashDone = 1;
            }
            else{
                IsZSDFlash = 0;
                break;
            }
*/
        if (!IsStrobe){
            if (mpHal3AObj->isStrobeOn()){
                MCAM_DBG("[mHalCamPreviewProc]pre-cap,isStrobeOn");
                mHalCamCBHandle(MHAL_CAM_CB_FLASHON, NULL);
                IsZSDOff = 0x1;
            }
            IsStrobe = 0x1;
        }

		if ((pmhalCamParam->camZsdParam.u4ZsdEnable == 0x2) &&
			(pmhalCamParam->u4ShotMode==MHAL_CAM_CAP_MODE_NORMAL) &&
			IsZSDOff!=0x1) {
            MCAM_DBG("[mHalCamPreviewProc] ZSD NORMAL SHOT(strobe off), skip pre-capture\n");
			break;
		}


        #if (MHAL_CAM_3A_OPTION)
        if (pmhalCamParam->camZsdParam.u4ZsdEnable == 0x1) {
            CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "get/release frame for zsd");
            ICameraIO::BuffInfo_t rVdoBufInfo;
            mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_VIDEO_PORT, &rVdoBufInfo);
            mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_VIDEO_PORT, &rVdoBufInfo);
        }
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "getPreviewFrame");
        err = mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
        //MCAM_ASSERT(err >= 0, "mpCameraIOObj->getPreviewFrame err\n");
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "mHalCam3AProc");
        err = mHalCam3AProc();
        //MCAM_ASSERT(err >= 0, "mHalCam3AProc err \n");
        //MCAM_DBG("[mHalCamPreviewProc]getReadyForCapture = %d, getFlashActive = %d",mpHal3AObj->getReadyForCapture(),mpHal3AObj->getFlashActive());
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "process not readyforpreCapture");
        if( !(mpHal3AObj->getReadyForCapture()) ||
            !(mpHal3AObj->getFlashActive()))
        {
            // Get current hw display buffer number
            mhalCamDispBufCb.u4BufIndex = rDispBufInfo.hwIndex;
            mhalCamDispBufCb.u4TimStampS = rDispBufInfo.timeStampS;
            mhalCamDispBufCb.u4TimStampUs= rDispBufInfo.timeStampUs;
            // Callback to upper layer
            mHalCamCBHandle(MHAL_CAM_CB_PREVIEW, &mhalCamDispBufCb);
        }
        //
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "process readyforpreCapture");
        if (mpHal3AObj->getReadyForCapture()) {
            MCAM_DBG("[mHalCamPreviewProc][Perf][Shutter Delay][Display Delay]Start");
            mHalCamCBHandle(MHAL_CAM_CB_PREVIEW, &mhalCamDispBufCb);
            // Capture's ISO setting
            CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "get ISO from 3A E");
            MINT32 iso;
            if ((pmhalCamParam->camZsdParam.u4ZsdEnable)&&(pmhalCamParam->u4ShotMode==MHAL_CAM_CAP_MODE_NORMAL)&&IsZSDOff!=0x1){
                mpHal3AObj->getISO(0, &iso);
            }
            else{
                mpHal3AObj->getISO(1, &iso);
            }
            CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "set ISO to ISP E");
            err = mpIspHalObj->sendCommand(ISP_CMD_SET_ISO, iso);
            if (err < 0) {
                MCAM_ERR("[mHalCamPreviewProc]  ISP_CMD_SET_ISO err \n\n");
            }
            mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
            mHalCamSetState(MHAL_CAM_STOP);
            break;
        }
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "releasePreviewFrame");
        err = mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
        #endif
    }
	/*
    if (IsZSDFlash == 1){
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "[ZSD]Flash on");
        MCAM_DBG("[mHalCamPreviewProc][ZSD]Flash on \n");
        mpHal3AObj->strobeDelay();      //cotta--added for strobe protection
        mpHal3AObj->set3AZSDMode(1);    //cotta-- set ZSD flag on before turn on strobe
        mpHal3AObj->setCaptureParam(0);
    }*/
    if (pmhalCamParam->camZsdParam.u4ZsdEnable
        && (pmhalCamParam->u4ShotMode == MHAL_CAM_CAP_MODE_NORMAL)
        && IsZSDOff != 1){
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "preview stop for zsd");
        if (pmhalCamParam->camZsdParam.u4ZsdEnable == 0x1){
            mHalCamZsdWaitBuff(1);
        }
        else{
                #ifdef MTK_ZSD_AF_ENHANCE
        	MCAM_DBG("[ZSD] Set ISP tuning: ISP_CAM_MODE_CAPTURE_FLY_ZSD\n");
        	err = mHalCamSetIspMode(ISP_CAM_MODE_CAPTURE_FLY_ZSD, (int) mmHalCamParam.u4CamIspMode);
    		if (err < 0) {
        		MCAM_ERR("[ZSD] Set ISP tuning CAPTURE err (0x%x)\n", err);
    		}
                #endif
            #if 0
            mpCameraIOObj->setZSDPreviewFrame(mmHalCamParam.frmYuv.w,mmHalCamParam.frmYuv.h, mmHalCamParam.u4ZoomVal);
            /*Because BW limiation, can NOT get display port here*/
            int nub;
            for (nub= 0; nub<4; nub++){
                mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_ZSD_PORT, &rDispBufInfo);
                MCAM_DBG1("[mHalCamPreviewProc] [ZSD] %d hwIndex: %d, fillCnt: %d, timeS = %d, timeUs = %d\n",
                    nub, rDispBufInfo.hwIndex, rDispBufInfo.fillCnt, rDispBufInfo.timeStampS, rDispBufInfo.timeStampUs);

                /*To get latest ready frame*/
                if (rDispBufInfo.fillCnt > 1){
                    readyIndex = rDispBufInfo.hwIndex + (rDispBufInfo.fillCnt-1);
                    if (readyIndex >= 3){
                        readyIndex -= 3;
                    }
                    for (int i=0;i< (int)rDispBufInfo.fillCnt;i++){
                        mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_ZSD_PORT, &rDispBufInfo);
                        mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_ZSD_PORT, &rDispBufInfo);
                    }
                    MCAM_DBG1("[mHalCamPreviewProc][ZSD]Skip Disp buff to idx: %d\n", readyIndex);
                }
                else{
                    readyIndex = rDispBufInfo.hwIndex;
                }
                /*Got ZSD frame, process image here: START*/

                /*Got ZSD frame, process image here: END*/

                mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_ZSD_PORT, &rDispBufInfo);
            }


            #else
            //zsd new pass
            mpCameraIOObj->setZSDLastPreviewFrame(mmHalCamParam.frmYuv.w,mmHalCamParam.frmYuv.h, mmHalCamParam.u4ZoomVal);
            if (mCamDumpOPT){
                ::mHalMiscDumpToFile((char*) "//sdcard//zsd_full_yuyv.bin", (UINT8*)mmHalCamParam.camZsdParam.u4ZsdAddr,
                                     mmHalCamParam.camZsdParam.u4ZsdWidth*mmHalCamParam.camZsdParam.u4ZsdHeight*2);
            }
            mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT_EX, &rDispBufInfo);
            mhalCamDispBufCb.u4BufIndex = rDispBufInfo.hwIndex;
            mhalCamDispBufCb.u4TimStampS = rDispBufInfo.timeStampS;
            mhalCamDispBufCb.u4TimStampUs= rDispBufInfo.timeStampUs;
            mHalCamCBHandle(MHAL_CAM_CB_PREVIEW, &mhalCamDispBufCb);
            MCAM_DBG1("[mHalCamPreviewProc]  DISP -2 hwIndex: %d, fillCnt: %d, timeS = %d, timeUs = %d\n",
                rDispBufInfo.hwIndex, rDispBufInfo.fillCnt, rDispBufInfo.timeStampS, rDispBufInfo.timeStampUs);
            mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_DISP_PORT_EX, &rDispBufInfo);

            mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT_EX, &rDispBufInfo);
            mhalCamDispBufCb.u4BufIndex = rDispBufInfo.hwIndex;
            mhalCamDispBufCb.u4TimStampS = rDispBufInfo.timeStampS;
            mhalCamDispBufCb.u4TimStampUs= rDispBufInfo.timeStampUs;
            mHalCamCBHandle(MHAL_CAM_CB_PREVIEW, &mhalCamDispBufCb);
            MCAM_DBG1("[mHalCamPreviewProc]  DISP -1 hwIndex: %d, fillCnt: %d, timeS = %d, timeUs = %d\n",
                rDispBufInfo.hwIndex, rDispBufInfo.fillCnt, rDispBufInfo.timeStampS, rDispBufInfo.timeStampUs);
            #endif
            mHalCamCBHandle(MHAL_CAM_CB_ZSD_PREVIEW_DONE, NULL);
        }
        mHalCamSetState(MHAL_CAM_STOP);
    }
    if (mmHalCamParam.cam3AParam.EngMode == 1) { // Preview mode in Engineer mode
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "3A setPreviewToCaptureImage(0)");
        mpHal3AObj->setPreviewToCaptureImage(0);
    }
    MCAM_DBG("[mHalCamPreviewProc] End \n");

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
//!++ Revised: ICS FD
// Changing this function to be a memebr of mHalCamFD needs to consider:
// mHalCamGetState, ASD, pCameraIOObj, mpHal3AObj->setFDInfo
MINT32
mHalCam::mHalCamFDProc(
)
{
    MINT32 err = MHAL_NO_ERROR;
    #if (MHAL_CAM_FD_OPTION)

    MCAM_DBG("[mHalCamFDProc] +");

    static MUINT32 fdFrameCnt = 0;
    ICameraIO::BuffInfo_t buffInfo;

    //
    while ( (mHalCamGetState() & MHAL_CAM_PREVIEW_MASK)) {
        if (!mfdObj) {
            break;
        }
        if (!mfdObj->getFDstate()) {
            break;
        }
        ///
        mfdObj->lock();

        /// (1) get buffer from FD port
        err = mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_FD_PORT, &buffInfo);
        fdFrameCnt++;
        MCAM_DBG1("[mHalCamFDProc]  FD hwIndex: %d, fillCnt: %d, fdCnt = %d\n, addr = 0x%x",
                               buffInfo.hwIndex, buffInfo.fillCnt, fdFrameCnt, mfdObj->getFDBuf()->getVirtAddr()+buffInfo.hwIndex * mHalCamFD::FD_WIDTH * mHalCamFD::FD_HEIGHT *2);

        if (mCamDumpOPT) {
            if (fdFrameCnt == PREVIEW_DUMP_NO) {
                ::mHalMiscDumpToFile((char*) "//sdcard//fd.raw", (UINT8*)(mfdObj->getFDBuf()->getVirtAddr()+buffInfo.hwIndex * mHalCamFD::FD_WIDTH * mHalCamFD::FD_HEIGHT *2),
                                     mfdObj->getFDBuf()->getPoolSize());
                if (mfdObj->getFDstate()) {
                    mfdObj->dump();
                }
            }
        }

        /// (2) do ASD
        #if (MHAL_CAM_ASD_OPTION)
        if(mpHalASDObj)
        {
            MHAL_ASD_FRAME_STRUCT   ASDFrame;
            ASDFrame.Width = mHalCamFD::FD_WIDTH;
            ASDFrame.Height = mHalCamFD::FD_HEIGHT;
            ASDFrame.DataSize = mfdObj->getFDBuf()->getPoolSize();
            ASDFrame.pData = (UINT8*)(mfdObj->getFDBuf()->getVirtAddr()+buffInfo.hwIndex * mHalCamFD::FD_WIDTH * mHalCamFD::FD_HEIGHT *2);
            mpHalASDObj->SetFrame(&ASDFrame);
        }
        #endif

        /// (3) do FD
        #if defined(MTK_M4U_SUPPORT)
        err = mfdObj->mpFDHalObj->halFDDo(mfdObj->getFDBuf()->getVirtAddr()+buffInfo.hwIndex * mHalCamFD::FD_WIDTH * mHalCamFD::FD_HEIGHT *2);
        #else
        err = mfdObj->mpFDHalObj->halFDDo(mfdObj->getFDBuf()->getPhyAddr()+buffInfo.hwIndex * mHalCamFD::FD_WIDTH * mHalCamFD::FD_HEIGHT *2);
        #endif

        /// (4) set FD result
        mfdObj->setContentProvider(mfdProvider->getFDResult(), mfdProvider->getFaceInfo());
        MINT32 isNewFaceIn, isNewSmileIn;
        mfdObj->setFaceResult(isNewFaceIn, isNewSmileIn);
        if (isNewSmileIn) {
            mHalCamCBHandle(MHAL_CAM_CB_SMILE, NULL);
        }
        // always callback to update FD box
        mHalCamCBHandle(MHAL_CAM_CB_FD, mfdProvider->getFDResult());

        //mfdProvider->dump();
        //mfdObj->dump();

        /// (5) Refill the FD buffer
        for (MUINT32 i = 0; i < buffInfo.fillCnt; i++) {
            err = mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_FD_PORT, &buffInfo);
        }

        /// (6)
        err = mpHal3AObj->setFDInfo(mfdObj->getFDstate(), (MUINT32)mfdProvider->getFDResult());

    }

    /// (6.1) let aaa_hal know fd has left
    if (mpHal3AObj) {
        err = mpHal3AObj->setFDInfo(0, (MUINT32)mfdProvider->getFDResult());
    }
    ////
    if (mfdObj) {
        mfdObj->unlock();
    }

    ////
    MCAM_DBG("[mHalCamFDProc] -");
    #endif

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamCaptureProc(
)
{
    AutoCPTLog cptlog(Event_mHalCamCaptureProc);
    MINT32 err = MHAL_NO_ERROR;

    MCAM_DBG("[mHalCamCaptureProc] \n");

    if(!mmHalCamParam.camZsdParam.u4ZsdEnable){
        mpHal3AObj->strobeDelay();  //cotta--added for strobe protection
    }

    if  ( ! mpShot || ! mpShot->capture() )
    {
        MCAM_ERR("[mHalCamCaptureProc]<ShotMode:%d> mpShot(%p)->capture()", mmHalCamParam.u4ShotMode, mpShot);
        err = -1;
    }

    mHalCamSetState(MHAL_CAM_IDLE);
    //
    mHalCamCBHandle(MHAL_CAM_CB_CAPTURE_DONE, NULL);
    //
    MCAM_DBG("[mHalCamCaptureProc] End \n");

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamPanoProc(
)
{
    MINT32 ret = MHAL_NO_ERROR;
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
static MVOID *
mHalCamMainHighThread(
    MVOID *arg
)
{
    ::prctl(PR_SET_NAME,"mHalCamMainHighThread",0,0,0);

    //  detach thread
    ::pthread_detach(::pthread_self());

    mHalCamState_e eState;

    MCAM_DBG("[mHalCamMainHighThread] tid: %d \n", gettid());

    eState = pmHalCamObj->mHalCamGetState();
    while (eState != MHAL_CAM_UNINIT) {
        ::sem_wait(&semMainHigh);
        MCAM_DBG("Got semMainHigh \n");
        eState = pmHalCamObj->mHalCamGetState();
        switch (eState) {
        case MHAL_CAM_PREVIEW:
            pmHalCamObj->mHalCamPreviewProc();
            ::sem_post(&semMainHighBack);
            break;
        case MHAL_CAM_CAPTURE:
            pmHalCamObj->mHalCamCaptureProc();
            break;
        case MHAL_CAM_PANORAMA:
            pmHalCamObj->mHalCamPanoProc();
            break;
        case MHAL_CAM_UNINIT:
            break;
        default:
            MCAM_DBG("[mHalCamMainHighThread] T.B.D");
            break;
        }
        eState = pmHalCamObj->mHalCamGetState();
    }

    ::sem_post(&semMainHighEnd);
    MCAM_DBG("[mHalCamMainHighThread] End \n");

    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
static MVOID *
mHalCamFDThread(
    MVOID *arg
)
{
    ::pthread_detach(::pthread_self());

    // set thread name
    ::prctl(PR_SET_NAME,"mHalCamFDThread",0,0,0);

    // set policy and priority
    int policy = SCHED_OTHER;
    int priority = 0;
    struct sched_param sched_p;
    sched_getparam(0, &sched_p);
    sched_p.sched_priority = 0;
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
    //
    MINT32 err = MHAL_NO_ERROR;
    //
    #if (MHAL_CAM_FD_OPTION)

    MCAM_DBG("[mHalCamFDThread] tid: %d \n", gettid());

    /// FIXME: partially separated from MhalCam
    while (!pmHalCamObj->mfdObj->mCamDestroy) {
        ::sem_wait(&semFDthread);
        err = pmHalCamObj->mHalCamFDProc();
        if (err < 0) {
            MCAM_ERR("[mHalCamFDThread] mHalCamFDProc err %d \n", err);
            break;
        }
    }

    //
    ::sem_post(&semFDthreadEnd);
    MCAM_DBG("[mHalCamFDThread] End \n");

    #endif

    return NULL;
}



/*******************************************************************************
*
********************************************************************************/

MVOID mHalCam::mHalCamShowParam(mhalCamParam_t *pmhalCamParam)
{
    MCAM_DBG("===========mHalCamParam==========================\n");
    MCAM_DBG("YuvVirtAddr: 0x%08x/0x%08x, RgbVirtAddr: 0x%08x/0x%08x \n",
        pmhalCamParam->frmYuv.virtAddr, pmhalCamParam->frmYuv.phyAddr,
        pmhalCamParam->frmRgb.virtAddr, pmhalCamParam->frmRgb.phyAddr);
    MCAM_DBG("QvVirtAddr: 0x%08x/0x%08x, JpgVirtAddr: 0x%08x/0x%08x \n",
        pmhalCamParam->frmQv.virtAddr, pmhalCamParam->frmQv.phyAddr,
        pmhalCamParam->frmJpg.virtAddr, pmhalCamParam->frmJpg.phyAddr);
    MCAM_DBG("BsVirtAddr:0x%08x/0x%08x, VdoVirtAddr:: 0x%08x/0x%08x \n",
        pmhalCamParam->frmBS.virtAddr, pmhalCamParam->frmBS.phyAddr,
        pmhalCamParam->frmVdo.virtAddr, pmhalCamParam->frmVdo.phyAddr);
    MCAM_DBG("yuv: %d, %d, rgb: %d, %d \n",
        pmhalCamParam->frmYuv.w, pmhalCamParam->frmYuv.h,
        pmhalCamParam->frmRgb.w, pmhalCamParam->frmRgb.h);
    MCAM_DBG("qv: %d, %d, jpg: %d, %d \n",
        pmhalCamParam->frmQv.w, pmhalCamParam->frmQv.h,
        pmhalCamParam->frmJpg.w, pmhalCamParam->frmJpg.h);
    MCAM_DBG("camode: %d, fps: %d \n",
        pmhalCamParam->u4CamMode, pmhalCamParam->cam3AParam.prvFps);
    MCAM_DBG("isospeed: %d, afMode: %d, afMeter: %d, awbMode: %d \n",
        pmhalCamParam->cam3AParam.isoSpeedMode, pmhalCamParam->cam3AParam.afMode,
        pmhalCamParam->cam3AParam.afMeterMode, pmhalCamParam->cam3AParam.awbMode);
    MCAM_DBG("EngMode: %d, EngPos: %d, BestPos: %d\n",
        pmhalCamParam->cam3AParam.afEngMode, pmhalCamParam->cam3AParam.afEngPos, mAFBestPos);
    MCAM_DBG("AFMode: %d, AFMeter: %d\n",
        pmhalCamParam->cam3AParam.afMode, pmhalCamParam->cam3AParam.afMeterMode);
    MCAM_DBG("Jpeg Orientation: %D \n",
        pmhalCamParam->u4JPEGOrientation);
    MCAM_DBG("=================================================\n");
}


/*******************************************************************************
*
********************************************************************************/
#define EIS_FACTOR   (115)
MINT32
mHalCam::mHalCamPreviewStart(
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize
)
{
    AutoCPTLog cptlog(Event_mHalCamPreviewStart);
    MCAM_DBG("[mHalCamPreviewStart] E \n");
    MCAM_ASSERT(mHalCamGetState() == MHAL_CAM_IDLE, "Camera State not IDLE \n");
    //
    MINT32 err = MHAL_NO_ERROR;
    mhalCamParam_t *pmhalCamParam = (mhalCamParam_t *) a_pInBuffer;
    ICameraIO::ImgCfg_t rInCfg;
    ICameraIO::ImgCfg_t *pDispCfg = NULL, *pVdoCfg = NULL, *pFDCfg = NULL, *pZSDCfg = NULL;
    ICameraIO::MemInfo_t *pDispMemInfo = NULL, *pVdoMemInfo = NULL, *pFDMemInfo = NULL, *pZSDMemInfo = NULL;

    MUINT32 previewPort = 0;
    MUINT32 isZSDOn = pmhalCamParam->camZsdParam.u4ZsdEnable;
    MUINT32 isZSDSkipPrev = pmhalCamParam->camZsdParam.u4ZsdSkipPrev;
    //
    CamTimer camTmr("mHalCamPreviewStart", MHAL_CAM_GENERIC_PROFILE_OPT);
    //
    mHalCamShowParam(pmhalCamParam);
    //
    mHalCamFreeZSDMem();
    //
    ::memcpy(&mmHalCamParam, pmhalCamParam, sizeof(mhalCamParam_t));
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "mHalSetResMgr");
    mHalSetResMgr(RES_MGR_HAL_MODE_PREVIEW_ON);
    //! the callback is for EIS crop, it will occupy sysram.
    //! ATV no need to use this due to TVout conflict it
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "set EISConfig");
    if ( mHalCamSensorInfo[mCameraId].devType != MHAL_CAM_SENSOR_DEV_ATV) {
        //! ICS EIS
        //! the image size will be the final size * 1.15 for EIS use
        //! if eis on, final size * 1.15 ->crop -> final size
        //! if eis off, final size * 1.15 -> resize -> final size
        mEISSrcW = ROUND_TO_2X(pmhalCamParam->frmYuv.w * EIS_FACTOR/100);
        mEISSrcH = ROUND_TO_2X(pmhalCamParam->frmYuv.h * EIS_FACTOR/100);
        mpCameraIOObj->setCallbacks(mHalCamIOCB, this);
    }
    else {
        mEISSrcW = pmhalCamParam->frmYuv.w ;
        mEISSrcH = pmhalCamParam->frmYuv.h;
        mpCameraIOObj->setCallbacks(NULL, NULL);
    }
    //
    rInCfg.imgWidth = mEISSrcW;
    rInCfg.imgHeight = mEISSrcH;
    rInCfg.rotation = 0;

    #if (MHAL_CAM_EIS_OPTION)
    setEISConfig(mEISSrcW,
                      mEISSrcH,
                      pmhalCamParam->cam3AParam.eisW,
                      pmhalCamParam->cam3AParam.eisH);
    #endif

    //
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "config Display port");
    MCAM_DBG("mRetFakeOrientation = %d\n", mRetFakeSubOrientation);
    if(mHalCamSensorInfo[mCameraId].devType == MHAL_CAM_SENSOR_DEV_SUB && MTRUE == mRetFakeSubOrientation && pmhalCamParam->u4CamMode != MHAL_CAM_MODE_VT)
    {
        if (mHalCamSensorInfo[mCameraId].orientation == 0) {

            MUINT32 imgWidth = rInCfg.imgWidth;
            MUINT32 imgHeight = rInCfg.imgHeight;
            rInCfg.imgWidth = imgHeight;
            rInCfg.imgHeight = imgWidth;
            rInCfg.rotation = 3;
            MCAM_DBG("front sensor orientation is %d, do rotation. \n", mHalCamSensorInfo[mCameraId].orientation);

        } else if (mHalCamSensorInfo[mCameraId].orientation == 180)
        {
            MUINT32 imgWidth = rInCfg.imgWidth;
            MUINT32 imgHeight = rInCfg.imgHeight;
            rInCfg.imgWidth = imgHeight;
            rInCfg.imgHeight = imgWidth;
            rInCfg.rotation = 3;
            MCAM_DBG("front sensor orientation is %d, do rotation. \n", mHalCamSensorInfo[mCameraId].orientation);
        } else if (mHalCamSensorInfo[mCameraId].orientation == 90)
        {
            rInCfg.rotation = 2;
            MCAM_DBG("front sensor orientation is %d, do rotation. \n", mHalCamSensorInfo[mCameraId].orientation);
        }
    }

    //
    if (pmhalCamParam->u4CamMode == MHAL_CAM_MODE_VT) {
        // VT Mode, rotate the buffer to 0 degree
        if (mHalCamSensorInfo[mCameraId].orientation == 90) {
            //rInCfg.imgWidth = pmhalCamParam->frmYuv.w;
            //rInCfg.imgHeight = (pmhalCamParam->frmYuv.w * pmhalCamParam->frmYuv.w / pmhalCamParam->frmYuv.h + 0xf)& (~0xf);
            MUINT32 imgWidth = rInCfg.imgWidth;
            MUINT32 imgHeight = rInCfg.imgHeight;
            rInCfg.imgWidth = imgHeight;
            rInCfg.imgHeight = imgWidth;
            rInCfg.rotation = 1;
        } else if (mHalCamSensorInfo[mCameraId].orientation == 270) {
            //rInCfg.imgWidth = pmhalCamParam->frmYuv.w;
            //rInCfg.imgHeight = (pmhalCamParam->frmYuv.w * pmhalCamParam->frmYuv.w / pmhalCamParam->frmYuv.h + 0xf)& (~0xf);
            MUINT32 imgWidth = rInCfg.imgWidth;
            MUINT32 imgHeight = rInCfg.imgHeight;
            rInCfg.imgWidth = imgHeight;
            rInCfg.imgHeight = imgWidth;
			rInCfg.rotation = 3;
        } else if (mHalCamSensorInfo[mCameraId].orientation == 180) {
            rInCfg.rotation = 2;
        }
    }
    //
    //! Display port config,
    //! The Display output image is also used in VT mode
    MCAM_DBG("Display Port Init\n");
    pDispCfg = new ICameraIO::ImgCfg_t();
    pDispMemInfo = new ICameraIO::MemInfo_t();
    err = configDispPort(pmhalCamParam, pDispCfg, pDispMemInfo);
    MCAM_ASSERT(err >= 0, "configDispPort err \n");
    previewPort |= ICameraIO::ePREVIEW_DISP_PORT;
    //
    //! FD port config and FD init, only work at mtk preview mode to save power
    if (pmhalCamParam->u4CamMode == MHAL_CAM_MODE_MPREVIEW) {
        if (isZSDOn){
            CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "config ZSD port");
            pVdoCfg = new ICameraIO::ImgCfg_t();
            pVdoMemInfo = new ICameraIO::MemInfo_t();
            err = configZSDPort(pmhalCamParam, pVdoCfg, pVdoMemInfo,isZSDOn)
            MCAM_ASSERT(err >= 0, "configZSDPort err \n");
            if(isZSDOn == 0x1){
                previewPort |= ICameraIO::ePREVIEW_VIDEO_PORT;
            }
        }
    }
    // Video output
#if 0
    if (!isZSDOn) {
        pVdoCfg = new ICameraIO::ImgCfg_t();
        pVdoMemInfo = new ICameraIO::MemInfo_t();
        err = configVideoPort(pmhalCamParam, pVdoCfg, pVdoMemInfo);
        MCAM_ASSERT(err >= 0, "configVideoPort err");
        previewPort |= ICameraIO::ePREVIEW_VIDEO_PORT;
    }
#endif

    #if (MHAL_CAM_FD_OPTION)
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "init/config FD port");
    if (pmhalCamParam->u4CamMode == MHAL_CAM_MODE_MPREVIEW ||
        pmhalCamParam->u4CamMode == MHAL_CAM_MODE_DEFAULT )
    {
        //
        // FD port config
        //
        pFDCfg = new ICameraIO::ImgCfg_t();
        pFDMemInfo = new ICameraIO::MemInfo_t();
        err = configFDPort(pFDCfg, pFDMemInfo);
        MCAM_ASSERT(err >= 0, "configFDPort err \n");
        previewPort |= ICameraIO::ePREVIEW_FD_PORT;

        //!++ ICS FD
        // Whether do rotate or not. Currently judge by preview width/height
        // Should sync with AP
        //
        MINT32 fdRotation = 0;

        if ( mHalCamSensorInfo[mCameraId].devType != MHAL_CAM_SENSOR_DEV_ATV  &&
              mmHalCamParam.frmYuv.w < mmHalCamParam.frmYuv.h )//FIXME
        {
            fdRotation = 1;    //0: 0 degree, 1:90 degree, 2:180 degree, 3:270 degree
        }
        mfdObj->Init(fdRotation, FD_BUFFER_CNT, mFDWorkingBuf);
        mfdObj->setCallback( mmHalCamParam.mhalObserver );
        mfdObj->setDispInfo(0,
                           0,
                           mmHalCamParam.frmYuv.w,
                           mmHalCamParam.frmYuv.h,
                           fdRotation,
                           mHalCamSensorInfo[mCameraId].orientation/90,
                           mHalCamSensorInfo[mCameraId].facing);
    }
    #endif
    //
    #if (MHAL_CAM_VIDEO_SNAPSHOT)
    if(pmhalCamParam->u4CamMode == MHAL_CAM_MODE_MVIDEO)
    {
        MCAM_DBG("[mHalCamPreviewStart]VSS:Video mode");
        if(mpVideoSnapshot)
        {
            VIDEO_SNAPSHOT_CONFIG_STRUCT VideoSnapshotConfig;
            pFDCfg = new ICameraIO::ImgCfg_t();
            pFDMemInfo = new ICameraIO::MemInfo_t();
            //
            VideoSnapshotConfig.Width       = pDispCfg->imgWidth;
            VideoSnapshotConfig.Height      = pDispCfg->imgHeight;
            VideoSnapshotConfig.SensorType  = mapSensorType(meSensorType);
            VideoSnapshotConfig.DeviceId    = mapDeviceID(mHalCamSensorInfo[mCameraId].devType);
            VideoSnapshotConfig.pHal3AObj   = mpHal3AObj;
            VideoSnapshotConfig.pCamParam   = &mmHalCamParam;
            VideoSnapshotConfig.CB          = pmhalCamParam->mhalObserver;
            //
            mpVideoSnapshot->Config(&VideoSnapshotConfig);
            if(pDispCfg->imgWidth <= VIDEO_SNAPSHOT_SKIA_WIDTH)
            {
                configVSSFDPort(pFDCfg, pFDMemInfo);
                previewPort |= ICameraIO::ePREVIEW_FD_PORT;
            }
        }
        else
        {
            MCAM_ERR("[mHalCamPreviewStart]VSS:mpVideoSnapshot is NULL");
        }
    }
    #endif
    //
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "setPreviewPipe");
    ICameraIO::PreviewPipeCfg_t* pPreviewPipeCfg  = new ICameraIO::PreviewPipeCfg_t();
    pPreviewPipeCfg->pInputConfig = &rInCfg;
    pPreviewPipeCfg->pDispConfig = pDispCfg;
    pPreviewPipeCfg->pVideoConfig = pVdoCfg;
    pPreviewPipeCfg->pFDConfig = pFDCfg;
    //

    MUINT32 sensorW = 0, sensorH = 0;  // check whether use capture frame for video 1080p or not
    ICameraIO::eSensorMode eSensorMode;
    err = mpIspHalObj->sendCommand(ISP_CMD_GET_SENSOR_PRV_RANGE, (MINT32)&sensorW, (MINT32)&sensorH);

    if((1920 == pDispCfg->imgWidth && 1088 == pDispCfg->imgHeight)
        && (sensorW < pDispCfg->imgWidth || sensorH < pDispCfg->imgHeight))
    {
        MCAM_DBG("For video 1080p, use capture frame");
        eSensorMode = ICameraIO::eSENSOR_CAPTURE_MODE;
        mmHalCamParam.cam3AParam.prvFps = 15;
        mmHalCamParam.cam3AParam.eisMode= 0;
    }
    else{
        eSensorMode = ICameraIO::eSENSOR_PREVIEW_MODE;
    }
    err = mpCameraIOObj->setSensorMode(eSensorMode);

    //
    if (isZSDOn){
        err = mpCameraIOObj->setZSDPreviewPipe(
                                        previewPort,         //enable port
                                        NULL,                //image is from mem ?
                                        pPreviewPipeCfg,     //preview pipe config
                                        pDispMemInfo,        //display out
                                        pVdoMemInfo,         //ZSD out
                                        pFDMemInfo,          //fd out
                                        isZSDSkipPrev,       //Skip open sensor again
                                        isZSDOn              //Select ZSD pass
                                        );
    }
    else{
        err = mpCameraIOObj->setPreviewPipe(
                                        previewPort,         //enable port
                                        NULL,                //image is from mem ?
                                        pPreviewPipeCfg,     //preview pipe config
                                        pDispMemInfo,        //display out
                                        pVdoMemInfo,         //video out
                                        pFDMemInfo           //fd out
                                        );
    }
    delete pPreviewPipeCfg;

    // Preview is always normal mode, isp tuning pipe config
    #if (MHAL_CAM_ISP_TUNING)
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "config IspTuning pipe");
    if  ( isZSDOn ) {
		#ifdef MTK_ZSD_AF_ENHANCE
        MCAM_DBG("[ZSD] Set ISP tuning: ISP_CAM_MODE_PREVIEW_FLY_ZSD\n");
        err = mHalCamSetIspMode(ISP_CAM_MODE_PREVIEW_FLY_ZSD, (int) pmhalCamParam->u4CamIspMode);
		#else
        err = mHalCamSetIspMode(ISP_CAM_MODE_CAPTURE_FLY_ZSD, (int) pmhalCamParam->u4CamIspMode);
		#endif
    }
    else if ( pmhalCamParam->u4CamMode == MHAL_CAM_MODE_MVIDEO ) {
        if(ICameraIO::eSENSOR_CAPTURE_MODE == eSensorMode)
        {
            MCAM_DBG("For video 1080p, set ISP_CAM_MODE_CAPTURE_FLY_ZSD");
            err = mHalCamSetIspMode(ISP_CAM_MODE_CAPTURE_FLY_ZSD, (int) pmhalCamParam->u4CamIspMode);  // 1080p zsd
        }
        else{
            err = mHalCamSetIspMode(ISP_CAM_MODE_VIDEO, (int) pmhalCamParam->u4CamIspMode);
        }
    }
    else {
        err = mHalCamSetIspMode(ISP_CAM_MODE_PREVIEW, (int) pmhalCamParam->u4CamIspMode);
    }
    if (err < 0) {
        MCAM_ERR("[mHalCamPreviewStart] mHalCamSetIspMode err (0x%x)\n", err);
        goto mHalCamPreviewStart_EXIT;
    }
    #endif

    // FIXME
    //! Should before Cam3AInit() due to
    //! FD proc will be invoke in 3AInit();
    //! (won't happen in ICS)
    mHalCamSetState(MHAL_CAM_PREVIEW);
    //
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "mHalCam3AInit");

    if(isZSDOn){
        err = mHalCam3AInit(pmhalCamParam->u4CamMode, pmhalCamParam->cam3AParam, 1);
    }
    else if (pmhalCamParam->u4CamMode == MHAL_CAM_MODE_MVIDEO && ICameraIO::eSENSOR_CAPTURE_MODE == eSensorMode){
        pmhalCamParam->cam3AParam.prvFps = 15;
        pmhalCamParam->cam3AParam.eisMode= 0;
        err = mHalCam3AInit(pmhalCamParam->u4CamMode, pmhalCamParam->cam3AParam, 1);
    }
    else{
        err = mHalCam3AInit(pmhalCamParam->u4CamMode, pmhalCamParam->cam3AParam, 0);
    }
    if (err < 0) {
        MCAM_ERR("[mHalCamPreviewStart] mHalCam3AInit() fail\n");
        goto mHalCamPreviewStart_EXIT;
    }
    //!
    //! due to 3AInit may cause a lot of time,
    //! start after the 3AInit to prevent frame drop
    err = mpCameraIOObj->start();
    if (err < 0) {
        MCAM_ERR ("[mHalCamPreviewStart] mpCameraIOObj->start() err (0x%x)\n", err );
        goto mHalCamPreviewStart_EXIT;
    }
    // workaround due to if do'nt set this , ISP will fail.
    mpHal3AObj->setPreviewParam();

    //
#ifdef  MHAL_CAM_FLICKER_SERVICE_SUPPORT
    {
        CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "create/init FlickerService");
        using namespace NSCamera;
        NSCamera::ESensorType const eSensorType = mapSensorType(meSensorType);
        NSCamera::EDeviceId const   eDeviceId = mapDeviceID(mHalCamSensorInfo[mCameraId].devType);
        //  Flicker
        if  ( NULL == mvpFrameService[eFSID_Flicker] )
        {
            IFlickerService* pService = IFlickerService::createInstance(eSensorType, eDeviceId);
            if  ( pService && pService->init(mpHal3AObj, mmHalCamParam.cam3AParam.antiBandingMode, eDeviceId, isZSDOn) )
            {
                mvpFrameService[eFSID_Flicker] = pService;
            }
            else
            {
                MCAM_ERR("[mHalCamPreviewStart] IFlickerService(%p)", pService);
            }
        }
    }
#endif
    // update zoom info
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "update zoom info");
    if (!isZSDOn) {
        if (pmhalCamParam->u4CamMode == MHAL_CAM_MODE_MVIDEO)
        {
        if (pmhalCamParam->cam3AParam.eisMode == MHAL_CAM_FEATURE_ON) {
            mpCameraIOObj->setZoomCropSrc(0, 0, pmhalCamParam->cam3AParam.eisW, pmhalCamParam->cam3AParam.eisH);
        }
        else {
            mpCameraIOObj->setZoomCropSrc(0, 0, mEISSrcW, mEISSrcH);
        }
    }
        else
        {
            // turn-off eis mode when preivew.
            mmHalCamParam.cam3AParam.eisMode= 0;
            mpCameraIOObj->setZoomCropSrc(0, 0, mEISSrcW, mEISSrcH);
        }
    }
    err = mHalCamSetPreviewZoom(pmhalCamParam->u4ZoomVal);
    if (err < 0) {
        MCAM_ERR ("[mHalCamPreviewStart] mHalCamPreviewZoom() err(0x%x) \n", err);
        goto mHalCamPreviewStart_EXIT;
    }
    if (isZSDOn == 0x1){
        pmhalCamParam->camZsdParam.u4ZsdZoomWidth = mmHalCamParam.camZsdParam.u4ZsdZoomWidth;
        pmhalCamParam->camZsdParam.u4ZsdZoomHeigth = mmHalCamParam.camZsdParam.u4ZsdZoomHeigth;
        MCAM_DBG("[mHalCamPreviewStart][ZSD]update zoom W:%d,H:%d \n", pmhalCamParam->camZsdParam.u4ZsdZoomWidth,pmhalCamParam->camZsdParam.u4ZsdZoomHeigth);
    }
    // AF setting
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "AF setting");
    if (pmhalCamParam->cam3AParam.afEngMode != 0) {
        // Disable AE/AWB
        mHalCam3ACtrl(0, 0, 1);
        mpHal3AObj->setMFPos(mAFBestPos + pmhalCamParam->cam3AParam.afEngPos);
    }

    if (pmhalCamParam->cam3AParam.afEngMode == 5) {
        mAFBestPos = mpHal3AObj->getAFBestPos();
        MCAM_DBG("mAFBestPos: %d", mAFBestPos);
        if (mAFBestPos) {
            mpHal3AObj->setMFPos(mAFBestPos);
        }
    }
    //
    #if (MHAL_CAM_ASD_OPTION)
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "ASD RegCallback");
    if(mpHalASDObj)
    {
        mpHalASDObj->RegCallback(pmhalCamParam->mhalObserver);
    }
    #endif
    CPTLog(Event_mHalCamPreviewStart, CPTFlagSeparator);
    //
    g_mhalObserver = pmhalCamParam->mhalObserver;
    // Clear Zoom Queue
    mLock.lock();
    mZoomQueue.clear();
    mLock.unlock();
    ::sem_post(&semMainHigh);

    mHalCamFocusThread(1);

mHalCamPreviewStart_EXIT:
    //
    if (err < 0) {
        // free the memory alread allocated
        mpCameraIOObj->stop();

        #if (MHAL_CAM_FD_OPTION)
        mfdObj->Uninit();
        #endif

        if (mFDWorkingBuf) {
            delete mFDWorkingBuf;
            mFDWorkingBuf = NULL;
        }
        mHalCamSetState(MHAL_CAM_ERROR);
    }
    //
    if (pDispCfg) {
        delete pDispCfg;
        delete pDispMemInfo;
    }
    //
    if (pVdoCfg) {
        delete pVdoCfg;
        delete pVdoMemInfo;
    }
    //
    if (pFDCfg) {
        delete pFDCfg;
        delete pFDMemInfo;
    }
    //
    camTmr.printTime();
    MCAM_DBG("[mHalCamPreviewStart] X \n");

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamPreviewStop(
)
{
    AutoCPTLog cptlog(Event_mHalCamPreviewStop);
    MINT32 err = MHAL_NO_ERROR;
    MCAM_DBG("[mHalCamPreviewStop] Start \n");
    CamTimer camTmr("mHalCamPreviewStop", MHAL_CAM_GENERIC_PROFILE_OPT);

    // for recording to stop
    mIsVideoRecording = MFALSE;
    mhalCamParam_t *pmhalCamParam = &mmHalCamParam;
    mHalCamState_e state = mHalCamGetState();
    //
    mHalCamFocusThread(0);

#if (MHAL_CAM_FRAME_SERVICE_SUPPORT)
    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator, "preUninit FrameService");
    for (MUINT32 i = 0; i < NSCamera::eFSID_NUM; i++)
    {
        if  ( NULL != mvpFrameService[i] )
        {
            mvpFrameService[i]->preUninit();
        }
    }
#endif
    //
    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator, "set state");
    if (state == MHAL_CAM_IDLE) {
        return err;
    }
    else if (state & MHAL_CAM_PREVIEW_MASK) {
        if (state == MHAL_CAM_FOCUS) {
            // It is in focus state, back to preview state first
            mHalCamSetState(MHAL_CAM_PREVIEW);
        }
        else if (state == MHAL_CAM_ZOOM) {
            // It is in zoom state, back to preview state first
            mHalCamSetState(MHAL_CAM_PREVIEW);
        }
        mHalCamSetState(MHAL_CAM_STOP);
    }
    else if (state == MHAL_CAM_PRE_CAPTURE) {
        // In preview capture, has to wait 3A ready flag
        MCAM_DBG("[mHalCamPreviewStop] it is MHAL_CAM_PRE_CAPTURE state \n");
    }
    else if (state == MHAL_CAM_CAPTURE) {
        // It is in capture flow now, preview has been already stopped
        MCAM_DBG("[mHalCamPreviewStop] it is MHAL_CAM_CAPTURE state \n");
        return err;
    }
    else {
        // Unknown
        MCAM_ERR("[mHalCamPreviewStop] state: %d \n", state);
        MCAM_ASSERT(0, "state is in under control situaion");
    }

    //MCAM_DBG("[mHalCamPreviewStop] Wait CAM_IDLE \n");
    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator, "wait semMainHighBack");
    ::sem_wait(&semMainHighBack);

    // !++ Revised: ICS FD
    // wait for fd done
    #if (MHAL_CAM_FD_OPTION)
    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator, "wait FD done");
    mfdObj->waitFDdone();
    #endif

     //
#if (MHAL_CAM_FRAME_SERVICE_SUPPORT)
    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator, "uninit FrameService");
    for (MUINT32 i = 0; i < NSCamera::eFSID_NUM; i++)
    {
        if  ( NULL != mvpFrameService[i] )
        {
            mvpFrameService[i]->uninit();
            mvpFrameService[i]->destroyInstance();
            mvpFrameService[i] = NULL;
        }
    }
#endif

    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator, "mHalCamStop");
    err = mHalCamStop();
    MCAM_ASSERT(err >= 0, "mHalCamStop err \n");
    //
    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator, "CameraIO setCallbacks");
    mpCameraIOObj->setCallbacks(NULL, NULL);

    //
#if (MHAL_CAM_EIS_OPTION)
    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator, "disable EIS");
    mpEISHalObj->disableEIS();
#endif
    //

    //
    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator, "delete mVdoWorkingBuf");
    if (mVdoWorkingBuf) {
        delete mVdoWorkingBuf;
        mVdoWorkingBuf = NULL;
    }
    //
    //!++ ICS FD
    #if (MHAL_CAM_FD_OPTION)
    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator, "uninit FD");
    mfdObj->Uninit();
    #endif
    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator , "delete mFDWorkingBuf");
    if (mFDWorkingBuf) {
        delete mFDWorkingBuf;
        mFDWorkingBuf = NULL;
    }
    //
    #if (MHAL_CAM_VIDEO_SNAPSHOT)
    if(mpVideoSnapshot)
    {
        mpVideoSnapshot->Release();
    }
    else
    {
        MCAM_ERR("mpVideoSnapshot is NULL");
    }
    #endif
    //
    mHalCamSetState(MHAL_CAM_IDLE);
    //
    mAaaState = MHAL_CAM_IDLE;
    //
    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator , "mHalSetResMgr");
    mHalSetResMgr(RES_MGR_HAL_MODE_PREVIEW_OFF);

    // Unlock Sysram resource
    //
    //MCAM_DBG("mHalCamPreviewStop End \n");

    camTmr.printTime();
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamPreCapture(
)
{
    MINT32 err = MHAL_NO_ERROR;

    MCAM_DBG("[mHalCamPreCapture] \n");

    mHalCamSetState(MHAL_CAM_PRE_CAPTURE);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamCaptureInit(
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize
)
{
    AutoCPTLog cptlog(Event_mHalCamCaptureInit);

    MINT32 err = MHAL_NO_ERROR;
    mhalCamParam_t *pmhalCamParam = (mhalCamParam_t *) a_pInBuffer;
    MUINT32 ispInW, ispInH;
    MUINT32 memSize;
    //
    MCAM_DBG("[mHalCamCaptureInit] Start \n");
    //
    mHalCamShowParam(pmhalCamParam);
    MCAM_ASSERT(mHalCamGetState() == MHAL_CAM_IDLE, "Camera State not IDLE \n");
    //MCAM_ASSERT((pmhalCamParam->frmJpg.bufSize >= MHAL_CAM_RAW_BUF_SIZE), "JPEG buffer size is too small for temporary raw buffer \n");
    //
    memcpy(&mmHalCamParam, pmhalCamParam, sizeof(mhalCamParam_t));
    //
    mHalSetResMgr(RES_MGR_HAL_MODE_CAPTURE_ON);
    // Use Jpeg buffer as raw buffer
    //
    g_mhalObserver = pmhalCamParam->mhalObserver;
    //
    //
    NSCamera::ESensorType const eShotSensorType = mapSensorType(meSensorType);
    NSCamera::EDeviceId const eShotDeviceId = mapDeviceID(mHalCamSensorInfo[mCameraId].devType);
    //
    MCAM_DBG("[mHalCamCaptureInit] u4ShotMode %d \n",mmHalCamParam.u4ShotMode);

    switch  (mmHalCamParam.u4ShotMode)
    {
#ifdef  MTK_CAM_HDR_SUPPORT
    case MHAL_CAM_CAP_MODE_HDR:
        {
            IHdr*   pShot = IHdr::createInstance(eShotSensorType, eShotDeviceId);
            if  ( ! pShot || ! pShot->init(mmHalCamParam, mpHal3AObj) )
            {
                MCAM_ERR("[mHalCamCaptureInit]<MHAL_CAM_CAP_MODE_HDR> pShot(%p)->init() fail", pShot);
                if  ( pShot )
                {
                    pShot->destroyInstance();
                    pShot = NULL;
                }
                err = -1;
                break;
            }
            mpShot = pShot;
        }
        break;
#endif  //MTK_CAM_HDR_SUPPORT
#ifdef  MTK_CAM_FACEBEAUTY_SUPPORT
    case MHAL_CAM_CAP_MODE_FACE_BEAUTY:
        {
            camera_face_metadata_m* faceinfo = mfdProvider->getFDResult();
            if(faceinfo->number_of_faces)
            {
                IMhal_facebeauty*   pShot = IMhal_facebeauty::createInstance(eShotSensorType, eShotDeviceId);
                if  ( ! pShot || ! pShot->init(mmHalCamParam, mpHal3AObj, (MUINT32)mfdProvider->getFDResult(), (MUINT32)mfdProvider->getFaceInfo()) )
                {
                    MCAM_ERR("[mHalCamCaptureInit]<MHAL_CAM_CAP_MODE_FB_SHOT> pShot(%p)->init() fail", pShot);
                    if  ( pShot )
                    {
                        pShot->destroyInstance();
                        pShot = NULL;
                    }
                    err = -1;
                    goto mHalCamCaptureConfig_EXIT;
                }
                mpShot = pShot;
            }
            else
            {
                NormalShot* pShot = NormalShot::createInstance("NormalShot", eShotSensorType, eShotDeviceId);
                if  ( ! pShot || ! pShot->init(mmHalCamParam, mpHal3AObj, mpCameraIOObj) )
                {
                    MCAM_ERR("[mHalCamCaptureInit]<ShotMode:%d> pShot(%p)->init() fail", mmHalCamParam.u4ShotMode, pShot);
                    if  ( pShot )
                    {
                        pShot->destroyInstance();
                        pShot = NULL;
                    }
                    err = -1;
                    goto mHalCamCaptureConfig_EXIT;
                }
                mpShot = pShot;
            }
        }
        break;
#endif
    case MHAL_CAM_CAP_MODE_BEST_SHOT:
        {
            IBestShot* pShot = IBestShot::createInstance(eShotSensorType, eShotDeviceId);
            if  ( ! pShot || ! pShot->init(mmHalCamParam, mpHal3AObj, mpCameraIOObj) )
            {
                MCAM_ERR("[mHalCamCaptureInit]<MHAL_CAM_CAP_MODE_BEST_SHOT> pShot(%p)->init() fail", pShot);
                if  ( pShot )
                {
                    pShot->destroyInstance();
                    pShot = NULL;
                }
                err = -1;
                break;
            }
            mpShot = pShot;
        }
        break;

    case MHAL_CAM_CAP_MODE_BURST_SHOT:
        {
            BurstShot* pShot = BurstShot::createInstance(eShotSensorType, eShotDeviceId);
            if  ( ! pShot || ! pShot->init(mmHalCamParam, mpHal3AObj, mpCameraIOObj) )
            {
                MCAM_ERR("[mHalCamCaptureInit]<MHAL_CAM_CAP_MODE_BURST_SHOT> pShot(%p)->init() fail", pShot);
                if  ( pShot )
                {
                    pShot->destroyInstance();
                    pShot = NULL;
                }
                err = -1;
                break;
            }
            mpShot = pShot;
        }
        break;

    case MHAL_CAM_CAP_MODE_CONTINUOUS_SHOT:
        {
            ContinuousShot* pShot = ContinuousShot::createInstance(eShotSensorType, eShotDeviceId);
            if  ( ! pShot || ! pShot->init(mmHalCamParam, mpHal3AObj, mpCameraIOObj) )
            {
                MCAM_ERR("[mHalCamCaptureInit]<MHAL_CAM_CAP_MODE_CONTINUOUS_SHOT> pShot(%p)->init() fail", pShot);
                if  ( pShot )
                {
                    pShot->destroyInstance();
                    pShot = NULL;
                }
                err = -1;
                break;
            }
            mpShot = pShot;
        }
        break;
    case MHAL_CAM_CAP_MODE_NORMAL:
    default:
        {
            NormalShot* pShot = NormalShot::createInstance("NormalShot", eShotSensorType, eShotDeviceId);
            if  ( ! pShot || ! pShot->init(mmHalCamParam, mpHal3AObj, mpCameraIOObj) )
            {
                MCAM_ERR("[mHalCamCaptureInit]<ShotMode:%d> pShot(%p)->init() fail", mmHalCamParam.u4ShotMode, pShot);
                if  ( pShot )
                {
                    pShot->destroyInstance();
                    pShot = NULL;
                }
                err = -1;
                break;
            }
            mpShot = pShot;
        }
        break;
    }
    //
mHalCamCaptureConfig_EXIT:
    //
    if (err < 0) {
         mHalCamSetState(MHAL_CAM_ERROR);
    }
    //
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamCaptureUninit(
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize
)
{
    AutoCPTLog cptlog(Event_mHalCamCaptureUninit);
    MINT32 err = MHAL_NO_ERROR;
    //
    if  ( mpShot )
    {
        mpShot->uninit();
        mpShot->destroyInstance();
        mpShot = NULL;
    }
    //
    if  ( mpFlashlightObj )
    {
        mpHal3AObj->set3AZSDMode(0);    // cotta-- initialize
        mpFlashlightObj->mHalFlashlightSetFire(0);
    }
    //
    mHalSetResMgr(RES_MGR_HAL_MODE_CAPTURE_OFF);
    //

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamCaptureStart(
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize
)
{
    AutoCPTLog cptlog(Event_mHalCamCaptureStart);
    MINT32 err = MHAL_NO_ERROR;
    mhalCamParam_t *pmhalCamParam = (mhalCamParam_t *) a_pInBuffer;

    MCAM_DBG("[mHalCaptureStart] Start \n");
    //
    memcpy(&mmHalCamParam, pmhalCamParam, sizeof(mhalCamParam_t));
    //

    if(mHalCamSensorInfo[mCameraId].devType == MHAL_CAM_SENSOR_DEV_SUB && MTRUE == mRetFakeSubOrientation && pmhalCamParam->u4CamMode != MHAL_CAM_MODE_VT)
    {
        if (mHalCamSensorInfo[mCameraId].orientation == 0)
        {
            mmHalCamParam.u4Rotate = 3;
        }
        else if (mHalCamSensorInfo[mCameraId].orientation == 180)
        {
            mmHalCamParam.u4Rotate = 3;
        }
        else if (mHalCamSensorInfo[mCameraId].orientation == 90)
        {
            mmHalCamParam.u4Rotate = 2;
        }
        MCAM_DBG("front sensor capture rotation reset to = %d \n", mmHalCamParam.u4Rotate);
    }

    if(1 == mmHalCamParam.u4Scalado || 0 != mmHalCamParam.u4Rotate || 0 != mmHalCamParam.u4JPEGOrientation)
    {
        mmHalCamParam.u4DumpYuvData = 1;
    }

    if  ( mpShot )
    {
        mpShot->setParam(mmHalCamParam);
    }
    //
    mHalCamSetState(MHAL_CAM_CAPTURE);
    //
    CPTLogStr(Event_mHalCamCaptureStart, CPTFlagSeparator, "post semMainHigh");
    ::sem_post(&semMainHigh);
    //
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamSetZoom(
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize
)
{
    MINT32 err = MHAL_NO_ERROR;
    MUINT32 zoom = *(MUINT32 *) a_pInBuffer;
    //
    MCAM_DBG("[mHalCamSetZoom] %d \n", zoom);
    //
    Mutex::Autolock _l(mLock);
    mmHalCamParam.u4ZoomVal = zoom;
    //
    // if the last zoom value in the queue is the same as the setting --> skip
    // else push it in queue
    if (!mZoomQueue.empty()) {
        MUINT32 lastZoom = *mZoomQueue.end();
        if (lastZoom != zoom) {
            mZoomQueue.push_back(zoom);
        }
    }
    else {
        mZoomQueue.push_back(zoom);
    }
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamSendCommand(
    MINT32 cmd,
    MINT32 arg1,
    MINT32 arg2,
    MINT32 arg3
)
{
    MINT32 ret = MHAL_NO_ERROR;

    MCAM_DBG("[mHalCamSendCommand] cmd: 0x%x \n", cmd);
    //
    switch (cmd) {
    case CAM_CMD_SET_SHOT_MODE:
        mmHalCamParam.u4ShotMode = arg1;
        MCAM_DBG("new shot mode: %d", arg1);
        //
#if (MHAL_CAM_ASD_OPTION)
        if(mpHalASDObj)
        {
            if(arg1 == MHAL_CAM_CAP_MODE_ASD)
            {
                mpHalASDObj->Enable(MTRUE);
            }
            else
            {
                mpHalASDObj->Enable(MFALSE);
            }
        }
#endif
        break;
    //
    case CAM_CMD_GET_RAW_IMAGE_INFO:
        ret = mHalCamGetCaptureInfo((mhalCamRawImageInfo_t *) arg1, arg2);
        break;
    //
    case CAM_CMD_GET_BS_INFO:
        break;
    //
    case CAM_CMD_SET_FLASHLIGHT_PARAMETER:
        ret = mHalCamSetFlashlightParameter((MINT32 *) arg1);
        break;
    //!++ ICS FD
    case CAM_CMD_SET_SMILE_PREVIEW:
        #if (MHAL_CAM_FD_OPTION)
        if (arg1 == 1) {
            MCAM_DBG("CAM_CMD_SET_SMILE_PREVIEW Enable");
            if (mfdObj)
                mfdObj->setSDstate(1);
        }
        else {
            MCAM_DBG("CAM_CMD_SET_SMILE_PREVIEW Disable");
            if (mfdObj)
                mfdObj->setSDstate(0);
        }
        #endif
        break;
    //!++ ICS FD
    case CAM_CMD_SET_FACE_PREVIEW:
        #if (MHAL_CAM_FD_OPTION)
        if (arg1 == 1) {
            MCAM_DBG("CAM_CMD_SET_FACE_PREVIEW Enable");
            //
            if (mfdObj) {
                mfdObj->setFDstate(1);
                /* Let mHalCamMainNormalThread run for FD Proc */
                // FIXME: move under mHalCamFD
                ::sem_post(&semFDthread);
            }
        }
        else {
            MCAM_DBG("CAM_CMD_SET_FACE_PREVIEW Disable");
            if (mfdObj)
                mfdObj->setFDstate(0);
        }
        #endif
        break;
    //
    case CAM_CMD_DO_PANORAMA:
        ret = mHalCamDoPanorama((mhalCamParam_t *) arg1);
        break;
    //
    case CAM_CMD_CANCEL_PANORAMA:
        ret = mHalCamCancelPanorama((mhalCamParam_t *) arg1);
        break;
    //
    case CAM_CMD_START_MAV:
    {
        #ifdef MTK_MAV_SUPPORT
            MCAM_DBG("CAM_CMD_START_MAV");
            MINT32 errMsg = MHAL_NO_ERROR;
            mhal_MAV *mhalMAVobj = mhal_MAV::createInstance(mapSensorType(meSensorType));
            if (mhalMAVobj)
            {
                errMsg = mhalMAVobj->init(this, mpHal3AObj, mpCameraIOObj, mFDWorkingBuf);
                if (errMsg != MHAL_NO_ERROR)
                {
                    return errMsg;
                }
                mhal3DObj = mhalMAVobj;
            }
            else
            {
               MCAM_ERR("MAV fail: new MAV object ");
               if (errMsg != MHAL_NO_ERROR)
               {
                   return errMsg;
               }
            }

            mhalCamParam_t *pmhalCamParam = (mhalCamParam_t *)arg1;
            mmHalCamParam.camExifParam.orientation = pmhalCamParam->camExifParam.orientation;
            errMsg = mhal3DObj->mHalCamFeatureInit((mhalCamParam_t *) arg1);
            if (errMsg != MHAL_NO_ERROR)
            {
                return errMsg;
            }
        #endif
        break;
    }
    //
    case CAM_CMD_STOP_MAV:
    {
        #ifdef MTK_MAV_SUPPORT
            MCAM_DBG("CAM_CMD_STOP_MAV, %d", *(MINT32 *) arg1);
            MINT32 errMsg = MHAL_NO_ERROR;

            if(mhal3DObj)
            {
                if (*(MINT32 *) arg1 == 1)
                {
                    if ((mHalCamGetState() & MHAL_CAM_PREVIEW_MASK) == 0)
                    {
                        MCAM_DBG("[abnormal]Not in the preview state");
                        MINT32 cbMsg = ERR_RESET;
                        mHalCamCBHandle(MHAL_CAM_CB_ERR, &cbMsg);
                    }
                    mHalCamPreviewStop();
                    errMsg = mhal3DObj->mHalCamFeatureCompress();
                    if (errMsg != MHAL_NO_ERROR)
                    {
                        return errMsg;
                    }
                    mHalCamPreviewStart( &mmHalCamParam, sizeof(mhalCamParam_t) );
                    // let AP knows MPO is done, and block autofocus until preview has started
                    mHalCamCBHandle(MHAL_CAM_CB_MAV, NULL);
                }
                else
                {
                    LOGD("Cancel MAV. Either by user-self or by error handling");
                }
                mhal3DObj->mHalCamFeatureUninit();
                mhal3DObj->destroyInstance();
                mhal3DObj = NULL;
            }
            else
            {
               MCAM_ERR("MAV fail: mhal3DObj is NULL");
            }
        #endif

        break;
    }
    case CAM_CMD_START_AUTORAMA:
    {
        #if (MHAL_CAM_AUTORAMA_OPTION)
            MINT32 err = MHAL_NO_ERROR;
            MCAM_DBG("CAM_CMD_START_AUTORAMA");

            mhal_Autorama *mhalAUTORAMAobj = mhal_Autorama::createInstance(mapSensorType(meSensorType));
            if ( mhalAUTORAMAobj )
            {
                err = mhalAUTORAMAobj->init(this, mpHal3AObj, mpCameraIOObj, mFDWorkingBuf, mHalCamSensorInfo[mCameraId].orientation);
                if ( err != MHAL_NO_ERROR )
                {
                    return err;
                }
                mhal3DObj = mhalAUTORAMAobj;
            }
            else
            {
               MCAM_ERR("AUTORAMA fail: new AUTORAMA object");
               if ( err != MHAL_NO_ERROR )
               {
                   return err;
               }
            }

            mhalCamParam_t *pmhalCamParam = (mhalCamParam_t *)arg1;
            mmHalCamParam.camExifParam.orientation = pmhalCamParam->camExifParam.orientation;
            err = mhal3DObj->mHalCamFeatureInit((mhalCamParam_t *) arg1);
            if ( err != MHAL_NO_ERROR )
            {
                return err;
            }
        #endif

        break;
    }
    //
    case CAM_CMD_STOP_AUTORAMA:
    {
        #if (MHAL_CAM_AUTORAMA_OPTION)
            MINT32 err = MHAL_NO_ERROR;
            MCAM_DBG("CAM_CMD_STOP_AUTORAMA, %d", *(MINT32 *) arg1);
            if(mhal3DObj)
            {
                if (*(MINT32 *) arg1 == 1)
                {
                    if ((mHalCamGetState() & MHAL_CAM_PREVIEW_MASK) == 0)
                    {
                        MCAM_DBG("[abnormal]Not in the preview state");
                        MINT32 cbMsg = ERR_RESET;
                        mHalCamCBHandle(MHAL_CAM_CB_ERR, &cbMsg);
                    }
                    // Do merge
                    if( !((mhal_Autorama*)mhal3DObj)->AUTORAMADone() )
                    {
                        MCAM_DBG("  CAM_CMD_STOP_AUTORAMA: Merge Accidently ");
                        err = mhal3DObj->mHalCamFeatureMerge();
                        if (err != MHAL_NO_ERROR) {
                            return err ;
                        }
                    }
                    mHalCamPreviewStop();
                    mhal3DObj->mHalCamFeatureCompress();
                    mHalCamPreviewStart( &mmHalCamParam, sizeof(mhalCamParam_t) );

                    MINT32 cb[2] = {0, 50};
                    mHalCamCBHandle(MHAL_CAM_CB_AUTORAMA, (void*)&cb);
                }
                else
                {
                    MCAM_DBG("  CAM_CMD_STOP_AUTORAMA: Cancel");
                }
                mhal3DObj->mHalCamFeatureUninit();
                mhal3DObj->destroyInstance();
                mhal3DObj = NULL;
            }
            else
            {
               MCAM_ERR("AUTORAMA fail: mhal3DObj is NULL");
            }
        #endif
        break;
    }
    case CAM_CMD_START_3DSHOT: // between 1st pic and 2nd pic
    {
        break;
    }
    case CAM_CMD_STOP_3DSHOT:
    {
        break;
    }

    //
    case CAM_CMD_SET_ATV_DISP:
        mIsAtvDisp = MTRUE;
        break;
    //
    case CAM_CMD_GET_ATV_DISP_DELAY:
        ret = mpIspHalObj->sendCommand(ISP_CMD_GET_ATV_DISP_DELAY, (int)arg1);
        break;
    //
    case CAM_CMD_GET_BUF_SUPPORT_FMT:
        //! arg1 is the support format that proprotary for MTK
        //! arg2 is the default format of preview format
        using namespace NSCamera;
//        ((MINT32*)arg1)[0] = ePIXEL_FORMAT_NV21;
//        ((MINT32*)arg1)[1] = ePIXEL_FORMAT_NV21;
//        ((MINT32*)arg1)[0] = ePIXEL_FORMAT_YV12;
//        ((MINT32*)arg1)[1] = ePIXEL_FORMAT_YV12;

        ((MINT32*)arg1)[0] = ePIXEL_FORMAT_I420;
        ((MINT32*)arg1)[1] = ePIXEL_FORMAT_I420;

        ret = MHAL_NO_ERROR;
        break;
    case CAM_CMD_GET_3A_SUPPORT_FEATURE:
        {
            mHalCam3ASupportFeature_t *p3AFeature = (mHalCam3ASupportFeature_t*)arg1;
            p3AFeature->u4AutoWhiteBalanceLock = (MUINT32)mpHal3AObj->isAutoWhiteBalanceLockSupported();
            p3AFeature->u4ExposureLock = (MUINT32) mpHal3AObj->isAutoExposureLockSupported();
            p3AFeature->u4FocusAreaNum = (MUINT32) mpHal3AObj->getMaxNumFocusAreas();
            p3AFeature->u4MeterAreaNum = (MUINT32) mpHal3AObj->getMaxNumMeteringAreas();
            ret = MHAL_NO_ERROR;
        }
        break;
    //
    #if (MHAL_CAM_VIDEO_SNAPSHOT)
    case CAM_CMD_VIDEO_SNAPSHOT:
    {

        MCAM_DBG("CAM_CMD_VIDEO_SNAPSHOT");
        if(mpVideoSnapshot)
        {
            mpVideoSnapshot->TakePic((MUINT32)arg1);
        }
        else
        {
            MCAM_ERR("mpVideoSnapshot is NULL");
        }
        break;
    }
    #endif
    //
    case CAM_CMD_CANCEL_PICTURE:
        if  ( mpShot ) {
            MCAM_DBG("<CAM_CMD_CANCEL_PICTURE>");
            mpShot->cancel();
        }
        break;
    //
    default:
        MCAM_DBG("cmd(%d) not implement",cmd);
        ret = -1;
        break;
    }

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::
mHalCamSetFeatureMode(MVOID *a_pInBuffer, MUINT32 a_u4InBufSize)
{
    MINT32 err = MHAL_NO_ERROR;

    mhalCam3AParam_t*const pmhalCam3AParam = reinterpret_cast<mhalCam3AParam_t*>(a_pInBuffer);

    NSFeature::FeatureHal* pFeatureHalObj = NULL;

    if  ( ISP_SENSOR_DEV_ATV == mHalCamSensorInfo[mCameraId].devType )
    {
        //  FIXME: No feature table is reserved for ATV. But we still return success.
        MCAM_ERR("[mHalCamSetFeatureMode]:not support ATV\n");
        goto lbExit;
    }

    pFeatureHalObj = NSFeature::FeatureHal::createInstance(meFeatureCamRole);
    if (!pFeatureHalObj) {
        MCAM_ERR("[mHalCamSetFeatureMode]:invalid resource(pFeatureHalObj==NULL)\n");
        err = MHAL_INVALID_RESOURCE;
        goto lbExit;
    }

    if ( ! pmhalCam3AParam || a_u4InBufSize < sizeof(mhalCam3AParam_t) ) {
        MCAM_ERR("[mHalCamSetFeatureMode]:invalid para(a_pInBuffer, a_u4InBufSize)=(%p, %d)\n", a_pInBuffer, a_u4InBufSize);
        err = MHAL_INVALID_PARA;
        goto lbExit;
    }

    if (!pFeatureHalObj->setSceneMode(pmhalCam3AParam->sceneMode)) {
        MCAM_ERR("[mHalCamSetFeatureMode]:invalid new scene (%d)\n", pmhalCam3AParam->sceneMode);
        err = MHAL_INVALID_PARA;
        goto lbExit;
    }

lbExit:
    if (pFeatureHalObj) {
        pFeatureHalObj->destroyInstance();
    }
    MCAM_DBG("[-mHalCamSetFeatureMode]:err(%d)\n", err);
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::
mHalCamGetFeatureEnum(
    MVOID*const   a_pOutBuf,
    MUINT32 const a_u4OutBufNum,
    MUINT32&      ra_u4NumReturned
)
{
    MINT32 err = MHAL_NO_ERROR;
    MHAL_BOOL rtRet;
    NSFeature::FeatureHal* pFeatureHalObj = NULL;

    if  ( ISP_SENSOR_DEV_ATV == mHalCamSensorInfo[mCameraId].devType )
    {
        //  FIXME: No feature table is reserved for ATV. We return failure so that a set of default table is assigned by callers.
        MCAM_ERR("[mHalCamGetFeatureEnum]:not support ATV\n");
        err = MHAL_UNKNOWN_ERROR;
        goto lbExit;
    }

    pFeatureHalObj = NSFeature::FeatureHal::createInstance(meFeatureCamRole);
    if (!pFeatureHalObj) {
        MCAM_ERR("[mHalCamGetFeatureEnum]:invalid resource(pFeatureHalObj==NULL)\n");
        err = MHAL_INVALID_RESOURCE;
        goto lbExit;
    }

    rtRet = pFeatureHalObj->queryFeatures(a_pOutBuf, a_u4OutBufNum, ra_u4NumReturned);
    if (!rtRet) {
        MCAM_ERR(
            "[mHalCamGetFeatureEnum]:"
            "invalid para(a_pOutBuf, a_u4OutBufNum)=(%p, %d)\n"
            , a_pOutBuf, a_u4OutBufNum
        );
        err = MHAL_INVALID_PARA;
        goto lbExit;
    }

lbExit:
    if (pFeatureHalObj) {
        pFeatureHalObj->destroyInstance();
    }
    MCAM_DBG("-[mHalFeatureSetMode]:(err,ra_u4NumReturned)=(%d,%d)\n", err, ra_u4NumReturned);
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 mHalCam::mHalCamGetCaptureInfo(
    mhalCamRawImageInfo_t *prawImageInfo,
    MINT32 mode
)
{
    MINT32 err = 0;
    halISPRawImageInfo_t ispRawInfo;
    //
    MCAM_DBG("[mHalCamGetCaptureInfo] \n");
    //
    err = mpIspHalObj->sendCommand(ISP_CMD_GET_RAW_INFO, (int) &ispRawInfo, mode);
    if (err < 0) {
        MCAM_ERR("  ISP_CMD_GET_RAW_INFO err");
        return err;
    }
    //
    mmHalRawImageInfo.u4Width = ispRawInfo.u4Width;
    mmHalRawImageInfo.u4Height = ispRawInfo.u4Height;
    mmHalRawImageInfo.u4BitDepth = ispRawInfo.u4BitDepth;
    mmHalRawImageInfo.u4IsPacked = ispRawInfo.u4IsPacked;
    //mmHalRawImageInfo.u4Size = ispRawInfo.u4Size + MHAL_CAM_JPEG_ADDR_OFFSET;
    mmHalRawImageInfo.u4Size = ispRawInfo.u4Size;
    mmHalRawImageInfo.u1Order = ispRawInfo.u1Order;

    //for (int i = 0; i < 4; i++) {
    //    mmHalRawImageInfo.u1Order[i] = ispRawInfo.u1Order[i];
    //}
    //
    memcpy(prawImageInfo, &mmHalRawImageInfo, sizeof(mhalCamRawImageInfo_t));

    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 mHalCam::mHalCamDoPanorama(mhalCamParam_t *pmhalCamParam)
{
    INT32 ret = MHAL_NO_ERROR;

    MCAM_DBG("[mHalCamDoPanorama] \n");
    //
    memcpy(&mmHalCamParam, pmhalCamParam, sizeof(mhalCamParam_t));
    //
    mHalCamSetState(MHAL_CAM_PANORAMA);
    //
    ::sem_post(&semMainHigh);

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 mHalCam::mHalCamCancelPanorama(mhalCamParam_t *pmhalCamParam)
{
    INT32 ret = MHAL_NO_ERROR;

    MCAM_DBG("[mHalCamCancelPanorama] \n");
    //
    mHalCam3ACtrl(1, 1, 1);
    // Restore the AF indicator when cancel panorama
    mmHalCamParam.u4IsDrawAFIndicator = pmhalCamParam->u4IsDrawAFIndicator;
    mmHalCamParam.cam3AParam.afIndicator = pmhalCamParam->cam3AParam.afIndicator;

    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamInit(
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize
)
{
    AutoCPTLog cptlog(Event_mHalCamInit);
    MINT32 err = MHAL_NO_ERROR;
    MINT32 cameraId = 0;
    MUINT32 sensorDev = ISP_SENSOR_DEV_MAIN;
    Hal3ASensorType_e type[2] = {HAL3A_SENSOR_TYPE_RAW, HAL3A_SENSOR_TYPE_YUV};

    MCAM_DBG("[mHalCamInit] \n");
    //
    cameraId = (MINT32) a_pInBuffer;
    //
    MCAM_DBG("  cameraId: %d, %d \n", cameraId, mHalCamSensorInfo[cameraId].devType);
    mCameraId = cameraId;
    mCameraState = MHAL_CAM_INIT;
    mIsAtvDisp = MFALSE;
    sensorDev = mHalCamSensorInfo[cameraId].devType;
    //
    CPTLogStr(Event_mHalCamInit, CPTFlagSeparator, "init ResMgr");
    if(!(mResMgrHalObj->Init()))
    {
        err  = MHAL_INVALID_RESOURCE;
        return err;
    }
    //
    CPTLogStr(Event_mHalCamInit, CPTFlagSeparator, "mHalSetResMgr");
    mHalSetResMgr(RES_MGR_HAL_MODE_PREVIEW_OFF);
    //
    meFeatureCamRole = NSFeature::ECamRole_Main;
    if (sensorDev == ISP_SENSOR_DEV_SUB) {
        meFeatureCamRole = NSFeature::ECamRole_Sub;
    }
    //should create/init before isp/feature/3a
    CPTLogStr(Event_mHalCamInit, CPTFlagSeparator, "create/init Flashlight");
    mpFlashlightObj = halFLASHLIGHTBase::createInstance();
    if (mpFlashlightObj) {
        err = mpFlashlightObj->mHalFlashlightInit(sensorDev);
        if (err < 0) {
            return err;
        }
    }
    //
    CPTLogStr(Event_mHalCamInit, CPTFlagSeparator, "create/init IspHal");
    mpIspHalObj = IspHal::createInstance();
    mpIspHalObj->sendCommand(ISP_CMD_SET_SENSOR_DEV, (int) sensorDev);
    err = mpIspHalObj->init();
    if (err < 0) {
        return err;
    }
    err = mpIspHalObj->sendCommand(ISP_CMD_GET_SENSOR_TYPE, (int) &meSensorType);
    if (err < 0) {
        return err;
    }
    //
    CPTLogStr(Event_mHalCamInit, CPTFlagSeparator, "create/init 3A");
    mpHal3AObj = Hal3ABase::createInstance(type[meSensorType], sensorDev);
    if (mpHal3AObj == NULL) {
        return err;
    }
    //
    err = mpHal3AObj->init(sensorDev);
    if (err < 0) {
        MCAM_ERR("  mpHal3AObj->init fail \n");
        return -1;
    }
    //
    #if (MHAL_CAM_EIS_OPTION)
    CPTLogStr(Event_mHalCamInit, CPTFlagSeparator, "create EIS");
    //! Only camera can use EIS function.
    if (sensorDev != ISP_SENSOR_DEV_ATV) {
        mpEISHalObj = EisHalBase::createInstance();
    }
    else {
        mpEISHalObj = new EisNone();
    }
    #endif
    //

    #if (MHAL_CAM_FD_OPTION) //!++ FD ICS
    CPTLogStr(Event_mHalCamInit, CPTFlagSeparator, "create FD/FDProvider");
    mfdObj =  new mHalCamFD();
    mfdObj->createInstance(HAL_FD_OBJ_HW);
    ///
    ::sem_init(&semFDthread, 0, 0);
    ::sem_init(&semFDthreadEnd, 0, 0);
    pthread_create(&threadHMainNormal, NULL, mHalCamFDThread, NULL);
    ///
    mfdProvider = new mHalFDProvider();
    mfdProvider->createInstance(mHalCamFD::MAX_FACE_NUM);
    #endif

    #if (MHAL_CAM_ASD_OPTION)
    CPTLogStr(Event_mHalCamInit, CPTFlagSeparator, "create/init ASD");
    mpHalASDObj = mHal_ASD::CreateInstance();
    CPTLog(Event_ASDInit, CPTFlagStart);
    if(mpHalASDObj->Init() != MHAL_NO_ERROR)
    {
        MCAM_ERR("init ASD fail");
    }
    CPTLog(Event_ASDInit, CPTFlagEnd);
    #endif
    //
    CPTLogStr(Event_mHalCamInit, CPTFlagSeparator, "create/init CameraIO");
    //
    #if (MHAL_CAM_VIDEO_SNAPSHOT)
    mpVideoSnapshot = VideoSnapshot::CreateInstance();
    if(mpVideoSnapshot->Init() != MHAL_NO_ERROR)
    {
        MCAM_ERR("init VideoSnapshot fail");
    }
    #endif
    //
    mpCameraIOObj = ICameraIO::createInstance();
    err = mpCameraIOObj->init(mpIspHalObj);
    if (err < 0) {
        return err;
    }
    // Init semphore
    CPTLogStr(Event_mHalCamInit, CPTFlagSeparator, "init sem/create thread");
    ::sem_init(&semMainHigh, 0, 0);
    ::sem_init(&semMainHighBack, 0, 0);
    ::sem_init(&semMainHighEnd, 0, 0);

    // Create main thread for preview and capture
    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_RR, RTPM_PRIO_CAMERA_COMPRESS};
    pthread_create(&threadHMainHigh, &attr, mHalCamMainHighThread, NULL);

    //
    mHalCamSetState(MHAL_CAM_IDLE);
    //
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.camera.dump", value, "0");
    mCamDumpOPT = atoi(value);

    property_get("debug.camera.debug", value, "1");
    MCAM_LOG_DBG = atoi(value);
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamUninit(
)
{
    AutoCPTLog cptlog(Event_mHalCamUninit);
    MINT32 err = MHAL_NO_ERROR;
    mHalCamState_e eState;

    MCAM_DBG("[mHalCamUninit] \n");

    // Check it is in the idle mode
    // If it is not, has to wait until idle
    eState = mHalCamGetState();
    if ((eState != MHAL_CAM_IDLE) && (eState != MHAL_CAM_ERROR)) {
        MCAM_DBG("  Camera is not in the idle state ... \n");
        if (eState & MHAL_CAM_PREVIEW_MASK) {
            mHalCamPreviewStop();
        }
        else if (eState & MHAL_CAM_CAPTURE_MASK) {

        }
        // Polling until idle
        while (eState != MHAL_CAM_IDLE) {
            // Wait 10 ms per time
            usleep(10000);
            eState = mHalCamGetState();
        }
        MCAM_DBG("  Now camera is in the idle state \n");
    }
    //
    mHalCamSetState(MHAL_CAM_UNINIT);
    ::sem_post(&semMainHigh);
    CPTLogStr(Event_mHalCamUninit, CPTFlagSeparator, "wait semMainHighEnd");
    ::sem_wait(&semMainHighEnd);
    MCAM_DBG("wait for semMainHighEnd");
    //
    CPTLogStr(Event_mHalCamUninit, CPTFlagSeparator, "uninit FrameService");
#if (MHAL_CAM_FRAME_SERVICE_SUPPORT)
    for (MUINT32 i = 0; i < NSCamera::eFSID_NUM; i++)
    {
        if  ( NULL != mvpFrameService[i] )
        {
            mvpFrameService[i]->uninit();
            mvpFrameService[i]->destroyInstance();
            mvpFrameService[i] = NULL;
        }
    }
#endif
    //
    CPTLogStr(Event_mHalCamUninit, CPTFlagSeparator, "uninit ASD");
    #if (MHAL_CAM_ASD_OPTION)
    if(mpHalASDObj)
    {
        mpHalASDObj->Uninit();
        mpHalASDObj->DestroyInstance();
        mpHalASDObj = NULL;
    }
    #endif
    //
    #if (MHAL_CAM_VIDEO_SNAPSHOT)
    if(mpVideoSnapshot)
    {
        mpVideoSnapshot->Uninit();
        mpVideoSnapshot->DestroyInstance();
        mpVideoSnapshot = NULL;
    }
    #endif
    // mhal3Dobj uninitialization should be executed at least earlier than 3A uninitialization
    CPTLogStr(Event_mHalCamUninit, CPTFlagSeparator, "uninit 3D");
    #if (MHAL_CAM_MAV_OPTION)
    if (mhal3DObj){
        MCAM_DBG("%s shut down accidently", mhal3DObj->getFeatureName());
        mhal3DObj->mHalCamFeatureUninit();
        mhal3DObj->destroyInstance();
        mhal3DObj = NULL;
    }
    #endif
    //
    CPTLogStr(Event_mHalCamUninit, CPTFlagSeparator, "uninit ISP");
    err = mpIspHalObj->uninit();
    if (err < 0) {
        return err;
    }
    mpIspHalObj->destroyInstance();
    mpIspHalObj = NULL;
    //
    CPTLogStr(Event_mHalCamUninit, CPTFlagSeparator, "uninit 3A");
    if (mpHal3AObj) {
        mpHal3AObj->uninit();
        mpHal3AObj->destroyInstance();
        mpHal3AObj = NULL;
    }
    //
    CPTLogStr(Event_mHalCamUninit, CPTFlagSeparator, "destroy FD");
    #if (MHAL_CAM_FD_OPTION) // !++ ICS FD
    if (mfdObj) {
        mfdObj->setCamDestroy();
    }
    ::sem_post(&semFDthread);
    ::sem_wait(&semFDthreadEnd);
    MCAM_DBG("FDthread is back");
    if (mfdObj) {
        mfdObj->destroyInstance();
        mfdObj = NULL;
    }

    if (mfdProvider) {
        mfdProvider->destroyInstance();
        mfdProvider = NULL;
    }
    #endif
    //
    CPTLogStr(Event_mHalCamUninit, CPTFlagSeparator, "destroy EIS");
    #if (MHAL_CAM_EIS_OPTION)
        if (mpEISHalObj) {
            mpEISHalObj->destroyInstance();
            mpEISHalObj = NULL;
        }
    #endif
    //
    CPTLogStr(Event_mHalCamUninit, CPTFlagSeparator, "uninit/destroy Flashlight");
    if (mpFlashlightObj) {
        mpFlashlightObj->mHalFlashlightSetFire(0);
        mpFlashlightObj->mHalFlashlightUninit();
        mpFlashlightObj->destroyInstance();
        mpFlashlightObj = NULL;
    }
    //
    CPTLogStr(Event_mHalCamUninit, CPTFlagSeparator, "uninit/destroy CameraIO");
    if (mpCameraIOObj) {
         mpCameraIOObj->uninit();
         mpCameraIOObj->destroyInstance();
         mpCameraIOObj = NULL;
    }
    //
    CPTLogStr(Event_mHalCamUninit, CPTFlagSeparator, "uninit ResMgr");
    mHalSetResMgr(RES_MGR_HAL_MODE_NONE);
    mResMgrHalObj->Uninit();

    return err;
}



/******************************************************************************
*
******************************************************************************/
//!++ Revised: ICS FD
// come from preview start
// currently keep this independent from mHalCamFD
MINT32
mHalCam::
configFDPort(
    ICameraIO::ImgCfg_t* fdCfg,
    ICameraIO::MemInfo_t* fdMemCfg
)
{
    MINT32 err = 0;

    MCAM_DBG("configFDPort");

    MINT32 fdH = mHalCamFD::FD_HEIGHT;
    MINT32 fdW = mHalCamFD::FD_WIDTH;
    MINT32 fdRotation = 0;

    // (1) switch W and H, sync with AP
    if ( mHalCamSensorInfo[mCameraId].devType != MHAL_CAM_SENSOR_DEV_ATV  &&
          mmHalCamParam.frmYuv.w < mmHalCamParam.frmYuv.h )//FIXME
    {
        fdRotation = 1;    //0: 0 degree, 1:90 degree, 2:180 degree, 3:270 degree
        if (fdRotation &0x01) {
            SWAP(fdH, fdW);
        }
    }

    // (2) FD buffer allocator:
    MINT32 fdBuf = fdW * fdH * 2 * FD_BUFFER_CNT; // 2 --> RGB565
    MCAM_DBG("fdBuf = %d", fdBuf);
    #if defined (MTK_M4U_SUPPORT)
    mFDWorkingBuf = new mHalCamMemPool("FDWorkBuf", fdBuf, 0);
    MCAM_ASSERT( mFDWorkingBuf->getVirtAddr() != 0, "[mHalCamPreviewStart] Err,  mFDWorkingBuf) == NULL\n");
    #else
    mFDWorkingBuf = new mHalCamMemPool("FDWorkBuf", fdBuf, 1);
    MCAM_ASSERT( mFDWorkingBuf->getVirtAddr() != 0, " mFDWorkingBuf == NULL\n");
    #endif


    fdCfg->imgWidth = fdW;
    fdCfg->imgHeight = fdH;
    fdCfg->imgFmt = NSCamera::ePIXEL_FORMAT_RGB565;
    fdCfg->rotation = fdRotation;
    //
    fdMemCfg->virtAddr = mFDWorkingBuf->getVirtAddr();
    fdMemCfg->phyAddr = mFDWorkingBuf->getPhyAddr();
    fdMemCfg->bufCnt = FD_BUFFER_CNT;
    fdMemCfg->bufSize =  fdW * fdH * 2;

    return err ;
}

/******************************************************************************
*
*******************************************************************************/
MINT32
mHalCam::
configVideoPort(
    mhalCamParam_t* const pmhalCamParam,
    ICameraIO::ImgCfg_t* pVideoCfg,
    ICameraIO::MemInfo_t* pVideoMemCfg
)
{
    MUINT32 upScaleCoef = 0;          //up scale Coefficient
    MUINT32 downScaleCoef = 0;        //down scale coeffcient

    //
    #if (MHAL_CAM_ISP_TUNING)
    NSCamCustom::TuningParam_CRZ_T crz;
    crz = NSCamCustom::getParam_CRZ_Video();
    upScaleCoef = crz.uUpScaleCoeff;
    downScaleCoef = crz.uDnScaleCoeff;
    #endif
    //
    pVideoCfg->imgFmt =  (NSCamera::EPixelFormat)pmhalCamParam->frmVdo.frmFormat;;
    pVideoCfg->imgWidth = pmhalCamParam->frmVdo.w;
    pVideoCfg->imgHeight = pmhalCamParam->frmVdo.h;
    pVideoCfg->rotation = 0;
    pVideoCfg->flip = 0;
    pVideoCfg->upScaleCoef = upScaleCoef;
    pVideoCfg->downScaleCoef = downScaleCoef;
    //
    pVideoMemCfg->virtAddr = pmhalCamParam->frmVdo.virtAddr;
    pVideoMemCfg->phyAddr = pmhalCamParam->frmVdo.phyAddr;
    pVideoMemCfg->bufCnt = pmhalCamParam->frmVdo.frmCount;
    pVideoMemCfg->bufSize = pmhalCamParam->frmYuv.w * pmhalCamParam->frmYuv.h * 3 / 2;
    return 0;
}

/******************************************************************************
*
*******************************************************************************/
MINT32
mHalCam::
configDispPort(
    mhalCamParam_t* pmhalCamParam,
    ICameraIO::ImgCfg_t* pDispCfg,
    ICameraIO::MemInfo_t* pDispMemCfg)
{
    MUINT32 upScaleCoef = 0;          //up scale Coefficient
    MUINT32 downScaleCoef = 0;        //down scale coeffcient

    //
    #if (MHAL_CAM_ISP_TUNING)
    NSCamCustom::TuningParam_CRZ_T crz;
    crz = NSCamCustom::getParam_CRZ_Preview();
    upScaleCoef = crz.uUpScaleCoeff;
    downScaleCoef = crz.uDnScaleCoeff;
    #endif
    //
    pDispCfg->rotation = 0;
#if 0
    //Marked in MT6575 for MDP limitation
    if (pmhalCamParam->u4CamMode == MHAL_CAM_MODE_VT) {
        // VT Mode, rotate the buffer to 0 degree
        if ((mHalCamSensorInfo[mCameraId].orientation == 90) ||
            (mHalCamSensorInfo[mCameraId].orientation == 270)) {
            SWAP(pmhalCamParam->frmYuv.w, pmhalCamParam->frmYuv.h);
            mmHalCamParam.frmYuv.w = pmhalCamParam->frmYuv.w;
            mmHalCamParam.frmYuv.h = pmhalCamParam->frmYuv.h;
            pDispCfg->rotation = 90;
        }
    }
#endif
    //
    pDispCfg->imgFmt = (NSCamera::EPixelFormat)pmhalCamParam->frmYuv.frmFormat;
    pDispCfg->imgWidth = pmhalCamParam->frmYuv.w;
    pDispCfg->imgHeight = pmhalCamParam->frmYuv.h;
    pDispCfg->flip = 0;
    pDispCfg->upScaleCoef = upScaleCoef;
    pDispCfg->downScaleCoef = downScaleCoef;
    //
    pDispMemCfg->virtAddr = pmhalCamParam->frmYuv.virtAddr;
    pDispMemCfg->phyAddr = pmhalCamParam->frmYuv.phyAddr;
    pDispMemCfg->bufCnt = pmhalCamParam->frmYuv.frmCount;
    pDispMemCfg->bufSize = pmhalCamParam->frmYuv.w * pmhalCamParam->frmYuv.h * 3 / 2;
    return 0;
}

/******************************************************************************
*
*******************************************************************************/

MINT32
mHalCam::
configZSDPort(
    mhalCamParam_t* pmhalCamParam,
    ICameraIO::ImgCfg_t* pZSDCfg,
    ICameraIO::MemInfo_t* pZSDMemCfg,
    MINT32 pZSDPass)
{
    MINT32 err = 0;
    int width, height;

    if (pZSDPass == 0x1){
        #if(1)
        err = mpIspHalObj->sendCommand(ISP_CMD_GET_SENSOR_FULL_RANGE, (int)&width, (int)&height);
        #else
        err = mpIspHalObj->sendCommand(ISP_CMD_GET_SENSOR_PRV_RANGE, (int)&width, (int)&height);
        #endif
        if (err < 0) {
            MCAM_ERR("[configZSDPort] ISP_CMD_GET_RAW_INFO err");
            return err;
        }
        MCAM_DBG("[configZSDPort] w:%d, h:%d \n", width, height);

        mZSDWorkingBuf = new mHalCamMemPool("ZSDBuf(Port1)", width*height*2*ZSD_BUFFER_CNT, 0);
        if (mZSDWorkingBuf == NULL){
            MCAM_DBG("[configZSDPort] No memory for ZSD buffer\n");
            return -1;
        }
        pZSDCfg->imgWidth = width;
        pZSDCfg->imgHeight = height;
        pZSDCfg->imgFmt = NSCamera::ePIXEL_FORMAT_UYVY;  //this is yuyv422 ?
        pZSDCfg->rotation = 0;
        pZSDCfg->flip = 0;

        pZSDMemCfg->virtAddr = mZSDWorkingBuf->getVirtAddr();
        pZSDMemCfg->phyAddr = mZSDWorkingBuf->getPhyAddr();
        pZSDMemCfg->bufCnt = ZSD_BUFFER_CNT;
        pZSDMemCfg->bufSize = width*height*2;

        pmhalCamParam->camZsdParam.u4ZsdAddr = pZSDMemCfg->virtAddr;
        pmhalCamParam->camZsdParam.u4ZsdWidth =pZSDCfg->imgWidth;
        pmhalCamParam->camZsdParam.u4ZsdHeight = pZSDCfg->imgHeight;

        mmHalCamParam.camZsdParam.u4ZsdAddr = pZSDMemCfg->virtAddr;
        mmHalCamParam.camZsdParam.u4ZsdWidth = pZSDCfg->imgWidth;
        mmHalCamParam.camZsdParam.u4ZsdHeight = pZSDCfg->imgHeight;
        mmHalCamParam.camZsdParam.u4ZsdSkipPrev = pmhalCamParam->camZsdParam.u4ZsdSkipPrev;
        mmHalCamParam.camZsdParam.u4ZsdZoomWidth = 0;
        mmHalCamParam.camZsdParam.u4ZsdZoomHeigth = 0;
        mmHalCamParam.camZsdParam.u4ZsdEnable = 1;

        MCAM_DBG("[configZSDPort] pmhalCamParam->u4ZsdAddr:0x%x \n", mmHalCamParam.camZsdParam.u4ZsdAddr);
        MCAM_DBG("[configZSDPort] pmhalCamParam->u4ZsdSkipPrev:%d \n", mmHalCamParam.camZsdParam.u4ZsdSkipPrev);

        return err;

    }
    else{
        #if (1) //zsd preview pipe
        err = mpIspHalObj->sendCommand(ISP_CMD_GET_SENSOR_FULL_RANGE, (int)&width, (int)&height);
        #else
        err = mpIspHalObj->sendCommand(ISP_CMD_GET_SENSOR_PRV_RANGE, (int)&width, (int)&height);
        #endif
        if (err < 0) {
            MCAM_ERR("[configZSDPort.N] ISP_CMD_GET_RAW_INFO err");
            return err;
        }
        MCAM_DBG("[configZSDPort.N] w:%d, h:%d \n", width, height);

        // TODO: Allocate MDP working buffer here, is it OK?
        #define MDP_WORK_BUFFER_CNT (3)
        MUINT32 zsdWorkingBufSize = width * height * 2 * ZSD_BUFFER_CNT;
        MUINT32 mdpWorkingBufSize = pmhalCamParam->frmYuv.w * pmhalCamParam->frmYuv.h * 2 * MDP_WORK_BUFFER_CNT;
        zsdWorkingBufSize = (zsdWorkingBufSize > mdpWorkingBufSize ? zsdWorkingBufSize : mdpWorkingBufSize);

        mZSDWorkingBuf = new mHalCamMemPool("ZSDBuf(Port1)", zsdWorkingBufSize, 0);
        if (mZSDWorkingBuf == NULL){
            MCAM_DBG("[configZSDPort.N] No memory for ZSD buffer\n");
            return -1;
        }
        pZSDCfg->imgWidth = width;
        pZSDCfg->imgHeight = height;
        pZSDCfg->imgFmt = NSCamera::ePIXEL_FORMAT_UYVY;  //this is yuyv422 ?
        pZSDCfg->rotation = 0;
        pZSDCfg->flip = 0;

        pZSDMemCfg->virtAddr = mZSDWorkingBuf->getVirtAddr();
        pZSDMemCfg->phyAddr = mZSDWorkingBuf->getPhyAddr();
        pZSDMemCfg->bufCnt = ZSD_BUFFER_CNT;
        pZSDMemCfg->bufSize = width*height*2;

        pmhalCamParam->camZsdParam.u4ZsdAddr = pZSDMemCfg->virtAddr;
        pmhalCamParam->camZsdParam.u4ZsdWidth =pZSDCfg->imgWidth;
        pmhalCamParam->camZsdParam.u4ZsdHeight = pZSDCfg->imgHeight;

        mmHalCamParam.camZsdParam.u4ZsdAddr = pZSDMemCfg->virtAddr;
        mmHalCamParam.camZsdParam.u4ZsdWidth = pZSDCfg->imgWidth;
        mmHalCamParam.camZsdParam.u4ZsdHeight = pZSDCfg->imgHeight;
        mmHalCamParam.camZsdParam.u4ZsdSkipPrev = pmhalCamParam->camZsdParam.u4ZsdSkipPrev;
        mmHalCamParam.camZsdParam.u4ZsdZoomWidth = 0;
        mmHalCamParam.camZsdParam.u4ZsdZoomHeigth = 0;
        mmHalCamParam.camZsdParam.u4ZsdEnable = 0x2;

        MCAM_DBG("[configZSDPort.N] pmhalCamParam->u4ZsdAddr:0x%x \n", mmHalCamParam.camZsdParam.u4ZsdAddr);
        MCAM_DBG("[configZSDPort.N] pmhalCamParam->u4ZsdSkipPrev:%d \n", mmHalCamParam.camZsdParam.u4ZsdSkipPrev);

        return err;
    }
}
/******************************************************************************
*
******************************************************************************/
#if (MHAL_CAM_VIDEO_SNAPSHOT)
MINT32
mHalCam::
configVSSFDPort(
    ICameraIO::ImgCfg_t* fdCfg,
    ICameraIO::MemInfo_t* fdMemCfg)
{
    MINT32 err = 0;
    //
    MCAM_DBG("configVSSFDPort");
    //
    fdCfg->imgWidth = mmHalCamParam.frmYuv.w;
    fdCfg->imgHeight = mmHalCamParam.frmYuv.h;
    fdCfg->imgFmt = NSCamera::ePIXEL_FORMAT_RGB565;
    fdCfg->rotation = 0;
    //
    fdMemCfg->virtAddr = mpVideoSnapshot->GetAddr(VIDEO_SNAPSHOT_BUF_TYPE_VSS);
    fdMemCfg->phyAddr = 0;
    fdMemCfg->bufCnt = VSS_BUFFER_CNT;
    fdMemCfg->bufSize =  fdCfg->imgWidth * fdCfg->imgHeight * fdMemCfg->bufCnt;
    //
    return err ;
}
#endif
/******************************************************************************
*
*******************************************************************************/
MINT32
mHalCam::
mHalCamIOCB(
    MINT32 msgType,
    MINT32 arg1,
    MINT32 arg2,
    MVOID *user
)
{
    mHalCam *pCamObj = (mHalCam*)user;
    MINT32 err = 0;
    switch (msgType) {
        case ICameraIO::eCAMIO_CB_EIS:
            err = pCamObj->mHalCamEISProc((MINT32*)arg1, (MINT32*)arg2);
            break;
        default:
            break;
    }
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32
mHalCam::
mHalCamEISProc(
    MINT32 *pGmvX,
    MINT32 *pGmvY
)
{
    MINT32 err = 0;
    *pGmvX = 0;
    *pGmvY = 0;
#if (MHAL_CAM_EIS_OPTION)
    mhalCamParam_t* pmHalCamParam = (mhalCamParam_t*)&mmHalCamParam;
    //
    CamTimer camTmr("mHalCamEISProc", MHAL_CAM_PRV_PROFILE_OPT);
    //
    MINT32 gmvX = 0, gmvY = 0;
    //
    err = mpEISHalObj->doEIS(gmvX, gmvY);
    if (err != MHAL_NO_ERROR) {
        MCAM_ERR("[mHalCamEISProc] eisMode(%d), mpEISHalObj->doEIS() err(%x)", pmHalCamParam->cam3AParam.eisMode, err);
    }
    if (pmHalCamParam->cam3AParam.eisMode == MHAL_CAM_FEATURE_ON && pmHalCamParam->u4CamMode == MHAL_CAM_MODE_MVIDEO) {
        //TODO, update gmvX, gmvY to MDP
        *pGmvX = gmvX;
        *pGmvY = gmvY;
    }
    //
    if  (MHAL_NO_ERROR == err)
    {
        mpEISHalObj->getSWGMV(gmvX, gmvY);
        //
        using namespace NSContent;
        //  FIXME: should use another struct and I/F.
        Ctn_EisGmv  contentEisGmv;
        ::memset(&contentEisGmv, 0, sizeof(contentEisGmv));
        contentEisGmv.u4Count = 1;
        contentEisGmv.pi4GmvX = &gmvX;
        contentEisGmv.pi4GmvY = &gmvY;
        IContentManager::getInstance().updateContent(eCtnID_EisGmv, &contentEisGmv);
        //
        MCAM_DBG1("[mHalCamEISProc] GmvX = %d, GmvY = %d\n", gmvX, gmvY);
    }
    //
    camTmr.printTime();
    //
#endif  //MHAL_CAM_EIS_OPTION
    return err;
}

/******************************************************************************
*
*******************************************************************************/
#define SKIP_ZOOM_STEP  2
MINT32
mHalCam::
mHalCamPreviewZoomProc(
)
{
    MINT32 err = 0;
    Mutex::Autolock _l(mLock);
    if (!mZoomQueue.empty()) {
        CamTimer camTmr("mHalCamPreviewZoomProc", MHAL_CAM_PRV_PROFILE_OPT);
        //if the zoom value in queue large than SKIP_ZOOM_STEP, skip SKIP_ZOOM_STEP.
        if (mZoomQueue.size() > SKIP_ZOOM_STEP) {
            for (int i = 0; i < SKIP_ZOOM_STEP; i++) {
                MUINT32 skipZoom = *mZoomQueue.begin();
                mZoomQueue.erase(mZoomQueue.begin());
                MCAM_DBG("[mHalCamPreviewZoomProc] skip zoom:%d\n", skipZoom);
            }
        }

        MUINT32 zoom = *mZoomQueue.begin();
        mZoomQueue.erase(mZoomQueue.begin());
        MCAM_DBG("[mHalCamPreviewZoomProc] zoomVal = %d\n" ,zoom);
        //
        err = mHalCamSetPreviewZoom(zoom);
        if (err != MHAL_NO_ERROR) {
            MCAM_WARN("[mHalCamPreviewZoomProc] mHalCamPreviewZoom err:0x%x\n", err);
        }
        mHalCamCBHandle(MHAL_CAM_CB_ZOOM, &(mmHalCamParam.camZsdParam));
        camTmr.printTime();
    }
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32
mHalCam::
mHalCamFreeZSDMem(void)
{
    if (mZSDWorkingBuf != NULL){
        MCAM_DBG("[mHalCamFreeZSDMem], release buff %p \n",mZSDWorkingBuf);
        delete mZSDWorkingBuf;
        mZSDWorkingBuf = NULL;
    }
    else{
        MCAM_DBG("[mHalCamFreeZSDMem], non buffer released \n");
    }

    return 0;
}
/******************************************************************************
*
*******************************************************************************/
MINT32
mHalCam::
mHalCamZsdWaitBuff(MUINT32 onFlash)
{
    ICameraIO::BuffInfo_t rDispBufInfo;
    ICameraIO::BuffInfo_t rVdoBufInfo;
    mhalCamTimeStampBufCBInfo rBufCBInfo(0);

    //always wait a new buffer for ALPS00235420
    //if (onFlash == 1)
    if (1)
    {
        mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_VIDEO_PORT, &rVdoBufInfo);
        mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_VIDEO_PORT, &rVdoBufInfo);
        mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_VIDEO_PORT, &rVdoBufInfo);

        mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
        MCAM_DBG("[mHalCamZsdWaitBuff][ZSD]Last DISP hwIndex: %d, fillCnt: %d, bufAddr = 0x%x,\n", rDispBufInfo.hwIndex, rDispBufInfo.fillCnt, rDispBufInfo.bufAddr);
        if (rDispBufInfo.fillCnt != 1){
            rBufCBInfo.u4BufIndex= rDispBufInfo.hwIndex + (rDispBufInfo.fillCnt-1);
            if (rBufCBInfo.u4BufIndex >= PREVIEW_BUFFER_CNT){
                rBufCBInfo.u4BufIndex -= PREVIEW_BUFFER_CNT;
            }
            for (int i=0;i< (int)rDispBufInfo.fillCnt;i++){
                mpCameraIOObj->releasePreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
                mpCameraIOObj->getPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
            }
            MCAM_DBG("[mHalCamZsdWaitBuff][ZSD]Skip buff to idx: %d \n", rBufCBInfo.u4BufIndex);
            rBufCBInfo.u4TimStampS = rDispBufInfo.timeStampS;
            rBufCBInfo.u4TimStampUs= rDispBufInfo.timeStampUs;
        }
        else{
            rBufCBInfo.u4BufIndex = rDispBufInfo.hwIndex;
            rBufCBInfo.u4TimStampS = rDispBufInfo.timeStampS;
            rBufCBInfo.u4TimStampUs= rDispBufInfo.timeStampUs;
        }
        mHalCamCBHandle(MHAL_CAM_CB_PREVIEW, &rBufCBInfo);
    }
    if (mmHalCamParam.camZsdParam.u4ZsdDump == 1){
        ::mHalMiscDumpToFile((char*) "//sdcard//zsd_last_prv.bin", (UINT8*)mmHalCamParam.frmYuv.virtAddr+rBufCBInfo.u4BufIndex*mmHalCamParam.frmYuv.frmSize, mmHalCamParam.frmYuv.frmSize);
    }

    return 0;
}

/******************************************************************************
*
*******************************************************************************/
namespace
{
    //  Only one hardware.
    mHalCamBase*volatile    g_mHalCamSingleton = NULL;
    android::Mutex          g_mtxSingletonLock;
}

/******************************************************************************
*
*******************************************************************************/
mHalCamBase*
mHalCam::
getInstance()
{
    AutoCPTLog cptlog(Event_mHalCamCreate);
    MCAM_DBG("[getInstance] E \n");
    //
    android::Mutex::Autolock _l(g_mtxSingletonLock);
    //
    if  ( NULL != g_mHalCamSingleton )
    {
        MCAM_ERR("[mHalCam] does not support multi-open; singleton instance:%p", g_mHalCamSingleton);
        return  NULL;
    }
    //
    g_mHalCamSingleton = new mHalCam();
    return g_mHalCamSingleton;
}

/******************************************************************************
*
*******************************************************************************/
void
mHalCam::
destroyInstance()
{
    MCAM_DBG("[destroyInstance] E \n");
    //
    android::Mutex::Autolock _l(g_mtxSingletonLock);
    //
    if  ( this == g_mHalCamSingleton )
    {
        g_mHalCamSingleton = NULL;
    }
    else
    {
        MCAM_DBG("[mHalCam] is destroying current instance(%p) != singleton instance(%p)", this, g_mHalCamSingleton);
    }
    //
    delete this;
}

