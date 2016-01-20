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
#ifndef _ISP_EXIF_DEBUG_H_
#define _ISP_EXIF_DEBUG_H_
/*******************************************************************************
*
*******************************************************************************/
namespace NSIspExifDebug
{

enum { IspDebugTagVersion = 3 };

enum IspDebugTagID
{
    IspTagVersion,
    //  RAWIspCamInfo
    CapMode, 
    SceneIdx,
    ISOValue,
    ISOIdx,
    CCMIdx,
    ShadingIdx,
    LightValue_x10,
    // Effect
    EffectMode,
    // UserSelectLevel
    EdgeIdx,
    HueIdx,
    SatIdx,
    BrightIdx,
    ContrastIdx,
    //  Preprocess Index
    Preproc__IDX_SHADING, 
    Preproc__IDX_OB, 
    Preproc__IDX_DP, 
    Preproc__IDX_NR1, 
    Preproc__IDX_NR2, 
    Preproc__IDX_EE,
    Preproc__IDX_SATURATION,
    Preproc__IDX_CONTRAST,
    Preproc__IDX_HUE,
    Preproc__IDX_PCA,
    // Preprocrss gain control
    Preproc__GainCtrl_cam_ctrl1,
    //  Preprocess Shading
    Preproc__SHADING_raw_acc_cfg, 
    Preproc__SHADING_raw_acc_win, 
    Preproc__SHADING_raw_acc_result_b, 
    Preproc__SHADING_raw_acc_result_gb, 
    Preproc__SHADING_raw_acc_result_gr, 
    Preproc__SHADING_raw_acc_result_r, 
    Preproc__SHADING_shading_ctrl1, 
    Preproc__SHADING_shading_ctrl2, 
    Preproc__SHADING_shading_read_addr, 
    Preproc__SHADING_shading_last_blk, 
    Preproc__SHADING_shading_ratio_cfg, 
    //  Preprocess OB
    Preproc__OB_rgboff, 
    //  Preprocess DP
    Preproc__DP_ctrl, 
    Preproc__DP_dp1, 
    Preproc__DP_dp2, 
    Preproc__DP_dp3, 
    Preproc__DP_dp4, 
    //  Preprocess NR1
    Preproc__NR1_ctrl, 
    Preproc__NR1_ct, 
    Preproc__NR1_nr_cfg1, 
    Preproc__NR1_nr_cfg2, 
    Preproc__NR1_nr_vlr14, 
    Preproc__NR1_nr_vlr58, 
    Preproc__NR1_nr_vlgr14, 
    Preproc__NR1_nr_vlgr58, 
    Preproc__NR1_nr_vlgb14, 
    Preproc__NR1_nr_vlgb58, 
    Preproc__NR1_nr_vlb14, 
    Preproc__NR1_nr_vlb58, 
    Preproc__NR1_nr_edge0, 
    Preproc__NR1_nr_edge1, 
    //  Preprocess NR2
    Preproc__NR2_ctrl, 
    Preproc__NR2_cfg1, 
    Preproc__NR2_cfg2, 
    Preproc__NR2_pty, 
    Preproc__NR2_ptc, 
    Preproc__NR2_luma, 
    Preproc__NR2_lce_gain, 
    Preproc__NR2_lce_gain_div, 
    Preproc__NR2_mode1_cfg1, 
    Preproc__NR2_mode2_cfg2, 
    Preproc__NR2_mode3_cfg3, 
    //  Preprocess EE
    Preproc__EE_y_egain,
    Preproc__EE_ee_ctrl,
    Preproc__EE_ed_ctrl1,
    Preproc__EE_ed_ctrl2,
    Preproc__EE_ed_ctrl3,
    Preproc__EE_ed_ctrl4,
    Preproc__EE_ed_ctrl5,
    //  Preprocess Saturation
    Preproc__SAT_ctrl,
    Preproc__SAT_y,
    Preproc__SAT_gain,
    Preproc__SAT_gain_ofs,
    Preproc__SAT_bnd,
    Preproc__SAT_uv_gain,
    Preproc__SAT_uv_gain_slope,
    Preproc__SAT_uv_x,
    //  Preprocess Contrast
    Preproc__CONTRAST_ctrl,
    Preproc__CONTRAST_ofs,
    Preproc__CONTRAST_gain_bnd,
    //  Preprocess Hue
    Preproc__HUE_ctrl,
    Preproc__HUE_cfg,
    //  Preprocess PCA
    Preproc__PCA_ctrl,
    Preproc__PCA_gmc,
    //
    //  Index
    IDX_SHADING, 
    IDX_OB, 
    IDX_DP, 
    IDX_NR1, 
    IDX_NR2, 
    IDX_DM, 
    IDX_EE, 
    IDX_SATURATION, 
    IDX_CONTRAST, 
    IDX_HUE, 
    IDX_CCM, 
    IDX_GAMMA, 
    IDX_PCA, 
    //
    // Gain control
    GainCtrl_cam_ctrl1,
    //
    //  Shading
    SHADING_raw_acc_cfg, 
    SHADING_raw_acc_win, 
    SHADING_raw_acc_result_b, 
    SHADING_raw_acc_result_gb, 
    SHADING_raw_acc_result_gr, 
    SHADING_raw_acc_result_r, 
    SHADING_shading_ctrl1, 
    SHADING_shading_ctrl2, 
    SHADING_shading_read_addr, 
    SHADING_shading_last_blk, 
    SHADING_shading_ratio_cfg, 
    //  OB
    OB_rgboff, 
    //  DP
    DP_ctrl, 
    DP_dp1, 
    DP_dp2, 
    DP_dp3, 
    DP_dp4, 
    //  NR1
    NR1_ctrl, 
    NR1_ct, 
    NR1_nr_cfg1, 
    NR1_nr_cfg2, 
    NR1_nr_vlr14, 
    NR1_nr_vlr58, 
    NR1_nr_vlgr14, 
    NR1_nr_vlgr58, 
    NR1_nr_vlgb14, 
    NR1_nr_vlgb58, 
    NR1_nr_vlb14, 
    NR1_nr_vlb58, 
    NR1_nr_edge0, 
    NR1_nr_edge1, 
    //  NR2
    NR2_ctrl, 
    NR2_cfg1, 
    NR2_cfg2, 
    NR2_pty, 
    NR2_ptc, 
    NR2_luma, 
    NR2_lce_gain, 
    NR2_lce_gain_div, 
    NR2_mode1_cfg1, 
    NR2_mode2_cfg2, 
    NR2_mode3_cfg3, 
    //  DM
    DM_ctrl, 
    DM_inter1, 
    DM_inter2, 
    //  Color clip
    CCLIP_ctrl,
    CCLIP_gtc,
    CCLIP_adc,
    CCLIP_bac,
    //  EE
    EE_y_egain, 
    EE_ee_ctrl, 
    EE_ed_ctrl1,
    EE_ed_ctrl2,
    EE_ed_ctrl3,
    EE_ed_ctrl4,
    EE_ed_ctrl5,
    //  Saturation
    SAT_ctrl, 
    SAT_y, 
    SAT_gain, 
    SAT_gain_ofs, 
    SAT_bnd, 
    SAT_uv_gain, 
    SAT_uv_gain_slope, 
    SAT_uv_x, 
    //  Contrast
    CONTRAST_ctrl, 
    CONTRAST_ofs, 
    CONTRAST_gain_bnd, 
    //  Hue
    HUE_ctrl, 
    HUE_cfg, 
    //  CCM
    CCM_00, 
    CCM_01, 
    CCM_02, 
    CCM_03,
    CCM_04,
    //  GAMMA
    GAMMA_00,
    GAMMA_01,
    GAMMA_02,
    GAMMA_03,
    GAMMA_04,
    //  PCA
    PCA_ctrl, 
    PCA_gmc, 
    //  Edge Gamma
    EGAMMA_ctrl, 
    EGAMMA_cfg1, 
    EGAMMA_cfg2, 
    EGAMMA_cfg3, 
    // Effect Mode YCCGO
    EFFECT_YCCGO_ctrl,
    EFFECT_YCCGO_cfg1,
    EFFECT_YCCGO_cfg2,
    EFFECT_YCCGO_cfg3,
    EFFECT_YCCGO_cfg4,
    EFFECT_YCCGO_cfg5,
    EFFECT_YCCGO_cfg6,
    EFFECT_YCCGO_cfg7,
    EFFECT_YCCGO_cfg8,
    EFFECT_YCCGO_cfg9,
    // Effect Mode Edge
    EFFECT_EDGE_ed_ctrl,
    EFFECT_EDGE_ed_inter1,
    EFFECT_EDGE_ed_inter2,
    EFFECT_EDGE_edgcore,
    EFFECT_EDGE_edggain1,
    EFFECT_EDGE_edggain2,
    EFFECT_EDGE_edgthre,
    EFFECT_EDGE_edgvcon,
    EFFECT_EDGE_cpscon2,
    EFFECT_EDGE_ee_ctrl,
    EFFECT_EDGE_ed_ctrl1,
    EFFECT_EDGE_ed_ctrl2,
    EFFECT_EDGE_ed_ctrl3,
    EFFECT_EDGE_ed_ctrl4,
    EFFECT_EDGE_ed_ctrl5,
    // Effect Mode CCM
    EFFECT_CCM_ccm1,
    EFFECT_CCM_ccm2,
    EFFECT_CCM_ccm3,
    EFFECT_CCM_ccm4,
    EFFECT_CCM_ccm5,
    // Gamma ECO
    GAMMA_ECO_cam_cpscon2,
    // YGB2YCC YOfst
    YGB2YCC_YOFST_rgb2ycc_con,
    //
    //  Common
    COMM_00, 
    COMM_01, 
    COMM_02, 
    COMM_03, 
    COMM_04, 
    COMM_05, 
    COMM_06, 
    COMM_07, 
    COMM_08, 
    COMM_09, 
    COMM_10, 
    COMM_11, 
    COMM_12, 
    COMM_13, 
    COMM_14, 
    COMM_15, 
    COMM_16, 
    COMM_17, 
    COMM_18, 
    COMM_19, 
    COMM_20, 
    COMM_21, 
    COMM_22, 
    COMM_23, 
    COMM_24, 
    COMM_25, 
    COMM_26, 
    COMM_27, 
    COMM_28, 
    COMM_29, 
    COMM_30, 
    COMM_31, 
    COMM_32, 
    COMM_33, 
    COMM_34, 
    COMM_35, 
    COMM_36, 
    COMM_37, 
    COMM_38, 
    COMM_39, 
    COMM_40, 
    COMM_41, 
    COMM_42, 
    COMM_43, 
    COMM_44, 
    COMM_45, 
    COMM_46, 
    COMM_47, 
    COMM_48, 
    COMM_49, 
    COMM_50, 
    COMM_51, 
    COMM_52, 
    COMM_53, 
    COMM_54, 
    COMM_55, 
    COMM_56, 
    COMM_57, 
    COMM_58, 
    COMM_59, 
    COMM_60, 
    COMM_61, 
    COMM_62, 
    COMM_63, 
    //
};

enum
{
    // Preprocrss gain control
    Preproc__GainCtrl_Begin =   Preproc__GainCtrl_cam_ctrl1,
    //  Preprocess Shading
    Preproc__SHADING_Begin  =   Preproc__SHADING_raw_acc_cfg, 
    //  Preprocess OB
    Preproc__OB_Begin       =   Preproc__OB_rgboff, 
    //  Preprocess DP
    Preproc__DP_Begin       =   Preproc__DP_ctrl, 
    //  Preprocess NR1
    Preproc__NR1_Begin      =   Preproc__NR1_ctrl, 
    //  Preprocess NR2
    Preproc__NR2_Begin      =   Preproc__NR2_ctrl, 
    //  EE
    Preproc__EE_Begin       =   Preproc__EE_y_egain,
    //  Saturation
    Preproc__SAT_Begin      =   Preproc__SAT_ctrl,
    //  Contrast
    Preproc__CONTRAST_Begin =   Preproc__CONTRAST_ctrl,
    //  Hue
    Preproc__HUE_Begin      =   Preproc__HUE_ctrl,
    //  PCA
    Preproc__PCA_Begin      =   Preproc__PCA_ctrl,
    // Gain Control
    GainCtrl_Begin          =  GainCtrl_cam_ctrl1,
    //  OB
    OB_Begin                =   OB_rgboff, 
    //  DP
    DP_Begin                =   DP_ctrl, 
    //  NR1
    NR1_Begin               =   NR1_ctrl, 
    //  NR2
    NR2_Begin               =   NR2_ctrl, 
    //  EE
    EE_Begin                =   EE_y_egain, 
    //  DM
    DM_Begin                =   DM_ctrl, 
    //  Color Clip
    CCLIP_Begin             =   CCLIP_ctrl,
    //  Saturation
    SAT_Begin               =   SAT_ctrl, 
    //  Contrast
    CONTRAST_Begin          =   CONTRAST_ctrl, 
    //  Hue
    HUE_Begin               =   HUE_ctrl, 
    //  CCM
    CCM_Begin               =   CCM_00, 
    //  GAMMA
    GAMMA_Begin             =   GAMMA_00, 
    //  Shading
    SHADING_Begin           =   SHADING_raw_acc_cfg, 
    //  PCA
    PCA_Begin               =   PCA_ctrl, 
    //  Edge Gamma
    EGAMMA_Begin            =   EGAMMA_ctrl, 
    //  Effect Mode YCCGO
    EFFECT_YCCGO_Begin      =   EFFECT_YCCGO_ctrl,
    //  Effect Mode Edge
    EFFECT_EDGE_Begin       =   EFFECT_EDGE_ed_ctrl,
    //  Effect Mode CCM
    EFFECT_CCM_Begin        =   EFFECT_CCM_ccm1,
    // Gamma ECO
    GAMMA_ECO_Begin         =   GAMMA_ECO_cam_cpscon2,
    // YGB2YCC YOFST
    YGB2YCC_YOFST_Begin     =   YGB2YCC_YOFST_rgb2ycc_con,
    //  Common
    COMM_Begin              =   COMM_00, 
    //
    //
    TagID_Total_Num         =   COMM_63 + 1
};

struct IspDebugTag
{
    MUINT32     u4ID;
    MUINT32     u4Val;
};


struct IspDebugInfo
{
    IspDebugTag     tags[TagID_Total_Num];
};


typedef struct IspExifDebugInfo
{
    struct  Header
    {
        MUINT32     u4KeyID;
        MUINT32     u4ModuleCount;
        MUINT32     u4DebugInfoOffset;
    }   hdr;

    IspDebugInfo    debugInfo;

} IspExifDebugInfo_T;


};  //  namespace NSIspExifDebug
/*******************************************************************************
*
*******************************************************************************/
namespace NSIspTuning
{


/*******************************************************************************
*
*******************************************************************************/
template <MUINT32 total_module, MUINT32 tag_module>
struct ModuleNum
{
/*
    |   8  |       8      |   8  |     8      |
    | 0x00 | total_module | 0x00 | tag_module |
*/
    enum
    {
        val = ((total_module & 0xFF) << 16) | ((tag_module & 0xFF))
    };
};


template <MUINT32 module_id, MUINT32 tag_id, MUINT32 line_keep = 0>
struct ModuleTag
{
/*
    |     8     |      1    |   7  |    16    |
    | module_id | line_keep | 0x00 |  tag_id  |
*/
    enum
    {
        val = ((module_id & 0xFF) << 24)
            | ((line_keep & 0x01) << 23)
            | ((tag_id  & 0xFFFF) << 0)
    };
};


inline MUINT32 getModuleTag(MUINT32 module_id, MUINT32 tag_id, MUINT32 line_keep = 0)
{
/*
    |     8     |      1    |   7  |    16    |
    | module_id | line_keep | 0x00 |  tag_id  |
*/
    return  ((module_id & 0xFF) << 24)
          | ((line_keep & 0x01) << 23)
          | ((tag_id  & 0xFFFF) << 0)
            ;
}


enum { EModuleID_IspDebug = 0x0004 };
template <MUINT32 tag_id, MUINT32 line_keep = 0>
struct IspTag
{
    enum { val = ModuleTag<EModuleID_IspDebug, tag_id, line_keep>::val };
};


inline MUINT32 getIspTag(MUINT32 tag_id, MUINT32 line_keep = 0)
{
    return  getModuleTag(EModuleID_IspDebug, tag_id, line_keep);
}


//  Default of IspExifDebugInfo::Header
static NSIspExifDebug::IspExifDebugInfo::Header const g_scIspExifDebugInfoHdr =
{
    u4KeyID:            0xF4F5F6F7, 
    u4ModuleCount:      ModuleNum<1, 1>::val, 
    u4DebugInfoOffset:  sizeof(NSIspExifDebug::IspExifDebugInfo_T::Header)
};


/*******************************************************************************
*
*******************************************************************************/
};  //  namespace NSIspTuning
#endif // _ISP_EXIF_DEBUG_H_

