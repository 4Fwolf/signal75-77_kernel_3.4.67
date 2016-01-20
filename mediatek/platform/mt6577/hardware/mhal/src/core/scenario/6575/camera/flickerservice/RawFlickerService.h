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
#ifndef _RAW_FLICKER_SERVICE_H_
#define _RAW_FLICKER_SERVICE_H_


/*******************************************************************************
*
********************************************************************************/
#include <frameservice/FrameService.h>
#include <aaa_param.h>


/*******************************************************************************
*
********************************************************************************/
class FlickerHalBase;


/*******************************************************************************
*
********************************************************************************/
namespace NSCamera
{
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/
class RawFlickerService : public IFlickerService::FlickerService
{
public:     ////    Constructor/Destructor.
    RawFlickerService(char const*const szFrameServiceName, EDeviceId_t const eDeviceId);

public:     ////    Interfaces.
    /*
     *  mDataLock is applied within this operation instead of others.
     */
    virtual MBOOL   onFrameUpdated();
    //
    virtual MBOOL   setWindowInfo(MUINT32 const u4Width, MUINT32 const u4Height);
    //
    virtual MBOOL   enableAutoDetection();
    virtual MBOOL   disableAutoDetection();
    //
protected:  ////    Utilities.
    virtual MBOOL   doInit(EDeviceId_t eDeviceId, MINT32 bZsdOn);
    virtual MBOOL   doPreUninit();
    virtual MBOOL   doUninit();

protected:  ////    Utilities.
    /*
     *  Don't apply mDataLock to these operations since it has been done within onFrameUpdated().
     */
    MBOOL           updateEISInfo();
    MBOOL           updateAAAInfo();

protected:  ////    Attributes.
    //
    MINT32          mi4DetectedResult;

    /*
     *  AF Windows Status.
     *
     *  FIXME:
     *      AF_WIN_NUM is defined in aaa_param.h --> aaa_param_mt6575.h --> af_param_mt6575.h
     *      NUM should be queried from aaa hal.
     */
    enum { eAFWinNum = AF_WIN_NUM };
    MINT32          mai4AFWin[eAFWinNum];
    //
    //  EIS GMV X/Y
    enum { eMaxGMVNum = 1 };
    MINT32          mai4GMV_X[eMaxGMVNum];
    MINT32          mai4GMV_Y[eMaxGMVNum];

protected:  ////    Pipes/Hal.
    FlickerHalBase* mpFlickerHal;
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamera
#endif  //  _RAW_FLICKER_SERVICE_H_

