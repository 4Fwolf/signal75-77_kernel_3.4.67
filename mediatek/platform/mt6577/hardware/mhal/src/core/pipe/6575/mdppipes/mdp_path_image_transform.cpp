/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#define LOG_TAG "MDP"

#include "mdp_path.h"

#include "MediaTypes.h"

//open syscall
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//close syscall
#include <unistd.h>

//PQ : ISP driver
#include "isp_disp.h"
#include "display_isp_tuning_if_base.h"

/*/////////////////////////////////////////////////////////////////////////////
    Macro Re-Definition
  /////////////////////////////////////////////////////////////////////////////*/

#define AUTOCALISPREG_CNT 12
#define ISPREG_CNT 27

typedef struct {
    MUINT8  y_gain;
    MUINT8  sat_gain;
    MUINT8  hue_shift;
    MUINT8  reserved;
} ISP_NVRAM_PCA_BIN_T;

int MdpPathImageTransformYuv_1( struct MdpPathImageTransformParameter* p_parameter )
{
    int ret_val = MDP_ERROR_CODE_OK;

    //MDP Drv operation
    int             mdp_element_count = 4;
    MDPELEMENT_I*   mdp_element_list[MDP_ELEMENT_MAX_NUM];

    //MDP Elements
    RDMA0           me_rdma0;
    MOUT            me_mout;
    PRZ1            me_prz1;
    VDOROT1         me_vdorot1;

    MDP_SHOWFUNCTION();

    /*Resource List*/
    mdp_element_list[0] = (MDPELEMENT_I*)&me_rdma0;
    mdp_element_list[1] = (MDPELEMENT_I*)&me_mout;
    mdp_element_list[2] = (MDPELEMENT_I*)&me_prz1;
    mdp_element_list[3] = (MDPELEMENT_I*)&me_vdorot1;


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Setup mdp elements parameter
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    /***RDMA0***//*Crop*/
    //RDMA_I
    me_rdma0.uInBufferCnt = 1;
    me_rdma0.src_img_yuv_addr[0] = p_parameter->src_yuv_img_addr;
    me_rdma0.src_img_size = p_parameter->src_img_size;
    me_rdma0.src_img_roi  = p_parameter->src_img_roi;
    me_rdma0.src_color_format = p_parameter->src_color_format;


    //me_rdma0.bContinuousHWTrigger = 0;// HW trigger + continous + framesync
    me_rdma0.bHWTrigger = 0;
    me_rdma0.bContinuous = 0;
    me_rdma0.bCamIn = 0;

    me_rdma0.bEISEn = 0;
    me_rdma0.u4EISCON = 0;//EIS setting
    //RDMA0
    me_rdma0.to_cam = 0;
    me_rdma0.to_ovl = 0;
    me_rdma0.to_mout = 1;

    me_rdma0.trigger_src = 0;

    /***MOUT***/
    me_mout.src_sel = 0;      // 0-R_DMA0_MOUT, 1-OVL_DMA_MIMO

    me_mout.to_jpg_dma = 0;   //bit[0] : JPEG_DMA, bit[1] : PRZ0, bit[2] : VRZ,bit[3] : VDO_ROT1
    me_mout.to_prz0 = 0;
    me_mout.to_vrz0 = 0;
    me_mout.to_prz1 = 1;

    me_mout.bCamIn = 0;


    /***PRZ1***//*Rescale*/
    //RESZ_I
    me_prz1.src_img_size.w = me_rdma0.src_img_roi.w;
    me_prz1.src_img_size.h = me_rdma0.src_img_roi.h;
    me_prz1.src_img_roi.x = 0;
    me_prz1.src_img_roi.y = 0;
    me_prz1.src_img_roi.w = me_prz1.src_img_size.w;
    me_prz1.src_img_roi.h = me_prz1.src_img_size.h;

    me_prz1.dst_img_size.w = p_parameter->dst_img_roi.w; /*Currently dst roi is not used*/
    me_prz1.dst_img_size.h = p_parameter->dst_img_roi.h;

    me_prz1.uUpScaleCoeff = p_parameter->resz_up_scale_coeff;
    me_prz1.uDnScaleCoeff = p_parameter->resz_dn_scale_coeff;

    /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
    me_prz1.uEEHCoeff = p_parameter->resz_ee_h_str;
    me_prz1.uEEVCoeff = p_parameter->resz_ee_v_str;

    me_prz1.bContinuous = 0;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    me_prz1.bCamIn = 0;
    //me_prz1.bBypass = 0;

    me_prz1.bBypass = 0;
                /*if no crop...*/
    if( (me_prz1.src_img_roi.x == 0 ) &&
        (me_prz1.src_img_roi.y == 0 ) &&
        (me_prz1.src_img_roi.w == me_prz1.src_img_size.w ) &&
        (me_prz1.src_img_roi.h == me_prz1.src_img_size.h ) )
    {
                /*and if no scale...*/
        if( (me_prz1.src_img_roi.w == me_prz1.dst_img_size.w ) &&
            (me_prz1.src_img_roi.h == me_prz1.dst_img_size.h ) )
        {
                /*JUST bypass it! for 1280x720 video record*/
           me_prz1.bBypass= 1;
        }
    }

    //PRZ1
    /*N/A*/


    /***VDOROT1***/
    //ROTDMA_I
    me_vdorot1.dst_img_yuv_addr[0] = p_parameter->dst_yuv_img_addr;

    me_vdorot1.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_vdorot1.src_img_roi.y = 0;
    me_vdorot1.src_img_roi.w = me_prz1.dst_img_size.w;
    me_vdorot1.src_img_roi.h = me_prz1.dst_img_size.h;

    me_vdorot1.dst_img_size = p_parameter->dst_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_vdorot1.dst_color_format = p_parameter->dst_color_format;/*YUY2_Pack*/

    me_vdorot1.uOutBufferCnt = 1;
    me_vdorot1.bContinuous = 0;// 1:Continuous mode does not allow time sharing.
    me_vdorot1.bFlip = p_parameter->dst_rotate_angle >> 2;
    me_vdorot1.bRotate = (0x3 & p_parameter->dst_rotate_angle);//0:0, 1:90, 2:180, 3:270
    me_vdorot1.bDithering = 0;
    me_vdorot1.bCamIn = 0;// 1: real time path, ex : camera input
    me_vdorot1.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

    //VDOROT1
    me_vdorot1.src_sel= 0;// 0-PRZ1, 1-VRZ0



    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Trigger HW
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    ret_val = _MdpPathTriggerHw( mdp_element_list , mdp_element_count  );

    if( ret_val == MDP_ERROR_CODE_LOCK_RESOURCE_FAIL )
        return ret_val;

    return ret_val;


}

  /*/////////////////////////////////////////////////////////////////////////////
      RDMA0->MOUT->PRZ0->VDO0
    /////////////////////////////////////////////////////////////////////////////*/
