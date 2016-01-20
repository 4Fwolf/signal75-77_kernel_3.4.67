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
#define LOG_TAG "NSIspTuning::ParamctrlRAW"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (0)
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


MERROR_ENUM
ParamctrlRAW::
do_validatePerFrame()
{
    MERROR_ENUM err = MERR_UNKNOWN;

    MY_LOG("[+do_validatePerFrame]");


    //  (1) dynamic tuning
    if  ( isDynamicTuning()    //  Dynamic Tuning: Enable
       && (m_IspCamInfo.eCamMode != ECamMode_YUV2JPG_Scalado)
       && (m_IspCamInfo.eCamMode != ECamMode_FB_PostProcess_NR2_Only)
       && (m_IspCamInfo.eCamMode != ECamMode_FB_PostProcess_PCA_Only)) // bypass getting default index setting
    {
        IndexMgr idxmgr;

        //  a) Get default index setting.
        INDEX_T const*const pDefaultIndex = m_pIspTuningCustom->getDefaultIndex(
            m_IspCamInfo.eCamMode, m_IspCamInfo.eIdx_Scene, m_IspCamInfo.eIdx_ISO
        );
        if  ( ! pDefaultIndex )
        {
            MY_LOG("[ERROR][validatePerFrame]pDefaultIndex==NULL");
            err = MERR_CUSTOM_DEFAULT_INDEX_NOT_FOUND;
            goto lbExit;
        }
        idxmgr = *pDefaultIndex;

#if ENABLE_MY_LOG
        MY_LOG("[BEFORE][evaluate_nvram_index]");
        idxmgr.dump();
#endif

        //  b) Customize the index setting.
        m_pIspTuningCustom->evaluate_nvram_index(m_IspCamInfo, idxmgr);

#if ENABLE_MY_LOG
        MY_LOG("[AFTER][evaluate_nvram_index]");
        idxmgr.dump();
#endif

        //  c) Restore customized index set to member.
        m_IspRegMgr = idxmgr;
    }


    //  (2) Apply Per-Frame Parameters.
    if  (
            ! prepareHw_PerFrame_IspUserIndex() //  Will change param members.
        ||  ! prepareHw_PerFrame_All()          //  Prepare param members to the ispmgr's buffer.
        ||  ! prepareHw_PerFrame_ColorEffect()  //  Change the ispmgr's buffer directly.
        ||  ! applyToHw_PerFrame_All()          //  Apply the ispmgr's buffer to H/W.
        )
    {
        err = MERR_SET_ISP_REG;
        goto lbExit;
    }


    //  (3) Save Exif debug info if necessary.
    err = saveDebugInfo();
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }


    err = MERR_OK;

lbExit:
#if ENABLE_MY_ERR
    if  ( MERR_OK != err )
    {
        MY_ERR("[-do_validatePerFrame]err(%X)", err);
    }
#endif
    MY_LOG("[-do_validatePerFrame]err(%X)", err);
    return  err;
}

