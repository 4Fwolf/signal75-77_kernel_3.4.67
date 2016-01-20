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
  C++ Version - MdpPathDisplayFromMemoryHdmi
  /////////////////////////////////////////////////////////////////////////////*/
int MdpPathDisplayFromMemoryHdmi::Config( struct MdpPathDisplayFromMemoryHdmiParameter* p_parameter )
{
    int b_HDMI_PATH = p_parameter->en_hdmi_port;
    int b_DISP_PATH = p_parameter->en_disp_port;

    

    b_camera_in  = 0;
    b_continuous = 0;   //default value for camera



    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Setup path on/off combination
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    /*No path has been enabled*/
    if( ( b_HDMI_PATH == 0 ) && ( b_DISP_PATH == 0 )  )
    {
        m_mdp_element_count = 0;
        return -1;
    } 
    else
    {
        m_mdp_element_count = 0;
        
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_rdma0;
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_mout;
        
        if( b_HDMI_PATH == 1 )
        {
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_prz0;/*0*/
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_rgb0;/*0*/
        }

        if( b_DISP_PATH == 1 )
        {
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vrz0;/*1*/
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vdo1;/*1*/
        }

    }



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
    me_rdma0.u4EISCON = 0;

    //RDMA0
    me_rdma0.to_cam = 0;
    me_rdma0.to_ovl = 0;
    me_rdma0.to_mout = 1;

    //HW Trigger
    me_rdma0.trigger_src = 0;


    
    /***MOUT***/
    me_mout.src_sel = 0;      // 0-R_DMA0_MOUT, 1-OVL_DMA_MIMO

    me_mout.to_jpg_dma = 0;   //bit[0] : JPEG_DMA, bit[1] : PRZ0, bit[2] : VRZ,bit[3] : VDO_ROT1
    me_mout.to_prz0 = b_HDMI_PATH;
    me_mout.to_vrz0 = b_DISP_PATH;
    me_mout.to_prz1 = 0;
    
    me_mout.bCamIn = b_camera_in;


    if( b_HDMI_PATH == 1 )
    {
        /***PRZ0***//*Rescale*/
         //RESZ_I
         me_prz0.src_img_size.w = me_rdma0.src_img_roi.w;
         me_prz0.src_img_size.h = me_rdma0.src_img_roi.h;

         me_prz0.src_img_roi  = MdpRect( me_prz0.src_img_size );

         me_prz0.dst_img_size = MdpSize( p_parameter->hdmi_img_roi.w, p_parameter->hdmi_img_roi.h );
         
        
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
         me_prz0.to_vdo_rot0 = 0;
         me_prz0.to_rgb_rot0 = 1;
         me_prz0.to_vrz0= 0;


         
         /***RGBROT0***/
         //ROTDMA_I
         me_rgb0.dst_img_yuv_addr[0] = p_parameter->hdmi_yuv_img_addr; 
         
         me_rgb0.src_img_roi = MdpRect( me_prz0.dst_img_size );
         
         me_rgb0.dst_img_size = p_parameter->hdmi_img_size;  
         
         me_rgb0.dst_color_format = p_parameter->hdmi_color_format;
         
         me_rgb0.uOutBufferCnt = 1;
         me_rgb0.bContinuous = b_continuous;// 1:Continuous mode does not allow time sharing.
         me_rgb0.bFlip      = ( p_parameter->hdmi_rotate_angle >> 2) & 0x1;
         me_rgb0.bRotate    = ( p_parameter->hdmi_rotate_angle & 0x3 );//0:0, 1:90, 2:180, 3:270
         me_rgb0.bDithering = 0;
         me_rgb0.bCamIn = b_camera_in;// 1: real time path, ex : camera input
         me_rgb0.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable
         
         //RGBROT0
         me_rgb0.src_sel = 0;//0-PRZ0_MOUT ,1-IPP_MOUT

         
    }



    if( b_DISP_PATH == 1 )
    {
        /***VRZ0***//*Rescale*/
        //RESZ_I
        me_vrz0.src_img_size.w = me_rdma0.src_img_roi.w;
        me_vrz0.src_img_size.h = me_rdma0.src_img_roi.h;
        
        me_vrz0.dst_img_size.w = p_parameter->disp_img_roi.w;
        me_vrz0.dst_img_size.h = p_parameter->disp_img_roi.h;
        
        me_vrz0.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
        me_vrz0.bCamIn = b_camera_in;
        
        //VRZ0
        /*N/A*/
        me_vrz0.src_sel = 0;// 0-MOUT, 1-BRZ_MOUT, 2-IPP_MOUT, 3-OVL_DMA_MIMO, 4-PRZ0_MOUT


        /***VDOROT1***/
        //ROTDMA_I
        me_vdo1.dst_img_yuv_addr[0] = p_parameter->disp_yuv_img_addr;

        me_vdo1.src_img_roi = MdpRect( me_vrz0.dst_img_size );
        
        me_vdo1.dst_img_size = p_parameter->disp_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
        me_vdo1.dst_color_format = p_parameter->disp_color_format;/*YUY2_Pack*/
        
        me_vdo1.uOutBufferCnt = 1;
        me_vdo1.bContinuous = b_continuous;// 1:Continuous mode does not allow time sharing.
        me_vdo1.bFlip = (p_parameter->disp_rotate_angle & 0x4) >> 2;
        me_vdo1.bRotate = p_parameter->disp_rotate_angle& 0x3;//0:0, 1:90, 2:180, 3:270
        me_vdo1.bDithering = 0;
        me_vdo1.bCamIn = b_camera_in;// 1: real time path, ex : camera input
        me_vdo1.bEnHWTriggerRDMA0 = 0;//p_parameter->b_hw_trigger_in;// 0:Disable , 1:Enable    

        //VDOROT1
        me_vdo1.src_sel = 1;// 0-PRZ1, 1-VRZ0
    }
    
    

    return 0;
    
}