int MdpPathImageTransformYuv_2( struct MdpPathImageTransformParameter* p_parameter )
{
    int ret_val = MDP_ERROR_CODE_OK;

    //MDP Drv operation
    int             mdp_element_count = 4;
    MDPELEMENT_I*   mdp_element_list[MDP_ELEMENT_MAX_NUM];

    //MDP Elements
    RDMA0            me_rdma0;
    MOUT              me_mout;
    PRZ0               me_prz0;
    VDOROT0        me_vdorot0;

    MDP_SHOWFUNCTION();

    /*Resource List*/
    mdp_element_list[0] = (MDPELEMENT_I*)&me_rdma0;
    mdp_element_list[1] = (MDPELEMENT_I*)&me_mout;
    mdp_element_list[2] = (MDPELEMENT_I*)&me_prz0;
    mdp_element_list[3] = (MDPELEMENT_I*)&me_vdorot0;


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Setup mdp elements parameter
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    /***RDMA0***//*Crop*/
    //RDMA_I
    me_rdma0.uInBufferCnt = 1;
    me_rdma0.src_img_yuv_addr[0] = p_parameter->src_yuv_img_addr;
    me_rdma0.src_img_size = p_parameter->src_img_size;
    me_rdma0.src_img_roi  = p_parameter->src_img_roi;
    me_rdma0.src_color_format = p_parameter->src_color_format;


    //me_rdma0.bContinuousHWTrigger = 0;// HW trigger + continous + framesync
    me_rdma0.bHWTrigger = 0;
    me_rdma0.bContinuous = 0;
    me_rdma0.bCamIn = 0;

    me_rdma0.bEISEn = 0;
    me_rdma0.u4EISCON = 0;//EIS setting
    //RDMA0
    me_rdma0.to_cam = 0;
    me_rdma0.to_ovl = 0;
    me_rdma0.to_mout = 1;

    me_rdma0.trigger_src = 0;

    /***MOUT***/
    me_mout.src_sel = 0;      // 0-R_DMA0_MOUT, 1-OVL_DMA_MIMO

    me_mout.to_jpg_dma = 0;   //bit[0] : JPEG_DMA, bit[1] : PRZ0, bit[2] : VRZ,bit[3] : VDO_ROT1
    me_mout.to_prz0 = 1;
    me_mout.to_vrz0 = 0;
    me_mout.to_prz1 = 0;

    me_mout.bCamIn = 0;


    /***PRZ0***//*Rescale*/
    //RESZ_I
    me_prz0.src_img_size.w = me_rdma0.src_img_roi.w;
    me_prz0.src_img_size.h = me_rdma0.src_img_roi.h;
    me_prz0.src_img_roi.x = 0;
    me_prz0.src_img_roi.y = 0;
    me_prz0.src_img_roi.w = me_prz0.src_img_size.w;
    me_prz0.src_img_roi.h = me_prz0.src_img_size.h;

    me_prz0.dst_img_size.w = p_parameter->dst_img_roi.w; /*Currently dst roi is not used*/
    me_prz0.dst_img_size.h = p_parameter->dst_img_roi.h;


    me_prz0.uUpScaleCoeff = p_parameter->resz_up_scale_coeff;
    me_prz0.uDnScaleCoeff = p_parameter->resz_dn_scale_coeff;

    /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
    me_prz0.uEEHCoeff = p_parameter->resz_ee_h_str;
    me_prz0.uEEVCoeff = p_parameter->resz_ee_v_str;


    me_prz0.bContinuous = 0;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    me_prz0.bCamIn = 0;
    //me_prz0.bBypass = 0;

    me_prz0.bBypass = 0;
                /*if no crop...*/
    if( (me_prz0.src_img_roi.x == 0 ) &&
        (me_prz0.src_img_roi.y == 0 ) &&
        (me_prz0.src_img_roi.w == me_prz0.src_img_size.w ) &&
        (me_prz0.src_img_roi.h == me_prz0.src_img_size.h ) )
    {
                /*and if no scale...*/
        if( (me_prz0.src_img_roi.w == me_prz0.dst_img_size.w ) &&
            (me_prz0.src_img_roi.h == me_prz0.dst_img_size.h ) )
        {
                /*JUST bypass it! for 1280x720 video record*/
           me_prz0.bBypass= 1;
        }
    }

    //PRZ0
    me_prz0.src_sel = 0;//0-MOUT,1-IPP_MOUT,2-CAM,3-BRZ_MOUT
    /*output sel*/
    me_prz0.to_vdo_rot0 = 1;
    me_prz0.to_rgb_rot0 = 0;
    me_prz0.to_vrz0 = 0;


    /***VDOROT0***/
    //ROTDMA_I
    me_vdorot0.dst_img_yuv_addr[0] = p_parameter->dst_yuv_img_addr;

    me_vdorot0.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_vdorot0.src_img_roi.y = 0;
    me_vdorot0.src_img_roi.w = me_prz0.dst_img_size.w;
    me_vdorot0.src_img_roi.h = me_prz0.dst_img_size.h;

    me_vdorot0.dst_img_size = p_parameter->dst_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_vdorot0.dst_color_format = p_parameter->dst_color_format;/*YUY2_Pack*/

    me_vdorot0.uOutBufferCnt = 1;
    me_vdorot0.bContinuous = 0;// 1:Continuous mode does not allow time sharing.
    me_vdorot0.bFlip = p_parameter->dst_rotate_angle >> 2;
    me_vdorot0.bRotate = (0x3 & p_parameter->dst_rotate_angle);//0:0, 1:90, 2:180, 3:270
    me_vdorot0.bDithering = 0;
    me_vdorot0.bCamIn = 0;// 1: real time path, ex : camera input
    me_vdorot0.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

    //VDOROT0
    me_vdorot0.src_sel= 0;// 0-PRZ0_MOUT, 1-IPP_MOUT


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Trigger HW
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    ret_val = _MdpPathTriggerHw( mdp_element_list , mdp_element_count  );

    if( ret_val == MDP_ERROR_CODE_LOCK_RESOURCE_FAIL )
        return ret_val;

    return ret_val;


}



  /*/////////////////////////////////////////////////////////////////////////////
        RDMA0->CRZ->IPP->VDO0
    /////////////////////////////////////////////////////////////////////////////*/
