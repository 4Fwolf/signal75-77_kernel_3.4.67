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
#define LOG_TAG "mHalCamN3D"
//
#include <utils/Errors.h>
#include <utils/threads.h>
#include <cutils/properties.h>
//
#include <linux/rtpm_prio.h>
#include <linux/cache.h> 
#include <sys/mman.h>
#include <sys/prctl.h>
//
#include <mhal/inc/camera.h>
#include <cam_types.h>
#include <cam_port.h>
//
#include "shot_log.h"
#include "shot_profile.h"
#include "IShot.h"
#include "Shotbase.h"
#include <camexif/CamExifN3D.h>
#include <camexif/DbgCamExifN3D.h>
#include <mhal_misc.h>
#include <jpeg_hal.h>
#include <isp_hal.h>
#include <mdp_hal.h>
#include <aaa_hal_base.h>
#include <MediaAssert.h>
#include "CameraProfile.h"

#ifdef MTK_NATIVE_3D_SUPPORT
//property
#include "property/CamUtilsProperty.h"
using namespace android::MHalCamUtils;
#endif
/*******************************************************************************
*
********************************************************************************/
// !!! Please remove me !!!
// Capture
#define MHAL_CAM_THUMB_ADDR_OFFSET  (64 * 1024)
#define MHAL_CAM_JPEG_ADDR_OFFSET   (128 * 1024)
#define MHAL_CAM_HW_JPEG_HEADER_OFFSET   (1 * 1024)
#define MHAL_CAM_JPEG_STEREO_3D_ENABLE   "Camera.Stereo.3D.Enable"


