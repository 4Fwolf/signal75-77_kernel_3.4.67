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
#ifndef _PARAM_CTRL_IF_H_
#define _PARAM_CTRL_IF_H_

#include "isp_tuning.h"
#include "isp_tuning_hal.h"
#include "camera_feature.h"
//
using namespace NSFeature;


namespace NSIspTuning
{


/*******************************************************************************
* Parameter Control Interface
*******************************************************************************/
class ParamctrlIF
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    virtual void    destroyInstance() = 0;

protected:  ////    Ctor/Dtor.
    virtual ~ParamctrlIF() {}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Attributes
    virtual ESensorRole_T   getSensorRole() const = 0;
    virtual ESensorType_T   getSensorType() const = 0;
    virtual EOperMode_T getOperMode()   const = 0;
    virtual MERROR_ENUM setOperMode(EOperMode_T const eOperMode) = 0;

    virtual MVOID       enableDynamicTuning(MBOOL const fgEnable) = 0;
    virtual MBOOL       isDynamicTuning() const = 0;

    virtual MVOID       enableDynamicCCM(MBOOL const fgEnable) = 0;
    virtual MBOOL       isDynamicCCM() const = 0;

    virtual MVOID       updateShadingNVRAMdata(MBOOL const fgEnable) = 0;
    virtual MBOOL       isShadingNVRAMdataChange() const  = 0;
    
    virtual MERROR_ENUM setSensorMode(ESensorMode_T const eSensorMode) = 0;
    virtual MERROR_ENUM setCamMode(ECamMode_T const eCamMode) = 0;
    virtual MERROR_ENUM setSceneMode(EIndex_Scene_T const eScene) = 0;

    virtual MBOOL       isToInvokeOfflineCapture() const = 0;

    virtual MERROR_ENUM setIso(MUINT32 const u4ISOValue) = 0;

    virtual MERROR_ENUM setCCT(MINT32 const i4CCT) = 0;
    virtual MERROR_ENUM setCCTIndex_CCM (
                MINT32 const i4CCT, 
                MINT32 const i4FluorescentIndex
            ) = 0;
    virtual MERROR_ENUM setCCTIndex_Shading(
                MINT32 const i4CCT,
                MINT32 const i4DaylightFluorescentIndex,
                MINT32 const i4DaylightProb,
                MINT32 const i4DaylightFluorescentProb,
                MINT32 const i4SceneLV) = 0;
    virtual MERROR_ENUM setIndex_Shading(MINT32 const i4IDX) = 0;
    virtual MERROR_ENUM getIndex_Shading(MVOID*const pCmdArg) = 0;

    virtual MERROR_ENUM setZoomRatio(MUINT32 const u4ZoomRatio_x100) = 0;
    virtual MERROR_ENUM setSceneLightValue(MINT32 const i4SceneLV_x10) = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Color Effect.
    virtual MERROR_ENUM setEffect(EIndex_Effect_T const eEffect) = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    ISP End-User-Defined Tuning Index.
    virtual MERROR_ENUM setIspUserIdx_Edge(
                EIndex_Isp_Edge_T const eIndex
            ) = 0;
    virtual MERROR_ENUM setIspUserIdx_Hue(
                EIndex_Isp_Hue_T const eIndex
            ) = 0;
    virtual MERROR_ENUM setIspUserIdx_Sat(
                EIndex_Isp_Saturation_T const eIndex
            ) = 0;
    virtual MERROR_ENUM setIspUserIdx_Bright(
                EIndex_Isp_Brightness_T const eIndex
            ) = 0;
    virtual MERROR_ENUM setIspUserIdx_Contrast(
                EIndex_Isp_Contrast_T const eIndex
            ) = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    ISP Enable/Disable.
    virtual MERROR_ENUM setEnable_Meta_Gamma(MBOOL const fgForceEnable) = 0;
    virtual MBOOL       getEnable_Meta_Gamma()  const = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Exif.
    virtual MERROR_ENUM queryExifDebugInfo(
        NSIspExifDebug::IspExifDebugInfo_T& rExifDebugInfo
    ) const = 0;

public:     ////    Operations.
    virtual MERROR_ENUM construct() = 0;
    virtual MERROR_ENUM init() = 0;
    virtual MERROR_ENUM uninit() = 0;
    virtual MERROR_ENUM validate(MBOOL const fgForce) = 0;
    virtual MERROR_ENUM validateFrameless() = 0;
    virtual MERROR_ENUM validatePerFrame(MBOOL const fgForce) = 0;

};


};  //  namespace NSIspTuning
#endif // _PARAM_CTRL_IF_H_