void Printout_MdpPathDisplayFromMemoryHdmiParameter( MdpPathDisplayFromMemoryHdmiParameter* p_param )
{

    /*Source*/
    MDP_INFO("src_color_format = %d\n", (int)p_param->src_color_format);
    MDP_INFO("src_img_size = %lu , %lu\n", p_param->src_img_size.w, p_param->src_img_size.h );
    MDP_INFO("src_img_roi = %lu , %lu , %lu , %lu\n", p_param->src_img_roi.x, p_param->src_img_roi.y, p_param->src_img_roi.w, p_param->src_img_roi.h);
    MDP_INFO("src_yuv_img_addr.y = 0x%08X\n", (unsigned int)p_param->src_yuv_img_addr.y );
    MDP_INFO("\n");

    /*For HDMI */
    MDP_INFO("en_hdmi_port = %d\n", p_param->en_hdmi_port);    //enable dst port0
    MDP_INFO("hdmi_color_format = %d\n", (int)p_param->hdmi_color_format );
    MDP_INFO("hdmi_img_size = %lu , %lu\n", p_param->hdmi_img_size.w, p_param->hdmi_img_size.h );       /*Currently, "dst roi"  should be equal to "dst size"*/
    MDP_INFO("hdmi_img_roi = %lu , %lu , %lu , %lu\n", p_param->hdmi_img_roi.x, p_param->hdmi_img_roi.y, p_param->hdmi_img_roi.w, p_param->hdmi_img_roi.h );     /*dst_img_roi is not used*/
    MDP_INFO("hdmi_yuv_img_addr.y = 0x%08X\n", (unsigned int)p_param->hdmi_yuv_img_addr.y );
    MDP_INFO("hdmi_rotate_angle = %d\n", (int)p_param->hdmi_rotate_angle );
    MDP_INFO("\n");


    /*For Disp */
    MDP_INFO("en_disp_port1 = %d\n", p_param->en_disp_port);    //enable dst port1
    MDP_INFO("disp_color_format = %d\n", (int)p_param->disp_color_format );
    MDP_INFO("disp_img_size = %lu , %lu\n", p_param->disp_img_size.w, p_param->disp_img_size.h );       /*Currently, "dst roi"  should be equal to "dst size"*/
    MDP_INFO("disp_img_roi = %lu , %lu , %lu , %lu\n", p_param->disp_img_roi.x, p_param->disp_img_roi.y, p_param->disp_img_roi.w, p_param->disp_img_roi.h );     /*dst_img_roi is not used*/
    MDP_INFO("disp_yuv_img_addr.y = 0x%08X\n", (unsigned int)p_param->disp_yuv_img_addr.y );
    MDP_INFO("disp_rotate_angle = %d\n", (int)p_param->disp_rotate_angle );
    MDP_INFO("\n");


    



    
}



