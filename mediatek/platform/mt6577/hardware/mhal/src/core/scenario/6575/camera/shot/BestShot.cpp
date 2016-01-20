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
#define LOG_TAG "mHalCam"
//
#include <utils/Errors.h>
#include <utils/threads.h>
//
#include <mhal/inc/camera.h>
//
#include <cam_types.h>
#include <cam_port.h>
//
#include <aaa_hal_base.h>
#include "shot_log.h"
#include "IShot.h"
#include "IBestShot.h"
#include "Shotbase.h"
#include "NormalShot.h"
#include "BestShot.h"
using namespace std;


/******************************************************************************
*
*******************************************************************************/
IBestShot*
IBestShot::
createInstance(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
{
    using namespace NSCamera;
    switch  (eSensorType)
    {
    case eSensorType_RAW:
        return  new IBestShot(new BestShot("BestShot", eSensorType, eDeviceId));
    default:
        CAM_LOGE("[IBestShot::createInstance] unsupported sensor type:%d", eSensorType);
        break;
    }
    return  NULL;
}


/******************************************************************************
*
*******************************************************************************/
MVOID
IBestShot::
destroyInstance()
{
    MY_LOGI("[IBestShot::destroyInstance]");
    delete  this;
}


/******************************************************************************
*
*******************************************************************************/
IBestShot::
IBestShot(BestShot*const pShot)
    : IShot()
    , mpShot(pShot)
{
}


/******************************************************************************
*
*******************************************************************************/
IBestShot::
~IBestShot()
{
    MY_LOGI("[IBestShot::~IBestShot]");
    delete  mpShot;
}


/******************************************************************************
*
*******************************************************************************/
char const*
IBestShot::
getShotName() const
{
    return mpShot->getShotName();
}


/******************************************************************************
*
*******************************************************************************/
IShot::ESensorType_t
IBestShot::
getSensorType() const
{
    return mpShot->getSensorType();
}


/******************************************************************************
*
*******************************************************************************/
IShot::EDeviceId_t
IBestShot::
getDeviceId() const
{
    return mpShot->getDeviceId();
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
IBestShot::
init(ShotParam const& rShotParam, Hal3ABase*const pHal3A, ICameraIO*const pCameraIO)
{
    return mpShot->init(rShotParam, pHal3A, pCameraIO);
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
IBestShot::
uninit()
{
    return mpShot->uninit();
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
IBestShot::
capture()
{
    return mpShot->capture();
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
IBestShot::
setParam(ShotParam const& rShotParam)
{
    return mpShot->setParam(rShotParam);
}


/******************************************************************************
*
*******************************************************************************/
BestShot::
BestShot(char const*const szShotName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
    : NormalShot(szShotName, eSensorType, eDeviceId)
    , mu4CurrentCount(0)
    , mu4TotalCount(0)
    , mu4BestShotValue(0)
    , mu4BestShotIndex(0)
    //
    , mvTmpBuf_postview()
    , mvTmpBuf_jpeg()
{
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
init(ShotParam const& rShotParam, Hal3ABase*const pHal3A, ICameraIO*const pCameraIO)
{
    mu4BestShotValue= 0;
    mu4BestShotIndex= 0;
    mu4CurrentCount = 0;
    mu4TotalCount   = rShotParam.u4BusrtCnt;
    if  ( 0 == mu4TotalCount )
    {
        MY_LOGE("[init] total shot count = 0");
        return  MFALSE;
    }
    //
    //
    if  ( ! NormalShot::init(rShotParam, pHal3A, pCameraIO) )
    {
        return  MFALSE;
    }
    //
    //
    MY_LOGI("[init] mpHal3A->enableBestShot(), total shot count = %d", mu4TotalCount);
    mpHal3A->enableBestShot();
    //
    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
uninit()
{
    MY_LOGI("[uninit] mpHal3A->disableBestShot()");
    mpHal3A->disableBestShot();
    NormalShot::uninit();
    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
invokeShutterCB()
{
    if  ( mu4TotalCount-1 == mu4CurrentCount )
    {   //  the last one.
        return  NormalShot::invokeShutterCB();
    }
    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
waitShutterCBDone()
{
    if  ( mu4TotalCount-1 == mu4CurrentCount )
    {   //  the last one.
        return  NormalShot::waitShutterCBDone();
    }
    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
handleImageReady()
{
    mpHal3A->bestShotProcess(); // for Best Shot, AF Process 
    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
handleCaptureDone(MINT32 const i4ErrCode /*= 0*/)
{
    MBOOL ret = MTRUE;
    //
    MUINT32 const u4BestShotValue = mpHal3A->getBestShotValue();
    MY_LOGI(
        "[handleCaptureDone] best shot values:(old,new)=(%d,%d), best shot index:(old,new)=(%d,%d)", 
        mu4BestShotValue, u4BestShotValue, mu4BestShotIndex, mu4CurrentCount
    );
    if  ( u4BestShotValue >= mu4BestShotValue )
    {
        mu4BestShotValue = u4BestShotValue;
        mu4BestShotIndex = mu4CurrentCount;
    }
    //
    //
    if  ( mu4TotalCount-1 == mu4CurrentCount )
    {   //  the last one.
        ret = handleCaptureDone_LastOne();
    }
    else
    {   //  not last one.
        ret = handleCaptureDone_Intermediate();
    }
    //
    MY_LOGD("[handleCaptureDone] (mu4CurrentCount, ret)=(%d, %d)", mu4CurrentCount, ret);
    mu4CurrentCount++;
    //
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
handleCaptureDone_Intermediate()
{
    if  ( mu4BestShotIndex == mu4CurrentCount )
    {   //  [best index == current index]

        MY_LOGI("[handleCaptureDone_Intermediate] best index == current index: %d", mu4BestShotIndex);

        // Generate thumbnail
        MUINT32 u4ThumbSize = 0;
        makeQuickViewThumbnail(u4ThumbSize);

        // Flatten Jpeg Picture Buffer
        MUINT32 u4JpgPictureSize = 0;
        flattenJpgPicBuf(getJpgEncDoneSize(), u4ThumbSize, u4JpgPictureSize);

        //  Save Postview / Jpeg.
        saveToTmpBuf(mvTmpBuf_postview, (MUINT8*)mShotParam.frmQv.virtAddr, mShotParam.frmQv.frmSize);
        saveToTmpBuf(mvTmpBuf_jpeg, (MUINT8*)mShotParam.frmJpg.virtAddr, u4JpgPictureSize);
    }
    //
    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
handleCaptureDone_LastOne()
{
    MBOOL   ret = MFALSE;
    //
    MY_LOGI("[handleCaptureDone_LastOne] the last best-shot: %d is selected", mu4BestShotIndex);
    //
    if  ( mu4TotalCount-1 == mu4BestShotIndex )
    {   //  [best index == the last one]
        //  regard it as normal capture.
        MY_LOGI("[handleCaptureDone_LastOne] best index==the last one --> regard it as normal capture");
        ret = NormalShot::handleCaptureDone(0);
    }
    else
    {   //  [best index != the last one]
        //
        MY_LOGI("[handleCaptureDone_LastOne] best index!=the last one --> restore the buffers of best index");
        //
        //  [1] restore the buffers of best index.
        //
        MUINT32 u4PostviewSize = 0;
        MUINT32 u4JpgPictureSize = 0;
        loadFromTmpBuf(mvTmpBuf_postview, (MUINT8*)mShotParam.frmQv.virtAddr, u4PostviewSize);
        loadFromTmpBuf(mvTmpBuf_jpeg, (MUINT8*)mShotParam.frmJpg.virtAddr, u4JpgPictureSize);

        //  [2] invoke callbacks.
        //
        // Raw callback
        invokeCallback(MHAL_CAM_CB_RAW, NULL);

        // Postview callback
        invokeCallback(MHAL_CAM_CB_POSTVIEW, NULL);

        // Jpeg callback, for JPEG bitstream callback to AP
        invokeCallback(MHAL_CAM_CB_JPEG, &u4JpgPictureSize, u4JpgPictureSize);

        ret = MTRUE;
    }
    //
    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
saveToTmpBuf(TmpBuf_t& rvTmpBuf, MUINT8 const*const pData, MUINT32 const u4Size)
{
    MY_LOGI("[saveToTmpBuf] + (data,size)=(%p,%d)", pData, u4Size);
    //
    if  ( ! pData || 0 == u4Size )
    {
        MY_LOGW("[saveToTmpBuf] bad arguments - (pData,u4Size)=(%p,%d)", pData, u4Size);
        return  MFALSE;
    }
    //
    rvTmpBuf.resize(u4Size);
    ::memcpy(rvTmpBuf.begin(), pData, u4Size);
    //
    MY_LOGI("[saveToTmpBuf] - TmpBuf:(size,capacity)=(%d,%d)", rvTmpBuf.size(), rvTmpBuf.capacity());
    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
BestShot::
loadFromTmpBuf(TmpBuf_t const& rvTmpBuf, MUINT8*const pData, MUINT32& ru4Size)
{
    MY_LOGI("[loadFromTmpBuf] + data(%p); TmpBuf:(size,capacity)=(%d,%d)", pData, rvTmpBuf.size(), rvTmpBuf.capacity());
    //
    if  ( ! pData )
    {
        MY_LOGW("[loadFromTmpBuf] bad arguments - pData(%p)", pData);
        return  MFALSE;
    }
    //
    ru4Size = rvTmpBuf.size();
    ::memcpy(pData, rvTmpBuf.begin(), ru4Size);
    //
    MY_LOGI("[loadFromTmpBuf] -");
    return  MTRUE;
}

