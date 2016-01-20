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
#define LOG_TAG "NSIspTuning::ParamctrlRAW_attributes"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
#include "paramctrl_raw.h"
//
using namespace android;
using namespace NSIspTuning;


MBOOL
ParamctrlRAW::
isToInvokeOfflineCapture() const
{
    return  m_pIspTuningCustom
        ?   m_pIspTuningCustom->is_to_invoke_offline_capture(m_IspCamInfo)
        :   MFALSE;
}


MERROR_ENUM
ParamctrlRAW::
setIso(MUINT32 const u4ISOValue)
{
    EIndex_ISO_T const eIdx_ISO = 
        m_pIspTuningCustom->map_ISO_value_to_index(u4ISOValue);

    Mutex::Autolock lock(m_Lock);

    checkParamChange(m_IspCamInfo.u4ISOValue, u4ISOValue);
    checkParamChange(m_IspCamInfo.eIdx_ISO, eIdx_ISO);

    m_IspCamInfo.u4ISOValue = u4ISOValue;
    m_IspCamInfo.eIdx_ISO   = eIdx_ISO;

    MY_LOG(
        "[-setIso][m_u4ParamChangeCount:%d](u4ISOValue, eIdx_ISO)=(%d, %d)"
        , getParamChangeCount(), u4ISOValue, eIdx_ISO
    );
    return  MERR_OK;
}


MERROR_ENUM
ParamctrlRAW::
setCCT(MINT32 const i4CCT)
{
    Mutex::Autolock lock(m_Lock);

    MINT32 const i4CCT_old = m_IspCamInfo.i4CCT;

    if  ( checkParamChange(i4CCT_old, i4CCT) )
    {
        m_IspCamInfo.i4CCT = i4CCT_old;
    }

    MY_LOG(
        "[-setCCT][m_u4ParamChangeCount:%d]"
        "(i4CCT_old, i4CCT)=(%d, %d)"
        , getParamChangeCount(), i4CCT_old, i4CCT
    );
    return  MERR_OK;
}


MERROR_ENUM
ParamctrlRAW::
setCCTIndex_CCM(MINT32 const i4CCT, MINT32 const i4FluorescentIndex)
{
    Mutex::Autolock lock(m_Lock);

    EIndex_CCM_CCT_T const
    eIdx_CCM_CCT_old = m_IspCamInfo.eIdx_CCM_CCT;

    EIndex_CCM_CCT_T const
    eIdx_CCM_CCT_new = m_pIspTuningCustom->evaluate_CCM_CCT_index (
            eIdx_CCM_CCT_old, i4CCT, i4FluorescentIndex
    );

    if  ( checkParamChange(eIdx_CCM_CCT_old, eIdx_CCM_CCT_new) )
    {
        m_IspCamInfo.eIdx_CCM_CCT = eIdx_CCM_CCT_new;
    }

    MY_LOG(
        "[-setCCTIndex_CCM][m_u4ParamChangeCount:%d]"
        " CCM CCT index(old, new)=(%d, %d)"
        , getParamChangeCount(), eIdx_CCM_CCT_old, eIdx_CCM_CCT_new
    );
    return  MERR_OK;
}

MERROR_ENUM
ParamctrlRAW::
setCCTIndex_Shading(
    MINT32 const i4CCT,
    MINT32 const i4DaylightFluorescentIndex,
    MINT32 const i4DaylightProb,
    MINT32 const i4DaylightFluorescentProb,
    MINT32 const i4SceneLV)
{
    Mutex::Autolock lock(m_Lock);

    EIndex_Shading_CCT_T const
    eIdx_Shading_CCT_old = m_IspCamInfo.eIdx_Shading_CCT;

    EIndex_Shading_CCT_T const
    eIdx_Shading_CCT_new = m_pIspTuningCustom->evaluate_Shading_CCT_index (
            eIdx_Shading_CCT_old, i4CCT, i4DaylightFluorescentIndex, i4DaylightProb, i4DaylightFluorescentProb, i4SceneLV
    );
    
    if  ( checkParamChange(eIdx_Shading_CCT_old, eIdx_Shading_CCT_new) )
    {
        m_IspCamInfo.eIdx_Shading_CCT = eIdx_Shading_CCT_new;
    }

    MY_LOG(
        "[-setCCTIndex_Shading][m_u4ParamChangeCount:%d]"
        " Shading CCT index(old, new)=(%d, %d)"
        , getParamChangeCount(), eIdx_Shading_CCT_old, eIdx_Shading_CCT_new
    );
    return  MERR_OK;
}

// for tuning tool(CCT) to control shading index directly without provide color temperature
MERROR_ENUM
ParamctrlRAW::
setIndex_Shading(MINT32 const i4IDX)
{
    Mutex::Autolock lock(m_Lock);

    if (i4IDX > eIDX_Shading_CCT_D65)
    {
       LOGD("[setIndex_Shading] Error, Shading index(%d) out of range"
        ,i4IDX
        );
        return MERR_BAD_PARAM;
    }

    m_LscMgr.setIdx(i4IDX);
            
    MY_LOG(
        "[setIndex_Shading]"
        " Shading index (cmd, result)=(%d,%d)"
        , i4IDX
        , m_LscMgr.getIdx()
    );
    return  MERR_OK;
}


MERROR_ENUM
ParamctrlRAW::
getIndex_Shading(MVOID*const pCmdArg)
{
    Mutex::Autolock lock(m_Lock);

    MERROR_ENUM err = MERR_OK;

    MUINT8*const pArg = reinterpret_cast<MUINT8*>(pCmdArg);    
    if  ( ! pArg )
    {
        err = MERR_BAD_PARAM;
        LOGD("[getIndex_Shading] pArg == NULL.");
        goto lbExit;
    }
    //*pArg = reinterpret_cast<MUINT8>(m_LscMgr.getIdx());
    *pArg = m_LscMgr.getIdx();
    
lbExit:
    if  ( MERR_OK != err )
    {
        MY_ERR("[getIndex_Shading] (err, pArg)=(0x%08X, %p)", err, pArg);
    }
    return  err; 
}


MERROR_ENUM
ParamctrlRAW::
setZoomRatio(MUINT32 const u4ZoomRatio_x100)
{
    Mutex::Autolock lock(m_Lock);
    
    MUINT32 const u4ZoomRatio_old = m_IspCamInfo.u4ZoomRatio_x100;

    if  ( checkParamChange(u4ZoomRatio_old, u4ZoomRatio_x100) )
    {
        m_IspCamInfo.u4ZoomRatio_x100 = u4ZoomRatio_x100;
    }

    MY_LOG(
        "[-setZoomRatio][m_u4ParamChangeCount:%d](old, new)=(%d, %d)"
        , getParamChangeCount(), u4ZoomRatio_old, u4ZoomRatio_x100
    );
    return  MERR_OK;
}


MERROR_ENUM
ParamctrlRAW::
setSceneLightValue(MINT32 const i4SceneLV_x10)
{
    Mutex::Autolock lock(m_Lock);

    MINT32 const i4SceneLV_old = m_IspCamInfo.i4LightValue_x10;

    if  ( checkParamChange(i4SceneLV_old, i4SceneLV_x10) )
    {
        m_IspCamInfo.i4LightValue_x10 = i4SceneLV_x10;
    }

    MY_LOG(
        "[-setSceneLightValue][m_u4ParamChangeCount:%d](old, new)=(%d, %d)"
        , getParamChangeCount(), i4SceneLV_old, i4SceneLV_x10
    );
    return  MERR_OK;
}

