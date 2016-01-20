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

/*/////////////////////////////////////////////////////////////////////////////
    Macro Re-Definition
  /////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////
  C++ Version - MdpPathDisplayFromMemory
  /////////////////////////////////////////////////////////////////////////////*/
int MdpPathDisplayFromMemory::Config( struct MdpPathDisplayFromMemoryParameter* p_parameter )
{
    int b_PATH0_EN = p_parameter->en_dst_port0;
    int b_PATH1_EN = p_parameter->en_dst_port1;
    int b_PATH2_EN = p_parameter->en_dst_port2;


    unsigned long src_buffer_count  = p_parameter->src_buffer_count;
    unsigned long dst0_buffer_count = p_parameter->dst0_buffer_count;
    unsigned long dst1_buffer_count = p_parameter->dst1_buffer_count;
    unsigned long dst2_buffer_count = p_parameter->dst2_buffer_count;

    unsigned long desc_mode_b_continuous;


    //--
    if( src_buffer_count > MDP_MAX_RINGBUFFER_CNT )
    {
        MDP_ERROR("%s: src buffer count(%d) > MDP_MAX_RINGBUFFER_CNT(%d)\n", this->name_str(),(int)src_buffer_count, MDP_MAX_RINGBUFFER_CNT);
        return -1;
    }
    if( src_buffer_count == 0 )
        src_buffer_count = 1;
    //--
    if( dst0_buffer_count > MDP_MAX_RINGBUFFER_CNT )
    {
        MDP_ERROR("%s: dst0_buffer_count(%d) > MDP_MAX_RINGBUFFER_CNT(%d)\n", this->name_str(),(int)dst0_buffer_count, MDP_MAX_RINGBUFFER_CNT);
        return -1;
    }
    if( dst0_buffer_count == 0 )
        dst0_buffer_count = 1;
    //--
    if( dst1_buffer_count > MDP_MAX_RINGBUFFER_CNT )
    {
        MDP_ERROR("%s: dst1_buffer_count(%d) > MDP_MAX_RINGBUFFER_CNT(%d)\n", this->name_str(),(int)dst1_buffer_count, MDP_MAX_RINGBUFFER_CNT);
        return -1;
    }
    if( dst1_buffer_count == 0 )
        dst1_buffer_count = 1;
    //--
    if( dst2_buffer_count > MDP_MAX_RINGBUFFER_CNT )
    {
        MDP_ERROR("%s: dst0_buffer_count(%d) > MDP_MAX_RINGBUFFER_CNT(%d)\n", this->name_str(),(int)dst2_buffer_count, MDP_MAX_RINGBUFFER_CNT);
        return -1;
    }
    if( dst2_buffer_count == 0 )
        dst2_buffer_count = 1;
    //--




    b_camera_in = p_parameter->b_camera_in;
    b_continuous = p_parameter->b_continuous;   //default value for camera



    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Setup path on/off combination
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /*No path has been enabled*/
    if( ( b_PATH0_EN == 0 ) && ( b_PATH1_EN == 0 ) && ( b_PATH2_EN == 0 ) )
    {
        m_mdp_element_count = 0;
        MDP_ERROR("no path enabled\n");
        return -1;
    } else

    {
        m_mdp_element_count = 0;

        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_rdma0;
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_mout;

        if( b_PATH0_EN == 1 )
        {
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_prz0;/*0*/
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vdo0;/*0*/
        }

        if( b_PATH1_EN == 1 )
        {
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_prz1;/*1*/
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vdo1;/*1*/
        }

        if( b_PATH2_EN == 1 )
        {
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vrz0;/*2*/
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_rgb1;/*2*/
        }
    }




#if defined(MDP_FLAG_1_SUPPORT_DESCRIPTOR)
        desc_mode_b_continuous = 0;             /*In desc mode, continous of RDMA/ROTDMA should be 0*/
        b_continuous = 1;                       /*In descriptor mode, continues should be on except RDMA and ROTDMA*/
#else
        desc_mode_b_continuous = b_continuous;  /*In register mode, continous of RDMA/ROTDMA just as before*/
#endif



    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Setup mdp elements parameter
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /***RDMA0***//*Crop*/
    //RDMA_I
    me_rdma0.uInBufferCnt = src_buffer_count;
    me_rdma0.src_img_yuv_addr[0] = p_parameter->src_yuv_img_addr;
    me_rdma0.src_img_size = p_parameter->src_img_size;
    me_rdma0.src_img_roi =  p_parameter->src_img_roi;
    me_rdma0.src_color_format = p_parameter->src_color_format;


    //me_rdma0.bContinuousHWTrigger = b_continuous;// HW trigger + continous + framesync
    me_rdma0.bHWTrigger = p_parameter->b_hw_trigger_in;
    me_rdma0.bContinuous = desc_mode_b_continuous;
    me_rdma0.bCamIn = b_camera_in;




    me_rdma0.bEISEn = p_parameter->b_eis;

    if( me_rdma0.bEISEn ){
        me_rdma0.u4EISCON = MdpDrv_RDMA_EIS_CON_Value( p_parameter->eis_float_x, p_parameter->eis_float_y );//EIS setting
    }else {
        me_rdma0.u4EISCON = 0;
    }
    //RDMA0
    me_rdma0.to_cam = 0;
    me_rdma0.to_ovl = 0;
    me_rdma0.to_mout = 1;

    //HW Trigger
    me_rdma0.trigger_src = p_parameter->hw_trigger_src;



    /***MOUT***/
    me_mout.src_sel = 0;      // 0-R_DMA0_MOUT, 1-OVL_DMA_MIMO

    me_mout.to_jpg_dma = 0;   //bit[0] : JPEG_DMA, bit[1] : PRZ0, bit[2] : VRZ,bit[3] : VDO_ROT1
    me_mout.to_prz0 = b_PATH0_EN;
    me_mout.to_vrz0 = b_PATH2_EN;
    me_mout.to_prz1 = b_PATH1_EN;

    me_mout.bCamIn = b_camera_in;


    if( b_PATH0_EN == 1 )
    {
    /***PRZ0***//*Rescale*/
     //RESZ_I
     me_prz0.src_img_size.w = me_rdma0.src_img_roi.w;
     me_prz0.src_img_size.h = me_rdma0.src_img_roi.h;

     me_prz0.src_img_roi = p_parameter->dst0_src_img_roi;

     me_prz0.dst_img_size.w = p_parameter->dst0_img_size.w;//p_parameter->dst_img_roi.w; /*Currently dst roi is not used*/
     me_prz0.dst_img_size.h = p_parameter->dst0_img_size.h;//p_parameter->dst_img_roi.h;

     me_prz0.uUpScaleCoeff = p_parameter->resz_coeff.prz0_up_scale_coeff;
     me_prz0.uDnScaleCoeff = p_parameter->resz_coeff.prz0_dn_scale_coeff;

     /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
     me_prz0.uEEHCoeff = p_parameter->resz_coeff.prz0_ee_h_str;
     me_prz0.uEEVCoeff = p_parameter->resz_coeff.prz0_ee_v_str;

     me_prz0.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
     me_prz0.bCamIn = b_camera_in;


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
     me_prz0.to_vdo_rot0 = 1;
     me_prz0.to_rgb_rot0 = 0;
     me_prz0.to_vrz0= 0;
    }

    if( b_PATH1_EN == 1 )
    {
    /***PRZ1***//*Rescale*/
    //RESZ_I
    me_prz1.src_img_size.w = me_rdma0.src_img_roi.w;
    me_prz1.src_img_size.h = me_rdma0.src_img_roi.h;

    me_prz1.src_img_roi = p_parameter->dst1_src_img_roi;

    me_prz1.dst_img_size.w = p_parameter->dst1_img_size.w;//p_parameter->dst_img_roi.w; /*Currently dst roi is not used*/
    me_prz1.dst_img_size.h = p_parameter->dst1_img_size.h;//p_parameter->dst_img_roi.h;

    me_prz1.uUpScaleCoeff = p_parameter->resz_coeff.prz1_up_scale_coeff;
    me_prz1.uDnScaleCoeff = p_parameter->resz_coeff.prz1_dn_scale_coeff;

    /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
    me_prz1.uEEHCoeff = p_parameter->resz_coeff.prz1_ee_h_str;
    me_prz1.uEEVCoeff = p_parameter->resz_coeff.prz1_ee_v_str;;

    me_prz1.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    me_prz1.bCamIn = b_camera_in;

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
    }

    if( b_PATH2_EN == 1 )
    {
    /***VRZ0***//*Rescale*/
    //RESZ_I
    me_vrz0.src_img_size.w = me_rdma0.src_img_roi.w;
    me_vrz0.src_img_size.h = me_rdma0.src_img_roi.h;

    me_vrz0.dst_img_size.w = p_parameter->dst2_img_size.w;//p_parameter->dst_img_roi.w; /*Currently dst roi is not used*/
    me_vrz0.dst_img_size.h = p_parameter->dst2_img_size.h;//p_parameter->dst_img_roi.h;

    me_vrz0.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    me_vrz0.bCamIn = b_camera_in;

    //VRZ0
    /*N/A*/
    me_vrz0.src_sel = 0;// 0-MOUT, 1-BRZ_MOUT, 2-IPP_MOUT, 3-OVL_DMA_MIMO, 4-PRZ0_MOUT
    }

    if( b_PATH0_EN == 1 )
    {
    /***VDOROT0***/
    //ROTDMA_I
    me_vdo0.dst_img_yuv_addr[0] = p_parameter->dst0_yuv_img_addr;

    me_vdo0.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_vdo0.src_img_roi.y = 0;
    me_vdo0.src_img_roi.w = p_parameter->dst0_img_size.w;
    me_vdo0.src_img_roi.h = p_parameter->dst0_img_size.h;

    me_vdo0.dst_img_size = p_parameter->dst0_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_vdo0.dst_color_format = p_parameter->dst0_color_format;/*YUY2_Pack*/

    me_vdo0.uOutBufferCnt = dst0_buffer_count;
    me_vdo0.bContinuous = desc_mode_b_continuous;// 1:Continuous mode does not allow time sharing.
    me_vdo0.bFlip = (p_parameter->dst0_rotate_angle & 0x4) >> 2;
    me_vdo0.bRotate = p_parameter->dst0_rotate_angle & 0x3;//0:0, 1:90, 2:180, 3:270
    me_vdo0.bDithering = 0;
    me_vdo0.bCamIn = b_camera_in;// 1: real time path, ex : camera input
    me_vdo0.bEnHWTriggerRDMA0 = 0;//p_parameter->b_hw_trigger_in;// 0:Disable , 1:Enable

    //VDOROT0
    me_vdo0.src_sel = 0;// 0-PRZ0_MOUT, 1-IPP_MOUT
    }

    if( b_PATH1_EN == 1 )
    {
    /***VDOROT1***/
    //ROTDMA_I
    me_vdo1.dst_img_yuv_addr[0] = p_parameter->dst1_yuv_img_addr;

    me_vdo1.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_vdo1.src_img_roi.y = 0;
    me_vdo1.src_img_roi.w = p_parameter->dst1_img_size.w;
    me_vdo1.src_img_roi.h = p_parameter->dst1_img_size.h;

    me_vdo1.dst_img_size = p_parameter->dst1_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_vdo1.dst_color_format = p_parameter->dst1_color_format;/*YUY2_Pack*/

    me_vdo1.uOutBufferCnt = dst1_buffer_count;
    me_vdo1.bContinuous = desc_mode_b_continuous;// 1:Continuous mode does not allow time sharing.
    me_vdo1.bFlip = (p_parameter->dst1_rotate_angle & 0x4) >> 2;
    me_vdo1.bRotate = p_parameter->dst1_rotate_angle& 0x3;//0:0, 1:90, 2:180, 3:270
    me_vdo1.bDithering = 0;
    me_vdo1.bCamIn = b_camera_in;// 1: real time path, ex : camera input
    me_vdo1.bEnHWTriggerRDMA0 = 0;//p_parameter->b_hw_trigger_in;// 0:Disable , 1:Enable

    //VDOROT1
    me_vdo1.src_sel = 0;// 0-PRZ1, 1-VRZ0
    }


    if( b_PATH2_EN == 1 )
    {
    /***RGBROT1***/
    //ROTDMA_I
    me_rgb1.dst_img_yuv_addr[0] = p_parameter->dst2_yuv_img_addr;

    me_rgb1.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_rgb1.src_img_roi.y = 0;
    me_rgb1.src_img_roi.w = p_parameter->dst2_img_size.w;
    me_rgb1.src_img_roi.h = p_parameter->dst2_img_size.h;

    me_rgb1.dst_img_size = p_parameter->dst2_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_rgb1.dst_color_format = p_parameter->dst2_color_format;/*YUY2_Pack*/

    me_rgb1.uOutBufferCnt = dst2_buffer_count;
    me_rgb1.bContinuous = desc_mode_b_continuous;// 1:Continuous mode does not allow time sharing.
    me_rgb1.bFlip = (p_parameter->dst2_rotate_angle & 0x4) >> 2;
    me_rgb1.bRotate = p_parameter->dst2_rotate_angle & 0x3;//0:0, 1:90, 2:180, 3:270
    me_rgb1.bDithering = 0;
    me_rgb1.bCamIn = b_camera_in;// 1: real time path, ex : camera input
    me_rgb1.bEnHWTriggerRDMA0 = 0;//p_parameter->b_hw_trigger_in;// 0:Disable , 1:Enable

    //RGBROT1
    /*Nothing*/
    }


    return 0;

}


