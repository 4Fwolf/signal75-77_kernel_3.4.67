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

#define LOG_TAG "scenario/mHalCamN3D"
#include <utils/Errors.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <utils/threads.h>
//
#include <mhal/inc/MediaHal.h>
#include <mhal/inc/camera.h>
#include <scenario_types.h>
#include <cam_types.h>
#include "mhal_misc.h"
//
#include <mcam_log.h>
#include <mcam_profile.h>
#include "mhal_cam_n3d.h"
#include "kd_camera_feature.h"
#include "camera_custom_if.h"
//
#include "cam_port.h"
#include "ICameraIO.h"
//
#include "shot/IShot.h"
#include "shot/Shotbase.h"
#include "shot/NormalShot.h"
#include "CameraProfile.h"
//
#include "mhal_interface.h"             //For Mt6575_mHalBitblt
//
#include "native3d/N3D_NormalShot.h"

//
//#include <asm/arch/mt6573_sync_write.h>
/*******************************************************************************
*
********************************************************************************/
#define SWAP(a, b) {int c = (a); (a) = (b); (b) = c; }

// Capture
#define MHAL_CAM_RAW_BUF_SIZE       (0x1000000)
#define MHAL_CAM_THUMB_ADDR_OFFSET  (64 * 1024)
#define MHAL_CAM_JPEG_ADDR_OFFSET   (128 * 1024)
#define MHAL_CAM_HW_JPEG_HEADER_OFFSET   (1 * 1024)

//


//Buffer Cnt 
#define PREVIEW_BUFFER_CNT              8
#define VIDEO_BUFFER_CNT                8 

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
//
static mHalCamN3D* pmHalCamObj = NULL;
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
#ifdef  MTK_NATIVE_3D_SUPPORT
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
    case ISP_SENSOR_DEV_MAIN | ISP_SENSOR_DEV_MAIN_2:
        eDeviceId = NSCamera::eDevId_ImgSensor0; // back camera
        break;
    default:
        eDeviceId = NSCamera::eDevId_Unknown;
        break;
    }
