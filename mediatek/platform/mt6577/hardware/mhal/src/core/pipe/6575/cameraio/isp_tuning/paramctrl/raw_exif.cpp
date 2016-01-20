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
#define LOG_TAG "NSIspTuning::ParamctrlRAW_exif"
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
using namespace NSIspExifDebug;


namespace
{


//  Tag <- Isp Index.
inline
MVOID
setIspIdx(
    IspDebugTag (&rTags)[TagID_Total_Num], 
    IspDebugTagID const eTagID, 
    MUINT32 const u4Idx
)
{
    rTags[eTagID].u4ID  = getIspTag(eTagID);
    rTags[eTagID].u4Val = u4Idx;
}


//  Tag <- Isp Regs.
template <class ISP_xxx_T, MUINT32 TagID_begin>
inline
MVOID
setIspTags(
    IspDebugTag (&rTags)[TagID_Total_Num]
)
{
    enum { E_NUM = ISP_xxx_T::COUNT };
    ISP_xxx_T param;
    getIspHWBuf(param);
    for (MUINT32 i = 0; i < E_NUM; i++)
    {
        MUINT32 const u4TagID = TagID_begin + i;
        rTags[u4TagID].u4ID   = getIspTag(u4TagID);
        rTags[u4TagID].u4Val  = param.set[i];
        MY_LOG("[%d]=0x%08X", i, rTags[u4TagID].u4Val);
    }
    STATIC_CHECK(
        TagID_begin+E_NUM-1 < TagID_Total_Num, 
        tag_index_over_total_num
    );
}


};


