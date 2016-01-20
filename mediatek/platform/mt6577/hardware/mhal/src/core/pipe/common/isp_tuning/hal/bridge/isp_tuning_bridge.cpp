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
#define LOG_TAG "NSIspTuning::Bridge"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG           (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
#define USE_CUSTOM_ISP_TUNING
#include "isp_tuning.h"
#include "paramctrl_if.h"
#include "isp_tuning_hal.h"
#include "isp_tuning_bridge.h"
//
using namespace NSIspExifDebug;
using namespace NSIspTuning;


IspTuningBridge::
IspTuningBridge(ParamctrlIF*const pParamctrlIF)
    : IspTuningHal()
    , m_pParamctrl(pParamctrlIF)
{
}


IspTuningBridge::
~IspTuningBridge()
{
    m_pParamctrl->destroyInstance();
}


MINT32
IspTuningBridge::
construct()
{
    return  m_pParamctrl->construct();
}


MINT32
IspTuningBridge::
init()
{
    return  m_pParamctrl->init();
}


MINT32
IspTuningBridge::
uninit()
{
    return  m_pParamctrl->uninit();
}


MINT32
IspTuningBridge::
validate(bool const fgForce /*= false*/)
{
    return  m_pParamctrl->validate(fgForce);
}


MINT32
IspTuningBridge::
validateFrameless()
{
    return  m_pParamctrl->validateFrameless();
}


MINT32
IspTuningBridge::
validatePerFrame(bool const fgForce /*= false*/)
{
    return  m_pParamctrl->validatePerFrame(fgForce);
}


ESensorRole_T
IspTuningBridge::
getSensorRole() const
{
    return  m_pParamctrl->getSensorRole();
}



EOperMode_T
IspTuningBridge::
getOperMode()   const
{
    return  m_pParamctrl->getOperMode();
}


MINT32
IspTuningBridge::
setOperMode(EOperMode_T const eOperMode /*= EOperMode_Normal*/)
{
    return  m_pParamctrl->setOperMode(eOperMode);
}

MINT32
IspTuningBridge::
setSensorMode(ESensorMode_T const eSensorMode)
{
    MERROR_ENUM err = MERR_OK;
    err = m_pParamctrl->setSensorMode(eSensorMode);
    return  err;
}

MINT32
IspTuningBridge::
setCamMode(ECamMode_T const eCamMode)
{
    MERROR_ENUM err = MERR_OK;
    err = m_pParamctrl->setCamMode(eCamMode);
    return  err;
}


MINT32
IspTuningBridge::
setSceneMode(MUINT32 const u4Scene)
{
    if  ( Fid2Type<FID_SCENE_MODE>::Num <= u4Scene )
    {
        MY_ERR("[setSceneMode](NUM,sid)=(%d,%d)", Fid2Type<FID_SCENE_MODE>::Num, u4Scene);
        return  MERR_BAD_PARAM;
    }
    return  m_pParamctrl->setSceneMode(
        static_cast<EIndex_Scene_T>(u4Scene)
    );
}


MINT32
IspTuningBridge::
setEffect(MUINT32 const u4Effect)
{
    if  ( Fid2Type<FID_COLOR_EFFECT>::Num <= u4Effect )
    {
        MY_ERR("[setEffect](NUM,u4Effect)=(%d,%d)", Fid2Type<FID_COLOR_EFFECT>::Num, u4Effect);
        return  MERR_BAD_PARAM;
    }

    return  m_pParamctrl->setEffect(
        static_cast<EIndex_Effect_T>(u4Effect)
    );
}


MINT32
IspTuningBridge::
updateIso(MUINT32 const u4ISOValue)
{
    return  m_pParamctrl->setIso(u4ISOValue);
}


MINT32
IspTuningBridge::
updateFluorescentAndCCT(MVOID*const pCCT)
{
    MERROR_ENUM err = MERR_OK;

    ISP_CCT_T rCCT = *static_cast<ISP_CCT_T *>(pCCT);

    //  (1) CCT
    err = m_pParamctrl->setCCT(rCCT.i4CCT);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

    //  (2) CCT Index for CCM
    err = m_pParamctrl->setCCTIndex_CCM(rCCT.i4CCT, rCCT.i4FluorescentIndex);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

    //  (3) CCT Index for Shading
    err = m_pParamctrl->setCCTIndex_Shading(rCCT.i4CCT, rCCT.i4DaylightFluorescentIndex, rCCT.i4DaylightProb, rCCT.i4DaylightFluorescentProb, rCCT.i4SceneLV);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

lbExit:
    return  err;
}

MINT32
IspTuningBridge::
setShadingIndex(MINT32 const i4IDX)
{
    MERROR_ENUM err = MERR_BAD_PARAM;

    err = m_pParamctrl->setIndex_Shading(i4IDX);

    return err;
}


MINT32
IspTuningBridge::    
getShadingIndex (MVOID*const pCmdArg)
{
    MERROR_ENUM err = MERR_BAD_PARAM;

    err = m_pParamctrl->getIndex_Shading((MVOID *)pCmdArg);

    return  err;    
}

