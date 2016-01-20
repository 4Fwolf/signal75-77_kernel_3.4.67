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
#define LOG_TAG "CtrlManager_Misc"
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
//  Local Define
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define TOTAL_TABLE_SCENE_NUM   static_cast<MUINT32>(NSFeature::ENumOfScene)


/*******************************************************************************
* MACRO Define: Scene Independent
*******************************************************************************/
#define GETFINFO_SCENE_INDEP()          _GETFINFO_SCENE_INDEP("[Misc]")
#define END_GETFINFO_SCENE_INDEP()  _END_GETFINFO_SCENE_INDEP("[Misc]")


/*******************************************************************************
* MACRO Define: Scene Dependent
*******************************************************************************/
#define GETFINFO_SCENE_DEP()            _GETFINFO_SCENE_DEP("[Misc]")
#define END_GETFINFO_SCENE_DEP()    _END_GETFINFO_SCENE_DEP("[Misc]")


/*******************************************************************************
* MACRO Define: Config Scene
*******************************************************************************/
#define CONFIG_SCENE(_sid)              _CONFIG_SCENE(_sid, "[Misc]")
#define END_CONFIG_SCENE()          _END_CONFIG_SCENE("[Misc]")


/*******************************************************************************
* MACRO Define: Config Feature
*******************************************************************************/
#define CHECK_FID_SI    CHECK_FID_MISC_SI
#define CHECK_FID_SD    CHECK_FID_MISC_SD

#include "cfg_ftbl_misc.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation of class CtrlManager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FInfoIF*
CtrlManager::
GetFInfo_Misc(FID_T const fid)
{
    return  NSFeature::NSMisc::NSSceneIndep::GetFInfo(fid);
}


FInfoIF*
CtrlManager::
GetFInfo_Misc(FID_T const fid, SID_T const sid)
{
    return  NSFeature::NSMisc::NSSceneDep::
            GetFInfo<TOTAL_TABLE_SCENE_NUM>(fid, sid);
}


MBOOL
CtrlManager::
InitFeatures_Misc(FeatureHalBridge& rFHBridge)
{
    MBOOL fgRet = MFALSE;


    //  Scene-Indep. Misc Features.
#if ENABLE_MY_LOG
    MY_LOG("[InitFeatures_Misc][SI]");
#endif
    fgRet = InitFeatures_SI <
        FID_BEGIN_MISC_SI, FID_END_MISC_SI, PF_GETFINFO_MISC_SI_T
    >(rFHBridge, GetFInfo_Misc);
    if  ( ! fgRet )
        goto lbExit;


    //  Scene-Dep. Misc Features.
#if ENABLE_MY_LOG
    MY_LOG("[InitFeatures_Misc][SD]");
#endif
    fgRet = InitFeatures_SD <
        FID_BEGIN_MISC_SD, FID_END_MISC_SD, PF_GETFINFO_MISC_SD_T
    >(rFHBridge, GetFInfo_Misc);
    if  ( ! fgRet )
        goto lbExit;


    fgRet = MTRUE;
lbExit:
    return  fgRet;
}