/*******************************************************************************
* 
*******************************************************************************/
MERROR_ENUM
ParamctrlRAW::
saveDebugInfo()
{
    IspDebugTag (&rTags)[TagID_Total_Num] = m_stIspDebugInfo.tags;

    switch  ( m_IspCamInfo.eCamMode )
    {
    case ECamMode_Offline_Capture_Pass1://  NORMAL
    case ECamMode_HDR_Cap_Pass1_SF:     //  HDR Pass1: Single Frame
    case ECamMode_HDR_Cap_Pass1_MF1:    //  HDR Pass1: Multi Frame Stage1
    case ECamMode_Online_Capture_ZSD:   //  ZSD Pass1
        //  Preprocessing Capture.
        break;
    default:
        goto lbExit;
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //  Here, we are in preprocessing capture.
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //
    ////////////////////////////////////////////////////////////////////////////
    //  ISPRegs
    ////////////////////////////////////////////////////////////////////////////
    //
    //  Preprocess Gain Control
    MY_LOG("Preprocess gain control:");
    setIspTags<ISP_NVRAM_GAIN_CTRL_T, Preproc__GainCtrl_Begin>(rTags);
    //  Preprocess Shading
    MY_LOG("Preprocess shading:");
    setIspIdx(rTags, Preproc__IDX_SHADING, m_IspRegMgr.getIdx_Shading());
    setIspTags<ISP_NVRAM_SHADING_T, Preproc__SHADING_Begin>(rTags);
    //
    //  Preprocess OB
    MY_LOG("Preprocess ob:");
    setIspIdx(rTags, Preproc__IDX_OB, m_IspRegMgr.getIdx_OB());
    setIspTags<ISP_NVRAM_OB_T, Preproc__OB_Begin>(rTags);
    //
    //  Preprocess DP
    MY_LOG("Preprocess dp:");
    setIspIdx(rTags, Preproc__IDX_DP, m_IspRegMgr.getIdx_DP());
    setIspTags<ISP_NVRAM_DP_T, Preproc__DP_Begin>(rTags);
    //
    //  Preprocess NR1
    MY_LOG("Preprocess nr1:");
    setIspIdx(rTags, Preproc__IDX_NR1, m_IspRegMgr.getIdx_NR1());
    setIspTags<ISP_NVRAM_NR1_T, Preproc__NR1_Begin>(rTags);
    //
    //  Preprocess NR2
    MY_LOG("Preprocess nr2:");
    setIspIdx(rTags, Preproc__IDX_NR2, m_IspRegMgr.getIdx_NR2());
    setIspTags<ISP_NVRAM_NR2_T, Preproc__NR2_Begin>(rTags);

    if (m_IspCamInfo.eCamMode == ECamMode_Online_Capture_ZSD)
    {
        //
        //  Preprocess EE
        MY_LOG("Preprocess ee:");
        setIspIdx(rTags, Preproc__IDX_EE, m_IspRegMgr.getIdx_EE());
        setIspTags<ISP_NVRAM_EE_T, Preproc__EE_Begin>(rTags);
        //
        //  saturation
        MY_LOG("Preprocess saturation:");
        setIspIdx(rTags, Preproc__IDX_SATURATION, m_IspRegMgr.getIdx_Saturation());
        setIspTags<ISP_NVRAM_SATURATION_T, Preproc__SAT_Begin>(rTags);
        //
        //  contrast
        MY_LOG("Preprocess contrast:");
        setIspIdx(rTags, Preproc__IDX_CONTRAST, m_IspRegMgr.getIdx_Contrast());
        setIspTags<ISP_NVRAM_CONTRAST_T, Preproc__CONTRAST_Begin>(rTags);
        //
        //  hue
        MY_LOG("Preprocess hue:");
        setIspIdx(rTags, Preproc__IDX_HUE, m_IspRegMgr.getIdx_Hue());
        setIspTags<ISP_NVRAM_HUE_T, Preproc__HUE_Begin>(rTags);
        //
        //  PCA
        MY_LOG("Preprocess pca:");
        setIspIdx(rTags, Preproc__IDX_PCA, m_pIPcaMgr->getIdx());
        setIspTags<ISP_NVRAM_PCA_T, Preproc__PCA_Begin>(rTags);
    }

lbExit:
    return  MERR_OK;
}


/*******************************************************************************
* 
*******************************************************************************/
MERROR_ENUM
ParamctrlRAW::
queryExifDebugInfo(NSIspExifDebug::IspExifDebugInfo_T& rExifDebugInfo) const
{
    Mutex::Autolock lock(m_Lock);

    ////////////////////////////////////////////////////////////////////////////
    //  (1) Header.
    ////////////////////////////////////////////////////////////////////////////
    rExifDebugInfo.hdr  = g_scIspExifDebugInfoHdr;

    ////////////////////////////////////////////////////////////////////////////
    //  (2) Body.
    ////////////////////////////////////////////////////////////////////////////
    IspDebugTag (&rTags)[TagID_Total_Num] = m_stIspDebugInfo.tags;

    // ISP debug tag version
    setIspIdx(rTags, IspTagVersion, IspDebugTagVersion);

    //
    ////////////////////////////////////////////////////////////////////////////
    //  (2.1) ISPRegs
    ////////////////////////////////////////////////////////////////////////////
    //
    //  Gain Control
    MY_LOG("gain control:");
    setIspTags<ISP_NVRAM_GAIN_CTRL_T, GainCtrl_Begin>(rTags);
    //  Shading
    MY_LOG("shading:");
    setIspIdx(rTags, IDX_SHADING, m_IspRegMgr.getIdx_Shading());
    setIspTags<ISP_NVRAM_SHADING_T, SHADING_Begin>(rTags);
    //  OB
    MY_LOG("ob:");
    setIspIdx(rTags, IDX_OB, m_IspRegMgr.getIdx_OB());
    setIspTags<ISP_NVRAM_OB_T, OB_Begin>(rTags);
    //
    //  DP
    MY_LOG("dp:");
    setIspIdx(rTags, IDX_DP, m_IspRegMgr.getIdx_DP());
    setIspTags<ISP_NVRAM_DP_T, DP_Begin>(rTags);
    //
    //  NR1
    MY_LOG("nr1:");
    setIspIdx(rTags, IDX_NR1, m_IspRegMgr.getIdx_NR1());
    setIspTags<ISP_NVRAM_NR1_T, NR1_Begin>(rTags);
    //
    //  NR2
    MY_LOG("nr2:");
    setIspIdx(rTags, IDX_NR2, m_IspRegMgr.getIdx_NR2());
    setIspTags<ISP_NVRAM_NR2_T, NR2_Begin>(rTags);
    //
    //  DM
    MY_LOG("dm:");
    setIspIdx(rTags, IDX_DM, m_IspRegMgr.getIdx_DM());
    setIspTags<ISP_NVRAM_DEMOSAIC_T, DM_Begin>(rTags);
    //
    //  Color Clip
    MY_LOG("cclip:");
    setIspTags<ISP_NVRAM_CCLIP_T, CCLIP_Begin>(rTags);

    //
    //  EE
    MY_LOG("ee:");
    setIspIdx(rTags, IDX_EE, m_IspRegMgr.getIdx_EE());
    setIspTags<ISP_NVRAM_EE_T, EE_Begin>(rTags);
    //
    //  saturation
    MY_LOG("saturation:");
    setIspIdx(rTags, IDX_SATURATION, m_IspRegMgr.getIdx_Saturation());
    setIspTags<ISP_NVRAM_SATURATION_T, SAT_Begin>(rTags);
    //
    //  contrast
    MY_LOG("contrast:");
    setIspIdx(rTags, IDX_CONTRAST, m_IspRegMgr.getIdx_Contrast());
    setIspTags<ISP_NVRAM_CONTRAST_T, CONTRAST_Begin>(rTags);
    //
    //  hue
    MY_LOG("hue:");
    setIspIdx(rTags, IDX_HUE, m_IspRegMgr.getIdx_Hue());
    setIspTags<ISP_NVRAM_HUE_T, HUE_Begin>(rTags);
    //
    //  CCM
    MY_LOG("ccm:");
    setIspIdx(rTags, IDX_CCM, m_IspRegMgr.getIdx_CCM());
    setIspTags<ISP_NVRAM_CCM_T, CCM_Begin>(rTags);
    //
    //  Gamma
    MY_LOG("gamma:");
    setIspIdx(rTags, IDX_GAMMA, m_IspRegMgr.getIdx_Gamma());
    setIspTags<ISP_NVRAM_GAMMA_T, GAMMA_Begin>(rTags);
    //
    //  PCA
    MY_LOG("pca:");
    setIspIdx(rTags, IDX_PCA, m_pIPcaMgr->getIdx());
    setIspTags<ISP_NVRAM_PCA_T, PCA_Begin>(rTags);
    //
    //  Edge Gamma
    MY_LOG("edge gamma:");
    setIspTags<ISP_NVRAM_EDGE_GAMMA_T, EGAMMA_Begin>(rTags);
    //
    //  Effect Mode YCCGO
    MY_LOG("effect mode yccgo:");
    setIspTags<ISP_EFFECT_YCCGO_T, EFFECT_YCCGO_Begin>(rTags);
    //
    //  Effect Mode Edge
    MY_LOG("effect mode edge:");
    setIspTags<ISP_EFFECT_EDGE_T, EFFECT_EDGE_Begin>(rTags);
    //
    //  Effect Mode ccm
    MY_LOG("effect mode ccm:");
    setIspTags<ISP_EFFECT_CCM_T, EFFECT_CCM_Begin>(rTags);
    //
    //  Gamma ECO
    MY_LOG("gamma ECO:");
    setIspTags<ISP_NVRAM_GAMMA_ECO_T, GAMMA_ECO_Begin>(rTags);
    //
    //  YGB2YCC YOFST
    MY_LOG("YGB2YCC YOFST:");
    setIspTags<ISP_NVRAM_RGB2YCC_YOFST_T, YGB2YCC_YOFST_Begin>(rTags);

    //
    ////////////////////////////////////////////////////////////////////////////
    //  (2.2) ISPComm
    ////////////////////////////////////////////////////////////////////////////
    for (MUINT32 i = 0; i < sizeof(ISP_NVRAM_COMMON_STRUCT)/sizeof(MUINT32); i++)
    {
        MUINT32 const u4TagID = COMM_Begin + i;
        rTags[u4TagID].u4ID   = getIspTag(u4TagID);
        rTags[u4TagID].u4Val  = m_rIspComm.CommReg[i];
    }
    //
    //  (2.3) RAWIspCamInfo
    MY_LOG("RAWIspCamInfo:");
    setIspIdx(rTags, CapMode, m_IspCamInfo.eCamMode);
    setIspIdx(rTags, SceneIdx, m_IspCamInfo.eIdx_Scene);
    setIspIdx(rTags, ISOValue, m_IspCamInfo.u4ISOValue);
    setIspIdx(rTags, ISOIdx, m_IspCamInfo.eIdx_ISO);
    setIspIdx(rTags, CCMIdx, m_IspCamInfo.eIdx_CCM_CCT);
    setIspIdx(rTags, ShadingIdx, m_IspCamInfo.eIdx_Shading_CCT);
    setIspIdx(rTags, LightValue_x10, m_IspCamInfo.i4LightValue_x10);
    //
    //  (2.4) EffectMode
    MY_LOG("EffectMode:");
    setIspIdx(rTags, EffectMode, getEffect());
    //
    //  (2.5) UserSelectLevel
    MY_LOG("UserSelectLevel:");
    setIspIdx(rTags, EdgeIdx, getIspUsrSelectLevel().eIdx_Edge);
    setIspIdx(rTags, HueIdx, getIspUsrSelectLevel().eIdx_Hue);
    setIspIdx(rTags, SatIdx, getIspUsrSelectLevel().eIdx_Sat);
    setIspIdx(rTags, BrightIdx, getIspUsrSelectLevel().eIdx_Bright);
    setIspIdx(rTags, ContrastIdx, getIspUsrSelectLevel().eIdx_Contrast);
    //
    //  (2.6) Output
    rExifDebugInfo.debugInfo = m_stIspDebugInfo;

    //  (3) Reset to the default.
    ::memset(&m_stIspDebugInfo, 0, sizeof(m_stIspDebugInfo));

    return  MERR_OK;
}