#endif
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
MVOID mHalCamN3D::mHalSetResMgr(MUINT32 const mode)
{
    //
    RES_MGR_HAL_MODE_STRUCT    rModeInfo;
    rModeInfo.Mode = (RES_MGR_HAL_MODE_ENUM)mode;
    rModeInfo.ModeSub = RES_MGR_HAL_MODE_SUB_NONE;
    //
    rModeInfo.Dev = RES_MGR_HAL_DEV_CAM;

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
UINT32 mHalMiscDumpToFileN3D(
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
mHalCamN3D::mHalCamGetBufMemInfo(MVOID *a_pOutBuffer) 
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
            MCAM_ERR("[mHalCamGetBufMemInfo] do not support MHAL_CAM_BUF_PREVIEW_ATV");
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
mHalCamN3D::mHalCamN3D()
    : mHalCamBase()
    , meFeatureCamRole(NSFeature::ECamRole_Main) //xxxx NSFeature::ECamRole_N3D_Main
    , mpHal3AObj(NULL)
    , mpIspHalObj(NULL)
    , mpCameraIOObj(NULL) 
    , mpFlashlightObj(NULL)
    , mIsVideoRecording(MFALSE)
    , mCameraState(MHAL_CAM_N3D_IDLE)
    , mAaaWaitVDNum(0)
    , mAaaState(MHAL_CAM_N3D_IDLE)
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
    
{  
    mAFBestPos = 0;
    //
    meSensorType = ISP_SENSOR_TYPE_RAW;
    ::memset(&mmHalCamParam, 0 , sizeof(mhalCamParam_t));
    ::memset(&mmHalRawImageInfo, 0, sizeof(mhalCamRawImageInfo_t));
    pmHalCamObj = this;
    mResMgrHalObj = ResMgrHal::CreateInstance();

    mPreAfStatus = AF_MARK_NONE;
    mCurAfStatus = AF_MARK_NONE;
}

/*******************************************************************************
*
********************************************************************************/
mHalCamN3D::~mHalCamN3D()
{
    pmHalCamObj = NULL;

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
mHalCamN3D::mHalCamSetState(
    mHalCamN3dState_e newState
)
{
    Mutex::Autolock lock(mLock);

    MCAM_DBG("[mHalCamSetState] Now: 0x%04x, Next: 0x%04x \n", mCameraState, newState);
    //
    if (newState == MHAL_CAM_N3D_ERROR) {
        goto MHAL_CAM_SET_STATE_EXIT;
    }
    //
    switch (mCameraState) {
    case MHAL_CAM_N3D_INIT:
        switch (newState) {
        case MHAL_CAM_N3D_IDLE:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_N3D_INIT \n");
            break;
        }
        break;
    //
    case MHAL_CAM_N3D_IDLE:
        switch (newState) {
        case MHAL_CAM_N3D_IDLE:
        case MHAL_CAM_N3D_PREVIEW:
        case MHAL_CAM_N3D_CAPTURE:
        case MHAL_CAM_N3D_UNINIT:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_N3D_IDLE \n");
            break;
        }
        break;
    //
    case MHAL_CAM_N3D_PREVIEW:
        switch (newState) {
        case MHAL_CAM_N3D_PREVIEW:
        case MHAL_CAM_N3D_FOCUS:
        case MHAL_CAM_N3D_STOP:
        case MHAL_CAM_N3D_PRE_CAPTURE:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_N3D_PREVIEW \n");
            break;
        }
        break;
    //
    case MHAL_CAM_N3D_FOCUS:
        switch (newState) {
        case MHAL_CAM_N3D_PREVIEW:
        case MHAL_CAM_N3D_PRE_CAPTURE:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_N3D_FOCUS \n");
            break;
        }
        break;
    //
    case MHAL_CAM_N3D_PRE_CAPTURE:
        switch (newState) {
        case MHAL_CAM_N3D_CAPTURE:
        case MHAL_CAM_N3D_STOP:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_N3D_PRE_CAPTURE \n");
            break;
        }
        break;
    //
    case MHAL_CAM_N3D_CAPTURE:
        switch (newState) {
        case MHAL_CAM_N3D_IDLE:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_N3D_CAPTURE \n");
            break;
        }
        break;
    //
    case MHAL_CAM_N3D_STOP:
        switch (newState) {
        case MHAL_CAM_N3D_IDLE:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_N3D_STOP \n");
            break;
        }
        break;
    //
    case MHAL_CAM_N3D_ERROR:
        switch (newState) {
        case MHAL_CAM_N3D_STOP:
        case MHAL_CAM_N3D_UNINIT:
            break;
        default:
            MCAM_ASSERT(0, "State error MHAL_CAM_N3D_STOP \n");
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
mHalCamN3dState_e
mHalCamN3D::mHalCamGetState(
)
{
    Mutex::Autolock _l(mLock);     
    return mCameraState;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
mHalCamN3D::mHalCamWaitState(
    mHalCamN3dState_e waitState
)
{
    volatile mHalCamN3dState_e state;
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
mHalCamN3D::
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
mHalCamN3D::mHalCamSetIspMode(
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
mHalCamN3D::mHalCamStop(
)
{
    MINT32 err = MHAL_NO_ERROR;

    CamTimer camTmr("mHalCamStop", MHAL_CAM_GENERIC_PROFILE_OPT);
    //
    MCAM_DBG("[mHalCamStop] E");
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

    MCAM_DBG("[mHalCamStop] X");

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCamN3D::mHalCamWaitPreviewStable(
)
{
    AutoCPTLog cptlog(Event_mHalCamWaitPreviewStable);
    MINT32 err = MHAL_NO_ERROR;
    MUINT32 delayFrame = 0;
    MUINT32 cnt = 0;
    MCAM_DBG("[mHalCamWaitPreviewStable] \n");

#ifdef  MTK_NATIVE_3D_SUPPORT
    //! Wait for statistic data valid
    //! For raw sensor, the 3A statistic need 2 frame 
    //! for stable. 
    MCAM_DBG("[mHalCamWaitPreviewStable] delay 2 frames for 3A Statistic Stable\n");
    while (cnt < 2 && (mHalCamGetState() & MHAL_CAM_N3D_PREVIEW_MASK)) {
        err = mpCameraIOObj->dropN3DPreviewFrame(); 
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
    while (cnt < delayFrame  && (mHalCamGetState() & MHAL_CAM_N3D_PREVIEW_MASK)) {
        #if (MHAL_CAM_3A_OPTION)
        err = mHalCam3AProc();
        //MCAM_ASSERT(err >= 0, "mHalCam3AProc err \n");
        #endif
        err = mpCameraIOObj->dropN3DPreviewFrame(); 
        if (err < 0) {
            MCAM_ERR("[mHalCamWaitPreviewStable] dropFrame() fail \n"); 
            return err; 
        }    
        cnt++; 
    }
#endif
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCamN3D::mHalCamPreviewProc(
)
{
    AutoCPTLog cptlog(Event_mHalCamPreviewProc);

    MINT32 err = MHAL_NO_ERROR;
    mFrameCnt = 0;
    //FIXME, may cause race condition w/ other thread 
    mhalCamParam_t *pmhalCamParam = &mmHalCamParam;
    ICameraIO::BuffInfo_t rDispBufInfo;
    DebugFPS debugFps("preview"); 
    mhalCamTimeStampBufCBInfo mhalCamDispBufCb(0); 

    MCAM_DBG("[mHalCamPreviewProc] E \n");    
    //
    CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "wait preview stable");
    if (pmhalCamParam->camZsdParam.u4ZsdSkipPrev != 1) {   
        mHalCamWaitPreviewStable();
    }
#ifdef  MTK_NATIVE_3D_SUPPORT
    //
    MUINT32 debugTimeLog = 0;
    char value[PROPERTY_VALUE_MAX] = {'\0'}; 
    property_get("debug.camera.n3d.time", value, "0");
    debugTimeLog = atoi(value);
    //
    MINT32 startTime = 0;
    MINT32 endTime = 0;
    struct timeval tv;
    if (debugTimeLog) {
        gettimeofday(&tv, NULL);
        startTime = tv.tv_sec * 1000000 + tv.tv_usec;
    }
    //
    while (mHalCamGetState() & MHAL_CAM_N3D_PREVIEW_MASK) {
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "preview while in");
        CamTimer camTmr("mHalCamPreviewProc", MHAL_CAM_PRV_PROFILE_OPT);
        // 2. De-queue display buffer
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "De-queue display buffer"); 
        //
        if (debugTimeLog) {
            gettimeofday(&tv, NULL);
            endTime = tv.tv_sec * 1000000 + tv.tv_usec;
            MCAM_DBG("[debugN3DTime] getN3DPreviewFrame Start  ========> %d us \n", (endTime - startTime));
        }
        //
        err = mpCameraIOObj->getN3DPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
        //
        if (debugTimeLog) {
            gettimeofday(&tv, NULL);
            startTime = tv.tv_sec * 1000000 + tv.tv_usec;
        }
        //
        CPTLog(Event_getPreviewFrame, CPTFlagPulse);
        camTmr.printTime("getN3DPreviewFrame (disp)"); 
        MCAM_DBG1("[mHalCamPreviewProc]  DISP hwIndex: %d, fillCnt: %d, timeS = %d, timeUs = %d\n",
                               rDispBufInfo.hwIndex, rDispBufInfo.fillCnt, rDispBufInfo.timeStampS, rDispBufInfo.timeStampUs);
        
        // 3. Update to display buffer No and callback to SF 
        mhalCamDispBufCb.u4BufIndex = rDispBufInfo.hwIndex; 
        mhalCamDispBufCb.u4TimStampS = rDispBufInfo.timeStampS;
        mhalCamDispBufCb.u4TimStampUs= rDispBufInfo.timeStampUs;
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "callback dispbuf to SF");  
        mHalCamCBHandle(MHAL_CAM_CB_PREVIEW, &mhalCamDispBufCb); 
        camTmr.printTime("MHAL_CAM_CB_PREVIEW");                         
        debugFps.checkFPS(30);
        
        // 4. Video Process
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "Video Process");
        {
            if(MTRUE ==  mIsVideoRecording ) {
                VideoFrame rVdoFrame; 
                rVdoFrame.hwIndex = rDispBufInfo.hwIndex;
                rVdoFrame.fillCnt = rDispBufInfo.fillCnt; 
                rVdoFrame.bufAddr = rDispBufInfo.bufAddr; 
                rVdoFrame.timeStampS = rDispBufInfo.timeStampS; 
                rVdoFrame.timeStampUs = rDispBufInfo.timeStampUs;
                if (mVdoBusyFrmCnt < PREVIEW_BUFFER_CNT) {
                    postVideoFrame(rVdoFrame); 
                    mHalCamVideoProc();
                    // after post the video buffer, check if the busy count is full, 
                    // only release video buffer when the buffer is not full 
                    while (mVdoBusyFrmCnt == PREVIEW_BUFFER_CNT && MTRUE ==  mIsVideoRecording ) {                  
                        MCAM_WARN("[mHalCamPreviewProc] OMX is busy, buffer is drop, busy cnt:%d\n", mVdoBusyFrmCnt); 
                        usleep(33000); 
                    }
                    err = mpCameraIOObj->releaseN3DPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo); 
                }
            }
            else {
                err = mpCameraIOObj->releaseN3DPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo); 
            }
        }

        // 5. 3A Process 
        //    do the AWB, AE, AF proceess at each frame in 
        #if (MHAL_CAM_3A_OPTION)
        {
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "3A process");
        err = mHalCam3AProc();
        }
        #endif

        // 10. Notify the frame is coming 
        mFrameCnt++; 
        MCAM_DBG("[mHalCamPreviewProc] Brocast the preview frame in frameCnt:%d E \n", mFrameCnt); 

        if (mCamDumpOPT) {
            if (mFrameCnt == PREVIEW_DUMP_NO) {
                CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "dump process");
                ::mHalMiscDumpToFileN3D((char*) "//sdcard/prv.raw", (UINT8*)pmhalCamParam->frmYuv.virtAddr, pmhalCamParam->frmYuv.frmSize); 
            }
        }                       
        camTmr.printTime("mHalCamPreviewProc");
    }
    MCAM_DBG("[mHalCamPreviewProc]  exit while \n");
    //
    CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "3A setFlashActive");
    mpHal3AObj->setFlashActive(MFALSE);
    while (mHalCamGetState() == MHAL_CAM_N3D_PRE_CAPTURE) {
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "preCapture while in");
        #if (MHAL_CAM_3A_OPTION)
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "getN3DPreviewFrame");
        err = mpCameraIOObj->getN3DPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo);
        MCAM_ASSERT(err >= 0, "mpCameraIOObj->getN3DPreviewFrame err\n");
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "mHalCam3AProc");
        err = mHalCam3AProc();
        MCAM_ASSERT(err >= 0, "mHalCam3AProc err \n");
        MCAM_DBG("[mHalCamPreviewProc]getReadyForCapture = %d, getFlashActive = %d",mpHal3AObj->getReadyForCapture(),mpHal3AObj->getFlashActive());
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
            mpHal3AObj->getISO(1, &iso);
            CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "set ISO to ISP E");
            err = mpIspHalObj->sendCommand(ISP_CMD_SET_ISO, iso);
            if (err < 0) {
                MCAM_ERR("[mHalCamPreviewProc]  ISP_CMD_SET_ISO err \n\n");
            }
            mpCameraIOObj->releaseN3DPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo); 
            mHalCamSetState(MHAL_CAM_N3D_STOP);
            break;
        }
        CPTLogStr(Event_mHalCamPreviewProc_detail, CPTFlagPulse, "releaseN3DPreviewFrame");
        err = mpCameraIOObj->releaseN3DPreviewFrame(ICameraIO::ePREVIEW_DISP_PORT, &rDispBufInfo); 
        #endif
    }
