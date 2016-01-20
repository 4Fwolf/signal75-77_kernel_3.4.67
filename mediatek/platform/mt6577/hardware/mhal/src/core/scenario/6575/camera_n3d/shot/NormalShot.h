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
#ifndef _NORMAL_SHOT_N3D_H_
#define _NORMAL_SHOT_N3D_H_


/*******************************************************************************
*
********************************************************************************/
class IShotN3D;
class ShotBaseN3D;
class ICameraIO;
using NSCamera::PortVect;
using NSCamera::PortInfo;
using NSCamera::SensorPortInfo;
using NSCamera::JpegPortInfo;
using NSCamera::PostviewPortInfo;
using NSCamera::MemoryPortInfo;


/*******************************************************************************
*
********************************************************************************/
class NormalShotN3D : public ShotBaseN3D
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Constructor/Destructor.
    NormalShotN3D(char const*const szShotName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId);

public:     ////
    static NormalShotN3D*  createInstance(char const*const szShotName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId);
    virtual MVOID       destroyInstance();

public:     ////    Interfaces.
    //
    virtual MBOOL   init(ShotParam const& rShotParam, Hal3ABase*const pHal3A, ICameraIO*const pCameraIO);
    virtual MBOOL   uninit();
    virtual MBOOL   capture();

public:     ////    
    virtual MBOOL   configPipe(PortVect const& rvpInPorts, PortVect const& rvpOutPorts);
    virtual MBOOL   createImage(PortVect const& rvpInPorts, PortVect const& rvpOutPorts);
    virtual MBOOL   createRawImage(PortVect const& rvpInPorts, PortVect const& rvpOutPorts);
    virtual MBOOL   createJpgImage(PortVect const& rvpInPorts, PortVect const& rvpOutPorts);

public:     ////
    virtual MBOOL   querySensorPort(SensorPortInfo& rPort);
    virtual MBOOL   queryJpegPort(JpegPortInfo& rPort);
    virtual MBOOL   queryPostviewPort(PostviewPortInfo& rPort);
    virtual MBOOL   queryRawMemPort(MemoryPortInfo& rPort);
    virtual MBOOL   queryInputMemPort(MemoryPortInfo& rPort);

protected:  ////
    virtual MBOOL   handleImageReady();

protected:  ////
    ICameraIO*      mpCameraIOObj;

};


#endif  //  _NORMAL_SHOT_N3D_H_

