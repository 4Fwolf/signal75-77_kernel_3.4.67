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
#ifndef _FRAME_SERVICE_H_
#define _FRAME_SERVICE_H_


/*******************************************************************************
*
********************************************************************************/
#include <scenario_types.h>
#include <cam_types.h>


/*******************************************************************************
*
********************************************************************************/
namespace NSCamera
{
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/
class FrameService
{
public:     ////
    typedef ESensorType     ESensorType_t;
    typedef EDeviceId       EDeviceId_t;

protected:  ////    Constructor/Destructor.
    FrameService(char const*const szFrameSvcName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
        : mszFrameSvcName(szFrameSvcName)
        , meSensorType(eSensorType)
        , meDeviceId(eDeviceId)
    {}
    virtual ~FrameService() {}

public:     ////    Attributes.
    //
    inline char const*      getFrameSvcName() const { return mszFrameSvcName; }
    inline ESensorType_t    getSensorType() const   { return meSensorType; }
    inline EDeviceId_t      getDeviceId() const     { return meDeviceId; }
    //
protected:  ////    Attributes.
    char const*const mszFrameSvcName;
    ESensorType_t   meSensorType;
    EDeviceId_t     meDeviceId;

};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamera
#endif  //  _FRAME_SERVICE_H_

