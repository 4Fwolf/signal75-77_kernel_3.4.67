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
#define LOG_TAG "CtrlManager_ExModule"
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
//
#include "camera_custom_sensor.h"
#include "sensor_drv.h"
#include "strobe_drv.h"
#include "mcu_drv.h"
#include "isp_drv.h"

using namespace NSFeature;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  External Function Prototype.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace NSFeature {
namespace NSCustom  {
    extern PF_GETFINFO_SCENE_INDEP_T GetFInfo_RAW_Main();
    extern PF_GETFINFO_SCENE_INDEP_T GetFInfo_RAW_Sub ();
    extern PF_GETFINFO_SCENE_INDEP_T GetFInfo_YUV_Main();
    extern PF_GETFINFO_SCENE_INDEP_T GetFInfo_YUV_Sub ();
    extern PF_GETFINFO_SCENE_INDEP_T GetFInfo_N3D_YUV_Main();    
    extern PF_GETFINFO_SCENE_INDEP_T GetFInfo_B3D_YUV_Main();
};  //  namespace NSCustom
};  //  namespace NSFeature


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation of class CtrlManager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
CtrlManager::
GetFeatureProvider_Custom_FromExModule(
    PF_GETFINFO_CUSTOM_T& rpf
)
{
    switch (m_eCurrentCamRole)
    {
    case ECamRole_Main:
        rpf = (ESensorType_RAW == m_eSensorType)
            ?  NSCustom::GetFInfo_RAW_Main()
            :  NSCustom::GetFInfo_YUV_Main();
        break;
    case ECamRole_Sub:
        rpf = (ESensorType_RAW == m_eSensorType)
            ?  NSCustom::GetFInfo_RAW_Sub()
            :  NSCustom::GetFInfo_YUV_Sub();
        break;
    case ECamRole_N3D_Main:
        rpf = NSCustom::GetFInfo_N3D_YUV_Main();
        break;        
    case ECamRole_B3D_Main:
        rpf = NSCustom::GetFInfo_B3D_YUV_Main();
        break;  
    default:
        //  Shouldn't happen.
        MY_ERR(
            "[GetFeatureProvider_Custom_FromExModule]"
            "Not implement when m_eCurrentCamRole=%d", 
            m_eCurrentCamRole
        );
        rpf = NULL;
        break;
    }

    return  (NULL != rpf);
}


MBOOL
CtrlManager::
GetFeatureProvider_RAW_FromExModule(
    PF_GETFINFO_RAW_SI_T& rpf_SI, 
    PF_GETFINFO_RAW_SD_T& rpf_SD
)
{
    MBOOL   fgRet = MFALSE;
    FeatureInfoProvider rFInfoProvider;
    rpf_SI = NULL;
    rpf_SD = NULL;

    if ( 0 == IspDrv::getFeatureProvider(&rFInfoProvider) )
    {
        rpf_SI = rFInfoProvider.pfGetFInfo_SceneIndep;
        rpf_SD = rFInfoProvider.pfGetFInfo_SceneDep;
    }

    fgRet = (rpf_SI && rpf_SD);
    return  fgRet;
}


MBOOL
CtrlManager::
GetFeatureProvider_YUV_FromExModule(
    PF_GETFINFO_YUV_SI_T& rpf_SI, 
    PF_GETFINFO_YUV_SD_T& rpf_SD
)
{
    MBOOL   fgRet = MTRUE;
    FeatureInfoProvider rFInfoProvider;
    rpf_SI = NULL;
    rpf_SD = NULL;

    SensorInfoBase*const pSensorInfo = GetSensorInfo_FromExModule();
    if  ( ! pSensorInfo )
    {
        goto lbExit;
    }

    fgRet = pSensorInfo->GetFeatureProvider(rFInfoProvider);
    if  ( ! fgRet )
    {
        MY_LOG("[pSensorInfo->GetFeatureProvider] return failure\n");
        goto lbExit;
    }

    rpf_SI = rFInfoProvider.pfGetFInfo_SceneIndep;
    rpf_SD = rFInfoProvider.pfGetFInfo_SceneDep;

lbExit:
    fgRet = (rpf_SI && rpf_SD);
    return  fgRet;
}

