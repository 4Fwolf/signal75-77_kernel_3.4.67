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
  C++ Version - MdpPathCamera2PreviewToMemory
  /////////////////////////////////////////////////////////////////////////////*/
int MdpPathCamera2PreviewToMemory::Config( struct MdpPathCamera2PreviewToMemoryParameter* p_parameter )
{
    
    unsigned long src_buffer_count = p_parameter->pseudo_src_buffer_count;
    unsigned long dst_buffer_count = p_parameter->dst_buffer_count;
    unsigned long desc_mode_b_continuous;

    /*-----------------------------------------------------------------------------
        Parameter Sanity Check
      -----------------------------------------------------------------------------*/
    if( src_buffer_count > MDP_MAX_RINGBUFFER_CNT )
    {
        MDP_ERROR("%s: src buffer count(%d) > MDP_MAX_RINGBUFFER_CNT(%d)\n", this->name_str(),(int)src_buffer_count, MDP_MAX_RINGBUFFER_CNT);
        return -1;
    }
    if( src_buffer_count == 0 )
        src_buffer_count = 1;

    
    if( dst_buffer_count > MDP_MAX_RINGBUFFER_CNT )
    {
        MDP_ERROR("%s: dst buffer count(%d) > MDP_MAX_RINGBUFFER_CNT(%d)\n", this->name_str(),(int)dst_buffer_count, MDP_MAX_RINGBUFFER_CNT);
        return -1;
    }
    if( dst_buffer_count == 0 )
        dst_buffer_count = 1;

    
    /*-----------------------------------------------------------------------------
        Mdp List
      -----------------------------------------------------------------------------*/
    m_mdp_element_count = 0;
    
    //Pseudo Source Enable: Use RDMA0 to take place of camera sensor
    if( p_parameter->pseudo_src_enable )
    {
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_rdma0;
        
        b_camera_in = 0;    
        b_continuous = 0;   
    } else
    {
        b_camera_in = 1;    
        b_continuous = 1; 
    }

    m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_prz0;
    m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vdorot0;

    /*DEBUG:If enable, force to be single frame*/
    if( p_parameter->debug_preview_single_frame_enable )
    {
        b_continuous = 0;
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
    //Pseudo Source Enable: Use RDMA0 to take place of camera sensor
    if( p_parameter->pseudo_src_enable )
    {
        /*.............................................................................*/
        /***RDMA0***//*Crop*/
        /*.............................................................................*/
        //RDMA_I
        me_rdma0.uInBufferCnt = src_buffer_count;
        me_rdma0.src_img_yuv_addr[0] = p_parameter->pseudo_src_yuv_img_addr;                                 
        me_rdma0.src_img_size = p_parameter->src_img_size;

        me_rdma0.src_img_roi.x = 0;     //Don't use roi,let prz0 take care, because rdma0 is pseudo
        me_rdma0.src_img_roi.y = 0;
        me_rdma0.src_img_roi.w = p_parameter->src_img_size.w;
        me_rdma0.src_img_roi.h = p_parameter->src_img_size.h;

        me_rdma0.src_color_format = p_parameter->pseudo_src_color_format;
        
        me_rdma0.bHWTrigger = 0;
        me_rdma0.bContinuous = desc_mode_b_continuous;
        me_rdma0.bCamIn = 0;
        

        me_rdma0.bEISEn = 0;
        me_rdma0.u4EISCON = 0;//EIS setting
        //RDMA0
        me_rdma0.to_cam = 1;
        me_rdma0.to_ovl = 0;
        me_rdma0.to_mout = 0;
        
        me_rdma0.trigger_src = 0;
    }

        
    /*.............................................................................*/
    /***PRZ0***//*Rescale*/
    /*.............................................................................*/
    //RESZ_I
    me_prz0.src_img_size = p_parameter->src_img_size;
    me_prz0.src_img_roi =  p_parameter->src_img_roi;
    me_prz0.dst_img_size = p_parameter->dst_img_size;

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
    /*src_sel 0-MOUT, 1-IPP_MOUT,2-CAM(Sensor),3-BRZ_MOUT,6-CAM(RDMA0),10-CAM(BRZ)*/
    if( p_parameter->pseudo_src_enable ){
        me_prz0.src_sel = 6;
    }else{
        me_prz0.src_sel = 2;
    }

    /*output sel*/
    me_prz0.to_vdo_rot0 = 1;
    me_prz0.to_rgb_rot0 = 0;
    me_prz0.to_vrz0= 0;    


    

    /*m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vdorot0;*/
    /*.............................................................................*/
    /***VDOROT0***/
    /*.............................................................................*/
    //ROTDMA_I
    me_vdorot0.dst_img_yuv_addr[0] = p_parameter->dst_yuv_img_addr;

    me_vdorot0.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
    me_vdorot0.src_img_roi.y = 0;
    me_vdorot0.src_img_roi.w = me_prz0.dst_img_size.w;
    me_vdorot0.src_img_roi.h = me_prz0.dst_img_size.h;

    me_vdorot0.dst_img_size = me_prz0.dst_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
    me_vdorot0.dst_color_format = p_parameter->dst_color_format;

    me_vdorot0.uOutBufferCnt = dst_buffer_count;
    me_vdorot0.bContinuous = desc_mode_b_continuous;// 1:Continuous mode does not allow time sharing.
    me_vdorot0.bFlip = (p_parameter->dst_rotate_angle& 0x4) >> 2;
    me_vdorot0.bRotate = (0x3 & p_parameter->dst_rotate_angle);//0:0, 1:90, 2:180, 3:270
    me_vdorot0.bDithering = 0;
    me_vdorot0.bCamIn = b_camera_in;// 1: real time path, ex : camera input
    me_vdorot0.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

    //VDOROT0
    me_vdorot0.src_sel= 0;// 0-PRZ0_MOUT, 1-IPP_MOUT



    return 0;
    
}





