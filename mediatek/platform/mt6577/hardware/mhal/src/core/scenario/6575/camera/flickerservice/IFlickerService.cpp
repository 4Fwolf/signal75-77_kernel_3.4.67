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


using namespace android;
using namespace NSCamera;


/*******************************************************************************
*
********************************************************************************/
IFlickerService*
IFlickerService::
createInstance(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
{
    //
    FlickerService* pImpl = ( eSensorType == eSensorType_RAW )
        ?   new RawFlickerService("RawFlickerService", eDeviceId)
        :   new FlickerService("FlickerService", eSensorType, eDeviceId)
            ;
    if  ( ! pImpl )
    {
        MY_LOGE("[IFlickerService::createInstance] new FlickerService fail");
        return  NULL;
    }
    //
    IFlickerService*pIFlickerService = new IFlickerService(pImpl);
    if  ( ! pIFlickerService )
    {
        MY_LOGE("[IFlickerService::createInstance] new IFlickerService fail");
        delete  pImpl;
        return  NULL;
    }

    return  pIFlickerService;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IFlickerService::
destroyInstance()
{
    delete  mpImpl; //  Firstly, delete the implement instance here instead of destructor.
    delete  this;   //  Finally, delete myself.
}


/*******************************************************************************
*
********************************************************************************/
IFlickerService::
IFlickerService(FlickerService*const pFlickerService)
    : mLock()
    , mpImpl(pFlickerService)
{
}


/*******************************************************************************
*
********************************************************************************/
IFlickerService::
~IFlickerService()
{
}


/*******************************************************************************
*
********************************************************************************/
char const*
IFlickerService::
getFrameSvcName() const
{
    return  getImpl()->getFrameSvcName();
}


/*******************************************************************************
*
********************************************************************************/
IFlickerService::ESensorType_t
IFlickerService::
getSensorType() const
{
    return  getImpl()->getSensorType();
}


/*******************************************************************************
*
********************************************************************************/
IFlickerService::EDeviceId_t
IFlickerService::
getDeviceId() const
{
    return  getImpl()->getDeviceId();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IFlickerService::
init(Hal3ABase*const pHal3A, MUINT32 const u4FlickerMode, EDeviceId_t eDeviceId, MINT32 bZsdOn)
{
    Mutex::Autolock lock(getLockRef());
    //
    return  getImpl()->init(pHal3A, u4FlickerMode, eDeviceId, bZsdOn);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IFlickerService::
preUninit()
{
    Mutex::Autolock lock(getLockRef());
    //
    return  getImpl()->preUninit();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IFlickerService::
uninit()
{
    Mutex::Autolock lock(getLockRef());
    //
    return  getImpl()->uninit();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IFlickerService::
onFrameUpdated()
{
    struct timeval tvBegin, tvEnd;
    ::gettimeofday(&tvBegin, NULL);
    //
    /*
     *  Operation locking:
     *  We do not want to lock this operation since it must take less time and
     *  other operations could be waiting for something under locking.
     *
     *  Data locking:
     *  Data inside Impl must be locked by Impl for race conditions.
     */
//    Mutex::Autolock lock(getLockRef());
    MBOOL ret = getImpl()->onFrameUpdated();
    //
    ::gettimeofday(&tvEnd, NULL);
    int const msElapsed = (tvEnd.tv_sec - tvBegin.tv_sec) * 1000 + (tvEnd.tv_usec - tvBegin.tv_usec) / 1000;
    if  ( msElapsed > 5 )
    {   //  Show logs if > 5 ms
        MY_DBG("[onFrameUpdated] tid(%d), elapsed ms(%d)", gettid(), msElapsed);
    }
    //
    return  ret;
}


/*******************************************************************************
* IFSCmd
********************************************************************************/
IFlickerService::
IFSCmd::
IFSCmd(IFrameService*const pIService)
    : mpIService(reinterpret_cast<IFlickerService*>(pIService))
{
}


MBOOL
IFlickerService::
IFSCmd::
verifySelf()
{
    if  ( NULL == mpIService )
    {
        MY_LOGW("[IFlickerService::IFSCmd::verifySelf] NULL mpIService");
        return  MFALSE;
    }

    if  ( mpIService->getFrameSvcId() != IFlickerService::eFSID )
    {
        MY_LOGW("[IFlickerService::IFSCmd::verifySelf] ID(%d) != IFlickerService::eFSID(%d)", mpIService->getFrameSvcId(), IFlickerService::eFSID);
        return  MFALSE;
    }

    return  MTRUE;
}


/*******************************************************************************
* FSCmd_SetFlickerMode
********************************************************************************/
IFlickerService::
FSCmd_SetFlickerMode::
FSCmd_SetFlickerMode(IFrameService*const pIService, MUINT32 const u4FlickerMode, MBOOL const fgIsVideoRecording)
    : IFSCmd(pIService)
    , mu4FlickerMode(u4FlickerMode)
    , mfgIsVideoRecording(fgIsVideoRecording)
{
}


MBOOL
IFlickerService::
FSCmd_SetFlickerMode::
execute()
{
    if  ( verifySelf() )
    {
        Mutex::Autolock lock(mpIService->getLockRef());
        //
        FlickerService*const pService = reinterpret_cast<IFlickerService*>(mpIService)->getImpl();
        return  pService->setFlickerMode(mu4FlickerMode, mfgIsVideoRecording);
    }
    return  MFALSE;
}


/*******************************************************************************
* FSCmd_EnableAutoDetect
********************************************************************************/
IFlickerService::
FSCmd_EnableAutoDetect::
FSCmd_EnableAutoDetect(IFrameService*const pIService, MBOOL const fgEnable)
    : IFSCmd(pIService)
    , mfgEnable(fgEnable)
{
}


MBOOL
IFlickerService::
FSCmd_EnableAutoDetect::
execute()
{
    if  ( verifySelf() )
    {
        Mutex::Autolock lock(mpIService->getLockRef());
        //
        FlickerService*const pService = reinterpret_cast<IFlickerService*>(mpIService)->getImpl();
        return  (mfgEnable ? pService->enableAutoDetection() : pService->disableAutoDetection());
    }
    return  MFALSE;
}


/*******************************************************************************
* FSCmd_SetWinInfo
********************************************************************************/
IFlickerService::
FSCmd_SetWinInfo::
FSCmd_SetWinInfo(IFrameService*const pIService, MUINT32 const u4WinWidth, MUINT32 const u4WinHeight)
    : IFSCmd(pIService)
    , mu4WinWidth(u4WinWidth)
    , mu4WinHeight(u4WinHeight)
{
}


MBOOL
IFlickerService::
FSCmd_SetWinInfo::
execute()
{
    if  ( verifySelf() )
    {
        Mutex::Autolock lock(mpIService->getLockRef());
        //
        FlickerService*const pService = reinterpret_cast<IFlickerService*>(mpIService)->getImpl();
        return  pService->setWindowInfo(mu4WinWidth, mu4WinHeight);
    }
    return  MFALSE;
}