#endif
    MCAM_DBG("[mHalCamPreviewProc] End \n");

    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCamN3D::mHalCamCaptureProc(
)
{
    AutoCPTLog cptlog(Event_mHalCamCaptureProc);
    MINT32 err = MHAL_NO_ERROR;

    MCAM_DBG("[mHalCamCaptureProc] \n");

    mpHal3AObj->strobeDelay();  //cotta--added for strobe protection

    if  ( ! mpShot || ! mpShot->capture() )
    {
        MCAM_ERR("[mHalCamCaptureProc]<ShotMode:%d> mpShot(%p)->capture()", mmHalCamParam.u4ShotMode, mpShot);
        err = -1;
    }

    mHalCamSetState(MHAL_CAM_N3D_IDLE);
    //
    mHalCamCBHandle(MHAL_CAM_CB_CAPTURE_DONE, NULL);
    //
    MCAM_DBG("[mHalCamCaptureProc] End \n");

    return err;
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

    mHalCamN3dState_e eState;

    MCAM_DBG("[mHalCamMainHighThread] tid: %d \n", gettid());

    eState = pmHalCamObj->mHalCamGetState();
    while (eState != MHAL_CAM_N3D_UNINIT) {
        ::sem_wait(&semMainHigh);
        MCAM_DBG("Got semMainHigh \n");
        eState = pmHalCamObj->mHalCamGetState();
        switch (eState) {
        case MHAL_CAM_N3D_PREVIEW:
            pmHalCamObj->mHalCamPreviewProc();
            ::sem_post(&semMainHighBack);
            break;
        case MHAL_CAM_N3D_CAPTURE:
            pmHalCamObj->mHalCamCaptureProc();
            break;
        case MHAL_CAM_N3D_UNINIT:
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

MVOID mHalCamN3D::mHalCamShowParam(mhalCamParam_t *pmhalCamParam)
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
    MCAM_DBG("yuv: %d, %d, 0x%x, rgb: %d, %d \n",
        pmhalCamParam->frmYuv.w, pmhalCamParam->frmYuv.h, (NSCamera::EPixelFormat)pmhalCamParam->frmYuv.frmFormat,
        pmhalCamParam->frmRgb.w, pmhalCamParam->frmRgb.h);
    MCAM_DBG("qv: %d, %d, 0x%x, jpg: %d, %d, 0x%x \n",
        pmhalCamParam->frmQv.w, pmhalCamParam->frmQv.h, (NSCamera::EPixelFormat)pmhalCamParam->frmQv.frmFormat,
        pmhalCamParam->frmJpg.w, pmhalCamParam->frmJpg.h, (NSCamera::EPixelFormat)pmhalCamParam->frmJpg.frmFormat);
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
MINT32
mHalCamN3D::mHalCamPreviewStart(
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize
)
{
    AutoCPTLog cptlog(Event_mHalCamPreviewStart);
    MCAM_DBG("[mHalCamPreviewStart] E \n");    
    MCAM_ASSERT(mHalCamGetState() == MHAL_CAM_N3D_IDLE, "Camera State not IDLE \n");
    //    
    MINT32 err = MHAL_NO_ERROR;
    mhalCamParam_t *pmhalCamParam = (mhalCamParam_t *) a_pInBuffer;
    ICameraIO::ImgCfg_t rInCfg; 
    ICameraIO::ImgCfg_t *pDispCfg = NULL, *pVdoCfg = NULL; 
    ICameraIO::MemInfo_t *pDispMemInfo = NULL, *pVdoMemInfo = NULL; 
    
    MUINT32 previewPort = 0; 
    //
    CamTimer camTmr("mHalCamPreviewStart", MHAL_CAM_GENERIC_PROFILE_OPT);    
    //
    mHalCamShowParam(pmhalCamParam);
    //
    //
    ::memcpy(&mmHalCamParam, pmhalCamParam, sizeof(mhalCamParam_t));    
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "mHalSetResMgr");
    mHalSetResMgr(RES_MGR_HAL_MODE_PREVIEW_ON);
    // 
    rInCfg.imgWidth = pmhalCamParam->frmYuv.w; 
    rInCfg.imgHeight = pmhalCamParam->frmYuv.h;
    rInCfg.rotation = 0;
    
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
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "setPreviewPipe"); 
    ICameraIO::PreviewPipeCfg_t* pPreviewPipeCfg  = new ICameraIO::PreviewPipeCfg_t(); 
    pPreviewPipeCfg->pInputConfig = &rInCfg; 
    pPreviewPipeCfg->pDispConfig = pDispCfg; 
    pPreviewPipeCfg->pVideoConfig = pVdoCfg; 
    //
    err = mpCameraIOObj->setSensorMode(ICameraIO::eSENSOR_PREVIEW_MODE); 
    //
#ifdef  MTK_NATIVE_3D_SUPPORT
    if ( pmhalCamParam->u4CamMode == MHAL_CAM_MODE_MPREVIEW ) {
        err = mpCameraIOObj->setCameraMode(ICameraIO::eCAMERA_MODE_PREVIEW);
    }
    else if ( pmhalCamParam->u4CamMode == MHAL_CAM_MODE_MVIDEO ) {
        err = mpCameraIOObj->setCameraMode(ICameraIO::eCAMERA_MODE_VIDEO);
    }
    else {
        MCAM_WARN("[mHalCamPreviewStart] u4CamMode(%d)", pmhalCamParam->u4CamMode);
    }
    //
    err = mpCameraIOObj->setN3DPreviewPipe(
                                    previewPort,         //enable port 
                                    NULL,                //image is from mem ?
                                    pPreviewPipeCfg,     //preview pipe config 
                                    pDispMemInfo,        //display out 
                                    pVdoMemInfo          //video out 
                                    ); 
    delete pPreviewPipeCfg; 
#endif
    //
    // Preview is always normal mode, isp tuning pipe config 
    #if (MHAL_CAM_ISP_TUNING)
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "config IspTuning pipe");
    if ( pmhalCamParam->u4CamMode == MHAL_CAM_MODE_MVIDEO ) {
        err = mHalCamSetIspMode(ISP_CAM_MODE_VIDEO, (int) pmhalCamParam->u4CamIspMode);
    }
    else {
        err = mHalCamSetIspMode(ISP_CAM_MODE_PREVIEW, (int) pmhalCamParam->u4CamIspMode);
    }
    if (err < 0) {
        MCAM_ERR("[mHalCamPreviewStart] mHalCamSetIspMode err (0x%x)\n", err); 
        goto mHalCamPreviewStart_EXIT;
    }    
    #endif 
    
    //
    // FIXME 
    //! Should before Cam3AInit() due to 
    //! FD proc will be invoke in 3AInit(); 
    //! (won't happen in ICS)
    mHalCamSetState(MHAL_CAM_N3D_PREVIEW);
    //
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "mHalCam3AInit");
    err = mHalCam3AInit(pmhalCamParam->u4CamMode, pmhalCamParam->cam3AParam);
    if (err < 0) {
        MCAM_ERR("[mHalCamPreviewStart] mHalCam3AInit() fail\n");
        goto mHalCamPreviewStart_EXIT;
    }
    // AF setting 
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "AF setting");
    if (pmhalCamParam->cam3AParam.afEngMode != 0) {
        // Disable AE/AWB
        mHalCam3ACtrl(0, 0, 1);
        mpHal3AObj->setMFPos(mAFBestPos + pmhalCamParam->cam3AParam.afEngPos);
    }    

    // start CameraIO
    CPTLogStr(Event_mHalCamPreviewStart, CPTFlagSeparator, "start CameraIO");
    err = mpCameraIOObj->start(); 
    if (err < 0) {
        MCAM_ERR ("[mHalCamPreviewStart] mpCameraIOObj->start() err (0x%x)\n", err );   
        goto mHalCamPreviewStart_EXIT;
    }
    //
    //
    g_mhalObserver = pmhalCamParam->mhalObserver;
    ::sem_post(&semMainHigh);
    
