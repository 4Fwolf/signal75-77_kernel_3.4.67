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
#define LOG_TAG "NSIspTuning::ispmgr"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/xlog.h>
//
#include "debug.h"
#include "ispmgr_mt6575.h"
//
#include "isp_drv.h"
//
namespace NSIspTuning
{


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Utility.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define PUT_FIELD(to_field, from_field)\
    FIELD_VAL(to_field) = setbits(FIELD_VAL(to_field), rParam.from_field)

#define GET_FIELD(from_field, to_field)\
    rParam.to_field.val = FIELD_VAL(from_field)


template <class _ISP_XXX_T>
inline
MUINT32
setbits(MUINT32 const to_field, _ISP_XXX_T const from_field)
{
    MUINT32 const u4Mask = _ISP_XXX_T::MASK;
    //  (1) clear bits + (2) set bits
    return  ((to_field & ~u4Mask) | (from_field.val & u4Mask));
}

/*******************************************************************************
* Color Clip
*******************************************************************************/
ISP_MGR_CCLIP_T&
ISP_MGR_CCLIP_T::
getInstance()
{
    static ISP_MGR_CCLIP_T singleton;
    return singleton;
}


template <>
ISP_MGR_CCLIP_T&
ISP_MGR_CCLIP_T::
put(ISP_NVRAM_CCLIP_T const& rParam)
{
    PUT_FIELD(CAM_CCLIP_CON, cclip_ctrl);
    PUT_FIELD(CAM_CCLIP_GTC, cclip_gtc);
    PUT_FIELD(CAM_CCLIP_ADC, cclip_adc);
    PUT_FIELD(CAM_CCLIP_BAC, cclip_bac);
    return  (*this);
}


template <>
ISP_MGR_CCLIP_T&
ISP_MGR_CCLIP_T::
get(ISP_NVRAM_CCLIP_T& rParam)
{
    GET_FIELD(CAM_CCLIP_CON, cclip_ctrl);
    GET_FIELD(CAM_CCLIP_GTC, cclip_gtc);
    GET_FIELD(CAM_CCLIP_ADC, cclip_adc);
    GET_FIELD(CAM_CCLIP_BAC, cclip_bac);
    return  (*this);
}

/*******************************************************************************
* Gain Control
*******************************************************************************/
ISP_MGR_GAIN_CTRL_T&
ISP_MGR_GAIN_CTRL_T::
getInstance()
{
    static ISP_MGR_GAIN_CTRL_T singleton;
    return singleton;
}


template <>
ISP_MGR_GAIN_CTRL_T&
ISP_MGR_GAIN_CTRL_T::
put(ISP_NVRAM_GAIN_CTRL_T const& rParam)
{
    PUT_FIELD(CAM_CTRL1, cam_ctrl1);
    return  (*this);
}


template <>
ISP_MGR_GAIN_CTRL_T&
ISP_MGR_GAIN_CTRL_T::
get(ISP_NVRAM_GAIN_CTRL_T& rParam)
{
    GET_FIELD(CAM_CTRL1, cam_ctrl1);
    return  (*this);
}

/*******************************************************************************
* RGB2YCC Yoffset
*******************************************************************************/
ISP_MGR_RGB2YCC_YOFST_T&
ISP_MGR_RGB2YCC_YOFST_T::
getInstance()
{
    static ISP_MGR_RGB2YCC_YOFST_T singleton;
    return singleton;
}


template <>
ISP_MGR_RGB2YCC_YOFST_T&
ISP_MGR_RGB2YCC_YOFST_T::
put(ISP_NVRAM_RGB2YCC_YOFST_T const& rParam)
{
    PUT_FIELD(CAM_RGB2YCC_CON, rgb2ycc_yofst);
    return  (*this);
}


template <>
ISP_MGR_RGB2YCC_YOFST_T&
ISP_MGR_RGB2YCC_YOFST_T::
get(ISP_NVRAM_RGB2YCC_YOFST_T& rParam)
{
    GET_FIELD(CAM_RGB2YCC_CON, rgb2ycc_yofst);
    return  (*this);
}


/*******************************************************************************
* Shading
*******************************************************************************/
ISP_MGR_SHADING_T&
ISP_MGR_SHADING_T::
getInstance()
{
    static ISP_MGR_SHADING_T singleton;
    return singleton;
}


MBOOL
ISP_MGR_SHADING_T::
apply()
{
    if (m_u4ShadingParamChangeCount >0)
    {//protect not to update shading parameters every frame either they are the same value.
        ISP_MGR_BASE_T::apply();
	    m_u4ShadingParamChangeCount--;
    }
    return  MTRUE;
}


template <>
ISP_MGR_SHADING_T&
ISP_MGR_SHADING_T::
put(ISP_NVRAM_SHADING_T const& rParam)
{
    PUT_FIELD(RAW_DATA_ACC_CFG, raw_acc_cfg);
    PUT_FIELD(RAW_ACC_WIN, raw_acc_win);
    PUT_FIELD(RAW_ACC_RESULT_B, raw_acc_result_b);
    PUT_FIELD(RAW_ACC_RESULT_GB, raw_acc_result_gb);
    PUT_FIELD(RAW_ACC_RESULT_GR, raw_acc_result_gr);
    PUT_FIELD(RAW_ACC_RESULT_R, raw_acc_result_r);
    PUT_FIELD(SHADING_CTRL1, shading_ctrl1);
    PUT_FIELD(SHADING_CTRL2, shading_ctrl2);
    PUT_FIELD(SHADING_READ_ADDR, shading_read_addr);
    PUT_FIELD(SHADING_LAST_BLK_CFG, shading_last_blk);
    PUT_FIELD(SHADING_RATIO_CFG, shading_ratio_cfg);
    m_u4ShadingParamChangeCount++;
    return  (*this);
}


template <>
ISP_MGR_SHADING_T&
ISP_MGR_SHADING_T::
get(ISP_NVRAM_SHADING_T& rParam)
{
    GET_FIELD(RAW_DATA_ACC_CFG, raw_acc_cfg);
    GET_FIELD(RAW_ACC_WIN, raw_acc_win);
    GET_FIELD(RAW_ACC_RESULT_B, raw_acc_result_b);
    GET_FIELD(RAW_ACC_RESULT_GB, raw_acc_result_gb);
    GET_FIELD(RAW_ACC_RESULT_GR, raw_acc_result_gr);
    GET_FIELD(RAW_ACC_RESULT_R, raw_acc_result_r);
    GET_FIELD(SHADING_CTRL1, shading_ctrl1);
    GET_FIELD(SHADING_CTRL2, shading_ctrl2);
    GET_FIELD(SHADING_READ_ADDR, shading_read_addr);
    GET_FIELD(SHADING_LAST_BLK_CFG, shading_last_blk);
    GET_FIELD(SHADING_RATIO_CFG, shading_ratio_cfg);
    return  (*this);
}


ISP_MGR_SHADING_T&
ISP_MGR_SHADING_T::
setEnableShading(MBOOL const fgEnable)
{
    reinterpret_cast<ISP_SHADING_CTRL1_T*>(FIELD_VAL_PTR(SHADING_CTRL1))->SHADING_EN = (fgEnable ? 1 : 0);
    m_u4ShadingParamChangeCount++;
    MY_LOG("[-setEnableShading][ISP_MGR_SHADING_T](enable:%d)", fgEnable);
    return  (*this);
}


MBOOL
ISP_MGR_SHADING_T::
directSetTBA(MUINT32 const u4TBA)
{
    MBOOL fgRet = MFALSE;

    if  ( getIspReg() )
    {
        FIELD_VAL(SHADING_READ_ADDR) = u4TBA;//Modify TBA at class ISP_MGR_SHADING_T, too.
        fgRet = writeReg(REG_ADDR(CAM_SDRADDR), u4TBA);
    }
    MY_LOG("[ISP_MGR_SHADING_T] Set SHADING_READ_ADDR 0x%x, fgRet=%d\n", u4TBA, fgRet);
    return  fgRet;
}


/*******************************************************************************
* OB
*******************************************************************************/
ISP_MGR_OB_T&
ISP_MGR_OB_T::
getInstance()
{
    static ISP_MGR_OB_T singleton;
    return singleton;
}


template <>
ISP_MGR_OB_T&
ISP_MGR_OB_T::
put(ISP_NVRAM_OB_T const& rParam)
{
    PUT_FIELD(COMP_OFFSET_ADJUST, rgboff);
    return  (*this);
}


template <>
ISP_MGR_OB_T&
ISP_MGR_OB_T::
get(ISP_NVRAM_OB_T& rParam)
{
    GET_FIELD(COMP_OFFSET_ADJUST, rgboff);
    return  (*this);
}


/*******************************************************************************
* DM
*******************************************************************************/
ISP_MGR_DM_T&
ISP_MGR_DM_T::
getInstance()
{
    static ISP_MGR_DM_T singleton;
    return singleton;
}


template <>
ISP_MGR_DM_T&
ISP_MGR_DM_T::
put(ISP_NVRAM_DEMOSAIC_T const& rParam)
{
    PUT_FIELD(COLOR_PROC_STAGE_CTRL, ctrl);
    PUT_FIELD(INTERPOLATION_REG1, inter1);
    PUT_FIELD(INTERPOLATION_REG2, inter2);
    return  (*this);
}


template <>
ISP_MGR_DM_T&
ISP_MGR_DM_T::
get(ISP_NVRAM_DEMOSAIC_T& rParam)
{
    GET_FIELD(COLOR_PROC_STAGE_CTRL, ctrl);
    GET_FIELD(INTERPOLATION_REG1, inter1);
    GET_FIELD(INTERPOLATION_REG2, inter2);
    return  (*this);
}


template <>
ISP_MGR_DM_T&
ISP_MGR_DM_T::
put(ISP_EFFECT_EDGE_T const& rParam)
{
    PUT_FIELD(COLOR_PROC_STAGE_CTRL, ed_ctrl);
    PUT_FIELD(INTERPOLATION_REG1, ed_inter1);
    PUT_FIELD(INTERPOLATION_REG2, ed_inter2);
    return  (*this);
}


template <>
ISP_MGR_DM_T&
ISP_MGR_DM_T::
get(ISP_EFFECT_EDGE_T& rParam)
{
    GET_FIELD(COLOR_PROC_STAGE_CTRL, ed_ctrl);
    GET_FIELD(INTERPOLATION_REG1, ed_inter1);
    GET_FIELD(INTERPOLATION_REG2, ed_inter2);
    return  (*this);
}


/*******************************************************************************
* NR1
*******************************************************************************/
ISP_MGR_NR1_T&
ISP_MGR_NR1_T::
getInstance()
{
    static ISP_MGR_NR1_T singleton;
    return singleton;
}


template <>
ISP_MGR_NR1_T&
ISP_MGR_NR1_T::
put(ISP_NVRAM_DP_T const& rParam)
{
    PUT_FIELD(NR1_CTRL, ctrl);
    PUT_FIELD(NR1_DEFECT_PX_CFG1, dp1);
    PUT_FIELD(NR1_DEFECT_PX_CFG2, dp2);
    PUT_FIELD(NR1_DEFECT_PX_CFG3, dp3);
    PUT_FIELD(NR1_DEFECT_PX_CFG4, dp4);
    return  (*this);
}


template <>
ISP_MGR_NR1_T&
ISP_MGR_NR1_T::
get(ISP_NVRAM_DP_T& rParam)
{
    GET_FIELD(NR1_CTRL, ctrl);
    GET_FIELD(NR1_DEFECT_PX_CFG1, dp1);
    GET_FIELD(NR1_DEFECT_PX_CFG2, dp2);
    GET_FIELD(NR1_DEFECT_PX_CFG3, dp3);
    GET_FIELD(NR1_DEFECT_PX_CFG4, dp4);
    return  (*this);
}


template <>
ISP_MGR_NR1_T&
ISP_MGR_NR1_T::
put(ISP_NVRAM_NR1_T const& rParam)
{
    PUT_FIELD(NR1_CTRL, ctrl);
    PUT_FIELD(NR1_CX_COMP_CFG, ct);
    PUT_FIELD(NR1_NR_CFG1, nr_cfg1);
    PUT_FIELD(NR1_NR_CFG2, nr_cfg2);
    PUT_FIELD(NR1_NR_CFG3, nr_vlr14);
    PUT_FIELD(NR1_NR_CFG4, nr_vlr58);
    PUT_FIELD(NR1_NR_CFG5, nr_vlgr14);
    PUT_FIELD(NR1_NR_CFG6, nr_vlgr58);
    PUT_FIELD(NR1_NR_CFG7, nr_vlgb14);
    PUT_FIELD(NR1_NR_CFG8, nr_vlgb58);
    PUT_FIELD(NR1_NR_CFG9, nr_vlb14);
    PUT_FIELD(NR1_NR_CFG10, nr_vlb58);
    PUT_FIELD(NR1_NR_CFG11, nr_edge0);
    PUT_FIELD(NR1_NR_CFG12, nr_edge1);
    return  (*this);
}


template <>
ISP_MGR_NR1_T&
ISP_MGR_NR1_T::
get(ISP_NVRAM_NR1_T& rParam)
{
    GET_FIELD(NR1_CTRL, ctrl);
    GET_FIELD(NR1_CX_COMP_CFG, ct);
    GET_FIELD(NR1_NR_CFG1, nr_cfg1);
    GET_FIELD(NR1_NR_CFG2, nr_cfg2);
    GET_FIELD(NR1_NR_CFG3, nr_vlr14);
    GET_FIELD(NR1_NR_CFG4, nr_vlr58);
    GET_FIELD(NR1_NR_CFG5, nr_vlgr14);
    GET_FIELD(NR1_NR_CFG6, nr_vlgr58);
    GET_FIELD(NR1_NR_CFG7, nr_vlgb14);
    GET_FIELD(NR1_NR_CFG8, nr_vlgb58);
    GET_FIELD(NR1_NR_CFG9, nr_vlb14);
    GET_FIELD(NR1_NR_CFG10, nr_vlb58);
    GET_FIELD(NR1_NR_CFG11, nr_edge0);
    GET_FIELD(NR1_NR_CFG12, nr_edge1);
    return  (*this);
}


ISP_MGR_NR1_T&
ISP_MGR_NR1_T::
setEnableDP(MBOOL const fgEnable)
{
    reinterpret_cast<ISP_NR1_CTRL_T*>(FIELD_VAL_PTR(NR1_CTRL))->DP_EN = (fgEnable ? 1 : 0);
    MY_LOG("[-setEnableDP][ISP_MGR_NR1_T](enable:%d)", fgEnable);
    return  (*this);
}


ISP_MGR_NR1_T&
ISP_MGR_NR1_T::
setEnableCT(MBOOL const fgEnable)
{
    reinterpret_cast<ISP_NR1_CTRL_T*>(FIELD_VAL_PTR(NR1_CTRL))->CT_EN = (fgEnable ? 1 : 0);
    MY_LOG("[-setEnableCT][ISP_MGR_NR1_T](enable:%d)", fgEnable);
    return  (*this);
}


ISP_MGR_NR1_T&
ISP_MGR_NR1_T::
setEnableNR(MBOOL const fgEnable)
{
    reinterpret_cast<ISP_NR1_CTRL_T*>(FIELD_VAL_PTR(NR1_CTRL))->NR_EN = (fgEnable ? 1 : 0);
    MY_LOG("[-setEnableNR][ISP_MGR_NR1_T](enable:%d)", fgEnable);
    return  (*this);
}


/*******************************************************************************
* NR2
*******************************************************************************/
ISP_MGR_NR2_T&
ISP_MGR_NR2_T::
getInstance()
{
    static ISP_MGR_NR2_T singleton;
    return singleton;
}


template <>
ISP_MGR_NR2_T&
ISP_MGR_NR2_T::
put(ISP_NVRAM_NR2_T const& rParam)
{
    PUT_FIELD(NR2_CTRL, ctrl);
    PUT_FIELD(NR2_COMM_CFG1, cfg1);
    PUT_FIELD(NR2_CFG2, cfg2);
    PUT_FIELD(NR2_CFG3, pty);
    PUT_FIELD(NR2_CFG4, ptc);
    PUT_FIELD(NR2_COMM_CFG2, luma);
    PUT_FIELD(NR2_LCE_CFG1, lce_gain);
    PUT_FIELD(NR2_LCE_CFG2, lce_gain_div);
    PUT_FIELD(NR2_MODE1_CFG1, mode1_cfg1);
    PUT_FIELD(NR2_MODE1_CFG2, mode1_cfg2);
    PUT_FIELD(NR2_MODE1_CFG3, mode1_cfg3);
    return  (*this);
}


template <>
ISP_MGR_NR2_T&
ISP_MGR_NR2_T::
get(ISP_NVRAM_NR2_T& rParam)
{
    GET_FIELD(NR2_CTRL, ctrl);
    GET_FIELD(NR2_COMM_CFG1, cfg1);
    GET_FIELD(NR2_CFG2, cfg2);
    GET_FIELD(NR2_CFG3, pty);
    GET_FIELD(NR2_CFG4, ptc);
    GET_FIELD(NR2_COMM_CFG2, luma);
    GET_FIELD(NR2_LCE_CFG1, lce_gain);
    GET_FIELD(NR2_LCE_CFG2, lce_gain_div);
    GET_FIELD(NR2_MODE1_CFG1, mode1_cfg1);
    GET_FIELD(NR2_MODE1_CFG2, mode1_cfg2);
    GET_FIELD(NR2_MODE1_CFG3, mode1_cfg3);
    return  (*this);
}


ISP_MGR_NR2_T&
ISP_MGR_NR2_T::
setDisable()
{
    ISP_NR2_CTRL_T& r = *reinterpret_cast<ISP_NR2_CTRL_T*>(FIELD_VAL_PTR(NR2_CTRL));
    r.ENY = 0;
    r.ENC = 0;
    MY_LOG("[-setDisable][ISP_MGR_NR2_T]");
    return  (*this);
}


/*******************************************************************************
* EDGE
*******************************************************************************/
ISP_MGR_EDGE_T&
ISP_MGR_EDGE_T::
getInstance()
{
    static ISP_MGR_EDGE_T singleton;
    return singleton;
}


template <>
ISP_MGR_EDGE_T&
ISP_MGR_EDGE_T::
put(ISP_EFFECT_EDGE_T const& rParam)
{
    PUT_FIELD(EDGE_CORE, edgcore);
    PUT_FIELD(EDGE_GAIN_REG1, edggain1);
    PUT_FIELD(EDGE_GAIN_REG2, edggain2);
    PUT_FIELD(EDGE_THRESHOLD, edgthre);
    PUT_FIELD(EDGE_VERT_CTRL, edgvcon);
    return  (*this);
}


template <>
ISP_MGR_EDGE_T&
ISP_MGR_EDGE_T::
get(ISP_EFFECT_EDGE_T& rParam)
{
    GET_FIELD(EDGE_CORE, edgcore);
    GET_FIELD(EDGE_GAIN_REG1, edggain1);
    GET_FIELD(EDGE_GAIN_REG2, edggain2);
    GET_FIELD(EDGE_THRESHOLD, edgthre);
    GET_FIELD(EDGE_VERT_CTRL, edgvcon);
    return  (*this);
}


ISP_MGR_EDGE_T&
ISP_MGR_EDGE_T::
setDisable()
{
    FIELD_VAL(EDGE_CORE) = 0;
    FIELD_VAL(EDGE_GAIN_REG1) = 0;
    FIELD_VAL(EDGE_GAIN_REG2) = 0;
    FIELD_VAL(EDGE_THRESHOLD) = 0;
    FIELD_VAL(EDGE_VERT_CTRL) = 0;
    MY_LOG("[-setDisable][ISP_MGR_EDGE_T]");
    return  (*this);
}


/*******************************************************************************
* EE
*******************************************************************************/
ISP_MGR_EE_T&
ISP_MGR_EE_T::
getInstance()
{
    static ISP_MGR_EE_T singleton;
    return singleton;
}

template <>
ISP_MGR_EE_T&
ISP_MGR_EE_T::
put(ISP_NVRAM_GAMMA_ECO_T const& rParam)
{
    PUT_FIELD(COLOR_PROC_STAGE_CTRL2, gamma_eco_en);
    return  (*this);
}


template <>
ISP_MGR_EE_T&
ISP_MGR_EE_T::
get(ISP_NVRAM_GAMMA_ECO_T& rParam)
{
    GET_FIELD(COLOR_PROC_STAGE_CTRL2, gamma_eco_en);
    return  (*this);
}

template <>
ISP_MGR_EE_T&
ISP_MGR_EE_T::
put(ISP_NVRAM_EDGE_GAMMA_T const& rParam)
{
    PUT_FIELD(EDGE_ENHANCE_CTRL, ctrl);
    PUT_FIELD(EDGE_GAMMA_CFG1, cfg1);
    PUT_FIELD(EDGE_GAMMA_CFG2, cfg2);
    PUT_FIELD(EDGE_GAMMA_CFG3, cfg3);
    return  (*this);
}


template <>
ISP_MGR_EE_T&
ISP_MGR_EE_T::
get(ISP_NVRAM_EDGE_GAMMA_T& rParam)
{
    GET_FIELD(EDGE_ENHANCE_CTRL, ctrl);
    GET_FIELD(EDGE_GAMMA_CFG1, cfg1);
    GET_FIELD(EDGE_GAMMA_CFG2, cfg2);
    GET_FIELD(EDGE_GAMMA_CFG3, cfg3);
    return  (*this);
}


template <>
ISP_MGR_EE_T&
ISP_MGR_EE_T::
put(ISP_NVRAM_EE_T const& rParam)
{
    PUT_FIELD(COLOR_PROC_STAGE_CTRL2, y_egain);
    PUT_FIELD(EDGE_ENHANCE_CTRL, ee_ctrl);
    PUT_FIELD(EDGE_DETECT_CTRL1, ed_ctrl1);
    PUT_FIELD(EDGE_DETECT_CTRL2, ed_ctrl2);
    PUT_FIELD(EDGE_DETECT_CTRL3, ed_ctrl3);
    PUT_FIELD(EDGE_DETECT_CTRL4, ed_ctrl4);
    PUT_FIELD(EDGE_DETECT_CTRL5, ed_ctrl5);
    return  (*this);
}


template <>
ISP_MGR_EE_T&
ISP_MGR_EE_T::
get(ISP_NVRAM_EE_T& rParam)
{
    GET_FIELD(COLOR_PROC_STAGE_CTRL2, y_egain);
    GET_FIELD(EDGE_ENHANCE_CTRL, ee_ctrl);
    GET_FIELD(EDGE_DETECT_CTRL1, ed_ctrl1);
    GET_FIELD(EDGE_DETECT_CTRL2, ed_ctrl2);
    GET_FIELD(EDGE_DETECT_CTRL3, ed_ctrl3);
    GET_FIELD(EDGE_DETECT_CTRL4, ed_ctrl4);
    GET_FIELD(EDGE_DETECT_CTRL5, ed_ctrl5);
    return  (*this);
}


template <>
ISP_MGR_EE_T&
ISP_MGR_EE_T::
put(ISP_EFFECT_EDGE_T const& rParam)
{
    PUT_FIELD(COLOR_PROC_STAGE_CTRL2, cpscon2);
    PUT_FIELD(EDGE_ENHANCE_CTRL, ee_ctrl);
    PUT_FIELD(EDGE_DETECT_CTRL1, ed_ctrl1);
    PUT_FIELD(EDGE_DETECT_CTRL2, ed_ctrl2);
    PUT_FIELD(EDGE_DETECT_CTRL3, ed_ctrl3);
    PUT_FIELD(EDGE_DETECT_CTRL4, ed_ctrl4);
    PUT_FIELD(EDGE_DETECT_CTRL5, ed_ctrl5);
    return  (*this);
}


template <>
ISP_MGR_EE_T&
ISP_MGR_EE_T::
get(ISP_EFFECT_EDGE_T& rParam)
{
    GET_FIELD(COLOR_PROC_STAGE_CTRL2, cpscon2);
    GET_FIELD(EDGE_ENHANCE_CTRL, ee_ctrl);
    GET_FIELD(EDGE_DETECT_CTRL1, ed_ctrl1);
    GET_FIELD(EDGE_DETECT_CTRL2, ed_ctrl2);
    GET_FIELD(EDGE_DETECT_CTRL3, ed_ctrl3);
    GET_FIELD(EDGE_DETECT_CTRL4, ed_ctrl4);
    GET_FIELD(EDGE_DETECT_CTRL5, ed_ctrl5);
    return  (*this);
}


ISP_MGR_EE_T&
ISP_MGR_EE_T::
setDisable()
{
    FIELD_VAL(COLOR_PROC_STAGE_CTRL2) = 0;
    FIELD_VAL(EDGE_ENHANCE_CTRL) = 0;
    FIELD_VAL(EDGE_DETECT_CTRL1) = 0;
    FIELD_VAL(EDGE_DETECT_CTRL2) = 0;
    FIELD_VAL(EDGE_DETECT_CTRL3) = 0;
    FIELD_VAL(EDGE_DETECT_CTRL4) = 0;
    FIELD_VAL(EDGE_DETECT_CTRL5) = 0;
    MY_LOG("[-setDisable][ISP_MGR_EE_T]");
    return  (*this);
}


ISP_MGR_EE_T&
ISP_MGR_EE_T::
setEnableGamma(MBOOL const fgEnable)
{
    ISP_CPSCON2_T& r = *reinterpret_cast<ISP_CPSCON2_T*>(FIELD_VAL_PTR(COLOR_PROC_STAGE_CTRL2));
    r.BYPGM = (fgEnable ? 0 : 1);
#if 0
    MY_LOG("[-setEnableGamma][ISP_NVRAM_EE_T](enable:%d)", fgEnable);
#endif
    return  (*this);
}

/*******************************************************************************
* YCCGO
*******************************************************************************/
ISP_MGR_YCCGO_T&
ISP_MGR_YCCGO_T::
getInstance()
{
    static ISP_MGR_YCCGO_T singleton;
    return singleton;
}


template <>
ISP_MGR_YCCGO_T&
ISP_MGR_YCCGO_T::
put(ISP_NVRAM_SATURATION_T const& rParam)
{
    PUT_FIELD(YCCGO_CTRL, ctrl);
    PUT_FIELD(YCCGO_CFG2, y);
    PUT_FIELD(YCCGO_CFG3, gain);
    PUT_FIELD(YCCGO_CFG4, gain_ofs);
    PUT_FIELD(YCCGO_CFG6, bnd);
    PUT_FIELD(YCCGO_CFG7, uv_gain);
    PUT_FIELD(YCCGO_CFG8, uv_gain_slope);
    PUT_FIELD(YCCGO_CFG9, uv_x);
    return  (*this);
}


template <>
ISP_MGR_YCCGO_T&
ISP_MGR_YCCGO_T::
put(ISP_NVRAM_CONTRAST_T const& rParam)
{
    PUT_FIELD(YCCGO_CTRL, ctrl);
    PUT_FIELD(YCCGO_CFG4, ofs);
    PUT_FIELD(YCCGO_CFG5, gain_bnd);
    return  (*this);
}


template <>
ISP_MGR_YCCGO_T&
ISP_MGR_YCCGO_T::
put(ISP_NVRAM_HUE_T const& rParam)
{
    PUT_FIELD(YCCGO_CTRL, ctrl);
    PUT_FIELD(YCCGO_CFG1, cfg);
    return  (*this);
}


template <>
ISP_MGR_YCCGO_T&
ISP_MGR_YCCGO_T::
get(ISP_NVRAM_SATURATION_T& rParam)
{
    GET_FIELD(YCCGO_CTRL, ctrl);
    GET_FIELD(YCCGO_CFG2, y);
    GET_FIELD(YCCGO_CFG3, gain);
    GET_FIELD(YCCGO_CFG4, gain_ofs);
    GET_FIELD(YCCGO_CFG6, bnd);
    GET_FIELD(YCCGO_CFG7, uv_gain);
    GET_FIELD(YCCGO_CFG8, uv_gain_slope);
    GET_FIELD(YCCGO_CFG9, uv_x);
    return  (*this);
}


template <>
ISP_MGR_YCCGO_T&
ISP_MGR_YCCGO_T::
get(ISP_NVRAM_CONTRAST_T& rParam)
{
    GET_FIELD(YCCGO_CTRL, ctrl);
    GET_FIELD(YCCGO_CFG4, ofs);
    GET_FIELD(YCCGO_CFG5, gain_bnd);
    return  (*this);
}


template <>
ISP_MGR_YCCGO_T&
ISP_MGR_YCCGO_T::
get(ISP_NVRAM_HUE_T& rParam)
{
    GET_FIELD(YCCGO_CTRL, ctrl);
    GET_FIELD(YCCGO_CFG1, cfg);
    return  (*this);
}


template <>
ISP_MGR_YCCGO_T&
ISP_MGR_YCCGO_T::
put(ISP_EFFECT_YCCGO_T const& rParam)
{
    PUT_FIELD(YCCGO_CTRL, ctrl);
    PUT_FIELD(YCCGO_CFG1, cfg1);
    PUT_FIELD(YCCGO_CFG2, cfg2);
    PUT_FIELD(YCCGO_CFG3, cfg3);
    PUT_FIELD(YCCGO_CFG4, cfg4);
    PUT_FIELD(YCCGO_CFG5, cfg5);
    PUT_FIELD(YCCGO_CFG6, cfg6);
    PUT_FIELD(YCCGO_CFG7, cfg7);
    PUT_FIELD(YCCGO_CFG8, cfg8);
    PUT_FIELD(YCCGO_CFG9, cfg9);
    return  (*this);
}


template <>
ISP_MGR_YCCGO_T&
ISP_MGR_YCCGO_T::
get(ISP_EFFECT_YCCGO_T& rParam)
{
    GET_FIELD(YCCGO_CTRL, ctrl);
    GET_FIELD(YCCGO_CFG1, cfg1);
    GET_FIELD(YCCGO_CFG2, cfg2);
    GET_FIELD(YCCGO_CFG3, cfg3);
    GET_FIELD(YCCGO_CFG4, cfg4);
    GET_FIELD(YCCGO_CFG5, cfg5);
    GET_FIELD(YCCGO_CFG6, cfg6);
    GET_FIELD(YCCGO_CFG7, cfg7);
    GET_FIELD(YCCGO_CFG8, cfg8);
    GET_FIELD(YCCGO_CFG9, cfg9);
    return  (*this);
}


ISP_MGR_YCCGO_T&
ISP_MGR_YCCGO_T::
setDisable()
{
    ISP_YCCGO_CTRL_T& r = *reinterpret_cast<ISP_YCCGO_CTRL_T*>(FIELD_VAL_PTR(YCCGO_CTRL));
    r.ENC3 = 0;
    r.ENC2 = 0;
    r.ENC1 = 0;
    r.ENY3 = 0;
    r.ENY2 = 0;
    r.ENY1 = 0;
    MY_LOG("[-setDisable][ISP_MGR_YCCGO_T]");
    return  (*this);
}


/*******************************************************************************
* CCM
*******************************************************************************/
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
getInstance()
{
    static ISP_MGR_CCM_T singleton;
    return singleton;
}


template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
put(ISP_NVRAM_CCM_T const& rParam)
{
    PUT_FIELD(COLOR_MATRIX1, ccm1);
    PUT_FIELD(COLOR_MATRIX2, ccm2);
    PUT_FIELD(COLOR_MATRIX3, ccm3);
    PUT_FIELD(COLOR_MATRIX4, ccm4);
    PUT_FIELD(COLOR_MATRIX5, ccm5);
    return  (*this);
}


template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
get(ISP_NVRAM_CCM_T& rParam)
{
    GET_FIELD(COLOR_MATRIX1, ccm1);
    GET_FIELD(COLOR_MATRIX2, ccm2);
    GET_FIELD(COLOR_MATRIX3, ccm3);
    GET_FIELD(COLOR_MATRIX4, ccm4);
    GET_FIELD(COLOR_MATRIX5, ccm5);
    return  (*this);
}


template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
put(ISP_EFFECT_CCM_T const& rParam)
{
    PUT_FIELD(COLOR_MATRIX1, ccm1);
    PUT_FIELD(COLOR_MATRIX2, ccm2);
    PUT_FIELD(COLOR_MATRIX3, ccm3);
    PUT_FIELD(COLOR_MATRIX4, ccm4);
    PUT_FIELD(COLOR_MATRIX5, ccm5);
    return  (*this);
}


template <>
ISP_MGR_CCM_T&
ISP_MGR_CCM_T::
get(ISP_EFFECT_CCM_T& rParam)
{
    GET_FIELD(COLOR_MATRIX1, ccm1);
    GET_FIELD(COLOR_MATRIX2, ccm2);
    GET_FIELD(COLOR_MATRIX3, ccm3);
    GET_FIELD(COLOR_MATRIX4, ccm4);
    GET_FIELD(COLOR_MATRIX5, ccm5);
    return  (*this);
}


/*******************************************************************************
* GAMMA
*******************************************************************************/
ISP_MGR_GAMMA_T&
ISP_MGR_GAMMA_T::
getInstance()
{
    static ISP_MGR_GAMMA_T singleton;
    return singleton;
}


template <>
ISP_MGR_GAMMA_T&
ISP_MGR_GAMMA_T::
put(ISP_NVRAM_GAMMA_T const& rParam)
{
    PUT_FIELD(GAMMA_REG1, gamma1);
    PUT_FIELD(GAMMA_REG2, gamma2);
    PUT_FIELD(GAMMA_REG3, gamma3);
    PUT_FIELD(GAMMA_REG4, gamma4);
    PUT_FIELD(GAMMA_REG5, gamma5);
    return  (*this);
}


template <>
ISP_MGR_GAMMA_T&
ISP_MGR_GAMMA_T::
get(ISP_NVRAM_GAMMA_T& rParam)
{
    GET_FIELD(GAMMA_REG1, gamma1);
    GET_FIELD(GAMMA_REG2, gamma2);
    GET_FIELD(GAMMA_REG3, gamma3);
    GET_FIELD(GAMMA_REG4, gamma4);
    GET_FIELD(GAMMA_REG5, gamma5);
    return  (*this);
}


/*******************************************************************************
* PCA
*******************************************************************************/
ISP_MGR_PCA_T&
ISP_MGR_PCA_T::
getInstance()
{
    static ISP_MGR_PCA_T singleton;
    return singleton;
}


template <>
ISP_MGR_PCA_T&
ISP_MGR_PCA_T::
put(ISP_NVRAM_PCA_T const& rParam)
{
    PUT_FIELD(PCA_CTRL, ctrl);
    PUT_FIELD(PCA_GMC_CTRL, gmc);
    return  (*this);
}


template <>
ISP_MGR_PCA_T&
ISP_MGR_PCA_T::
get(ISP_NVRAM_PCA_T& rParam)
{
    GET_FIELD(PCA_CTRL, ctrl);
    GET_FIELD(PCA_GMC_CTRL, gmc);
    return  (*this);
}


ISP_MGR_PCA_T&
ISP_MGR_PCA_T::
setDisable()
{
    reinterpret_cast<ISP_PCA_CTRL_T*>(FIELD_VAL_PTR(PCA_CTRL))->EN = 0;
    MY_LOG("[-setDisable][ISP_MGR_PCA_T]");
    return  (*this);
}


MBOOL
ISP_MGR_PCA_T::
directSetTBA(MUINT32 const u4TBA)
{
    MBOOL fgRet = MFALSE;
    if  ( getIspReg() )
    {
        fgRet = writeReg(REG_ADDR(CAM_PCA_TBA), u4TBA);
    }
    MY_LOG("[-directSetTBA][ISP_MGR_PCA_T] (u4TBA, fgRet)=(0x%08X, %d)", u4TBA, fgRet);
    return  fgRet;
}


/*******************************************************************************
* 
*******************************************************************************/
};  //NSIspTuning

