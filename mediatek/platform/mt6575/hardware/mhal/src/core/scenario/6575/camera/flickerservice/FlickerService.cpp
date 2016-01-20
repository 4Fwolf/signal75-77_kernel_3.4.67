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
#include <scenario_types.h>
#include <cam_types.h>
//
#include "my_log.h"
//
#include <frameservice/IFrameService.h>
#include <frameservice/IFlickerService.h>
#include "FlickerService.h"
#include <aaa_hal_base.h>
#include <camera_feature.h>


using namespace android;
using namespace NSCamera;
using namespace NSFeature;


/*******************************************************************************
*
********************************************************************************/
IFlickerService::
FlickerService::
FlickerService(char const*const szFrameSvcName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
    : FrameService(szFrameSvcName, eSensorType, eDeviceId)
    //
    , mDataLock()
    , mfgFlickerModeChanged(MTRUE)
    , mu4WinWidth(0)
    , mu4WinHeight(0)
    , mu4FlickerMode(-1)
    , mfgIsAutoDetectEnabled(MFALSE)
    //
    , mpHal3A(NULL)
{
}


/*******************************************************************************
*
********************************************************************************/
IFlickerService::
FlickerService::
~FlickerService()
{
    uninit();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IFlickerService::
FlickerService::
init(Hal3ABase*const pHal3A, MUINT32 const u4FlickerMode, EDeviceId_t eDeviceId, MINT32 bZsdOn)
{
	XLOGD("FlickerService::init line=%d",__LINE__);
    /*
     *  Here we use the template method pattern, since
     *  not only data members must be initialized,
     *  but also member functions must be invoked.
     */
    MBOOL   ret = MFALSE;
    //
    //  (1) Check parameters.
    if  ( ! pHal3A )
    {
        MY_ERR("[init] (pHal3A)=(%p)", pHal3A);
        goto lbExit;
    }

    //  (2) Initialize data members.
    {
        Mutex::Autolock lock(mDataLock);
        //
        mpHal3A     = pHal3A;
        mu4WinWidth = mu4WinHeight = 0;
    }

	XLOGD("FlickerService::init line=%d",__LINE__);
    //  (3) Invokde hook-function.
    if  ( ! doInit(eDeviceId, bZsdOn) )
    {
        goto lbExit;
    }

    XLOGD("FlickerService::init line=%d",__LINE__);

    //  (4) After everything is ok, set the flicker mode.
    if  ( ! setFlickerMode(u4FlickerMode) )
    {
        goto lbExit;
    }

    ret = MTRUE;
lbExit:
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IFlickerService::
FlickerService::
preUninit()
{
    //  (1) Invokde hook-function.
    return  doPreUninit();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IFlickerService::
FlickerService::
uninit()
{
    //  (1) Invokde hook-function.
    doUninit();

    //  (2) Reset data members.
    {
        Mutex::Autolock lock(mDataLock);
        //
        mpHal3A     = NULL;
        //
        mu4WinWidth = mu4WinHeight = 0;
        mu4FlickerMode = -1;
        mfgFlickerModeChanged = MTRUE;
    }

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IFlickerService::
FlickerService::
setWindowInfo(MUINT32 const u4Width, MUINT32 const u4Height)
{
    MY_INFO("[setWindowInfo] (w,h)=(%d,%d)", u4Width, u4Height);

    Mutex::Autolock lock(mDataLock);
    //
    mu4WinWidth     = u4Width;
    mu4WinHeight    = u4Height;

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IFlickerService::
FlickerService::
setFlickerMode(MUINT32 const u4FlickerMode, MBOOL const fgIsVideoRecording /*= MFALSE*/)
{
    MY_INFO("[FlickerService::setFlickerMode] (old,new)=(%d,%d) video recording?(%d)", mu4FlickerMode, u4FlickerMode, fgIsVideoRecording);
    //
    MBOOL   ret = MFALSE;
    //
    if  ( u4FlickerMode != mu4FlickerMode )
    {
        MINT32  err = 0;
        err = mpHal3A->set3AParam(HAL_3A_AE_FLICKER_MODE, u4FlickerMode, 0, 0);
        if  ( err )
        {
            MY_ERR("[FlickerService::setFlickerMode] mpHal3A->set3AParam(HAL_3A_AE_FLICKER_MODE) - err(%x)", err);
            goto lbExit;
        }

        setAttr_FlickerMode(u4FlickerMode);
        mfgFlickerModeChanged = MTRUE;
    }
    //
    //  Enable auto detection only when not recording video.
    if  ( AE_FLICKER_MODE_AUTO == u4FlickerMode && ! fgIsVideoRecording )
    {
        ret = enableAutoDetection();
    }
    else
    {
        ret = disableAutoDetection();
    }

lbExit:
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IFlickerService::
FlickerService::
enableAutoDetection()
{
    MY_INFO("[enableAutoDetection] do nothing");
    //
    setAttr_AutoDetectEnabled(MTRUE);
    //
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IFlickerService::
FlickerService::
disableAutoDetection()
{
    MY_INFO("[disableAutoDetection] do nothing");
    //
    setAttr_AutoDetectEnabled(MFALSE);
    //
#if 0   //FIXME: Hal3A YUV should support this command.
    MINT32 err = 0;
    //
    err = mpHal3A->set3AParam(HAL_3A_AE_FLICKER_AUTO_MODE, HAL_FLICKER_AUTO_OFF, 0, 0);
    if  (0 > err)
    {
        MY_ERR("[disableAutoDetection] mpHal3A->set3AParam(HAL_3A_AE_FLICKER_AUTO_MODE, HAL_FLICKER_AUTO_OFF, 0, 0) - err(%x)", err);
        return  MFALSE;
    }
#endif
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IFlickerService::
FlickerService::
setAttr_FlickerMode(MUINT32 const u4FlickerMode)
{
    Mutex::Autolock lock(mDataLock);
    //
    mu4FlickerMode = u4FlickerMode;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IFlickerService::
FlickerService::
setAttr_AutoDetectEnabled(MBOOL const fgIsAutoDetectEnabled)
{
    Mutex::Autolock lock(mDataLock);
    //
    mfgIsAutoDetectEnabled = fgIsAutoDetectEnabled;
}

