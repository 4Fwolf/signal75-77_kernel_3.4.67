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
#ifndef _PARAM_CTRL_COMM_H_
#define _PARAM_CTRL_COMM_H_

/*******************************************************************************
*
*******************************************************************************/
#include <utils/threads.h>
#define USE_CUSTOM_ISP_TUNING
#include "isp_tuning.h"
#include "paramctrl_if.h"


namespace NSIspTuning
{


/*******************************************************************************
* Parameter Control Common Class
*******************************************************************************/
class ParamctrlComm : public ParamctrlIF
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Common Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Common.
    mutable android::Mutex  m_Lock;
    ESensorRole_T const     m_eSensorRole;
    ESensorMode_T           m_eSensorMode;
    EOperMode_T             m_eOperMode;

private:    ////    Reference to the member of struct IspCamInfo.
    IspCamInfo&             m_rIspCamInfo;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes
    virtual ESensorRole_T   getSensorRole() const { return m_eSensorRole; }

    virtual ESensorMode_T   getSensorMode() const { return m_eSensorMode; }
    virtual MERROR_ENUM     setSensorMode(ESensorMode_T const eSensorMode);

    virtual EOperMode_T     getOperMode()   const { return m_eOperMode; }
    virtual MERROR_ENUM     setOperMode(EOperMode_T const eOperMode);

    virtual MERROR_ENUM setCamMode(ECamMode_T const eCamMode);
    virtual MERROR_ENUM setSceneMode(EIndex_Scene_T const eScene);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Exif.
    virtual MERROR_ENUM queryExifDebugInfo(
        NSIspExifDebug::IspExifDebugInfo_T& rExifDebugInfo
    ) const;

protected:  ////    Ctor/Dtor.
    ParamctrlComm(ESensorRole_T const eSensorRole, IspCamInfo*const pIspCamInfo);

public:     ////    Operations.
    virtual MERROR_ENUM construct();
    virtual MERROR_ENUM init();
    virtual MERROR_ENUM uninit();

protected:  ////    Invoked only by init().
    virtual MERROR_ENUM do_init() { return  MERR_OK; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Validate.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.
    virtual MERROR_ENUM validate(MBOOL const fgForce);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Operations.
    //  Invoked when init() or status change, like Camera Mode. 
    virtual MERROR_ENUM validateFrameless();

protected:  ////    Invoked only by validateFrameless().
    virtual MBOOL   prepareHw_Frameless_All() = 0;
    virtual MBOOL   applyToHw_Frameless_All() = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Operations.
    //  Invoked per frame.
    virtual MERROR_ENUM validatePerFrame(MBOOL const fgForce);

protected:  ////    Invoked only by validatePerFrame().
    virtual MERROR_ENUM do_validatePerFrame() = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Parameters Change.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////

    template <class T>
    MBOOL checkParamChange(T const old_value, T const new_value)
    {
        if  ( old_value != new_value )
        {
            m_u4ParamChangeCount++;
            return  MTRUE;
        }
        return  MFALSE;
    }

protected:  ////
    inline MUINT32  getParamChangeCount() const { return m_u4ParamChangeCount; }
    inline MVOID    resetParamChangeCount() { m_u4ParamChangeCount = 0; }

private:    ////    Data Members.
    //  It means that any params have changed if > 0.
    MUINT32         m_u4ParamChangeCount;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Dynamic Tuning.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes

    virtual MVOID   enableDynamicTuning(MBOOL const fgEnable);
    virtual MBOOL   isDynamicTuning() const { return m_fgDynamicTuning; }

    virtual MVOID   enableDynamicCCM(MBOOL const fgEnable);
    virtual MBOOL   isDynamicCCM() const { return m_fgDynamicCCM; }

    virtual MVOID   updateShadingNVRAMdata(MBOOL const fgEnable);
    virtual MBOOL   isShadingNVRAMdataChange() const  {return m_fgShadingNVRAMdataChange; }

private:    ////    Data Members.
    //  Enable dynamic tuning if true; otherwise false.
    //  It's true by default (normal mode).
    MBOOL           m_fgDynamicTuning;

    //  Enable dynamic CCM if true; otherwise false.
    //  It's true by default (normal mode).
    MBOOL           m_fgDynamicCCM;

protected:  ////
    //  Enable Shading if true; otherwise false.
    //  It's true by default (Enable).
    MBOOL           m_fgShadingNVRAMdataChange;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Color Effect.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual MERROR_ENUM setEffect(EIndex_Effect_T const eEffect);

protected:  ////
    inline EIndex_Effect_T getEffect() const { return m_eIdx_Effect; }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////    Data Members.
    EIndex_Effect_T m_eIdx_Effect;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP End-User-Define Tuning Index.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual MERROR_ENUM setIspUserIdx_Edge(EIndex_Isp_Edge_T const eIndex);
    virtual MERROR_ENUM setIspUserIdx_Hue(EIndex_Isp_Hue_T const eIndex);
    virtual MERROR_ENUM setIspUserIdx_Sat(EIndex_Isp_Saturation_T const eIndex);
    virtual MERROR_ENUM setIspUserIdx_Bright(EIndex_Isp_Brightness_T const eIndex);
    virtual MERROR_ENUM setIspUserIdx_Contrast(EIndex_Isp_Contrast_T const eIndex);

protected:  ////

    inline
    IspUsrSelectLevel_T const&
    getIspUsrSelectLevel() const
    {
        return m_IspUsrSelectLevel;
    }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////    Data Members.
    IspUsrSelectLevel_T     m_IspUsrSelectLevel;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISP Force Enable/Disable
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Gamma.

    virtual MERROR_ENUM setEnable_Meta_Gamma(MBOOL const fgForceEnable);
    virtual MBOOL       getEnable_Meta_Gamma()  const { return m_fgForceEnable_Meta_Gamma; }

private:    ////    Gamma.
    MBOOL               m_fgForceEnable_Meta_Gamma;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

};


/*******************************************************************************
* Parameter Control Template
*******************************************************************************/
template <ESensorType_T _SensorType>
class ParamctrlTemplate : public ParamctrlComm
{
public:     ////    Ctor/Dtor.
    ParamctrlTemplate(ESensorRole_T const eSensorRole, IspCamInfo*const pIspCamInfo)
        : ParamctrlComm(eSensorRole, pIspCamInfo)
    {
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes
    virtual ESensorType_T   getSensorType() const { return _SensorType; }

};


};  //  namespace NSIspTuning
#endif // _PARAM_CTRL_COMM_H_