int MdpPathImageTransformYuv_3( struct MdpPathImageTransformParameter* p_parameter )
{
    int ret_val = MDP_ERROR_CODE_OK;

    //MDP Drv operation
    int             mdp_element_count = 4;
    MDPELEMENT_I*   mdp_element_list[MDP_ELEMENT_MAX_NUM];

    //MDP Elements
    RDMA0       me_rdma0;
    CRZ         me_crz;
    IPP         me_ipp;
    VDOROT0     me_vdorot0;

    MDP_SHOWFUNCTION();

    /*Resource List*/
    mdp_element_list[0] = (MDPELEMENT_I*)&me_rdma0;
    mdp_element_list[1] = (MDPELEMENT_I*)&me_crz;
    mdp_element_list[2] = (MDPELEMENT_I*)&me_ipp;
    mdp_element_list[3] = (MDPELEMENT_I*)&me_vdorot0;


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      Setup mdp elements parameter
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    /***RDMA0***//*Crop*/
    //RDMA_I
    me_rdma0.uInBufferCnt = 1;
    me_rdma0.src_img_yuv_addr[0] = p_parameter->src_yuv_img_addr;
    me_rdma0.src_img_size = p_parameter->src_img_size;
    me_rdma0.src_img_roi  = p_parameter->src_img_roi;
    me_rdma0.src_color_format = p_parameter->src_color_format;


    //me_rdma0.bContinuousHWTrigger = 0;// HW trigger + continous + framesync
    me_rdma0.bHWTrigger = 0;
    me_rdma0.bContinuous = 0;
    me_rdma0.bCamIn = 0;

    me_rdma0.bEISEn = 0;
    me_rdma0.u4EISCON = 0;//EIS setting
    //RDMA0
    me_rdma0.to_cam = 1;
    me_rdma0.to_ovl = 0;
    me_rdma0.to_mout = 0;

    me_rdma0.trigger_src = 0;

    /***CRZ***/
    //RESZ_I
    me_crz.src_img_size.w = me_rdma0.src_img_roi.w;
    me_crz.src_img_size.h = me_rdma0.src_img_roi.h;
    me_crz.src_img_roi.x = 0;
    me_crz.src_img_roi.y = 0;
    me_crz.src_img_roi.w = me_crz.src_img_size.w;
    me_crz.src_img_roi.h = me_crz.src_img_size.h;

    me_crz.dst_img_size.w = p_parameter->dst_img_roi.w;
    me_crz.dst_img_size.h = p_parameter->dst_img_roi.h;


    me_crz.uUpScaleCoeff = p_parameter->resz_up_scale_coeff;
    me_crz.uDnScaleCoeff = p_parameter->resz_dn_scale_coeff;


    // CRZ
    me_crz.src_sel = 0; // 0:RDMA0 , 1:BRZ, 2:Camera

    /***IPP***/
    // IPP
    /*output sel*/
    me_ipp.to_jpg_dma = 0;
    me_ipp.to_vdo_rot0 = 1;
    me_ipp.to_prz0 = 0;
    me_ipp.to_vrz0 = 0;
    me_ipp.to_rgb_rot0 = 0;

    /*input sel*/
    me_ipp.src_sel = 1;//0 : OVL, 1:CRZ
    me_ipp.bCamIn = 0;

    if (p_parameter->src_color_format < 8)
    {
        me_ipp.bEnR2Y = 1;
        me_ipp.bBypass = 0;
    }
    else
    {
        me_ipp.bEnR2Y = 0;
        me_ipp.bBypass = 1;
    }


    /***VDOROT0***/
    //ROTDMA_I
    me_vdorot0.dst_img_yuv_addr[0] = p_parameter->dst_yuv_img_addr;

    me_vdorot0.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_vdorot0.src_img_roi.y = 0;
    me_vdorot0.src_img_roi.w = me_crz.dst_img_size.w;
    me_vdorot0.src_img_roi.h = me_crz.dst_img_size.h;

    me_vdorot0.dst_img_size = p_parameter->dst_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_vdorot0.dst_color_format = p_parameter->dst_color_format;/*YUY2_Pack*/

    me_vdorot0.uOutBufferCnt = 1;
    me_vdorot0.bContinuous = 0;// 1:Continuous mode does not allow time sharing.
    me_vdorot0.bFlip = p_parameter->dst_rotate_angle >> 2;
    me_vdorot0.bRotate = (0x3 & p_parameter->dst_rotate_angle);//0:0, 1:90, 2:180, 3:270
    me_vdorot0.bDithering = 0;
    me_vdorot0.bCamIn = 0;// 1: real time path, ex : camera input
    me_vdorot0.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

    //VDOROT0
    me_vdorot0.src_sel= 1;// 0-PRZ0_MOUT, 1-IPP_MOUT


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      Trigger HW
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    ret_val = _MdpPathTriggerHw( mdp_element_list , mdp_element_count  );

    if( ret_val == MDP_ERROR_CODE_LOCK_RESOURCE_FAIL )
        return ret_val;

    return ret_val;


}


  /*/////////////////////////////////////////////////////////////////////////////
      RDMA1->VRZ1->RGB2
    /////////////////////////////////////////////////////////////////////////////*/

