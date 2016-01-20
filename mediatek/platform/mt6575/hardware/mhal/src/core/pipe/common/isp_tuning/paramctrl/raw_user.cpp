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
#define LOG_TAG "NSIspTuning::ParamctrlRAW_user"
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
using namespace NSIspTuning;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
ParamctrlRAW::
prepareHw_PerFrame_IspUserIndex()
{
    MBOOL fgRet = MFALSE;

    //  (0) Invoked only when Normal Operation Mode.
    if  ( EOperMode_Normal != getOperMode() )
    {
        fgRet = MTRUE;
        goto lbExit;
    }


    //  Hue
    {
        //  (a) Customize the nvram index based on the user setting.
        MUINT8 const u8Idx_Hue = m_pIspTuningCustom->
            map_user_setting_to_nvram_index<ISP_NVRAM_HUE_T>(
                m_IspRegMgr.getIdx_Hue(),   // The current nvram index.
                getIspUsrSelectLevel()      // Get the user setting.
            );
        //  (b) Overwrite the params member.
        fgRet = m_IspRegMgr.setIdx_Hue(u8Idx_Hue);
        if  ( ! fgRet )
        {
            MY_ERR(
                "[ERROR][prepareHw_PerFrame_IspUserIndex]"
                "setIdx_Hue: bad idx(%d)", u8Idx_Hue
            );
            goto lbExit;
        }
    }


    //  Contrast + Brightness
    {
        //  (a) Customize the nvram index based on the user setting.
        MUINT8 const u8Idx_Contrast = m_pIspTuningCustom->
            map_user_setting_to_nvram_index<ISP_NVRAM_CONTRAST_T>(
                m_IspRegMgr.getIdx_Contrast(),  // The current nvram index.
                getIspUsrSelectLevel()          // Get the user setting.
            );
        //  (b) Overwrite the params member.
        fgRet = m_IspRegMgr.setIdx_Contrast(u8Idx_Contrast);
        if  ( ! fgRet )
        {
            MY_ERR(
                "[ERROR][prepareHw_PerFrame_IspUserIndex]"
                "setIdx_Contrast: bad idx(%d)", u8Idx_Contrast
            );
            goto lbExit;
        }
    }


    //  Sharpness
    {
        //  (a) Customize the nvram index based on the user setting.
        MUINT8 const u8Idx_EE = m_pIspTuningCustom->
            map_user_setting_to_nvram_index<ISP_NVRAM_EE_T>(
                m_IspRegMgr.getIdx_EE(),    // The current nvram index.
                getIspUsrSelectLevel()      // Get the user setting.
            );
        //  (b) Overwrite the params member.
        fgRet = m_IspRegMgr.setIdx_EE(u8Idx_EE);
        if  ( ! fgRet )
        {
            MY_ERR(
                "[ERROR][prepareHw_PerFrame_IspUserIndex]"
                "setIdx_EE: bad idx(%d)", u8Idx_EE
            );
            goto lbExit;
        }
    }


    //  Saturation
    {
        //  (a) Customize the nvram index based on the user setting.
        MUINT8 const u8Idx_Sat = m_pIspTuningCustom->
            map_user_setting_to_nvram_index<ISP_NVRAM_SATURATION_T>(
                m_IspRegMgr.getIdx_Saturation(),// The current nvram index.
                getIspUsrSelectLevel()          // Get the user setting.
            );
        //  (b) Overwrite the params member.
        fgRet = m_IspRegMgr.setIdx_Saturation(u8Idx_Sat);
        if  ( ! fgRet )
        {
            MY_ERR(
                "[ERROR][prepareHw_PerFrame_IspUserIndex]"
                "setIdx_Saturation: bad idx(%d)", u8Idx_Sat
            );
            goto lbExit;
        }
    }


    MY_LOG(
        "[prepareHw_PerFrame_IspUserIndex](sat,contrast,hue,ee)=(%d,%d,%d,%d)"
        , m_IspRegMgr.getIdx_Saturation()
        , m_IspRegMgr.getIdx_Contrast()
        , m_IspRegMgr.getIdx_Hue()
        , m_IspRegMgr.getIdx_EE()
    );


    fgRet = MTRUE;

lbExit:
    return  fgRet;
}

