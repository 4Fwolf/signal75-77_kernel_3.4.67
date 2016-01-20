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
#define LOG_TAG "NSIspTuning::ParamctrlComm_attributes"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
#include "paramctrl_comm.h"
//
using namespace android;
using namespace NSIspTuning;


MERROR_ENUM
ParamctrlComm::
setOperMode(EOperMode_T const eOperMode)
{
    MY_LOG("[+setOperMode](old, new)=(%d, %d)", m_eOperMode, eOperMode);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_eOperMode, eOperMode) )
    {
        m_eOperMode = eOperMode;
    }

    return  MERR_OK;
}


MERROR_ENUM
ParamctrlComm::
setSensorMode(ESensorMode_T const eSensorMode)
{
    MY_LOG("[+setSensorMode](old, new)=(%d, %d)", m_eSensorMode, eSensorMode);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_eSensorMode, eSensorMode) )
    {
        m_eSensorMode = eSensorMode;
    }

    return  MERR_OK;
}


MVOID
ParamctrlComm::
enableDynamicTuning(MBOOL const fgEnable)
{
    MY_LOG("[+enableDynamicTuning](old, new)=(%d, %d)", m_fgDynamicTuning, fgEnable);

    Mutex::Autolock lock(m_Lock);
    
    if  ( checkParamChange(m_fgDynamicTuning, fgEnable) )
    {
        m_fgDynamicTuning = fgEnable;
    }
}


MVOID
ParamctrlComm::
enableDynamicCCM(MBOOL const fgEnable)
{
    MY_LOG("[+enableDynamicCCM](old, new)=(%d, %d)", m_fgDynamicCCM, fgEnable);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_fgDynamicCCM, fgEnable) )
    {
        m_fgDynamicCCM = fgEnable;
    }
}


MVOID
ParamctrlComm::
updateShadingNVRAMdata(MBOOL const fgEnable)
{
    MY_LOG("[+updateShadingNVRAMdata](old, new)=(%d, %d)", m_fgShadingNVRAMdataChange, fgEnable);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_fgShadingNVRAMdataChange, fgEnable) )
    {
        m_fgShadingNVRAMdataChange = fgEnable;
    }

    MY_LOG("[-updateShadingNVRAMdata] return");
    return;
}


MERROR_ENUM
ParamctrlComm::
setCamMode(ECamMode_T const eCamMode)
{
    MY_LOG("[+setCamMode](old, new)=(%d, %d)", m_rIspCamInfo.eCamMode, eCamMode);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.eCamMode, eCamMode) )
    {
        m_rIspCamInfo.eCamMode = eCamMode;
    }

    return  MERR_OK;
}


MERROR_ENUM
ParamctrlComm::
setSceneMode(EIndex_Scene_T const eScene)
{
    MY_LOG("[+setSceneMode] scene(old, new)=(%d, %d)", m_rIspCamInfo.eIdx_Scene, eScene);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_rIspCamInfo.eIdx_Scene, eScene) )
    {
        m_rIspCamInfo.eIdx_Scene = eScene;
    }

    return  MERR_OK;
}


MERROR_ENUM
ParamctrlComm::
setEffect(EIndex_Effect_T const eEffect)
{
    MY_LOG("[+setEffect] effect(old, new)=(%d, %d)", m_eIdx_Effect, eEffect);

    Mutex::Autolock lock(m_Lock);

    if  ( checkParamChange(m_eIdx_Effect, eEffect) )
    {
        m_eIdx_Effect = eEffect;
    }

    return  MERR_OK;
}

