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
  MdpPathCameraPreviewZeroShutter
  /////////////////////////////////////////////////////////////////////////////*/
int MdpPathCameraPreviewZeroShutter::Config( struct MdpPathCameraPreviewZeroShutterParameter* p_parameter )
{
    int b_port0 = p_parameter->en_dst_port0;
    int b_port1 = p_parameter->en_dst_port1;
    int b_port2 = p_parameter->en_dst_port2;


    b_zsd_dst_port0 = p_parameter->en_dst_port0;
    b_zsd_dst_port1 = p_parameter->en_dst_port1;
    b_zsd_dst_port2 = p_parameter->en_dst_port2;

    /*-----------------------------------------------------------------------------
      Manage element resource
      -----------------------------------------------------------------------------*/
    if( ( b_port0 == 0 ) && ( b_port1 == 0 ) && ( b_port2 == 0 ) )
    {
        m_mdp_element_count = 0;
        return -1;
    } 
    
    m_mdp_element_count = 0;
        
    //Pseudo Source Enable: Use RDMA0 to take place of camera sensor
    if( p_parameter->pseudo_src_enable )
    {
        /*Resource List*/
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_rdma0;

        b_camera_in = 0; 
        b_continuous = 0; 
        
    } else
    {
        b_camera_in = 1;   
        b_continuous = 1;  
    }

    
    m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_crz;
    m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_ovl;

    if( b_port0 || b_port2 )
    {
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_ipp;
        
        if( b_port0 )
        {
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_prz0;
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vdorot0;
        }

        if( b_port2 )
        {
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vrz0;
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_rgbrot1;
        }
        
    }

    if( b_port1 )
    {
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_mout;
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_prz1;
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vdorot1;

        /*VDO_ROT1 is regard as encode port output in zsd path*/
        me_vdorot1.b_is_zero_shutter_encode_port_ = 1;
    }



    /*DEBUG:If enable, force to be single frame*/
    if( p_parameter->debug_preview_single_frame_enable )
    {
        b_continuous = 0;
    }



    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Setup mdp elements parameter
      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    //Pseudo Source Enable: Use RDMA0 to take place of camera sensor
    if( p_parameter->pseudo_src_enable )
    {
        /*m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_rdma0;*/
        /*.............................................................................*/
        /***RDMA0***//*Crop*/
        /*.............................................................................*/
        //RDMA_I
        me_rdma0.uInBufferCnt = 1;
        me_rdma0.src_img_yuv_addr[0] = p_parameter->pseudo_src_yuv_img_addr;
        me_rdma0.src_img_size = p_parameter->src_img_size;
        me_rdma0.src_img_roi =  p_parameter->src_img_roi;
        me_rdma0.src_color_format = p_parameter->pseudo_src_color_format;

        me_rdma0.bHWTrigger = 0;
        me_rdma0.bContinuous = b_continuous;
        me_rdma0.bCamIn = b_camera_in;
        
        me_rdma0.bEISEn = 0;
        me_rdma0.u4EISCON = 0;//EIS setting
        //RDMA0
        me_rdma0.to_cam = 1;
        me_rdma0.to_ovl = 0;
        me_rdma0.to_mout = 0;

        //HW Trigger
        me_rdma0.trigger_src = 0;

    }

    
    /*m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_crz;*/
    /*.............................................................................*/
    /***CRZ***//*Crop and DownScale*/
    /*.............................................................................*/
    //RESZ_I
    me_crz.src_img_size = p_parameter->src_img_size;

    me_crz.src_img_roi  = p_parameter->src_img_roi;
#if 0
    me_crz.dst_img_size.w = p_parameter->src_img_roi.w;
    me_crz.dst_img_size.h = p_parameter->src_img_roi.h;
#else
    me_crz.dst_img_size.w = p_parameter->dst1_img_size.w;
    me_crz.dst_img_size.h = p_parameter->dst1_img_size.h;
#endif

    me_crz.uUpScaleCoeff = p_parameter->resz_coeff.crz_up_scale_coeff;
    me_crz.uDnScaleCoeff = p_parameter->resz_coeff.crz_dn_scale_coeff;

    /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
    me_crz.uEEHCoeff = 0;
    me_crz.uEEVCoeff = 0;

    me_crz.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
    me_crz.bCamIn = b_camera_in;
    me_crz.bBypass = 0;

    //CRZ
    if( p_parameter->pseudo_src_enable )
    {
        me_crz.src_sel = 0;// 0:RDMA0 , 1:BRZ, 2:Camera
    } else
    {
        me_crz.src_sel = 2;// 0:RDMA0 , 1:BRZ, 2:Camera
    }

    
    /*m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_ovl;*/
    /*.............................................................................*/
    /***OVL***/
    /*.............................................................................*/
    //OVL
    me_ovl.bMaskEn = 0;
    me_ovl.bIsCompressMask = 0;
    me_ovl.bContinuous = b_continuous;
    me_ovl.bCamIn = b_camera_in;
    me_ovl.uMaskCnt = 0;
    me_ovl.bSource = 1;// 0-RDMA0 ,1-CRZ
    
    me_ovl.bToIPP = ( b_port0 || b_port2 ) ? 1 : 0;
    me_ovl.bToMOUT = b_port1? 1 : 0;
    me_ovl.bToVRZ = 0;

    me_ovl.bOverlayIPP = 0;//0 : no overlay, 1:overlay , 2:single color overlay
    me_ovl.bOverlayMOUT = 0;//0 : no overlay, 1:overlay , 2:single color overlay
    me_ovl.bOverlayVRZ = 0;//0 : no overlay, 1:overlay , 2:single color overlay

    me_ovl.bBypass = 1;

    

    

    if( b_port0 || b_port2 )
    {
        
        /*m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_ipp;*/
        /*.............................................................................*/
        /***IPP***/
        /*.............................................................................*/
        //IPP
        /*enable flag*/
        me_ipp.bEnRGBReplace = 0;
        me_ipp.bEnColorInverse = 0;
        me_ipp.bEnColorAdj = 0;
        me_ipp.bEnColorize = 0;
        me_ipp.bEnSatAdj = 0;
        me_ipp.bEnHueAdj = 0;
        me_ipp.bEnContractBrightAdj = 0;

        /*output sel*/
        me_ipp.to_jpg_dma = 0;
        me_ipp.to_vdo_rot0 = 0;
        me_ipp.to_prz0 = b_port0;
        me_ipp.to_vrz0 = b_port2;
        me_ipp.to_rgb_rot0 = 0;

        /*input sel*/
        me_ipp.src_sel = 0;//0 : OVL, 1:CRZ

        me_ipp.bCamIn = b_camera_in;
        me_ipp.bBypass = 0;


            
        
        if( b_port0 )
        {
            /*m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_prz0;*/
            /*.............................................................................*/
            /***PRZ0***//*Rescale*/
            /*.............................................................................*/
            //RESZ_I
            #if 0
            me_prz0.src_img_size = me_crz.dst_img_size;
            me_prz0.src_img_roi = p_parameter->dst0_img_roi;
            me_prz0.dst_img_size = p_parameter->dst0_img_size;
            #else
            me_prz0.src_img_size = me_crz.dst_img_size;
            me_prz0.src_img_roi = MdpRect( 0, 0, me_crz.dst_img_size.w, me_crz.dst_img_size.h);
            me_prz0.dst_img_size = p_parameter->dst0_img_size;
            #endif

            me_prz0.uUpScaleCoeff = p_parameter->resz_coeff.prz0_up_scale_coeff;
            me_prz0.uDnScaleCoeff = p_parameter->resz_coeff.prz0_dn_scale_coeff;;

            /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
            me_prz0.uEEHCoeff = p_parameter->resz_coeff.prz0_ee_h_str;
            me_prz0.uEEVCoeff = p_parameter->resz_coeff.prz0_ee_v_str;

            me_prz0.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
            me_prz0.bCamIn = b_camera_in;
            me_prz0.bBypass = 0;

            //PRZ0
            me_prz0.src_sel = 1;//0-MOUT,1-IPP_MOUT,2-CAM,3-BRZ_MOUT

            /*output sel*/
            me_prz0.to_vdo_rot0 = 1;
            me_prz0.to_rgb_rot0 = 0;
            me_prz0.to_vrz0= 0;

    
    
            /*m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vdorot0;*/
            /*.............................................................................*/
            /***VDOROT0***/
            /*.............................................................................*/
            //ROTDMA_I
            me_vdorot0.dst_img_yuv_addr[0] = p_parameter->dst0_yuv_img_addr;

            me_vdorot0.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
            me_vdorot0.src_img_roi.y = 0;
            me_vdorot0.src_img_roi.w = me_prz0.dst_img_size.w;
            me_vdorot0.src_img_roi.h = me_prz0.dst_img_size.h;

            me_vdorot0.dst_img_size = me_prz0.dst_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
            me_vdorot0.dst_color_format = p_parameter->dst0_color_format;

            me_vdorot0.uOutBufferCnt = p_parameter->dst0_buffer_count;
            me_vdorot0.bContinuous = b_continuous;// 1:Continuous mode does not allow time sharing.
            me_vdorot0.bFlip = p_parameter->dst0_rotate_angle >> 2;
            me_vdorot0.bRotate = (0x3 & p_parameter->dst0_rotate_angle);//0:0, 1:90, 2:180, 3:270
            me_vdorot0.bDithering = 0;
            me_vdorot0.bCamIn = b_camera_in;// 1: real time path, ex : camera input
            me_vdorot0.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

            //VDOROT0
            me_vdorot0.src_sel= 0;// 0-PRZ0_MOUT, 1-IPP_MOUT
            
            
        }

        if( b_port2 )
        {
            /*m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vrz0;*/
            /*.............................................................................*/
            /***VRZ0***/
            /*.............................................................................*/
            //VRZ_I
            me_vrz0.src_img_size  = me_crz.dst_img_size;
            me_vrz0.dst_img_size = p_parameter->dst2_img_size;

            me_vrz0.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
            me_vrz0.bCamIn = b_camera_in;

            //VRZ0
            me_vrz0.src_sel = 2;// 0-MOUT, 1-BRZ_MOUT, 2-IPP_MOUT, 3-OVL_DMA_MIMO, 4-PRZ0_MOUT

            /*m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_rgbrot1;*/
            /*.............................................................................*/
            /***RGBROT1***/
            /*.............................................................................*/
            //ROTDMA_I
            me_rgbrot1.dst_img_yuv_addr[0] = p_parameter->dst2_yuv_img_addr;

            me_rgbrot1.src_img_roi.x = 0;
            me_rgbrot1.src_img_roi.y = 0;
            me_rgbrot1.src_img_roi.w = me_vrz0.dst_img_size.w;
            me_rgbrot1.src_img_roi.h = me_vrz0.dst_img_size.h;

            me_rgbrot1.dst_img_size = me_vrz0.dst_img_size;
            me_rgbrot1.dst_color_format = p_parameter->dst2_color_format;
            me_rgbrot1.uOutBufferCnt = p_parameter->dst2_buffer_count;
            me_rgbrot1.bContinuous = b_continuous;// 1:Continuous mode does not allow time sharing.
            me_rgbrot1.bFlip = p_parameter->dst2_rotate_angle >> 2;;
            me_rgbrot1.bRotate = (0x3 & p_parameter->dst2_rotate_angle);//0:0, 1:90, 2:180, 3:270
            me_rgbrot1.bDithering = 0;
            me_rgbrot1.bCamIn = b_camera_in;// 1: real time path, ex : camera input
            me_rgbrot1.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

            //RGBROT1
            /*Nothing*/

            
            
        }
        
    }

    if( b_port1 )
    {
        /*m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_mout;*/
        /*.............................................................................*/
        /***MOUT***/
        /*.............................................................................*/
        me_mout.src_sel = 1;      // 0-R_DMA0_MOUT, 1-OVL_DMA_MIMO
        
        me_mout.to_jpg_dma = 0;   //bit[0] : JPEG_DMA, bit[1] : PRZ0, bit[2] : VRZ,bit[3] : VDO_ROT1
        me_mout.to_prz0 = 0;
        me_mout.to_vrz0 = 0;
        me_mout.to_prz1 = 1;
        
        me_mout.bCamIn = b_camera_in;


        
        
        /*m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_prz1;*/
        /*.............................................................................*/
        /***PRZ1***//*Rescale*/
        /*.............................................................................*/
        //RESZ_I
        #if 0
        me_prz1.src_img_size = me_crz.dst_img_size;;
        me_prz1.src_img_roi = p_parameter->dst1_img_roi;
        me_prz1.dst_img_size = p_parameter->dst1_img_size;
        #else
        me_prz1.src_img_size = me_crz.dst_img_size;
        me_prz1.src_img_roi = MdpRect( 0, 0, me_crz.dst_img_size.w, me_crz.dst_img_size.h );
        me_prz1.dst_img_size = p_parameter->dst1_img_size;
        #endif
        
        me_prz1.uUpScaleCoeff = p_parameter->resz_coeff.prz1_up_scale_coeff;
        me_prz1.uDnScaleCoeff = p_parameter->resz_coeff.prz1_dn_scale_coeff;

        /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
        me_prz1.uEEHCoeff = p_parameter->resz_coeff.prz1_ee_h_str;
        me_prz1.uEEVCoeff = p_parameter->resz_coeff.prz1_ee_v_str;

        me_prz1.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
        me_prz1.bCamIn = b_camera_in;

        /*Bypass for full size*/
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
                     /*JUST bypass it! for 8M zero shutter delay full frame */
                me_prz1.bBypass= 1;
                
             }
         }
         if( me_prz1.bBypass == 0 )
         {
            MDP_ERROR("PRZ1 should be bypass in ZSD! src( %lu, %lu) src_roi( %lu, %lu, %lu, %lu ) dst( %lu, %lu )\n",
                        me_prz1.src_img_size.w, me_prz1.src_img_size.h,
                        me_prz1.src_img_roi.x, me_prz1.src_img_roi.y, me_prz1.src_img_roi.w, me_prz1.src_img_roi.h,
                        me_prz1.dst_img_size.w, me_prz1.dst_img_size.h );
            return -1;
         }

        //PRZ1
        /*N/A*/

    
        /*m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vdorot1;*/
        /*.............................................................................*/
        /***VDOROT1***/
        /*.............................................................................*/
        //ROTDMA_I
        me_vdorot1.dst_img_yuv_addr[0] = p_parameter->dst1_yuv_img_addr;
        
        me_vdorot1.src_img_roi.x = 0;    /*For ROTDMA, src img rect =src roi rect, we don't use roi functionality*/
        me_vdorot1.src_img_roi.y = 0;
        me_vdorot1.src_img_roi.w = me_prz1.dst_img_size.w;
        me_vdorot1.src_img_roi.h = me_prz1.dst_img_size.h;
        
        me_vdorot1.dst_img_size = me_prz1.dst_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
        me_vdorot1.dst_color_format = p_parameter->dst1_color_format;
        
        me_vdorot1.uOutBufferCnt = p_parameter->dst1_buffer_count;
        me_vdorot1.bContinuous = b_continuous;// 1:Continuous mode does not allow time sharing.
        me_vdorot1.bFlip = p_parameter->dst1_rotate_angle >> 2;
        me_vdorot1.bRotate = (0x3 & p_parameter->dst1_rotate_angle);//0:0, 1:90, 2:180, 3:270
        me_vdorot1.bDithering = 0;
        me_vdorot1.bCamIn = b_camera_in;// 1: real time path, ex : camera input
        me_vdorot1.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

        //VDOROT1
        me_vdorot1.src_sel = 0;// 0-PRZ1, 1-VRZ0
    
    }

    return 0;


}





