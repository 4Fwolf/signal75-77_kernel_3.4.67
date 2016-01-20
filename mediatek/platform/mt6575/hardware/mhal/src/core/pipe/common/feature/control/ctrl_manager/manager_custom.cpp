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
#define LOG_TAG "CtrlManager_Custom"
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

using namespace NSFeature;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Local Define
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef ENABLE_CUSTOM_CONFLICT_TEST
    #define ENABLE_CUSTOM_CONFLICT_TEST (1)
#endif

#define CUSTOM_CONFLICT_TEST(express, fmt, msg...)  \
    if  ( ! (express) )                             \
    {                                               \
        MY_ERR("[Custom Conflict] "fmt, msg);       \
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation of class CtrlManager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
CtrlManager::
TryOverwriteTargetFeatureByCustom(
        FInfoIF*const pFInfo_Tgt, 
        FInfoIF*const pFInfo_Src    //  Pointer to custom feature.
    )
{
#if ENABLE_MY_LOG || ENABLE_CUSTOM_CONFLICT_TEST
    FID_T fid = pFInfo_Tgt->GetFID();
#endif

    //  Custom
    FInfoIF::Feature_T const
    fDefault_Src= pFInfo_Src->GetDefault();

    FInfoIF::Feature_T const*const
    pTable_Src  = pFInfo_Src->GetTable();

    MUINT32 const u4Count_Src = pFInfo_Src->GetCount();

#if ENABLE_MY_LOG
    //  Before copying, log the default and all the table of Custom.
    MY_LOG("[FID:%d][Before][Target <- Custom]Custom:\n", fid);
    pFInfo_Src->Dump();
#endif

#if ENABLE_CUSTOM_CONFLICT_TEST
    //  Is the Custom Default Value within Custom Table?
    CUSTOM_CONFLICT_TEST(
        pFInfo_Src->IsWithin( fDefault_Src ), 
        "[FID:%d]Custom Default(%d) is out of Custom Table"
        , fid, fDefault_Src
    );

    //  Is the Custom Default Value within Target Table?
    CUSTOM_CONFLICT_TEST(
        pFInfo_Tgt->IsWithin( fDefault_Src ), 
        "[FID:%d]Custom Default(%d) is out of Target Table"
        , fid, fDefault_Src
    );

    //  Are all items in Custom Table within Target Table?
    if  ( 0 < u4Count_Src )
    {
        //  FIXME: optimize if possible.
        for (MUINT32 i = 0; i < u4Count_Src; i++)
        {
            CUSTOM_CONFLICT_TEST(
                pFInfo_Tgt->IsWithin( pTable_Src[i] ), 
                "[FID:%d]The %d-th Custom Item (%d) is out of Target Table"
                , fid, i, pTable_Src[i]
            );
        }
    }
#endif  //  ENABLE_CUSTOM_CONFLICT_TEST

    //  Copy. (Target <- Custom)
    if  ( ! pFInfo_Tgt->Copy(pFInfo_Src) )
    {
        MY_LOG("[Target <- Custom] Fail to Copy: (fid)=(%d)\n", fid);
        return  MFALSE;
    }

#if ENABLE_MY_LOG
    //  After copying, log the default and all the table of Target.
    MY_LOG("[FID:%d][After][Target <- Custom]Target:\n", fid);
    pFInfo_Tgt->Dump();
#endif

    return  MTRUE;
}


MBOOL
CtrlManager::
InitFeatures_Custom(FeatureHalBridge& rFHBridge)
{
    MBOOL fgRet = MFALSE;
    FID_T fid = 0;

    // target feature
    FInfoIF* pFInfo_Tgt = NULL;
    // source feature (misc/raw/yuv/custom)
    FInfoIF* pFInfo_Src = NULL;

    //  Pointer to Function for Getting Custom Features.
    PF_GETFINFO_CUSTOM_T pfGetFInfo_Custom = NULL;

    //  Get Feature Provider of Current Custom Features.
    fgRet = GetFeatureProvider_Custom(pfGetFInfo_Custom);
    if  ( ! fgRet )
    {
        MY_LOG( "GetFeatureProvider_Custom() return FALSE "
                "(pfGetFInfo_Custom)=(%p)\n", 
                pfGetFInfo_Custom);
        goto lbExit;
    }

    //  Custom conflict test (assert message)
    //  Scene-Indep. Custom Features.
    for (fid = FID_BEGIN_SI; fid < FID_END_SI; fid++)
    {
        //  Custom feature.
        pFInfo_Src = pfGetFInfo_Custom(fid);
        if  ( ! pFInfo_Src )
            continue;
        //  Target feature.
        pFInfo_Tgt = rFHBridge.GetFInfo(fid);
        CUSTOM_CONFLICT_TEST(
            NULL != pFInfo_Tgt, 
            "customer specifies an unsupported fid(%d)", fid
        );

		if (NULL == pFInfo_Tgt)
			continue;
		
        //  Here both pFInfo_Src & pFInfo_Tgt are not NULL.
        fgRet = TryOverwriteTargetFeatureByCustom(pFInfo_Tgt, pFInfo_Src);
        if  ( ! fgRet )
            goto lbExit;
    }


    //  Here custom-specific scene-mode feature must be given.
    //  Now we can set up Scene-Support flags.
    rFHBridge.UpdateScenesInfo();


    //  Custom conflict test (assert message)
    //  For Scene-Dep. features, only Auto Scene can be customized.
    for (fid = FID_BEGIN_SD; fid < FID_END_SD; fid++)
    {
        //  Custom feature.
        pFInfo_Src = pfGetFInfo_Custom(fid);
        if  ( ! pFInfo_Src )
            continue;
        //  Target feature.
        pFInfo_Tgt = rFHBridge.GetFInfo(fid, SCENE_MODE_OFF);
        CUSTOM_CONFLICT_TEST(
            NULL != pFInfo_Tgt, 
            "customer specifies an unsupported fid(%d) in Auto Scene", fid
        );

		if (NULL == pFInfo_Tgt)
			continue;
			
        //  Here both pFInfo_Src & pFInfo_Tgt are not NULL.
        fgRet = TryOverwriteTargetFeatureByCustom(pFInfo_Tgt, pFInfo_Src);
        if  ( ! fgRet )
            goto lbExit;
    }

    fgRet = MTRUE;

lbExit:
    return  fgRet;
}