int MdpPathImageTransformRgb_1(struct MdpPathImageTransformParameter * p_parameter)
{
    int ret_val = 0;

    //MDP Drv operation
    int             mdp_element_count = 3;
    MDPELEMENT_I*   mdp_element_list[MDP_ELEMENT_MAX_NUM];

    //MDP Elements
    RDMA1           me_rdma1;
    VRZ1            me_vrz1;
    RGBROT2         me_rgbrot2;

    MDP_SHOWFUNCTION();

    /*Resource List*/
    mdp_element_list[0] = (MDPELEMENT_I*)&me_rdma1;
    mdp_element_list[1] = (MDPELEMENT_I*)&me_vrz1;
    mdp_element_list[2] = (MDPELEMENT_I*)&me_rgbrot2;


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Setup mdp elements parameter
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    /***RDMA1***//*Crop*/
    //RDMA_I
    me_rdma1.uInBufferCnt = 1;
    me_rdma1.src_img_yuv_addr[0] = p_parameter->src_yuv_img_addr;
    me_rdma1.src_img_size = p_parameter->src_img_size;
    me_rdma1.src_img_roi  = p_parameter->src_img_roi;
    me_rdma1.src_color_format = p_parameter->src_color_format;


    //me_rdma0.bContinuousHWTrigger = 0;// HW trigger + continous + framesync
    me_rdma1.bHWTrigger = 0;
    me_rdma1.bContinuous = 0;
    me_rdma1.bCamIn = 0;

    me_rdma1.bEISEn = 0;
    me_rdma1.u4EISCON = 0;//EIS setting
    //RDMA1
    /*N/A*/



    /***VRZ1***//*Rescale*/
    //VRZ_I
    me_vrz1.src_img_size.w = me_rdma1.src_img_roi.w;
    me_vrz1.src_img_size.h = me_rdma1.src_img_roi.h;

    me_vrz1.dst_img_size.w = p_parameter->dst_img_roi.w; /*Currently dst roi is not used*/
    me_vrz1.dst_img_size.h = p_parameter->dst_img_roi.h;

    me_vrz1.bContinuous = 0;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    me_vrz1.bCamIn = 0;

    me_vrz1.bBypass = 0;
        /*and if no scale...*/
    if( (me_vrz1.src_img_size.w == me_vrz1.dst_img_size.w ) &&
        (me_vrz1.src_img_size.h == me_vrz1.dst_img_size.h ) )
    {
        /*JUST bypass it! for 1280x720 video record*/
        me_vrz1.bBypass= 1;
    }


    //VRZ1
    me_vrz1.src_sel = 0;// 0-R_DMA1, 1-BRZ_MOUT


    /***RGBROT2***/
    //ROTDMA_I
    me_rgbrot2.dst_img_yuv_addr[0] = p_parameter->dst_yuv_img_addr;

    me_rgbrot2.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_rgbrot2.src_img_roi.y = 0;
    me_rgbrot2.src_img_roi.w = me_vrz1.dst_img_size.w;
    me_rgbrot2.src_img_roi.h = me_vrz1.dst_img_size.h;

    me_rgbrot2.dst_img_size = p_parameter->dst_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_rgbrot2.dst_color_format = p_parameter->dst_color_format;/*YUY2_Pack*/

    me_rgbrot2.uOutBufferCnt = 1;
    me_rgbrot2.bContinuous = 0;// 1:Continuous mode does not allow time sharing.
    me_rgbrot2.bFlip = p_parameter->dst_rotate_angle >> 2;
    me_rgbrot2.bRotate = (0x3 & p_parameter->dst_rotate_angle);//0:0, 1:90, 2:180, 3:270
    me_rgbrot2.bDithering = p_parameter->dst_dither_en;
    me_rgbrot2.bCamIn = 0;// 1: real time path, ex : camera input
    me_rgbrot2.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

    //RGBROT2
    /*N/A*/




    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Trigger HW
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    ret_val = _MdpPathTriggerHw( mdp_element_list , mdp_element_count  );

    return ret_val;


}


/*/////////////////////////////////////////////////////////////////////////////
    RDMA0->MOUT->PRZ0->RGB0
  /////////////////////////////////////////////////////////////////////////////*/
