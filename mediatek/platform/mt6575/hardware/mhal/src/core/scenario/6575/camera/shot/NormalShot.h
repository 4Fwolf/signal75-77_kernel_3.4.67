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
#ifndef _NORMAL_SHOT_H_
#define _NORMAL_SHOT_H_


/*******************************************************************************
*
********************************************************************************/
class IShot;
class ShotBase;
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
class NormalShot : public ShotBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Constructor/Destructor.
    NormalShot(char const*const szShotName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId);

public:     ////
    static NormalShot*  createInstance(char const*const szShotName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId);
    virtual MVOID       destroyInstance();

public:     ////    Interfaces.
    //
    virtual MBOOL   init(ShotParam const& rShotParam, Hal3ABase*const pHal3A, ICameraIO*const pCameraIO);
    virtual MBOOL   uninit();
    virtual MBOOL   capture();

public:     ////    
    virtual MBOOL   configPipe(PortVect const& rvpInPorts, PortVect const& rvpOutPorts);
    virtual MBOOL   createImage(PortVect const& rvpInPorts, PortVect const& rvpOutPorts);
    virtual MBOOL   createBayerRawImage(PortVect const& rvpInPorts,  PortVect const& rvpOutPorts); 
    virtual MBOOL   createRawImage(PortVect const& rvpInPorts, PortVect const& rvpOutPorts);
    virtual MBOOL   createRawImage2(PortVect const& rvpInPorts, PortVect const& rvpOutPorts); 
    virtual MBOOL   createJpgImage(PortVect const& rvpInPorts, PortVect const& rvpOutPorts);
    virtual MBOOL   createRotatedImage(MemoryPortInfo const&rSrcMemPort, MemoryPortInfo const &rRotatedMemPort, MUINT32 const &u4Orientation, PostviewPortInfo& rQVPort); 

public:     ////
    virtual MBOOL   querySensorPort(SensorPortInfo& rPort);
    virtual MBOOL   queryJpegPort(JpegPortInfo& rPort);
    virtual MBOOL   queryPostviewPort(PostviewPortInfo& rPort);
    virtual MBOOL   queryRawMemPort(MemoryPortInfo& rPort);
    virtual MBOOL   queryInputMemPort(MemoryPortInfo& rPort);
	virtual MBOOL    IsOfflineCapMode();
	virtual MINT32    set3ACaptureParam(MUINT32 u4AaaMode);
    virtual MBOOL   queryAWB2pass();

protected:  ////
    virtual MBOOL   handleImageReady();

protected:
    /// for jpeg rotation 
    virtual MBOOL    allocateRotaedMemoryPort(MemoryPortInfo const &rSrcPort, MemoryPortInfo &rPort, MUINT32 const &u4Rotate);
    virtual MBOOL    freeRotatedMemoryPort(MemoryPortInfo &rPort); 

protected:  ////
    ICameraIO*      mpCameraIOObj;

};


#endif  //  _NORMAL_SHOT_H_