MINT32
IspTuningBridge::
updateZoomRatio(MUINT32 const u4ZoomRatio_x100)
{
    return  m_pParamctrl->setZoomRatio(u4ZoomRatio_x100);
}


MINT32
IspTuningBridge::
updateSceneLightValue(MINT32 const i4SceneLV_x10)
{
    return  m_pParamctrl->setSceneLightValue(i4SceneLV_x10);
}


MINT32
IspTuningBridge::
queryExifDebugInfo(IspExifDebugInfo_T& rExifDebugInfo) const
{
    return  m_pParamctrl->queryExifDebugInfo(rExifDebugInfo);
}


MINT32
IspTuningBridge::
sendCommand(MVOID*const pCmdArg)
{
    MERROR_ENUM err = MERR_BAD_PARAM;

    CmdArg_T*const pArg = reinterpret_cast<CmdArg_T*>(pCmdArg);
    if  ( ! pArg )
    {
        MY_ERR("[sendCommand] pArg == NULL.");
        goto lbExit;
    }

    switch  (pArg->eCmd)
    {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case ECmd_DecideOfflineCapture:
        if  ( pArg->pOutBuf && pArg->u4OutBufSize >= sizeof(MBOOL) )
        {
            *reinterpret_cast<MBOOL*>(pArg->pOutBuf) = m_pParamctrl->isToInvokeOfflineCapture();
            pArg->u4ActualOutSize = sizeof(MBOOL);
            err = MERR_OK;
        }
        break;
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case ECmd_SetDynamicTuning:
        if  ( pArg->pInBuf && pArg->u4InBufSize >= sizeof(MBOOL) )
        {
            m_pParamctrl->enableDynamicTuning(*reinterpret_cast<MBOOL*>(pArg->pInBuf));
            err = MERR_OK;
        }
        break;
    //
    case ECmd_GetDynamicTuning:
        if  ( pArg->pOutBuf && pArg->u4OutBufSize >= sizeof(MBOOL) )
        {
            *reinterpret_cast<MBOOL*>(pArg->pOutBuf) = m_pParamctrl->isDynamicTuning();
            pArg->u4ActualOutSize = sizeof(MBOOL);
            err = MERR_OK;
        }
        break;
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case ECmd_SetDynamicCCM:
        if  ( pArg->pInBuf && pArg->u4InBufSize >= sizeof(MBOOL) )
        {
            m_pParamctrl->enableDynamicCCM(*reinterpret_cast<MBOOL*>(pArg->pInBuf));
            err = MERR_OK;
        }
        break;
    //
    case ECmd_GetDynamicCCM:
        if  ( pArg->pOutBuf && pArg->u4OutBufSize >= sizeof(MBOOL) )
        {
            *reinterpret_cast<MBOOL*>(pArg->pOutBuf) = m_pParamctrl->isDynamicCCM();
            pArg->u4ActualOutSize = sizeof(MBOOL);
            err = MERR_OK;
        }
        break;
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case ECmd_SetForceCtrl_Gamma:
        if  ( pArg->pInBuf && pArg->u4InBufSize >= sizeof(MBOOL) )
        {
            MBOOL const fgEnable = *reinterpret_cast<MBOOL*>(pArg->pInBuf);
            err = m_pParamctrl->setEnable_Meta_Gamma(fgEnable);
        }
        break;
    //
    case ECmd_GetForceCtrl_Gamma:
        if  ( pArg->pOutBuf && pArg->u4OutBufSize >= sizeof(MBOOL) )
        {
            MBOOL& rfgEnable = *reinterpret_cast<MBOOL*>(pArg->pOutBuf);
            rfgEnable = m_pParamctrl->getEnable_Meta_Gamma();
            pArg->u4ActualOutSize = sizeof(MBOOL);
            err = MERR_OK;
        }
        break;
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    case ECmd_ModShadingNVRAMData:
        if  ( pArg->pInBuf && pArg->u4InBufSize >= sizeof(MBOOL) )
        {
            m_pParamctrl->updateShadingNVRAMdata(*reinterpret_cast<MBOOL*>(pArg->pInBuf));
            err = MERR_OK;
        }
        break;
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    
    default:
        MY_ERR("[sendCommand] bad command: %d", pArg->eCmd);
        err = MERR_BAD_CTRL_CODE;
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    }

lbExit:
    if  ( MERR_OK != err )
    {
        MY_ERR("[sendCommand] (err, pArg)=(0x%08X, %p)", err, pArg);
        if  ( pArg )
        {
            MY_ERR(
                "(eCmd, pInBuf, u4InBufSize, pOutBuf, u4OutBufSize)=(%d, %p, %d, %p, %d)"
                , pArg->eCmd, pArg->pInBuf, pArg->u4InBufSize, pArg->pOutBuf, pArg->u4OutBufSize
            );
        }
    }
    return  err;
}

