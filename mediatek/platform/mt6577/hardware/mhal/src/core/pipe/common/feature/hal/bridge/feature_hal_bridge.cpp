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
#define LOG_TAG "FeatureHalBridge"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG           (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
//
#include "camera_feature.h"
#include "feature_hal.h"
#include "feature_hal_bridge.h"
#include "control_base.h"

using namespace NSFeature;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation of class FeatureHalBridge
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FeatureHalBridge::
FeatureHalBridge(ControlBase*const pCtrl)
    : FeatureHal()
    , m_rCtrl(*pCtrl)
{
    MY_ASSERT(NULL != pCtrl, "FeatureHalBridge(pCtrl=NULL)");
}


MBOOL
FeatureHalBridge::
Init()
{
    return  m_rCtrl.InitFeatures(*this);
}


MBOOL
FeatureHalBridge::
setSceneMode(SID_T const sid)
{
    MY_LOG("[+setSceneMode](Old,New)=(%d,%d)", m_rCtrl.GetSceneMode(), sid);
    if  ( Fid2Type<FID_SCENE_MODE>::Num <= sid || ! IsSceneSupport(sid) )
    {
#if ENABLE_MY_LOG
        MY_ERR(
            "[setSceneMode](NUM,sid,support)=(%d,%d,%d)"
            , Fid2Type<FID_SCENE_MODE>::Num, sid, IsSceneSupport(sid)
        );
#endif
        return  MFALSE;
    }
    FInfoIF*const pFInfo_Scene = GetFInfo_Scene();
    if  ( pFInfo_Scene )
    {
        return  pFInfo_Scene->SetDefault(sid)
            &&  m_rCtrl.SetSceneMode(
                    static_cast<Fid2Type<FID_SCENE_MODE>::Type>(sid)
                );
    }
    return  MFALSE;
}


void
FeatureHalBridge::
DumpScenesInfo()
{
#if ENABLE_MY_LOG
    SID_T const* pSid = SceneBegin();
    SID_T const* const pSEnd = SceneEnd();
    SID_T sid = 0;

    MY_LOG("[+DumpScenesInfo](pSid,pSEnd)=(%p,%p)", pSid, pSEnd);
#if 0
    for (; pSEnd != pSid; pSid++)
    {
        sid = *pSid;
        MY_LOG("[DumpScenesInfo]:In-Table Scene(%d)", sid);
    }
#endif
    MUINT32 u4MaskSceneSupport = 0;
    for (sid = SCENE_MODE_BEGIN; sid < Fid2Type<FID_SCENE_MODE>::Num; sid++)
    {
        u4MaskSceneSupport |= (IsSceneSupport(sid) << sid);    
    }
    MY_LOG("[-DumpScenesInfo]:u4MaskSceneSupport:0x%08X", u4MaskSceneSupport);
#endif  //  ENABLE_MY_LOG
}

