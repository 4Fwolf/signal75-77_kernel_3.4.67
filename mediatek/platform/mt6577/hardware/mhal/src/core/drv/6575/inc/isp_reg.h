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
#ifndef _ISP_REG_H_
#define _ISP_REG_H_

/*******************************************************************************
*
********************************************************************************/
#define ISP_BITS(RegBase, RegName, FieldName)  (RegBase->RegName.u.bits.FieldName)
#define ISP_REG(RegBase, RegName) (RegBase->RegName.u.reg)
//
#define ISP_BASE_HW     0xC2090000
#define ISP_BASE_RANGE  0x3000
//
typedef unsigned int MUINT32;
typedef unsigned int u32;
/***************************************************************************************/
typedef volatile struct _0000{
    union {
        struct {
            MUINT32 REV0     :4;
            MUINT32 REV1     :4;
            MUINT32 TGCLK_SEL     :1;
            MUINT32 NONE0     :1;
            MUINT32 ASYNC_INF_SEL     :1;
            MUINT32 CLKFL_POL     :1;
            MUINT32 PXCLK_IN     :1;
            MUINT32 PAD_PCLK_INV     :1;
            MUINT32 CAM_PCLK_INV     :1;
            MUINT32 HVALID_EN     :1;
            MUINT32 CLKFL     :4;
            MUINT32 CLKRS     :4;
            MUINT32 CLKCNT     :4;
            MUINT32 CLKPOL     :1;
            MUINT32 CLKEN     :1;
            MUINT32 REV2     :1;
            MUINT32 PCEN     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_phscnt_t;

typedef volatile struct _0004{
    union {
        struct {
            MUINT32 LINES     :12;
            MUINT32 NONE0     :4;
            MUINT32 PIXELS     :13;
            MUINT32 NONE1     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_camwin_t;

typedef volatile struct _0008{
    union {
        struct {
            MUINT32 PIXEL_END     :13;
            MUINT32 NONE0     :3;
            MUINT32 PIXEL_START     :12;
            MUINT32 NONE1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_grabcol_t;

typedef volatile struct _000C{
    union {
        struct {
            MUINT32 LINE_END     :12;
            MUINT32 NONE0     :4;
            MUINT32 LINE_START     :12;
            MUINT32 NONE1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_grabrow_t;

typedef volatile struct _0010{
    union {
        struct {
            MUINT32 EN     :1;
            MUINT32 NONE0     :2;
            MUINT32 REV0     :1;
            MUINT32 RST     :1;
            MUINT32 PWRON     :1;
            MUINT32 HSPOL     :1;
            MUINT32 VSPOL     :1;
            MUINT32 ISP_FRAME_START_POL     :1;
            MUINT32 NONE1     :1;
            MUINT32 REV1     :1;
            MUINT32 NONE2     :5;
            MUINT32 MEMIN_DUMMYLINE     :6;
            MUINT32 NONE3     :10;
        } bits;
        MUINT32 reg;
    } u;
}cam_csmode_t;

typedef volatile struct _0014{
    union {
        struct {
            MUINT32 BC_CAPTURE_EN     :1;
            MUINT32 NONE0     :15;
            MUINT32 BC_CAPTURE_NO     :5;
            MUINT32 NONE1     :11;
        } bits;
        MUINT32 reg;
    } u;
}cam_bcctrl_t;

typedef volatile struct _0018{
    union {
        struct {
            MUINT32 FR_CON     :3;
            MUINT32 NONE0     :3;
            MUINT32 TAKE_PIC     :1;
            MUINT32 SP_MODE     :1;
            MUINT32 SP_DELAY     :3;
            MUINT32 NONE1     :1;
            MUINT32 CAMRAW_BUSY_EN     :1;
            MUINT32 CAMRAW_BUSY_THL     :3;
            MUINT32 AV_SYNC_LINENO     :12;
            MUINT32 NONE2     :2;
            MUINT32 VD_INT_POL     :1;
            MUINT32 AV_SYNC_SEL     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_vfcon_t;

typedef volatile struct _001C{
    union {
        struct {
            MUINT32 EXPDONE_INT_EN     :1;
            MUINT32 REZOVRUN_INT_EN     :1;
            MUINT32 GMCOVRUN_INT_EN     :1;
            MUINT32 IDLE_INT_EN     :1;
            MUINT32 ISPDONE_INT_EN     :1;
            MUINT32 REV1     :2;
            MUINT32 FLASH_INT_EN     :1;
            MUINT32 TG1_INT_EN     :1;
            MUINT32 TG2_INT_EN     :1;
            MUINT32 VSYNC_INT_EN     :1;
            MUINT32 FLK_INT_EN     :1;
            MUINT32 PCA_INT_EN     :1;
            MUINT32 REV2     :1;
            MUINT32 LCE_INT_EN     :1;
            MUINT32 REV3     :1;
            MUINT32 INT_WCLR_EN     :1;
            MUINT32 NONE0     :15;
        } bits;
        MUINT32 reg;
    } u;
}cam_inten_t;

typedef volatile struct _0020{
    union {
        struct {
            MUINT32 EXPDONE_INT     :1;
            MUINT32 REZOVRUN_INT     :1;
            MUINT32 GMCOVRUN_INT     :1;
            MUINT32 IDLE_INT     :1;
            MUINT32 ISPDONE_INT     :1;
            MUINT32 NONE0     :2;
            MUINT32 FLASH_INT     :1;
            MUINT32 TG1_INT     :1;
            MUINT32 TG2_INT     :1;
            MUINT32 VSYNC_INT     :1;
            MUINT32 FLK_INT     :1;
            MUINT32 PCA_INT     :1;
            MUINT32 NONE1     :1;
            MUINT32 LCE_INT     :1;
            MUINT32 NONE2     :17;
        } bits;
        MUINT32 reg;
    } u;
}cam_intsta_t;

typedef volatile struct _0024{
    union {
        struct {
            MUINT32 INPATH_SEL     :2;
            MUINT32 INPATH_LINE_DIS     :1;
            MUINT32 BAYER10_IN     :1;
            MUINT32 INPATH_RATE     :4;
            MUINT32 INTYPE_SEL     :3;
            MUINT32 INDATA_FORMAT     :1;
            MUINT32 SWAP_CBCR     :1;
            MUINT32 SWAP_Y      :1;
            MUINT32 JPGINF_EN     :1;
            MUINT32 REV1     :1;
            MUINT32 OUTPATH_EN     :1;
            MUINT32 MEMW_GBURST     :3;
            MUINT32 OUTPATH_TYPE     :2;
            MUINT32 REV2     :1;
            MUINT32 REZ_DISCONN     :1;
            MUINT32 BAYER10_OUT     :1;
            MUINT32 CAM_WDMA_OK_SEL     :1;
            MUINT32 NONE0     :2;
            MUINT32 INPATH_THROT_DIS     :1;
            MUINT32 CNTMODE     :2;
            MUINT32 CNTON     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_path_t;

typedef volatile struct _0028{
    union {
        struct {
            MUINT32 CAM_INADDR     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_inaddr_t;

typedef volatile struct _002C{
    union {
        struct {
            MUINT32 CAM_OUTADDR     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_outaddr_t;

typedef volatile struct _0030{
    union {
        struct {
            MUINT32 RAW_PATH_SEL     :1;
            MUINT32 YUV_PATH_SEL     :1;
            MUINT32 NONE0     :2;
            MUINT32 MEMIN_OUTS_EN     :1;
            MUINT32 NONE1     :7;
            MUINT32 MEMIN_BURST16     :2;
            MUINT32 MEMIN_CRZ_BUSY_EN     :1;
            MUINT32 NONE2     :1;
            MUINT32 CRZ_RESZLB_LB_CONTROL     :1;
            MUINT32 MEMIN_AUTO_EN     :1;
            MUINT32 NONE3     :14;
        } bits;
        MUINT32 reg;
    } u;
}cam_memin_t;

typedef volatile struct _0034{
    union {
        struct {
            MUINT32 BAYEROUT_RAWSEL     :1;
            MUINT32 NONE0     :15;
            MUINT32 BAYEROUT_DM_EN     :1;
            MUINT32 NONE1     :3;
            MUINT32 EIS_INF_EN     :1;
            MUINT32 NONE2     :11;
        } bits;
        MUINT32 reg;
    } u;
}cam_memout_t;

typedef volatile struct _0038{
    union {
        struct {
            MUINT32 SYN_CCLIP_EN     :1;
            MUINT32 SYN_LRZ_EN     :1;
            MUINT32 SYN_LCE_ON     :1;
            MUINT32 SYN_SHADING_EN     :1;
            MUINT32 SYN_CT_EN     :1;
            MUINT32 SYN_DP2_EN     :1;
            MUINT32 SYN_DP1_EN     :1;
            MUINT32 SYN_NR1_EN     :1;
            MUINT32 SYN_BEN     :1;
            MUINT32 SYN_NR2_ENC     :1;
            MUINT32 SYN_NR2_ENY     :1;
            MUINT32 SYN_PCA_EN     :1;
            MUINT32 NONE0     :1;
            MUINT32 SYN_OUTPATH_EN     :1;
            MUINT32 NONE1     :18;
        } bits;
        MUINT32 reg;
    } u;
}cam_funsta_t;

typedef volatile struct _003C{
    union {
        struct {
            MUINT32 REV1     :1;
            MUINT32 NONE0     :7;
            MUINT32 AWBST_GAIN     :8;
            MUINT32 AWB_PGAIN     :10;
            MUINT32 NONE1     :2;
            MUINT32 AWB_PGAIN_EN     :1;
            MUINT32 AWBST_GAIN_EN     :1;
            MUINT32 NONE2     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_3again_t;

typedef volatile struct _0040{
    union {
        struct {
            MUINT32 NONE0     :1;
            MUINT32 PGAIN_FRAC     :7;
            MUINT32 PGAIN_INT     :2;
            MUINT32 NONE1     :2;
            MUINT32 GPID     :1;
            MUINT32 GLID     :1;
            MUINT32 REV1     :1;
            MUINT32 PGAIN_DB     :1;
            MUINT32 P_LIMIT     :8;
            MUINT32 GAIN_COMP     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctrl1_t;

typedef volatile struct _0044{
    union {
        struct {
            MUINT32 NONE0     :6;
            MUINT32 INTEN     :1;
            MUINT32 NONE1     :1;
            MUINT32 FHIS_SEL     :1;
            MUINT32 REV6     :1;
            MUINT32 NONE2     :1;
            MUINT32 REV5     :1;
            MUINT32 NONE3     :1;
            MUINT32 AF_SEL     :1;
            MUINT32 REV4     :1;
            MUINT32 AF_EN     :1;
            MUINT32 REV3     :1;
            MUINT32 REV2     :2;
            MUINT32 FHIS_EN     :1;
            MUINT32 DFDB     :1;
            MUINT32 RAWACCM_MODE     :1;
            MUINT32 AEWIN_EN     :1;
            MUINT32 AECCM_EN     :1;
            MUINT32 AEAWG_EN     :1;
            MUINT32 AEHIS_EN     :1;
            MUINT32 REV1     :1;
            MUINT32 NONE4     :1;
            MUINT32 ISPMEM_R1T     :1;
            MUINT32 NONE5     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctrl2_t;

typedef volatile struct _0048{
    union {
        struct {
            MUINT32 AE_HEIGHT     :9;
            MUINT32 AE_VOFFSET     :7;
            MUINT32 AE_WIDTH     :9;
            MUINT32 AE_HOFFSET     :7;
        } bits;
        MUINT32 reg;
    } u;
}cam_aewinh_t;

typedef volatile struct _004C{
    union {
        struct {
            MUINT32 AEHIS_WIND     :8;
            MUINT32 AEHIS_WINU     :8;
            MUINT32 AEHIS_WINR     :8;
            MUINT32 AEHIS_WINL     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_aehiswin_t;

typedef volatile struct _0050{
    union {
        struct {
            MUINT32 AE_GAIN     :9;
            MUINT32 NONE0     :3;
            MUINT32 AE_ACC_SEL     :1;
            MUINT32 NONE1     :3;
            MUINT32 AE_VOFFSET_H     :5;
            MUINT32 NONE2     :3;
            MUINT32 AE_HOFFSET_H     :5;
            MUINT32 NONE3     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_aegain_t;

typedef volatile struct _0054{
    union {
        struct {
            MUINT32 GS_EPTIMEU     :8;
            MUINT32 GS_TRTIME     :8;
            MUINT32 GS_EPTIME     :10;
            MUINT32 NONE0     :2;
            MUINT32 GSCTRL_EN     :1;
            MUINT32 STROBE_POL     :1;
            MUINT32 NONE1     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_gsctrl_t;

typedef volatile struct _0058{
    union {
        struct {
            MUINT32 MS_TIMEU     :8;
            MUINT32 NONE0     :8;
            MUINT32 MS_SWTR     :1;
            MUINT32 NONE1     :3;
            MUINT32 MS_SWTR_EN     :1;
            MUINT32 MSCTRL_MODE     :2;
            MUINT32 NONE2     :1;
            MUINT32 MS_GSMODE     :1;
            MUINT32 NONE3     :3;
            MUINT32 MSCTRL_EN     :1;
            MUINT32 SINGLETR_EN     :1;
            MUINT32 MECHSH1_POL     :1;
            MUINT32 MECHSH0_POL     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_msctrl_t;

typedef volatile struct _005C{
    union {
        struct {
            MUINT32 MS1T4     :8;
            MUINT32 MS1T3     :8;
            MUINT32 MS1T2     :8;
            MUINT32 MS1T1     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_ms1time_t;

typedef volatile struct _0060{
    union {
        struct {
            MUINT32 MS2T4     :8;
            MUINT32 MS2T3     :8;
            MUINT32 MS2T2     :8;
            MUINT32 MS2T1     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_ms2time_t;

typedef volatile struct _0064{
    union {
        struct {
            MUINT32 GB_GAIN_EXT     :2;
            MUINT32 GB_GAIN     :9;
            MUINT32 NONE0     :5;
            MUINT32 B_GAIN_EXT     :2;
            MUINT32 B_GAIN     :9;
            MUINT32 NONE1     :5;
        } bits;
        MUINT32 reg;
    } u;
}cam_rgbgain1_t;

typedef volatile struct _0068{
    union {
        struct {
            MUINT32 GR_GAIN_EXT     :2;
            MUINT32 GR_GAIN     :9;
            MUINT32 NONE0     :5;
            MUINT32 R_GAIN_EXT     :2;
            MUINT32 R_GAIN     :9;
            MUINT32 NONE1     :5;
        } bits;
        MUINT32 reg;
    } u;
}cam_rgbgain2_t;

typedef volatile struct _006C{
    union {
        struct {
            MUINT32 AWB_WIND     :8;
            MUINT32 AWB_WINU     :8;
            MUINT32 AWB_WINR     :8;
            MUINT32 AWB_WINL     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbwin_t;

typedef volatile struct _0070{
    union {
        struct {
            MUINT32 DISLJ     :1;
            MUINT32 VLEDGEN     :1;
            MUINT32 NONE0     :1;
            MUINT32 Researved2     :1;
            MUINT32 NONE1     :1;
            MUINT32 HLEDGEN     :1;
            MUINT32 NONE2     :1;
            MUINT32 Researved1     :1;
            MUINT32 DM_BPCMP_EN     :1;
            MUINT32 NONE3     :23;
        } bits;
        MUINT32 reg;
    } u;
}cam_cpscon1_t;

typedef volatile struct _0074{
    union {
        struct {
            MUINT32 THRE_RT     :5;
            MUINT32 NONE0     :3;
            MUINT32 THRE_DHV     :6;
            MUINT32 NONE1     :2;
            MUINT32 THRE_SM     :5;
            MUINT32 NONE2     :2;
            MUINT32 RB_SMOOTH_EN     :1;
            MUINT32 Researved1     :6;
            MUINT32 NONE3     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_inter1_t;

typedef volatile struct _0078{
    union {
        struct {
            MUINT32 THRE_LEDGE     :7;
            MUINT32 NONE0     :1;
            MUINT32 LINE_RBTHD     :5;
            MUINT32 NONE1     :3;
            MUINT32 NYQ_TH     :8;
            MUINT32 NONE2     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_inter2_t;

typedef volatile struct _0090{
    union {
        struct {
            MUINT32 OFF11     :7;
            MUINT32 S11     :1;
            MUINT32 OFF10     :7;
            MUINT32 S10     :1;
            MUINT32 OFF01     :7;
            MUINT32 S01     :1;
            MUINT32 OFF00     :7;
            MUINT32 S00     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_rgboff_t;

typedef volatile struct _0094{
    union {
        struct {
            MUINT32 M12_EXT     :3;
            MUINT32 M12     :8;
            MUINT32 NONE0     :5;
            MUINT32 M11_EXT     :3;
            MUINT32 M11     :8;
            MUINT32 NONE1     :5;
        } bits;
        MUINT32 reg;
    } u;
}cam_matrix1_t;

typedef volatile struct _0098{
    union {
        struct {
            MUINT32 M21_EXT     :3;
            MUINT32 M21     :8;
            MUINT32 NONE0     :5;
            MUINT32 M13_EXT     :3;
            MUINT32 M13     :8;
            MUINT32 NONE1     :5;
        } bits;
        MUINT32 reg;
    } u;
}cam_matrix2_t;

typedef volatile struct _009C{
    union {
        struct {
            MUINT32 M23_EXT     :3;
            MUINT32 M23     :8;
            MUINT32 NONE0     :5;
            MUINT32 M22_EXT     :3;
            MUINT32 M22     :8;
            MUINT32 NONE1     :5;
        } bits;
        MUINT32 reg;
    } u;
}cam_matrix3_t;

typedef volatile struct _00A0{
    union {
        struct {
            MUINT32 M32_EXT     :3;
            MUINT32 M32     :8;
            MUINT32 NONE0     :5;
            MUINT32 M31_EXT     :3;
            MUINT32 M31     :8;
            MUINT32 NONE1     :5;
        } bits;
        MUINT32 reg;
    } u;
}cam_matrix4_t;

typedef volatile struct _00A4{
    union {
        struct {
            MUINT32 NONE0     :16;
            MUINT32 M33_EXT     :3;
            MUINT32 M33     :8;
            MUINT32 NONE1     :5;
        } bits;
        MUINT32 reg;
    } u;
}cam_matrix5_t;

typedef volatile struct _00AC{
    union {
        struct {
            MUINT32 Y_EGAIN     :4;
            MUINT32 OPDGM_IVT     :1;
            MUINT32 REV1     :1;
            MUINT32 GMA_ECO_EN     :1;
            MUINT32 BYPGM     :1;
            MUINT32 NONE0     :24;
        } bits;
        MUINT32 reg;
    } u;
}cam_cpscon2_t;

typedef volatile struct _00B0{
    union {
        struct {
            MUINT32 FLARE_BGAIN     :8;
            MUINT32 FLARE_GGAIN     :8;
            MUINT32 FLARE_RGAIN     :8;
            MUINT32 NONE0     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_flregain_t;

typedef volatile struct _00B4{
    union {
        struct {
            MUINT32 FLARE_B     :6;
            MUINT32 NONE0     :1;
            MUINT32 SIGN_B     :1;
            MUINT32 FLARE_G     :6;
            MUINT32 NONE1     :1;
            MUINT32 SIGN_G     :1;
            MUINT32 FLARE_R     :6;
            MUINT32 NONE2     :1;
            MUINT32 SIGN_R     :1;
            MUINT32 NONE3     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_flreoff_t;

typedef volatile struct _00B8{
    union {
        struct {
            MUINT32 CSUP_EDGE_GAIN     :5;
            MUINT32 UV_LP_EN     :1;
            MUINT32 REV1     :1;
            MUINT32 VSUP_EN     :1;
            MUINT32 REV2     :16;
            MUINT32 NONE0     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_ychan_t;

typedef volatile struct _00BC{
    union {
        struct {
            MUINT32 VGAIN     :8;
            MUINT32 UGAIN     :8;
            MUINT32 YOFST     :8;
            MUINT32 YGAIN     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_rgb2ycc_con_t;

typedef volatile struct _00CC{
    union {
        struct {
            MUINT32 EN     :1;
            MUINT32 NONE0     :31;
        } bits;
        MUINT32 reg;
    } u;
}cam_cclip_con_t;

typedef volatile struct _00D0{
    union {
        struct {
            MUINT32 CCLIP_TC     :10;
            MUINT32 NONE0     :22;
        } bits;
        MUINT32 reg;
    } u;
}cam_cclip_gtc_t;

typedef volatile struct _00D4{
    union {
        struct {
            MUINT32 CCLIP_D_TH1     :10;
            MUINT32 CCLIP_D_SLOPE     :6;
            MUINT32 NONE0     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_cclip_adc_t;

typedef volatile struct _00D8{
    union {
        struct {
            MUINT32 CCLIP_B_OFFSET     :10;
            MUINT32 CCLIP_B_SLOPE     :4;
            MUINT32 NONE0     :2;
            MUINT32 CCLIP_B_DIFF     :10;
            MUINT32 NONE1     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_cclip_bac_t;

typedef volatile struct _00E0{
    union {
        struct {
            MUINT32 FLK_EN     :1;
            MUINT32 FLK_AE_EN     :1;
            MUINT32 NONE0     :2;
            MUINT32 FLK_MEM_MODE     :1;
            MUINT32 REV0     :3;
            MUINT32 REV1     :4;
            MUINT32 AE_THRE_SHFT_BIT     :3;
            MUINT32 NONE1     :1;
            MUINT32 AE_CHG_NUM     :5;
            MUINT32 NONE2     :7;
            MUINT32 FLK_DONE_STATUS     :1;
            MUINT32 FIFO_OVERFLOW_STATUS     :1;
            MUINT32 FIFO_UNDERFLOW_STATUS     :1;
            MUINT32 NONE3     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_flk_con_t;

typedef volatile struct _00E4{
    union {
        struct {
            MUINT32 INTVL_Y     :11;
            MUINT32 NONE0     :5;
            MUINT32 INTVL_X     :11;
            MUINT32 NONE1     :5;
        } bits;
        MUINT32 reg;
    } u;
}cam_flk_intvl_t;

typedef volatile struct _00E8{
    union {
        struct {
            MUINT32 GADDR_BASE     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_flk_gaddr_t;

#define CAM_HISRLT_SIZE 10
typedef volatile struct _00F0{
    union {
        struct {
            MUINT32 CAM_HISRLT     :22;
            MUINT32 NONE0     :10;
        } bits;
        MUINT32 reg;
    } u;
}cam_hisrlt_t;

typedef volatile struct _011C{
    union {
        struct {
            MUINT32 NONE0     :10;
            MUINT32 REV1     :2;
            MUINT32 REV2     :1;
            MUINT32 REV3     :2;
            MUINT32 REV4     :1;
            MUINT32 REV5     :1;
            MUINT32 NONE1     :3;
            MUINT32 YUV422_SLOW     :1;
            MUINT32 NONE2     :11;
        } bits;
        MUINT32 reg;
    } u;
}cam_yuv422_t;

typedef volatile struct _0120{
    union {
        struct {
            MUINT32 OUTPATH_SEL     :2;
            MUINT32 NONE0     :6;
            MUINT32 REV1     :8;
            MUINT32 REV2     :8;
            MUINT32 REV3     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_path2_t;

typedef volatile struct _0130{
    union {
        struct {
            MUINT32 RAWOUT_CROP_VEND     :12;
            MUINT32 NONE0     :4;
            MUINT32 RAWOUT_CROP_VSTART     :12;
            MUINT32 RAWOUT_CROP_EN     :1;
            MUINT32 NONE1     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawout_cropv_t;

typedef volatile struct _0134{
    union {
        struct {
            MUINT32 RAWOUT_CROP_HEND     :12;
            MUINT32 NONE0     :4;
            MUINT32 RAWOUT_CROP_HSTART     :12;
            MUINT32 NONE1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawout_croph_t;

typedef volatile struct _0154{
    union {
        struct {
            MUINT32 NONE0     :16;
            MUINT32 FIFO_LEVEL     :4;
            MUINT32 GBURST     :4;
            MUINT32 EN     :1;
            MUINT32 GMC_ON     :1;
            MUINT32 NONE1     :2;
            MUINT32 READ32     :1;
            MUINT32 NONE2     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_defect0_t;

typedef volatile struct _0158{
    union {
        struct {
            MUINT32 NONE0     :2;
            MUINT32 TABLE_ADDR     :30;
        } bits;
        MUINT32 reg;
    } u;
}cam_defect1_t;

typedef volatile struct _015C{
    union {
        struct {
            MUINT32 XLOC     :12;
            MUINT32 NONE0     :3;
            MUINT32 FIFO_COUNT_0     :1;
            MUINT32 YLOC     :12;
            MUINT32 FIFO_COUNT_4_1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_defect2_t;

typedef volatile struct _0160{
    union {
        struct {
            MUINT32 COUNT     :16;
            MUINT32 EN     :1;
            MUINT32 NONE0     :15;
        } bits;
        MUINT32 reg;
    } u;
}cam_adefectm_t;

typedef volatile struct _016C{
    union {
        struct {
            MUINT32 RAW_GRGAIN     :9;
            MUINT32 NONE0     :7;
            MUINT32 RAW_RGAIN     :9;
            MUINT32 NONE1     :7;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawgain0_t;

typedef volatile struct _0170{
    union {
        struct {
            MUINT32 RAW_GBGAIN     :9;
            MUINT32 NONE0     :7;
            MUINT32 RAW_BGAIN     :9;
            MUINT32 NONE1     :7;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawgain1_t;

typedef volatile struct _0174{
    union {
        struct {
            MUINT32 RWINV_END     :12;
            MUINT32 NONE0     :4;
            MUINT32 RWINV_START      :12;
            MUINT32 RWIN_EN     :1;
            MUINT32 NONE1     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_rwinv_sel_t;

typedef volatile struct _0178{
    union {
        struct {
            MUINT32 RWINH_END     :12;
            MUINT32 NONE0     :4;
            MUINT32 RWINH_START      :12;
            MUINT32 NONE1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_rwinh_sel_t;

typedef volatile struct _0180{
    union {
        struct {
            MUINT32 MEMIN_PVCOUNT     :12;
            MUINT32 NONE0     :4;
            MUINT32 MEMIN_PHCOUNT     :12;
            MUINT32 NONE1     :2;
            MUINT32 MEMIN_PAUSE_CONT     :1;
            MUINT32 MEMIN_PAUSE_EN     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_memindb_ctrl_t;

typedef volatile struct _0184{
    union {
        struct {
            MUINT32 VSIZE_COUNT     :12;
            MUINT32 NONE0     :4;
            MUINT32 HSIZE_COUNT     :13;
            MUINT32 NONE1     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_tgsize_sta_t;

typedef volatile struct _0188{
    union {
        struct {
            MUINT32 LAST_ADD     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_lastaddr_t;

typedef volatile struct _018C{
    union {
        struct {
            MUINT32 XFER_COUNT     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_xfercnt_t;

typedef volatile struct _0190{
    union {
        struct {
            MUINT32 CLK_DIV     :4;
            MUINT32 PATTERN     :3;
            MUINT32 STILL     :1;
            MUINT32 RST     :1;
            MUINT32 ON     :1;
            MUINT32 REV0     :2;
            MUINT32 FULL_RANGE     :1;
            MUINT32 LINECHG_EN     :1;
            MUINT32 REV1     :2;
            MUINT32 IDLE_PIXEL_PER_LINE     :8;
            MUINT32 VSYNC     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_mdlcfg1_t;

typedef volatile struct _0194{
    union {
        struct {
            MUINT32 PIXEL     :13;
            MUINT32 REV0     :3;
            MUINT32 LINE     :12;
            MUINT32 REV1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_mdlcfg2_t;

typedef volatile struct _01A0{
    union {
        struct {
            MUINT32 REV1     :6;
            MUINT32 NONE0     :10;
            MUINT32 REZ_OVRUN_FLIMIT_NO     :4;
            MUINT32 REZ_OVRUN_FLIMIT_EN     :1;
            MUINT32 NONE1     :3;
            MUINT32 CAMCRZ_INIT_PERIOD     :4;
            MUINT32 CAMCRZ_INIT_EN     :1;
            MUINT32 NONE2     :3;
        } bits;
        MUINT32 reg;
    } u;
}camcrz_ctrl_t;

typedef volatile struct _01A4{
    union {
        struct {
            MUINT32 NONE0     :16;
            MUINT32 REZ_OVRUN_FCOUN     :4;
            MUINT32 NONE1     :4;
            MUINT32 CAMCRZ_FIFOCNT     :6;
            MUINT32 NONE2     :2;
        } bits;
        MUINT32 reg;
    } u;
}camcrz_sta_t;

typedef volatile struct _01A8{
    union {
        struct {
            MUINT32 Y01     :8;
            MUINT32 Y02     :8;
            MUINT32 Y03     :8;
            MUINT32 Y04     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_gma_reg1_t;

typedef volatile struct _01AC{
    union {
        struct {
            MUINT32 Y05     :8;
            MUINT32 Y06     :8;
            MUINT32 Y07     :8;
            MUINT32 Y08     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_gma_reg2_t;

typedef volatile struct _01B0{
    union {
        struct {
            MUINT32 Y09     :8;
            MUINT32 Y10     :8;
            MUINT32 Y11     :8;
            MUINT32 Y12     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_gma_reg3_t;

typedef volatile struct _01B4{
    union {
        struct {
            MUINT32 Y13     :8;
            MUINT32 Y14     :8;
            MUINT32 Y15     :8;
            MUINT32 Y16     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_gma_reg4_t;

typedef volatile struct _01B8{
    union {
        struct {
            MUINT32 Y17     :8;
            MUINT32 Y18     :8;
            MUINT32 Y19     :8;
            MUINT32 Y20     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_gma_reg5_t;

typedef volatile struct _01BC{
    union {
        struct {
            MUINT32 RAWACC_EN     :1;
            MUINT32 NONE0     :31;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawacc_reg_t;

typedef volatile struct _01C0{
    union {
        struct {
            MUINT32 RAWWIN_D     :8;
            MUINT32 RAWWIN_U     :8;
            MUINT32 RAWWIN_R     :8;
            MUINT32 RAWWIN_L     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawwin_reg_t;

typedef volatile struct _01C4{
    union {
        struct {
            MUINT32 RAWSUM_B     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawsum_b_t;

typedef volatile struct _01C8{
    union {
        struct {
            MUINT32 RAWSUM_GB     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawsum_gb_t;

typedef volatile struct _01CC{
    union {
        struct {
            MUINT32 RAWSUM_GR     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawsum_gr_t;

typedef volatile struct _01D0{
    union {
        struct {
            MUINT32 RAWSUM_R     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawsum_r_t;

typedef volatile struct _01D4{
    union {
        struct {
            MUINT32 PIXEL_COUNT     :13;
            MUINT32 NONE0     :3;
            MUINT32 LINE_COUNT     :12;
            MUINT32 SYN_VF_DATA_EN     :1;
            MUINT32 CAPTURE_BUSY     :1;
            MUINT32 CAM_BUSY     :1;
            MUINT32 NONE1     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg_status_t;

typedef volatile struct _01D8{
    union {
        struct {
            MUINT32 HARD_RESET     :1;
            MUINT32 CKGEN_RESET     :1;
            MUINT32 NONE0     :6;
            MUINT32 CAM_FRAME_COUNT     :8;
            MUINT32 SW_RESET     :1;
            MUINT32 NONE1     :7;
            MUINT32 CAM_CS     :5;
            MUINT32 NONE2     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_reset_t;

typedef volatile struct _01E0{
    union {
        struct {
            MUINT32 FLASH_LNUNIT_NO     :20;
            MUINT32 FLASH_LNUNIT     :4;
            MUINT32 FLASH_STARTPNT     :1;
            MUINT32 NONE0     :2;
            MUINT32 FLASH_POL     :1;
            MUINT32 FLASH_EN     :1;
            MUINT32 FLASH_FRAME_DELAY     :2;
            MUINT32 FLASH_OUT     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_flash_ctrl0_t;

typedef volatile struct _01E4{
    union {
        struct {
            MUINT32 FLASH_PIXEL     :13;
            MUINT32 NONE0     :3;
            MUINT32 FLASH_LINE     :12;
            MUINT32 NONE1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_flash_ctrl1_t;

typedef volatile struct _01E8{
    union {
        struct {
            MUINT32 FLASHB_PIXEL     :13;
            MUINT32 NONE0     :3;
            MUINT32 FLASHB_LINE     :12;
            MUINT32 FLASHB_START_FRAME     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_flashb_ctrl0_t;

typedef volatile struct _01EC{
    union {
        struct {
            MUINT32 FLASHB_LNUNIT_NO     :20;
            MUINT32 FLASHB_CONT_FRAME     :3;
            MUINT32 REV     :1;
            MUINT32 NONE0     :4;
            MUINT32 FLASHB_LNUNIT     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_flashb_ctrl1_t;

typedef volatile struct _01F8{
    union {
        struct {
            MUINT32 SD_DEBUG1     :30;
            MUINT32 NONE0     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_sd_dbg0_t;

typedef volatile struct _01FC{
    union {
        struct {
            MUINT32 SD_DEBUG2     :30;
            MUINT32 NONE0     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_sd_dbg1_t;

typedef volatile struct _0200{
    union {
        struct {
            MUINT32 SD_DEBUG2     :30;
            MUINT32 NONE0     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_sd_dbg2_t;

typedef volatile struct _0204{
    union {
        struct {
            MUINT32 SD_DEBUG3     :30;
            MUINT32 NONE0     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_sd_dbg3_t;

typedef volatile struct _0208{
    union {
        struct {
            MUINT32 SHADING_GAIN     :13;
            MUINT32 NONE0     :19;
        } bits;
        MUINT32 reg;
    } u;
}cam_sd_dbg4_t;

typedef volatile struct _020C{
    union {
        struct {
            MUINT32 HTCNT     :12;
            MUINT32 NONE0     :4;
            MUINT32 WDCNT     :12;
            MUINT32 NONE1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_sd_dbg5_t;

typedef volatile struct _0210{
    union {
        struct {
            MUINT32 BAYERCNT     :14;
            MUINT32 NONE0     :2;
            MUINT32 GMC_MAST     :3;
            MUINT32 NONE1     :13;
        } bits;
        MUINT32 reg;
    } u;
}cam_gmcdebug_t;

typedef volatile struct _0214{
    union {
        struct {
            MUINT32 SDBLK_YOFST     :6;
            MUINT32 NONE0     :10;
            MUINT32 SDBLK_XOFST     :6;
            MUINT32 NONE1     :2;
            MUINT32 SD_COEFRD_MODE     :1;
            MUINT32 NONE2     :3;
            MUINT32 SHADING_EN     :1;
            MUINT32 SDBLK_TRIG     :1;
            MUINT32 NONE3     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_shading1_t;

typedef volatile struct _0218{
    union {
        struct {
            MUINT32 SDBLK_HEIGHT     :12;
            MUINT32 SDBLK_YNUM     :4;
            MUINT32 SDBLK_WIDTH     :12;
            MUINT32 SDBLK_XNUM     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_shading2_t;

typedef volatile struct _021C{
    union {
        struct {
            MUINT32 SDBLK_RADDR     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_sdraddr_t;

typedef volatile struct _0220{
    union {
        struct {
            MUINT32 SDBLK_lHEIGHT     :12;
            MUINT32 NONE0     :4;
            MUINT32 SDBLK_lWIDTH     :12;
            MUINT32 NONE1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_sdlblock_t;

typedef volatile struct _0224{
    union {
        struct {
            MUINT32 RATIO11     :6;
            MUINT32 NONE0     :2;
            MUINT32 RATIO10     :6;
            MUINT32 NONE1     :2;
            MUINT32 RATIO01     :6;
            MUINT32 NONE2     :2;
            MUINT32 RATIO00     :6;
            MUINT32 NONE3     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_sdratio_t;

typedef volatile struct _0228{
    union {
        struct {
            MUINT32 TG1_PIXEL     :13;
            MUINT32 NONE0     :3;
            MUINT32 TG1_LINE     :12;
            MUINT32 NONE1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg1ctrl_t;

typedef volatile struct _022C{
    union {
        struct {
            MUINT32 TG2_PIXEL     :13;
            MUINT32 NONE0     :3;
            MUINT32 TG2_LINE     :12;
            MUINT32 NONE1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2ctrl_t;

typedef volatile struct _023C{
    union {
        struct {
            MUINT32 ON     :1;
            MUINT32 GLOBAL     :1;
            MUINT32 GLOBAL_VHALF     :7;
            MUINT32 SI_SIZE     :2;
            MUINT32 START_ADDR_CNT     :1;
            MUINT32 NONE0     :4;
            MUINT32 BAYER2G_GAMMA_EN     :1;
            MUINT32 NONE1     :3;
            MUINT32 BAYER2G_RATIO     :2;
            MUINT32 NONE2     :2;
            MUINT32 BAYER2G_GSEL     :1;
            MUINT32 NONE3     :7;
        } bits;
        MUINT32 reg;
    } u;
}cam_lce_con_t;

typedef volatile struct _0240{
    union {
        struct {
            MUINT32 BC_START_I     :13;
            MUINT32 NONE0     :3;
            MUINT32 BC_START_J     :13;
            MUINT32 NONE1     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_lce_bc_sp_t;

typedef volatile struct _0244{
    union {
        struct {
            MUINT32 BC_END_I     :13;
            MUINT32 NONE0     :3;
            MUINT32 BC_END_J     :13;
            MUINT32 NONE1     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_lce_bc_ep_t;

typedef volatile struct _0248{
    union {
        struct {
            MUINT32 BC_MAG_KUBNY     :15;
            MUINT32 NONE0     :1;
            MUINT32 BC_MAG_KUBNX     :15;
            MUINT32 NONE1     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_lce_zr_t;

typedef volatile struct _024C{
    union {
        struct {
            MUINT32 START_ADDR_A     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_lce_mem_sa1_t;

typedef volatile struct _0250{
    union {
        struct {
            MUINT32 START_ADDR_B     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_lce_mem_sa2_t;

typedef volatile struct _0254{
    union {
        struct {
            MUINT32 PA     :8;
            MUINT32 PB     :8;
            MUINT32 BA     :8;
            MUINT32 NONE0     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_lce_qua_t;

typedef volatile struct _0258{
    union {
        struct {
            MUINT32 GAIN_THR1     :6;
            MUINT32 NONE0     :2;
            MUINT32 GAIN_THR2     :6;
            MUINT32 NONE1     :2;
            MUINT32 GAIN_THR3     :6;
            MUINT32 NONE2     :10;
        } bits;
        MUINT32 reg;
    } u;
}cam_lce_thr_t;

typedef volatile struct _025C{
    union {
        struct {
            MUINT32 EN     :1;
            MUINT32 RSV     :1;
            MUINT32 NONE0     :30;
        } bits;
        MUINT32 reg;
    } u;
}cam_lrz_con_t;

typedef volatile struct _0260{
    union {
        struct {
            MUINT32 LRZ_DBG     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_lrz_dbg_t;

typedef volatile struct _0264{
    union {
        struct {
            MUINT32 LRZ_TAR_H     :13;
            MUINT32 NONE0     :3;
            MUINT32 LRZ_TAR_V     :13;
            MUINT32 NONE1     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_lrz_tar_t;

typedef volatile struct _0268{
    union {
        struct {
            MUINT32 LRZ_RATIO_H     :21;
            MUINT32 NONE0     :11;
        } bits;
        MUINT32 reg;
    } u;
}cam_lrz_hdr_t;

typedef volatile struct _026C{
    union {
        struct {
            MUINT32 LRZ_RATIO_V     :21;
            MUINT32 NONE0     :11;
        } bits;
        MUINT32 reg;
    } u;
}cam_lrz_vdr_t;

typedef volatile struct _0274{
    union {
        struct {
            MUINT32 DATE     :8;
            MUINT32 MONTH     :8;
            MUINT32 YEAR     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_version_t;

typedef volatile struct _027C{
    union {
        struct {
            MUINT32 AWBSUM_WIND     :8;
            MUINT32 AWBSUM_WINU     :8;
            MUINT32 AWBSUM_WINR     :8;
            MUINT32 AWBSUM_WINL     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbsum_win_t;

typedef volatile struct _0280{
    union {
        struct {
            MUINT32 PAXEL_YL     :8;
            MUINT32 PAXEL_RGBH     :8;
            MUINT32 AWBDM_DEBUG     :1;
            MUINT32 NONE0     :3;
            MUINT32 REV     :1;
            MUINT32 NONE1     :3;
            MUINT32 SMAREA_NO     :3;
            MUINT32 NONE2     :1;
            MUINT32 SMAREA_EN     :1;
            MUINT32 COLOREDGE_EN     :1;
            MUINT32 NEUTRAL_EN     :1;
            MUINT32 AWB_EN     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_awb_ctrl_t;

typedef volatile struct _0284{
    union {
        struct {
            MUINT32 NEUTRAL_TH     :12;
            MUINT32 NONE0     :4;
            MUINT32 CEDGEY_TH     :8;
            MUINT32 CEDGEX_TH     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbth_t;

typedef volatile struct _0288{
    union {
        struct {
            MUINT32 AWBH12     :9;
            MUINT32 NONE0     :7;
            MUINT32 AWBH11     :9;
            MUINT32 NONE1     :7;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxyh1_t;

typedef volatile struct _028C{
    union {
        struct {
            MUINT32 AWBH22     :9;
            MUINT32 NONE0     :7;
            MUINT32 AWBH21     :9;
            MUINT32 NONE1     :7;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxyh2_t;

typedef volatile struct _0290{
    union {
        struct {
            MUINT32 AWBCE_WINR     :16;
            MUINT32 AWBCE_WINL     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbce_winh_t;

typedef volatile struct _0294{
    union {
        struct {
            MUINT32 AWBCE_WIND     :16;
            MUINT32 AWBCE_WINU     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbce_winv_t;

typedef volatile struct _0298{
    union {
        struct {
            MUINT32 AWBXY_WINR0     :16;
            MUINT32 AWBXY_WINL0     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinh0_t;

typedef volatile struct _029C{
    union {
        struct {
            MUINT32 AWBXY_WIND0     :16;
            MUINT32 AWBXY_WINU0     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinv0_t;

typedef volatile struct _02A0{
    union {
        struct {
            MUINT32 AWBXY_WINR1     :16;
            MUINT32 AWBXY_WINL1     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinh1_t;

typedef volatile struct _02A4{
    union {
        struct {
            MUINT32 AWBXY_WIND1     :16;
            MUINT32 AWBXY_WINU1     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinv1_t;

typedef volatile struct _02A8{
    union {
        struct {
            MUINT32 AWBXY_WINR2     :16;
            MUINT32 AWBXY_WINL2     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinh2_t;

typedef volatile struct _02AC{
    union {
        struct {
            MUINT32 AWBXY_WIND2     :16;
            MUINT32 AWBXY_WINU2     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinv2_t;

typedef volatile struct _02B0{
    union {
        struct {
            MUINT32 AWBXY_WINR3     :16;
            MUINT32 AWBXY_WINL3     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinh3_t;

typedef volatile struct _02B4{
    union {
        struct {
            MUINT32 AWBXY_WIND3     :16;
            MUINT32 AWBXY_WINU3     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinv3_t;

typedef volatile struct _02B8{
    union {
        struct {
            MUINT32 AWBXY_WINR4     :16;
            MUINT32 AWBXY_WINL4     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinh4_t;

typedef volatile struct _02BC{
    union {
        struct {
            MUINT32 AWBXY_WIND4     :16;
            MUINT32 AWBXY_WINU4     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinv4_t;

typedef volatile struct _02C0{
    union {
        struct {
            MUINT32 AWBXY_WINR5     :16;
            MUINT32 AWBXY_WINL5     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinh5_t;

typedef volatile struct _02C4{
    union {
        struct {
            MUINT32 AWBXY_WIND5     :16;
            MUINT32 AWBXY_WINU5     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinv5_t;

typedef volatile struct _02C8{
    union {
        struct {
            MUINT32 AWBXY_WINR6     :16;
            MUINT32 AWBXY_WINL6     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinh6_t;

typedef volatile struct _02CC{
    union {
        struct {
            MUINT32 AWBXY_WIND6     :16;
            MUINT32 AWBXY_WINU6     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinv6_t;

typedef volatile struct _02D0{
    union {
        struct {
            MUINT32 AWBXY_WINR7     :16;
            MUINT32 AWBXY_WINL7     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinh7_t;

typedef volatile struct _02D4{
    union {
        struct {
            MUINT32 AWBXY_WIND7     :16;
            MUINT32 AWBXY_WINU7     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinv7_t;

typedef volatile struct _02D8{
    union {
        struct {
            MUINT32 AWBXY_WINR8     :16;
            MUINT32 AWBXY_WINL8     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinh8_t;

typedef volatile struct _02DC{
    union {
        struct {
            MUINT32 AWBXY_WIND8     :16;
            MUINT32 AWBXY_WINU8     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinv8_t;

typedef volatile struct _02E0{
    union {
        struct {
            MUINT32 AWBXY_WINR9     :16;
            MUINT32 AWBXY_WINL9     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinh9_t;

typedef volatile struct _02E4{
    union {
        struct {
            MUINT32 AWBXY_WIND9     :16;
            MUINT32 AWBXY_WINU9     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinv9_t;

typedef volatile struct _02E8{
    union {
        struct {
            MUINT32 AWBXY_WINRA     :16;
            MUINT32 AWBXY_WINLA     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinh10_t;

typedef volatile struct _02EC{
    union {
        struct {
            MUINT32 AWBXY_WINDA     :16;
            MUINT32 AWBXY_WINUA     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinv10_t;

typedef volatile struct _02F0{
    union {
        struct {
            MUINT32 AWBXY_WINRB     :16;
            MUINT32 AWBXY_WINLB     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinh11_t;

typedef volatile struct _02F4{
    union {
        struct {
            MUINT32 AWBXY_WINDB     :16;
            MUINT32 AWBXY_WINUB     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxywinv11_t;

typedef volatile struct _02F8{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbsum_pcnt_t;

typedef volatile struct _02FC{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbsum_rsum_t;

typedef volatile struct _0300{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbsum_gsum_t;

typedef volatile struct _0304{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbsum_bsum_t;

typedef volatile struct _0308{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbce_pcnt_t;

typedef volatile struct _030C{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbce_rsum_t;

typedef volatile struct _0310{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbce_gsum_t;

typedef volatile struct _0314{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbce_bsum_t;

typedef volatile struct _0318{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_pcnt0_t;

typedef volatile struct _031C{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_rsum0_t;

typedef volatile struct _0320{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_gsum0_t;

typedef volatile struct _0324{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_bsum0_t;

typedef volatile struct _0328{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_pcnt1_t;

typedef volatile struct _032C{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_rsum1_t;

typedef volatile struct _0330{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_gsum1_t;

typedef volatile struct _0334{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_bsum1_t;

typedef volatile struct _0338{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_pcnt2_t;

typedef volatile struct _033C{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_rsum2_t;

typedef volatile struct _0340{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_gsum2_t;

typedef volatile struct _0344{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_bsum2_t;

typedef volatile struct _0348{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_pcnt3_t;

typedef volatile struct _034C{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_rsum3_t;

typedef volatile struct _0350{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_gsum3_t;

typedef volatile struct _0354{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_bsum3_t;

typedef volatile struct _0358{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_pcnt4_t;

typedef volatile struct _035C{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_rsum4_t;

typedef volatile struct _0360{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_gsum4_t;

typedef volatile struct _0364{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_bsum4_t;

typedef volatile struct _0368{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_pcnt5_t;

typedef volatile struct _036C{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_rsum5_t;

typedef volatile struct _0370{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_gsum5_t;

typedef volatile struct _0374{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_bsum5_t;

typedef volatile struct _0378{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_pcnt6_t;

typedef volatile struct _037C{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_rsum6_t;

typedef volatile struct _0380{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_gsum6_t;

typedef volatile struct _0384{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_bsum6_t;

typedef volatile struct _0388{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_pcnt7_t;

typedef volatile struct _038C{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_rsum7_t;

typedef volatile struct _0390{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_gsum7_t;

typedef volatile struct _0394{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_bsum7_t;

typedef volatile struct _0398{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_pcnt8_t;

typedef volatile struct _039C{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_rsum8_t;

typedef volatile struct _03A0{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_gsum8_t;

typedef volatile struct _03A4{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_bsum8_t;

typedef volatile struct _03A8{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_pcnt9_t;

typedef volatile struct _03AC{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_rsum9_t;

typedef volatile struct _03B0{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_gsum9_t;

typedef volatile struct _03B4{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_bsum9_t;

typedef volatile struct _03B8{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_pcnta_t;

typedef volatile struct _03BC{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_rsuma_t;

typedef volatile struct _03C0{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_gsuma_t;

typedef volatile struct _03C4{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_bsuma_t;

typedef volatile struct _03C8{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_pcntb_t;

typedef volatile struct _03CC{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_rsumb_t;

typedef volatile struct _03D0{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_gsumb_t;

typedef volatile struct _03D4{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbxy_bsumb_t;

typedef volatile struct _03D8{
    union {
        struct {
            MUINT32 BOTTOM     :8;
            MUINT32 TOP     :8;
            MUINT32 RIGHT     :8;
            MUINT32 LEFT     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_afwin0_t;

typedef volatile struct _03DC{
    union {
        struct {
            MUINT32 BOTTOM     :8;
            MUINT32 TOP     :8;
            MUINT32 RIGHT     :8;
            MUINT32 LEFT     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_afwin1_t;

typedef volatile struct _03E0{
    union {
        struct {
            MUINT32 BOTTOM     :8;
            MUINT32 TOP     :8;
            MUINT32 RIGHT     :8;
            MUINT32 LEFT     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_afwin2_t;

typedef volatile struct _03E4{
    union {
        struct {
            MUINT32 BOTTOM     :8;
            MUINT32 TOP     :8;
            MUINT32 RIGHT     :8;
            MUINT32 LEFT     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_afwin3_t;

typedef volatile struct _03E8{
    union {
        struct {
            MUINT32 BOTTOM     :8;
            MUINT32 TOP     :8;
            MUINT32 RIGHT     :8;
            MUINT32 LEFT     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_afwin4_t;

typedef volatile struct _03EC{
    union {
        struct {
            MUINT32 BOTTOM     :8;
            MUINT32 TOP     :8;
            MUINT32 RIGHT     :8;
            MUINT32 LEFT     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_afwin5_t;

typedef volatile struct _03F0{
    union {
        struct {
            MUINT32 BOTTOM     :8;
            MUINT32 TOP     :8;
            MUINT32 RIGHT     :8;
            MUINT32 LEFT     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_afwin6_t;

typedef volatile struct _03F4{
    union {
        struct {
            MUINT32 BOTTOM     :8;
            MUINT32 TOP     :8;
            MUINT32 RIGHT     :8;
            MUINT32 LEFT     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_afwin7_t;

typedef volatile struct _03F8{
    union {
        struct {
            MUINT32 BOTTOM     :8;
            MUINT32 TOP     :8;
            MUINT32 RIGHT     :8;
            MUINT32 LEFT     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_afwin8_t;

#define CAM_AF0_SUM_SIZE 5
typedef volatile struct _03FC{
    union {
        struct {
            MUINT32 AF0_SUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af0_sum_t;

#define CAM_AF1_SUM_SIZE 5
typedef volatile struct _0410{
    union {
        struct {
            MUINT32 AF1_SUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af1_sum_t;

#define CAM_AF2_SUM_SIZE 5
typedef volatile struct _0424{
    union {
        struct {
            MUINT32 AF2_SUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af2_sum_t;

#define CAM_AF3_SUM_SIZE 5
typedef volatile struct _0438{
    union {
        struct {
            MUINT32 AF3_SUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af3_sum_t;

#define CAM_AF4_SUM_SIZE 5
typedef volatile struct _044C{
    union {
        struct {
            MUINT32 AF4_SUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af4_sum_t;

#define CAM_AF5_SUM_SIZE 5
typedef volatile struct _0460{
    union {
        struct {
            MUINT32 AF5_SUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af5_sum_t;

#define CAM_AF6_SUM_SIZE 5
typedef volatile struct _0474{
    union {
        struct {
            MUINT32 AF6_SUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af6_sum_t;

#define CAM_AF7_SUM_SIZE 5
typedef volatile struct _0488{
    union {
        struct {
            MUINT32 AF7_SUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af7_sum_t;

#define CAM_AF8_SUM_SIZE 5
typedef volatile struct _049C{
    union {
        struct {
            MUINT32 AF8_SUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af8_sum_t;

typedef volatile struct _04B0{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af0_gsum_t;

typedef volatile struct _04B4{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af1_gsum_t;

typedef volatile struct _04B8{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af2_gsum_t;

typedef volatile struct _04BC{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af3_gsum_t;

typedef volatile struct _04C0{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af4_gsum_t;

typedef volatile struct _04C4{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af5_gsum_t;

typedef volatile struct _04C8{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af6_gsum_t;

typedef volatile struct _04D0{
    union {
        struct {
            MUINT32 DATA     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_af7_gsum_t;

typedef volatile struct _04D4{
    union {
        struct {
            MUINT32 AF_TH0     :8;
            MUINT32 AF_TH1     :8;
            MUINT32 AF_TH2     :8;
            MUINT32 AF_TH3     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_afth0_t;

typedef volatile struct _04D8{
    union {
        struct {
            MUINT32 AF_TH4     :8;
            MUINT32 NONE0     :24;
        } bits;
        MUINT32 reg;
    } u;
}cam_afth1_t;

typedef volatile struct _04E0{
    union {
        struct {
            MUINT32 FD_AEWIN0R     :10;
            MUINT32 NONE0     :6;
            MUINT32 FD_AEWIN0L     :10;
            MUINT32 NONE1     :2;
            MUINT32 FD_AEWIN0_EN     :1;
            MUINT32 NONE2     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_fd_aewin0lr_t;

typedef volatile struct _04E4{
    union {
        struct {
            MUINT32 FD_AEWIN0D     :10;
            MUINT32 NONE0     :6;
            MUINT32 FD_AEWIN0U     :10;
            MUINT32 NONE1     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_fd_aewin0ud_t;

typedef volatile struct _04E8{
    union {
        struct {
            MUINT32 FD_AEWIN1R     :10;
            MUINT32 NONE0     :6;
            MUINT32 FD_AEWIN1L     :10;
            MUINT32 NONE1     :2;
            MUINT32 FD_AEWIN1_EN     :1;
            MUINT32 NONE2     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_fd_aewin1lr_t;

typedef volatile struct _04EC{
    union {
        struct {
            MUINT32 FD_AEWIN1D     :10;
            MUINT32 NONE0     :6;
            MUINT32 FD_AEWIN1U     :10;
            MUINT32 NONE1     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_fd_aewin1ud_t;

typedef volatile struct _04F0{
    union {
        struct {
            MUINT32 FD_AEWIN2R     :10;
            MUINT32 NONE0     :6;
            MUINT32 FD_AEWIN2L     :10;
            MUINT32 NONE1     :2;
            MUINT32 FD_AEWIN2_EN     :1;
            MUINT32 NONE2     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_fd_aewin2lr_t;

typedef volatile struct _04F4{
    union {
        struct {
            MUINT32 FD_AEWIN2D     :10;
            MUINT32 NONE0     :6;
            MUINT32 FD_AEWIN2U     :10;
            MUINT32 NONE1     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_fd_aewin2ud_t;

typedef volatile struct _04F8{
    union {
        struct {
            MUINT32 FD_AEWIN3R     :10;
            MUINT32 NONE0     :6;
            MUINT32 FD_AEWIN3L     :10;
            MUINT32 NONE1     :2;
            MUINT32 FD_AEWIN3_EN     :1;
            MUINT32 NONE2     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_fd_aewin3lr_t;

typedef volatile struct _04FC{
    union {
        struct {
            MUINT32 FD_AEWIN3D     :10;
            MUINT32 NONE0     :6;
            MUINT32 FD_AEWIN3U     :10;
            MUINT32 NONE1     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_fd_aewin3ud_t;

typedef volatile struct _0500{
    union {
        struct {
            MUINT32 ENY     :1;
            MUINT32 ENC     :1;
            MUINT32 NONE0     :14;
            MUINT32 MODE     :1;
            MUINT32 NONE1     :3;
            MUINT32 IIR_MODE     :2;
            MUINT32 NONE2     :10;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr2_con_t;

typedef volatile struct _0504{
    union {
        struct {
            MUINT32 Y_DP_MIN_TH     :5;
            MUINT32 NONE0     :3;
            MUINT32 Y_DP_MAX_TH     :5;
            MUINT32 NONE1     :2;
            MUINT32 Y_DP_EN     :1;
            MUINT32 YUV444_MODE     :1;
            MUINT32 NONE2     :2;
            MUINT32 S_SM_EDGE     :1;
            MUINT32 QEC     :1;
            MUINT32 NONE3     :11;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr2_cfg_c1_t;

typedef volatile struct _0508{
    union {
        struct {
            MUINT32 GNC     :4;
            MUINT32 GNY     :4;
            MUINT32 SC1     :4;
            MUINT32 SY1     :4;
            MUINT32 S3     :3;
            MUINT32 S2     :3;
            MUINT32 UV_SMPL     :2;
            MUINT32 NONE0     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr2_cfg2_t;

typedef volatile struct _050C{
    union {
        struct {
            MUINT32 PTY4     :8;
            MUINT32 PTY3     :8;
            MUINT32 PTY2     :8;
            MUINT32 PTY1     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr2_cfg3_t;

typedef volatile struct _0510{
    union {
        struct {
            MUINT32 PTC4     :8;
            MUINT32 PTC3     :8;
            MUINT32 PTC2     :8;
            MUINT32 PTC1     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr2_cfg4_t;

typedef volatile struct _0514{
    union {
        struct {
            MUINT32 LUMA_AVG     :1;
            MUINT32 LUMA_LUTY5     :5;
            MUINT32 LUMA_LUTY4     :5;
            MUINT32 LUMA_LUTY3     :5;
            MUINT32 LUMA_LUTY2     :5;
            MUINT32 LUMA_LUTY1     :5;
            MUINT32 NONE0     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr2_cfg_c2_t;

typedef volatile struct _0518{
    union {
        struct {
            MUINT32 LCE_GAIN3     :6;
            MUINT32 NONE0     :2;
            MUINT32 LCE_GAIN2     :6;
            MUINT32 NONE1     :2;
            MUINT32 LCE_GAIN1     :6;
            MUINT32 NONE2     :2;
            MUINT32 LCE_GAIN0     :6;
            MUINT32 NONE3     :1;
            MUINT32 LCE_LINK     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr2_cfg_l1_t;

typedef volatile struct _051C{
    union {
        struct {
            MUINT32 LCE_GAIN_DIV3     :5;
            MUINT32 NONE0     :3;
            MUINT32 LCE_GAIN_DIV2     :5;
            MUINT32 NONE1     :3;
            MUINT32 LCE_GAIN_DIV1     :5;
            MUINT32 NONE2     :3;
            MUINT32 LCE_GAIN_DIV0     :5;
            MUINT32 NONE3     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr2_cfg_l2_t;

typedef volatile struct _0520{
    union {
        struct {
            MUINT32 GNC_H     :4;
            MUINT32 GNY_H     :4;
            MUINT32 Y_V_FLT4     :2;
            MUINT32 Y_V_FLT3     :2;
            MUINT32 Y_V_FLT2     :2;
            MUINT32 Y_V_FLT1     :2;
            MUINT32 C_V_FLT5     :2;
            MUINT32 C_V_FLT4     :2;
            MUINT32 C_V_FLT3     :2;
            MUINT32 C_V_FLT2     :2;
            MUINT32 C_V_FLT1     :2;
            MUINT32 NONE0     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr2_cfg_n1_t;

typedef volatile struct _0524{
    union {
        struct {
            MUINT32 Y_H_FLT6     :2;
            MUINT32 Y_H_FLT5     :2;
            MUINT32 Y_H_FLT4     :2;
            MUINT32 Y_H_FLT3     :2;
            MUINT32 Y_H_FLT2     :2;
            MUINT32 Y_H_FLT1     :2;
            MUINT32 NONE0     :20;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr2_cfg_n2_t;

typedef volatile struct _0528{
    union {
        struct {
            MUINT32 H_PTY4     :8;
            MUINT32 H_PTY3     :8;
            MUINT32 H_PTY2     :8;
            MUINT32 H_PTY1     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr2_cfg_n3_t;

typedef volatile struct _0540{
    union {
        struct {
            MUINT32 REV0     :1;
            MUINT32 REV1     :1;
            MUINT32 REV2     :2;
            MUINT32 REV3     :1;
            MUINT32 NONE0     :27;
        } bits;
        MUINT32 reg;
    } u;
}cam_hstcon_t;

typedef volatile struct _0544{
    union {
        struct {
            MUINT32 REV0     :12;
            MUINT32 NONE0     :4;
            MUINT32 REV1     :12;
            MUINT32 NONE1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_hstcfg1_t;

typedef volatile struct _0548{
    union {
        struct {
            MUINT32 REV0     :12;
            MUINT32 NONE0     :4;
            MUINT32 REV1     :12;
            MUINT32 NONE1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_hstcfg2_t;

typedef volatile struct _054C{
    union {
        struct {
            MUINT32 REV0     :6;
            MUINT32 NONE0     :2;
            MUINT32 REV1     :6;
            MUINT32 NONE1     :18;
        } bits;
        MUINT32 reg;
    } u;
}cam_hstcfg3_t;

typedef volatile struct _0550{
    union {
        struct {
            MUINT32 DP_EN     :1;
            MUINT32 CT_EN     :1;
            MUINT32 NR_EN     :1;
            MUINT32 BEN     :1;
            MUINT32 BMD     :1;
            MUINT32 BOP     :1;
            MUINT32 CT_MD     :2;
            MUINT32 Reserved1     :1;
            MUINT32 BSFT     :1;
            MUINT32 HALFT_THRPT     :1;
            MUINT32 NONE0     :21;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_con_t;

typedef volatile struct _0554{
    union {
        struct {
            MUINT32 DP_THRD1     :10;
            MUINT32 NONE0     :2;
            MUINT32 DP_THRD3     :4;
            MUINT32 DP_THRD0     :10;
            MUINT32 NONE1     :2;
            MUINT32 DP_THRD2     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_dp1_t;

typedef volatile struct _0558{
    union {
        struct {
            MUINT32 DP_THRD5     :10;
            MUINT32 NONE0     :6;
            MUINT32 DP_THRD4     :10;
            MUINT32 NONE1     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_dp2_t;

typedef volatile struct _055C{
    union {
        struct {
            MUINT32 DP_THRD7     :10;
            MUINT32 NONE0     :6;
            MUINT32 DP_THRD6     :10;
            MUINT32 NONE1     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_dp3_t;

typedef volatile struct _0560{
    union {
        struct {
            MUINT32 DP_CD1     :1;
            MUINT32 DP_CD2     :1;
            MUINT32 DP_CD3     :1;
            MUINT32 DP_CD4     :1;
            MUINT32 DP_CD5     :1;
            MUINT32 DP_CD6     :1;
            MUINT32 DP_CD7     :1;
            MUINT32 DP_SEL     :3;
            MUINT32 DP_NUM     :3;
            MUINT32 NONE0     :19;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_dp4_t;

typedef volatile struct _0564{
    union {
        struct {
            MUINT32 CT_THRD     :8;
            MUINT32 CT_DIV     :1;
            MUINT32 NONE0     :3;
            MUINT32 CT_MD2     :1;
            MUINT32 NONE1     :19;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_ct_t;

typedef volatile struct _0568{
    union {
        struct {
            MUINT32 MBND     :10;
            MUINT32 S2     :3;
            MUINT32 S1     :3;
            MUINT32 GNF     :5;
            MUINT32 GNF_EDGE     :5;
            MUINT32 S3     :3;
            MUINT32 NONE0     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_nr1_t;

typedef volatile struct _056C{
    union {
        struct {
            MUINT32 GN3     :4;
            MUINT32 GN2     :4;
            MUINT32 GN1     :4;
            MUINT32 NONE0     :20;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_nr2_t;

typedef volatile struct _0570{
    union {
        struct {
            MUINT32 VLR4     :6;
            MUINT32 NONE0     :2;
            MUINT32 VLR3     :6;
            MUINT32 NONE1     :2;
            MUINT32 VLR2     :6;
            MUINT32 NONE2     :2;
            MUINT32 VLR1     :6;
            MUINT32 NONE3     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_nr3_t;

typedef volatile struct _0574{
    union {
        struct {
            MUINT32 VLR8     :6;
            MUINT32 NONE0     :2;
            MUINT32 VLR7     :6;
            MUINT32 NONE1     :2;
            MUINT32 VLR6     :6;
            MUINT32 NONE2     :2;
            MUINT32 VLR5     :6;
            MUINT32 NONE3     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_nr4_t;

typedef volatile struct _0578{
    union {
        struct {
            MUINT32 VLGR4     :6;
            MUINT32 NONE0     :2;
            MUINT32 VLGR3     :6;
            MUINT32 NONE1     :2;
            MUINT32 VLGR2     :6;
            MUINT32 NONE2     :2;
            MUINT32 VLGR1     :6;
            MUINT32 NONE3     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_nr5_t;

typedef volatile struct _057C{
    union {
        struct {
            MUINT32 VLGR8     :6;
            MUINT32 NONE0     :2;
            MUINT32 VLGR7     :6;
            MUINT32 NONE1     :2;
            MUINT32 VLGR6     :6;
            MUINT32 NONE2     :2;
            MUINT32 VLGR5     :6;
            MUINT32 NONE3     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_nr6_t;

typedef volatile struct _0580{
    union {
        struct {
            MUINT32 VLGB4     :6;
            MUINT32 NONE0     :2;
            MUINT32 VLGB3     :6;
            MUINT32 NONE1     :2;
            MUINT32 VLGB2     :6;
            MUINT32 NONE2     :2;
            MUINT32 VLGB1     :6;
            MUINT32 NONE3     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_nr7_t;

typedef volatile struct _0584{
    union {
        struct {
            MUINT32 VLGB8     :6;
            MUINT32 NONE0     :2;
            MUINT32 VLGB7     :6;
            MUINT32 NONE1     :2;
            MUINT32 VLGB6     :6;
            MUINT32 NONE2     :2;
            MUINT32 VLGB5     :6;
            MUINT32 NONE3     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_nr8_t;

typedef volatile struct _0588{
    union {
        struct {
            MUINT32 VLR4     :6;
            MUINT32 NONE0     :2;
            MUINT32 VLR3     :6;
            MUINT32 NONE1     :2;
            MUINT32 VLR2     :6;
            MUINT32 NONE2     :2;
            MUINT32 VLR1     :6;
            MUINT32 NONE3     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_nr9_t;

typedef volatile struct _058C{
    union {
        struct {
            MUINT32 VLR8     :6;
            MUINT32 NONE0     :2;
            MUINT32 VLR7     :6;
            MUINT32 NONE1     :2;
            MUINT32 VLR6     :6;
            MUINT32 NONE2     :2;
            MUINT32 VLR5     :6;
            MUINT32 NONE3     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_nr10_t;

typedef volatile struct _0590{
    union {
        struct {
            MUINT32 EDGE128     :5;
            MUINT32 NONE0     :3;
            MUINT32 EDGE96     :5;
            MUINT32 NONE1     :3;
            MUINT32 EDGE64     :5;
            MUINT32 NONE2     :3;
            MUINT32 EDGE32     :5;
            MUINT32 NONE3     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_nr11_t;

typedef volatile struct _0594{
    union {
        struct {
            MUINT32 EDGE256     :5;
            MUINT32 NONE0     :3;
            MUINT32 EDGE224     :5;
            MUINT32 NONE1     :3;
            MUINT32 EDGE192     :5;
            MUINT32 NONE2     :3;
            MUINT32 EDGE160     :5;
            MUINT32 NONE3     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_nr12_t;

typedef volatile struct _0598{
    union {
        struct {
            MUINT32 BHT     :12;
            MUINT32 NONE0     :4;
            MUINT32 BWD     :12;
            MUINT32 NONE1     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_nr1_bsz_t;

typedef volatile struct _05A0{
    union {
        struct {
            MUINT32 YEDGE_EN     :1;
            MUINT32 RGBEDGE_EN     :1;
            MUINT32 CLIP_OVER_EN     :1;
            MUINT32 CLIP_UNDER_EN     :1;
            MUINT32 FILTER_SEL     :1;
            MUINT32 ED_GM_EN     :1;
            MUINT32 ED_BOUND_EN     :1;
            MUINT32 NONE0     :1;
            MUINT32 CLIP_OVER_TH     :8;
            MUINT32 CLIP_UNDER_TH     :8;
            MUINT32 NONE1     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_ee_ctrl_t;

typedef volatile struct _05A4{
    union {
        struct {
            MUINT32 USM_ED_X1     :8;
            MUINT32 USM_ED_S1     :8;
            MUINT32 USM_ED_Y1     :10;
            MUINT32 NONE0     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_edctrl1_t;

typedef volatile struct _05A8{
    union {
        struct {
            MUINT32 USM_ED_X2     :8;
            MUINT32 USM_ED_S2     :8;
            MUINT32 USM_ED_Y2     :10;
            MUINT32 NONE0     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_edctrl2_t;

typedef volatile struct _05AC{
    union {
        struct {
            MUINT32 USM_ED_X3     :8;
            MUINT32 USM_ED_S3     :8;
            MUINT32 USM_ED_Y3     :10;
            MUINT32 NONE0     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_edctrl3_t;

typedef volatile struct _05B0{
    union {
        struct {
            MUINT32 USM_ED_X4     :8;
            MUINT32 USM_ED_S4     :8;
            MUINT32 USM_ED_Y4     :10;
            MUINT32 NONE0     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_edctrl4_t;

typedef volatile struct _05B4{
    union {
        struct {
            MUINT32 USM_ED_DIAG_AMP     :3;
            MUINT32 USM_ED_2DFILTER_EN     :1;
            MUINT32 NONE0     :4;
            MUINT32 USM_ED_S5     :8;
            MUINT32 USM_ED_TH_OVER     :8;
            MUINT32 USM_ED_TH_UNDER     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_edctrl5_t;

typedef volatile struct _05B8{
    union {
        struct {
            MUINT32 CORE_CON     :7;
            MUINT32 TOP_SLOPE     :1;
            MUINT32 SUP_H     :2;
            MUINT32 SDN_H     :2;
            MUINT32 SPECIAL_EN     :1;
            MUINT32 NONE0     :3;
            MUINT32 COREH2     :6;
            MUINT32 EMBOSS2_EN     :1;
            MUINT32 EMBOSS1_EN     :1;
            MUINT32 COREH     :7;
            MUINT32 NONE1     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_edgcore_t;

typedef volatile struct _05BC{
    union {
        struct {
            MUINT32 EGAINLINE     :4;
            MUINT32 KNEESEL     :2;
            MUINT32 REV0     :1;
            MUINT32 OILEN     :1;
            MUINT32 EGAIN_VB     :5;
            MUINT32 REV1     :3;
            MUINT32 EGAIN_H2     :5;
            MUINT32 REV2     :3;
            MUINT32 EGAIN_H     :4;
            MUINT32 SPECIPONLY     :2;
            MUINT32 SPECIGAIN     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_edggain1_t;

typedef volatile struct _05C0{
    union {
        struct {
            MUINT32 EGAIN_HC     :5;
            MUINT32 NONE0     :1;
            MUINT32 SPECIINV     :1;
            MUINT32 SPECIABS     :1;
            MUINT32 NONE1     :8;
            MUINT32 EGAIN_VC     :5;
            MUINT32 NONE2     :3;
            MUINT32 EGAIN_VA     :4;
            MUINT32 NONE3     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_edggain2_t;

typedef volatile struct _05C4{
    union {
        struct {
            MUINT32 THRL_EDGE_SUP     :7;
            MUINT32 NONE0     :1;
            MUINT32 THRE_EDGE_SUP     :7;
            MUINT32 ONLYC     :1;
            MUINT32 ETH_CON     :8;
            MUINT32 ETH3     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_edgthre_t;

typedef volatile struct _05C8{
    union {
        struct {
            MUINT32 E_TH3_V     :8;
            MUINT32 SDN_V     :2;
            MUINT32 SUP_V     :2;
            MUINT32 REV0     :3;
            MUINT32 VPEN     :1;
            MUINT32 HALF_V     :6;
            MUINT32 REV1     :2;
            MUINT32 E_TH1_V     :7;
            MUINT32 HPEN     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_edgvcon_t;

typedef volatile struct _05CC{
    union {
        struct {
            MUINT32 EGAMMA_B4     :8;
            MUINT32 EGAMMA_B3     :8;
            MUINT32 EGAMMA_B2     :8;
            MUINT32 EGAMMA_B1     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_egamma1_t;

typedef volatile struct _05D0{
    union {
        struct {
            MUINT32 EGAMMA_B8     :8;
            MUINT32 EGAMMA_B7     :8;
            MUINT32 EGAMMA_B6     :8;
            MUINT32 EGAMMA_B5     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_egamma2_t;

typedef volatile struct _05D4{
    union {
        struct {
            MUINT32 NONE0     :8;
            MUINT32 EGAMMA_B11     :8;
            MUINT32 EGAMMA_B10     :8;
            MUINT32 EGAMMA_B9     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_egamma3_t;

typedef volatile struct _0600{
    union {
        struct {
            MUINT32 ENC3     :1;
            MUINT32 ENC2     :1;
            MUINT32 ENC1     :1;
            MUINT32 ENY3     :1;
            MUINT32 ENY2     :1;
            MUINT32 ENY1     :1;
            MUINT32 NONE0     :2;
            MUINT32 ENBNDV     :1;
            MUINT32 ENBNDU     :1;
            MUINT32 ENBNDY     :1;
            MUINT32 NONE1     :5;
            MUINT32 UV_GAIN_ON     :1;
            MUINT32 NONE2     :15;
        } bits;
        MUINT32 reg;
    } u;
}cam_yccgo_con_t;

typedef volatile struct _0604{
    union {
        struct {
            MUINT32 H12     :8;
            MUINT32 H11     :8;
            MUINT32 MV     :8;
            MUINT32 MU     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_yccgo_cfg1_t;

typedef volatile struct _0608{
    union {
        struct {
            MUINT32 Y4     :8;
            MUINT32 Y3     :8;
            MUINT32 Y2     :8;
            MUINT32 Y1     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_yccgo_cfg2_t;

typedef volatile struct _060C{
    union {
        struct {
            MUINT32 G4     :8;
            MUINT32 G3     :8;
            MUINT32 G2     :8;
            MUINT32 G1     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_yccgo_cfg3_t;

typedef volatile struct _0610{
    union {
        struct {
            MUINT32 OFSTV     :8;
            MUINT32 OFSTU     :8;
            MUINT32 OFSTY     :8;
            MUINT32 G5     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_yccgo_cfg4_t;

typedef volatile struct _0614{
    union {
        struct {
            MUINT32 GAINY     :8;
            MUINT32 NONE0     :8;
            MUINT32 YBNDL     :8;
            MUINT32 YBNDH     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_yccgo_cfg5_t;

typedef volatile struct _0618{
    union {
        struct {
            MUINT32 VBNDL     :8;
            MUINT32 VBNDH     :8;
            MUINT32 UBNDL     :8;
            MUINT32 UBNDH     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_yccgo_cfg6_t;

typedef volatile struct _061C{
    union {
        struct {
            MUINT32 UV_GAIN1     :7;
            MUINT32 NONE0     :1;
            MUINT32 UV_GAIN2     :7;
            MUINT32 NONE1     :1;
            MUINT32 UV_GAIN3     :7;
            MUINT32 NONE2     :1;
            MUINT32 UV_GAIN4     :7;
            MUINT32 NONE3     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_yccgo_cfg7_t;

typedef volatile struct _0620{
    union {
        struct {
            MUINT32 UV_GAIN_SLOPE1     :7;
            MUINT32 NONE0     :1;
            MUINT32 UV_GAIN_SLOPE2     :7;
            MUINT32 NONE1     :1;
            MUINT32 UV_GAIN_SLOPE3     :7;
            MUINT32 NONE2     :9;
        } bits;
        MUINT32 reg;
    } u;
}cam_yccgo_cfg8_t;

typedef volatile struct _0624{
    union {
        struct {
            MUINT32 UV_X1     :7;
            MUINT32 NONE0     :1;
            MUINT32 UV_X2     :7;
            MUINT32 NONE1     :1;
            MUINT32 UV_X3     :7;
            MUINT32 NONE2     :9;
        } bits;
        MUINT32 reg;
    } u;
}cam_yccgo_cfg9_t;

typedef volatile struct _0630{
    union {
        struct {
            MUINT32 EN     :1;
            MUINT32 NONE0     :3;
            MUINT32 BUSY_SKIP     :1;
            MUINT32 LUT_360     :1;
            MUINT32 NONE1     :2;
            MUINT32 PCA_BUSY     :1;
            MUINT32 NONE2     :7;
            MUINT32 CHROMA_TH     :5;
            MUINT32 NONE3     :11;
        } bits;
        MUINT32 reg;
    } u;
}cam_pca_con_t;

typedef volatile struct _0634{
    union {
        struct {
            MUINT32 TABLE_ADDR     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_pca_tba_t;

typedef volatile struct _0638{
    union {
        struct {
            MUINT32 THROTTLE     :16;
            MUINT32 NONE0     :15;
            MUINT32 THEN     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_pca_gmc_t;
//
typedef volatile struct _0640{
    union {
        struct {
            //RCM CAM+640
            MUINT32 COM_ENC_MODE     : 2; //[1:0]
            MUINT32 COM_DEC_MODE     : 2; //[3:2]
            MUINT32 ISP_CODEC_SEL    : 1; //[4]
            MUINT32 ISP_CODEC_DEC    : 1; //[5]
            MUINT32 COM_CODEC_EN     : 1; //[6]
            MUINT32 REV     :1;
            MUINT32 NONE0     :24;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_con_t;
//
typedef volatile struct _0644{
    union {
        struct {
            //RAD CAM+644
            MUINT32 COM_RESULT_ADDR  : 32; //[31:0]
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_outaddr_t;
//
typedef volatile struct _0648{
    union {
        struct {
            //RLAD CAM+648
            MUINT32 COM_RESULT_LEN_ADDR : 32; //[31:0]
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_outlen_t;
//
typedef volatile struct _064C{
    union {
        struct {
            //BAD CAM+64C
            MUINT32 COM_BASE_ADDR    : 32; //[31:0]
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_baddr_t;
//
typedef volatile struct _0650{
    union {
        struct {
            //BLAD CAM+650
            MUINT32 COM_BASE_LEN_ADDR: 32; //[31:0]
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_blen_t;
//
typedef volatile struct _0654{
    union {
        struct {
            //ENC1 CAM+654
            MUINT32 COM_ENC_FSC_CLIP : 1; //[0]
            MUINT32 NONE0     :1;
            MUINT32 COM_ENC_INIT_MARGIN : 14; //[15:2]
            MUINT32 COM_ENC_FSC_STRIDE  : 14; //[29:16]
            MUINT32 NONE1     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_enctrl1_t;
//
typedef volatile struct _0658{
    union {
        struct {
            //ENC2 CAM+658
            MUINT32 COM_ENC_FSC_QUANT_INIT   : 3; //[2:0]
            MUINT32 NONE0     :1;
            MUINT32 COM_ENC_FSC_INIT_VAL0    : 16; //[19:4]
            MUINT32 NONE1     :8;
            MUINT32 COM_ENC_FSC_QUANT0       : 3; //[30:28]
            MUINT32 NONE2     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_enctrl2_t;
//
typedef volatile struct _065C{
    union {
        struct {
            //ENC3 CAM+65C
            MUINT32 COM_ENC_FSC_TARG_BYTE0   : 8; //[7:0]
            MUINT32 COM_ENC_FSC_QUANT1       : 3; //[10:8]
            MUINT32 NONE0     :5;
            MUINT32 COM_ENC_FSC_TARG_BYTE1   : 8; //[23:16]
            MUINT32 COM_ENC_FSC_QUANT2       : 3; //[26:24]
            MUINT32 NONE1     :5;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_enctrl3_t;
//
typedef volatile struct _0660{
    union {
        struct {
            //ENC4 CAM+660
            MUINT32 COM_ENC_FSC_TARG_BYTE2   : 8; //[7:0]
            MUINT32 COM_ENC_FSC_QUANT3       : 3; //[10:8]
            MUINT32 NONE0     :5;
            MUINT32 COM_ENC_FSC_TARG_BYTE3   : 8; //[23:16]
            MUINT32 COM_ENC_FSC_QUANT4       : 3; //[26:24]
            MUINT32 NONE1     :5;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_enctrl4_t;
//
typedef volatile struct _0664{
    union {
        struct {
            //ENC5 CAM+664
            MUINT32 COM_ENC_FSC_TARG_BYTE4   : 8; //[7:0]
            MUINT32 COM_ENC_FSC_QUANT5       : 3; //[10:8]
            MUINT32 NONE0     :5;
            MUINT32 COM_ENC_FSC_TARG_BYTE5   : 8; //[23:16]
            MUINT32 NONE1     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_enctrl5_t;
//
typedef volatile struct _0668{
    union {
        struct {
            //ENC6 CAM+668
            MUINT32 COM_ENC_T1       : 10; //[9:0]
            MUINT32 NONE0     :6;
            MUINT32 COM_ENC_T2       : 10; //[25:16]
            MUINT32 NONE1     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_enctrl6_t;
//
typedef volatile struct _066C{
    union {
        struct {
            //ENC7 CAM+66C
            MUINT32 COM_ENC_T3       : 10; //[9:0]
            MUINT32 NONE0     :22;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_enctrl7_t;
//
typedef volatile struct _0670{
    union {
        struct {
            //DEC1 CAM+670
            MUINT32 COM_DEC_FSC_CLIP : 1; //[0]
            MUINT32 NONE0     :15;
            MUINT32 COM_DEC_FSC_STRIDE : 14; //[29:16]
            MUINT32 NONE1     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_dectrl1_t;
//
typedef volatile struct _0674{
    union {
        struct {
            //DEC2 CAM+674
            MUINT32 COM_DEC_FSC_INIT_VAL0: 16; //[15:0]
            MUINT32 COM_DEC_FSC_QUANT0   : 3; //[18:16]
            MUINT32 NONE0     :1;
            MUINT32 COM_DEC_FSC_QUANT1   : 3; //[22:20]
            MUINT32 NONE1     :1;
            MUINT32 COM_DEC_FSC_QUANT2   : 3; //[26:24]
            MUINT32 NONE2     :1;
            MUINT32 COM_DEC_FSC_QUANT3   : 3; //[30:28]
            MUINT32 NONE3     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_dectrl2_t;
//
typedef volatile struct _0678{
    union {
        struct {
            //DEC3 CAM+678
            MUINT32 COM_DEC_FSC_QUANT4   : 3; //[2:0]
            MUINT32 NONE0     :1;
            MUINT32 COM_DEC_FSC_QUANT5   : 3; //[6:4]
            MUINT32 NONE1     :9;
            MUINT32 COM_DEC_T1           : 10; //[25:16]
            MUINT32 NONE2     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_dectrl3_t;
//
typedef volatile struct _067C{
    union {
        struct {
            //DEC4 CAM+67C
            MUINT32 COM_DEC_T2       : 10; //[9:0]
            MUINT32 NONE0     :6;
            MUINT32 COM_DEC_T3       : 10; //[25:16]
            MUINT32 NONE1     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawc_dectrl4_t;

typedef volatile struct _0680{
    union {
        struct {
            MUINT32 STATUS     :1;
            MUINT32 NONE0     :31;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_status_t;

typedef volatile struct _0684{
    union {
        struct {
            MUINT32 STATUS     :1;
            MUINT32 NONE0     :31;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_intsta_t;

typedef volatile struct _0688{
    union {
        struct {
            MUINT32 ENABLE     :1;
            MUINT32 NONE0     :31;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_intena_t;

typedef volatile struct _068C{
    union {
        struct {
            MUINT32 ENABLE     :1;
            MUINT32 RESET     :1;
            MUINT32 NONE0     :6;
            MUINT32 OUTPUT_SEL     :2;
            MUINT32 NONE1     :6;
            MUINT32 RESERVE     :2;
            MUINT32 NONE2     :14;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_control_t;

typedef volatile struct _0690{
    union {
        struct {
            MUINT32 IMG_WIDTH     :13;
            MUINT32 IMG_HEIGHT     :13;
            MUINT32 LB_CONTROL     :1;
            MUINT32 NONE0     :5;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_cfg_t;

typedef volatile struct _0694{
    union {
        struct {
            MUINT32 HSFT     :10;
            MUINT32 NONE0     :6;
            MUINT32 VSFT     :10;
            MUINT32 NONE1     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_offset_t;

typedef volatile struct _0698{
    union {
        struct {
            MUINT32 RATIO16     :5;
            MUINT32 NONE0     :3;
            MUINT32 RATIO32     :5;
            MUINT32 NONE1     :3;
            MUINT32 RATIO48     :5;
            MUINT32 NONE2     :3;
            MUINT32 RATIO64     :5;
            MUINT32 NONE3     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_edge1_t;

typedef volatile struct _069C{
    union {
        struct {
            MUINT32 RATIO80     :5;
            MUINT32 NONE0     :3;
            MUINT32 RATIO96     :5;
            MUINT32 NONE1     :3;
            MUINT32 RATIO112     :5;
            MUINT32 NONE2     :3;
            MUINT32 RATIO128     :5;
            MUINT32 NONE3     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_edge2_t;

typedef volatile struct _06A0{
    union {
        struct {
            MUINT32 BASE_TH1     :6;
            MUINT32 BASE_TH2     :6;
            MUINT32 BASE_RATIO1     :5;
            MUINT32 BASE_RATIO2     :5;
            MUINT32 S1     :3;
            MUINT32 S2     :3;
            MUINT32 NONE0     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_snr1_t;

typedef volatile struct _06A4{
    union {
        struct {
            MUINT32 INPUT_TH1     :6;
            MUINT32 INPUT_TH2     :6;
            MUINT32 INPUT_RATIO1     :5;
            MUINT32 INPUT_RATIO2     :5;
            MUINT32 NONE0     :10;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_snr2_t;

typedef volatile struct _06A8{
    union {
        struct {
            MUINT32 TNR_SAD     :8;
            MUINT32 SAD_LPF     :1;
            MUINT32 INPUT_SEL     :2;
            MUINT32 NONE0     :21;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_tnr_t;

typedef volatile struct _06AC{
    union {
        struct {
            MUINT32 LPF     :1;
            MUINT32 NEW_METHOD     :1;
            MUINT32 HALF_PENALTY     :2;
            MUINT32 DC_PENALTY     :2;
            MUINT32 RATIO_IIR     :2;
            MUINT32 BR_THRESHOLD     :5;
            MUINT32 NONE0     :19;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_pm_t;

typedef volatile struct _06B0{
    union {
        struct {
            MUINT32 ORDER     :1;
            MUINT32 RATIO     :6;
            MUINT32 NONE0     :25;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_golden_t;

typedef volatile struct _06B4{
    union {
        struct {
            MUINT32 ADDR     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_base1_t;

typedef volatile struct _06B8{
    union {
        struct {
            MUINT32 START_ADDR     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_base2_t;

typedef volatile struct _06BC{
    union {
        struct {
            MUINT32 WRAP     :13;
            MUINT32 NONE0     :19;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_base3_t;

typedef volatile struct _06C0{
    union {
        struct {
            MUINT32 ADDR     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_input0_t;

typedef volatile struct _06C4{
    union {
        struct {
            MUINT32 ADDR     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_input1_t;

typedef volatile struct _06C8{
    union {
        struct {
            MUINT32 START_ADDR     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_input2_t;

typedef volatile struct _06CC{
    union {
        struct {
            MUINT32 WRAP     :13;
            MUINT32 NONE0     :19;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_input3_t;

typedef volatile struct _06D0{
    union {
        struct {
            MUINT32 LEN_ADDR     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_input4_t;

typedef volatile struct _06D4{
    union {
        struct {
            MUINT32 ADDR     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_result1_t;

typedef volatile struct _06D8{
    union {
        struct {
            MUINT32 START_ADDR     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_result2_t;

typedef volatile struct _06DC{
    union {
        struct {
            MUINT32 WRAP     :13;
            MUINT32 NONE0     :19;
        } bits;
        MUINT32 reg;
    } u;
}cam_stnr_result3_t;

typedef volatile struct _0800{
    union {
        struct {
            MUINT32 TG_TOP_CHKSUM_EN     :1;
            MUINT32 NONE0     :3;
            MUINT32 LENS_COMP_CHKSUM_EN     :1;
            MUINT32 DEFECT_CHKSUM_EN     :1;
            MUINT32 NR1_CHKSUM_EN     :1;
            MUINT32 SHADING_CHKSUM_EN     :1;
            MUINT32 NR2OUT_CHKSUM_EN     :1;
            MUINT32 NR2IN_CHKSUM_EN     :1;
            MUINT32 NONE1     :18;
            MUINT32 CAM_EIS_CHKSUM_EN     :1;
            MUINT32 NONE2     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_chksum_ctrl_t;

typedef volatile struct _0804{
    union {
        struct {
            MUINT32 TG_TOP_CHKSUM     :10;
            MUINT32 NONE0     :6;
            MUINT32 LENS_COMP_CHKSUM     :10;
            MUINT32 NONE1     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_chksum_sta0_t;

typedef volatile struct _0808{
    union {
        struct {
            MUINT32 NR2IN_CHKSUM     :24;
            MUINT32 NONE0     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_chksum_sta1_t;

typedef volatile struct _080C{
    union {
        struct {
            MUINT32 NR2OUT_CHKSUM     :24;
            MUINT32 NONE0     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_chksum_sta2_t;

typedef volatile struct _0810{
    union {
        struct {
            MUINT32 SHADDING_CHKSUM     :10;
            MUINT32 NONE0     :6;
            MUINT32 DEFFECT_CHKSUM     :10;
            MUINT32 NONE1     :6;
        } bits;
        MUINT32 reg;
    } u;
}cam_chksum_sta3_t;

typedef volatile struct _0814{
    union {
        struct {
            MUINT32 NR1_CHKSUM     :10;
            MUINT32 NONE0     :22;
        } bits;
        MUINT32 reg;
    } u;
}cam_chksum_sta4_t;

typedef volatile struct _0818{
    union {
        struct {
            MUINT32 CAM_EIS_CHKSUM     :10;
            MUINT32 NONE0     :22;
        } bits;
        MUINT32 reg;
    } u;
}cam_chksum_sta5_t;

typedef volatile struct _0C00{
    union {
        struct {
            MUINT32 TGCLK_SEL     :2;
            MUINT32 CLKFL_POL     :1;
            MUINT32 NONE0     :1;
            MUINT32 EXT_RST     :1;
            MUINT32 EXT_PWRDN     :1;
            MUINT32 PAD_PCLK_INV     :1;
            MUINT32 CAM_PCLK_INV     :1;
            MUINT32 NONE1     :20;
            MUINT32 CLKPOL     :1;
            MUINT32 ADCLK_EN     :1;
            MUINT32 NONE2     :1;
            MUINT32 PCEN     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_ph_cnt_t;

typedef volatile struct _0C04{
    union {
        struct {
            MUINT32 CLKFL     :6;
            MUINT32 NONE0     :2;
            MUINT32 CLKRS     :6;
            MUINT32 NONE1     :2;
            MUINT32 CLKCNT     :6;
            MUINT32 NONE2     :10;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_sen_ck_t;

typedef volatile struct _0C08{
    union {
        struct {
            MUINT32 CMOS_EN     :1;
            MUINT32 DBL_DATA_BUS     :1;
            MUINT32 SOT_MODE     :1;
            MUINT32 SOT_CLR_MODE     :1;
            MUINT32 VSPOL     :1;
            MUINT32 HSPOL     :1;
            MUINT32 SEN_3D     :1;
            MUINT32 NONE0     :1;
            MUINT32 SOF_SRC     :2;
            MUINT32 EOF_SRC     :2;
            MUINT32 PXL_CNT_RST_SRC     :1;
            MUINT32 NONE1     :19;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_sen_mode_t;

typedef volatile struct _0C0C{
    union {
        struct {
            MUINT32 VFDATA_EN     :1;
            MUINT32 SINGLE_MODE     :1;
            MUINT32 NONE0     :2;
            MUINT32 FR_CON     :3;
            MUINT32 NONE1     :1;
            MUINT32 SP_DELAY     :3;
            MUINT32 NONE2     :1;
            MUINT32 SPDELAY_MODE     :1;
            MUINT32 NONE3     :19;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_vf_con_t;

typedef volatile struct _0C10{
    union {
        struct {
            MUINT32 PXL_S     :15;
            MUINT32 NONE0     :1;
            MUINT32 PXL_E     :15;
            MUINT32 NONE1     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_sen_grab_pxl_t;

typedef volatile struct _0C14{
    union {
        struct {
            MUINT32 LIN_S     :13;
            MUINT32 NONE0     :3;
            MUINT32 LIN_E     :13;
            MUINT32 NONE1     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_sen_grab_lin_t;

typedef volatile struct _0C18{
    union {
        struct {
            MUINT32 SEN_IN_LSB     :2;
            MUINT32 NONE0     :2;
            MUINT32 JPGINF_EN     :1;
            MUINT32 MEMIN_EN     :1;
            MUINT32 VDOIN_EN     :1;
            MUINT32 JPG_LINEND_EN     :1;
            MUINT32 DB_LOAD_DIS     :1;
            MUINT32 DB_LOAD_SRC     :1;
            MUINT32 NONE1     :2;
            MUINT32 YUV_U2S_DIS     :1;
            MUINT32 NONE2     :19;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_path_cfg_t;

typedef volatile struct _0C1C{
    union {
        struct {
            MUINT32 MEMIN_DUMMYPXL     :8;
            MUINT32 MEMIN_DUMMYLIN     :5;
            MUINT32 NONE0     :19;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_memin_ctl_t;

typedef volatile struct _0C20{
    union {
        struct {
            MUINT32 TG_INT1_LINENO     :13;
            MUINT32 NONE0     :3;
            MUINT32 TG_INT1_PXLNO     :15;
            MUINT32 VSYNC_INT_POL     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_int1_t;

typedef volatile struct _0C24{
    union {
        struct {
            MUINT32 TG_INT2_LINENO     :13;
            MUINT32 NONE0     :3;
            MUINT32 TG_INT2_PXLNO     :15;
            MUINT32 NONE1     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_int2_t;

typedef volatile struct _0C28{
    union {
        struct {
            MUINT32 SOF_CNT     :28;
            MUINT32 NONE0     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_sof_cnt_t;

typedef volatile struct _0C2C{
    union {
        struct {
            MUINT32 SOT_CNT     :28;
            MUINT32 NONE0     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_sot_cnt_t;

typedef volatile struct _0C30{
    union {
        struct {
            MUINT32 EOT_CNT     :28;
            MUINT32 NONE0     :4;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_eot_cnt_t;

typedef volatile struct _0C34{
    union {
        struct {
            MUINT32 GRAB_ERR_FLIMIT_NO     :4;
            MUINT32 GRAB_ERR_FLIMIT_EN     :1;
            MUINT32 GRAB_ERR_EN     :1;
            MUINT32 NONE0     :2;
            MUINT32 REZ_OVRUN_FLIMIT_NO     :4;
            MUINT32 REZ_OVRUN_FLIMIT_EN     :1;
            MUINT32 NONE1     :19;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_err_ctl_t;

typedef volatile struct _0C38{
    union {
        struct {
            MUINT32 DAT_NO     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_dat_no_t;

typedef volatile struct _0C40{
    union {
        struct {
            MUINT32 REZ_OVRUN_FCNT     :4;
            MUINT32 NONE0     :4;
            MUINT32 GRAB_ERR_FCNT     :4;
            MUINT32 NONE1     :20;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_frm_cnt_st_t;

typedef volatile struct _0C44{
    union {
        struct {
            MUINT32 LINE_CNT     :13;
            MUINT32 NONE0     :3;
            MUINT32 PXL_CNT     :15;
            MUINT32 NONE1     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_frmsize_st_t;

typedef volatile struct _0C48{
    union {
        struct {
            MUINT32 SYN_VF_DATA_EN     :1;
            MUINT32 NONE0     :7;
            MUINT32 TG_CAM_CS     :6;
            MUINT32 NONE1     :2;
            MUINT32 CAM_FRM_CNT     :8;
            MUINT32 NONE2     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_inter_st_t;

typedef volatile struct _0C4C{
    union {
        struct {
            MUINT32 LINE_CNT     :13;
            MUINT32 NONE0     :3;
            MUINT32 PXL_CNT     :15;
            MUINT32 NONE1     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_sen_frmsize_st_t;

typedef volatile struct _0C60{
    union {
        struct {
            MUINT32 TM_EN     :1;
            MUINT32 TM_RST     :1;
            MUINT32 TM_FMT     :1;
            MUINT32 NONE0     :1;
            MUINT32 TM_PAT     :4;
            MUINT32 TM_VSYNC     :8;
            MUINT32 TM_DUMMYPXL     :8;
            MUINT32 NONE1     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_tm_ctl_t;

typedef volatile struct _0C64{
    union {
        struct {
            MUINT32 TM_PXL     :13;
            MUINT32 NONE0     :3;
            MUINT32 TM_LINE     :13;
            MUINT32 NONE1     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_tm_size_t;

typedef volatile struct _0C68{
    union {
        struct {
            MUINT32 TM_CLK_CNT     :4;
            MUINT32 NONE0     :28;
        } bits;
        MUINT32 reg;
    } u;
}cam_tg2_tm_clk_t;

typedef volatile struct _0CF0{
    union {
        struct {
            MUINT32 V_SUB_OUT     :13;
            MUINT32 NONE0     :3;
            MUINT32 V_SUB_IN     :13;
            MUINT32 NONE1     :2;
            MUINT32 V_SUB_EN     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam2_drop_vsub_t;

typedef volatile struct _0CF4{
    union {
        struct {
            MUINT32 H_SUB_OUT     :13;
            MUINT32 NONE0     :3;
            MUINT32 H_SUB_IN     :13;
            MUINT32 NONE1     :2;
            MUINT32 H_SUB_EN     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam2_drop_hsub_t;

typedef volatile struct _0D04{
    union {
        struct {
            MUINT32 TG1_EN     :1;
            MUINT32 TG2_EN     :1;
            MUINT32 G2C_1_EN     :1;
            MUINT32 G2C_2_EN     :1;
            MUINT32 C42_1_EN     :1;
            MUINT32 C42_2_EN     :1;
            MUINT32 CCR_1_EN     :1;
            MUINT32 CCR_2_EN     :1;
            MUINT32 PCA_EN     :1;
            MUINT32 NONE0     :1;
            MUINT32 C24D_1_EN     :1;
            MUINT32 C24D_2_EN     :1;
            MUINT32 DROP_1_EN     :1;
            MUINT32 DROP_2_EN     :1;
            MUINT32 FIFO_1_EN     :1;
            MUINT32 FIFO_2_EN     :1;
            MUINT32 NONE1     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctl_mod_en_t;

typedef volatile struct _0D08{
    union {
        struct {
            MUINT32 TG1_FMT     :3;
            MUINT32 NONE0     :1;
            MUINT32 TG2_FMT     :3;
            MUINT32 NONE1     :25;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctl_fmt_sel_t;

typedef volatile struct _0D0C{
    union {
        struct {
            MUINT32 TG1_SW     :2;
            MUINT32 TG2_SW     :2;
            MUINT32 NONE0     :28;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctl_swap_t;

typedef volatile struct _0D10{
    union {
        struct {
            MUINT32 LMU_SEL     :1;
            MUINT32 LMU_RZ_SEL     :1;
            MUINT32 PCA_SEL     :1;
            MUINT32 NONE0     :1;
            MUINT32 COLOR1_SEL     :1;
            MUINT32 COLOR2_SEL     :1;
            MUINT32 CRZ_SEL     :1;
            MUINT32 NONE1     :1;
            MUINT32 CRZ_DISCONN     :1;
            MUINT32 PRZ_DISCONN     :1;
            MUINT32 IMGDMA_SEL     :1;
            MUINT32 NONE2     :21;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctl_sel_t;

typedef volatile struct _0D14{
    union {
        struct {
            MUINT32 NONE0     :8;
            MUINT32 CAMPRZ_INIT_PERIOD     :4;
            MUINT32 CAMPRZ_INIT_EN     :1;
            MUINT32 NONE1     :11;
            MUINT32 CAMCRZ_INIT_PERIOD     :4;
            MUINT32 CAMCRZ_INIT_EN     :1;
            MUINT32 NONE2     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctl_crz_t;

typedef volatile struct _0D20{
    union {
        struct {
            MUINT32 VS1_EN     :1;
            MUINT32 TG1_EN1     :1;
            MUINT32 TG1_EN2     :1;
            MUINT32 EXPDON1_EN     :1;
            MUINT32 TG1_GRAB_ERR_EN     :1;
            MUINT32 VS2_EN     :1;
            MUINT32 TG2_EN1     :1;
            MUINT32 TG2_EN2     :1;
            MUINT32 EXPDON2_EN     :1;
            MUINT32 TG2_GRAB_ERR_EN     :1;
            MUINT32 COLOR1_DON_EN     :1;
            MUINT32 COLOR2_DON_EN     :1;
            MUINT32 CRZ_OVRUN_EN     :1;
            MUINT32 PRZ_OVRUN_EN     :1;
            MUINT32 NONE0     :17;
            MUINT32 INT_WCLR_EN     :1;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctl_int_en_t;

typedef volatile struct _0D24{
    union {
        struct {
            MUINT32 VS1_ST     :1;
            MUINT32 TG1_ST1     :1;
            MUINT32 TG1_ST2     :1;
            MUINT32 EXPDON1_ST     :1;
            MUINT32 TG1_GRAP_ERR_ST     :1;
            MUINT32 VS2_ST     :1;
            MUINT32 TG2_ST1     :1;
            MUINT32 TG2_ST2     :1;
            MUINT32 EXPDON2_ST     :1;
            MUINT32 TG2_GRAP_ERR_ST     :1;
            MUINT32 COLOR1_DON_ST     :1;
            MUINT32 COLOR2_DON_ST     :1;
            MUINT32 CRZ_OVRUN_ST     :1;
            MUINT32 PRZ_OVRUN_ST     :1;
            MUINT32 NONE0     :18;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctl_int_status_t;

typedef volatile struct _0D28{
    union {
        struct {
            MUINT32 ALL_HW_RST     :1;
            MUINT32 APB_RST     :1;
            MUINT32 HW_RST     :1;
            MUINT32 NONE0     :29;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctl_sw_ctl_t;

typedef volatile struct _0D2C{
    union {
        struct {
            MUINT32 NONE0     :12;
            MUINT32 LMU_BAYER_MODE     :1;
            MUINT32 NONE1     :3;
            MUINT32 LMU_MUX     :1;
            MUINT32 NONE2     :3;
            MUINT32 LMU_SUB_OFST     :2;
            MUINT32 NONE3     :2;
            MUINT32 LMU_SUBSAMPLE     :2;
            MUINT32 NONE4     :2;
            MUINT32 LMU_EN     :1;
            MUINT32 NONE5     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_lmu_ctrl_t;

typedef volatile struct _0D30{
    union {
        struct {
            MUINT32 DEBUG_MOD_SEL     :4;
            MUINT32 DEBUG_SEL     :8;
            MUINT32 DEBUG_TOP_SEL     :4;
            MUINT32 NONE0     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctl_dbg_set_t;

typedef volatile struct _0D34{
    union {
        struct {
            MUINT32 CTL_DBG_PORT     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctl_dbg_port_t;

typedef volatile struct _0D38{
    union {
        struct {
            MUINT32 DATE     :8;
            MUINT32 MONTH     :8;
            MUINT32 YEAR     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctl_version_t;

typedef volatile struct _0D3C{
    union {
        struct {
            MUINT32 CTL_PROJ     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_ctl_proj_t;

typedef volatile struct _1000{
    union {
        struct {
            MUINT32 AESUM0_23_0     :24;
            MUINT32 AESUM1_7_0     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem0_t;

typedef volatile struct _1004{
    union {
        struct {
            MUINT32 AESUM1_23_8     :16;
            MUINT32 AESUM2_15_0     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem1_t;

typedef volatile struct _1008{
    union {
        struct {
            MUINT32 AESUM2_23_16     :8;
            MUINT32 AESUM3_23_0     :24;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem2_t;

typedef volatile struct _100C{
    union {
        struct {
            MUINT32 AESUM4     :24;
            MUINT32 AE     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem3_t;

typedef volatile struct _1010{
    union {
        struct {
            MUINT32 AESUM5_23_0     :24;
            MUINT32 AESUM6_7_0     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem4_t;

typedef volatile struct _1014{
    union {
        struct {
            MUINT32 AESUM6_23_8     :16;
            MUINT32 AESUM7_15_0     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem5_t;

typedef volatile struct _1018{
    union {
        struct {
            MUINT32 AESUM7_23_16     :8;
            MUINT32 AESUM8_23_0     :24;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem6_t;

typedef volatile struct _101C{
    union {
        struct {
            MUINT32 AESUM9     :24;
            MUINT32 AE     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem7_t;

typedef volatile struct _1020{
    union {
        struct {
            MUINT32 AESUM10_23_0     :24;
            MUINT32 AESUM11_7_0     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem8_t;

typedef volatile struct _1024{
    union {
        struct {
            MUINT32 AESUM11_23_8     :16;
            MUINT32 AESUM12_15_0     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem9_t;

typedef volatile struct _1028{
    union {
        struct {
            MUINT32 AESUM12_23_16     :8;
            MUINT32 AESUM13_23_0     :24;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem10_t;

typedef volatile struct _102C{
    union {
        struct {
            MUINT32 AESUM14     :24;
            MUINT32 AE     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem11_t;

typedef volatile struct _1030{
    union {
        struct {
            MUINT32 AESUM15_23_0     :24;
            MUINT32 AESUM16_7_0     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem12_t;

typedef volatile struct _1034{
    union {
        struct {
            MUINT32 AESUM16_23_8     :16;
            MUINT32 AESUM17_15_0     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem13_t;

typedef volatile struct _1038{
    union {
        struct {
            MUINT32 AESUM17_23_16     :8;
            MUINT32 AESUM18_23_0     :24;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem14_t;

typedef volatile struct _103C{
    union {
        struct {
            MUINT32 AESUM19     :24;
            MUINT32 AE     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem15_t;

typedef volatile struct _1040{
    union {
        struct {
            MUINT32 AESUM20_23_0     :24;
            MUINT32 AESUM21_7_0     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem16_t;

typedef volatile struct _1044{
    union {
        struct {
            MUINT32 AESUM21_23_8     :16;
            MUINT32 AESUM22_15_0     :16;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem17_t;

typedef volatile struct _1048{
    union {
        struct {
            MUINT32 AESUM22_23_16     :8;
            MUINT32 AESUM23_23_0     :24;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem18_t;

typedef volatile struct _104C{
    union {
        struct {
            MUINT32 AESUM24     :24;
            MUINT32 AE     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_aemem19_t;

typedef volatile struct _1050{
    union {
        struct {
            MUINT32 AEBLK_COUNT     :24;
            MUINT32 NONE0     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_aeblk_t;

typedef volatile struct _1054{
    union {
        struct {
            MUINT32 BAYERHEIGHT     :12;
            MUINT32 NONE0     :4;
            MUINT32 BAYERWIDTH     :13;
            MUINT32 NONE1     :3;
        } bits;
        MUINT32 reg;
    } u;
}cam_rawsize_t;

typedef volatile struct _1058{
    union {
        struct {
            MUINT32 AWB_BMIN     :5;
            MUINT32 AWB_BMAX     :5;
            MUINT32 AWB_GMIN     :5;
            MUINT32 NONE0     :1;
            MUINT32 AWB_GMAX     :4;
            MUINT32 AWB_RMIN     :5;
            MUINT32 AWB_RMAX     :5;
            MUINT32 NONE1     :2;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbdebug0_t;

typedef volatile struct _105C{
    union {
        struct {
            MUINT32 AWBXYWIN_RANGE     :12;
            MUINT32 WHITE_FLAG     :1;
            MUINT32 NONE0     :19;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbdebug1_t;

#define CAM_FLAREMEM_SIZE 9
typedef volatile struct _1060{
    union {
        struct {
            MUINT32 FLAREB0_9     :22;
            MUINT32 NONE0     :10;
        } bits;
        MUINT32 reg;
    } u;
}cam_flaremem_t;

typedef volatile struct _1084{
    union {
        struct {
            MUINT32 FLAREB9     :22;
            MUINT32 NONE0     :2;
            MUINT32 AFTAG     :8;
        } bits;
        MUINT32 reg;
    } u;
}cam_flaremem9_t;

#define CAM_AFN_SIZE 45
typedef volatile struct _1088{
    union {
        struct {
            MUINT32 AFN_SUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_afn_t;

#define CAM_AFGSUM_SIZE 9
typedef volatile struct _113C{
    union {
        struct {
            MUINT32 AFGSUMN     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_afgsum_t;

#define CAM_AWBNXY_CNT_SIZE 48
typedef volatile struct _1160{
    union {
        struct {
            MUINT32 AWBXY_DAT     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbnxy_cnt_t;

typedef volatile struct _1220{
    union {
        struct {
            MUINT32 AWBSUM_COUNT     :22;
            MUINT32 NONE0     :10;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbsum_cnt_t;

typedef volatile struct _1224{
    union {
        struct {
            MUINT32 AWBSUM_RSUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbsum_rsum_m_t;

typedef volatile struct _1228{
    union {
        struct {
            MUINT32 AWBSUM_GSUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbsum_gsum_m_t;

typedef volatile struct _122C{
    union {
        struct {
            MUINT32 AWBSUM_BSUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbsum_bsum_m_t;

typedef volatile struct _1230{
    union {
        struct {
            MUINT32 AWBOUT_CECOUNT     :22;
            MUINT32 NONE0     :10;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbce_count_t;

typedef volatile struct _1234{
    union {
        struct {
            MUINT32 AWBCE_RSUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbce_rsum_m_t;

typedef volatile struct _1238{
    union {
        struct {
            MUINT32 AWBCE_GSUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbce_gsum_m_t;

typedef volatile struct _123C{
    union {
        struct {
            MUINT32 AWBCE_BSUM     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_awbce_bsum_m_t;

#define CAM_AEHIS_SIZE 64
typedef volatile struct _1240{
    union {
        struct {
            MUINT32 AEHIS     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_aehis_t;

#define CAM_FDAESUM_SIZE 4
typedef volatile struct _1340{
    union {
        struct {
            MUINT32 FD_AESUM0_3     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_fdaesum_t;

#define CAM_FDAECNT_SIZE 4
typedef volatile struct _1350{
    union {
        struct {
            MUINT32 FD_AECNT0_3     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_fdaecnt_t;

#define CAM_FDAWBRSUM_SIZE 4
typedef volatile struct _1360{
    union {
        struct {
            MUINT32 FD_RSUM0_3     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_fdawbrsum_t;

#define CAM_FDAWBGSUM_SIZE 4
typedef volatile struct _1370{
    union {
        struct {
            MUINT32 FD_GSUM0_3     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_fdawbgsum_t;

#define CAM_FDAWBBSUM_SIZE 4
typedef volatile struct _1380{
    union {
        struct {
            MUINT32 FD_BSUM0_3     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_fdawbbsum_t;

#define CAM_FDAWBCNT_SIZE 4
typedef volatile struct _1390{
    union {
        struct {
            MUINT32 FD_AWBCNT0_3     :32;
        } bits;
        MUINT32 reg;
    } u;
}cam_fdawbcnt_t;

typedef volatile struct _2000{
    union {
        struct {
            MUINT32 CSI2_EN     :1;
            MUINT32 DLANE1_EN     :1;
            MUINT32 DLANE2_EN     :1;
            MUINT32 DLANE3_EN     :1;
            MUINT32 CSI2_ECC_EN     :1;
            MUINT32 CSI2_ED_SEL     :1;
            MUINT32 CSI2_CLK_MISS_EN     :1;
            MUINT32 CSI2_LP11_RST_EN     :1;
            MUINT32 CSI2_SYNC_RST_EN     :1;
            MUINT32 CSI2_ESC_EN     :1;
            MUINT32 CSI2_SCLK_SEL     :1;
            MUINT32 CSI2_SCLK4X_SEL     :1;
            MUINT32 CSI2_SW_RST     :1;
            MUINT32 CSI2_VSYNC_TYPE     :1;
            MUINT32 CSI2_HSRXEN_PFOOT_CLR     :1;
            MUINT32 CSI2_SYNC_CLR_EXTEND     :1;
            MUINT32 CSI2_ASYNC_OPTION     :1;
            MUINT32 CSI2_DATA_FLOW     :2;
            MUINT32 CSI2_BIST_ERROR_COUNT     :8;
            MUINT32 CSI2_BIST_START     :1;
            MUINT32 CSI2_BIST_DATA_OK     :1;
            MUINT32 CSI2_HS_FSM_OK     :1;
            MUINT32 CSI2_LANE_FSM_OK     :1;
            MUINT32 CSI2_BIST_CSI2_DATA_OK     :1;
        } bits;
        MUINT32 reg;
    } u;
}csi2_ctrl_t;

typedef volatile struct _2004{
    union {
        struct {
            MUINT32 LP2HS_CLK_TERM_DELAY     :8;
            MUINT32 NONE0     :8;
            MUINT32 LP2HS_DATA_SETTLE_DELAY     :8;
            MUINT32 LP2HS_DATA_TERM_DELAY     :8;
        } bits;
        MUINT32 reg;
    } u;
}csi2_delay_t;

typedef volatile struct _2008{
    union {
        struct {
            MUINT32 CRC_ERR_IRQ_EN     :1;
            MUINT32 ECC_ERR_IRQ_EN     :1;
            MUINT32 ECC_CORRECT_IRQ_EN     :1;
            MUINT32 CSI2SYNC_NONSYNC_IRQ_EN     :1;
            MUINT32 NONE0     :4;
            MUINT32 CSI2_WC_NUMBER     :16;
            MUINT32 CSI2_DATA_TYPE     :6;
            MUINT32 VCHANNEL_ID     :2;
        } bits;
        MUINT32 reg;
    } u;
}csi2_inten_t;

typedef volatile struct _200C{
    union {
        struct {
            MUINT32 CRC_ERR_IRQ     :1;
            MUINT32 ECC_ERR_IRQ     :1;
            MUINT32 ECC_CORRECT_IRQ     :1;
            MUINT32 CSI2SYNC_NONSYNC_IRQ     :1;
            MUINT32 CSI2_IRQ_CLR_SEL     :1;
            MUINT32 CSI2_SPARE     :3;
            MUINT32 NONE0     :12;
            MUINT32 CSI2OUT_HSYNC     :1;
            MUINT32 CSI2OUT_VSYNC     :1;
            MUINT32 NONE1     :10;
        } bits;
        MUINT32 reg;
    } u;
}csi2_intsta_t;

typedef volatile struct _2010{
    union {
        struct {
            MUINT32 CSI2_ECCDB_EN     :1;
            MUINT32 NONE0     :7;
            MUINT32 CSI2_ECCDB_BSEL     :24;
        } bits;
        MUINT32 reg;
    } u;
}csi2_eccdbg_t;

typedef volatile struct _2014{
    union {
        struct {
            MUINT32 CSI2_CRCDB_EN     :1;
            MUINT32 CSI2_SPARE     :7;
            MUINT32 CSI2_CRCDB_WSEL     :16;
            MUINT32 CSI2_CRCDB_BSEL     :8;
        } bits;
        MUINT32 reg;
    } u;
}csi2_crcdbg_t;

typedef volatile struct _2018{
    union {
        struct {
            MUINT32 CSI2_DEBUG_ON     :1;
            MUINT32 CSI2_DBG_SRC_SEL     :4;
            MUINT32 CSI2_DATA_HS_CS     :6;
            MUINT32 CSI2_CLK_LANE_CS     :5;
            MUINT32 NONE0     :6;
            MUINT32 LNC_LPRXDB_EN     :1;
            MUINT32 LN0_LPRXDB_EN     :1;
            MUINT32 LN1_LPRXDB_EN     :1;
            MUINT32 LN2_LPRXDB_EN     :1;
            MUINT32 LN3_LPRXDB_EN     :1;
            MUINT32 LNC_HSRXDB_EN     :1;
            MUINT32 LN0_HSRXDB_EN     :1;
            MUINT32 LN1_HSRXDB_EN     :1;
            MUINT32 LN2_HSRXDB_EN     :1;
            MUINT32 LN3_HSRXDB_EN     :1;
        } bits;
        MUINT32 reg;
    } u;
}csi2_dbg_t;

typedef volatile struct _201C{
    union {
        struct {
            MUINT32 DATE     :8;
            MUINT32 MONTH     :8;
            MUINT32 YEAR     :16;
        } bits;
        MUINT32 reg;
    } u;
}csi2_ver_t;

typedef volatile struct _2020{
    union {
        struct {
            MUINT32 CSI2_LINE_NO     :16;
            MUINT32 CSI2_FRAME_NO     :16;
        } bits;
        MUINT32 reg;
    } u;
}csi2_short_info_t;

typedef volatile struct _2024{
    union {
        struct {
            MUINT32 CSI2_DATA_LN0_CS     :7;
            MUINT32 NONE0     :1;
            MUINT32 CSI2_DATA_LN1_CS     :7;
            MUINT32 NONE1     :17;
        } bits;
        MUINT32 reg;
    } u;
}csi2_lnfsm_t;

typedef volatile struct _2028{
    union {
        struct {
            MUINT32 CSI2_DATA_LN0_MUX     :2;
            MUINT32 CSI2_DATA_LN1_MUX     :2;
            MUINT32 CSI2_DATA_LN2_MUX     :2;
            MUINT32 CSI2_DATA_LN3_MUX     :2;
            MUINT32 NONE0     :24;
        } bits;
        MUINT32 reg;
    } u;
}csi2_lnmux_t;

typedef volatile struct _202C{
    union {
        struct {
            MUINT32 CSI2_HSYNC_CNT     :13;
            MUINT32 NONE0     :19;
        } bits;
        MUINT32 reg;
    } u;
}csi2_hsync_cnt_t;

typedef volatile struct _2030{
    union {
        struct {
            MUINT32 CSI2_CAL_EN     :1;
            MUINT32 NONE0     :3;
            MUINT32 CSI2_CAL_STATE     :3;
            MUINT32 NONE1     :9;
            MUINT32 CSI2_CAL_CNT_1     :8;
            MUINT32 CSI2_CAL_CNT_2     :8;
        } bits;
        MUINT32 reg;
    } u;
}csi2_cal_t;

typedef volatile struct _8000{
    union {
        struct {
            MUINT32 INTEN0      : 1;
            MUINT32 INTEN1      : 1;
            MUINT32 INTEN2      : 1;
            MUINT32 INTEN3      : 1;
            MUINT32 INTEN4      : 1;
            MUINT32 INTEN5      : 1;
            MUINT32 INTEN6      : 1;
            MUINT32 NONE0       : 1;
            MUINT32 CYC         : 3;
            MUINT32 NONE1       : 1;
            MUINT32 CLK_INV     : 1;
            MUINT32 NONE2       : 3;
            MUINT32 SIF_SEL     : 1;
            MUINT32 CON         : 1;
            MUINT32 NONE3       : 2;
            MUINT32 DBG_MD      : 1;
            MUINT32 NONE4       : 7;
            MUINT32 WARN_MASK   : 1;
            MUINT32 NONE5       : 3;
        } bits;
        MUINT32 reg;
    } u;
} scam1_cfg_t;

typedef volatile struct _8004{
    union {
        struct {
            MUINT32 ENA     : 1;
            MUINT32 NONE0   : 15;
            MUINT32 RST     : 1;
            MUINT32 NONE1   : 15;
        } bits;
        MUINT32 reg;
    } u;
} scam1_con_t;

typedef volatile struct _800C{
    union {
        struct {
            MUINT32 INT0: 1;
            MUINT32 INT1: 1;
            MUINT32 INT2: 1;
            MUINT32 INT3: 1;
            MUINT32 INT4: 1;
            MUINT32 INT5: 1;
            MUINT32 INT6: 1;
            MUINT32 NONE0: 25;
        } bits;
        MUINT32 reg;
    } u;
} scam1_int_t;

typedef volatile struct _8010{
    union {
        struct {
            MUINT32 WIDTH   : 12;
            MUINT32 NONE0   : 4;
            MUINT32 HEIGHT  : 12;
            MUINT32 NONE1   : 4;
        } bits;
        MUINT32 reg;
    } u;
} scam1_size_t;

typedef volatile struct _8020{
    union {
        struct {
            MUINT32 DIS_GATED_CLK   : 1;
            MUINT32 NONE0           : 31;
        } bits;
        MUINT32 reg;
    } u;
} scam1_cfg2_t;

typedef volatile struct _8030{
    union {
        struct {
            MUINT32 LINE_ID     : 16;
            MUINT32 PACKET_SIZE : 16;
        } bits;
        MUINT32 reg;
    } u;
} scam1_info0_t;

typedef volatile struct _8034{
    union {
        struct {
            MUINT32 NONE0       : 8;
            MUINT32 DATA_ID     : 6;
            MUINT32 CRC_ON      : 1;
            MUINT32 ACTIVE      : 1;
            MUINT32 DATA_CNT    : 16;
        } bits;
        MUINT32 reg;
    } u;
} scam1_info1_t;

typedef volatile struct _8040{
    union {
        struct {
            MUINT32 FEND_CNT    : 4;
            MUINT32 W_CRC_CNT   : 4;
            MUINT32 W_SYNC_CNT  : 4;
            MUINT32 W_PID_CNT   : 4;
            MUINT32 W_LID_CNT   : 4;
            MUINT32 W_DID_CNT   : 4;
            MUINT32 W_SIZE_CNT  : 4;
            MUINT32 NONE0       : 4;
        } bits;
        MUINT32 reg;
    } u;
} scam1_sta_t;

typedef volatile struct _8100{
    union {
        struct {
            MUINT32 INTEN0      : 1;
            MUINT32 INTEN1      : 1;
            MUINT32 INTEN2      : 1;
            MUINT32 INTEN3      : 1;
            MUINT32 INTEN4      : 1;
            MUINT32 INTEN5      : 1;
            MUINT32 INTEN6      : 1;
            MUINT32 NONE0       : 1;
            MUINT32 CYC         : 3;
            MUINT32 NONE1       : 1;
            MUINT32 CLK_INV     : 1;
            MUINT32 NONE2       : 3;
            MUINT32 SIF_SEL     : 1;
            MUINT32 CON         : 1;
            MUINT32 NONE3       : 2;
            MUINT32 DBG_MD      : 1;
            MUINT32 NONE4       : 7;
            MUINT32 WARN_MASK   : 1;
            MUINT32 NONE5       : 3;
        } bits;
        MUINT32 reg;
    } u;
} scam2_cfg_t;

typedef volatile struct _8104{
    union {
        struct {
            MUINT32 ENA     : 1;
            MUINT32 NONE0   : 15;
            MUINT32 RST     : 1;
            MUINT32 NONE1   : 15;
        } bits;
        MUINT32 reg;
    } u;
} scam2_con_t;

typedef volatile struct _810C{
    union {
        struct {
            MUINT32 INT0: 1;
            MUINT32 INT1: 1;
            MUINT32 INT2: 1;
            MUINT32 INT3: 1;
            MUINT32 INT4: 1;
            MUINT32 INT5: 1;
            MUINT32 INT6: 1;
            MUINT32 NONE0: 25;
        } bits;
        MUINT32 reg;
    } u;
} scam2_int_t;

typedef volatile struct _8110{
    union {
        struct {
            MUINT32 WIDTH   : 12;
            MUINT32 NONE0   : 4;
            MUINT32 HEIGHT  : 12;
            MUINT32 NONE1   : 4;
        } bits;
        MUINT32 reg;
    } u;
} scam2_size_t;

typedef volatile struct _8120{
    union {
        struct {
            MUINT32 DIS_GATED_CLK   : 1;
            MUINT32 NONE0           : 31;
        } bits;
        MUINT32 reg;
    } u;
} scam2_cfg2_t;

typedef volatile struct _8130{
    union {
        struct {
            MUINT32 LINE_ID     : 16;
            MUINT32 PACKET_SIZE : 16;
        } bits;
        MUINT32 reg;
    } u;
} scam2_info0_t;

typedef volatile struct _8134{
    union {
        struct {
            MUINT32 NONE0       : 8;
            MUINT32 DATA_ID     : 6;
            MUINT32 CRC_ON      : 1;
            MUINT32 ACTIVE      : 1;
            MUINT32 DATA_CNT    : 16;
        } bits;
        MUINT32 reg;
    } u;
} scam2_info1_t;

typedef volatile struct _8140{
    union {
        struct {
            MUINT32 FEND_CNT    : 4;
            MUINT32 W_CRC_CNT   : 4;
            MUINT32 W_SYNC_CNT  : 4;
            MUINT32 W_PID_CNT   : 4;
            MUINT32 W_LID_CNT   : 4;
            MUINT32 W_DID_CNT   : 4;
            MUINT32 W_SIZE_CNT  : 4;
            MUINT32 NONE0       : 4;
        } bits;
        MUINT32 reg;
    } u;
} scam2_sta_t;

//
typedef volatile struct {
    scam1_cfg_t     SCAM1_CFG;      // 0x8000
    scam1_con_t     SCAM1_CON;      // 0x8004
    MUINT32 RESERVE8008[1];         // 0x8008
    scam1_int_t     SCAM1_INT;      // 0x800C
    scam1_size_t    SCAM1_SIZE;     // 0x8010
    MUINT32 RESERVE8014[3];         // 0x8014, 0x8018, 0x801C
    scam1_cfg2_t    SCAM1_CFG2;     // 0x8020
    MUINT32 RESERVE8024[3];         // 0x8024, 0x8028, 0x802C
    scam1_info0_t   SCAM1_INFO0;    // 0x8030
    scam1_info1_t   SCAM1_INFO1;    // 0x8034
    MUINT32 RESERVE8038[2];         // 0x8038, 0x803C
    scam1_sta_t     SCAM1_STA;      // 0x8040

    MUINT32 RESERVE8044[29];        // 0x8044~0x809C

    scam2_cfg_t     SCAM2_CFG;      // 0x8100
    scam2_con_t     SCAM2_CON;      // 0x8104
    MUINT32 RESERVE8108[1];         // 0x8108
    scam2_int_t     SCAM2_INT;      // 0x810C
    scam2_size_t    SCAM2_SIZE;     // 0x8110
    MUINT32 RESERVE8114[3];         // 0x8114, 0x8118, 0x811C
    scam2_cfg2_t    SCAM2_CFG2;     // 0x8020
    MUINT32 RESERVE8124[3];         // 0x8124, 0x8128, 0x812C
    scam2_info0_t   SCAM2_INFO0;    // 0x8130
    scam2_info1_t   SCAM2_INFO1;    // 0x8134
    MUINT32 RESERVE8138[2];         // 0x8138, 0x813C
    scam2_sta_t     SCAM2_STA;      // 0x8140

} scam_reg_t;

//-------------------------------------------------//
//-       sean lin                                -//
//-       In order to use ISP_REG for ISP On.off  -//
//-------------------------------------------------//


typedef volatile struct _isp_reg_t { 
  cam_phscnt_t CAM_PHSCNT;                                    // 0x0000
  cam_camwin_t CAM_CAMWIN;
  cam_grabcol_t CAM_GRABCOL;
  cam_grabrow_t CAM_GRABROW;
  cam_csmode_t CAM_CSMODE;                                    // 0x0010
  cam_bcctrl_t CAM_BCCTRL;
  cam_vfcon_t CAM_VFCON;
  cam_inten_t CAM_INTEN;
  cam_intsta_t CAM_INTSTA;                                    // 0x0020
  cam_path_t CAM_PATH;
  cam_inaddr_t CAM_INADDR;
  cam_outaddr_t CAM_OUTADDR;
  cam_memin_t CAM_MEMIN;                                      // 0x0030
  cam_memout_t CAM_MEMOUT;
  cam_funsta_t CAM_FUNSTA;
  cam_3again_t CAM_3AGAIN;
  cam_ctrl1_t CAM_CTRL1;                                      // 0x0040
  cam_ctrl2_t CAM_CTRL2;
  cam_aewinh_t CAM_AEWINH;
  cam_aehiswin_t CAM_AEHISWIN;
  cam_aegain_t CAM_AEGAIN;                                    // 0x0050
  cam_gsctrl_t CAM_GSCTRL;
  cam_msctrl_t CAM_MSCTRL;
  cam_ms1time_t CAM_MS1TIME;
  cam_ms2time_t CAM_MS2TIME;                                  // 0x0060
  cam_rgbgain1_t CAM_RGBGAIN1;
  cam_rgbgain2_t CAM_RGBGAIN2;
  cam_awbwin_t CAM_AWBWIN;
  cam_cpscon1_t CAM_CPSCON1;                                  // 0x0070
  cam_inter1_t CAM_INTER1;
  cam_inter2_t CAM_INTER2;
  MUINT32 RESERVE007C[5];
  cam_rgboff_t CAM_RGBOFF;                                    // 0x0090
  cam_matrix1_t CAM_MATRIX1;
  cam_matrix2_t CAM_MATRIX2;
  cam_matrix3_t CAM_MATRIX3;
  cam_matrix4_t CAM_MATRIX4;                                  // 0x00A0
  cam_matrix5_t CAM_MATRIX5;
  MUINT32 RESERVE00A8[1];
  cam_cpscon2_t CAM_CPSCON2;
  cam_flregain_t CAM_FLREGAIN;                                // 0x00B0
  cam_flreoff_t CAM_FLREOFF;
  cam_ychan_t CAM_YCHAN;
  cam_rgb2ycc_con_t CAM_RGB2YCC_CON;
  MUINT32 RESERVE00C0[3];
  cam_cclip_con_t CAM_CCLIP_CON;
  cam_cclip_gtc_t CAM_CCLIP_GTC;                              // 0x00D0
  cam_cclip_adc_t CAM_CCLIP_ADC;
  cam_cclip_bac_t CAM_CCLIP_BAC;
  MUINT32 RESERVE00DC[1];
  cam_flk_con_t CAM_FLK_CON;                                  // 0x00E0
  cam_flk_intvl_t CAM_FLK_INTVL;
  cam_flk_gaddr_t CAM_FLK_GADDR;
  MUINT32 RESERVE00EC[1];
  cam_hisrlt_t CAM_HISRLT[CAM_HISRLT_SIZE];                   // 0x00F0
  MUINT32 RESERVE0118[1];
  cam_yuv422_t CAM_YUV422;
  cam_path2_t CAM_PATH2;                                      // 0x0120
  MUINT32 RESERVE0124[3];
  cam_rawout_cropv_t CAM_RAWOUT_CROPV;                        // 0x0130
  cam_rawout_croph_t CAM_RAWOUT_CROPH;
  MUINT32 RESERVE0138[7];
  cam_defect0_t CAM_DEFECT0;
  cam_defect1_t CAM_DEFECT1;
  cam_defect2_t CAM_DEFECT2;
  cam_adefectm_t CAM_ADEFECTM;                                // 0x0160
  MUINT32 RESERVE0164[2];
  cam_rawgain0_t CAM_RAWGAIN0;
  cam_rawgain1_t CAM_RAWGAIN1;                                // 0x0170
  cam_rwinv_sel_t CAM_RWINV_SEL;
  cam_rwinh_sel_t CAM_RWINH_SEL;
  MUINT32 RESERVE017C[1];
  cam_memindb_ctrl_t CAM_MEMINDB_CTRL;                        // 0x0180
  cam_tgsize_sta_t CAM_TGSIZE_STA;
  cam_lastaddr_t CAM_LASTADDR;
  cam_xfercnt_t CAM_XFERCNT;
  cam_mdlcfg1_t CAM_MDLCFG1;                                  // 0x0190
  cam_mdlcfg2_t CAM_MDLCFG2;
  MUINT32 RESERVE0198[2];
  camcrz_ctrl_t CAMCRZ_CTRL;                                  // 0x01A0
  camcrz_sta_t CAMCRZ_STA;
  cam_gma_reg1_t CAM_GMA_REG1;
  cam_gma_reg2_t CAM_GMA_REG2;
  cam_gma_reg3_t CAM_GMA_REG3;                                // 0x01B0
  cam_gma_reg4_t CAM_GMA_REG4;
  cam_gma_reg5_t CAM_GMA_REG5;
  cam_rawacc_reg_t CAM_RAWACC_REG;
  cam_rawwin_reg_t CAM_RAWWIN_REG;                            // 0x01C0
  cam_rawsum_b_t CAM_RAWSUM_B;
  cam_rawsum_gb_t CAM_RAWSUM_GB;
  cam_rawsum_gr_t CAM_RAWSUM_GR;
  cam_rawsum_r_t CAM_RAWSUM_R;                                // 0x01D0
  cam_tg_status_t CAM_TG_STATUS;
  cam_reset_t CAM_RESET;
  MUINT32 RESERVE01DC[1];
  cam_flash_ctrl0_t CAM_FLASH_CTRL0;                          // 0x01E0
  cam_flash_ctrl1_t CAM_FLASH_CTRL1;
  cam_flashb_ctrl0_t CAM_FLASHB_CTRL0;
  cam_flashb_ctrl1_t CAM_FLASHB_CTRL1;
  MUINT32 RESERVE01F0[2];
  cam_sd_dbg0_t CAM_SD_DBG0;
  cam_sd_dbg1_t CAM_SD_DBG1;
  cam_sd_dbg2_t CAM_SD_DBG2;                                  // 0x0200
  cam_sd_dbg3_t CAM_SD_DBG3;
  cam_sd_dbg4_t CAM_SD_DBG4;
  cam_sd_dbg5_t CAM_SD_DBG5;
  cam_gmcdebug_t CAM_GMCDEBUG;                                // 0x0210
  cam_shading1_t CAM_SHADING1;
  cam_shading2_t CAM_SHADING2;
  cam_sdraddr_t CAM_SDRADDR;
  cam_sdlblock_t CAM_SDLBLOCK;                                // 0x0220
  cam_sdratio_t CAM_SDRATIO;
  cam_tg1ctrl_t CAM_TG1CTRL;
  cam_tg2ctrl_t CAM_TG2CTRL;
  MUINT32 RESERVE0230[3];
  cam_lce_con_t CAM_LCE_CON;
  cam_lce_bc_sp_t CAM_LCE_BC_SP;                              // 0x0240
  cam_lce_bc_ep_t CAM_LCE_BC_EP;
  cam_lce_zr_t CAM_LCE_ZR;
  cam_lce_mem_sa1_t CAM_LCE_MEM_SA1;
  cam_lce_mem_sa2_t CAM_LCE_MEM_SA2;                          // 0x0250
  cam_lce_qua_t CAM_LCE_QUA;
  cam_lce_thr_t CAM_LCE_THR;
  cam_lrz_con_t CAM_LRZ_CON;
  cam_lrz_dbg_t CAM_LRZ_DBG;                                  // 0x0260
  cam_lrz_tar_t CAM_LRZ_TAR;
  cam_lrz_hdr_t CAM_LRZ_HDR;
  cam_lrz_vdr_t CAM_LRZ_VDR;
  MUINT32 RESERVE0270[1];
  cam_version_t CAM_VERSION;
  MUINT32 RESERVE0278[1];
  cam_awbsum_win_t CAM_AWBSUM_WIN;
  cam_awb_ctrl_t CAM_AWB_CTRL;                                // 0x0280
  cam_awbth_t CAM_AWBTH;
  cam_awbxyh1_t CAM_AWBXYH1;
  cam_awbxyh2_t CAM_AWBXYH2;
  cam_awbce_winh_t CAM_AWBCE_WINH;                            // 0x0290
  cam_awbce_winv_t CAM_AWBCE_WINV;
  cam_awbxywinh0_t CAM_AWBXYWINH0;
  cam_awbxywinv0_t CAM_AWBXYWINV0;
  cam_awbxywinh1_t CAM_AWBXYWINH1;                            // 0x02A0
  cam_awbxywinv1_t CAM_AWBXYWINV1;
  cam_awbxywinh2_t CAM_AWBXYWINH2;
  cam_awbxywinv2_t CAM_AWBXYWINV2;
  cam_awbxywinh3_t CAM_AWBXYWINH3;                            // 0x02B0
  cam_awbxywinv3_t CAM_AWBXYWINV3;
  cam_awbxywinh4_t CAM_AWBXYWINH4;
  cam_awbxywinv4_t CAM_AWBXYWINV4;
  cam_awbxywinh5_t CAM_AWBXYWINH5;                            // 0x02C0
  cam_awbxywinv5_t CAM_AWBXYWINV5;
  cam_awbxywinh6_t CAM_AWBXYWINH6;
  cam_awbxywinv6_t CAM_AWBXYWINV6;
  cam_awbxywinh7_t CAM_AWBXYWINH7;                            // 0x02D0
  cam_awbxywinv7_t CAM_AWBXYWINV7;
  cam_awbxywinh8_t CAM_AWBXYWINH8;
  cam_awbxywinv8_t CAM_AWBXYWINV8;
  cam_awbxywinh9_t CAM_AWBXYWINH9;                            // 0x02E0
  cam_awbxywinv9_t CAM_AWBXYWINV9;
  cam_awbxywinh10_t CAM_AWBXYWINH10;
  cam_awbxywinv10_t CAM_AWBXYWINV10;
  cam_awbxywinh11_t CAM_AWBXYWINH11;                          // 0x02F0
  cam_awbxywinv11_t CAM_AWBXYWINV11;
  cam_awbsum_pcnt_t CAM_AWBSUM_PCNT;
  cam_awbsum_rsum_t CAM_AWBSUM_RSUM;
  cam_awbsum_gsum_t CAM_AWBSUM_GSUM;                          // 0x0300
  cam_awbsum_bsum_t CAM_AWBSUM_BSUM;
  cam_awbce_pcnt_t CAM_AWBCE_PCNT;
  cam_awbce_rsum_t CAM_AWBCE_RSUM;
  cam_awbce_gsum_t CAM_AWBCE_GSUM;                            // 0x0310
  cam_awbce_bsum_t CAM_AWBCE_BSUM;
  cam_awbxy_pcnt0_t CAM_AWBXY_PCNT0;
  cam_awbxy_rsum0_t CAM_AWBXY_RSUM0;
  cam_awbxy_gsum0_t CAM_AWBXY_GSUM0;                          // 0x0320
  cam_awbxy_bsum0_t CAM_AWBXY_BSUM0;
  cam_awbxy_pcnt1_t CAM_AWBXY_PCNT1;
  cam_awbxy_rsum1_t CAM_AWBXY_RSUM1;
  cam_awbxy_gsum1_t CAM_AWBXY_GSUM1;                          // 0x0330
  cam_awbxy_bsum1_t CAM_AWBXY_BSUM1;
  cam_awbxy_pcnt2_t CAM_AWBXY_PCNT2;
  cam_awbxy_rsum2_t CAM_AWBXY_RSUM2;
  cam_awbxy_gsum2_t CAM_AWBXY_GSUM2;                          // 0x0340
  cam_awbxy_bsum2_t CAM_AWBXY_BSUM2;
  cam_awbxy_pcnt3_t CAM_AWBXY_PCNT3;
  cam_awbxy_rsum3_t CAM_AWBXY_RSUM3;
  cam_awbxy_gsum3_t CAM_AWBXY_GSUM3;                          // 0x0350
  cam_awbxy_bsum3_t CAM_AWBXY_BSUM3;
  cam_awbxy_pcnt4_t CAM_AWBXY_PCNT4;
  cam_awbxy_rsum4_t CAM_AWBXY_RSUM4;
  cam_awbxy_gsum4_t CAM_AWBXY_GSUM4;                          // 0x0360
  cam_awbxy_bsum4_t CAM_AWBXY_BSUM4;
  cam_awbxy_pcnt5_t CAM_AWBXY_PCNT5;
  cam_awbxy_rsum5_t CAM_AWBXY_RSUM5;
  cam_awbxy_gsum5_t CAM_AWBXY_GSUM5;                          // 0x0370
  cam_awbxy_bsum5_t CAM_AWBXY_BSUM5;
  cam_awbxy_pcnt6_t CAM_AWBXY_PCNT6;
  cam_awbxy_rsum6_t CAM_AWBXY_RSUM6;
  cam_awbxy_gsum6_t CAM_AWBXY_GSUM6;                          // 0x0380
  cam_awbxy_bsum6_t CAM_AWBXY_BSUM6;
  cam_awbxy_pcnt7_t CAM_AWBXY_PCNT7;
  cam_awbxy_rsum7_t CAM_AWBXY_RSUM7;
  cam_awbxy_gsum7_t CAM_AWBXY_GSUM7;                          // 0x0390
  cam_awbxy_bsum7_t CAM_AWBXY_BSUM7;
  cam_awbxy_pcnt8_t CAM_AWBXY_PCNT8;
  cam_awbxy_rsum8_t CAM_AWBXY_RSUM8;
  cam_awbxy_gsum8_t CAM_AWBXY_GSUM8;                          // 0x03A0
  cam_awbxy_bsum8_t CAM_AWBXY_BSUM8;
  cam_awbxy_pcnt9_t CAM_AWBXY_PCNT9;
  cam_awbxy_rsum9_t CAM_AWBXY_RSUM9;
  cam_awbxy_gsum9_t CAM_AWBXY_GSUM9;                          // 0x03B0
  cam_awbxy_bsum9_t CAM_AWBXY_BSUM9;
  cam_awbxy_pcnta_t CAM_AWBXY_PCNTA;
  cam_awbxy_rsuma_t CAM_AWBXY_RSUMA;
  cam_awbxy_gsuma_t CAM_AWBXY_GSUMA;                          // 0x03C0
  cam_awbxy_bsuma_t CAM_AWBXY_BSUMA;
  cam_awbxy_pcntb_t CAM_AWBXY_PCNTB;
  cam_awbxy_rsumb_t CAM_AWBXY_RSUMB;
  cam_awbxy_gsumb_t CAM_AWBXY_GSUMB;                          // 0x03D0
  cam_awbxy_bsumb_t CAM_AWBXY_BSUMB;
  cam_afwin0_t CAM_AFWIN0;
  cam_afwin1_t CAM_AFWIN1;
  cam_afwin2_t CAM_AFWIN2;                                    // 0x03E0
  cam_afwin3_t CAM_AFWIN3;
  cam_afwin4_t CAM_AFWIN4;
  cam_afwin5_t CAM_AFWIN5;
  cam_afwin6_t CAM_AFWIN6;                                    // 0x03F0
  cam_afwin7_t CAM_AFWIN7;
  cam_afwin8_t CAM_AFWIN8;
  cam_af0_sum_t CAM_AF0_SUM[CAM_AF0_SUM_SIZE];
  cam_af1_sum_t CAM_AF1_SUM[CAM_AF1_SUM_SIZE];                // 0x0410
  cam_af2_sum_t CAM_AF2_SUM[CAM_AF2_SUM_SIZE];
  cam_af3_sum_t CAM_AF3_SUM[CAM_AF3_SUM_SIZE];
  cam_af4_sum_t CAM_AF4_SUM[CAM_AF4_SUM_SIZE];
  cam_af5_sum_t CAM_AF5_SUM[CAM_AF5_SUM_SIZE];                // 0x0460
  cam_af6_sum_t CAM_AF6_SUM[CAM_AF6_SUM_SIZE];
  cam_af7_sum_t CAM_AF7_SUM[CAM_AF7_SUM_SIZE];
  cam_af8_sum_t CAM_AF8_SUM[CAM_AF8_SUM_SIZE];
  cam_af0_gsum_t CAM_AF0_GSUM;                                // 0x04B0
  cam_af1_gsum_t CAM_AF1_GSUM;
  cam_af2_gsum_t CAM_AF2_GSUM;
  cam_af3_gsum_t CAM_AF3_GSUM;
  cam_af4_gsum_t CAM_AF4_GSUM;                                // 0x04C0
  cam_af5_gsum_t CAM_AF5_GSUM;
  cam_af6_gsum_t CAM_AF6_GSUM;
  MUINT32 RESERVE04CC[1];
  cam_af7_gsum_t CAM_AF7_GSUM;                                // 0x04D0
  cam_afth0_t CAM_AFTH0;
  cam_afth1_t CAM_AFTH1;
  MUINT32 RESERVE04DC[1];
  cam_fd_aewin0lr_t CAM_FD_AEWIN0LR;                          // 0x04E0
  cam_fd_aewin0ud_t CAM_FD_AEWIN0UD;
  cam_fd_aewin1lr_t CAM_FD_AEWIN1LR;
  cam_fd_aewin1ud_t CAM_FD_AEWIN1UD;
  cam_fd_aewin2lr_t CAM_FD_AEWIN2LR;                          // 0x04F0
  cam_fd_aewin2ud_t CAM_FD_AEWIN2UD;
  cam_fd_aewin3lr_t CAM_FD_AEWIN3LR;
  cam_fd_aewin3ud_t CAM_FD_AEWIN3UD;
  cam_nr2_con_t CAM_NR2_CON;                                  // 0x0500
  cam_nr2_cfg_c1_t CAM_NR2_CFG_C1;
  cam_nr2_cfg2_t CAM_NR2_CFG2;
  cam_nr2_cfg3_t CAM_NR2_CFG3;
  cam_nr2_cfg4_t CAM_NR2_CFG4;                                // 0x0510
  cam_nr2_cfg_c2_t CAM_NR2_CFG_C2;
  cam_nr2_cfg_l1_t CAM_NR2_CFG_L1;
  cam_nr2_cfg_l2_t CAM_NR2_CFG_L2;
  cam_nr2_cfg_n1_t CAM_NR2_CFG_N1;                            // 0x0520
  cam_nr2_cfg_n2_t CAM_NR2_CFG_N2;
  cam_nr2_cfg_n3_t CAM_NR2_CFG_N3;
  MUINT32 RESERVE052C[5];
  cam_hstcon_t CAM_HSTCON;                                    // 0x0540
  cam_hstcfg1_t CAM_HSTCFG1;
  cam_hstcfg2_t CAM_HSTCFG2;
  cam_hstcfg3_t CAM_HSTCFG3;
  cam_nr1_con_t CAM_NR1_CON;                                  // 0x0550
  cam_nr1_dp1_t CAM_NR1_DP1;
  cam_nr1_dp2_t CAM_NR1_DP2;
  cam_nr1_dp3_t CAM_NR1_DP3;
  cam_nr1_dp4_t CAM_NR1_DP4;                                  // 0x0560
  cam_nr1_ct_t CAM_NR1_CT;
  cam_nr1_nr1_t CAM_NR1_NR1;
  cam_nr1_nr2_t CAM_NR1_NR2;
  cam_nr1_nr3_t CAM_NR1_NR3;                                  // 0x0570
  cam_nr1_nr4_t CAM_NR1_NR4;
  cam_nr1_nr5_t CAM_NR1_NR5;
  cam_nr1_nr6_t CAM_NR1_NR6;
  cam_nr1_nr7_t CAM_NR1_NR7;                                  // 0x0580
  cam_nr1_nr8_t CAM_NR1_NR8;
  cam_nr1_nr9_t CAM_NR1_NR9;
  cam_nr1_nr10_t CAM_NR1_NR10;
  cam_nr1_nr11_t CAM_NR1_NR11;                                // 0x0590
  cam_nr1_nr12_t CAM_NR1_NR12;
  cam_nr1_bsz_t CAM_NR1_BSZ;
  MUINT32 RESERVE059C[1];
  cam_ee_ctrl_t CAM_EE_CTRL;                                  // 0x05A0
  cam_edctrl1_t CAM_EDCTRL1;
  cam_edctrl2_t CAM_EDCTRL2;
  cam_edctrl3_t CAM_EDCTRL3;
  cam_edctrl4_t CAM_EDCTRL4;                                  // 0x05B0
  cam_edctrl5_t CAM_EDCTRL5;
  cam_edgcore_t CAM_EDGCORE;
  cam_edggain1_t CAM_EDGGAIN1;
  cam_edggain2_t CAM_EDGGAIN2;                                // 0x05C0
  cam_edgthre_t CAM_EDGTHRE;
  cam_edgvcon_t CAM_EDGVCON;
  cam_egamma1_t CAM_EGAMMA1;
  cam_egamma2_t CAM_EGAMMA2;                                  // 0x05D0
  cam_egamma3_t CAM_EGAMMA3;
  MUINT32 RESERVE05D8[10];
  cam_yccgo_con_t CAM_YCCGO_CON;                              // 0x0600
  cam_yccgo_cfg1_t CAM_YCCGO_CFG1;
  cam_yccgo_cfg2_t CAM_YCCGO_CFG2;
  cam_yccgo_cfg3_t CAM_YCCGO_CFG3;
  cam_yccgo_cfg4_t CAM_YCCGO_CFG4;                            // 0x0610
  cam_yccgo_cfg5_t CAM_YCCGO_CFG5;
  cam_yccgo_cfg6_t CAM_YCCGO_CFG6;
  cam_yccgo_cfg7_t CAM_YCCGO_CFG7;
  cam_yccgo_cfg8_t CAM_YCCGO_CFG8;                            // 0x0620
  cam_yccgo_cfg9_t CAM_YCCGO_CFG9;
  MUINT32 RESERVE0628[2];
  cam_pca_con_t CAM_PCA_CON;                                  // 0x0630
  cam_pca_tba_t CAM_PCA_TBA;
  cam_pca_gmc_t CAM_PCA_GMC;
  MUINT32 RESERVE063C[1];
  cam_rawc_con_t CAM_RAWC_CON;                                // 0x0640
  cam_rawc_outaddr_t CAM_RAWC_OUTADDR;
  cam_rawc_outlen_t CAM_RAWC_OUTLEN;
  cam_rawc_baddr_t CAM_RAWC_BADDR;
  cam_rawc_blen_t CAM_RAWC_BLEN;                              // 0x0650
  cam_rawc_enctrl1_t CAM_RAWC_ENCTRL1;
  cam_rawc_enctrl2_t CAM_RAWC_ENCTRL2;
  cam_rawc_enctrl3_t CAM_RAWC_ENCTRL3;
  cam_rawc_enctrl4_t CAM_RAWC_ENCTRL4;                        // 0x0660
  cam_rawc_enctrl5_t CAM_RAWC_ENCTRL5;
  cam_rawc_enctrl6_t CAM_RAWC_ENCTRL6;
  cam_rawc_enctrl7_t CAM_RAWC_ENCTRL7;
  cam_rawc_dectrl1_t CAM_RAWC_DECTRL1;                        // 0x0670
  cam_rawc_dectrl2_t CAM_RAWC_DECTRL2;
  cam_rawc_dectrl3_t CAM_RAWC_DECTRL3;
  cam_rawc_dectrl4_t CAM_RAWC_DECTRL4;
  cam_stnr_status_t CAM_STNR_STATUS;                          // 0x0680
  cam_stnr_intsta_t CAM_STNR_INTSTA;
  cam_stnr_intena_t CAM_STNR_INTENA;
  cam_stnr_control_t CAM_STNR_CONTROL;
  cam_stnr_cfg_t CAM_STNR_CFG;                                // 0x0690
  cam_stnr_offset_t CAM_STNR_OFFSET;
  cam_stnr_edge1_t CAM_STNR_EDGE1;
  cam_stnr_edge2_t CAM_STNR_EDGE2;
  cam_stnr_snr1_t CAM_STNR_SNR1;                              // 0x06A0
  cam_stnr_snr2_t CAM_STNR_SNR2;
  cam_stnr_tnr_t CAM_STNR_TNR;
  cam_stnr_pm_t CAM_STNR_PM;
  cam_stnr_golden_t CAM_STNR_GOLDEN;                          // 0x06B0
  cam_stnr_base1_t CAM_STNR_BASE1;
  cam_stnr_base2_t CAM_STNR_BASE2;
  cam_stnr_base3_t CAM_STNR_BASE3;
  cam_stnr_input0_t CAM_STNR_INPUT0;                          // 0x06C0
  cam_stnr_input1_t CAM_STNR_INPUT1;
  cam_stnr_input2_t CAM_STNR_INPUT2;
  cam_stnr_input3_t CAM_STNR_INPUT3;
  cam_stnr_input4_t CAM_STNR_INPUT4;                          // 0x06D0
  cam_stnr_result1_t CAM_STNR_RESULT1;
  cam_stnr_result2_t CAM_STNR_RESULT2;
  cam_stnr_result3_t CAM_STNR_RESULT3;
  MUINT32 RESERVE06E0[72];
  cam_chksum_ctrl_t CAM_CHKSUM_CTRL;                          // 0x0800
  cam_chksum_sta0_t CAM_CHKSUM_STA0;
  cam_chksum_sta1_t CAM_CHKSUM_STA1;
  cam_chksum_sta2_t CAM_CHKSUM_STA2;
  cam_chksum_sta3_t CAM_CHKSUM_STA3;                          // 0x0810
  cam_chksum_sta4_t CAM_CHKSUM_STA4;
  cam_chksum_sta5_t CAM_CHKSUM_STA5;
  MUINT32 RESERVE081C[249];
  cam_tg2_ph_cnt_t CAM_TG2_PH_CNT;                            // 0x0C00
  cam_tg2_sen_ck_t CAM_TG2_SEN_CK;
  cam_tg2_sen_mode_t CAM_TG2_SEN_MODE;
  cam_tg2_vf_con_t CAM_TG2_VF_CON;
  cam_tg2_sen_grab_pxl_t CAM_TG2_SEN_GRAB_PXL;                // 0x0C10
  cam_tg2_sen_grab_lin_t CAM_TG2_SEN_GRAB_LIN;
  cam_tg2_path_cfg_t CAM_TG2_PATH_CFG;
  cam_tg2_memin_ctl_t CAM_TG2_MEMIN_CTL;
  cam_tg2_int1_t CAM_TG2_INT1;                                // 0x0C20
  cam_tg2_int2_t CAM_TG2_INT2;
  cam_tg2_sof_cnt_t CAM_TG2_SOF_CNT;
  cam_tg2_sot_cnt_t CAM_TG2_SOT_CNT;
  cam_tg2_eot_cnt_t CAM_TG2_EOT_CNT;                          // 0x0C30
  cam_tg2_err_ctl_t CAM_TG2_ERR_CTL;
  cam_tg2_dat_no_t CAM_TG2_DAT_NO;
  MUINT32 RESERVE0C3C[1];
  cam_tg2_frm_cnt_st_t CAM_TG2_FRM_CNT_ST;                    // 0x0C40
  cam_tg2_frmsize_st_t CAM_TG2_FRMSIZE_ST;
  cam_tg2_inter_st_t CAM_TG2_INTER_ST;
  cam_tg2_sen_frmsize_st_t CAM_TG2_SEN_FRMSIZE_ST;
  MUINT32 RESERVE0C50[4];
  cam_tg2_tm_ctl_t CAM_TG2_TM_CTL;                            // 0x0C60
  cam_tg2_tm_size_t CAM_TG2_TM_SIZE;
  cam_tg2_tm_clk_t CAM_TG2_TM_CLK;
  MUINT32 RESERVE0C6C[33];
  cam2_drop_vsub_t CAM2_DROP_VSUB;                            // 0x0CF0
  cam2_drop_hsub_t CAM2_DROP_HSUB;
  MUINT32 RESERVE0CF8[3];
  cam_ctl_mod_en_t CAM_CTL_MOD_EN;
  cam_ctl_fmt_sel_t CAM_CTL_FMT_SEL;
  cam_ctl_swap_t CAM_CTL_SWAP;
  cam_ctl_sel_t CAM_CTL_SEL;                                  // 0x0D10
  cam_ctl_crz_t CAM_CTL_CRZ;
  MUINT32 RESERVE0D18[2];
  cam_ctl_int_en_t CAM_CTL_INT_EN;                            // 0x0D20
  cam_ctl_int_status_t CAM_CTL_INT_STATUS;
  cam_ctl_sw_ctl_t CAM_CTL_SW_CTL;
  cam_lmu_ctrl_t CAM_LMU_CTRL;
  cam_ctl_dbg_set_t CAM_CTL_DBG_SET;                          // 0x0D30
  cam_ctl_dbg_port_t CAM_CTL_DBG_PORT;
  cam_ctl_version_t CAM_CTL_VERSION;
  cam_ctl_proj_t CAM_CTL_PROJ;
  MUINT32 RESERVE0D40[176];
  cam_aemem0_t CAM_AEMEM0;                                    // 0x1000
  cam_aemem1_t CAM_AEMEM1;
  cam_aemem2_t CAM_AEMEM2;
  cam_aemem3_t CAM_AEMEM3;
  cam_aemem4_t CAM_AEMEM4;                                    // 0x1010
  cam_aemem5_t CAM_AEMEM5;
  cam_aemem6_t CAM_AEMEM6;
  cam_aemem7_t CAM_AEMEM7;
  cam_aemem8_t CAM_AEMEM8;                                    // 0x1020
  cam_aemem9_t CAM_AEMEM9;
  cam_aemem10_t CAM_AEMEM10;
  cam_aemem11_t CAM_AEMEM11;
  cam_aemem12_t CAM_AEMEM12;                                  // 0x1030
  cam_aemem13_t CAM_AEMEM13;
  cam_aemem14_t CAM_AEMEM14;
  cam_aemem15_t CAM_AEMEM15;
  cam_aemem16_t CAM_AEMEM16;                                  // 0x1040
  cam_aemem17_t CAM_AEMEM17;
  cam_aemem18_t CAM_AEMEM18;
  cam_aemem19_t CAM_AEMEM19;
  cam_aeblk_t CAM_AEBLK;                                      // 0x1050
  cam_rawsize_t CAM_RAWSIZE;
  cam_awbdebug0_t CAM_AWBDEBUG0;
  cam_awbdebug1_t CAM_AWBDEBUG1;
  cam_flaremem_t CAM_FLAREMEM[CAM_FLAREMEM_SIZE];             // 0x1060
  cam_flaremem9_t CAM_FLAREMEM9;
  cam_afn_t CAM_AFN[CAM_AFN_SIZE];
  cam_afgsum_t CAM_AFGSUM[CAM_AFGSUM_SIZE];
  cam_awbnxy_cnt_t CAM_AWBNXY_CNT[CAM_AWBNXY_CNT_SIZE];       // 0x1160
  cam_awbsum_cnt_t CAM_AWBSUM_CNT;                            // 0x1220
  cam_awbsum_rsum_m_t CAM_AWBSUM_RSUM_M;
  cam_awbsum_gsum_m_t CAM_AWBSUM_GSUM_M;
  cam_awbsum_bsum_m_t CAM_AWBSUM_BSUM_M;
  cam_awbce_count_t CAM_AWBCE_COUNT;                          // 0x1230
  cam_awbce_rsum_m_t CAM_AWBCE_RSUM_M;
  cam_awbce_gsum_m_t CAM_AWBCE_GSUM_M;
  cam_awbce_bsum_m_t CAM_AWBCE_BSUM_M;
  cam_aehis_t CAM_AEHIS[CAM_AEHIS_SIZE];                      // 0x1240
  cam_fdaesum_t CAM_FDAESUM[CAM_FDAESUM_SIZE];                // 0x1340
  cam_fdaecnt_t CAM_FDAECNT[CAM_FDAECNT_SIZE];                // 0x1350
  cam_fdawbrsum_t CAM_FDAWBRSUM[CAM_FDAWBRSUM_SIZE];          // 0x1360
  cam_fdawbgsum_t CAM_FDAWBGSUM[CAM_FDAWBGSUM_SIZE];          // 0x1370
  cam_fdawbbsum_t CAM_FDAWBBSUM[CAM_FDAWBBSUM_SIZE];          // 0x1380
  cam_fdawbcnt_t CAM_FDAWBCNT[CAM_FDAWBCNT_SIZE];             // 0x1390
  MUINT32 RESERVE13A0[792];
  csi2_ctrl_t CSI2_CTRL;                                      // 0x2000
  csi2_delay_t CSI2_DELAY;
  csi2_inten_t CSI2_INTEN;
  csi2_intsta_t CSI2_INTSTA;
  csi2_eccdbg_t CSI2_ECCDBG;                                  // 0x2010
  csi2_crcdbg_t CSI2_CRCDBG;
  csi2_dbg_t CSI2_DBG;
  csi2_ver_t CSI2_VER;
  csi2_short_info_t CSI2_SHORT_INFO;                          // 0x2020
  csi2_lnfsm_t CSI2_LNFSM;
  csi2_lnmux_t CSI2_LNMUX;
  csi2_hsync_cnt_t CSI2_HSYNC_CNT;
  csi2_cal_t CSI2_CAL;                                        // 0x2030
} isp_reg_t;

#endif  // _ISP_REG_H_
