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
#ifndef _FLICKER_SERVICE_H_
#define _FLICKER_SERVICE_H_


/*******************************************************************************
*
********************************************************************************/
#include <frameservice/FrameService.h>


/*******************************************************************************
*
********************************************************************************/
class FrameService;
class Hal3ABase;


/*******************************************************************************
*
********************************************************************************/
namespace NSCamera
{
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/
class IFlickerService::FlickerService : public FrameService
{
public:     ////
    typedef ESensorType     ESensorType_t;
    typedef EDeviceId       EDeviceId_t;

public:     ////    Constructor/Destructor.
    FlickerService(char const*const szFrameSvcName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId);
    virtual ~FlickerService();

public:     ////    Interfaces.
    //
    virtual MBOOL   init(Hal3ABase*const pHal3A, MUINT32 const u4FlickerMode, EDeviceId_t eDeviceId, MINT32 bZsdOn);
    virtual MBOOL   preUninit();
    virtual MBOOL   uninit();
    //
    virtual MBOOL   onFrameUpdated()            { return MTRUE; }
    //
    virtual MBOOL   setWindowInfo(MUINT32 const u4Width, MUINT32 const u4Height);
    //
    /*
     *  AE_FLICKER_MODE_OFF, AE_FLICKER_MODE_AUTO, AE_FLICKER_MODE_50HZ, AE_FLICKER_MODE_60HZ
     */
    virtual MBOOL   setFlickerMode(MUINT32 const u4FlickerMode, MBOOL const fgIsVideoRecording = MFALSE);
    virtual MBOOL   enableAutoDetection();
    virtual MBOOL   disableAutoDetection();
    //
protected:  ////    Utilities.
    virtual MBOOL   doInit(EDeviceId_t eDeviceId, MINT32 bZsdOn)                    { return MTRUE; }
    virtual MBOOL   doPreUninit()               { return MTRUE; }
    virtual MBOOL   doUninit()                  { return MTRUE; }

public:     ////    Attributes.
    //
    inline MBOOL    isAutoDetectEnabled() const { return mfgIsAutoDetectEnabled; }
    MVOID           setAttr_AutoDetectEnabled(MBOOL const fgIsAutoDetectEnabled);
    //
    MVOID           setAttr_FlickerMode(MUINT32 const u4FlickerMode);

protected:  ////    Attributes.
    mutable android::Mutex  mDataLock;      //  Data-members lock; don't use it to lock operations.
    MBOOL           mfgFlickerModeChanged;  //  just for debug. TRUE if mu4FlickerMode has changed.

private:    ////    Attributes.
    MUINT32         mu4WinWidth;
    MUINT32         mu4WinHeight;
    //
    //  AE_FLICKER_MODE_OFF, AE_FLICKER_MODE_AUTO, AE_FLICKER_MODE_50HZ, AE_FLICKER_MODE_60HZ
    MUINT32         mu4FlickerMode;
    //
    MBOOL           mfgIsAutoDetectEnabled;

protected:  ////    Pipes/Hal.
    Hal3ABase*      mpHal3A;
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamera
#endif  //  _FLICKER_SERVICE_H_

