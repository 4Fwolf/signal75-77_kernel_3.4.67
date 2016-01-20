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
#include "control_base.h"
#include "ctrl_manager.h"
#include "feature_hal.h"
#include "feature_hal_bridge.h"
#include "functor.h"

using namespace NSFeature;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation of class CtrlManager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
CtrlManager::
InitFeatures_YUV(FeatureHalBridge& rFHBridge)
{
    MBOOL fgRet = MFALSE;

    //  Pointer to Function for Getting Scene-Independent YUV Features.
    PF_GETFINFO_YUV_SI_T pfGetFInfo_YUV_SI = NULL;
    //  Pointer to Function for Getting Scene-Dependent YUV Features.
    PF_GETFINFO_YUV_SD_T pfGetFInfo_YUV_SD = NULL;

    //  Get Feature Provider of Current YUV Sensor.
    fgRet = GetFeatureProvider_YUV(pfGetFInfo_YUV_SI, pfGetFInfo_YUV_SD);
    if  ( ! fgRet )
    {
        MY_LOG( "GetFeatureProvider_YUV() return FALSE "
                "(pfGetFInfo_YUV_SI, pfGetFInfo_YUV_SD)=(%p, %p)\n", 
                pfGetFInfo_YUV_SI, pfGetFInfo_YUV_SD);
        goto lbExit;
    }


    //  Scene-Indep. YUV Features.
#if ENABLE_MY_LOG
    MY_LOG("[InitFeatures_YUV][SI]");
#endif
    fgRet = InitFeatures_SI <
        FID_BEGIN_YUV_SI, FID_END_YUV_SI, PF_GETFINFO_YUV_SI_T
    >(rFHBridge, pfGetFInfo_YUV_SI);
    if  ( ! fgRet )
        goto lbExit;


    //  Here scene-mode feature must be given.
    //  Now we can set up Scene-Support flags.
    rFHBridge.UpdateScenesInfo();


    //  Scene-Dep. YUV Features.
#if ENABLE_MY_LOG
    MY_LOG("[InitFeatures_YUV][SD]");
#endif
    fgRet = InitFeatures_SD <
        FID_BEGIN_YUV_SD, FID_END_YUV_SD, PF_GETFINFO_YUV_SD_T
    >(rFHBridge, pfGetFInfo_YUV_SD);
    if  ( ! fgRet )
        goto lbExit;


    fgRet = MTRUE;

lbExit:
    return  fgRet;
}