int MdpPathImageTransformRgb_3( struct MdpPathImageTransformParameter* p_parameter )
{
    int ret_val = MDP_ERROR_CODE_OK;

    //MDP Drv operation
    int             mdp_element_count = 4;
    MDPELEMENT_I*   mdp_element_list[MDP_ELEMENT_MAX_NUM];

    //MDP Elements
    RDMA0           me_rdma0;
    MOUT            me_mout;
    PRZ0            me_prz0;
    RGBROT0         me_rgbrot0;

    MDP_SHOWFUNCTION();

    /*Resource List*/
    mdp_element_list[0] = (MDPELEMENT_I*)&me_rdma0;
    mdp_element_list[1] = (MDPELEMENT_I*)&me_mout;
    mdp_element_list[2] = (MDPELEMENT_I*)&me_prz0;
    mdp_element_list[3] = (MDPELEMENT_I*)&me_rgbrot0;


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Setup mdp elements parameter
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    /***RDMA0***//*Crop*/
    //RDMA_I
    me_rdma0.uInBufferCnt = 1;
    me_rdma0.src_img_yuv_addr[0] = p_parameter->src_yuv_img_addr;
    me_rdma0.src_img_size = p_parameter->src_img_size;
    me_rdma0.src_img_roi  = p_parameter->src_img_roi;
    me_rdma0.src_color_format = p_parameter->src_color_format;


    //me_rdma0.bContinuousHWTrigger = 0;// HW trigger + continous + framesync
    me_rdma0.bHWTrigger = 0;
    me_rdma0.bContinuous = 0;
    me_rdma0.bCamIn = 0;

    me_rdma0.bEISEn = 0;
    me_rdma0.u4EISCON = 0;//EIS setting
    //RDMA0
    me_rdma0.to_cam = 0;
    me_rdma0.to_ovl = 0;
    me_rdma0.to_mout = 1;

    me_rdma0.trigger_src = 0;

    /***MOUT***/
    me_mout.src_sel = 0;      // 0-R_DMA0_MOUT, 1-OVL_DMA_MIMO

    me_mout.to_jpg_dma = 0;   //bit[0] : JPEG_DMA, bit[1] : PRZ0, bit[2] : VRZ,bit[3] : VDO_ROT1
    me_mout.to_prz0 = 1;
    me_mout.to_vrz0 = 0;
    me_mout.to_prz1 = 0;

    me_mout.bCamIn = 0;


    /***PRZ0***//*Rescale*/
    //RESZ_I
    me_prz0.src_img_size.w = me_rdma0.src_img_roi.w;
    me_prz0.src_img_size.h = me_rdma0.src_img_roi.h;
    me_prz0.src_img_roi.x = 0;
    me_prz0.src_img_roi.y = 0;
    me_prz0.src_img_roi.w = me_prz0.src_img_size.w;
    me_prz0.src_img_roi.h = me_prz0.src_img_size.h;

    me_prz0.dst_img_size.w = p_parameter->dst_img_roi.w; /*Currently dst roi is not used*/
    me_prz0.dst_img_size.h = p_parameter->dst_img_roi.h;

    me_prz0.uUpScaleCoeff = p_parameter->resz_up_scale_coeff;
    me_prz0.uDnScaleCoeff = p_parameter->resz_dn_scale_coeff;

    /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
    me_prz0.uEEHCoeff = p_parameter->resz_ee_h_str;
    me_prz0.uEEVCoeff = p_parameter->resz_ee_v_str;

    me_prz0.bContinuous = 0;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    me_prz0.bCamIn = 0;
    //me_prz0.bBypass = 0;

    me_prz0.bBypass = 0;
                /*if no crop...*/
    if( (me_prz0.src_img_roi.x == 0 ) &&
        (me_prz0.src_img_roi.y == 0 ) &&
        (me_prz0.src_img_roi.w == me_prz0.src_img_size.w ) &&
        (me_prz0.src_img_roi.h == me_prz0.src_img_size.h ) )
    {
                /*and if no scale...*/
        if( (me_prz0.src_img_roi.w == me_prz0.dst_img_size.w ) &&
            (me_prz0.src_img_roi.h == me_prz0.dst_img_size.h ) )
        {
                /*JUST bypass it! for 1280x720 video record*/
           me_prz0.bBypass= 1;
        }
    }

    //PRZ0
    me_prz0.src_sel = 0;;//0-MOUT,1-IPP_MOUT,2-CAM,3-BRZ_MOUT

    /*output sel*/
    me_prz0.to_vdo_rot0 = 0;
    me_prz0.to_rgb_rot0 = 1;
    me_prz0.to_vrz0= 0;

    me_prz0.bCamIn= 0;



    /***RGBROT0***/
    //ROTDMA_I
    me_rgbrot0.dst_img_yuv_addr[0] = p_parameter->dst_yuv_img_addr;

    me_rgbrot0.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_rgbrot0.src_img_roi.y = 0;
    me_rgbrot0.src_img_roi.w = me_prz0.dst_img_size.w;
    me_rgbrot0.src_img_roi.h = me_prz0.dst_img_size.h;

    me_rgbrot0.dst_img_size = p_parameter->dst_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_rgbrot0.dst_color_format = p_parameter->dst_color_format;/*YUY2_Pack*/

    me_rgbrot0.uOutBufferCnt = 1;
    me_rgbrot0.bContinuous = 0;// 1:Continuous mode does not allow time sharing.
    me_rgbrot0.bFlip = p_parameter->dst_rotate_angle >> 2;
    me_rgbrot0.bRotate = (0x3 & p_parameter->dst_rotate_angle);//0:0, 1:90, 2:180, 3:270
    me_rgbrot0.bDithering = p_parameter->dst_dither_en;
    me_rgbrot0.bCamIn = 0;// 1: real time path, ex : camera input
    me_rgbrot0.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

    //RGBROT0
    me_rgbrot0.src_sel= 0;    //0-PRZ0_MOUT ,1-IPP_MOUT




    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Trigger HW
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    ret_val = _MdpPathTriggerHw( mdp_element_list , mdp_element_count  );

    if( ret_val == MDP_ERROR_CODE_LOCK_RESOURCE_FAIL )
            return ret_val;


    return ret_val;


}


    /*/////////////////////////////////////////////////////////////////////////////
        RDMA0->CRZ->IPP->VRZ0->RGB1
    /////////////////////////////////////////////////////////////////////////////*/
