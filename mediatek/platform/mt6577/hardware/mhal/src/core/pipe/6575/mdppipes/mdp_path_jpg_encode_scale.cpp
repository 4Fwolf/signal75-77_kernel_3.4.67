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
  C++ Version - MdpPathJpgEncodeScale
  /////////////////////////////////////////////////////////////////////////////*/
int MdpPathJpgEncodeScale::_StartPre( void* p_parameter )
{
#if defined( MDP_FLAG_1_SUPPORT_JPEGCODEC )

    if( b_jpg_path_disen_ == 0 )
    {
    //1.Lock Jpeg Encorder    
    if(JPEG_ENC_STATUS_OK != jpegEncLockEncoder(&jpg_encID_))
    {
        MDP_ERROR("Lock Jpg Encoder Fail");
        return -1;
    }

    //2.Config Jpg Enc HW
    if(JPEG_ENC_STATUS_OK != jpegEncConfigEncoder(jpg_encID_, jpg_inJpgEncParam_))
    {
        MDP_ERROR("Config Jpg Encoder Fail");
        return false;
    }

    //3.Trigger Jpeg Encoder before MDP
    if(JPEG_ENC_STATUS_OK != jpegEncStart(jpg_encID_))
    {
        MDP_ERROR("Trigger Jpg Encoder Fail");
        return false;
    }
    }

    return 0;
#else

    return -1;

#endif
}


int MdpPathJpgEncodeScale::_StartPost( void* p_parameter )
{

    
    
    
    return 0;
}

int MdpPathJpgEncodeScale::_WaitBusyPre( unsigned long rsc_to_wait_mask )
{


    return 0;
}

int MdpPathJpgEncodeScale::_WaitBusyPost( unsigned long rsc_to_wait_mask )
{
    
#if defined( MDP_FLAG_1_SUPPORT_JPEGCODEC )
    //Wait Jpeg Encoder after MDP
    JPEG_ENC_RESULT_ENUM result;
    
    if( b_jpg_path_disen_ == 0 )
    {
    
    if(JPEG_ENC_STATUS_OK != jpegEncWaitIRQ( jpg_encID_, 3000, &jpg_encode_size_, &result) )
    {
        MDP_ERROR("Jpg Encoder Wait IRQ Fail");
        return -1;
    }

    MDP_INFO("Jpg Encoder OK. size = %d \n\r", jpg_encode_size_ );
    }

    return 0;

#else

    return 0;
    
#endif
}


int MdpPathJpgEncodeScale::_EndPre( void* p_parameter )
{


    return 0;
}

int MdpPathJpgEncodeScale::_EndPost( void* p_parameter )
{
#if defined( MDP_FLAG_1_SUPPORT_JPEGCODEC )

    if( b_jpg_path_disen_ == 0 )
    {
    //1.UnLock Jpeg Encorder    
    jpegEncUnlockEncoder(jpg_encID_);
    }
    
    return 0;

#else

    return 0;

#endif
}