void Printout_MdpPathDisplayFromMemoryParameter( MdpPathDisplayFromMemoryParameter* p_param )
{
    /*HW Trigger Setting*/
    MDP_INFO("b_hw_trigger_in = %d\n", p_param->b_hw_trigger_in );    /*Enaable hw trigger input*/
    MDP_INFO("hw_trigger_src = %lu\n", p_param->hw_trigger_src );     /*0:VDO_ROT0 1:RGB_ROT0 2:RGB_ROT1 3:VDO_ROT1*/    /*trigger source*/
    MDP_INFO("b_camera_in = %d\n", p_param->b_camera_in );
    MDP_INFO("b_continuous = %d\n", p_param->b_continuous );
    MDP_INFO("b_eis = %d\n", p_param->b_eis );              /*enable EIS*/
        MDP_INFO("eis_float_x = %lu\n", p_param->eis_float_x );
        MDP_INFO("eis_float_y = %lu\n", p_param->eis_float_y );
    MDP_INFO("\n");

    /*Source*/
    MDP_INFO("src_buffer_count = %lu\n", p_param->src_buffer_count );
    MDP_INFO("src_color_format = %d\n", (int)p_param->src_color_format);
    MDP_INFO("src_img_size = %lu , %lu\n", p_param->src_img_size.w, p_param->src_img_size.h );
    MDP_INFO("src_img_roi = %lu , %lu , %lu , %lu\n", p_param->src_img_roi.x, p_param->src_img_roi.y, p_param->src_img_roi.w, p_param->src_img_roi.h);
    MDP_INFO("src_yuv_img_addr.y = 0x%08X\n", (unsigned int)p_param->src_yuv_img_addr.y );
    MDP_INFO("\n");

    /*For Display : NV21*/
    MDP_INFO("en_dst_port0 = %d\n", p_param->en_dst_port0);    //enable dst port0
    MDP_INFO("dst0_src_img_roi = %lu , %lu , %lu , %lu\n", p_param->dst0_src_img_roi.x, p_param->dst0_src_img_roi.y, p_param->dst0_src_img_roi.w, p_param->dst0_src_img_roi.h );
    MDP_INFO("dst0_buffer_count = %lu\n", p_param->dst0_buffer_count );
    MDP_INFO("dst0_color_format = %d\n", (int)p_param->dst0_color_format );
    MDP_INFO("dst0_img_size = %lu , %lu\n", p_param->dst0_img_size.w, p_param->dst0_img_size.h );       /*Currently, "dst roi"  should be equal to "dst size"*/
    MDP_INFO("dst0_img_roi = %lu , %lu , %lu , %lu\n", p_param->dst0_img_roi.x, p_param->dst0_img_roi.y, p_param->dst0_img_roi.w, p_param->dst0_img_roi.h );     /*dst_img_roi is not used*/
    MDP_INFO("dst0_yuv_img_addr.y = 0x%08X\n", (unsigned int)p_param->dst0_yuv_img_addr.y );
    MDP_INFO("dst0_rotate_angle = %d\n", (int)p_param->dst0_rotate_angle );
    MDP_INFO("\n");


    /*For Encode : YV12*/
    MDP_INFO("en_dst_port1 = %d\n", p_param->en_dst_port1);    //enable dst port1
    MDP_INFO("dst1_src_img_roi = %lu , %lu , %lu , %lu\n", p_param->dst1_src_img_roi.x, p_param->dst1_src_img_roi.y, p_param->dst1_src_img_roi.w, p_param->dst1_src_img_roi.h );
    MDP_INFO("dst1_buffer_count = %lu\n", p_param->dst1_buffer_count );
    MDP_INFO("dst1_color_format = %d\n", (int)p_param->dst1_color_format );
    MDP_INFO("dst1_img_size = %lu , %lu\n", p_param->dst1_img_size.w, p_param->dst1_img_size.h );       /*Currently, "dst roi"  should be equal to "dst size"*/
    MDP_INFO("dst1_img_roi = %lu , %lu , %lu , %lu\n", p_param->dst1_img_roi.x, p_param->dst1_img_roi.y, p_param->dst1_img_roi.w, p_param->dst1_img_roi.h );     /*dst_img_roi is not used*/
    MDP_INFO("dst1_yuv_img_addr.y = 0x%08X\n", (unsigned int)p_param->dst1_yuv_img_addr.y );
    MDP_INFO("dst1_rotate_angle = %d\n", (int)p_param->dst1_rotate_angle );
    MDP_INFO("\n");



    /*For FD : RGB*/
    MDP_INFO("en_dst_port2 = %d\n", p_param->en_dst_port2);    //enable dst port2
    MDP_INFO("dst2_buffer_count = %lu\n", p_param->dst2_buffer_count );
    MDP_INFO("dst2_color_format = %d\n", (int)p_param->dst2_color_format );
    MDP_INFO("dst2_img_size = %lu , %lu\n", p_param->dst2_img_size.w, p_param->dst2_img_size.h );       /*Currently, "dst roi"  should be equal to "dst size"*/
    MDP_INFO("dst2_img_roi = %lu , %lu , %lu , %lu\n", p_param->dst2_img_roi.x, p_param->dst2_img_roi.y, p_param->dst2_img_roi.w, p_param->dst2_img_roi.h );     /*dst_img_roi is not used*/
    MDP_INFO("dst2_yuv_img_addr.y = 0x%08X\n", (unsigned int)p_param->dst2_yuv_img_addr.y );
    MDP_INFO("dst2_rotate_angle = %d\n", (int)p_param->dst2_rotate_angle );
    MDP_INFO("\n");




}