mHalCamPreviewStart_EXIT:
    //
    if (err < 0) {
        // free the memory alread allocated 
        mpCameraIOObj->stop(); 
        
        mHalCamSetState(MHAL_CAM_N3D_ERROR);
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
    camTmr.printTime(); 
    MCAM_DBG("[mHalCamPreviewStart] X \n");   

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCamN3D::mHalCamPreviewStop(
)
{
    AutoCPTLog cptlog(Event_mHalCamPreviewStop);
    MINT32 err = MHAL_NO_ERROR;
    MCAM_DBG("[mHalCamPreviewStop] Start \n");
    CamTimer camTmr("mHalCamPreviewStop", MHAL_CAM_GENERIC_PROFILE_OPT);    
    
    mhalCamParam_t *pmhalCamParam = &mmHalCamParam;
    mHalCamN3dState_e state = mHalCamGetState();
    //
    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator, "set state");
    if (state == MHAL_CAM_N3D_IDLE) {
        return err;
    }
    else if (state & MHAL_CAM_N3D_PREVIEW_MASK) {
        if (state == MHAL_CAM_N3D_FOCUS) {
            // It is in focus state, back to preview state first
            mHalCamSetState(MHAL_CAM_N3D_PREVIEW);
        }
        mHalCamSetState(MHAL_CAM_N3D_STOP);
    }
    else if (state == MHAL_CAM_N3D_PRE_CAPTURE) {
        // In preview capture, has to wait 3A ready flag
        MCAM_DBG("[mHalCamPreviewStop] it is MHAL_CAM_N3D_PRE_CAPTURE state \n");
    }
    else if (state == MHAL_CAM_N3D_CAPTURE) {
        // It is in capture flow now, preview has been already stopped
        MCAM_DBG("[mHalCamPreviewStop] it is MHAL_CAM_N3D_CAPTURE state \n");
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
    
    err = mHalCamStop();
    MCAM_ASSERT(err >= 0, "mHalCamStop err \n");
    //
    mpCameraIOObj->setCallbacks(NULL, NULL);
    //
    mHalCamSetState(MHAL_CAM_N3D_IDLE);
    //
    mAaaState = MHAL_CAM_N3D_IDLE;
    //
    CPTLogStr(Event_mHalCamPreviewStop, CPTFlagSeparator , "mHalSetResMgr");
    mHalSetResMgr(RES_MGR_HAL_MODE_PREVIEW_OFF);

    // Unlock Sysram resource
    //
    MCAM_DBG("[mHalCamPreviewStop] End \n");

    camTmr.printTime(); 
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCamN3D::mHalCamPreCapture(
)
{
    MINT32 err = MHAL_NO_ERROR;

    MCAM_DBG("[mHalCamPreCapture] \n");

    mHalCamSetState(MHAL_CAM_N3D_PRE_CAPTURE);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCamN3D::mHalCamCaptureInit(
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize
)
{
    AutoCPTLog cptlog(Event_mHalCamCaptureInit);

    MINT32 err = MHAL_NO_ERROR;
    mhalCamParam_t *pmhalCamParam = (mhalCamParam_t *) a_pInBuffer;

    //
    MCAM_DBG("[mHalCamCaptureInit] E \n");
    //
    mHalCamShowParam(pmhalCamParam);
    MCAM_ASSERT(mHalCamGetState() == MHAL_CAM_N3D_IDLE, "Camera State not IDLE \n");
    //MCAM_ASSERT((pmhalCamParam->frmJpg.bufSize >= MHAL_CAM_RAW_BUF_SIZE), "JPEG buffer size is too small for temporary raw buffer \n");
    //
    ::memcpy(&mmHalCamParam, pmhalCamParam, sizeof(mhalCamParam_t));
    //
    //
    mHalSetResMgr(RES_MGR_HAL_MODE_CAPTURE_ON);
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
    case MHAL_CAM_CAP_MODE_NORMAL:
    default:
        {

            N3D_NormalShot* pShot = N3D_NormalShot::createInstance(eShotSensorType, eShotDeviceId);
            if  ( ! pShot || ! pShot->init(this, mmHalCamParam, mpHal3AObj, mpCameraIOObj, N3D_FILE_TYPE_JPS) )
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
         mHalCamSetState(MHAL_CAM_N3D_ERROR);
    }
    MCAM_DBG("[mHalCamCaptureInit] X \n");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCamN3D::mHalCamCaptureUninit(
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
mHalCamN3D::mHalCamCaptureStart(
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

    if(1 == mmHalCamParam.u4Scalado || 0 != mmHalCamParam.u4Rotate)
    {
        mmHalCamParam.u4DumpYuvData = 1;
    }

    if  ( mpShot )
    {
        mpShot->setParam(mmHalCamParam);
    }
    //
    mHalCamSetState(MHAL_CAM_N3D_CAPTURE);
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
mHalCamN3D::mHalCamSendCommand(
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
        break;
    //
    case CAM_CMD_GET_RAW_IMAGE_INFO:
        // mHalCamAdapter:  MHAL_CAM_CB_JPEG
        ret = mHalCamGetCaptureInfo((mhalCamRawImageInfo_t *) arg1, arg2);
        break;
    //
    case CAM_CMD_GET_BS_INFO:
        break;
    //
    case CAM_CMD_SET_FLASHLIGHT_PARAMETER:
        ret = mHalCamSetFlashlightParameter((MINT32 *) arg1);
        break;
    //
    case CAM_CMD_SET_SMILE_PREVIEW:
        MCAM_ERR("[mHalCamSendCommand]  CAM_CMD_SET_SMILE_PREVIEW -- do nothing");
        break;
    //
    case CAM_CMD_SET_FACE_PREVIEW:
        MCAM_ERR("[mHalCamSendCommand]  CAM_CMD_SET_FACE_PREVIEW -- do nothing");
        break;
    //
    case CAM_CMD_DO_PANORAMA:
        MCAM_ERR("[mHalCamSendCommand]  CAM_CMD_DO_PANORAMA -- do nothing");
        break;
    //
    case CAM_CMD_CANCEL_PANORAMA:
        MCAM_ERR("[mHalCamSendCommand]  CAM_CMD_CANCEL_PANORAMA -- do nothing");
        break;
    //
    case CAM_CMD_START_MAV:
        MCAM_ERR("[mHalCamSendCommand]  CAM_CMD_START_MAV -- do nothing");
        break;
    //
    case CAM_CMD_STOP_MAV:
        MCAM_ERR("[mHalCamSendCommand]  CAM_CMD_STOP_MAV -- do nothing");
        break;
    //
    case CAM_CMD_START_AUTORAMA:
        MCAM_ERR("[mHalCamSendCommand]  CAM_CMD_START_AUTORAMA -- do nothing");
        break;   
    //
    case CAM_CMD_STOP_AUTORAMA:
        MCAM_ERR("[mHalCamSendCommand]  CAM_CMD_STOP_AUTORAMA -- do nothing");
        break;    
    //
    case CAM_CMD_START_3DSHOT: // between 1st pic and 2nd pic
        MCAM_ERR("[mHalCamSendCommand]  CAM_CMD_START_3DSHOT -- do nothing");
        break;  
    //
    case CAM_CMD_STOP_3DSHOT:
        MCAM_ERR("[mHalCamSendCommand]  CAM_CMD_STOP_3DSHOT -- do nothing");
        break; 
    //  
    case CAM_CMD_SET_ATV_DISP:
        MCAM_ERR("[mHalCamSendCommand]  CAM_CMD_SET_ATV_DISP -- do not support ATV");
        break;
    //
    case CAM_CMD_GET_ATV_DISP_DELAY:
        MCAM_ERR("[mHalCamSendCommand]  CAM_CMD_GET_ATV_DISP_DELAY -- do not support ATV");
        break;
    //
    case CAM_CMD_GET_BUF_SUPPORT_FMT:
        {
            MCAM_DBG("[mHalCamSendCommand]  CAM_CMD_GET_BUF_SUPPORT_FMT");
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
        }
        break; 
    case CAM_CMD_GET_3A_SUPPORT_FEATURE: 
        {
            MCAM_DBG("[mHalCamSendCommand]  CAM_CMD_GET_3A_SUPPORT_FEATURE");
            mHalCam3ASupportFeature_t *p3AFeature = (mHalCam3ASupportFeature_t*)arg1; 
            p3AFeature->u4AutoWhiteBalanceLock = (MUINT32)mpHal3AObj->isAutoWhiteBalanceLockSupported(); 
            p3AFeature->u4ExposureLock = (MUINT32) mpHal3AObj->isAutoExposureLockSupported(); 
            p3AFeature->u4FocusAreaNum = (MUINT32) mpHal3AObj->getMaxNumFocusAreas(); 
            p3AFeature->u4MeterAreaNum = (MUINT32) mpHal3AObj->getMaxNumMeteringAreas(); 
            ret = MHAL_NO_ERROR; 
        }
        break;
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
mHalCamN3D::
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
mHalCamN3D::
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
MINT32 mHalCamN3D::mHalCamGetCaptureInfo(
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
MINT32
mHalCamN3D::mHalCamInit(
    MVOID *a_pInBuffer,
    MUINT32 a_u4InBufSize
)
{
    AutoCPTLog cptlog(Event_mHalCamInit);
    MINT32 err = MHAL_NO_ERROR;

#ifdef  MTK_NATIVE_3D_SUPPORT
    MUINT32 sensorDev = ISP_SENSOR_DEV_MAIN | ISP_SENSOR_DEV_MAIN_2;
    Hal3ASensorType_e type[2] = {HAL3A_SENSOR_TYPE_RAW, HAL3A_SENSOR_TYPE_YUV};

    MCAM_DBG("[mHalCamInit] \n");
    //
    mCameraId = mu4Camera3dId;
    MCAM_DBG("  mCameraId: %d, %d ", mCameraId, mHalCamSensorInfo[mCameraId].devType);
    mCameraState = MHAL_CAM_N3D_INIT;
    sensorDev = mHalCamSensorInfo[mCameraId].devType;

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
    MCAM_DBG("[mHalCamInit] ECamRole_N3D_Main\n");
    meFeatureCamRole = NSFeature::ECamRole_N3D_Main;

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
    err = mpHal3AObj->init(DUAL_CAMERA_MAIN_SENSOR);
    if (err < 0) {
        MCAM_ERR("  mpHal3AObj->init fail \n");
        return -1;
    }
    //
    //

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
    mHalCamSetState(MHAL_CAM_N3D_IDLE);
    //
    char value[PROPERTY_VALUE_MAX] = {'\0'}; 
    property_get("debug.camera.dump", value, "0");
    mCamDumpOPT = atoi(value); 

    property_get("debug.camera.debug", value, "1"); 
    MCAM_LOG_DBG = atoi(value); 
#endif
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCamN3D::mHalCamUninit(
)
{
    AutoCPTLog cptlog(Event_mHalCamUninit);
    MINT32 err = MHAL_NO_ERROR;
    mHalCamN3dState_e eState;

    MCAM_DBG("[mHalCamUninit] \n");

    // Check it is in the idle mode
    // If it is not, has to wait until idle
    eState = mHalCamGetState();
    if ((eState != MHAL_CAM_N3D_IDLE) && (eState != MHAL_CAM_N3D_ERROR)) {
        MCAM_DBG("  Camera is not in the idle state ... \n");
        if (eState & MHAL_CAM_N3D_PREVIEW_MASK) {
            mHalCamPreviewStop();
        }
        else if (eState & MHAL_CAM_N3D_CAPTURE_MASK) {

        }
        // Polling until idle
        while (eState != MHAL_CAM_N3D_IDLE) {
            // Wait 10 ms per time
            usleep(10000);
            eState = mHalCamGetState();
        }
        MCAM_DBG("  Now camera is in the idle state \n");
    }
    //
    mHalCamSetState(MHAL_CAM_N3D_UNINIT);
    ::sem_post(&semMainHigh);
    CPTLogStr(Event_mHalCamUninit, CPTFlagSeparator, "wait semMainHighEnd");
    ::sem_wait(&semMainHighEnd);
    MCAM_DBG("wait for semMainHighEnd");
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
*******************************************************************************/
MINT32 
mHalCamN3D::
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
    //MCAM_DBG("pmhalCamParam->frmYuv.frmFormat = %d", pmhalCamParam->frmYuv.frmFormat);
    //MCAM_DBG("pDispCfg->imgFmt = %d", pDispCfg->imgFmt);
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
mHalCamN3D::
getInstance()
{
    AutoCPTLog cptlog(Event_mHalCamCreate);
    MCAM_DBG("[getInstance] E \n");
    //
    android::Mutex::Autolock _l(g_mtxSingletonLock);
    //
    if  ( NULL != g_mHalCamSingleton )
    {
        MCAM_ERR("[mHalCamN3D] does not support multi-open; singleton instance:%p", g_mHalCamSingleton);
        return  NULL;
    }
    //
    g_mHalCamSingleton = new mHalCamN3D();
    return g_mHalCamSingleton;
}

/******************************************************************************
*
*******************************************************************************/
void
mHalCamN3D::
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
        MCAM_DBG("[mHalCamN3D] is destroying current instance(%p) != singleton instance(%p)", this, g_mHalCamSingleton);
    }
    //
    delete this;
}