MBOOL
CtrlManager::
GetStereo3DSupport_FromExModule(EN_STEREO_3D_TYPE_ENUM& reStereo3DType)
{
    #ifdef MTK_NATIVE_3D_SUPPORT

    reStereo3DType = STEREO_3D_CAPTURE_SIDE_BY_SIDE;

    MY_LOG("[GetN3DSupport]N3DType = %d", reStereo3DType);

    #else

    SensorDrv*const pSensorDrv = SensorDrv::createInstance(SENSOR_MAIN);

    reStereo3DType = STEREO_3D_NOT_SUPPORT;

    if  ( ! pSensorDrv )
    {
        MY_ERR("[GetB3DSupport]pSensorDrv==NULL");
        goto lbExit;
    }

    ACDK_SENSOR_INFO_STRUCT SensorInfo;
    ACDK_SENSOR_CONFIG_STRUCT SensorConfigData;

    if (0 != pSensorDrv->getInfo(ACDK_SCENARIO_ID_CAMERA_PREVIEW, &SensorInfo, &SensorConfigData))
    {
        MY_ERR("[GetB3DSupport]getInfo fail");
        goto lbExit;
    }

    reStereo3DType = (EN_STEREO_3D_TYPE_ENUM)SensorInfo.SensorDriver3D;

    MY_LOG("[GetB3DSupport]B3DType = %d", reStereo3DType);

    #endif

    return MTRUE;    

lbExit:

    return MFALSE;
}

SensorInfoBase*
CtrlManager::
GetSensorInfo_FromExModule()
{
    SensorInfoBase* pSensorInfo = NULL;
    SensorDrv*const pSensorDrv = SensorDrv::createInstance(SENSOR_MAIN);
    if  ( ! pSensorDrv )
    {
        MY_ERR("[GetSensorInfo]pSensorDrv==NULL");
        goto lbExit;
    }

    switch  (m_eCurrentCamRole)
    {
    case ECamRole_Main:
    case ECamRole_N3D_Main:        
    case ECamRole_B3D_Main:        
        pSensorInfo = pSensorDrv->getMainSensorInfo();
        break;
    case ECamRole_Sub:
        pSensorInfo = pSensorDrv->getSubSensorInfo();
        break;
    default:
        //  Shouldn't happen.
        MY_ERR("[GetSensorInfo] unexpected CamRole(%d)", m_eCurrentCamRole);
        break;
    }
    pSensorDrv->destroyInstance();

    if  ( ! pSensorInfo )
    {
        MY_ERR("[GetSensorInfo]pSensorInfo==NULL");
    }
lbExit:
    return  pSensorInfo;
}


MBOOL
CtrlManager::
GetSensorType_FromExModule()
{
    SensorInfoBase*const pSensorInfo = GetSensorInfo_FromExModule();
    if  ( ! pSensorInfo )
    {
        return  MFALSE;
    }

    switch ( pSensorInfo->GetType() )
    {
    case NSFeature::SensorInfoBase::EType_RAW:
        m_eSensorType = ESensorType_RAW;
        break;
    case NSFeature::SensorInfoBase::EType_YUV:
        m_eSensorType = ESensorType_YUV;
        break;
    default:
        //  Shouldn't happen.
        MY_ERR("[GetSensorType] unexpected type(%d)", pSensorInfo->GetType());
        return  MFALSE;
    }

    return  MTRUE;
}


