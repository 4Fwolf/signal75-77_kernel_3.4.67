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
#ifndef _PARAM_CTRL_YUV_H_
#define _PARAM_CTRL_YUV_H_

/*******************************************************************************
*
*******************************************************************************/
#include "paramctrl_comm.h"


namespace NSIspTuning
{


/*******************************************************************************
* Parameter Control - YUV Sensor
*******************************************************************************/
class ParamctrlYUV : public ParamctrlTemplate<ESensorType_YUV>
{
public:
    typedef ParamctrlTemplate<ESensorType_YUV>  Parent_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    static ParamctrlYUV*    createInstance(ESensorRole_T const eSensorRole);
    virtual void            destroyInstance();

protected:  ////    Ctor/Dtor.
    ParamctrlYUV(ESensorRole_T const eSensorRole);
    ~ParamctrlYUV();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes

    virtual
    MBOOL
    isToInvokeOfflineCapture() const
    {
        return MFALSE;
    }

    virtual
    MERROR_ENUM
    setIso(MUINT32 const u4ISOValue)
    {
        return MERR_UNSUPPORT;
    }

    virtual
    MERROR_ENUM
    setCCT(MINT32 const i4CCT)
    {
        return MERR_UNSUPPORT;
    }

    virtual
    MERROR_ENUM
    setCCTIndex_CCM(MINT32 const i4CCT, MINT32 const i4FluorescentIndex)
    {
        return MERR_UNSUPPORT;
    }

    virtual
    MERROR_ENUM
    setCCTIndex_Shading(
                MINT32 const i4CCT,
                MINT32 const i4DaylightFluorescentIndex,
                MINT32 const i4DaylightProb,
                MINT32 const i4DaylightFluorescentProb,
                MINT32 const i4SceneLV)
    {
        return MERR_UNSUPPORT;
    }

    virtual
    MERROR_ENUM
    setIndex_Shading(MINT32 const i4IDX)
    {
        return MERR_UNSUPPORT;
    }

    virtual 
    MERROR_ENUM 
    getIndex_Shading(MVOID*const pCmdArg)
    {
        return MERR_UNSUPPORT;
    }

    virtual
    MERROR_ENUM
    setZoomRatio(MUINT32 const u4ZoomRatio_x100)
    {
        return MERR_UNSUPPORT;
    }

    virtual
    MERROR_ENUM
    setSceneLightValue(MINT32 const i4SceneLV_x10)
    {
        return MERR_UNSUPPORT;
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation: Frameless
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    All
    virtual MBOOL   prepareHw_Frameless_All();
    virtual MBOOL   applyToHw_Frameless_All();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation: Per-Frame
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Invoked only by validatePerFrame().
    virtual MERROR_ENUM do_validatePerFrame();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////
    virtual MERROR_ENUM setSceneMode(EIndex_Scene_T const eScene);
    virtual MERROR_ENUM setEffect(EIndex_Effect_T const eEffect);

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
    MERROR_ENUM     apply(MUINT32 u4Fid, MUINT32 u4Val);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    YUVIspCamInfo           m_IspCamInfo;

};


};  //  namespace NSIspTuning
#endif // _PARAM_CTRL_YUV_H_