/*******************************************************************************
*
********************************************************************************/
ShotBaseN3D::
ShotBaseN3D(char const*const szShotName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
    : IShotN3D()
    //
    , mszShotName(szShotName)
    , meSensorType(eSensorType)
    , meDeviceId(eDeviceId)
    , mu4ShotDumpOPT(0)
    //
    , mShotParam()
    //
    , mpHal3A(NULL)
    , mpIspHal(NULL)
    , mpMdpHal(NULL)
    //
    , mu4JpgEncDoneSize(0)
    //
    , mShutterCBInfo()
    , mShutterCBThread()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotBaseN3D::
init(ShotParam const& rShotParam, Hal3ABase*const pHal3A)
{
    MBOOL   ret = MFALSE;
    MINT32  err = 0;

    if  ( ! pHal3A )
    {
        MY_LOGE("[init] (pHal3A)=(%p)", pHal3A);
        goto lbExit;
    }

    mShotParam  = rShotParam;
    mpHal3A     = pHal3A;
    //
    if  (
            ! (mpIspHal = IspHal::createInstance()) //fail to create
        ||  0 > ( err = mpIspHal->init() )          //fail to init
        )
    {
        MY_LOGE("[init] (mpIspHal, ec)=(%p, %X)", mpIspHal, err);
        goto lbExit;
    }
    //
    if  (
            ! (mpMdpHal = MdpHal::createInstance()) //fail to create
        ||  0 > ( err = mpMdpHal->init() )          //fail to init
        )
    {
        MY_LOGE("[init] (mpMdpHal, ec)=(%p, %X)", mpMdpHal, err);
        goto lbExit;
    }
    //
    //
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        ::property_get("debug.camera.dump", value, "0");
        mu4ShotDumpOPT = ::atoi(value);
    }
    //


    ret = MTRUE;
lbExit:
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotBaseN3D::
uninit()
{
    if  (mpMdpHal)
    {
        mpMdpHal->uninit();
        mpMdpHal->destroyInstance();
        mpMdpHal = NULL;
    }
    //
    if  (mpIspHal)
    {
        mpIspHal->uninit();
        mpIspHal->destroyInstance();
        mpIspHal = NULL;
    }
    //
    mpHal3A     = NULL;

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotBaseN3D::
setParam(ShotParam const& rShotParam)
{
    mShotParam = rShotParam;
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotBaseN3D::
invokeCallback(MUINT32 const u4Type, MVOID*const pData, MUINT32 const u4Size /*= 0*/)
{
    //
    MY_LOGD("[invokeCallback] + tid(%d), type(0x%x)", ::gettid(), u4Type);
    //
    mShotParam.shotObserver.notify(u4Type, pData, u4Size);
    //
    MY_LOGD("[invokeCallback] - tid(%d), type(0x%x)", ::gettid(), u4Type);
    //
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID*
ShotBaseN3D::
shutterCBThread(MVOID* arg)
{
    ::prctl(PR_SET_NAME,"shutterCBThread", 0, 0, 0);
    //
    CAM_LOGD("[shutterCBThread] + tid(%d)", ::gettid());
    //
    ShotProfileN3D profile("shutterCBThread");
    //
    //  Save the information in local since the object may be destroyed during this thread is running.
    ShutterCBInfo CBInfo = *reinterpret_cast<ShutterCBInfo*>(arg);
    //
    //  Perform Callback Function.
    if  ( CBInfo.mpfCallback )
    {
        CBInfo.mpfCallback(&CBInfo.mInfo);
    }
    //
    profile.print();
    CAM_LOGD("[shutterCBThread] - tid(%d)", ::gettid());
    return  NULL;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotBaseN3D::
invokeShutterCB()
{
    CPTLog(Event_NShot_invokeShutterCB, CPTFlagStart);
    MY_LOGD("[invokeShutterCB] +");
    //
    mHalCamObserver const& rObserver = mShotParam.shotObserver;
    mShutterCBInfo.set(rObserver.mpfCallback, mHalCamCBInfo(rObserver.mCookie, MHAL_CAM_CB_SHUTTER));
    //
    pthread_attr_t const attr = {PTHREAD_CREATE_DETACHED, NULL, 1024*1024, PAGE_SIZE, SCHED_RR, RTPM_PRIO_CAMERA_SHUTTER};

    MINT32 err = ::pthread_create(&mShutterCBThread, &attr, shutterCBThread, &mShutterCBInfo);
    if  (err)
    {
        MY_LOGE("[invokeShutterCB] pthread_create return err(%d)", err);
    }
    //
    MY_LOGD("[invokeShutterCB] - ret(%d)", (0==err));
    CPTLog(Event_NShot_invokeShutterCB, CPTFlagEnd);
    return  (0==err);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotBaseN3D::
waitShutterCBDone()
{
    MINT32  err = 0;

    MY_LOGD("[waitShutterCBDone] not to wait");

    MY_LOGD("[waitShutterCBDone] End - (pid,tid)=(%d,%d)", getpid(), gettid());

    return  (0==err);
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
ShotBaseN3D::
getAlignedSize(MUINT32 const u4Size)
{
    return (u4Size + (L1_CACHE_BYTES)) & ~(L1_CACHE_BYTES-1);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotBaseN3D::
handleCaptureDone(MINT32 const i4ErrCode /*= 0*/)
{
    AutoCPTLog cptlog(Event_NShot_handleCaptureDone);
    MY_LOGD("[handleCaptureDone] i4ErrCode(%d)", i4ErrCode);

    MBOOL ret = MTRUE;

#if 0   //  FIXME TODO
    if (i4ErrCode < 0) {
        //return error code to upper layer
        //mHalCamCbHandle(MHAL_CAM_CB_CAP_ERR, &i4ErrCode); 
    }
#endif

    // Process thumbnail
    CPTLogStr(Event_NShot_handleCaptureDone, CPTFlagSeparator, "process Thumbnail");
    MUINT32 u4ThumbSize = 0;
    if  ( ! makeQuickViewThumbnail(u4ThumbSize) )
    {
    }

    // Raw callback, for quickview callback 
    // QV Image put in mHalCamParam.qv 
    
    CPTLogStr(Event_NShot_handleCaptureDone, CPTFlagSeparator, "callback Raw");
    invokeCallback(MHAL_CAM_CB_RAW, NULL);

    // Postview callback, donothing
    CPTLogStr(Event_NShot_handleCaptureDone, CPTFlagSeparator, "callback PostView");
    invokeCallback(MHAL_CAM_CB_POSTVIEW, NULL);

    // Flatten Jpeg Picture Buffer for callback (including Exif)
    CPTLogStr(Event_NShot_handleCaptureDone, CPTFlagSeparator, "flatten Jpeg Picture Buffer");
    MUINT32 u4JpgPictureSize = 0;
    ret = flattenJpgPicBuf(getJpgEncDoneSize(), u4ThumbSize, u4JpgPictureSize);

    //  Save file if needed.
    if  (mu4ShotDumpOPT)
    {
        CPTLogStr(Event_NShot_handleCaptureDone, CPTFlagSeparator, "dump Jpeg");
        ::mHalMiscDumpToFile((char*)"//sdcard//1.jpg", (MUINT8*)(mShotParam.frmJpg.virtAddr), u4JpgPictureSize);
    }

    // Jpeg callback, for JPEG bitstream callback to AP
    // Note: buffer may be freed during this callback.
    CPTLogStr(Event_NShot_handleCaptureDone, CPTFlagSeparator, "callback Jpeg");
    invokeCallback(MHAL_CAM_CB_JPEG, &u4JpgPictureSize, u4JpgPictureSize);

    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotBaseN3D::
queryIspBayerResolution(MUINT32& ru4Width, MUINT32& ru4Height) const
{
    //  ISP Source Resolution.
    MINT32 err = 0;

    MUINT32 u4PaddW = 0, u4PaddH = 0;

    err = mpIspHal->sendCommand(ISP_CMD_GET_RAW_DUMMY_RANGE, (int)&u4PaddW, (int)&u4PaddH);
    if  (err)
    {
        MY_LOGE("[queryIspBayerResolution] ISP_CMD_GET_RAW_DUMMY_RANGE err(%x)", err);
        goto lbExit;
    }

    if  ( ! queryIspYuvResolution(ru4Width, ru4Height) )
    {
        err = -1;
        goto lbExit;
    }

    ru4Width  += u4PaddW;
    ru4Height += u4PaddH;

lbExit:
    return  (0==err);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotBaseN3D::
queryIspYuvResolution(MUINT32& ru4Width, MUINT32& ru4Height) const
{
    //  ISP Destination Resolution.
    MINT32 err = 0;
    MUINT32 u4W = 0, u4H = 0;

    err = mpIspHal->sendCommand(
        mShotParam.u4CapPreFlag ? ISP_CMD_GET_SENSOR_PRV_RANGE : ISP_CMD_GET_SENSOR_FULL_RANGE, 
        (int)&u4W, (int)&u4H
    );
    if  (err)
    {
        MY_LOGE("[queryIspYuvResolution] ISP_CMD_GET_SENSOR_<PRV/FULL>_RANGE (u4CapPreFlag,err)=(%d,%x)", mShotParam.u4CapPreFlag, err);
    }

    ru4Width    = u4W;
    ru4Height   = u4H;

    return  (0==err);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotBaseN3D::
waitJpgCaptureDone()
{
    MUINT32 u4JpgEncSize = 0;
    MUINT32 err = 0;

    //  (1) Wait for MDP.
    err = mpMdpHal->waitDone(0x01);
    if  (err)
    {
        MY_LOGE("[waitJpgCaptureDone] mpMdpHal->waitDone() - ec(%x)", err);
        goto lbExit;
    }

    //  (2) Update Jpg Enc Done Size.
    err = mpMdpHal->sendCommand(CMD_GET_JPEG_FILE_SIZE, reinterpret_cast<MINT32>(&u4JpgEncSize));
    if  (err)
    {
        MY_LOGE("[waitJpgCaptureDone] mpMdpHal->sendCommand(CMD_GET_JPEG_FILE_SIZE) - ec(%x)", err);
        goto lbExit;
    }

    MY_LOGD("[waitJpgCaptureDone] u4JpgEncSize=%d", u4JpgEncSize);
    setJpgEncDoneSize(u4JpgEncSize);

lbExit:
    return  (0==err);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotBaseN3D::
makeQuickViewThumbnail(MUINT32& ru4ThumbSize)
{
    ShotProfileN3D profile("makeQuickViewThumbnail", getShotName());
    MY_LOGD("[makeQuickViewThumbnail]");

    MBOOL   ret = MTRUE;
    BufInfo const thumbnailBufInfo = BufInfo(
                            mShotParam.frmJpg.virtAddr + MHAL_CAM_THUMB_ADDR_OFFSET, 
                            mShotParam.frmJpg.phyAddr + MHAL_CAM_THUMB_ADDR_OFFSET, 
                            (64 * 1024),
                            1
                        );
    BufInfo const qvBufInfo = BufInfo (
                            mShotParam.frmQv.virtAddr, 
                            mShotParam.frmQv.phyAddr, 
                            mShotParam.frmQv.bufSize, 
                            1
                        ); 

    //
    mhalCamFrame_t const& frmQv = mShotParam.frmQv;
    //
    MUINT32 const u4ThumbW = mShotParam.u4ThumbW;
    MUINT32 const u4ThumbH = mShotParam.u4ThumbH;

    ru4ThumbSize = 0;
    MUINT32 u4Orientation = mShotParam.u4JPEGOrientation; 

    if  ( 0 == u4ThumbW || 0 == u4ThumbH )
    {
        // No thumbnail
        MY_LOGI("[makeQuickViewThumbnail] No thumbnail - (u4ThumbW,u4ThumbH)=(%d,%d)", u4ThumbW, u4ThumbH);
    }
    else
    {
        // Encode thumbnail
        mhalCamFrame_t src, dst;
        ::memset(&src, 0, sizeof(mhalCamFrame_t));
        ::memset(&dst, 0, sizeof(mhalCamFrame_t));

        src.phyAddr = qvBufInfo.u4BufPA;
        src.virtAddr = qvBufInfo.u4BufVA;
        src.w = frmQv.w;
        src.h = frmQv.h;
        src.frmFormat = JpgEncHal::kYV12_Planar_Format; //JpgEncHal::kABGR_8888_Format;

        dst.phyAddr = thumbnailBufInfo.u4BufPA;
        dst.virtAddr = thumbnailBufInfo.u4BufVA;
        dst.w = u4ThumbW;
        dst.h = u4ThumbH;
        dst.bufSize = thumbnailBufInfo.u4BufSize;
        
        if (1 == u4Orientation || 3 == u4Orientation) {
            src.w = frmQv.h;
            src.h = frmQv.w;
            dst.w = u4ThumbH;
            dst.h = u4ThumbW;  
        } 

        ret = encodeJpg(&src, &dst, MTRUE);
        if  ( ret )
        {
            ru4ThumbSize = dst.frmSize;
            //  Save file if needed.
            if  (mu4ShotDumpOPT)
            {
                ::mHalMiscDumpToFile((char*)"//sdcard//thumb.jpg", (MUINT8*)(thumbnailBufInfo.u4BufVA), ru4ThumbSize);
            }
        }
        else
        {
            MY_LOGE("[makeQuickViewThumbnail] encodeJpg fail");
        }
    }

    MY_LOGD("[makeQuickViewThumbnail] thumbSize(%d), ret(%d)", ru4ThumbSize, ret);
    profile.print();
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotBaseN3D::
encodeJpg(mhalCamFrame_t*const psrc, mhalCamFrame_t*const pdst, MBOOL const isSOI)
{
    MY_LOGD("[encodeJpg]");

    MBOOL   ret = MFALSE;
    //
    MUINT32 u4JpgSize = 0;
    //
    JpgEncHal jpgEnc;

    jpgEnc.setSrcWidth(psrc->w);
    jpgEnc.setSrcHeight(psrc->h);
    jpgEnc.setDstWidth(pdst->w);
    jpgEnc.setDstHeight(pdst->h);
    jpgEnc.setQuality(85);
    jpgEnc.setSrcFormat((JpgEncHal::SrcFormat)psrc->frmFormat);
    jpgEnc.setEncFormat(JpgEncHal::kYUV_422_Format);
    #if defined(MTK_M4U_SUPPORT) 
    jpgEnc.setSrcAddr((void *) psrc->virtAddr);
    jpgEnc.setDstAddr((void *) pdst->virtAddr);
    #else
    jpgEnc.setSrcAddr((void *) psrc->phyAddr);
    jpgEnc.setDstAddr((void *) pdst->phyAddr);
    #endif
    jpgEnc.setDstSize(pdst->bufSize);
    jpgEnc.enableSOI(isSOI);

    //FIXME, 
    //jpgEnc.setWaitResTime(500);
    //
    if  ( ! jpgEnc.lock() ) {
        MY_LOGE("[encodeJpg] jpgEnc.lock() failed");
        goto lbExit;
    }
    //
    if ( ! jpgEnc.start(&u4JpgSize) ) {
        MY_LOGE("[encodeJpg] jpgEnc.start() failed");
    }

    jpgEnc.unlock();

    ret = MTRUE;

lbExit:

    pdst->frmSize = u4JpgSize;

    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotBaseN3D::
flattenJpgPicBuf(MUINT32 const u4JpgImgSize, MUINT32 const u4ThumbSize, MUINT32& ru4JpgPictureSize)
{
    ShotProfileN3D profile("flattenJpgPicBuf", getShotName());

    MBOOL   ret = MTRUE;
    mhalCamFrame_t const& frmJpg = mShotParam.frmJpg;
    //
    MUINT32 u4App1HeaderSize = 0;
    MUINT32 u4App1Size = 0;
    MUINT32 u4AppnDebugSize = 0;
    //
    MBOOL isEnable3D = MFALSE;
    MUINT32 u4App3Size = 0;

    char value[256] = {'\0'};
#ifdef MTK_NATIVE_3D_SUPPORT
    Property::get(MHAL_CAM_JPEG_STEREO_3D_ENABLE, value, "0");
    isEnable3D = (MBOOL)(atoi(value));
#endif
    MY_LOGD("[flattenJpgPicBuf] MHAL_CAM_JPEG_STEREO_3D_ENABLE(%d)", isEnable3D);
    //
    MUINT8 *psrc = NULL, *pdest = NULL;
    //
    ru4JpgPictureSize = 0;


    //  (0) Init Exif Object.
    DbgCamExifN3D dbgCamExifN3D(meSensorType, meDeviceId);
    ret = dbgCamExifN3D.init(
        CamExifParam(mShotParam.camExifParam, mShotParam.cam3AParam, mShotParam.u4ZoomRatio), 
        mpHal3A,
        CamDbgParam(mShotParam.u4ShotMode)
    );


    //  (1) Fill Exif info
    pdest = reinterpret_cast<MUINT8*>(frmJpg.virtAddr);
    ret = dbgCamExifN3D.makeExifApp1(frmJpg.w, frmJpg.h, u4ThumbSize, pdest, &u4App1HeaderSize);
    if  ( ! ret )
    {
        goto lbExit;
    }
    MHAL_ASSERT(((pdest[0]==0xFF)&&(pdest[1]==0xd8)), "JPEG header error");
    pdest += u4App1HeaderSize;


    //  (2) Copy thumb Jpg to the end of the exif buffer
    psrc = (MUINT8 *) frmJpg.virtAddr + MHAL_CAM_THUMB_ADDR_OFFSET;
    ::memcpy(pdest, psrc, u4ThumbSize);
    pdest += u4ThumbSize;


    //
    u4App1Size = u4App1HeaderSize + u4ThumbSize;
    MY_LOGD("[flattenJpgPicBuf] u4App1Size: %d", u4App1Size);
    if  ( u4App1Size > 64*1024 )
    {
        MY_LOGW("[flattenJpgPicBuf] u4App1Size(%d) is bigger than 64KB", u4App1Size);
    }

    // (2.x) Stereoscopic Data Descriptor
    if (isEnable3D) {
#ifdef MTK_NATIVE_3D_SUPPORT
        // new interface
        // use n3d compiler option for MP patch back
        ret = dbgCamExifN3D.makeExifApp3(isEnable3D, pdest, &u4App3Size);
#else
        // old interface
        ret = dbgCamExifN3D.makeExifApp3(pdest, &u4App3Size);
#endif
        pdest += u4App3Size;

        MY_LOGD("[flattenJpgPicBuf] pdest: 0x%x", *pdest);
    }

    //  (3) Append Debug Exif Info.
    ret = dbgCamExifN3D.appendDebugExif(pdest, &u4AppnDebugSize);
    pdest += u4AppnDebugSize;


    //  FIXME why?
    //  (4) Copy main Jpg to the end of the exif buffer
    psrc = (MUINT8 *) frmJpg.virtAddr + MHAL_CAM_JPEG_ADDR_OFFSET;
    ::memcpy(pdest, psrc, u4JpgImgSize);


    //  (5) 
    ru4JpgPictureSize = u4App1Size + u4App3Size + u4AppnDebugSize + u4JpgImgSize;

lbExit:
    dbgCamExifN3D.uninit();

    profile.print();
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
BufInfo
ShotBaseN3D::
getBufInfo_JpgEnc() const
{
    MY_LOGD("[getBufInfo_JpgEnc] ");

    return  BufInfo(
        mShotParam.frmJpg.virtAddr + MHAL_CAM_JPEG_ADDR_OFFSET, 
        mShotParam.frmJpg.phyAddr + MHAL_CAM_JPEG_ADDR_OFFSET, 
        mShotParam.frmJpg.bufSize - MHAL_CAM_JPEG_ADDR_OFFSET, 
        1
    );
}


/*******************************************************************************
*
********************************************************************************/
BufInfo
ShotBaseN3D::
getBufInfo_Raw() const
{
    MY_LOGD("[getBufInfo_Raw] ");

    return  BufInfo(
        mShotParam.frmJpg.virtAddr + MHAL_CAM_JPEG_ADDR_OFFSET + MHAL_CAM_HW_JPEG_HEADER_OFFSET, 
        mShotParam.frmJpg.phyAddr + MHAL_CAM_JPEG_ADDR_OFFSET + MHAL_CAM_HW_JPEG_HEADER_OFFSET,
        mShotParam.frmJpg.bufSize - MHAL_CAM_JPEG_ADDR_OFFSET - MHAL_CAM_HW_JPEG_HEADER_OFFSET,
        1
    );
}
