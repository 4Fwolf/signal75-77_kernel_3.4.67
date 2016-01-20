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
#define LOG_TAG "feature_RAW"
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#define USE_CAMERA_FEATURE_MACRO 1  //define before "camera_feature.h"
#include "camera_feature.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Local Define
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define TOTAL_TABLE_SCENE_NUM   static_cast<MUINT32>(NSFeature::ENumOfScene)


/*******************************************************************************
* MACRO Define: Scene Independent
*******************************************************************************/
#define GETFINFO_SCENE_INDEP()          _GETFINFO_SCENE_INDEP("[RAW]")
#define END_GETFINFO_SCENE_INDEP()  _END_GETFINFO_SCENE_INDEP("[RAW]")


/*******************************************************************************
* MACRO Define: Scene Dependent
*******************************************************************************/
#define GETFINFO_SCENE_DEP()            _GETFINFO_SCENE_DEP("[RAW]")
#define END_GETFINFO_SCENE_DEP()    _END_GETFINFO_SCENE_DEP("[RAW]")


/*******************************************************************************
* MACRO Define: Config Scene
*******************************************************************************/
#define CONFIG_SCENE(_sid)              _CONFIG_SCENE(_sid, "[RAW]")
#define END_CONFIG_SCENE()          _END_CONFIG_SCENE("[RAW]")


/*******************************************************************************
* MACRO Define: Config Feature
*******************************************************************************/
#define CHECK_FID_SI    CHECK_FID_RAW_SI
#define CHECK_FID_SD    CHECK_FID_RAW_SD


/*******************************************************************************
* Implementation of Feature Tables
*******************************************************************************/
#include "cfg_ftbl_raw_sceneindep.h"
static
inline
NSFeature::PF_GETFINFO_SCENE_INDEP_T
GetFInfo_RAW_SI()
{
    return  NSFeature::NSRAW::NSSceneIndep::GetFInfo;
}


#include "cfg_ftbl_raw_scenedep.h"
static
inline
NSFeature::PF_GETFINFO_SCENE_DEP_T
GetFInfo_RAW_SD()
{
    return  NSFeature::NSRAW::NSSceneDep::GetFInfo<TOTAL_TABLE_SCENE_NUM>;
}


/*******************************************************************************
* Implementation of class IspDrv
*******************************************************************************/
#include "isp_drv.h"
int
IspDrv::
getFeatureProvider(void*const pOutBuf)
{
    NSFeature::FeatureInfoProvider*const
    pFInfoProvider = reinterpret_cast<NSFeature::FeatureInfoProvider*>(pOutBuf);
    if (!pFInfoProvider) {
        return - 1;
    }

    pFInfoProvider->pfGetFInfo_SceneIndep = GetFInfo_RAW_SI();
    pFInfoProvider->pfGetFInfo_SceneDep   = GetFInfo_RAW_SD();

    return 0;
}