/*/////////////////////////////////////////////////////////////////////////////
  C Version - MdpPathDisplayFromMemory
  /////////////////////////////////////////////////////////////////////////////*/

int MdpPathDisplayFromMemory_Func( struct MdpPathDisplayFromMemoryParameter* p_parameter )
{
    int ret_val = 0;

    //MDP Drv operation
    int             mdp_element_count = 8; //default value
    MDPELEMENT_I*   mdp_element_list[MDP_ELEMENT_MAX_NUM];

    //MDP Elements
    RDMA0       me_rdma0;
    MOUT        me_mout;

    PRZ0        me_prz0;
    PRZ1        me_prz1;
    VRZ0        me_vrz0;

    VDOROT0     me_vdo0;
    VDOROT1     me_vdo1;
    RGBROT1     me_rgb1;


    //Misc TODO:When 2 stage, should camera_in and continuous be turn on??
    int     b_camera_in = 0;//1;    //default value
    int     b_continuous = 0;//1;   //default value

    MDP_SHOWFUNCTION();


    /*Resource List*/
    mdp_element_list[0] = (MDPELEMENT_I*)&me_rdma0;
    mdp_element_list[1] = (MDPELEMENT_I*)&me_mout;
    mdp_element_list[2] = (MDPELEMENT_I*)&me_prz0;
    mdp_element_list[3] = (MDPELEMENT_I*)&me_prz1;
    mdp_element_list[4] = (MDPELEMENT_I*)&me_vrz0;
    mdp_element_list[5] = (MDPELEMENT_I*)&me_vdo0;
    mdp_element_list[6] = (MDPELEMENT_I*)&me_vdo1;
    mdp_element_list[7] = (MDPELEMENT_I*)&me_rgb1;







    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Setup mdp elements parameter
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /***RDMA0***//*Crop*/
    //RDMA_I
    me_rdma0.uInBufferCnt = 1;
    me_rdma0.src_img_yuv_addr[0] = p_parameter->src_yuv_img_addr;
    me_rdma0.src_img_size = p_parameter->src_img_size;
    me_rdma0.src_img_roi =  p_parameter->src_img_roi;
    me_rdma0.src_color_format = p_parameter->src_color_format;


    //me_rdma0.bContinuousHWTrigger = b_continuous;// HW trigger + continous + framesync
    me_rdma0.bHWTrigger = 0;
    me_rdma0.bContinuous = b_continuous;
    me_rdma0.bCamIn = b_camera_in;

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
    me_mout.to_vrz0 = 1;
    me_mout.to_prz1 = 1;

    me_mout.bCamIn = b_camera_in;


    /***PRZ0***//*Rescale*/
     //RESZ_I
     me_prz0.src_img_size.w = me_rdma0.src_img_roi.w;
     me_prz0.src_img_size.h = me_rdma0.src_img_roi.h;
     me_prz0.src_img_roi.x = 0;
     me_prz0.src_img_roi.y = 0;
     me_prz0.src_img_roi.w = me_prz0.src_img_size.w;
     me_prz0.src_img_roi.h = me_prz0.src_img_size.h;

     me_prz0.dst_img_size.w = p_parameter->dst0_img_size.w;//p_parameter->dst_img_roi.w; /*Currently dst roi is not used*/
     me_prz0.dst_img_size.h = p_parameter->dst0_img_size.h;//p_parameter->dst_img_roi.h;

     me_prz0.uUpScaleCoeff = p_parameter->resz_coeff.prz0_up_scale_coeff;
     me_prz0.uDnScaleCoeff = p_parameter->resz_coeff.prz0_dn_scale_coeff;

     /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
     me_prz0.uEEHCoeff = p_parameter->resz_coeff.prz0_ee_h_str;
     me_prz0.uEEVCoeff = p_parameter->resz_coeff.prz0_ee_v_str;

     me_prz0.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
     me_prz0.bCamIn = b_camera_in;
     me_prz0.bBypass = 0;

     //PRZ0
     me_prz0.src_sel = 0;;//0-MOUT,1-IPP_MOUT,2-CAM,3-BRZ_MOUT

     /*output sel*/
     me_prz0.to_vdo_rot0 = 1;
     me_prz0.to_rgb_rot0 = 0;
     me_prz0.to_vrz0= 0;


    /***PRZ1***//*Rescale*/
    //RESZ_I
    me_prz1.src_img_size.w = me_rdma0.src_img_roi.w;
    me_prz1.src_img_size.h = me_rdma0.src_img_roi.h;
    me_prz1.src_img_roi.x = 0;
    me_prz1.src_img_roi.y = 0;
    me_prz1.src_img_roi.w = me_prz1.src_img_size.w;
    me_prz1.src_img_roi.h = me_prz1.src_img_size.h;

    me_prz1.dst_img_size.w = p_parameter->dst1_img_size.w;//p_parameter->dst_img_roi.w; /*Currently dst roi is not used*/
    me_prz1.dst_img_size.h = p_parameter->dst1_img_size.h;//p_parameter->dst_img_roi.h;

    me_prz1.uUpScaleCoeff = p_parameter->resz_coeff.prz1_up_scale_coeff;
    me_prz1.uDnScaleCoeff = p_parameter->resz_coeff.prz1_dn_scale_coeff;

    /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
    me_prz1.uEEHCoeff = p_parameter->resz_coeff.prz1_ee_h_str;
    me_prz1.uEEVCoeff = p_parameter->resz_coeff.prz1_ee_v_str;

    me_prz1.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    me_prz1.bCamIn = b_camera_in;
    me_prz1.bBypass = 0;

    //PRZ1
    /*N/A*/


    /***VRZ0***//*Rescale*/
    //RESZ_I
    me_vrz0.src_img_size.w = me_rdma0.src_img_roi.w;
    me_vrz0.src_img_size.h = me_rdma0.src_img_roi.h;

    me_vrz0.dst_img_size.w = p_parameter->dst2_img_size.w;//p_parameter->dst_img_roi.w; /*Currently dst roi is not used*/
    me_vrz0.dst_img_size.h = p_parameter->dst2_img_size.h;//p_parameter->dst_img_roi.h;

    me_vrz0.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    me_vrz0.bCamIn = b_camera_in;

    //VRZ0
    /*N/A*/
    me_vrz0.src_sel = 0;// 0-MOUT, 1-BRZ_MOUT, 2-IPP_MOUT, 3-OVL_DMA_MIMO, 4-PRZ0_MOUT


    /***VDOROT0***/
    //ROTDMA_I
    me_vdo0.dst_img_yuv_addr[0] = p_parameter->dst0_yuv_img_addr;

    me_vdo0.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_vdo0.src_img_roi.y = 0;
    me_vdo0.src_img_roi.w = p_parameter->dst0_img_size.w;
    me_vdo0.src_img_roi.h = p_parameter->dst0_img_size.h;

    me_vdo0.dst_img_size = p_parameter->dst0_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_vdo0.dst_color_format = p_parameter->dst0_color_format;/*YUY2_Pack*/

    me_vdo0.uOutBufferCnt = 1;
    me_vdo0.bContinuous = b_continuous;// 1:Continuous mode does not allow time sharing.
    me_vdo0.bFlip = 0;
    me_vdo0.bRotate = p_parameter->dst0_rotate_angle;//0:0, 1:90, 2:180, 3:270
    me_vdo0.bDithering = 0;
    me_vdo0.bCamIn = b_camera_in;// 1: real time path, ex : camera input
    me_vdo0.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

    //VDOROT0
    me_vdo0.src_sel = 0;// 0-PRZ0_MOUT, 1-IPP_MOUT


    /***VDOROT1***/
    //ROTDMA_I
    me_vdo1.dst_img_yuv_addr[0] = p_parameter->dst1_yuv_img_addr;

    me_vdo1.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_vdo1.src_img_roi.y = 0;
    me_vdo1.src_img_roi.w = p_parameter->dst1_img_size.w;
    me_vdo1.src_img_roi.h = p_parameter->dst1_img_size.h;

    me_vdo1.dst_img_size = p_parameter->dst1_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_vdo1.dst_color_format = p_parameter->dst1_color_format;/*YUY2_Pack*/

    me_vdo1.uOutBufferCnt = 1;
    me_vdo1.bContinuous = b_continuous;// 1:Continuous mode does not allow time sharing.
    me_vdo1.bFlip = 0;
    me_vdo1.bRotate = p_parameter->dst1_rotate_angle;//0:0, 1:90, 2:180, 3:270
    me_vdo1.bDithering = 0;
    me_vdo1.bCamIn = b_camera_in;// 1: real time path, ex : camera input
    me_vdo1.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

    //VDOROT1
    me_vdo1.src_sel = 0;// 0-PRZ1, 1-VRZ0


    /***RGBROT1***/
    //ROTDMA_I
    me_rgb1.dst_img_yuv_addr[0] = p_parameter->dst2_yuv_img_addr;

    me_rgb1.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_rgb1.src_img_roi.y = 0;
    me_rgb1.src_img_roi.w = p_parameter->dst2_img_size.w;
    me_rgb1.src_img_roi.h = p_parameter->dst2_img_size.h;

    me_rgb1.dst_img_size = p_parameter->dst2_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_rgb1.dst_color_format = p_parameter->dst2_color_format;/*YUY2_Pack*/

    me_rgb1.uOutBufferCnt = 1;
    me_rgb1.bContinuous = b_continuous;// 1:Continuous mode does not allow time sharing.
    me_rgb1.bFlip = 0;
    me_rgb1.bRotate = p_parameter->dst2_rotate_angle;//0:0, 1:90, 2:180, 3:270
    me_rgb1.bDithering = 0;
    me_rgb1.bCamIn = b_camera_in;// 1: real time path, ex : camera input
    me_rgb1.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

    //RGBROT1
    /*Nothing*/







    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Trigger HW
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    ret_val = _MdpPathTriggerHw( mdp_element_list , mdp_element_count  );

    return ret_val;


}




