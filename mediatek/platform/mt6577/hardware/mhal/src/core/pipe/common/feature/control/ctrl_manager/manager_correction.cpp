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
#include <sys/sysconf.h>
//
#include "debug.h"
//
#include "camera_feature.h"
#include "feature_hal.h"
#include "feature_hal_bridge.h"
#include "control_base.h"
#include "ctrl_manager.h"
#include "functor.h"

using namespace NSFeature;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation of class CtrlManager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace
{


//  AF mode must be supported at least one item.
struct FUNC_AF_Mode
{
    enum { FID = FID_AF_MODE };
    MBOOL operator()(
        FInfoIF* const pFInfo, FID_T const fid, SID_T const sid = -1
    ) const
    {
        if  ( ! pFInfo )
            return  MFALSE;
        if  ( 0 == pFInfo->GetCount() )
        {   //  If empty, set just one item as the default.
            pFInfo->SetToOne(AF_MODE_INFINITY);
#if ENABLE_MY_LOG
            MY_LOG("<correct>[AF_MODE][sid:%d] INFINITY\n", sid);
#endif
        }
        return  MTRUE;
    }
};

//  Not support Auto Flicker.
struct FUNC_Not_Support_AutoFlicker
{
    enum { FID = FID_AE_FLICKER };
    MBOOL operator()(
        FInfoIF* const pFInfo, FID_T const fid, SID_T const sid = -1
    ) const
    {
        if  ( ! pFInfo )
            return  MFALSE;
        if  ( pFInfo->Remove(AE_FLICKER_MODE_AUTO) )
        {
#if ENABLE_MY_LOG
            MY_WARN("<correct> Force to remove AE_FLICKER_MODE_AUTO");
#endif
        }
        return  MTRUE;
    }
};


//  Not support HDR capture mode.
struct FUNC_Not_Support_HDR
{
    enum { FID = FID_SCENE_MODE };
    MBOOL operator()(
        FInfoIF* const pFInfo, FID_T const fid, SID_T const sid = -1
    ) const
    {
        if  ( ! pFInfo )
            return  MFALSE;
        if  ( pFInfo->Remove(SCENE_MODE_HDR) )
        {
#if ENABLE_MY_LOG
            MY_WARN("<correct> Force to remove SCENE_MODE_HDR");
#endif
        }
        return  MTRUE;
    }
};


//  Not support EIS.
struct FUNC_Not_Support_EIS
{
    enum { FID = FID_EIS };
    MBOOL operator()(
        FInfoIF* const pFInfo, FID_T const fid, SID_T const sid = -1
    ) const
    {
        if  ( ! pFInfo )
            return  MFALSE;
        if  ( pFInfo->ClearAll() )
        {
#if ENABLE_MY_LOG
            MY_WARN("<correct> Force to disable FID_EIS");
#endif
        }
        return  MTRUE;
    }
};


};  //  namespace


MBOOL
CtrlManager::
InitFeatures_NoWarningCorrection(FeatureHalBridge& rFHBridge)
{
//  Correct without warning

    MBOOL fgRet = MFALSE;

    int const memtotal_in_byte = ::sysconf(_SC_PHYS_PAGES) * getpagesize();
    bool const fgIsLowMem = (memtotal_in_byte <= 256 * 1024 *1024);  //  low memory if < 256MB (2Gb)

    MY_LOG(
        "[InitFeatures_NoWarningCorrection] (fgIsLowMem, memtotal_in_byte, getpagesize)=(%d, %d, %d)"
        , fgIsLowMem, memtotal_in_byte, getpagesize()
    );

    if  ( fgIsLowMem )
    {
        //  Force to disable AutoFlicker.
        if  ( ! Functor_OneFID_TgtDo<FUNC_Not_Support_AutoFlicker>()(rFHBridge) )
        {
            goto lbExit;
        }
        
        //  Force to disable HDR.
        if  ( ! Functor_OneFID_TgtDo<FUNC_Not_Support_HDR>()(rFHBridge) )
        {
            goto lbExit;
        }

        //  Force to disable EIS.
        if  ( ! Functor_OneFID_TgtDo<FUNC_Not_Support_EIS>()(rFHBridge) )
        {
            goto lbExit;
        }
    }


    //  AF mode must be supported at least one item.
    if  ( ! Functor_OneFID_TgtDo<FUNC_AF_Mode>()(rFHBridge) )
    {
        goto lbExit;
    }


    fgRet = MTRUE;
lbExit:
    return  fgRet;
}

