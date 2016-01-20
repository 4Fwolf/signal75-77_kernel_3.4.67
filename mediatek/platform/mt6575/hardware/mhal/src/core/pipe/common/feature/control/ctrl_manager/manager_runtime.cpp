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
#define LOG_TAG "CtrlManager_RunTime"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG           (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
//
#define USE_CAMERA_FEATURE_MACRO 1  //define before "camera_feature.h"
#include "camera_feature.h"
#include "control_base.h"
#include "ctrl_manager.h"
#include "feature_hal.h"
#include "feature_hal_bridge.h"
#include "functor.h"

using namespace NSFeature;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation of class CtrlManager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace
{


/*
    FID_AE_STROBE
    The strobe driver decides whether this feature is supported or not.
    And the supported table depends on the flashlight type.
*/
#define CONFIG_FEATURE_FLASHLIGHT_RT(_fid, _default_macro, _table...)   \
    __CONFIG_FEATURE(_fid, Fid2Type<_fid>::Type)                        \
        _default_macro                                                  \
        static type aTable[] = { _table };                              \
    __END_CONFIG_FEATURE()

struct FUNC_RunTimeFlashlight
{
    enum { FID = FID_AE_STROBE };
    MBOOL operator()(
        FInfoIF* const pFInfo_Tgt, FID_T const fid, SID_T const sid = -1
    ) const
    {
        if  ( ! pFInfo_Tgt )
            return  MFALSE;

        FInfoIF* pFInfo_Flashlight = NULL;
        switch  (m_eFlashType)
        {
        case CtrlManager::E_FLASHLIGHT_LED_ONOFF:
            {
                CONFIG_FEATURE_FLASHLIGHT_RT(FID, 
                    BY_DEFAULT( FLASHLIGHT_FORCE_OFF ), 
                    FLASHLIGHT_FORCE_ON,
                    FLASHLIGHT_FORCE_OFF,
                    FLASHLIGHT_TORCH
                )
                pFInfo_Flashlight = &FInfo;
            }
            break;
        case CtrlManager::E_FLASHLIGHT_LED_CONSTANT:
            {
                if(m_eSensorType == ESensorType_RAW)
                {
                    CONFIG_FEATURE_FLASHLIGHT_RT(FID, 
                        BY_DEFAULT( FLASHLIGHT_FORCE_OFF ), 
                        FLASHLIGHT_AUTO, 
                        FLASHLIGHT_FORCE_ON, 
                        FLASHLIGHT_FORCE_OFF,
                        FLASHLIGHT_TORCH
                    )
                    pFInfo_Flashlight = &FInfo;
                }
                else
                {
                    CONFIG_FEATURE_FLASHLIGHT_RT(FID, 
                        BY_DEFAULT( FLASHLIGHT_FORCE_OFF ), 
                        //FLASHLIGHT_AUTO, 
                        FLASHLIGHT_FORCE_ON, 
                        FLASHLIGHT_FORCE_OFF,
                        FLASHLIGHT_TORCH
                    )
                    pFInfo_Flashlight = &FInfo;
                }
                //pFInfo_Flashlight = &FInfo;
            }
            break;
        case CtrlManager::E_FLASHLIGHT_LED_PEAK:
            {
                CONFIG_FEATURE_FLASHLIGHT_RT(FID, 
                    BY_DEFAULT( FLASHLIGHT_FORCE_OFF ), 
                    FLASHLIGHT_AUTO, 
                    FLASHLIGHT_FORCE_ON, 
                    FLASHLIGHT_FORCE_OFF, 
                    FLASHLIGHT_REDEYE
                )
                pFInfo_Flashlight = &FInfo;
            }
            break;
        case CtrlManager::E_FLASHLIGHT_NONE:
        default:
            break;
        }

        if  ( ! pFInfo_Flashlight )
        {
            pFInfo_Tgt->ClearAll();
#if ENABLE_MY_LOG
            MY_LOG("[FUNC_RunTimeFlashlight][sid:%d] ClearAll\n", sid);
#endif
        }
        else if ( ! pFInfo_Tgt->Copy(pFInfo_Flashlight) )
        {   //  Copy. (Target <- Source)
            MY_LOG("[Target <- Source] Fail to Copy: FLASHLIGHT\n");
            return  MFALSE;
        }
        return  MTRUE;
    }

    FUNC_RunTimeFlashlight(CtrlManager::EN_FLASHLIGHT_TYPE_T const eFlashType,
                ESensorType const eSensorType)
        : m_eFlashType(eFlashType)
        , m_eSensorType(eSensorType)
    {}
private:
    CtrlManager::EN_FLASHLIGHT_TYPE_T const m_eFlashType;
    ESensorType m_eSensorType;
};


/*
    FID_AF_LAMP
    The strobe driver decides whether this feature is supported or not.
    And the supported table depends on the flashlight type.
*/
#define CONFIG_FEATURE_AFLAMP_RT(_fid, _default_macro, _table...)   \
    __CONFIG_FEATURE(_fid, Fid2Type<_fid>::Type)                        \
        _default_macro                                                  \
        static type aTable[] = { _table };                              \
    __END_CONFIG_FEATURE()

struct FUNC_RunTimeAFLamp
{
    enum { FID = FID_AF_LAMP };
    MBOOL operator()(
        FInfoIF* const pFInfo_Tgt, FID_T const fid, SID_T const sid = -1
    ) const
    {
        if  ( ! pFInfo_Tgt )
            return  MFALSE;

        FInfoIF* pFInfo_AFLamp = NULL;
        switch  (m_eFlashType)
        {
        case CtrlManager::E_FLASHLIGHT_LED_ONOFF:
            {
                CONFIG_FEATURE_AFLAMP_RT(FID, 
                    BY_DEFAULT( AF_LAMP_OFF ), 
                    AF_LAMP_OFF,
                    AF_LAMP_ON,
                    AF_LAMP_AUTO
                )
                pFInfo_AFLamp = &FInfo;
            }
            break;
        case CtrlManager::E_FLASHLIGHT_LED_CONSTANT:
            {
                if(m_eSensorType == ESensorType_RAW)
                {
                    CONFIG_FEATURE_AFLAMP_RT(FID, 
                        BY_DEFAULT( AF_LAMP_OFF ), 
                        AF_LAMP_OFF,
                        AF_LAMP_ON,
                        AF_LAMP_AUTO
                    )
                    pFInfo_AFLamp = &FInfo;
                }
                else
                {
                    CONFIG_FEATURE_AFLAMP_RT(FID, 
                        BY_DEFAULT( AF_LAMP_OFF ), 
                        AF_LAMP_OFF,
                        AF_LAMP_ON,
                        //AF_LAMP_AUTO
                    )
                    pFInfo_AFLamp = &FInfo;
                }
                //pFInfo_AFLamp = &FInfo;
            }
            break;
        case CtrlManager::E_FLASHLIGHT_LED_PEAK:
            {
                CONFIG_FEATURE_AFLAMP_RT(FID, 
                    BY_DEFAULT( AF_LAMP_OFF ), 
                    AF_LAMP_OFF
                )
                pFInfo_AFLamp = &FInfo;
            }
            break;
        case CtrlManager::E_FLASHLIGHT_NONE:
        default:
            break;
        }

        if  ( ! pFInfo_AFLamp )
        {
            pFInfo_Tgt->ClearAll();
#if ENABLE_MY_LOG
            MY_LOG("[FUNC_RunTimeAFLamp][sid:%d] ClearAll\n", sid);
#endif
        }
        else if ( ! pFInfo_Tgt->Copy(pFInfo_AFLamp) )
        {   //  Copy. (Target <- Source)
            MY_LOG("[Target <- Source] Fail to Copy: AFLAMP\n");
            return  MFALSE;
        }
        return  MTRUE;
    }

    FUNC_RunTimeAFLamp(CtrlManager::EN_FLASHLIGHT_TYPE_T const eFlashType,
        ESensorType const eSensorType)
        : m_eFlashType(eFlashType)
        , m_eSensorType(eSensorType)
    {}
private:
    CtrlManager::EN_FLASHLIGHT_TYPE_T const m_eFlashType;
    ESensorType m_eSensorType;
};


/*
    FID_AF_MODE
    FID_AF_METERING

    The above featues are supported only when both cases hold true:
    (a) specified in the built-in table.
    (b) supported by the device (run-time)
*/
template <FID_T _fid>
struct FUNC_RunTimeSupport
{
    enum { FID = _fid };
    MBOOL operator()(
        FInfoIF* const pFInfo_Tgt, FID_T const fid, SID_T const sid = -1
    ) const
    {
        if  ( ! pFInfo_Tgt )
            return  MFALSE;
        if  ( ! m_fgIsSupport )
        {   //  If not supported in run-time, force to 
            //  make unsupported in the target table.
            pFInfo_Tgt->ClearAll();
#if ENABLE_MY_LOG
            MY_LOG("[FUNC_RunTimeSupport][fid,sid:%d,%d] ClearAll\n", FID, sid);
#endif
        }
        return  MTRUE;
    }

    FUNC_RunTimeSupport(MBOOL const fgIsSupport)
        : m_fgIsSupport(fgIsSupport)
    {}
private:
    MBOOL const m_fgIsSupport;
};


};  //  namespace