MBOOL
CtrlManager::
GetFlashlightSupport_FromExModule(EN_FLASHLIGHT_TYPE_T& reFlashType)
{
    StrobeDrv*const pStrobeDrv = StrobeDrv::createInstance();
    if  ( ! pStrobeDrv )
    {
        MY_ERR("[GetFlashlightSupport]pStrobeDrv==NULL");
        return  MFALSE;
    }

    StrobeDrv::FLASHLIGHT_TYPE_ENUM
    eFlashlightType = pStrobeDrv->getFlashlightType();

    pStrobeDrv->destroyInstance();

    switch  (eFlashlightType)
    {
    case StrobeDrv::FLASHLIGHT_NONE:
        reFlashType = E_FLASHLIGHT_NONE;
        MY_LOG("[FlashlightType]:FLASHLIGHT_NONE");
        break;
    case StrobeDrv::FLASHLIGHT_LED_CONSTANT:    // constant strobe type LED
        reFlashType = E_FLASHLIGHT_LED_CONSTANT;
        MY_LOG("[FlashlightType]:FLASHLIGHT_LED_CONSTANT");
        break;
    case StrobeDrv::FLASHLIGHT_LED_PEAK:    // peak strobe type LED
        reFlashType = E_FLASHLIGHT_LED_PEAK;
        MY_LOG("[FlashlightType]:FLASHLIGHT_LED_PEAK");
        break;
    case StrobeDrv::FLASHLIGHT_LED_ONOFF:   // LED always on/off
    default:
        reFlashType = E_FLASHLIGHT_LED_ONOFF;
        MY_LOG("[GetFlashlightSupport]:FLASHLIGHT_LED_ONOFF");
        break;
    }

    return  MTRUE;
}


MBOOL
CtrlManager::
GetAFSupport_FromExModule(MBOOL& rfgAFSupport)
{
/*
    If MCUDrv fails, by default, AF is regarded as unsupported and 
    this function return success.
    Don't assert error messages over here.
*/
    MINT32 i4CurrSensorDev = 0, i4CurrSensorId = 0, i4CurrLensId = 0;

    rfgAFSupport = MFALSE;

#if 1//FIXME

    int iErrCode = MCUDrv::MCU_NO_ERROR;

    mcuMotorInfo rMcuMotorInfo;
    SensorDrv* pSensorDrv = NULL;
    MCUDrv* pMCUDrv = NULL;

    if (m_eCurrentCamRole == ECamRole_Main || m_eCurrentCamRole == ECamRole_N3D_Main || m_eCurrentCamRole == ECamRole_B3D_Main)  {

        pSensorDrv = SensorDrv::createInstance(SENSOR_MAIN);
        i4CurrSensorDev  = 0;
        i4CurrSensorId = pSensorDrv->getMainSensorID();
    }
    else  {

        pSensorDrv = SensorDrv::createInstance(SENSOR_SUB);
        i4CurrSensorDev  = 1;
        i4CurrSensorId = pSensorDrv->getSubSensorID();        
    }      

    MCUDrv::lensSearch(i4CurrSensorDev, i4CurrSensorId);
    i4CurrLensId = MCUDrv::getCurrLensID();    
    //MY_LOG("[CTRL_hal][CurrSensorId] %d [CurrLensId] %d", i4CurrSensorId, i4CurrLensId);   
    pMCUDrv = MCUDrv::createInstance(i4CurrLensId);

    if  ( ! pMCUDrv )
    {
        MY_LOG("[GetAFSupport][Create]pMCUDrv==NULL");
        goto lbExit;
    }

    iErrCode = pMCUDrv->init();
    if  ( MCUDrv::MCU_NO_ERROR != iErrCode )
    {
        MY_LOG("[GetAFSupport][init]iErrCode(0x%08X)", iErrCode);
        goto lbExit;
    }

    iErrCode = pMCUDrv->getMCUInfo(&rMcuMotorInfo);
    if  ( MCUDrv::MCU_NO_ERROR != iErrCode )
    {
        MY_LOG("[GetAFSupport][getMCUInfo]iErrCode(0x%08X)", iErrCode);
        goto lbExit;
    }

    pMCUDrv->uninit();

    rfgAFSupport = (rMcuMotorInfo.bIsMotorOpen)
        ?   MTRUE
        :   MFALSE;

lbExit:

    if  ( pMCUDrv )
    {
        pMCUDrv->destroyInstance();
    }

    if  ( pSensorDrv )
    {
        pSensorDrv->destroyInstance();
    }

#endif//FIXME
    MY_LOG("[-GetAFSupport]:AFSupport:%d", rfgAFSupport);
    return  MTRUE;
}

