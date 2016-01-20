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
#define LOG_TAG "NSIspTuning::ParamctrlRAW_effect"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG   (0)
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


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <EIndex_Effect_T eEffect>
MVOID
ParamctrlRAW::
prepareEffect()
{
    MY_LOG("[+prepareEffect]:%d", eEffect);

    ISP_EFFECT_T IspEffect;

    getIspHWBuf(IspEffect.yccgo);
    getIspHWBuf(IspEffect.edge);
    getIspHWBuf(IspEffect.ccm);

    m_pIspTuningCustom->prepare_effect<eEffect>(IspEffect);

    putIspHWBuf(IspEffect.yccgo);
    putIspHWBuf(IspEffect.edge);
    putIspHWBuf(IspEffect.ccm);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
ParamctrlRAW::
prepareHw_PerFrame_ColorEffect()
{
    MBOOL fgRet = MFALSE;

    //  (0) Invoked only when Normal Operation Mode
    if  ((EOperMode_Normal != getOperMode())
      || (m_IspCamInfo.eCamMode == ECamMode_YUV2JPG_Scalado)
      || (m_IspCamInfo.eCamMode == ECamMode_YUV2JPG_ZSD)
      || (m_IspCamInfo.eCamMode == ECamMode_FB_PostProcess_NR2_Only)
      || (m_IspCamInfo.eCamMode == ECamMode_FB_PostProcess_PCA_Only))
    {
        fgRet = MTRUE;
        goto lbExit;
    }

    switch  ( getEffect() )
    {
    case MEFFECT_MONO:
        prepareEffect<MEFFECT_MONO>();
        break;
    case MEFFECT_SEPIA:
        prepareEffect<MEFFECT_SEPIA>();
        break;
    case MEFFECT_AQUA:
        prepareEffect<MEFFECT_AQUA>();
        break;
    case MEFFECT_NEGATIVE:
        prepareEffect<MEFFECT_NEGATIVE>();
        break;
    case MEFFECT_SOLARIZE:
        prepareEffect<MEFFECT_SOLARIZE>();
        break;
    case MEFFECT_POSTERIZE:
        prepareEffect<MEFFECT_POSTERIZE>();
        break;
    case MEFFECT_BLACKBOARD:
        prepareEffect<MEFFECT_BLACKBOARD>();
        break;
    case MEFFECT_WHITEBOARD:
        prepareEffect<MEFFECT_WHITEBOARD>();
        break;
    case MEFFECT_OFF:           //  Do nothing.
    case MEFFECT_SEPIAGREEN:    //  Unsupport.
    case MEFFECT_SEPIABLUE:     //  Unsupport.
    default:
        break;
    }

    fgRet = MTRUE;

lbExit:
    return  fgRet;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
ParamctrlRAW::
setEffect(EIndex_Effect_T const eEffect)
{
    switch  ( eEffect )
    {
    case MEFFECT_SEPIAGREEN:    //  Unsupport.
    case MEFFECT_SEPIABLUE:     //  Unsupport.
        return  MERR_UNSUPPORT;
    default:
        break;
    }

    return  ParamctrlComm::setEffect(eEffect);
}

