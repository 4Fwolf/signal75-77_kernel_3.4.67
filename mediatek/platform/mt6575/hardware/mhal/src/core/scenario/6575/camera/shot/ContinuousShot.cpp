/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#include "shot_log.h"
#include "IShot.h"
#include "Shotbase.h"
#include "NormalShot.h"
#include "ContinuousShot.h"
#include <isp_hal.h>
//
#include <aaa_hal_base.h>
#include <cutils/properties.h>
#include "CameraProfile.h"


/*******************************************************************************
*
********************************************************************************/
ContinuousShot*
ContinuousShot::
createInstance(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
{
    using namespace NSCamera;
    switch  (eSensorType)
    {
    case eSensorType_RAW:
    case eSensorType_YUV:
        return  new ContinuousShot("ContinuousShot", eSensorType, eDeviceId);
    default:
        CAM_LOGE("[ContinuousShot::createInstance] unsupported sensor type:%d", eSensorType);
        break;
    }
    return  NULL;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ContinuousShot::
destroyInstance()
{
    delete  this;
}


/*******************************************************************************
*
********************************************************************************/
ContinuousShot::
ContinuousShot(char const*const szShotName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
    : NormalShot(szShotName, eSensorType, eDeviceId)
    , mu4CurrentCount(0)
    , mtvLastCaptureStart()
    , mfgIsOfflineCapMode(MFALSE)
{
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
ContinuousShot::
init(ShotParam const& rShotParam, Hal3ABase*const pHal3A, ICameraIO*const pCameraIO)
{
    if  ( ! NormalShot::init(rShotParam, pHal3A, pCameraIO) )
    {
        return  MFALSE;
    }

    mpIspHal->sendCommand(ISP_HAL_CMD_SET_FREE_M4U_FLAG, MFALSE);

    if(mShotParam.u4strobeon)
        mpHal3A->setStrobeOn(MTRUE);

    mu4CurrentCount = 0;
    mfgIsOfflineCapMode = MFALSE;
    return  MTRUE;
}

MBOOL 
ContinuousShot::
uninit()
{   

    if(mfgIsOfflineCapMode)
    {
        AutoCPTLog cptlog(Event_NShot_callbackPostView);
        MY_LOGD("Quickview call back for last capture");
        ShotBase::invokeCallback(MHAL_CAM_CB_POSTVIEW, NULL);
    }

    if(mShotParam.u4strobeon)
        mpHal3A->setStrobeOn(MFALSE);
	  	
    mpIspHal->sendCommand(ISP_HAL_CMD_SET_FREE_M4U_FLAG, MTRUE);

    if  ( ! NormalShot::uninit() )
    {
        return  MFALSE;
    }

    return MTRUE;
}

MBOOL
ContinuousShot::
capture()
{
    MBOOL ret = MFALSE;


    if  ( mShotParam.u4ContinuousShotSpeed < 5)
    {
        timeval tv;
        ::gettimeofday(&tv, NULL);

        MUINT32 const usLast = mtvLastCaptureStart.tv_sec * 1000000 + mtvLastCaptureStart.tv_usec;
        MUINT32 const usCurr = tv.tv_sec * 1000000 + tv.tv_usec;
        MUINT32 const usDiff = (usCurr-usLast);
        MUINT32 const usMinContinuousShotToShotTime = 1000000/mShotParam.u4ContinuousShotSpeed;

        //  Make sure that each capture takes the specified time at least before starting the next capture.
        if  ( usDiff < usMinContinuousShotToShotTime )
        {
            MUINT32 const usSleep = usMinContinuousShotToShotTime - usDiff;
            MY_LOGD("[capture] (mu4CurrentCount, usSleep, usMinContinuousShotToShotTime)=(%d, %d, %d)", mu4CurrentCount, usSleep, usMinContinuousShotToShotTime);
            ::usleep( usSleep );
        }
    }

    ::gettimeofday(&mtvLastCaptureStart, NULL);

    if  ( 0 != mu4CurrentCount )
    {
        mpHal3A->setBypassSensorSetting(MTRUE);
    }

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
ContinuousShot::
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
ContinuousShot::
IsOfflineCapMode()
{
    MBOOL fgIsOfflineCapMode = MFALSE;

    if  ( mShotParam.u4ContinuousShotSpeed <= 5)
    {
        fgIsOfflineCapMode = MTRUE;
    }


    char value[PROPERTY_VALUE_MAX] = {'\0'};     
    property_get("continuous.shot.speed", value, "-1");
    float capFps = atof(value); 


    if(7.499 < capFps && 7.5001 > capFps)
    {
        MY_LOGD("set fast capture by property_set");
        fgIsOfflineCapMode = MFALSE;
    }
    else if(4.999 < capFps && 5.001 > capFps)
    {
        MY_LOGD("set normal capture by property_set");
        fgIsOfflineCapMode = MTRUE;
    }

    if  (
            (mShotParam.u4ZoomRatio != 100)
        ||  (mShotParam.u4IsDumpRaw == 1)
        ||  (mShotParam.u4Scalado == 1)
        ||  (mShotParam.u4Rotate != 0)
        ||  (1 == mShotParam.u4DumpYuvData)
        ||  (mShotParam.frmJpg.w > 2560)
        )
    {
        
        MY_LOGD("Force to offline capture, ZoomRatio=%d, DumpRaw=%d, Scalado=%d, Rotate=%d, DumpYuv=%d, frmJpg.w=%d", 
            mShotParam.u4ZoomRatio,
            mShotParam.u4IsDumpRaw,
            mShotParam.u4Scalado,
            mShotParam.u4Rotate,
            mShotParam.u4DumpYuvData,
            mShotParam.frmJpg.w);
		fgIsOfflineCapMode = MTRUE;
    }
    MY_LOGI("[capture] fgIsOfflineCapMode:0x%x", fgIsOfflineCapMode);

    mfgIsOfflineCapMode = fgIsOfflineCapMode;
    return fgIsOfflineCapMode;
}

MINT32 
ContinuousShot::set3ACaptureParam(MUINT32 u4AaaMode)
{
    return mpHal3A->setCaptureParam(1);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ContinuousShot::
invokeQvCB(MUINT32 const u4Type, MVOID*const pData, MUINT32 const u4Size /*= 0*/)
{
    //
    if(mfgIsOfflineCapMode)
        return  MTRUE;
    else
        return ShotBase::invokeQvCB(u4Type, pData, u4Size);
}
/******************************************************************************
*
*******************************************************************************/
MBOOL
ContinuousShot::
invokeShutterCB()
{
	//
	if(mu4CurrentCount > 0)
		return	MTRUE;
	else
	{
		MY_LOGI("only invoke shutter callback first time");
		return ShotBase::invokeShutterCB();
	}
}


/******************************************************************************
*
*******************************************************************************/


MBOOL
ContinuousShot::    
createRawImage(PortVect const& rvpInPorts, PortVect const& rvpOutPorts)
{
    if  ( ! NormalShot::createRawImage(rvpInPorts, rvpOutPorts) )
    {
        return  MFALSE;
    }

    if  ( 0 == mu4CurrentCount )
    {
        return MTRUE;
    }
    
    AutoCPTLog cptlog(Event_NShot_callbackPostView);
    return invokeCallback(MHAL_CAM_CB_POSTVIEW, NULL);
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
ContinuousShot::    
queryAWB2pass()
{
    MUINT32 mPtr;
    
    if (mShotParam.u4strobeon == 0x1){
        mpHal3A->setState(AAA_STATE_CAPTURE);
        mpHal3A->do3A(AAA_STATE_CAPTURE, &mPtr);
    }
    //customer, 1st 
    else if (0==mu4CurrentCount && 0x1==mShotParam.u4awb2pass){
        mpHal3A->setState(AAA_STATE_CAPTURE);
        mpHal3A->do3A(AAA_STATE_CAPTURE, &mPtr);
    }
    return  MTRUE;
}