/*
    FID_STEREO_3D_TYPE
*/
#define CONFIG_FEATURE_STEREO_3D_TYPE_RT(_fid, _default_macro, _table...)   \
    __CONFIG_FEATURE(_fid, Fid2Type<_fid>::Type)                        \
        _default_macro                                                  \
        static type aTable[] = { _table };                              \
    __END_CONFIG_FEATURE()

struct FUNC_RunTimeStereo3DType
{
    enum { FID = FID_STEREO_3D_TYPE };
    MBOOL operator()(
        FInfoIF* const pFInfo_Tgt, FID_T const fid, SID_T const sid = -1
    ) const
    {
        if  ( ! pFInfo_Tgt )
            return  MFALSE;

        FInfoIF* pFInfo_Stereo3DType = NULL;

        
        MY_LOG("[RunTimeStereo3DType] %d\n", m_eStereo3DType);

        switch  (m_eStereo3DType)
        {
        case CtrlManager::STEREO_3D_NOT_SUPPORT:
            {
                CONFIG_FEATURE_STEREO_3D_TYPE_RT(FID, 
                    BY_DEFAULT( STEREO_3D_NOT_SUPPORT ), 
                    STEREO_3D_NOT_SUPPORT
                )
                pFInfo_Stereo3DType = &FInfo;
            }
            break;
        case CtrlManager::STEREO_3D_CAPTURE_FRAME_SEQUENTIAL:
            {
                CONFIG_FEATURE_STEREO_3D_TYPE_RT(FID, 
                    BY_DEFAULT( STEREO_3D_FRAME_SEQ ), 
                    STEREO_3D_FRAME_SEQ
                )
                pFInfo_Stereo3DType = &FInfo;
            }
            break;
        case CtrlManager::STEREO_3D_CAPTURE_SIDE_BY_SIDE:
            {
                CONFIG_FEATURE_STEREO_3D_TYPE_RT(FID, 
                    BY_DEFAULT( STEREO_3D_SIDE_BY_SIDE ), 
                    STEREO_3D_SIDE_BY_SIDE
                )
                pFInfo_Stereo3DType = &FInfo;
            }
            break;
        case CtrlManager::STEREO_3D_CAPTURE_TOP_BOTTOM:
            {
                CONFIG_FEATURE_STEREO_3D_TYPE_RT(FID, 
                    BY_DEFAULT( STEREO_3D_TOP_BOTTOM ), 
                    STEREO_3D_TOP_BOTTOM
                )
                pFInfo_Stereo3DType = &FInfo;
            }
            break;
        }

        if  ( ! pFInfo_Stereo3DType )
        {
            pFInfo_Tgt->ClearAll();
#if ENABLE_MY_LOG
            MY_LOG("[FUNC_RunTimeStereo3DType][sid:%d] ClearAll\n", sid);
#endif
        }
        else if ( ! pFInfo_Tgt->Copy(pFInfo_Stereo3DType) )
        {   //  Copy. (Target <- Source)
            MY_LOG("[Target <- Source] Fail to Copy: Stereo3DType\n");
            return  MFALSE;
        }
        return  MTRUE;
    }

