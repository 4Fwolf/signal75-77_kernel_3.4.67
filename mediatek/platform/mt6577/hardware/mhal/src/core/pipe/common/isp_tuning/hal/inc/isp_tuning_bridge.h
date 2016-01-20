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
#ifndef _ISP_TUNING_BRIDGE_H_
#define _ISP_TUNING_BRIDGE_H_


namespace NSIspTuning
{
/*******************************************************************************
*
*******************************************************************************/
class ParamctrlIF;


/*******************************************************************************
*
*******************************************************************************/
class IspTuningBridge : public IspTuningHal
{
    friend  IspTuningHal* IspTuningHal::createInstance  (
        ESensorType_T const eSensorType, 
        ESensorRole_T const eSensorRole
    );
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.
    virtual void    destroyInstance();
    virtual MINT32  construct();
    virtual MINT32  init();
    virtual MINT32  uninit();
    virtual MINT32  validate(bool const fgForce /*= false*/);
    virtual MINT32  validateFrameless();
    virtual MINT32  validatePerFrame(bool const fgForce /*= false*/);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Attributes.
    virtual ESensorRole_T   getSensorRole() const;

    virtual EOperMode_T getOperMode()   const;
    virtual MINT32      setOperMode(EOperMode_T const eOperMode);

    virtual MINT32  setSensorMode(ESensorMode_T const eSensorMode);
    virtual MINT32  setCamMode(ECamMode_T const eCamMode);
    virtual MINT32  setSceneMode(MUINT32 const u4Scene);
    virtual MINT32  setEffect(MUINT32 const u4Effect);

    virtual MINT32  updateIso(MUINT32 const u4ISOValue);
    virtual MINT32  updateFluorescentAndCCT (MVOID*const pCCT);
    virtual MINT32  setShadingIndex (MINT32 const i4IDX);
    virtual MINT32  getShadingIndex (MVOID*const pCmdArg);

    virtual MINT32  updateZoomRatio(MUINT32 const u4ZoomRatio_x100);
    virtual MINT32  updateSceneLightValue(MINT32 const i4SceneLV_x10);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    ISP End-User-Defined Tuning Index.
    virtual MINT32  setIspUserIdx_Edge(MUINT32 const u4Index);
    virtual MINT32  setIspUserIdx_Hue(MUINT32 const u4Index);
    virtual MINT32  setIspUserIdx_Sat(MUINT32 const u4Index);
    virtual MINT32  setIspUserIdx_Bright(MUINT32 const u4Index);
    virtual MINT32  setIspUserIdx_Contrast(MUINT32 const u4Index);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Exif.

    virtual MINT32  queryExifDebugInfo(
        NSIspExifDebug::IspExifDebugInfo_T& rExifDebugInfo
    ) const;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Commands.
    virtual MINT32  sendCommand(MVOID*const pCmdArg);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    IspTuningBridge(IspTuningBridge const&);
    //  Copy-assignment operator is disallowed.
    IspTuningBridge& operator=(IspTuningBridge const&);

protected:  ////    
    IspTuningBridge(ParamctrlIF*const pParamctrlIF);
    ~IspTuningBridge();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Data Members.
    ParamctrlIF*const   m_pParamctrl;   //  Pointer to ParamctrlIF.

};


};  //  namespace NSIspTuning

#endif // _ISP_TUNING_BRIDGE_H_

