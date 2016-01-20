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
#ifndef _I_FLICKER_SERVICE_H_
#define _I_FLICKER_SERVICE_H_


/*******************************************************************************
*
********************************************************************************/
#include <scenario_types.h>
#include <cam_types.h>


/*******************************************************************************
*
********************************************************************************/
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
class IFlickerService : public IFrameService
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static EFrameSvcId const eFSID = eFSID_Flicker;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command Class.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    friend class IFSCmd;
    //
    //  Interface of Command Class.
    class IFSCmd : public IFSCommand
    {
    public:     ////    Interfaces.
        IFSCmd(IFrameService*const pIService);
        virtual MBOOL   verifySelf();
    protected:
        IFlickerService*const   mpIService;
    };

    //  Command: Set Flicker Mode
    class FSCmd_SetFlickerMode : public IFSCmd
    {
    public:     ////    Interfaces.
        FSCmd_SetFlickerMode(IFrameService*const pIService, MUINT32 const u4FlickerMode, MBOOL const fgIsVideoRecording);
        virtual MBOOL   execute();
    protected:  ////    Data Members.
        MUINT32         mu4FlickerMode;
        MBOOL           mfgIsVideoRecording;
    };

    //  Command: Enable/Disable Auto Detection
    class FSCmd_EnableAutoDetect : public IFSCmd
    {
    public:     ////    Interfaces.
        FSCmd_EnableAutoDetect(IFrameService*const pIService, MBOOL const fgEnable);
        virtual MBOOL   execute();
    protected:  ////    Data Members.
        MBOOL           mfgEnable;
    };

    //  Command: Set Window Info.
    class FSCmd_SetWinInfo : public IFSCmd
    {
    public:     ////    Interfaces.
        FSCmd_SetWinInfo(IFrameService*const pIService, MUINT32 const u4WinWidth, MUINT32 const u4WinHeight);
        virtual MBOOL   execute();
    protected:  ////    Data Members.
        MUINT32         mu4WinWidth;
        MUINT32         mu4WinHeight;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    class FlickerService;

protected:  ////    Constructor/Destructor.
    IFlickerService(FlickerService*const pFlickerService);
    virtual ~IFlickerService();

public:     ////
    static IFlickerService* createInstance(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId);
    virtual MVOID           destroyInstance();

public:     ////    Attributes.
    //
    virtual EFrameSvcId     getFrameSvcId() const   { return eFSID; }
    virtual char const*     getFrameSvcName() const;
    virtual ESensorType_t   getSensorType() const;
    virtual EDeviceId_t     getDeviceId() const;

public:     ////    Interfaces.
    //
    virtual MBOOL           init(Hal3ABase*const pHal3A, MUINT32 const u4FlickerMode, EDeviceId_t eDeviceId, MINT32 bZsdOn);
    virtual MBOOL           preUninit();
    virtual MBOOL           uninit();
    //
    virtual MBOOL           onFrameUpdated();

protected:  ////    Implementation.
    android::Mutex&                 getLockRef()    { return mLock; }
    mutable android::Mutex          mLock;
    //
    inline FlickerService const*    getImpl() const { return mpImpl; }
    inline FlickerService*          getImpl()       { return mpImpl; }
    FlickerService*                 mpImpl;

};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamera
#endif  //  _I_FLICKER_SERVICE_H_