int MdpPathImageTransformRgb_4( struct MdpPathImageTransformParameter* p_parameter )
{
    int ret_val = MDP_ERROR_CODE_OK;

    //MDP Drv operation
    int             mdp_element_count = 5;
    MDPELEMENT_I*   mdp_element_list[MDP_ELEMENT_MAX_NUM];

    //MDP Elements
    RDMA0       me_rdma0;
    CRZ         me_crz;
    IPP         me_ipp;
    VRZ0        me_vrz0;
    RGBROT1     me_rgbrot1;

    MDP_SHOWFUNCTION();

    /*Resource List*/
    mdp_element_list[0] = (MDPELEMENT_I*)&me_rdma0;
    mdp_element_list[1] = (MDPELEMENT_I*)&me_crz;
    mdp_element_list[2] = (MDPELEMENT_I*)&me_ipp;
    mdp_element_list[3] = (MDPELEMENT_I*)&me_vrz0;
    mdp_element_list[4] = (MDPELEMENT_I*)&me_rgbrot1;


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      Setup mdp elements parameter
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    /***RDMA0***//*Crop*/
    //RDMA_I
    me_rdma0.uInBufferCnt = 1;
    me_rdma0.src_img_yuv_addr[0] = p_parameter->src_yuv_img_addr;
    me_rdma0.src_img_size = p_parameter->src_img_size;
    me_rdma0.src_img_roi  = p_parameter->src_img_roi;
    me_rdma0.src_color_format = p_parameter->src_color_format;


    //me_rdma0.bContinuousHWTrigger = 0;// HW trigger + continous + framesync
    me_rdma0.bHWTrigger = 0;
    me_rdma0.bContinuous = 0;
    me_rdma0.bCamIn = 0;

    me_rdma0.bEISEn = 0;
    me_rdma0.u4EISCON = 0;//EIS setting
    //RDMA0
    me_rdma0.to_cam = 1;
    me_rdma0.to_ovl = 0;
    me_rdma0.to_mout = 0;

    me_rdma0.trigger_src = 0;

    /***CRZ***/
    //RESZ_I
    me_crz.src_img_size.w = me_rdma0.src_img_roi.w;
    me_crz.src_img_size.h = me_rdma0.src_img_roi.h;
    me_crz.src_img_roi.x = 0;
    me_crz.src_img_roi.y = 0;
    me_crz.src_img_roi.w = me_crz.src_img_size.w;
    me_crz.src_img_roi.h = me_crz.src_img_size.h;

    me_crz.dst_img_size.w = p_parameter->dst_img_roi.w;
    me_crz.dst_img_size.h = p_parameter->dst_img_roi.h;

    me_crz.uUpScaleCoeff = p_parameter->resz_up_scale_coeff;
    me_crz.uDnScaleCoeff = p_parameter->resz_dn_scale_coeff;


    // CRZ
    me_crz.src_sel = 0; // 0:RDMA0 , 1:BRZ, 2:Camera

    /***IPP***/
    // IPP
    /*output sel*/
    me_ipp.to_jpg_dma = 0;
    me_ipp.to_vdo_rot0 = 0;
    me_ipp.to_prz0 = 0;
    me_ipp.to_vrz0 = 1;
    me_ipp.to_rgb_rot0 = 0;

    /*input sel*/
    me_ipp.src_sel = 1;//0 : OVL, 1:CRZ
    me_ipp.bCamIn = 0;

    if (p_parameter->src_color_format > 7)
    {
        me_ipp.bEnY2R = 1;
        me_ipp.bBypass = 0;
    }
    else
    {
        me_ipp.bEnY2R = 0;
        me_ipp.bBypass = 1;
    }

    /***VRZ0***/
    //RESZ_I
    me_vrz0.bContinuous = 0;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    me_vrz0.bCamIn = 0;
    me_vrz0.bBypass = 1;

    //VRZ0
    me_vrz0.src_sel = 2;// 0-MOUT, 1-BRZ_MOUT, 2-IPP_MOUT, 3-OVL_DMA_MIMO, 4-PRZ0_MOUT


    /***RGBROT1***/
    //ROTDMA_I
    me_rgbrot1.dst_img_yuv_addr[0] = p_parameter->dst_yuv_img_addr;

    me_rgbrot1.src_img_roi.x = 0;
    me_rgbrot1.src_img_roi.y = 0;
    me_rgbrot1.src_img_roi.w = me_crz.dst_img_size.w;
    me_rgbrot1.src_img_roi.h = me_crz.dst_img_size.h;

    me_rgbrot1.dst_img_size = p_parameter->dst_img_size;
    me_rgbrot1.dst_color_format = p_parameter->dst_color_format;
    me_rgbrot1.uOutBufferCnt = 1;
    me_rgbrot1.bContinuous = 0;// 1:Continuous mode does not allow time sharing.
    me_rgbrot1.bFlip = p_parameter->dst_rotate_angle >> 2;;
    me_rgbrot1.bRotate = (0x3 & p_parameter->dst_rotate_angle);//0:0, 1:90, 2:180, 3:270
    me_rgbrot1.bDithering = p_parameter->dst_dither_en;
    me_rgbrot1.bCamIn = 0;// 1: real time path, ex : camera input
    me_rgbrot1.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

    //RGBROT1
    /*Nothing*/


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      Trigger HW
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    ret_val = _MdpPathTriggerHw( mdp_element_list , mdp_element_count  );

    if( ret_val == MDP_ERROR_CODE_LOCK_RESOURCE_FAIL )
        return ret_val;

    return ret_val;


}

/*/////////////////////////////////////////////////////////////////////////////
    RDMA0->CAM->CRZ->IPP->PRZ0->RGB0
  /////////////////////////////////////////////////////////////////////////////*/

static int PqCust_ConfigPre( void* p_custdata )
{
    return 0;
}
static int PqCust_ConfigPost( void* p_custdata )
{
    return 0;
}
static int PqCust_EnablePre( void* p_custdata )
{
    return 0;
}
static int PqCust_EnablePost( void* p_custdata )
{
    return 0;
}
static int PqCust_WaitPre( void* p_custdata )
{
    return 0;
}
static int PqCust_WaitPost( void* p_custdata )
{
    return 0;
}
static int PqCust_DisablePre( void* p_custdata )
{
    return 0;
}
static int PqCust_DisablePost( void* p_custdata )
{
    return 0;
}

static int PqCust_CamConfig( void* p_custdata )
{
    return 0;
}
static int PqCust_CamEnable( void* p_custdata )
{
    IspDisp* pIspDisp = (IspDisp*) p_custdata;

    if( pIspDisp == NULL )
    {
        MDP_ERROR("pIspDisp is NULL\n");
        return MDP_ERROR_CODE_FAIL;
    }

    if(!pIspDisp->Run(MTRUE))
    {
        MDP_ERROR("isp_disp_run start fail");
        return MDP_ERROR_CODE_FAIL;
    }

    return 0;
}
static int PqCust_CamWait( void* p_custdata )
{
    return 0;
}
static int PqCust_CamDisable( void* p_custdata )
{
    IspDisp* pIspDisp = (IspDisp*) p_custdata;

    if( pIspDisp == NULL )
    {
        MDP_ERROR("pIspDisp is NULL\n");
        return MDP_ERROR_CODE_FAIL;
    }

    //Close isp
    pIspDisp->Run(MFALSE);

    return 0;
}

