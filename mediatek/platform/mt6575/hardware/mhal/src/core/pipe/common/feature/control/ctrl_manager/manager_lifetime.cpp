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
#define LOG_TAG "CtrlManager"
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
//
#include "camera_feature.h"
#include "control_base.h"
#include "ctrl_manager.h"
#include "feature_hal.h"
#include "feature_hal_bridge.h"

using namespace NSFeature;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation of class CtrlManager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CtrlManager::
CtrlManager()
    : m_eCurrentCamRole(ECamRole_Main)
    , m_eSensorType(ESensorType_RAW)
    , m_eCurrentScene(SCENE_MODE_OFF)
{
}


CtrlManager::
~CtrlManager()
{
}


MBOOL
CtrlManager::
InitFeatures(FeatureHalBridge& rFHBridge)
{
    MBOOL fgRet = MFALSE;

    MY_LOG("[InitFeatures]");

//  [1] Main/Sub camera.
    m_eCurrentCamRole = rFHBridge.GetCamRole();

//  [2] (RAW/YUV) sensor built-in features
    if  ( ! GetSensorType_FromExModule() )
    {
        goto lbExit;
    }

    if  ( ESensorType_YUV == m_eSensorType )
    {   //  YUV features
        fgRet = InitFeatures_YUV(rFHBridge);
    }
    else
    {   //  RAW features
        fgRet = InitFeatures_RAW(rFHBridge);
    }

    if  ( ! fgRet )
    {
        goto lbExit;
    }

//  [3] Misc. features
    fgRet = InitFeatures_Misc(rFHBridge);
    if  ( ! fgRet )
    {
        goto lbExit;
    }

//  [4] Run-time features
    fgRet = InitFeatures_RunTime(rFHBridge);
    if  ( ! fgRet )
    {
        goto lbExit;
    }

//  [5] Custom-specific features
    fgRet = InitFeatures_Custom(rFHBridge);
    if  ( ! fgRet )
    {
        goto lbExit;
    }

//  [6] Limitation
//  Conflict test (assert message)

//  [7] Correct without warning
    fgRet = InitFeatures_NoWarningCorrection(rFHBridge);
    if  ( ! fgRet )
    {
        goto lbExit;
    }

    fgRet = MTRUE;
lbExit:
    return  fgRet;
}

