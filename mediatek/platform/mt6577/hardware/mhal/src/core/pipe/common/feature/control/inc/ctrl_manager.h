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
#ifndef _CTRL_MANAGER_H_
#define _CTRL_MANAGER_H_


namespace NSFeature
{


/*******************************************************************************
*
*******************************************************************************/
class FeatureHalBridge;


/*******************************************************************************
*
*******************************************************************************/
class CtrlManager : public ControlBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Common.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Typedef.

    //  Type of Pointer to Function for Getting Scene-Independent Features.
    typedef NSFeature::PF_GETFINFO_SCENE_INDEP_T    PF_GETFINFO_SI_T;
    //  Type of Pointer to Function for Getting Scene-Dependent Features.
    typedef NSFeature::PF_GETFINFO_SCENE_DEP_T      PF_GETFINFO_SD_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    

    virtual MBOOL InitFeatures(FeatureHalBridge& rFHBridge);

    virtual
    MBOOL
    SetSceneMode(Fid2Type<FID_SCENE_MODE>::Type eScene)
    {
        m_eCurrentScene = eScene;
        return  MTRUE;
    }

    virtual
    Fid2Type<FID_SCENE_MODE>::Type
    GetSceneMode() const
    {
        return  m_eCurrentScene;
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Data Members.

    // Current Camera: Main/Sub
    // ps: update only when InitFeatures is invoked.
    ECamRole_T                      m_eCurrentCamRole;

    // Current Sensor: RAW/YUV
    // ps: update only when InitFeatures is invoked.
    ESensorType                     m_eSensorType;

    // Current Scene Mode.
    Fid2Type<FID_SCENE_MODE>::Type  m_eCurrentScene;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor/Init/Deinit.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    CtrlManager(CtrlManager const&);
    //  Copy-assignment operator is disallowed.
    CtrlManager& operator=(CtrlManager const&);

protected:  ////    Ctor/Dtor/Init/Deinit.
    CtrlManager();
    virtual ~CtrlManager();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RAW Features.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    

    //  Type of Pointer to Function for Getting Scene-Independent RAW Features.
    typedef PF_GETFINFO_SI_T    PF_GETFINFO_RAW_SI_T;

    //  Type of Pointer to Function for Getting Scene-Dependent RAW Features.
    typedef PF_GETFINFO_SD_T    PF_GETFINFO_RAW_SD_T;

protected:  ////    

    MBOOL   InitFeatures_RAW(FeatureHalBridge& rFHBridge);

    inline
    MBOOL
    GetFeatureProvider_RAW  (
        PF_GETFINFO_RAW_SI_T& rpf_SI, PF_GETFINFO_RAW_SD_T& rpf_SD
    )
    {
        return  GetFeatureProvider_RAW_FromExModule(rpf_SI, rpf_SD);
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  YUV Features.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    

    //  Type of Pointer to Function for Getting Scene-Independent YUV Features.
    typedef PF_GETFINFO_SI_T    PF_GETFINFO_YUV_SI_T;

    //  Type of Pointer to Function for Getting Scene-Dependent YUV Features.
    typedef PF_GETFINFO_SD_T    PF_GETFINFO_YUV_SD_T;

protected:  ////    

    MBOOL   InitFeatures_YUV(FeatureHalBridge& rFHBridge);

    inline
    MBOOL
    GetFeatureProvider_YUV  (
        PF_GETFINFO_YUV_SI_T& rpf_SI, PF_GETFINFO_YUV_SD_T& rpf_SD
    )
    {
        return  GetFeatureProvider_YUV_FromExModule(rpf_SI, rpf_SD);
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Misc. Features.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    

    //  Type of Pointer to Function for Getting Scene-Independent Misc Features.
    typedef PF_GETFINFO_SI_T   PF_GETFINFO_MISC_SI_T;

    //  Type of Pointer to Function for Getting Scene-Dependent Misc Features.
    typedef PF_GETFINFO_SD_T   PF_GETFINFO_MISC_SD_T;

protected:  ////    

    MBOOL           InitFeatures_Misc(FeatureHalBridge& rFHBridge);
    static FInfoIF* GetFInfo_Misc(FID_T const fid);
    static FInfoIF* GetFInfo_Misc(FID_T const fid, SID_T const sid);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Run-Time Features.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    

    MBOOL           InitFeatures_RunTime(FeatureHalBridge& rFHBridge);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Custom Features.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    //  Type of Pointer to Function for Getting Custom Features.
    typedef FInfoIF*(*PF_GETFINFO_CUSTOM_T)(FID_T const fid);

protected:  ////    
    MBOOL   InitFeatures_Custom(FeatureHalBridge& rFHBridge);
    MBOOL   TryOverwriteTargetFeatureByCustom(
        FInfoIF*const pFInfo_Tgt, 
        FInfoIF*const pFInfo_Src
    );

    inline
    MBOOL
    GetFeatureProvider_Custom(PF_GETFINFO_CUSTOM_T& rpf)
    {
        return  GetFeatureProvider_Custom_FromExModule(rpf);
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Others.

    MBOOL   InitFeatures_NoWarningCorrection(FeatureHalBridge& rFHBridge);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface to External Modules.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    typedef enum
    {
        E_FLASHLIGHT_NONE, 
        E_FLASHLIGHT_LED_ONOFF, 
        E_FLASHLIGHT_LED_CONSTANT,
        E_FLASHLIGHT_LED_PEAK,
        E_FLASHLIGHT_LED_TORCH
    } EN_FLASHLIGHT_TYPE_T;

	typedef enum
	{
	  STEREO_3D_NOT_SUPPORT = 0,
	  STEREO_3D_CAPTURE_FRAME_SEQUENTIAL,
	  STEREO_3D_CAPTURE_SIDE_BY_SIDE,
	  STEREO_3D_CAPTURE_TOP_BOTTOM
	} EN_STEREO_3D_TYPE_ENUM;

protected:  ////    

    MBOOL
    GetFlashlightSupport_FromExModule   (
        EN_FLASHLIGHT_TYPE_T& reFlashType
    );

	MBOOL GetStereo3DSupport_FromExModule(EN_STEREO_3D_TYPE_ENUM& reStereo3DType);

    class SensorInfoBase*GetSensorInfo_FromExModule();
    MBOOL   GetSensorType_FromExModule();
    MBOOL   GetAFSupport_FromExModule(MBOOL& rfgAFSupport);

    //  Custom-Specific Module.
    MBOOL   GetFeatureProvider_Custom_FromExModule(
                PF_GETFINFO_CUSTOM_T& rpf
            );

    //  RAW Sensor Driver.
    MBOOL   GetFeatureProvider_RAW_FromExModule(
                PF_GETFINFO_RAW_SI_T& rpf_SI, 
                PF_GETFINFO_RAW_SD_T& rpf_SD
            );

    //  YUV Sensor Driver.
    MBOOL   GetFeatureProvider_YUV_FromExModule(
                PF_GETFINFO_YUV_SI_T& rpf_SI, 
                PF_GETFINFO_YUV_SD_T& rpf_SD
            );
};


};  //  namespace NSFeature

#endif // _CTRL_MANAGER_H_

