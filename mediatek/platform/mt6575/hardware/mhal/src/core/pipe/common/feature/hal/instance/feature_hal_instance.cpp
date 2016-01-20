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
#define LOG_TAG "FeatureHalInstance"
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
//
#include "camera_feature.h"
#include "feature_hal.h"
#include "feature_hal_bridge.h"
#include "feature_hal_template.h"
#include "control_base.h"
#include "ctrl_manager.h"
#include "cfg_ftbl_target.h"

using namespace NSFeature;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Local Define
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define TOTAL_TABLE_SCENE_NUM       ENumOfScene
#define POINTER_TO_CONTROL_CONTEXT  (CtrlMgrSingleton::getInstance())

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantization of class ControlManager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CtrlMgrSingleton : public CtrlManager
{
public:
    static ControlBase* getInstance()
    {
        static CtrlMgrSingleton singleton;
        return  &singleton;
    }
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation of class template FeatureHalTemplate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ECamRole_T _RoleId, MUINT32 _SceneNum>
MBOOL
FeatureHalTemplate<_RoleId, _SceneNum>::
sm_afgSceneSupport[_SceneNum] = {MFALSE};


template <ECamRole_T _RoleId, MUINT32 _SceneNum>
FeatureHalTemplate<_RoleId, _SceneNum>::
FeatureHalTemplate()
    : FeatureHalBridge(POINTER_TO_CONTROL_CONTEXT)
    , m_pFInfo_Scene(GetFInfo(FID_SCENE_MODE))
{
    MY_ASSERT(NULL != m_pFInfo_Scene, "m_pFInfo_Scene == NULL")    
}


template <ECamRole_T _RoleId, MUINT32 _SceneNum>
FeatureHal*
FeatureHalTemplate<_RoleId, _SceneNum>::
getInstance()
{
    static  FeatureHalTemplate<_RoleId, _SceneNum> singleton;
    static  MBOOL fgInitialized = MFALSE;
    if  ( fgInitialized || (fgInitialized = singleton.Init()) )
        return  &singleton;
    return  NULL;
}


template <ECamRole_T _RoleId, MUINT32 _SceneNum>
void
FeatureHalTemplate<_RoleId, _SceneNum>::
destroyInstance()
{
}


template <ECamRole_T _RoleId, MUINT32 _SceneNum>
FInfoIF*
FeatureHalTemplate<_RoleId, _SceneNum>::
GetFInfo(FID_T const fid)
{
    return  NSFeature::NSTarget::NSSceneIndep::
            GetFInfo<_RoleId>(fid);
}


template <ECamRole_T _RoleId, MUINT32 _SceneNum>
FInfoIF*
FeatureHalTemplate<_RoleId, _SceneNum>::
GetFInfo(FID_T const fid, SID_T const sid)
{
    return  NSFeature::NSTarget::NSSceneDep::
            GetFInfo<_RoleId, _SceneNum>(fid, sid);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantization of class FeatureHal
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define INSTANTIATE_CAMERA(_RoleId) \
    case _RoleId: \
    { \
        typedef FeatureHalTemplate<_RoleId, TOTAL_TABLE_SCENE_NUM> type; \
        return type::getInstance(); \
    }


FeatureHal*
FeatureHal::
createInstance(MUINT32 const u4CameraRoleId)
{
    switch  (u4CameraRoleId)
    {
    INSTANTIATE_CAMERA(ECamRole_Main)
    INSTANTIATE_CAMERA(ECamRole_Sub)
    INSTANTIATE_CAMERA(ECamRole_N3D_Main)
    INSTANTIATE_CAMERA(ECamRole_B3D_Main)    
    default:
        MY_ERR("createInstance(%d)", u4CameraRoleId);
    }
    return  NULL;
}