int MdpPathJpgEncodeScale::Config( struct MdpPathJpgEncodeScaleParameter* p_parameter )
{
    int b_sensor    = p_parameter->b_sensor_input;
    int b_qv_en     = p_parameter->b_qv_path_en;
    int b_ff_en     = p_parameter->b_ff_path_en;
    MdpSize         crz_dst_size;   /*Use to decide 1st priority crz dst size*/
    
    int b_camera_in;
    int b_continuous;

    MdpDrvColorInfo_t   color_info;
    int b_qv_generic_yuv;
    int b_ff_generic_yuv;


    b_jpg_path_disen_ = p_parameter->b_jpg_path_disen;
    
   

    m_mdp_element_count = 0;

    if( (b_jpg_path_disen_ == 1) && (b_qv_en == 0) && (b_ff_en == 0) )
    {
        MDP_WARNING("MdpPathJpgEncodeScale no path enabled!\n");
        return 0;
    }
    
    /*-----------------------------------------------------------------------------
        Decide path by output color format
      -----------------------------------------------------------------------------*/
    MdpDrvColorFormatInfoGet( p_parameter->qv_color_format , &color_info );
    b_qv_generic_yuv = color_info.b_is_generic_yuv;
    
    MdpDrvColorFormatInfoGet( p_parameter->ff_color_format , &color_info );
    b_ff_generic_yuv = color_info.b_is_generic_yuv;

    
    
    
    /*-----------------------------------------------------------------------------
        Manage MDP resource list
      -----------------------------------------------------------------------------*/
    /*Input from sensor or memory*/
    if( b_sensor == 0 )
    {
            
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_rdma0;
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_crz;
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_ipp;
    }
    else
    {
        
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_crz;
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_ipp;
    }

    /*jpeg encode path*/
    if( b_jpg_path_disen_ == 0 )
    {
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_jpegdma;
    }

    /*quickview path*/
    if( b_qv_en )
    {
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_prz0;

        if( b_qv_generic_yuv ){
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vdo0;
        }else{
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_rgb0;
    }
    }

    
    /*full frame path*/
    if( b_ff_en )
    {
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vrz0;

        if( b_ff_generic_yuv ){
        m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_vdo1;
        }else{
            m_mdp_element_list[m_mdp_element_count++] = (MDPELEMENT_I*)&me_rgb1;
        }
    }

    

    /*-----------------------------------------------------------------------------
        Config MDP Elements
      -----------------------------------------------------------------------------*/

    if( b_sensor == 0 )
    {
        b_camera_in  = 0;
        b_continuous = 0;
    } else
    {
        b_camera_in  = 1;
        b_continuous = 0; //Always single capture when capture
    }

    
    //Do not use vsync, to keep register setting if timeout.or register setting will be cleared by vsync
    b_camera_in = 0;

    
    /*-----------------------------------------------------------------------------
        Decide CRZ destination size
      -----------------------------------------------------------------------------*/
    {
        MdpSize jpg_size, ff_size, qv_size;

        if( b_jpg_path_disen_ == 1 )
        {
        jpg_size = ( b_jpg_path_disen_ == 1 )   ? MdpSize( 0, 0 ) : p_parameter->jpg_img_size;
        ff_size  = ( b_ff_en == 0 )             ? MdpSize( 0, 0 ) : MdpSize( p_parameter->ff_img_roi.w, p_parameter->ff_img_roi.h );
        qv_size  = ( b_qv_en == 0 )             ? MdpSize( 0, 0 ) : MdpSize( p_parameter->qv_img_roi.w, p_parameter->qv_img_roi.h );

        crz_dst_size.w = MDP_MAX3( jpg_size.w, ff_size.w, qv_size.w);
        crz_dst_size.h = MDP_MAX3( jpg_size.h, ff_size.h, qv_size.h);
    }
        else
        {
            crz_dst_size = p_parameter->jpg_img_size;;
        }
    }
   

    /*................................*/
    /*Image source from memory (RDMA0)*/
    /*................................*/
    if( b_sensor == 0 ) 
    {
        /***RDMA0***//*Crop*/
        //RDMA_I
        me_rdma0.uInBufferCnt = 1;
        me_rdma0.src_img_yuv_addr[0] = p_parameter->src_yuv_img_addr;                                 
        me_rdma0.src_img_size = p_parameter->src_img_size;
        me_rdma0.src_img_roi = p_parameter->src_img_roi;
        me_rdma0.src_color_format = p_parameter->src_color_format;
        
        me_rdma0.bHWTrigger = 0;
        me_rdma0.bContinuous = b_continuous;
        me_rdma0.bCamIn = b_camera_in;

        me_rdma0.bEISEn = 0;
        me_rdma0.u4EISCON = 0;//EIS setting
        //RDMA0
        me_rdma0.to_cam = 1;
        me_rdma0.to_ovl = 0;
        me_rdma0.to_mout = 0;

        me_rdma0.trigger_src = 0;

        
        /***CRZ***//*Scale*/
        //RESZ_I
        me_crz.src_img_size.w = me_rdma0.src_img_roi.w;
        me_crz.src_img_size.h = me_rdma0.src_img_roi.h;
        
        me_crz.src_img_roi.x = 0;
        me_crz.src_img_roi.y = 0;
        me_crz.src_img_roi.w = me_rdma0.src_img_roi.w;
        me_crz.src_img_roi.h = me_rdma0.src_img_roi.h;
        
        
        me_crz.dst_img_size = crz_dst_size;
        
        me_crz.uUpScaleCoeff = p_parameter->resz_coeff.crz_up_scale_coeff;
        me_crz.uDnScaleCoeff = p_parameter->resz_coeff.crz_dn_scale_coeff;
        
        /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
        me_crz.uEEHCoeff = 0;
        me_crz.uEEVCoeff = 0;
        
        me_crz.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
        me_crz.bCamIn = b_camera_in;
        
        me_crz.src_sel = 0;// 0:RDMA0 , 1:BRZ, 2:Camera
        
        //me_crz.bBypass = 0;
        
        me_crz.bBypass = 0;
                    /*if no crop...*/
        if( (me_crz.src_img_roi.x == 0 ) &&                   
            (me_crz.src_img_roi.y == 0 ) &&
            (me_crz.src_img_roi.w == me_crz.src_img_size.w ) &&
            (me_crz.src_img_roi.h == me_crz.src_img_size.h ) )
        {
                    /*and if no scale...*/
            if( (me_crz.src_img_roi.w == me_crz.dst_img_size.w ) &&
                (me_crz.src_img_roi.h == me_crz.dst_img_size.h ) )
            {
                    /*JUST bypass it! for large size jpeg encode*/
               me_crz.bBypass= 1;
            }
        }
        
    }
    /*................................*/
    /*Image source from sensor        */
    /*................................*/
    else
    {

        /***CRZ***//*Crop and Scale*/
        //RESZ_I
        me_crz.src_img_size = p_parameter->src_img_size;

        me_crz.src_img_roi  = p_parameter->src_img_roi;

        me_crz.dst_img_size = crz_dst_size;//= p_parameter->dst_img_roi.w & .h; /*Currently dst roi is not used*/

        me_crz.uUpScaleCoeff = p_parameter->resz_coeff.crz_up_scale_coeff;
        me_crz.uDnScaleCoeff = p_parameter->resz_coeff.crz_dn_scale_coeff;

        /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
        me_crz.uEEHCoeff = 0;
        me_crz.uEEVCoeff = 0;

        me_crz.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
        me_crz.bCamIn = b_camera_in;
        me_crz.bBypass = 0;

        me_crz.src_sel = 2;// 0:RDMA0 , 1:BRZ, 2:Camera

    }


    /***IPP***/
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
    me_ipp.to_jpg_dma = (b_jpg_path_disen_==0)?1:0;
    me_ipp.to_vdo_rot0 = 0;
    me_ipp.to_prz0 = b_qv_en;
    me_ipp.to_vrz0 = b_ff_en;
    me_ipp.to_rgb_rot0 = 0;

    /*input sel*/
    me_ipp.src_sel = 1;//0 : OVL, 1:CRZ

    me_ipp.bCamIn = b_camera_in;
    me_ipp.bBypass = 0;


    /*................................*/
    /*Jpeg encode Path                */
    /*................................*/
    if( b_jpg_path_disen_ == 0 )
    {
        /***JPEGDMA***/
        me_jpegdma.src_img_size = me_crz.dst_img_size;
        me_jpegdma.output_format = p_parameter->jpg_yuv_color_format;   //integer : 411, 422, 444, 400, 410 etc...

        /*camera option*/
        me_jpegdma.bContinuous = b_continuous;
        me_jpegdma.bCamIn = b_camera_in;    // 1: camera input

        /*source sel*/
        me_jpegdma.src_sel = 0; // 0-IPP_MOUT, 1-MOUT
    }

    
    /*................................*/
    /*QuickView Path                  */
    /*................................*/
    if( b_qv_en )
    {
        /***PRZ0***/
        //RESZ_I
        me_prz0.src_img_size = me_crz.dst_img_size;

        me_prz0.src_img_roi.x = 0;
        me_prz0.src_img_roi.y = 0;
        me_prz0.src_img_roi.w = me_prz0.src_img_size.w;
        me_prz0.src_img_roi.h = me_prz0.src_img_size.h;
        
        //me_prz0.dst_img_size = p_parameter->qv_img_size;
        me_prz0.dst_img_size = MdpSize( p_parameter->qv_img_roi.w, p_parameter->qv_img_roi.h );

        me_prz0.uUpScaleCoeff = p_parameter->resz_coeff.prz0_up_scale_coeff;
        me_prz0.uDnScaleCoeff = p_parameter->resz_coeff.prz0_dn_scale_coeff;

        /*coefficient table for resizing.1:most blur 19:sharpest 0:linear interpolation rather than cubic*/
        me_prz0.uEEHCoeff = p_parameter->resz_coeff.prz0_ee_h_str;
        me_prz0.uEEVCoeff = p_parameter->resz_coeff.prz0_ee_v_str;

        me_prz0.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
        me_prz0.bCamIn = b_camera_in;
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
        /*input sel*/
        me_prz0.src_sel = 1;//0-MOUT,1-IPP_MOUT,2-CAM,3-BRZ_MOUT

        /*output sel*/
        me_prz0.to_vdo_rot0 = b_qv_generic_yuv ? 1 : 0;//0;
        me_prz0.to_rgb_rot0 = b_qv_generic_yuv ? 0 : 1;//1;
        me_prz0.to_vrz0 = 0;

        if( b_qv_generic_yuv )
        {
            /***VDOROT0***/
            //ROTDMA_I
            me_vdo0.dst_img_yuv_addr[0] = p_parameter->qv_yuv_img_addr;

            me_vdo0.src_img_roi = MdpRect( me_prz0.dst_img_size );

            me_vdo0.dst_img_size = p_parameter->qv_img_size; 
            
            me_vdo0.dst_color_format = p_parameter->qv_color_format;

            me_vdo0.uOutBufferCnt = 1;
            me_vdo0.bContinuous = b_continuous;// 1:Continuous mode does not allow time sharing.
            me_vdo0.bFlip = p_parameter->qv_flip;
            me_vdo0.bRotate = p_parameter->qv_rotate;//0:0, 1:90, 2:180, 3:270
            me_vdo0.bDithering = 0;
            me_vdo0.bCamIn = b_camera_in;// 1: real time path, ex : camera input
            me_vdo0.bEnHWTriggerRDMA0 = 0;//p_parameter->b_hw_trigger_in;// 0:Disable , 1:Enable    

            //VDOROT0
            me_vdo0.src_sel = 0;// 0-PRZ0_MOUT, 1-IPP_MOUT
            
        }
        else
        {
        /***RGBROT0***/
        //ROTDMA_I
        me_rgb0.dst_img_yuv_addr[0] = p_parameter->qv_yuv_img_addr; 
        
        me_rgb0.src_img_roi = MdpRect( me_prz0.dst_img_size );

        me_rgb0.dst_img_size = p_parameter->qv_img_size;  

        me_rgb0.dst_color_format = p_parameter->qv_color_format;

        me_rgb0.uOutBufferCnt = 1;
        me_rgb0.bContinuous = b_continuous;// 1:Continuous mode does not allow time sharing.
        me_rgb0.bFlip = p_parameter->qv_flip;
        me_rgb0.bRotate = p_parameter->qv_rotate;//0:0, 1:90, 2:180, 3:270
        me_rgb0.bDithering = 0;
        me_rgb0.bCamIn = b_camera_in;// 1: real time path, ex : camera input
        me_rgb0.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

        //RGBROT0
        me_rgb0.src_sel = 0;//0-PRZ0_MOUT ,1-IPP_MOUT
        }


    }
    
    if( b_ff_en )
    {
        /***VRZ0***/
        //VRZ_I
        me_vrz0.src_img_size    = me_crz.dst_img_size;
        me_vrz0.dst_img_size    = MdpSize( p_parameter->ff_img_roi.w, p_parameter->ff_img_roi.h );
        
        me_vrz0.bContinuous = b_continuous;//0: single shot, 1: continuous, set to continuous only under camera input case (CLS:sensor must be configured to continuous as well)
        me_vrz0.bCamIn = b_camera_in;
        me_vrz0.bBypass = 0;

        /*If no scale, by pass*/
        if( ( me_vrz0.src_img_size.w == me_vrz0.dst_img_size.w ) &&
            ( me_vrz0.src_img_size.h == me_vrz0.dst_img_size.h ) )
        {
            me_vrz0.bBypass = 1;
        }
                
        //VRZ0
        me_vrz0.src_sel = 2;// 0-MOUT, 1-BRZ_MOUT, 2-IPP_MOUT, 3-OVL_DMA_MIMO, 4-PRZ0_MOUT
            

        if( b_ff_generic_yuv )
        {
        /***VDOROT1***/
        //ROTDMA_I
        me_vdo1.dst_img_yuv_addr[0] = p_parameter->ff_yuv_img_addr;

        me_vdo1.src_img_roi = MdpRect( me_vrz0.dst_img_size );

        me_vdo1.dst_img_size = p_parameter->ff_img_size;  /*Currently, "dst roi"  should be equal to "dst size"*/
            me_vdo1.dst_color_format = p_parameter->ff_color_format;

        me_vdo1.uOutBufferCnt = 1;
        me_vdo1.bContinuous = 0;// 1:Continuous mode does not allow time sharing.
        me_vdo1.bFlip = p_parameter->ff_flip;
        me_vdo1.bRotate = p_parameter->ff_rotate;//0:0, 1:90, 2:180, 3:270
        me_vdo1.bDithering = 0;
        me_vdo1.bCamIn = b_camera_in;// 1: real time path, ex : camera input
        me_vdo1.bEnHWTriggerRDMA0 = 0;// 0:Disable , 1:Enable

        //VDOROT1
        me_vdo1.src_sel= 1;// 0-PRZ1, 1-VRZ0
        }
        else
        {
            /***RGBROT1***/
            //ROTDMA_I
            me_rgb1.dst_img_yuv_addr[0] = p_parameter->ff_yuv_img_addr;

            me_rgb1.src_img_roi = MdpRect( me_vrz0.dst_img_size );

            me_rgb1.dst_img_size = p_parameter->ff_img_size; 
            me_rgb1.dst_color_format = p_parameter->ff_color_format;

            me_rgb1.uOutBufferCnt = 1;
            me_rgb1.bContinuous = 0;// 1:Continuous mode does not allow time sharing.
            me_rgb1.bFlip = p_parameter->ff_flip;
            me_rgb1.bRotate = p_parameter->ff_rotate;//0:0, 1:90, 2:180, 3:270
            me_rgb1.bDithering = 0;
            me_rgb1.bCamIn = b_camera_in;// 1: real time path, ex : camera input
            me_rgb1.bEnHWTriggerRDMA0 = 0;//p_parameter->b_hw_trigger_in;// 0:Disable , 1:Enable    

            //RGBROT1
            /*Nothing*/
            
        }


        
    }



    
    /*-----------------------------------------------------------------------------
        Config Jpeg Encode Object
      -----------------------------------------------------------------------------*/
    #if defined( MDP_FLAG_1_SUPPORT_JPEGCODEC )

    if( b_jpg_path_disen_ == 0 )
    {
        // config jpeg encoder
        jpg_inJpgEncParam_.dstBufferAddr = (unsigned char*)p_parameter->jpg_buffer_addr;
        jpg_inJpgEncParam_.dstBufferSize = p_parameter->jpg_buffer_size;
        jpg_inJpgEncParam_.dstWidth = p_parameter->jpg_img_size.w;
        jpg_inJpgEncParam_.dstHeight = p_parameter->jpg_img_size.h;
        if(p_parameter->jpg_quality > 90)
        {
            jpg_inJpgEncParam_.dstQuality = JPEG_ENCODE_QUALITY_Q95;
        } 
        else if(p_parameter->jpg_quality > 60)
        {
            jpg_inJpgEncParam_.dstQuality = JPEG_ENCODE_QUALITY_Q80;
        }
        else if(p_parameter->jpg_quality > 40)
        {
            jpg_inJpgEncParam_.dstQuality = JPEG_ENCODE_QUALITY_Q60;
        }
        else
        {
            jpg_inJpgEncParam_.dstQuality = JPEG_ENCODE_QUALITY_Q39;
        }

        jpg_inJpgEncParam_.isPhyAddr = 0;

        if(p_parameter->jpg_b_add_soi)
            jpg_inJpgEncParam_.enableEXIF = 0;
        else
            jpg_inJpgEncParam_.enableEXIF = 1;

        switch ( p_parameter->jpg_yuv_color_format )
        {
        case 400:
            jpg_inJpgEncParam_.dstFormat = JPEG_SAMPLING_FORMAT_GRAYLEVEL;
        break;

        case 420:
            jpg_inJpgEncParam_.dstFormat = JPEG_SAMPLING_FORMAT_YUV420;
        break;

        case 422:
            jpg_inJpgEncParam_.dstFormat = JPEG_SAMPLING_FORMAT_YUV422;
        break;

        case 411:
            jpg_inJpgEncParam_.dstFormat = JPEG_SAMPLING_FORMAT_YUV411;
        break;

        case 444:
            jpg_inJpgEncParam_.dstFormat = JPEG_SAMPLING_FORMAT_YUV444;
        break;

        default :
            MDP_ERROR("Unsupport Encoder Format : %d", (int)p_parameter->jpg_yuv_color_format );
            return -1;
            
        }


        jpg_inJpgEncParam_.enableSyncReset = b_camera_in;

    }

    #endif /*#if defined( MDP_FLAG_1_SUPPORT_JPEGCODEC )*/
    

    return 0;
    
}