int MdpPathCameraPreviewZeroShutter::ConfigZoom( MdpRect crop_size, MdpRect* p_real_crop_size )
{

    MdpDrvConfigZSDZoom_Param stParam;

    //Prepare zoom setting for CRZ,PRZ0,VRZ,VDOROT1

    ROTDMA_I*       pOutRot = &me_vdorot1;  /*vdo1 output the encode frame*/
    MdpRect         crop_size_altered;
    MdpDrvColorInfo_t   ci;
    MdpColorFormat  dst_color;
    MdpRect         src_rect;
    MdpRect         dst_rect;

    crop_size_altered = crop_size;
    dst_color   = pOutRot->dst_color_format;
    
    MdpDrvColorFormatInfoGet( dst_color, &ci );
    
    /* Adjust crop_size*/
    {
        /*Check Alignment*/
        if( ci.y_stride_align != 0 ){
            MDP_ROUND_UP( crop_size_altered.w, ci.y_stride_align );
        } else
        {
            MDP_ROUND_UP( crop_size_altered.w, 1 );//Round uv stride to 2x align
        }

        /*return*/
        if( p_real_crop_size != NULL )
        {
            *p_real_crop_size = crop_size_altered;
        }
    }

    if (p_real_crop_size->w < me_crz.dst_img_size.w ||
        p_real_crop_size->h < me_crz.dst_img_size.h)
    {
        src_rect = *p_real_crop_size;
        dst_rect = *p_real_crop_size;
    }
    else
    {
        src_rect = *p_real_crop_size;
        dst_rect = me_crz.dst_img_size;
    }
    MDP_INFO_PERSIST("[ZSD] zoom real src(%d, %d, %d, %d) dst (%d, %d, %d, %d)\n",
        (int)src_rect.x, (int)src_rect.y, (int)src_rect.w, (int)src_rect.h,
        (int)dst_rect.x, (int)dst_rect.y, (int)dst_rect.w, (int)dst_rect.h)
#if 1

    //CRZ
    if (b_zsd_dst_port0 ||
        b_zsd_dst_port1 ||
        b_zsd_dst_port2)
    {
        stParam.u4CRZLBMAX = me_crz.CalcLBMAX(  MdpSize( src_rect.w, src_rect.h ),
                                                MdpSize( dst_rect.w, dst_rect.h ) );
        stParam.u4CRZSrcSZ = (src_rect.w | (src_rect.h << 16));
        stParam.u4CRZTarSZ = (dst_rect.w | (dst_rect.h << 16));
        stParam.u4CRZHRatio = me_crz.CalcHRATIO( src_rect.w, dst_rect.w );
        stParam.u4CRZVRatio = me_crz.CalcVRATIO( src_rect.h, dst_rect.h );
        stParam.u4CRZCropLR = (0x1 << 31 | (crop_size.x << 16) | (crop_size.x + src_rect.w - 1));
        stParam.u4CRZCropTB = ((crop_size.y << 16) | (crop_size.y + src_rect.h - 1));
    }

    //PRZ0
    if (b_zsd_dst_port0)
    {
        MdpSize dst_size;
        dst_size = me_prz0.dst_img_size;

        stParam.u4PRZ0LBMAX = me_prz0.CalcLBMAX(  MdpSize( dst_rect.w, dst_rect.h ),
                                                      MdpSize( dst_size.w, dst_size.h ) );
        stParam.u4PRZ0SrcSZ = (dst_rect.w | (dst_rect.h << 16));
        stParam.u4PRZ0TarSZ = (dst_size.w | (dst_size.h << 16));
        stParam.u4PRZ0HRatio = me_prz0.CalcHRATIO( dst_rect.w, dst_size.w );
        stParam.u4PRZ0VRatio = me_prz0.CalcVRATIO( dst_rect.h, dst_size.h );
    }

    //VRZ
    if (b_zsd_dst_port2)
    {
        MdpSize src_size = MdpSize( dst_rect.w , dst_rect.h);
        MdpSize dst_size = me_vrz0.dst_img_size;

        stParam.u4VRZSrcSZ = ((src_size.h << 16) | src_size.w);
        stParam.u4VRZTarSZ = ((dst_size.h << 16) | dst_size.w);
        stParam.u4VRZHRatio = (dst_size.w < src_size.w ?
                                            (((unsigned long)dst_size.w - 1) << 20)/(src_size.w - 1) :
                                            (((unsigned long)src_size.w) << 20)/dst_size.w);
        stParam.u4VRZVRatio = (dst_size.h < src_size.h ?
                                                (((unsigned long)dst_size.h - 1) << 20)/(src_size.h - 1) :
                                                (((unsigned long)src_size.h) << 20)/dst_size.h);
        stParam.u4VRZHRes = (src_size.w%dst_size.w);
        stParam.u4VRZVRes = (src_size.h%dst_size.h);
    }

    //VDOROT1
    if (b_zsd_dst_port1)
    //We don't care about rotation case because insufficient sysram for this port
    {
        stParam.u4VDO1Seg4 = stParam.u4CRZTarSZ;//SrcW+SrcH
        stParam.u4VDO1Seg5 = stParam.u4VDO1Seg4;//ClipW+ClipH
        stParam.u4VDO1Seg6 = 0;//ClipX+ClipY
#if 1
        zsd_reg_vdo1_.desc_src_img_roi.x = 0;
        zsd_reg_vdo1_.desc_src_img_roi.y = 0;
        zsd_reg_vdo1_.desc_src_img_roi.w = dst_rect.w;
        zsd_reg_vdo1_.desc_src_img_roi.h = dst_rect.h;

        //Size before rotate (image stride)
        zsd_reg_vdo1_.desc_dst_img_size = MdpSize( dst_rect.w, dst_rect.h );

        /*-----------------------------------------------------------------------------
            Frame Start Address Calculation/Y & UV stride calculation
        -----------------------------------------------------------------------------*/
        {
            ROTDMA_I::CalcFrameStartAddress_In   param_in;
            ROTDMA_I::CalcFrameStartAddress_Out  param_out;

            param_in.rotate = pOutRot->bRotate;                             /*rotation w/o flip. 0-3*/
            param_in.src_img_roi  = zsd_reg_vdo1_.desc_src_img_roi;        /*src roi image before rotation*/
            param_in.dst_img_size = zsd_reg_vdo1_.desc_dst_img_size;       /*stride before rotation*/
            param_in.dst_color_format = pOutRot->dst_color_format;
            param_in.b_is_generic_yuv = pOutRot->desc_is_generic_yuv_;
            param_in.byte_per_pixel = pOutRot->desc_byte_per_pixel_;
            /*y,u,v sampling period*/
            param_in.yv  = pOutRot->desc_yv_;
            param_in.yh  = pOutRot->desc_yh_;
            param_in.uvv = pOutRot->desc_uvv_;
            param_in.uvh = pOutRot->desc_uvh_;
            param_in.y_stride_align = pOutRot->desc_y_stride_align_;
            param_in.uv_stride_align = pOutRot->desc_uv_stride_align_;

            pOutRot->CalcFrameStartAddress( &param_in, &param_out );

            zsd_reg_vdo1_.desc_y_frame_start_in_byte    = param_out.y_frame_start_in_byte;
            zsd_reg_vdo1_.desc_uv_frame_start_in_byte   = param_out.uv_frame_start_in_byte;
            zsd_reg_vdo1_.desc_y_stride                 = param_out.y_stride;
            zsd_reg_vdo1_.desc_uv_stride                = param_out.uv_stride;
        }
#endif
        //DstW in Bytes
        if( ci.uv_stride_align != 0 ){
            stParam.u4VDO1Seg7 = ( ( zsd_reg_vdo1_.desc_y_stride * pOutRot->desc_byte_per_pixel_ ) & 0x0003FFFF ) |
                                                    ( ( zsd_reg_vdo1_.desc_uv_stride * pOutRot->desc_byte_per_pixel_ ) << 18 );

        }else  {
            stParam.u4VDO1Seg7 = ( zsd_reg_vdo1_.desc_y_stride * pOutRot->desc_byte_per_pixel_ ) & 0x0003FFFF ;
        }

    }

    //IOCTL
    if (b_zsd_dst_port1) {
        if(0 == MdpDrv_ConfigZSDZoom(&stParam))
        {
            //Update descriptor queue data
            //got to protect ROTDMA_I object if multiple threads.
            me_vdorot1.ConfigZsdZoom_DescUpdate( zsd_reg_vdo1_ );
        }
    }



#else
    //CRZ
    if (b_zsd_dst_port0 ||
        b_zsd_dst_port1 ||
        b_zsd_dst_port2)
    {
        stParam.u4CRZLBMAX = me_crz.CalcLBMAX(  MdpSize( crop_size_altered.w, crop_size_altered.h ), 
                                                MdpSize( crop_size_altered.w, crop_size_altered.h ) );
        stParam.u4CRZSrcSZ = (crop_size_altered.w | (crop_size_altered.h << 16));
        stParam.u4CRZTarSZ = stParam.u4CRZSrcSZ;
        stParam.u4CRZHRatio = (1 << 20);//me_crz.CalcHRATIO( crop_size_altered.w, crop_size_altered.w );
        stParam.u4CRZVRatio = (1 << 20);//me_crz.CalcVRATIO( crop_size_altered.h, crop_size_altered.h );
        stParam.u4CRZCropLR = (0x1 << 31 | (crop_size.x << 16) | (crop_size.x + crop_size_altered.w - 1));
        stParam.u4CRZCropTB = ((crop_size.y << 16) | (crop_size.y + crop_size_altered.h - 1));
    }

    //PRZ0
    if (b_zsd_dst_port0)
    {
        MdpSize dst_size;
        dst_size = me_prz0.dst_img_size;

        stParam.u4PRZ0LBMAX = me_prz0.CalcLBMAX(  MdpSize( crop_size_altered.w, crop_size_altered.h ), 
                                                      MdpSize( dst_size.w, dst_size.h ) );
        stParam.u4PRZ0SrcSZ = stParam.u4CRZTarSZ;//(crop_size_altered.w | (crop_size_altered.h << 16));
        stParam.u4PRZ0TarSZ = (dst_size.w | (dst_size.h << 16));
        stParam.u4PRZ0HRatio = me_prz0.CalcHRATIO( crop_size_altered.w, dst_size.w );
        stParam.u4PRZ0VRatio = me_prz0.CalcVRATIO( crop_size_altered.h, dst_size.h );
    }

    //VRZ
    if (b_zsd_dst_port2)
    {
        MdpSize src_size = MdpSize( crop_size_altered.w , crop_size_altered.h);
        MdpSize dst_size = me_vrz0.dst_img_size;
        
        stParam.u4VRZSrcSZ = ((src_size.h << 16) | src_size.w);
        stParam.u4VRZTarSZ = ((dst_size.h << 16) | dst_size.w);
        stParam.u4VRZHRatio = (dst_size.w < src_size.w ? 
                                            (((unsigned long)dst_size.w - 1) << 20)/(src_size.w - 1) :
                                            (((unsigned long)src_size.w) << 20)/dst_size.w);
        stParam.u4VRZVRatio = (dst_size.h < src_size.h ? 
                                                (((unsigned long)dst_size.h - 1) << 20)/(src_size.h - 1) :
                                                (((unsigned long)src_size.h) << 20)/dst_size.h);
        stParam.u4VRZHRes = (src_size.w%dst_size.w);
        stParam.u4VRZVRes = (src_size.h%dst_size.h);
    }

    //VDOROT1
    if (b_zsd_dst_port1)
    //We don't care about rotation case because insufficient sysram for this port
    {
        stParam.u4VDO1Seg4 = stParam.u4CRZTarSZ;//SrcW+SrcH
        stParam.u4VDO1Seg5 = stParam.u4VDO1Seg4;//ClipW+ClipH
        stParam.u4VDO1Seg6 = 0;//ClipX+ClipY
#if 1
        zsd_reg_vdo1_.desc_src_img_roi.x = 0;     
        zsd_reg_vdo1_.desc_src_img_roi.y = 0;     
        zsd_reg_vdo1_.desc_src_img_roi.w = crop_size_altered.w;     
        zsd_reg_vdo1_.desc_src_img_roi.h = crop_size_altered.h;     
        
        //Size before rotate (image stride)
        zsd_reg_vdo1_.desc_dst_img_size = MdpSize( crop_size_altered.w, crop_size_altered.h );

        /*-----------------------------------------------------------------------------
            Frame Start Address Calculation/Y & UV stride calculation
        -----------------------------------------------------------------------------*/
        {
            ROTDMA_I::CalcFrameStartAddress_In   param_in;
            ROTDMA_I::CalcFrameStartAddress_Out  param_out;

            param_in.rotate = pOutRot->bRotate;                             /*rotation w/o flip. 0-3*/
            param_in.src_img_roi  = zsd_reg_vdo1_.desc_src_img_roi;        /*src roi image before rotation*/
            param_in.dst_img_size = zsd_reg_vdo1_.desc_dst_img_size;       /*stride before rotation*/
            param_in.dst_color_format = pOutRot->dst_color_format;
            param_in.b_is_generic_yuv = pOutRot->desc_is_generic_yuv_;
            param_in.byte_per_pixel = pOutRot->desc_byte_per_pixel_;
            /*y,u,v sampling period*/
            param_in.yv  = pOutRot->desc_yv_;
            param_in.yh  = pOutRot->desc_yh_;
            param_in.uvv = pOutRot->desc_uvv_;
            param_in.uvh = pOutRot->desc_uvh_;
            param_in.y_stride_align = pOutRot->desc_y_stride_align_;
            param_in.uv_stride_align = pOutRot->desc_uv_stride_align_;

            pOutRot->CalcFrameStartAddress( &param_in, &param_out );

            zsd_reg_vdo1_.desc_y_frame_start_in_byte    = param_out.y_frame_start_in_byte;
            zsd_reg_vdo1_.desc_uv_frame_start_in_byte   = param_out.uv_frame_start_in_byte;
            zsd_reg_vdo1_.desc_y_stride                 = param_out.y_stride;
            zsd_reg_vdo1_.desc_uv_stride                = param_out.uv_stride;
        }
#endif
        //DstW in Bytes
        if( ci.uv_stride_align != 0 ){
            stParam.u4VDO1Seg7 = ( ( zsd_reg_vdo1_.desc_y_stride * pOutRot->desc_byte_per_pixel_ ) & 0x0003FFFF ) | 
                                                    ( ( zsd_reg_vdo1_.desc_uv_stride * pOutRot->desc_byte_per_pixel_ ) << 18 ); 
            
        }else  {
            stParam.u4VDO1Seg7 = ( zsd_reg_vdo1_.desc_y_stride * pOutRot->desc_byte_per_pixel_ ) & 0x0003FFFF ;
        }
        
    }

    //IOCTL
    if (b_zsd_dst_port1) {
        if(0 == MdpDrv_ConfigZSDZoom(&stParam))
        {
            //Update descriptor queue data
            //got to protect ROTDMA_I object if multiple threads.
            me_vdorot1.ConfigZsdZoom_DescUpdate( zsd_reg_vdo1_ );
        }
    }

#if 0
       //return me_crz.ConfigZoom( crop_size , MID_CRZ );
    
    /*TODO: Calculate register value and store into variable.
            wait till enqueue is execute, the register values are been set
    */
    ROTDMA_I*       pOutRot = &me_vdorot1;  /*vdo1 output the encode frame*/
    MdpRect         crop_size_altered;
    MdpSize         crop_size_after_rotate; 
    unsigned        rotate;
    MdpColorFormat  dst_color;

    /*Set Dirty bit*/
    b_zsd_zoom_regdata_dirty_ = 1;
    
    crop_size_altered = crop_size;
    rotate      = pOutRot->bRotate;   
    dst_color   = pOutRot->dst_color_format;
    
    
    /* Adjust crop_size*/
    {
        /*Check rotate*/
        if( rotate & 0x1 )  {
            crop_size_after_rotate.w = crop_size.h;
            crop_size_after_rotate.h = crop_size.w;
            
        } else              {
            crop_size_after_rotate.w = crop_size.w;
            crop_size_after_rotate.h = crop_size.h;
        }

        /*Check Alignment*/
        if( dst_color == ANDROID_YV12 )        {
            MDP_ROUND_UP( crop_size_after_rotate.w, 4 );//Round uv stride to 16x align
        } else        {
            MDP_ROUND_UP( crop_size_after_rotate.w, 1 );//Round uv stride to 2x align
        }
        
        /*Feed back*/
        if( rotate & 0x1 )  {
            crop_size_altered.h = crop_size_after_rotate.w;
        } else              {
            crop_size_altered.w = crop_size_after_rotate.w;
        }


        /*return*/
        if( p_real_crop_size != NULL )
        {
            *p_real_crop_size = crop_size_altered;
        }
    }
    
    /*CRZ Register*//*1:1*/
    {
        zsd_reg_crz_.LBMAX = me_crz.CalcLBMAX(  MdpSize( crop_size_altered.w, crop_size_altered.h ), 
                                                MdpSize( crop_size_altered.w, crop_size_altered.h ) );
        zsd_reg_crz_.reg_ORIGSZ = 0xFFFFFFFF;/*Ignore*/
        zsd_reg_crz_.reg_SRCSZ  = (crop_size_altered.w | (crop_size_altered.h << 16));
        zsd_reg_crz_.reg_TARSZ  = (crop_size_altered.w | ((unsigned long)crop_size_altered.h << 16));
        zsd_reg_crz_.reg_CROPLR = (0x1 << 31 | (crop_size.x << 16) | (crop_size.x + crop_size_altered.w - 1));
        zsd_reg_crz_.reg_CROPTB = ((crop_size.y << 16) | (crop_size.y + crop_size_altered.h - 1));
        zsd_reg_crz_.reg_HRATIO = me_crz.CalcHRATIO( crop_size_altered.w, crop_size_altered.w );
        zsd_reg_crz_.reg_VRATIO = me_crz.CalcVRATIO( crop_size_altered.h, crop_size_altered.h );
    }

    /*PRZ0 Register*//*Scale*/
    {
        MdpSize dst_size;

        dst_size = me_prz0.dst_img_size;
            
        zsd_reg_prz0_.LBMAX = me_prz0.CalcLBMAX(  MdpSize( crop_size_altered.w, crop_size_altered.h ), 
                                                  MdpSize( dst_size.w, dst_size.h ) );
        zsd_reg_prz0_.reg_ORIGSZ  = (crop_size_altered.w | (crop_size_altered.h << 16));
        zsd_reg_prz0_.reg_SRCSZ  = (crop_size_altered.w | (crop_size_altered.h << 16));
        zsd_reg_prz0_.reg_TARSZ  = (dst_size.w | (dst_size.h << 16));
        zsd_reg_prz0_.reg_CROPLR = (0x1 << 31 | (0 << 16) | (0 + crop_size_altered.w - 1));
        zsd_reg_prz0_.reg_CROPTB = ((0 << 16) | (0 + crop_size_altered.h - 1));
        zsd_reg_prz0_.reg_HRATIO = me_prz0.CalcHRATIO( crop_size_altered.w, dst_size.w );
        zsd_reg_prz0_.reg_VRATIO = me_prz0.CalcVRATIO( crop_size_altered.h, dst_size.h );
        
    }

    /*PRZ1 Register*//*1:1*/
    {
            
        zsd_reg_prz1_.LBMAX = me_prz1.CalcLBMAX(  MdpSize( crop_size_altered.w, crop_size_altered.h ), 
                                                  MdpSize( crop_size_altered.w, crop_size_altered.h ) );
        zsd_reg_prz1_.reg_ORIGSZ = (crop_size_altered.w | (crop_size_altered.h << 16));
        zsd_reg_prz1_.reg_SRCSZ  = (crop_size_altered.w | (crop_size_altered.h << 16));
        zsd_reg_prz1_.reg_TARSZ  = (crop_size_altered.w | (crop_size_altered.h << 16));
        zsd_reg_prz1_.reg_CROPLR = (0x1 << 31 | (0 << 16) | (0 + crop_size_altered.w - 1));
        zsd_reg_prz1_.reg_CROPTB = ((0 << 16) | (0 + crop_size_altered.h - 1));
        zsd_reg_prz1_.reg_HRATIO = me_prz1.CalcHRATIO( crop_size_altered.w, crop_size_altered.w );
        zsd_reg_prz1_.reg_VRATIO = me_prz1.CalcVRATIO( crop_size_altered.h, crop_size_altered.h );
    }

    /*VRZ0 Register*//*Scale*/
    {
        MdpSize src_size = MdpSize( crop_size_altered.w , crop_size_altered.h);
        MdpSize dst_size = me_vrz0.dst_img_size;
        

        zsd_reg_vrz0_.reg_SRCSZ     = ((src_size.h << 16) | src_size.w);
        zsd_reg_vrz0_.reg_TARSZ     = ((dst_size.h << 16) | dst_size.w);
        zsd_reg_vrz0_.reg_HRATIO    = (dst_size.w < src_size.w ? 
                                                (((unsigned long)dst_size.w - 1) << 20)/(src_size.w - 1) :
                                                (((unsigned long)src_size.w) << 20)/dst_size.w);
        zsd_reg_vrz0_.reg_VRATIO    = (dst_size.h < src_size.h ? 
                                                (((unsigned long)dst_size.h - 1) << 20)/(src_size.h - 1) :
                                                (((unsigned long)src_size.h) << 20)/dst_size.h);
        zsd_reg_vrz0_.reg_HRES      = (src_size.w%dst_size.w);
        zsd_reg_vrz0_.reg_VRES      = (src_size.h%dst_size.h);
        
    }

    /*VDO1 Update*/
    if( ResourceIdMaskGet() & MID_VDO_ROT1 )
    {
        //image roi before rotate
        zsd_reg_vdo1_.desc_src_img_roi.x = 0;     
        zsd_reg_vdo1_.desc_src_img_roi.y = 0;     
        zsd_reg_vdo1_.desc_src_img_roi.w = crop_size_altered.w;     
        zsd_reg_vdo1_.desc_src_img_roi.h = crop_size_altered.h;     
        
        //Size before rotate (image stride)
        zsd_reg_vdo1_.desc_dst_img_size = MdpSize( crop_size_altered.w, crop_size_altered.h );

        /*-----------------------------------------------------------------------------
            Frame Start Address Calculation/Y & UV stride calculation
        -----------------------------------------------------------------------------*/
        {
            ROTDMA_I::CalcFrameStartAddress_In   param_in;
            ROTDMA_I::CalcFrameStartAddress_Out  param_out;

            param_in.rotate = pOutRot->bRotate;                             /*rotation w/o flip. 0-3*/
            param_in.src_img_roi  = zsd_reg_vdo1_.desc_src_img_roi;        /*src roi image before rotation*/
            param_in.dst_img_size = zsd_reg_vdo1_.desc_dst_img_size;       /*stride before rotation*/
            param_in.dst_color_format = pOutRot->dst_color_format;
            param_in.b_is_generic_yuv = pOutRot->desc_is_generic_yuv_;
            param_in.byte_per_pixel = pOutRot->desc_byte_per_pixel_;
            /*y,u,v sampling period*/
            param_in.yv  = pOutRot->desc_yv_;
            param_in.yh  = pOutRot->desc_yh_;
            param_in.uvv = pOutRot->desc_uvv_;
            param_in.uvh = pOutRot->desc_uvh_;

            pOutRot->CalcFrameStartAddress( &param_in, &param_out );

            zsd_reg_vdo1_.desc_y_stride                 = param_out.y_stride;
            zsd_reg_vdo1_.desc_uv_stride                = param_out.uv_stride;
            zsd_reg_vdo1_.desc_y_frame_start_in_byte    = param_out.y_frame_start_in_byte;
            zsd_reg_vdo1_.desc_uv_frame_start_in_byte   = param_out.uv_frame_start_in_byte;
        }

    }
#endif
#endif
    return 0;
            
}


