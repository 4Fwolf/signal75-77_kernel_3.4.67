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
#include <cutils/properties.h>
//
#include <mhal/inc/camera.h>
//
#include <cam_types.h>
#include <cam_port.h>
//
#include "shot_log.h"
#include "IShot.h"
#include "Shotbase.h"
#include "NormalShot.h"
#include "BurstShot.h"
//
#include <aaa_hal_base.h>


/*******************************************************************************
*
********************************************************************************/
BurstShot*
BurstShot::
createInstance(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
{
    using namespace NSCamera;
    switch  (eSensorType)
    {
    case eSensorType_RAW:
    case eSensorType_YUV:
        return  new BurstShot("BurstShot", eSensorType, eDeviceId);
    default:
        CAM_LOGE("[BurstShot::createInstance] unsupported sensor type:%d", eSensorType);
        break;
    }
    return  NULL;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
BurstShot::
destroyInstance()
{
    delete  this;
}


/*******************************************************************************
*
********************************************************************************/
BurstShot::
BurstShot(char const*const szShotName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
    : NormalShot(szShotName, eSensorType, eDeviceId)
    , mu4CurrentCount(0)
    , mtvLastCaptureStart()
{
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
BurstShot::
init(ShotParam const& rShotParam, Hal3ABase*const pHal3A, ICameraIO*const pCameraIO)
{
    if  ( ! NormalShot::init(rShotParam, pHal3A, pCameraIO) )
    {
        return  MFALSE;
    }

    mu4CurrentCount = 0;
    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
namespace NSCamCustom
{
extern MUINT32 getMinBurstShotToShotTime();
}

MBOOL
BurstShot::
capture()
{
    MBOOL ret = MFALSE;

    if  ( 0 != mu4CurrentCount )
    {
        timeval tv;
        ::gettimeofday(&tv, NULL);

        MUINT32 const usLast = mtvLastCaptureStart.tv_sec * 1000000 + mtvLastCaptureStart.tv_usec;
        MUINT32 const usCurr = tv.tv_sec * 1000000 + tv.tv_usec;
        MUINT32 const usDiff = (usCurr-usLast);
        MUINT32 const usMinBurstShotToShotTime = NSCamCustom::getMinBurstShotToShotTime();

        //  Make sure that each capture takes the specified time at least before starting the next capture.
        if  ( usDiff < usMinBurstShotToShotTime )
        {
            MUINT32 const usSleep = usMinBurstShotToShotTime - usDiff;
            MY_LOGD("[capture] (mu4CurrentCount, usSleep, usMinBurstShotToShotTime)=(%d, %d, %d)", mu4CurrentCount, usSleep, usMinBurstShotToShotTime);
            ::usleep( usSleep );
            //
            char isCancelPicture[PROPERTY_VALUE_MAX] = {'\0'};
            ::property_get("camera.hal.event.cancelPicture", isCancelPicture, "0");
            if  ( 1 == ::atoi(isCancelPicture) )
            {
                MY_LOGW("camera.hal.event.cancelPicture=1");
                return  MFALSE;
            }
        }
        mpHal3A->setBypassSensorSetting(MTRUE);
    }

    ::gettimeofday(&mtvLastCaptureStart, NULL);

    ret = NormalShot::capture();

    if  ( 0 != mu4CurrentCount )
    {
        mpHal3A->setBypassSensorSetting(MFALSE);
    }

    MY_LOGD("[capture] (mu4CurrentCount, ret)=(%d, %d)", mu4CurrentCount, ret);

    mu4CurrentCount++;

    return  ret;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
BurstShot::
querySensorPort(SensorPortInfo& rPort)
{
    MBOOL   ret = MFALSE;

    ret = NormalShot::querySensorPort(rPort);

    if  ( 0 < mu4CurrentCount )
    {
        rPort.fgBypassSensorDelay = MTRUE;
        rPort.fgBypassSensorScenaio = MTRUE;
        MY_LOGI("[querySensorPort] (mu4CurrentCount, fgBypassSensorDelay, fgBypassSensorScenaio)=(%d, %d, %d)", mu4CurrentCount, rPort.fgBypassSensorDelay, rPort.fgBypassSensorScenaio);
    }

    return  ret;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
BurstShot::
queryAWB2pass()
{
    MUINT32 mPtr;
    
    if (mShotParam.u4strobeon == 0x1){
        mpHal3A->setState(AAA_STATE_CAPTURE);
        mpHal3A->do3A(AAA_STATE_CAPTURE, &mPtr);
    }
    else if (0==mu4CurrentCount && 0x1==mShotParam.u4awb2pass){
        mpHal3A->setState(AAA_STATE_CAPTURE);
        mpHal3A->do3A(AAA_STATE_CAPTURE, &mPtr);
    }
    return  MTRUE;
}
