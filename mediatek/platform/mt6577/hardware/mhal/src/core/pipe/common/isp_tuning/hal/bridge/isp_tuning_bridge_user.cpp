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
#define LOG_TAG "NSIspTuning::Bridge_user"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG           (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
#define USE_CUSTOM_ISP_TUNING
#include "isp_tuning.h"
#include "paramctrl_if.h"
#include "isp_tuning_hal.h"
#include "isp_tuning_bridge.h"
//
using namespace NSIspTuning;


MINT32
IspTuningBridge::
setIspUserIdx_Edge(MUINT32 const u4Index)
{
    switch (u4Index)
    {
    case ISP_EDGE_LOW:
    case ISP_EDGE_MIDDLE:
    case ISP_EDGE_HIGH:
        break;
    default:
        MY_LOG("[Error][setIspUserIdx_Edge] bad index: %d", u4Index);
        return  MERR_BAD_PARAM;
    }
    return  m_pParamctrl->setIspUserIdx_Edge(
                static_cast<EIndex_Isp_Edge_T >(u4Index)
            );
}


MINT32
IspTuningBridge::
setIspUserIdx_Hue(MUINT32 const u4Index)
{
    switch (u4Index)
    {
    case ISP_HUE_LOW:
    case ISP_HUE_MIDDLE:
    case ISP_HUE_HIGH:
        break;
    default:
        MY_LOG("[Error][setIspUserIdx_Hue] bad index: %d", u4Index);
        return  MERR_BAD_PARAM;
    }
    return  m_pParamctrl->setIspUserIdx_Hue(
                static_cast<EIndex_Isp_Hue_T >(u4Index)
            );
}


MINT32
IspTuningBridge::
setIspUserIdx_Sat(MUINT32 const u4Index)
{
    switch (u4Index)
    {
    case ISP_SAT_LOW:
    case ISP_SAT_MIDDLE:
    case ISP_SAT_HIGH:
        break;
    default:
        MY_LOG("[Error][setIspUserIdx_Sat] bad index: %d", u4Index);
        return  MERR_BAD_PARAM;
    }
    return  m_pParamctrl->setIspUserIdx_Sat(
                static_cast<EIndex_Isp_Saturation_T >(u4Index)
            );
}


MINT32
IspTuningBridge::
setIspUserIdx_Bright(MUINT32 const u4Index)
{
    switch (u4Index)
    {
    case ISP_BRIGHT_LOW:
    case ISP_BRIGHT_MIDDLE:
    case ISP_BRIGHT_HIGH:
        break;
    default:
        MY_LOG("[Error][setIspUserIdx_Bright] bad index: %d", u4Index);
        return  MERR_BAD_PARAM;
    }
    return  m_pParamctrl->setIspUserIdx_Bright(
                static_cast<EIndex_Isp_Brightness_T >(u4Index)
            );
}


MINT32
IspTuningBridge::
setIspUserIdx_Contrast(MUINT32 const u4Index)
{
    switch (u4Index)
    {
    case ISP_CONTRAST_LOW:
    case ISP_CONTRAST_MIDDLE:
    case ISP_CONTRAST_HIGH:
        break;
    default:
        MY_LOG("[Error][setIspUserIdx_Contrast] bad index: %d", u4Index);
        return  MERR_BAD_PARAM;
    }
    return  m_pParamctrl->setIspUserIdx_Contrast(
                static_cast<EIndex_Isp_Contrast_T >(u4Index)
            );
}