/*******************************************************************************
 QueueRefill:
    "Zero Shutter path" has special QueueRefill() due to zoom function.

    When ask for enqueue
    1 check if zoom parameter dirty. If yes...
        1.1 Wait until all ROT DMA queue empty. 
        1.2 Update register to all RESZ element.( CRZ/PRZ0/PRZ1/VRZ )
        1.3 Refill ROT descriptor. (The same)

 *******************************************************************************/
int MdpPathCameraPreviewZeroShutter::QueueRefill( unsigned long resource_to_refill_mask )
{
#if 0
    int             ret_val = MDP_ERROR_CODE_OK;
    ROTDMA_I*       pOutRot = &me_vdorot1;  /*vdo1 output the encode frame*/
    int             i;
    MDPELEMENT_I**  mdplist;
    int             mdpcount;
    unsigned long   resc_mask;

    mdplist  =  this->mdp_element_list();
    mdpcount =  this->mdp_element_count();

    if( b_zsd_zoom_regdata_dirty_ )
    {
        /*Clear Dirty bit*/
        b_zsd_zoom_regdata_dirty_ = 0;
    /*-----------------------------------------------------------------------------
        1.1 Wait until all ROT DMA queue empty. 
      -----------------------------------------------------------------------------*/
        for( i = 0 ; i < mdpcount ; i++ )
        {
            const unsigned long ALL_ROT_ID = MID_RGB_ROT0|MID_RGB_ROT1|MID_RGB_ROT2|MID_VDO_ROT0|MID_VDO_ROT1;
            if( mdplist[i]->id() & ALL_ROT_ID )
            {
                ((ROTDMA_I *)mdplist[i])->DescQueueWaitEmpty(); /*Busy wait until queue is empty*/
            }
        }
 
    /*-----------------------------------------------------------------------------
        1.2 Update register to all RESZ element.( CRZ/PRZ0/PRZ1/VRZ )
      -----------------------------------------------------------------------------*/
        resc_mask = ResourceIdMaskGet();
        me_crz.ConfigZsdZoom_RegWrite( zsd_reg_crz_ );
        if(  resc_mask & MID_PRZ0 )     {   me_prz0.ConfigZsdZoom_RegWrite( zsd_reg_prz0_ );    }
        if(  resc_mask & MID_VRZ0 )     {   me_vrz0.ConfigZsdZoom_RegWrite( zsd_reg_vrz0_ );    }
        if(  resc_mask & MID_PRZ1 )     {   me_prz1.ConfigZsdZoom_RegWrite( zsd_reg_prz1_ );    }
        if(  resc_mask & MID_VDO_ROT1 ) {   me_vdorot1.ConfigZsdZoom_DescUpdate( zsd_reg_vdo1_ );   }
      

    /*-----------------------------------------------------------------------------
        1.3 Refill ROT descriptor. 
      -----------------------------------------------------------------------------*/
    
    }
#endif

    /*Call paraent's QueueRefill*/
    return MdpPath_I::QueueRefill( resource_to_refill_mask );
}


int MdpPathCameraPreviewZeroShutter::_EndPost( void* pParam )
{
    /*
    MDP_DEBUG("ZSD delay 2s\n");
    usleep( 2000000 ); //2s
    */

    return 0;
}


