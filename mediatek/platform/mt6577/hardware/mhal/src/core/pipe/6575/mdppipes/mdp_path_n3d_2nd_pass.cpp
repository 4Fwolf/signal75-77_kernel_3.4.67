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
  C++ Version - MdpPathCameraPreviewToMemory
  /////////////////////////////////////////////////////////////////////////////*/
int MdpPathN3d2ndPass::Config( struct MdpPathN3d2ndPassParameter* p_parameter )
{
    unsigned long i;
    unsigned long src_buffer_count = p_parameter->src_buffer_count;
    unsigned long dst_buffer_count = p_parameter->dst_buffer_count;
    unsigned long b_is_vrz0_path = ( p_parameter->src_img_roi.w > RESZ_PRZ1_MAX_MINWIDTH )?1:0; //if width greater than 960,use vrz0 path
    unsigned long b_continuous_dma;     //continous bit for rdma,rotdma
    unsigned long b_continuous_nondma;  //continous bit for non-rdma/rotdma

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

    /*Resource List*/
    m_mdp_element_count = 0;
    m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_rdma0;
    m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_mout;
    if( b_is_vrz0_path ){
    m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vrz0;
    }else{
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_prz1;
    }
    m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vdo1;
    
   
    #if defined(MDP_FLAG_1_SUPPORT_DESCRIPTOR)
        b_continuous_dma    = 0;             /*In desc mode, continous of RDMA/ROTDMA should be 0*/
        b_continuous_nondma = 1;            /*In descriptor mode, continues should be on except RDMA and ROTDMA*/
    #else
        b_continuous_dma    = 0;            /*In register mode, continous of RDMA/ROTDMA just as before*/
        b_continuous_nondma = 0;
    #endif



    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Setup mdp elements parameter
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /***RDMA0***//*Crop*/
    //RDMA_I
    me_rdma0.uInBufferCnt = src_buffer_count;
    me_rdma0.src_img_yuv_addr[0] = p_parameter->src_yuv_img_addr;   /*Use [0] to store the first buffer, calculate the rest in element config()*/
    me_rdma0.src_img_size = p_parameter->src_img_size;
    me_rdma0.src_img_roi  = p_parameter->src_img_roi;               
    me_rdma0.src_color_format = p_parameter->src_color_format;
    
    
    //me_rdma0.bContinuousHWTrigger = 0;// HW trigger + continous + framesync
    me_rdma0.bHWTrigger = 0;
    me_rdma0.bContinuous = b_continuous_dma;
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
    me_mout.to_vrz0 = b_is_vrz0_path ;
    me_mout.to_prz1 = !b_is_vrz0_path;
    
    me_mout.bCamIn = 0;


    if( b_is_vrz0_path )
    {
    /***VRZ0***//*Rescale*/
    //RESZ_I
    me_vrz0.src_img_size   = MdpSize( me_rdma0.src_img_roi.w, me_rdma0.src_img_roi.h );
    me_vrz0.dst_img_size   = MdpSize( p_parameter->dst_img_roi.w, p_parameter->dst_img_roi.h );

    me_vrz0.bContinuous = b_continuous_nondma;
    me_vrz0.bCamIn = 0;

    //VRZ0
    /*N/A*/
    me_vrz0.src_sel = 0;// 0-MOUT, 1-BRZ_MOUT, 2-IPP_MOUT, 3-OVL_DMA_MIMO, 4-PRZ0_MOUT
    }
    else
    {
        /***PRZ1***//*Rescale*/
        //RESZ_I
        me_prz1.src_img_size = MdpSize( me_rdma0.src_img_roi.w, me_rdma0.src_img_roi.h );

        me_prz1.src_img_roi = p_parameter->dst_src_img_roi;

        me_prz1.dst_img_size = MdpSize( p_parameter->dst_img_roi.w, p_parameter->dst_img_roi.h );

        me_prz1.uUpScaleCoeff = p_parameter->resz_coeff.prz1_up_scale_coeff;
        me_prz1.uDnScaleCoeff = p_parameter->resz_coeff.prz1_dn_scale_coeff;

        /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
        me_prz1.uEEHCoeff = p_parameter->resz_coeff.prz1_ee_h_str;
        me_prz1.uEEVCoeff = p_parameter->resz_coeff.prz1_ee_v_str;;

        me_prz1.bContinuous = b_continuous_nondma;
        me_prz1.bCamIn = 0;

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


    /***VDOROT1***/
    //ROTDMA_I
    me_vdo1.dst_img_yuv_addr[0] = p_parameter->dst_yuv_img_addr;
    me_vdo1.src_img_roi = p_parameter->dst_img_roi;
    me_vdo1.dst_img_size = p_parameter->dst_img_size; 
    me_vdo1.dst_color_format = p_parameter->dst_color_format;
    
    me_vdo1.uOutBufferCnt = dst_buffer_count;
    me_vdo1.bContinuous = b_continuous_dma;
    me_vdo1.bFlip = p_parameter->dst_flip;
    me_vdo1.bRotate = p_parameter->dst_rotate;
    me_vdo1.bDithering = 0;
    me_vdo1.bCamIn = 0;// 1: real time path, ex : camera input
    me_vdo1.bEnHWTriggerRDMA0 = 0;

    //VDOROT1
    me_vdo1.src_sel = b_is_vrz0_path;// 0-PRZ1, 1-VRZ0
     

    return 0;
    
}