    FUNC_RunTimeStereo3DType(CtrlManager::EN_STEREO_3D_TYPE_ENUM const eStereo3DType)
        : m_eStereo3DType(eStereo3DType)
    {}
private:
    CtrlManager::EN_STEREO_3D_TYPE_ENUM const m_eStereo3DType;
};



MBOOL
CtrlManager::
InitFeatures_RunTime(FeatureHalBridge& rFHBridge)
{
    MBOOL fgRet = MFALSE;
    MBOOL fgAFSupport = MFALSE;
    EN_FLASHLIGHT_TYPE_T eFlashType = E_FLASHLIGHT_NONE;
    EN_STEREO_3D_TYPE_ENUM eStereo3DType = STEREO_3D_NOT_SUPPORT;
    ESensorType eSensorType = m_eSensorType;

/*
    FID_AE_STROBE
*/
    fgRet = GetFlashlightSupport_FromExModule(eFlashType)
        &&  Functor_OneFID_TgtDo< FUNC_RunTimeFlashlight >
                                ( FUNC_RunTimeFlashlight
                                 (eFlashType, eSensorType)
                                )(rFHBridge);
    if  ( ! fgRet )
    {
        goto lbExit;
    }

/*
    FID_AF_LAMP
*/
    fgRet = GetAFSupport_FromExModule(fgAFSupport)
        &&  GetFlashlightSupport_FromExModule(eFlashType)
        &&  Functor_OneFID_TgtDo< FUNC_RunTimeAFLamp >
                                ( FUNC_RunTimeAFLamp
                                 (eFlashType, eSensorType)
                                )(rFHBridge);
    if  ( ! fgRet )
    {
        goto lbExit;
    }

/*
    FID_AF_MODE
    FID_AF_METERING
*/
    fgRet = GetAFSupport_FromExModule(fgAFSupport)
        &&  Functor_OneFID_TgtDo< FUNC_RunTimeSupport<FID_AF_MODE > >
                                ( FUNC_RunTimeSupport<FID_AF_MODE >
                                 (fgAFSupport)
                                )(rFHBridge)
        &&  Functor_OneFID_TgtDo< FUNC_RunTimeSupport<FID_AF_METERING> >
                                ( FUNC_RunTimeSupport<FID_AF_METERING>
                                 (fgAFSupport)
                                )(rFHBridge);
    if  ( ! fgRet )
    {
        goto lbExit;
    }

    /*
        FID_STEREO_3D_TYPE
    */
    fgRet = GetStereo3DSupport_FromExModule(eStereo3DType)
        &&  Functor_OneFID_TgtDo< FUNC_RunTimeStereo3DType >
                                ( FUNC_RunTimeStereo3DType
                                 (eStereo3DType)
                                )(rFHBridge);
    if  ( ! fgRet )
    {
        goto lbExit;
    }


    fgRet = MTRUE;
lbExit:
    return  fgRet;
}

