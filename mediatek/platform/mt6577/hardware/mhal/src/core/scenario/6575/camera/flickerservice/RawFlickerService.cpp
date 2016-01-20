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
#include "RawFlickerService.h"
#include <content/IContentManager.h>
#include "kd_camera_feature.h"
#include <aaa_hal_base.h>
#include <flicker_hal_base.h>


using namespace android;
using namespace NSCamera;


/*******************************************************************************
*
********************************************************************************/
RawFlickerService::
RawFlickerService(char const*const szFrameSvcName, EDeviceId_t const eDeviceId)
    : FlickerService(szFrameSvcName, eSensorType_RAW, eDeviceId)
    //
    , mi4DetectedResult(0)
    , mai4AFWin()
    , mai4GMV_X()
    , mai4GMV_Y()
    //
    , mpFlickerHal(NULL)
{
    ::memset(mai4AFWin, 0, sizeof(mai4AFWin));
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RawFlickerService::
doInit(EDeviceId_t eDeviceId, MINT32 bZsdOn)
{
	XLOGD("RawFlickerService::doInit line=%d",__LINE__);
    MBOOL   ret = MFALSE;
    //
    if  ( ! FlickerService::doInit(eDeviceId, bZsdOn) )
    {
        goto lbExit;
    }

    //
    if  ( ! (mpFlickerHal = FlickerHalBase::createInstance()) )
    {
        MY_ERR("[doInit] NULL mpFlickerHal");
        goto lbExit;
    }


	CAMERA_DUAL_CAMERA_SENSOR_ENUM seonsorId;
	if(eDeviceId == eDevId_ImgSensor0)
		seonsorId=DUAL_CAMERA_MAIN_SENSOR;
	else if(eDeviceId == eDevId_ImgSensor1)
		seonsorId=DUAL_CAMERA_SUB_SENSOR;
	else
	{
		XLOGD("Error: can't map sensor ID line=%d",__LINE__);
		goto lbExit;
	}
	mpFlickerHal->setSensorInfo(seonsorId, bZsdOn);


    /*
        case ISP_SENSOR_DEV_MAIN:
        eDeviceId = NSCamera::eDevId_ImgSensor0;
        break;
    case ISP_SENSOR_DEV_SUB:
        eDeviceId = NSCamera::eDevId_ImgSensor1;
        break;
        */

    //

    //  Initialize data members.
    ::memset(mai4GMV_X, 0, sizeof(mai4GMV_X));
    ::memset(mai4GMV_Y, 0, sizeof(mai4GMV_Y));

    ret = MTRUE;
lbExit:

	XLOGD("RawFlickerService::doInit line=%d",__LINE__);
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RawFlickerService::
doPreUninit()
{
    MY_DBG("[doPreUninit] +");
    if  (mpFlickerHal)
    {
        disableAutoDetection();
    }
    //
    FlickerService::doPreUninit();

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RawFlickerService::
doUninit()
{
    if  (mpFlickerHal)
    {
        mpFlickerHal->destroyInstance();
        mpFlickerHal = NULL;
    }
    //
    FlickerService::doUninit();

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RawFlickerService::
updateEISInfo()
{
    using namespace NSContent;
    //
    MBOOL ret = MFALSE;
    //
    Ctn_EisGmv  content;
    ::memset(&content, 0, sizeof(content));
    content.u4Count = 1;
    content.pi4GmvX = mai4GMV_X;
    content.pi4GmvY = mai4GMV_Y;
    //
    IContentManager& rCtnMgr = IContentManager::getInstance();
    //
    ret = rCtnMgr.queryContent(eCtnID_EisGmv, &content);
    if  ( ! ret )
    {
        MY_LOGE("[updateEISInfo] rCtnMgr.queryContent()");
        goto lbExit;
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
RawFlickerService::
updateAAAInfo()
{
    //  (1) Update AF Window Info.
    AF_STAT_T   AFStatus;
    ::memset(&AFStatus, 0, sizeof(AFStatus));
    mpHal3A->getAFValueAll(AFStatus);
    for (int i = 0; i < eAFWinNum; i++)
    {
        mai4AFWin[i] = AFStatus.sWin[i].u4FV[0];
    }

//    MY_DBG("[updateAAAInfo] AF Win:(%d,%d,%d,%d,%d,%d,%d,%d,%d)", mai4AFWin[0], mai4AFWin[1], mai4AFWin[2], mai4AFWin[3], mai4AFWin[4], mai4AFWin[5], mai4AFWin[6], mai4AFWin[7], mai4AFWin[8]);
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RawFlickerService::
onFrameUpdated()
{
    Mutex::Autolock lock(mDataLock);
    //
    MBOOL   ret = MFALSE;
    MINT32  err = 0;
    MINT32  i4DetectedResult = -1;
    //
    //  (1) Bypass if auto detection is disable.
    if  ( ! isAutoDetectEnabled() )
    {
        ret = MTRUE;
        goto lbExit;
    }
    //
    //  (2) Update EIS/AAA Info.
    if  (
            ! updateEISInfo()
        ||  ! updateAAAInfo()
        )
    {
        goto lbExit;
    }
    //
    //  (3) Analyze the flicker by passing EIS information.
    err = mpFlickerHal->analyzeFlickerFrequency(1, mai4GMV_X, mai4GMV_Y, mai4AFWin);
    if  (err)
    {
        MY_ERR("[onFrameUpdated] mpFlickerHal->analyzeFlickerFrequency() - (tid,err)=(%d,%x)", gettid(), err);
        goto lbExit;
    }
    //
    //  (4) Get the flicker result from flicker hal
    err = mpFlickerHal->getFlickerStatus(&i4DetectedResult);
    if  (err)
    {
        MY_ERR("[onFrameUpdated] mpFlickerHal->getFlickerStatus() - (tid,err)=(%d,%x)", gettid(), err);
        goto lbExit;
    }
    //
    //  (5) Debug info.
    if  ( mi4DetectedResult != i4DetectedResult || mfgFlickerModeChanged )
    {
        MY_INFO("[onFrameUpdated] tid(%d), detected result:(old, new)=(%d, %d)", gettid(), mi4DetectedResult, i4DetectedResult);
        mfgFlickerModeChanged = MFALSE;
    }
    mi4DetectedResult = i4DetectedResult;
    //
    //  (6) Pass the flicker result to Hal 3A.
    err = mpHal3A->set3AParam(HAL_3A_AE_FLICKER_AUTO_MODE, mi4DetectedResult, 0, 0);
    if  (err)
    {
        MY_ERR("[onFrameUpdated] mpHal3A->set3AParam(HAL_3A_AE_FLICKER_AUTO_MODE) - (tid,err)=(%d,%x)", gettid(), err);
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
RawFlickerService::
setWindowInfo(MUINT32 const u4Width, MUINT32 const u4Height)
{
    if  ( ! FlickerService::setWindowInfo(u4Width, u4Height) )
    {
        return  MFALSE;
    }
    //
    MINT32 err = 0;
    err = mpFlickerHal->setWindowInfo(u4Width, u4Height);
    if  ( err )
    {
        MY_ERR("[setWindowInfo] mpFlickerHal->setWindowInfo() - err(%x)", err);
        return  MFALSE;
    }
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RawFlickerService::
enableAutoDetection()
{
    MY_INFO("[enableAutoDetection] - (tid,mpFlickerHal,mi4DetectedResult)=(%d,%p,%d)", gettid(), mpFlickerHal, mi4DetectedResult);
    //
    setAttr_AutoDetectEnabled(MTRUE);
    //
    mpFlickerHal->enableFlickerDetection(MTRUE);
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
RawFlickerService::
disableAutoDetection()
{
    MY_INFO("[disableAutoDetection] - (tid,mpFlickerHal,mi4DetectedResult)=(%d,%p,%d)", gettid(), mpFlickerHal, mi4DetectedResult);

    MINT32 err = 0;
    //
    setAttr_AutoDetectEnabled(MFALSE);
    //
    mpFlickerHal->enableFlickerDetection(MFALSE);
    //
    err = mpHal3A->set3AParam(HAL_3A_AE_FLICKER_AUTO_MODE, HAL_FLICKER_AUTO_OFF, 0, 0);
    if  (0 > err)
    {
        MY_ERR("[disableAutoDetection] mpHal3A->set3AParam(HAL_3A_AE_FLICKER_AUTO_MODE, HAL_FLICKER_AUTO_OFF, 0, 0) - err(%x)", err);
        return  MFALSE;
    }

    return  MTRUE;
}