int MdpPathImageProcess( struct MdpPathImageTransformParameter* p_parameter , unsigned long u4ProcFlag)
{
    int ret_val = MDP_ERROR_CODE_OK;

    //MDP Drv operation
    int             mdp_element_count = 5;
    MDPELEMENT_I*   mdp_element_list[5];

    //MDP Elements
    RDMA0           me_rdma0;
    CRZ             me_crz;
    IPP             me_ipp;
    PRZ0            me_prz0;
    RGBROT0         me_rgbrot0;
    VDOROT0         me_vdorot0;

    MdpCustTriggerHw_t  pq_cust_func_tbl;

    ISP_DISP_CONFIG_STRUCT    IspDispConfig;

    using namespace NSDisplayIspTuning;
    PRZ_T PRZ_PARAM;
    IspDisp* pIspDisp;
    DisplayIspTuningIFBase* pISPTuning = NULL;

    MDP_SHOWFUNCTION();

    /*-----------------------------------------------------------------------------
        Initial Cust HW Trigger Function
      -----------------------------------------------------------------------------*/
    pq_cust_func_tbl.cb_ConfigPre   =   PqCust_ConfigPre;
    pq_cust_func_tbl.cb_ConfigPost  =   PqCust_ConfigPost;

    pq_cust_func_tbl.cb_EnablePre   =   PqCust_EnablePre;
    pq_cust_func_tbl.cb_EnablePost  =   PqCust_EnablePost;

    pq_cust_func_tbl.cb_WaitPre     =   PqCust_WaitPre;
    pq_cust_func_tbl.cb_WaitPost    =   PqCust_WaitPost;

    pq_cust_func_tbl.cb_DisablePre  =   PqCust_DisablePre;
    pq_cust_func_tbl.cb_DisablePost =   PqCust_DisablePost;

    pq_cust_func_tbl.mdp_module_after_cam = MID_CRZ;

    pq_cust_func_tbl.cb_CamConfig   = PqCust_CamConfig;
    pq_cust_func_tbl.cb_CamEnable   = PqCust_CamEnable;
    pq_cust_func_tbl.cb_CamWait     = PqCust_CamWait;
    pq_cust_func_tbl.cb_CamDisable  = PqCust_CamDisable;

    //memset(&me_rdma0, 0, sizeof(RDMA0));
    //memset(&me_crz, 0, sizeof(CRZ));
    //memset(&me_ipp, 0, sizeof(IPP));
    //memset(&me_prz0, 0, sizeof(PRZ0));
    //memset(&me_rgbrot0, 0, sizeof(RGBROT0));

    /*Resource List*/
    mdp_element_list[0] = (MDPELEMENT_I*)&me_rdma0;
    mdp_element_list[1] = (MDPELEMENT_I*)&me_crz;
    mdp_element_list[2] = (MDPELEMENT_I*)&me_ipp;
    if(1280 < p_parameter->dst_img_roi.w)
    {
        mdp_element_list[3] = ((YV16_Planar > p_parameter->dst_color_format) ? (MDPELEMENT_I*)&me_rgbrot0 : (MDPELEMENT_I*)&me_vdorot0);
        mdp_element_count = 4;
    }
    else
    {
        mdp_element_list[3] = (MDPELEMENT_I*)&me_prz0;
        mdp_element_list[4] = ((YV16_Planar > p_parameter->dst_color_format) ? (MDPELEMENT_I*)&me_rgbrot0 : (MDPELEMENT_I*)&me_vdorot0);
        mdp_element_count = 5;
    }

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Setup mdp elements parameter
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /***RDMA0***//*Crop*/
    // RDMA_I
    me_rdma0.uInBufferCnt = 1;
    me_rdma0.src_img_yuv_addr[0] = p_parameter->src_yuv_img_addr;
    me_rdma0.src_img_size = p_parameter->src_img_size;
    me_rdma0.src_img_roi  = p_parameter->src_img_roi;
    me_rdma0.src_color_format = p_parameter->src_color_format;
    me_rdma0.bHWTrigger = 0;
    me_rdma0.bContinuous = 0;
    me_rdma0.bCamIn = 0;
    me_rdma0.bEISEn = 0;
    me_rdma0.u4EISCON = 0;//EIS setting

    // RDMA0
    me_rdma0.to_cam = 1;
    me_rdma0.to_ovl = 0;
    me_rdma0.to_mout = 0;
    me_rdma0.trigger_src = 0;

    // CRZ
    me_crz.src_sel = 2; // 0:RDMA0 , 1:BRZ, 2:Camera
    me_crz.src_img_size.w = me_rdma0.src_img_roi.w;
    me_crz.src_img_size.h = me_rdma0.src_img_roi.h;
    me_crz.src_img_roi.x = 0;
    me_crz.src_img_roi.y = 0;
    me_crz.src_img_roi.w = me_crz.src_img_size.w;
    me_crz.src_img_roi.h = me_crz.src_img_size.h;

    me_crz.dst_img_size.w = p_parameter->dst_img_roi.w;
    me_crz.dst_img_size.h = p_parameter->dst_img_roi.h;

    // IPP
    me_ipp.bBypass = 1;
    me_ipp.src_sel = 1; //0 : OVL, 1:CRZ

    //PRZ0 max input dimension : 1280
    if(1280 < p_parameter->dst_img_roi.w)
    {
        me_ipp.to_prz0 = 0;
        me_ipp.to_rgb_rot0 = ((YV16_Planar > p_parameter->dst_color_format) ? 1 : 0);
        me_ipp.to_vdo_rot0 = (me_ipp.to_rgb_rot0 ? 0 : 1);
    }
    else
    {
        me_ipp.to_prz0 = 1;
        me_ipp.to_rgb_rot0 = 0;
        me_ipp.to_vdo_rot0 = 0;
    }

    /***PRZ0***//*Rescale*/
    //RESZ_I
    me_prz0.src_img_size.w = me_crz.dst_img_size.w;
    me_prz0.src_img_size.h = me_crz.dst_img_size.h;
    me_prz0.src_img_roi.x = 0;
    me_prz0.src_img_roi.y = 0;
    me_prz0.src_img_roi.w = me_prz0.src_img_size.w;
    me_prz0.src_img_roi.h = me_prz0.src_img_size.h;

    me_prz0.dst_img_size.w = me_prz0.src_img_size.w; /*Currently dst roi is not used*/
    me_prz0.dst_img_size.h = me_prz0.src_img_size.h;

    me_prz0.uUpScaleCoeff = 8;
    me_prz0.uDnScaleCoeff = 15;

    me_prz0.bContinuous = 0;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    me_prz0.bCamIn = 0;
    me_prz0.bBypass = 0;

    //PRZ0
    me_prz0.src_sel = 1;//0-MOUT,1-IPP_MOUT,2-CAM,3-BRZ_MOUT

    /*output sel*/
    me_prz0.to_rgb_rot0 = ((YV16_Planar > p_parameter->dst_color_format) ? 1 : 0);
    me_prz0.to_vdo_rot0 = (me_prz0.to_rgb_rot0 ? 0 : 1);
    me_prz0.to_vrz0= 0;

    me_prz0.bCamIn= 0;

    /***RGBROT0***/
    //ROTDMA_I
    me_rgbrot0.dst_img_yuv_addr[0] = p_parameter->dst_yuv_img_addr;
    me_vdorot0.dst_img_yuv_addr[0] = me_rgbrot0.dst_img_yuv_addr[0];

    me_rgbrot0.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_vdorot0.src_img_roi.x = me_rgbrot0.src_img_roi.x;

    me_rgbrot0.src_img_roi.y = 0;
    me_vdorot0.src_img_roi.y = me_rgbrot0.src_img_roi.y;

    me_rgbrot0.src_img_roi.w = me_prz0.dst_img_size.w;
    me_vdorot0.src_img_roi.w = me_rgbrot0.src_img_roi.w;

    me_rgbrot0.src_img_roi.h = me_prz0.dst_img_size.h;
    me_vdorot0.src_img_roi.h = me_rgbrot0.src_img_roi.h;

    me_rgbrot0.dst_img_size = p_parameter->dst_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_vdorot0.dst_img_size = me_rgbrot0.dst_img_size;

    me_rgbrot0.dst_color_format = p_parameter->dst_color_format;
    me_vdorot0.dst_color_format = me_rgbrot0.dst_color_format;

    me_rgbrot0.uOutBufferCnt = 1;
    me_vdorot0.uOutBufferCnt = me_rgbrot0.uOutBufferCnt;

    me_rgbrot0.bContinuous = 0;// 1:Continuous mode does not allow time sharing.
    me_vdorot0.bContinuous = me_rgbrot0.bContinuous;

    me_rgbrot0.bFlip = p_parameter->dst_rotate_angle >> 2;
    me_vdorot0.bFlip = me_rgbrot0.bFlip;

    me_rgbrot0.bRotate = (0x3 & p_parameter->dst_rotate_angle);//0:0, 1:90, 2:180, 3:270
    me_vdorot0.bRotate = me_rgbrot0.bRotate;

#if defined MDP_DRIVER_DVT_SUPPORT
    me_rgbrot0.bDithering = p_parameter->dst_dither_en;
#else
    me_rgbrot0.bDithering = 0;
#endif
    me_vdorot0.bDithering = me_rgbrot0.bDithering;

    me_rgbrot0.bCamIn = 0;// 1: real time path, ex : camera input
    me_vdorot0.bCamIn = me_rgbrot0.bCamIn;

    me_rgbrot0.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable
    me_vdorot0.bEnHWTriggerRDMA0 = me_rgbrot0.bEnHWTriggerRDMA0;

    //RGBROT0
    if(1280 < p_parameter->dst_img_roi.w)
    {
        me_rgbrot0.src_sel= 1;    //0-PRZ0_MOUT ,1-IPP_MOUT
    }
    else
    {
        me_rgbrot0.src_sel= 0;    //0-PRZ0_MOUT ,1-IPP_MOUT
    }

     me_vdorot0.src_sel = me_rgbrot0.src_sel;

    //ISP part
    pIspDisp = IspDisp::CreateInstance();
    if(pIspDisp == NULL)
    {
        MDP_ERROR("Fail to get isp tuning interface base");
        return MDP_ERROR_CODE_FAIL;
    }
    //

    //
    if(!(pIspDisp->CheckSize(me_rdma0.src_img_roi.w,me_rdma0.src_img_roi.h)))
    {
        MDP_ERROR("pIspDisp->CheckSize() fail");
        ret_val = MDP_ERROR_CODE_FAIL;
        goto NORES_EXIT;
    }

    //Lock isp resource
    pISPTuning = DisplayIspTuningIFBase::createInstance();

    if(pISPTuning == NULL)
    {
        MDP_ERROR("Fail to get isp tuning interface base");
        return MDP_ERROR_CODE_FAIL;
     }
     if(!(pIspDisp->Lock(30)))
     {
         MDP_WARNING("pIspDisp->Lock() fail");
         ret_val = MDP_ERROR_CODE_LOCK_RESOURCE_FAIL;
         goto NORES_EXIT;
     }

    //Open isp
    if(!(pIspDisp->Init()))
    {
        MDP_ERROR("pIspDisp->Init fail()");
        ret_val = MDP_ERROR_CODE_FAIL;
        goto ERR_EXIT;
    }
    //Config isp
    IspDispConfig.InputSize.Width = me_rdma0.src_img_roi.w;
    IspDispConfig.InputSize.Height = me_rdma0.src_img_roi.h;
    if(!(pIspDisp->Config(&IspDispConfig)))
    {
        MDP_ERROR("pIspDisp->Config() fail");
        ret_val = MDP_ERROR_CODE_FAIL;
        goto ERR_EXIT;
    }

    //Init isp tuning if
    if(MHAL_NO_ERROR != pISPTuning->init())
    {
        MDP_ERROR("init isp tuning failed");
        ret_val = MDP_ERROR_CODE_FAIL;
        goto ERR_EXIT;
    }

    //Load ISP param

     if(MHAL_NO_ERROR != pISPTuning->loadISPParam())
     {
         MDP_ERROR("init isp tuning failed");
         ret_val = MDP_ERROR_CODE_FAIL;
         goto ERR_EXIT;
     }

     PRZ_PARAM = pISPTuning->getPRZParam();

     /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
//     me_prz0.uEEHCoeff = PRZ_PARAM.uEEHCoeff;
//     me_prz0.uEEVCoeff = PRZ_PARAM.uEEVCoeff;
     me_prz0.uEEHCoeff = (2 == u4ProcFlag) ? 0 : PRZ_PARAM.uEEHCoeff;
     me_prz0.uEEVCoeff = (2 == u4ProcFlag) ? 0 : PRZ_PARAM.uEEHCoeff;
     me_prz0.uUpScaleCoeff = PRZ_PARAM.uUpScaleCoeff;
     me_prz0.uDnScaleCoeff = PRZ_PARAM.uDnScaleCoeff;

    /*Clouds Move to callback function*/
    /*
    //Start isp process
    if(!(pIspDisp->Run(MTRUE)))
    {
        MDP_ERROR("isp_disp_run start fail");
        ret_val = MDP_ERROR_CODE_FAIL;
        goto ERR_EXIT;
    }
    */

    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Trigger HW
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    ret_val = _MdpPathTriggerHw( mdp_element_list , mdp_element_count ,
                                 &pq_cust_func_tbl, (void*)pIspDisp );

    if(MDP_ERROR_CODE_FAIL == ret_val)
    {
        pIspDisp->DumpReg();
    }

//    if( ret_val == MDP_ERROR_CODE_LOCK_RESOURCE_FAIL )
//            return ret_val;

ERR_EXIT:

    /*Clouds Move to callback function*/
    /*
    //Close isp
    isp_disp_run(MFALSE);
    pIspDisp->Run(MFALSE);
    isp_disp_run(MFALSE);
    */

    pISPTuning->unloadISPParam();
    pISPTuning->deinit();
    pISPTuning->destroyInstance();
    pIspDisp->Uninit();
    pIspDisp->Unlock();
    pIspDisp->DestroyInstance();

NORES_EXIT:

    return ret_val;
}

