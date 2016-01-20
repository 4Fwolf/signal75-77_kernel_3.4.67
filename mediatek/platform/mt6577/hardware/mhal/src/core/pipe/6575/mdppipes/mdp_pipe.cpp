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

#include "mhal_interface.h"   //For register loopmemory, this is not good
#include "mdp_pipe.h"


#include <string.h> //For memcpy

//strerror
#include <string.h>
#include <errno.h>


/*/////////////////////////////////////////////////////////////////////////////
    Global Variable
  /////////////////////////////////////////////////////////////////////////////*/
static RegisterLoopMemory_t    loop_mem_param[4][MT6575RDMA_MAX_RINGBUFFER_CNT];
static RegisterLoopMemoryObj_t loop_mem_obj[4][MT6575RDMA_MAX_RINGBUFFER_CNT];

/*/////////////////////////////////////////////////////////////////////////////
    Static Function
  /////////////////////////////////////////////////////////////////////////////*/

/*=============================================================================
    Input a roi, output a even number aligned roi (rough) and a fine roi
  =============================================================================*/
static void _SplitRoi( MdpRect roi, MdpRect *p_rough_crop_roi, MdpRect  *p_fine_crop_roi )
{
    unsigned long   delta_x = 0;
    unsigned long   delta_y = 0;

    /*2.1 Seamless 3D ROI calculation*/
    /*.............................................................................*/
    //Calculate rough roi , clip by RDMA which require even value
    p_rough_crop_roi->x = MDP_ROUND_DOWN_VALUE( roi.x, 1 ); //Round down to 2x
    p_rough_crop_roi->y = MDP_ROUND_DOWN_VALUE( roi.y, 1 ); //Round down to 2x

    delta_x = roi.x - p_rough_crop_roi->x;
    delta_y = roi.y - p_rough_crop_roi->y;

    //RDMA roi w/h also need to fit even rule.
    p_rough_crop_roi->w = MDP_ROUND_UP_VALUE( roi.w + delta_x , 1 );
    p_rough_crop_roi->h = MDP_ROUND_UP_VALUE( roi.h + delta_y , 1 );

    //calculate fine roi.(odd part if any)
    p_fine_crop_roi->x = delta_x;
    p_fine_crop_roi->y = delta_y;
    p_fine_crop_roi->w = roi.w;
    p_fine_crop_roi->h = roi.h;
}


/*#############################################################################
    C++ Version
  #############################################################################*/
/*/////////////////////////////////////////////////////////////////////////////
    MdpPipe_I
  /////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////
    MdpPipeCameraPreview
  /////////////////////////////////////////////////////////////////////////////*/
int MdpPipeCameraPreview::ConfigZoom( MdpRect crop_size, MdpRect* p_real_crop_size )
{
    int ret_val = 0;

    /*case 1: Normal Path Zoom*/
    if( ( en_zero_shutter_path_ == 0 || en_zero_shutter_path_ == 2 ) && en_one_pass_preview_path_ != 1 )
    {
        if( p_real_crop_size != NULL ){
            *p_real_crop_size = crop_size;
        }


    #if defined( MDP_FLAG_USE_RDMA_TO_CROP )
        if( en_sw_trigger_ == 0 )
        {
            MDP_ERROR("RDMA cropping in zoom is not implement for hw trigger path\n");
            ret_val = -1;
        }
        else
        {
            #if 1
            if( path_1_rotate_angle_ & 0x1 ) /*90/270 rotate*/
            {
                zoom_crop_region_2nd_path_.x = crop_size.y;
                zoom_crop_region_2nd_path_.y = crop_size.x;
                zoom_crop_region_2nd_path_.w = crop_size.h;
                zoom_crop_region_2nd_path_.h = crop_size.w;
            }
            else /*No rotation needed*/
            {
                zoom_crop_region_2nd_path_.x = crop_size.x;
                zoom_crop_region_2nd_path_.y = crop_size.y;
                zoom_crop_region_2nd_path_.w = crop_size.w;
                zoom_crop_region_2nd_path_.h = crop_size.h;
            }
            #else /*TEST CODE*/
            if( path_1_rotate_angle_ & 0x1 ) /*90/270 rotate*/
            {
                zoom_crop_region_2nd_path_.x = (int)(crop_size.y*0.3) & ~0x1;
                zoom_crop_region_2nd_path_.y = (int)(crop_size.x*0.3) & ~0x1;
                zoom_crop_region_2nd_path_.w = (int)(crop_size.h*0.3) & ~0x1;
                zoom_crop_region_2nd_path_.h = (int)(crop_size.w*0.3) & ~0x1;
            }
            else /*No rotation needed*/
            {
                zoom_crop_region_2nd_path_.x = (int)(crop_size.x*0.3) & ~0x1;
                zoom_crop_region_2nd_path_.y = (int)(crop_size.y*0.3) & ~0x1;
                zoom_crop_region_2nd_path_.w = (int)(crop_size.w*0.3) & ~0x1;
                zoom_crop_region_2nd_path_.h = (int)(crop_size.h*0.3) & ~0x1;
            }
            #endif

        }
    #else
        /*In CRZ crop and upscale, no matter sw or hw trigger, use the same crz config*/
        ret_val = m_MdpPathCameraPreviewToMemory.ConfigZoom( crop_size );

    #endif
    }
    /*case 2: Zero Shutter Path Zoom & 1080P one pass*/
    else
    {
        //MDP_ERROR("Zoom Function under zero shutter path is under construction\n");
        ret_val = m_MapPathCameraPreviewZeroShutter.ConfigZoom( crop_size , p_real_crop_size );
    }

    return ret_val;

}

int MdpPipeCameraPreview::Config( struct MdpPipeCameraPreviewParameter* p_parameter )
{
    if( p_parameter == 0 )
        return -1;

    en_one_pass_preview_path_   = p_parameter->en_one_pass_preview_path;
    en_sw_trigger_        = p_parameter->en_sw_trigger;
    en_zero_shutter_path_ = p_parameter->en_zero_shutter_path;
    en_n3d_preview_path_  = p_parameter->en_n3d_preview_path;

    /*one pass preview path (for 1080p support)*/
    if( en_one_pass_preview_path_ == 1 )
    {
        return _ConfigOnePassPath( p_parameter );
    }
    /*Native 3D preview path*/
    else if( en_n3d_preview_path_ == 1 )
    {
        MDP_INFO("Camera Preview Config: Native 3D path.\n\r");
        return _ConfigN3dPath( p_parameter );
    }
    /*Normal Preview Path @ ZSD Ver.2 Path*/
    else if( en_zero_shutter_path_ == 0 || en_zero_shutter_path_ == 2)
    {
        if( en_sw_trigger_ == 0 )
        {
            MDP_INFO("Camera Preview Config: 2 stage normal path (HW trigger).\n\r");
            return _ConfigNormalPathHwTrigger( p_parameter );
        } else
        {
            MDP_INFO("Camera Preview Config: 2 stage normal path (SW trigger).\n\r");
            return _ConfigNormalPathSwTrigger( p_parameter );
        }

    }
    /*ZSD ver.1 path*/
    else
    {
        MDP_INFO("Camera Preview Config: Zero shutter path.\n\r");
        return _ConfigZeroShutterPath( p_parameter );
    }

    return -1;

}

int MdpPipeCameraPreview::_ConfigNormalPathHwTrigger( struct MdpPipeCameraPreviewParameter* p_parameter )
{
    const int   B_HWTRIGGER = 1;
    int         ret = 0;
    struct      MdpPathCameraPreviewToMemoryParameter    camera_preview_path_param;
    struct      MdpPathDisplayFromMemoryParameter        display_from_memory_param;

    camera_preview_path_param.pseudo_src_enable = p_parameter->pseudo_src_enable;

    /*-----------------------------------------------------------------------------
      Parametr Sanity Test
      -----------------------------------------------------------------------------*/
      if( ( camera_preview_path_param.pseudo_src_enable == 1 ) &&
          ( p_parameter->en_dst_port0 == 1 || p_parameter->en_dst_port1 == 1 || p_parameter->en_dst_port2 == 1  ))
      {
        MDP_ERROR("pseduo source cannot combine with phase 2 output!\n\r");
        return -1;
      }


    /***Parameter Convertion***/
    /*-----------------------------------------------------------------------------
      camera_preview_path_param
      -----------------------------------------------------------------------------*/
    /*pseduo source*/
    camera_preview_path_param.pseudo_src_enable         =    p_parameter->pseudo_src_enable;
    camera_preview_path_param.pseudo_src_buffer_count   =    p_parameter->dst_buffer_count; /*Same as dst buffer count*/
    camera_preview_path_param.pseudo_src_color_format   =    p_parameter->pseudo_src_color_format;
    camera_preview_path_param.pseudo_src_yuv_img_addr   =    p_parameter->pseudo_src_yuv_img_addr;

    /*Debug*/
    camera_preview_path_param.debug_preview_single_frame_enable = p_parameter->debug_preview_single_frame_enable;

    /*HW Trigger*/
    camera_preview_path_param.b_hw_trigger_out = B_HWTRIGGER;   /*hw trigger RDMA0*/

    /*source*/
    camera_preview_path_param.src_img_size              =    p_parameter->src_img_size;
    camera_preview_path_param.src_img_roi               =    p_parameter->src_img_roi;

    /*Working buffer*/
    camera_preview_path_param.dst_buffer_count          =    p_parameter->dst_buffer_count;
    camera_preview_path_param.dst_color_format          =    p_parameter->dst_color_format;
    camera_preview_path_param.dst_img_size              =    p_parameter->dst_img_size;
    camera_preview_path_param.dst_img_roi               =    p_parameter->dst_img_roi;
    camera_preview_path_param.dst_yuv_img_addr          =    p_parameter->dst_yuv_img_addr;
    camera_preview_path_param.dst_rotate_angle          =    p_parameter->dst_rotate_angle;


    /*resz coeff*/
    camera_preview_path_param.resz_coeff                =    p_parameter->resz_coeff;

    /*-----------------------------------------------------------------------------
        display_from_memory_param
      -----------------------------------------------------------------------------*/
    /*HW Trigger*/
    display_from_memory_param.b_hw_trigger_in = B_HWTRIGGER;
    display_from_memory_param.hw_trigger_src = 1;//1:RGB_ROT0 from MdpPathCameraPreviewToMemory

    if( p_parameter->debug_preview_single_frame_enable == 1 )
    {
        display_from_memory_param.b_continuous = 0; //debug preview setting
    } else
    {
        display_from_memory_param.b_continuous = 1; //normal preview setting
    }

    if( p_parameter->pseudo_src_enable == 1 )
    {
        display_from_memory_param.b_camera_in = 0;  //debug preview setting
    } else
    {
        display_from_memory_param.b_camera_in = 1;  //normal preview setting
    }


    /*Src input from working buffer*/
    display_from_memory_param.src_buffer_count=          p_parameter->dst_buffer_count;
    display_from_memory_param.src_color_format=          p_parameter->dst_color_format;
    display_from_memory_param.src_img_size=              p_parameter->dst_img_size;
    display_from_memory_param.src_img_roi=               p_parameter->dst_img_roi;
    display_from_memory_param.src_yuv_img_addr=          p_parameter->dst_yuv_img_addr;


    /*output 0*/
    display_from_memory_param.en_dst_port0=              p_parameter->en_dst_port0;
    display_from_memory_param.dst0_buffer_count=         p_parameter->dst0_buffer_count;
    display_from_memory_param.dst0_color_format=         p_parameter->dst0_color_format;
    display_from_memory_param.dst0_img_size=             p_parameter->dst0_img_size;
    display_from_memory_param.dst0_img_roi=              p_parameter->dst0_img_roi;
    display_from_memory_param.dst0_yuv_img_addr=         p_parameter->dst0_yuv_img_addr;
    display_from_memory_param.dst0_rotate_angle=         p_parameter->dst0_rotate_angle;

    /*output 1*/
    display_from_memory_param.en_dst_port1=              p_parameter->en_dst_port1;
    display_from_memory_param.dst1_buffer_count=         p_parameter->dst1_buffer_count;
    display_from_memory_param.dst1_color_format=         p_parameter->dst1_color_format;
    display_from_memory_param.dst1_img_size=             p_parameter->dst1_img_size;
    display_from_memory_param.dst1_img_roi=              p_parameter->dst1_img_roi;
    display_from_memory_param.dst1_yuv_img_addr=         p_parameter->dst1_yuv_img_addr;
    display_from_memory_param.dst1_rotate_angle=         p_parameter->dst1_rotate_angle;

    /*output 2*/
    display_from_memory_param.en_dst_port2=              p_parameter->en_dst_port2;
    display_from_memory_param.dst2_buffer_count=         p_parameter->dst2_buffer_count;
    display_from_memory_param.dst2_color_format=         p_parameter->dst2_color_format;
    display_from_memory_param.dst2_img_size=             p_parameter->dst2_img_size;
    display_from_memory_param.dst2_img_roi=              p_parameter->dst2_img_roi;
    display_from_memory_param.dst2_yuv_img_addr=         p_parameter->dst2_yuv_img_addr;
    display_from_memory_param.dst2_rotate_angle=         p_parameter->dst2_rotate_angle;

    /*resz coeff*/
    display_from_memory_param.resz_coeff=                p_parameter->resz_coeff;


    ret = m_MdpPathCameraPreviewToMemory.Config( &camera_preview_path_param );
    if( ret != 0 )
    {
        MDP_ERROR("Path 1 config error!\n\r");
        return ret;
    }


    ret = m_MapPathDisplayFromMemory.Config( &display_from_memory_param );
    if( ret != 0 )
    {
        MDP_ERROR("Path 2 config error!\n\r");
        return ret;
    }

    return ret;


}

int MdpPipeCameraPreview::_ConfigNormalPathSwTrigger( struct MdpPipeCameraPreviewParameter* p_parameter )
{
    int             ret = 0;
    unsigned long   dummy_buffer_total_size;
    struct      MdpPathCameraPreviewToMemoryParameter    camera_preview_path_param;

    camera_preview_path_param.pseudo_src_enable = p_parameter->pseudo_src_enable;


    //last preview frame for zsd
    /*-----------------------------------------------------------------------------
      Parametr Sanity Test
      -----------------------------------------------------------------------------*/
      if( ( camera_preview_path_param.pseudo_src_enable == 1 ) &&
          ( p_parameter->en_dst_port0 == 1 || p_parameter->en_dst_port1 == 1 || p_parameter->en_dst_port2 == 1  ))
      {
        MDP_ERROR("pseduo source cannot combine with phase 2 output!\n");
        return -1;
      }

      if( ( p_parameter->src_img_roi.w < p_parameter->dst_img_size.w ) || ( p_parameter->src_img_roi.h < p_parameter->dst_img_size.h ) )
      {
        MDP_WARNING("Direct Link CRZ do upscale! src w/h=(%d/%d), dst w/h=(%d/%d)\n",
            (int)p_parameter->src_img_roi.w, (int)p_parameter->src_img_roi.h,
            (int)p_parameter->dst_img_size.w, (int)p_parameter->dst_img_size.h );
      }

    /*-----------------------------------------------------------------------------
      Remember setting for 2nd path
      -----------------------------------------------------------------------------*/
    /*remember the 1st path rotate angle*/
    path_1_rotate_angle_ = p_parameter->dst_rotate_angle;

    /*If enable crop in rdma, then remember the zoom ROI then use it in the 2nd phase*/
    #if defined (MDP_FLAG_USE_RDMA_TO_CROP )
    /*remember and initial zoom crop setting*/
    if( path_1_rotate_angle_ & 0x1 ) {/*90/270 rotate*/
        zoom_crop_region_2nd_path_.x = 0;
        zoom_crop_region_2nd_path_.y = 0;
        zoom_crop_region_2nd_path_.w = p_parameter->dst_img_size.h;
        zoom_crop_region_2nd_path_.h = p_parameter->dst_img_size.w;
    }
    else /*No rotation needed*/
    {
        zoom_crop_region_2nd_path_.x = 0;
        zoom_crop_region_2nd_path_.y = 0;
        zoom_crop_region_2nd_path_.w = p_parameter->dst_img_size.w;
        zoom_crop_region_2nd_path_.h = p_parameter->dst_img_size.h;
    }
    #endif



    /***Parameter Convertion***/
    /*-----------------------------------------------------------------------------
      camera_preview_path_param
      -----------------------------------------------------------------------------*/
    /*pseduo source*/
    camera_preview_path_param.pseudo_src_enable         =    p_parameter->pseudo_src_enable;
    camera_preview_path_param.pseudo_src_buffer_count   =    p_parameter->dst_buffer_count; /*Same as dst buffer count*/
    camera_preview_path_param.pseudo_src_color_format   =    p_parameter->pseudo_src_color_format;
    camera_preview_path_param.pseudo_src_yuv_img_addr   =    p_parameter->pseudo_src_yuv_img_addr;

    /*Debug*/
    camera_preview_path_param.debug_preview_single_frame_enable = p_parameter->debug_preview_single_frame_enable;

    /*HW Trigger*/
    camera_preview_path_param.b_hw_trigger_out = 0;     /*sw trigger*/

    /*source*/
    camera_preview_path_param.src_img_size              =    p_parameter->src_img_size;
    camera_preview_path_param.src_img_roi               =    p_parameter->src_img_roi;

    /*Working buffer*/
    camera_preview_path_param.dst_buffer_count          =    p_parameter->dst_buffer_count;
    camera_preview_path_param.dst_color_format          =    p_parameter->dst_color_format;
    camera_preview_path_param.dst_img_size              =    p_parameter->dst_img_size;
    camera_preview_path_param.dst_img_roi               =    p_parameter->dst_img_roi; //dst_img_roi is not use in this path
    camera_preview_path_param.dst_yuv_img_addr          =    p_parameter->dst_yuv_img_addr;
    camera_preview_path_param.dst_rotate_angle          =    p_parameter->dst_rotate_angle;

    /*resz coeff*/
    camera_preview_path_param.resz_coeff                =    p_parameter->resz_coeff;

    /*last preview for ZSD*/
    camera_preview_path_param.dst_yuv_img_addr_last_preview_ = p_parameter->dst_yuv_img_addr_last_preview_;
    camera_preview_path_param.b_en_zsd_path                  = p_parameter->en_zero_shutter_path;
    display_From_Memory_Src_Size_Last_Preview                = p_parameter->src_img_size;
    disp_src_buffer_last_preview_addr                        = p_parameter->dst_yuv_img_addr_last_preview_;
    camera_preview_path_param.zsd_dst_buffer_count_          = p_parameter->dst_buffer_count_zsd;


    /*---Config----*/
    ret = m_MdpPathCameraPreviewToMemory.Config( &camera_preview_path_param );
    if( ret != 0 )
    {
        MDP_ERROR("Path 1 config error!\n\r");
        return ret;
    }

    /*-----------------------------------------------------------------------------
        display_from_memory_param
      -----------------------------------------------------------------------------*/
    /*HW Trigger*/
    display_from_memory_param_.b_hw_trigger_in   = 0;     //Disable HW trigger
    display_from_memory_param_.hw_trigger_src    = 1;    //Dummy value
    display_from_memory_param_.b_continuous      = 0;    //Manual trigger path
    display_from_memory_param_.b_camera_in       = 0;    //Manual trigger path


    /*Src input from working buffer*/
    MDP_INFO("SW Trigger Src Buffer (RDMA0):\n");
    disp_src_buffer.Reset( 0 );
    disp_src_buffer.SetName("MID_RGB_ROT0");
    disp_src_buffer.enable = 1;
    disp_src_buffer.buffer_count = p_parameter->dst_buffer_count;
    disp_src_buffer.buffer[0]    = p_parameter->dst_yuv_img_addr;
    if( MdpDrv_CalImgBufferArraySizeAndFillIn(
                        p_parameter->dst_color_format,
                        disp_src_buffer.buffer,
                        p_parameter->dst_buffer_count,
                        p_parameter->dst_img_size,
                        MdpRect( 0, 0, p_parameter->dst_img_size.w, p_parameter->dst_img_size.h ),//p_parameter->dst_img_roi, /*Should not use roi to calculate size,the offset will vary*/
                        0, /*rotate*/
                        &dummy_buffer_total_size ) < 0 )
    {
        MDP_ERROR("Calc disp src memory array size error.\n" );
        return -1;
    }




    display_from_memory_param_.src_buffer_count=          1;//p_parameter->dst_buffer_count;//Single trigger
    display_from_memory_param_.src_color_format=          p_parameter->dst_color_format;

    if( path_1_rotate_angle_ & 0x1 ) {/*90/270 rotate*/
        /*set src to the dimension after prior phass rotate*/
        display_from_memory_param_.src_img_size.w   =   p_parameter->dst_img_size.h;
        display_from_memory_param_.src_img_size.h   =   p_parameter->dst_img_size.w;

        display_from_memory_param_.src_img_roi.x    =   p_parameter->dst_img_roi.y;
        display_from_memory_param_.src_img_roi.y    =   p_parameter->dst_img_roi.x;
        display_from_memory_param_.src_img_roi.w    =   p_parameter->dst_img_roi.h;
        display_from_memory_param_.src_img_roi.h    =   p_parameter->dst_img_roi.w;
    }
    else /*No rotation needed*/
    {
        display_from_memory_param_.src_img_size=              p_parameter->dst_img_size;
        display_from_memory_param_.src_img_roi=               p_parameter->dst_img_roi;
    }


    display_from_memory_param_.src_yuv_img_addr=          p_parameter->dst_yuv_img_addr;



    disp_src_buffer.RegisterLoopMemory( MEM_TYPE_INPUT,
                                        p_parameter->dst_color_format,
                                        display_from_memory_param_.src_img_size,
                                        display_from_memory_param_.src_img_roi,
                                        0 /*rotate*/);


    /*output 0*/
    disp_dst0_buffer.Reset( 1 );
    disp_dst0_buffer.enable = p_parameter->en_dst_port0;
    if( p_parameter->en_dst_port0 == 1 )
    {
        MDP_INFO("SW Trigger Dst0 Buffer (VDO0):\n");
        disp_dst0_buffer.SetName("MID_VDO_ROT0");
        disp_dst0_buffer.buffer_count = p_parameter->dst0_buffer_count;
        disp_dst0_buffer.buffer[0] = p_parameter->dst0_yuv_img_addr;
        if( MdpDrv_CalImgBufferArraySizeAndFillIn(
                            p_parameter->dst0_color_format,
                            disp_dst0_buffer.buffer,
                            p_parameter->dst0_buffer_count,
                            p_parameter->dst0_img_size,
                            MdpRect( 0, 0, p_parameter->dst0_img_size.w, p_parameter->dst0_img_size.h ),//p_parameter->dst0_img_roi,
                            p_parameter->dst0_rotate_angle, /*rotate*/
                            &dummy_buffer_total_size ) < 0 )
        {
            MDP_ERROR("Calc disp dst0 memory array size error.\n" );
            return -1;
        }

        disp_dst0_buffer.RegisterLoopMemory( MEM_TYPE_OUTPUT,
                                            p_parameter->dst0_color_format,
                                            p_parameter->dst0_img_size,
                                            p_parameter->dst0_img_roi,
                                            p_parameter->dst0_rotate_angle /*rotate*/);
    }

    display_from_memory_param_.en_dst_port0=              p_parameter->en_dst_port0;
    display_from_memory_param_.dst0_buffer_count=         1;//p_parameter->dst0_buffer_count;//Single trigger
    display_from_memory_param_.dst0_color_format=         p_parameter->dst0_color_format;
    display_from_memory_param_.dst0_img_size=             p_parameter->dst0_img_size;
    display_from_memory_param_.dst0_img_roi=              p_parameter->dst0_img_roi;
    display_from_memory_param_.dst0_yuv_img_addr=         p_parameter->dst0_yuv_img_addr;
    display_from_memory_param_.dst0_rotate_angle=         p_parameter->dst0_rotate_angle;

    /*output 1*/
    disp_dst1_buffer.Reset( 2 );
    disp_dst1_buffer.enable = p_parameter->en_dst_port1;
    if( p_parameter->en_dst_port1 == 1 )
    {
        MDP_INFO("SW Trigger Dst1 Buffer (VDO1):\n");
        disp_dst1_buffer.SetName("MID_VDO_ROT1");
        disp_dst1_buffer.buffer_count = p_parameter->dst1_buffer_count;
        disp_dst1_buffer.buffer[0] = p_parameter->dst1_yuv_img_addr;
        if( MdpDrv_CalImgBufferArraySizeAndFillIn(
                            p_parameter->dst1_color_format,
                            disp_dst1_buffer.buffer,
                            p_parameter->dst1_buffer_count,
                            p_parameter->dst1_img_size,
                            MdpRect( 0, 0, p_parameter->dst1_img_size.w, p_parameter->dst1_img_size.h ),//p_parameter->dst1_img_roi,
                            p_parameter->dst1_rotate_angle, /*rotate*/
                            &dummy_buffer_total_size ) < 0 )
        {
            MDP_ERROR("Calc disp dst1 memory array size error.\n" );
            return -1;
        }

        disp_dst1_buffer.RegisterLoopMemory( MEM_TYPE_OUTPUT,
                                            p_parameter->dst1_color_format,
                                            p_parameter->dst1_img_size,
                                            p_parameter->dst1_img_roi,
                                            p_parameter->dst1_rotate_angle /*rotate*/);
    }
    display_from_memory_param_.en_dst_port1=              p_parameter->en_dst_port1;
    display_from_memory_param_.dst1_buffer_count=         1;//p_parameter->dst1_buffer_count;//Single trigger
    display_from_memory_param_.dst1_color_format=         p_parameter->dst1_color_format;
    display_from_memory_param_.dst1_img_size=             p_parameter->dst1_img_size;
    display_from_memory_param_.dst1_img_roi=              p_parameter->dst1_img_roi;
    display_from_memory_param_.dst1_yuv_img_addr=         p_parameter->dst1_yuv_img_addr;
    display_from_memory_param_.dst1_rotate_angle=         p_parameter->dst1_rotate_angle;

    /*output 2*/
    disp_dst2_buffer.Reset( 3 );
    disp_dst2_buffer.enable = p_parameter->en_dst_port2;
    if( p_parameter->en_dst_port2 == 1 )
    {
        MDP_INFO("SW Trigger Dst2 Buffer (RGB1):\n");
        disp_dst2_buffer.SetName("MID_RGB_ROT1");
        disp_dst2_buffer.buffer_count = p_parameter->dst2_buffer_count;
        disp_dst2_buffer.buffer[0] = p_parameter->dst2_yuv_img_addr;
        if( MdpDrv_CalImgBufferArraySizeAndFillIn(
                            p_parameter->dst2_color_format,
                            disp_dst2_buffer.buffer,
                            p_parameter->dst2_buffer_count,
                            p_parameter->dst2_img_size,
                            MdpRect( 0, 0, p_parameter->dst2_img_size.w, p_parameter->dst2_img_size.h ),//p_parameter->dst2_img_roi,
                            p_parameter->dst2_rotate_angle, /*rotate*/
                            &dummy_buffer_total_size ) < 0 )
        {
            MDP_ERROR("Calc disp dst2 memory array size error.\n" );
            return -1;
        }

        disp_dst2_buffer.RegisterLoopMemory( MEM_TYPE_OUTPUT,
                                            p_parameter->dst2_color_format,
                                            p_parameter->dst2_img_size,
                                            p_parameter->dst2_img_roi,
                                            p_parameter->dst2_rotate_angle /*rotate*/);
    }
    display_from_memory_param_.en_dst_port2=              p_parameter->en_dst_port2;
    display_from_memory_param_.dst2_buffer_count=         1;//p_parameter->dst2_buffer_count;//Single trigger
    display_from_memory_param_.dst2_color_format=         p_parameter->dst2_color_format;
    display_from_memory_param_.dst2_img_size=             p_parameter->dst2_img_size;
    display_from_memory_param_.dst2_img_roi=              p_parameter->dst2_img_roi;
    display_from_memory_param_.dst2_yuv_img_addr=         p_parameter->dst2_yuv_img_addr;
    display_from_memory_param_.dst2_rotate_angle=         p_parameter->dst2_rotate_angle;


    /*resz coeff*/
    display_from_memory_param_.resz_coeff=                p_parameter->resz_coeff;

    //Todo : do this when working buffer dequeue
    /*

    ret = m_MapPathDisplayFromMemory.Config( &display_from_memory_param_ );
    if( ret != 0 )
    {
        MDP_ERROR("Path 2 config error!\n\r");
        return ret;
    }
    */

    return ret;


}



int MdpPipeCameraPreview::_ConfigZeroShutterPath( struct MdpPipeCameraPreviewParameter* p_parameter )
{
    int         ret = 0;
    struct      MdpPathCameraPreviewZeroShutterParameter    camera_preview_path_param;

    /*-----------------------------------------------------------------------------
      Parametr Sanity Test
      -----------------------------------------------------------------------------*/


    /***Parameter Convertion***/
    /*-----------------------------------------------------------------------------
      camera_preview_path_param
      -----------------------------------------------------------------------------*/
    /*pseduo source*/
    camera_preview_path_param.pseudo_src_enable         =    p_parameter->pseudo_src_enable;
    camera_preview_path_param.pseudo_src_color_format   =    p_parameter->pseudo_src_color_format;
    camera_preview_path_param.pseudo_src_yuv_img_addr   =    p_parameter->pseudo_src_yuv_img_addr;

    /*Debug*/
    camera_preview_path_param.debug_preview_single_frame_enable = p_parameter->debug_preview_single_frame_enable;


    /*source*/
    camera_preview_path_param.src_img_size              =    p_parameter->src_img_size;
    camera_preview_path_param.src_img_roi               =    p_parameter->src_img_roi;


    /*-----------------------------------------------------------------------------
        display_from_memory_param
      -----------------------------------------------------------------------------*/
    /*output 0*/
    camera_preview_path_param.en_dst_port0=              p_parameter->en_dst_port0;
    camera_preview_path_param.dst0_buffer_count=         p_parameter->dst0_buffer_count;
    camera_preview_path_param.dst0_color_format=         p_parameter->dst0_color_format;
    camera_preview_path_param.dst0_img_size=             p_parameter->dst0_img_size;
    camera_preview_path_param.dst0_img_roi=              p_parameter->dst0_img_roi;
    camera_preview_path_param.dst0_yuv_img_addr=         p_parameter->dst0_yuv_img_addr;
    camera_preview_path_param.dst0_rotate_angle=         p_parameter->dst0_rotate_angle;

    /*output 1*/
    camera_preview_path_param.en_dst_port1=              p_parameter->en_dst_port1;
    camera_preview_path_param.dst1_buffer_count=         p_parameter->dst1_buffer_count;
    camera_preview_path_param.dst1_color_format=         p_parameter->dst1_color_format;
    camera_preview_path_param.dst1_img_size=             p_parameter->dst1_img_size;
    camera_preview_path_param.dst1_img_roi=              p_parameter->dst1_img_roi;
    camera_preview_path_param.dst1_yuv_img_addr=         p_parameter->dst1_yuv_img_addr;
    camera_preview_path_param.dst1_rotate_angle=         p_parameter->dst1_rotate_angle;

    /*output 2*/
    camera_preview_path_param.en_dst_port2=              p_parameter->en_dst_port2;
    camera_preview_path_param.dst2_buffer_count=         p_parameter->dst2_buffer_count;
    camera_preview_path_param.dst2_color_format=         p_parameter->dst2_color_format;
    camera_preview_path_param.dst2_img_size=             p_parameter->dst2_img_size;
    camera_preview_path_param.dst2_img_roi=              p_parameter->dst2_img_roi;
    camera_preview_path_param.dst2_yuv_img_addr=         p_parameter->dst2_yuv_img_addr;
    camera_preview_path_param.dst2_rotate_angle=         p_parameter->dst2_rotate_angle;

    /*resz coeff*/
    camera_preview_path_param.resz_coeff=                p_parameter->resz_coeff;


    ret = m_MapPathCameraPreviewZeroShutter.Config( &camera_preview_path_param );
    if( ret != 0 )
    {
        MDP_ERROR("Path config error!\n\r");
    }


    return ret;


}


int MdpPipeCameraPreview::_ConfigN3dPath( struct MdpPipeCameraPreviewParameter* p_parameter )
{
    MdpPathCameraPreviewToMemoryParameter   tg1_param;
    MdpPathCamera2PreviewToMemoryParameter  tg2_param;
    /*struct  MdpPathN3d2ndPassParameter  n3d2ndpass_param_;*/

    unsigned long tg1_x_off,tg2_x_off;
    unsigned long dummy_buffer_total_size;






    /*-----------------------------------------------------------------------------*/
    /*TG1                                                                          */
    /*-----------------------------------------------------------------------------*/

        /*.............................................................................
          Parametr Sanity Test
          .............................................................................*/
    if( p_parameter->pseudo_src_enable == 1 )      //Use RDMA0 to take place of camera sensor as pseudo source
    {
        MDP_ERROR("N3D Preview Path do not support pseudo src\n");
        return -1;
    }


    if( p_parameter->debug_preview_single_frame_enable == 1 )      //Use RDMA0 to take place of camera sensor as pseudo source
    {
        MDP_ERROR("N3D Preview Path do not support debug single frame\n");
        return -1;
    }

    if( p_parameter->en_zero_shutter_path != 0 )      //Use RDMA0 to take place of camera sensor as pseudo source
    {
        MDP_ERROR("N3D Preview Path do not support zsd path\n");
        return -1;
    }
         /*.............................................................................
          Parameter Mapping
          .............................................................................*/
    tg1_param.pseudo_src_enable = 0;      //Use RDMA0 to take place of camera sensor as pseudo source
        tg1_param.pseudo_src_buffer_count = 0;
        tg1_param.pseudo_src_color_format = (MdpColorFormat)0;
        tg1_param.pseudo_src_yuv_img_addr.y = 0;


    /*Debug*/
    tg1_param.debug_preview_single_frame_enable = 0;   /*Debug Purpose: enable to disable continous frame when preview*/

    /*HW Trigger*/
    tg1_param.b_hw_trigger_out = 0;   /*hw trigger RDMA0*/

    /*//last preview frame for zsd */
    tg1_param.b_en_zsd_path = 0; /*0: not enable 1,2: enable*/

    /*SRC*/
    tg1_param.src_img_size  = p_parameter->src_img_size;
    tg1_param.src_img_roi   = p_parameter->src_img_roi;

    /*DST*/
    tg1_param.dst_buffer_count  = p_parameter->working_tg1_buffer_count;
    tg1_param.dst_color_format  = p_parameter->working_tg1_color_format;
    tg1_param.dst_img_size      = p_parameter->working_tg1_img_size;       /*Before rotate dimension*//*Currently, "dst roi"  should be equal to "dst size"*/
    tg1_param.dst_img_roi       = p_parameter->working_tg1_img_roi;        /*Before rotate dimension*//*dst_img_roi is not used*/
    tg1_param.dst_yuv_img_addr  = p_parameter->working_tg1_buffer_addr;
    tg1_param.dst_rotate_angle  = p_parameter->working_tg1_rotate_angle;
    //tg1_param.dst_yuv_img_addr_last_preview_; /*N/A*/


    /*resizer coeff*/
    tg1_param.resz_coeff = p_parameter->resz_coeff;





    /*-----------------------------------------------------------------------------*/
    /*TG2                                                                          */
    /*-----------------------------------------------------------------------------*/

        /*.............................................................................
          Parametr Sanity Test
          .............................................................................*/
          /*N/A*/

        /*.............................................................................
         Parameter Mapping
         .............................................................................*/
    /*pseduo source*/
    tg2_param.pseudo_src_enable = 0;      //Use RDMA0 to take place of camera sensor as pseudo source
        tg2_param.pseudo_src_buffer_count = 0;
        tg2_param.pseudo_src_color_format = (MdpColorFormat)0;
        tg2_param.pseudo_src_yuv_img_addr.y = 0;

    /*Debug*/
    tg2_param.debug_preview_single_frame_enable = 0;   /*Debug Purpose: enable to disable continous frame when preview*/

    /*HW Trigger*/
    tg2_param.b_hw_trigger_out = 0;   /*hw trigger RDMA0*/

    /*SRC*/
    tg2_param.src_img_size  = p_parameter->src_tg2_img_size;
    tg2_param.src_img_roi   = p_parameter->src_tg2_img_roi;

    /*DST*/
    tg2_param.dst_buffer_count  = p_parameter->working_tg2_buffer_count;
    tg2_param.dst_color_format  = p_parameter->working_tg2_color_format;
    tg2_param.dst_img_size      = p_parameter->working_tg2_img_size;       /*Currently, "dst roi"  should be equal to "dst size"*/
    tg2_param.dst_img_roi       = p_parameter->working_tg2_img_roi;        /*dst_img_roi is not used*/
    tg2_param.dst_yuv_img_addr  = p_parameter->working_tg2_buffer_addr;
    tg2_param.dst_rotate_angle  = p_parameter->working_tg2_rotate_angle;


    /*resizer coeff*/
    tg2_param.resz_coeff        = p_parameter->resz_coeff;








    /*-----------------------------------------------------------------------------*/
    /*N3D 2nd Pass                                                                          */
    /*-----------------------------------------------------------------------------*/
    if( p_parameter->n3d_working_buff_layout == 0 ){
        tg1_x_off = 0;
        tg2_x_off = p_parameter->dst0_img_roi.w/2;
    }
    else {
        tg1_x_off = p_parameter->dst0_img_roi.w/2;
        tg2_x_off = 0;
    }
        /*.............................................................................
         Tg1 Parameter Mapping
         .............................................................................*/
    /*Source*/
    n3d2ndpass_tg1_param_.src_buffer_count  =   1; //For single shot
    n3d2ndpass_tg1_param_.src_color_format  =   p_parameter->working_tg1_color_format;
    n3d2ndpass_tg1_param_.src_img_size      =   p_parameter->working_tg1_img_size;
    n3d2ndpass_tg1_param_.src_img_roi       =   MdpRect(0,0,0,0); //!!!Input Parameter when trigger!!!
    n3d2ndpass_tg1_param_.src_yuv_img_addr.y=   0;  //!!!Input Parameter when trigger!!!


    /*Output port*/
    n3d2ndpass_tg1_param_.dst_buffer_count  =   1;  //For single shot
    n3d2ndpass_tg1_param_.dst_yuv_img_addr.y=   0;  //!!!Input Parameter when trigger!!!
    n3d2ndpass_tg1_param_.dst_img_size      =   p_parameter->dst0_img_size; //stride
    n3d2ndpass_tg1_param_.dst_img_roi       =   MdpRect( tg1_x_off, 0, p_parameter->dst0_img_roi.w/2, p_parameter->dst0_img_roi.h);//if roi=0, default value is (0,0,size.w, size.h)
    n3d2ndpass_tg1_param_.dst_color_format  =   p_parameter->dst0_color_format;
    n3d2ndpass_tg1_param_.dst_flip  = (p_parameter->dst0_rotate_angle >> 2)&0x1;
    n3d2ndpass_tg1_param_.dst_rotate=  p_parameter->dst0_rotate_angle&0x3;

    /*resizer coeff*/
    n3d2ndpass_tg1_param_.resz_coeff= p_parameter->resz_coeff;


        /*.............................................................................
         Tg2 Parameter Mapping
         .............................................................................*/
    /*Source*/
    n3d2ndpass_tg2_param_.src_buffer_count  =   1; //For single shot
    n3d2ndpass_tg2_param_.src_color_format  =   p_parameter->working_tg1_color_format;
    n3d2ndpass_tg2_param_.src_img_size      =   p_parameter->working_tg1_img_size;
    n3d2ndpass_tg2_param_.src_img_roi       =   MdpRect(0,0,0,0); //!!!Input Parameter when trigger!!!
    n3d2ndpass_tg2_param_.src_yuv_img_addr.y=   0;  //!!!Input Parameter when trigger!!!


    /*Output port*/
    n3d2ndpass_tg2_param_.dst_buffer_count  =   1;  //For single shot
    n3d2ndpass_tg2_param_.dst_yuv_img_addr.y=   0;  //!!!Input Parameter when trigger!!!
    n3d2ndpass_tg2_param_.dst_img_size      =   p_parameter->dst0_img_size; //stride
    n3d2ndpass_tg2_param_.dst_img_roi       =   MdpRect( tg2_x_off, 0, p_parameter->dst0_img_roi.w/2, p_parameter->dst0_img_roi.h);//if roi=0, default value is (0,0,size.w, size.h)
    n3d2ndpass_tg2_param_.dst_color_format  =   p_parameter->dst0_color_format;
    n3d2ndpass_tg2_param_.dst_flip  = (p_parameter->dst0_rotate_angle >> 2)&0x1;
    n3d2ndpass_tg2_param_.dst_rotate=  p_parameter->dst0_rotate_angle&0x3;

    /*resizer coeff*/
    n3d2ndpass_tg2_param_.resz_coeff= p_parameter->resz_coeff;



    /*-----------------------------------------------------------------------------*/
    /*Buffer management                                                            */
    /*-----------------------------------------------------------------------------*/
    /*Tg1 working buffer: source 1*/
    n3d_tg1_work_src_buffer.Reset( 0 );
    n3d_tg1_work_src_buffer.SetName("MID_RGB_ROT0");
    n3d_tg1_work_src_buffer.enable = 1;
    n3d_tg1_work_src_buffer.buffer_count = p_parameter->working_tg1_buffer_count;
    n3d_tg1_work_src_buffer.buffer[0]    = p_parameter->working_tg1_buffer_addr;
    if( MdpDrv_CalImgBufferArraySizeAndFillIn(
                        p_parameter->working_tg1_color_format,
                        n3d_tg1_work_src_buffer.buffer,/*will be modified*/
                        p_parameter->working_tg1_buffer_count,
                        p_parameter->working_tg1_img_size,
                        MdpRect( p_parameter->working_tg1_img_size ),//p_parameter->dst_img_roi, /*Should not use roi to calculate size,the offset will vary*/
                        0, /*rotate*/
                        &dummy_buffer_total_size ) < 0 )
    {
        MDP_ERROR("Calc n3d_tg1_work_src_buffer memory array size error.\n" );
        return -1;
    }


    n3d_tg1_work_src_buffer.RegisterLoopMemory( MEM_TYPE_INPUT,
                                                p_parameter->working_tg1_color_format,
                                                p_parameter->working_tg1_img_size,
                                                MdpRect( p_parameter->working_tg1_img_size ),
                                                0 /*rotate*/);


    /*Tg2 working buffer: source 2*/
    n3d_tg2_work_src_buffer.Reset( 1 );
    n3d_tg2_work_src_buffer.SetName("MID_VDO_ROT0");
    n3d_tg2_work_src_buffer.enable = 1;
    n3d_tg2_work_src_buffer.buffer_count = p_parameter->working_tg2_buffer_count;
    n3d_tg2_work_src_buffer.buffer[0]    = p_parameter->working_tg2_buffer_addr;
    if( MdpDrv_CalImgBufferArraySizeAndFillIn(
                        p_parameter->working_tg1_color_format,
                        n3d_tg2_work_src_buffer.buffer,/*will be modified*/
                        p_parameter->working_tg2_buffer_count,
                        p_parameter->working_tg2_img_size,
                        MdpRect( p_parameter->working_tg2_img_size ),//p_parameter->dst_img_roi, /*Should not use roi to calculate size,the offset will vary*/
                        0, /*rotate*/
                        &dummy_buffer_total_size ) < 0 )
    {
        MDP_ERROR("Calc n3d_tg2_work_src_buffer memory array size error.\n" );
        return -1;
    }


    n3d_tg2_work_src_buffer.RegisterLoopMemory( MEM_TYPE_INPUT,
                                                p_parameter->working_tg2_color_format,
                                                p_parameter->working_tg2_img_size,
                                                MdpRect( p_parameter->working_tg2_img_size ),
                                                0 /*rotate*/);


    /*outout buffer*/
    n3d_dst_buffer.Reset( 2 );
    n3d_dst_buffer.SetName("MID_VDO_ROT1");
    n3d_dst_buffer.enable = 1;
    n3d_dst_buffer.buffer_count = p_parameter->dst0_buffer_count;
    n3d_dst_buffer.buffer[0]    = p_parameter->dst0_yuv_img_addr;
    if( MdpDrv_CalImgBufferArraySizeAndFillIn(
                        p_parameter->dst0_color_format,
                        n3d_dst_buffer.buffer,/*will be modified*/
                        p_parameter->dst0_buffer_count,
                        p_parameter->dst0_img_size,
                        MdpRect( p_parameter->dst0_img_size ),//p_parameter->dst_img_roi, /*Should not use roi to calculate size,the offset will vary*/
                        p_parameter->dst0_rotate_angle, /*rotate*/
                        &dummy_buffer_total_size ) < 0 )
    {
        MDP_ERROR("Calc n3d_dst_buffer memory array size error.\n" );
        return -1;
    }


    n3d_dst_buffer.RegisterLoopMemory(  MEM_TYPE_OUTPUT,
                                        p_parameter->dst0_color_format,
                                        p_parameter->dst0_img_size,
                                        MdpRect( p_parameter->dst0_img_size ),
                                        p_parameter->dst0_rotate_angle /*rotate*/);


    /*-----------------------------------------------------------------------------*/
    /*Config HW                                                                    */
    /*-----------------------------------------------------------------------------*/
    if( m_MdpPathN3d1stTg1.Config( &tg1_param ) < 0 )
    {
        MDP_ERROR("m_MdpPathN3d1stTg1.Config error\n");
        return -1;
    }


    if( m_MdpPathN3d1stTg2.Config( &tg2_param ) < 0 )
    {
        MDP_ERROR("m_MdpPathN3d1stTg2.Config error\n");
        return -1;
    }


    return 0;
}


int MdpPipeCameraPreview::_ConfigOnePassPath( struct MdpPipeCameraPreviewParameter* p_parameter )
{
    int         ret = 0;
    struct      MdpPathCameraPreviewToMemoryParameter    camera_preview_path_param;


    /*-----------------------------------------------------------------------------
      Parametr Sanity Test
      -----------------------------------------------------------------------------*/
      if( ( p_parameter->src_img_roi.w < p_parameter->dst_img_size.w ) || ( p_parameter->src_img_roi.h < p_parameter->dst_img_size.h ) )
      {
        MDP_WARNING("Direct Link CRZ do upscale! src w/h=(%d/%d), dst w/h=(%d/%d)\n",
            (int)p_parameter->src_img_roi.w, (int)p_parameter->src_img_roi.h,
            (int)p_parameter->dst_img_size.w, (int)p_parameter->dst_img_size.h );
      }



    /***Parameter Convertion***/
    /*-----------------------------------------------------------------------------
      camera_preview_path_param
      -----------------------------------------------------------------------------*/
    /*pseduo source*/
    camera_preview_path_param.pseudo_src_enable         =    p_parameter->pseudo_src_enable;
    camera_preview_path_param.pseudo_src_buffer_count   =    p_parameter->dst_buffer_count; /*Same as dst buffer count*/
    camera_preview_path_param.pseudo_src_color_format   =    p_parameter->pseudo_src_color_format;
    camera_preview_path_param.pseudo_src_yuv_img_addr   =    p_parameter->pseudo_src_yuv_img_addr;

    /*Debug*/
    camera_preview_path_param.debug_preview_single_frame_enable = p_parameter->debug_preview_single_frame_enable;

    /*HW Trigger*/
    camera_preview_path_param.b_hw_trigger_out = 0;     /*sw trigger*/

    /*source*/
    camera_preview_path_param.src_img_size              =    p_parameter->src_img_size;
    camera_preview_path_param.src_img_roi               =    p_parameter->src_img_roi;

    /*Working buffer*/
    camera_preview_path_param.dst_buffer_count          =    p_parameter->dst_buffer_count;
    camera_preview_path_param.dst_color_format          =    p_parameter->dst_color_format;
    camera_preview_path_param.dst_img_size              =    p_parameter->dst_img_size;
    camera_preview_path_param.dst_img_roi               =    p_parameter->dst_img_roi; //dst_img_roi is not use in this path
    camera_preview_path_param.dst_yuv_img_addr          =    p_parameter->dst_yuv_img_addr;
    camera_preview_path_param.dst_rotate_angle          =    p_parameter->dst_rotate_angle;

    /*resz coeff*/
    camera_preview_path_param.resz_coeff                =    p_parameter->resz_coeff;

    /*last preview for ZSD*/
    camera_preview_path_param.dst_yuv_img_addr_last_preview_ = p_parameter->dst_yuv_img_addr_last_preview_;
    camera_preview_path_param.b_en_zsd_path                  = p_parameter->en_zero_shutter_path;



    /*-----------------------------------------------------------------------------
      Misc Parameter
      -----------------------------------------------------------------------------*/
    {
        MdpDrvColorInfo_t   color_info;

        //Decide use me_rgbrot0 or me_vdorot0
        MdpDrvColorFormatInfoGet( camera_preview_path_param.dst_color_format , &color_info );

        b_is_generic_yuv_in_one_pass_preview_path_ = color_info.b_is_generic_yuv;

    }


    /*---Config----*/
    ret = m_MdpPathCameraPreviewToMemory.Config( &camera_preview_path_param );
    if( ret != 0 )
    {
        MDP_ERROR("Path 1 config error!\n\r");
        return ret;
    }


    return ret;


}




int MdpPipeCameraPreview::Start( void* pParam )
{
    int    ret = 0;

    /*one pass preview path (for 1080p support)*/
    if( en_one_pass_preview_path_ == 1 )
    {
        ret = m_MdpPathCameraPreviewToMemory.Start( pParam );
        if( ret != 0 )
        {
            MDP_ERROR("MdpPathCameraPreviewToMemory start error!(One pass preview path for 1080p)\n");
            return ret;
        }
    }
    /*Native 3D preview path*/
    else if( en_n3d_preview_path_ == 1 )
    {
        ret = m_MdpPathN3d1stTg1.Start( pParam );
        if( ret != 0 )
        {
            MDP_ERROR("m_MdpPathN3d1stTg1 start error!\n\r");
            return ret;
        }

        ret = m_MdpPathN3d1stTg2.Start( pParam );
        if( ret != 0 )
        {
            MDP_ERROR("m_MdpPathN3d1stTg2 start error!\n\r");
            return ret;
        }
    }
    /*Normal Path*/
    else if( en_zero_shutter_path_ == 0 || en_zero_shutter_path_ == 2)
    {
        /*HW Trigger*/
        if( en_sw_trigger_ == 0 )
        {
            //Start Path 2 first
            ret = m_MapPathDisplayFromMemory.Start( pParam );
            if( ret != 0 )
            {
                MDP_ERROR("Path 2 start error!\n\r");
                return ret;
            }


            ret = m_MdpPathCameraPreviewToMemory.Start( pParam );
            if( ret != 0 )
            {
                MDP_ERROR("Path 1 start error!\n\r");
                return ret;
            }
        }
        else
        /*SW Trigger*/
        {
            // last preview frame for zsd
            if (en_zero_shutter_path_ == 2) {
                m_MdpPathCameraPreviewToMemory.AdpateLastPreviewBuffer();
            }

            ret = m_MdpPathCameraPreviewToMemory.Start( pParam );
            if( ret != 0 )
            {
                MDP_ERROR("MdpPathCameraPreviewToMemory start error!\n\r");
                return ret;
            }

        }
    }
    /*Zero Shutter Path*/
    else
    {
        ret = m_MapPathCameraPreviewZeroShutter.Start( pParam );
        if( ret != 0 )
        {
            MDP_ERROR("start error!\n\r");
            return ret;
        }

    }



    return ret;


}

int MdpPipeCameraPreview::WaitBusy( unsigned long rsc_to_wait_mask  )
{
    int    ret = 0;

    /*one pass preview path (for 1080p support)*/
    if( en_one_pass_preview_path_ == 1 )
    {
        if( m_MdpPathCameraPreviewToMemory.WaitBusy( rsc_to_wait_mask  ) != 0 )
        {
            MDP_ERROR("One pass preview path WaitBusy error!\n\r");
            ret = -1;
        }
    }
    /*Native 3D preview path*/
    else if( en_n3d_preview_path_ == 1 )
    {
        if( m_MdpPathN3d1stTg1.WaitBusy( rsc_to_wait_mask  ) != 0 )
        {
            MDP_ERROR("Tg1 WaitBusy error!\n\r");
            ret = -1;
        }

        if( m_MdpPathN3d1stTg2.WaitBusy( rsc_to_wait_mask  ) != 0 )
        {
            MDP_ERROR("Tg2 WaitBusy error!\n\r");
            ret = -1;
        }
    }
    /*Normal Path*/
    else if( en_zero_shutter_path_ == 0 || en_zero_shutter_path_ == 2)
    {
        /*HW Trigger*/
        if( en_sw_trigger_ == 0 )
        {
            if( m_MdpPathCameraPreviewToMemory.WaitBusy( rsc_to_wait_mask  ) != 0 )
            {
                MDP_ERROR("Path 1 WaitBusy error!\n\r");
                ret = -1;
            }

            if( m_MapPathDisplayFromMemory.WaitBusy( rsc_to_wait_mask  ) != 0 )
            {
                MDP_ERROR("Path 2 WaitBusy error!\n\r");
                ret = -1;
            }
        } else
        /*SW Trigger*/
        {
            if( m_MdpPathCameraPreviewToMemory.WaitBusy( rsc_to_wait_mask  ) != 0 )
            {
                MDP_ERROR("Path 1 WaitBusy error!\n\r");
                ret = -1;
            }
        }
    }
    /*Zero Shutter Path*/
    else
    {
        if( m_MapPathCameraPreviewZeroShutter.WaitBusy( rsc_to_wait_mask  ) != 0 )
        {
            MDP_ERROR("Path 1 WaitBusy error!\n\r");
            ret = -1;
        }
    }


    return ret;

}

int MdpPipeCameraPreview::QueueGetFreeListOrWaitBusy( unsigned long mdp_id,unsigned long *p_StartIndex, unsigned long *p_Count  )
{
    int    ret = 0;

    //For calculate fps info
    _TickDeQueueFpsInfo( mdp_id );

    /*one pass preview path (for 1080p support)*/
    if( en_one_pass_preview_path_ == 1 )
    {
        if( m_MdpPathCameraPreviewToMemory.QueueGetFreeListOrWaitBusy( mdp_id,p_StartIndex,p_Count  ) < 0 ){
            ret = -1;
        }
        return ret;
    }
    else if( en_n3d_preview_path_ == 1 )
    {
        if( _QueueGetFreeListOrWaitBusyN3dPath( mdp_id, p_StartIndex, p_Count  ) < 0 )
                return -1;
    }
    /*Normal Path*/
    else if( en_zero_shutter_path_ == 0 || en_zero_shutter_path_ == 2)
    {
        /* HW Trigger Path*/
        if( en_sw_trigger_ == 0 )
        {
            unsigned long rsc_id_mask;

            //---Path 1-1
            rsc_id_mask = m_MdpPathCameraPreviewToMemory.ResourceIdMaskGet();
            if( rsc_id_mask & mdp_id )
            {
                if( m_MdpPathCameraPreviewToMemory.QueueGetFreeListOrWaitBusy( mdp_id,p_StartIndex,p_Count  ) < 0 )
                {
                    ret = -1;

                    //Dump both path Register
                    MDP_PRINTF("DumpRegister - MdpPipeCameraPreview (Both Path)\n" );
                    m_MdpPathCameraPreviewToMemory.DumpRegister( NULL );
                    m_MapPathDisplayFromMemory.DumpRegister(NULL );

                }
                return ret;
            }

            //---Path 1-2
            rsc_id_mask = m_MapPathDisplayFromMemory.ResourceIdMaskGet();
            if( rsc_id_mask & mdp_id )
            {
                if( m_MapPathDisplayFromMemory.QueueGetFreeListOrWaitBusy( mdp_id,p_StartIndex,p_Count  ) < 0 )
                {
                    ret = -1;

                    //Dump both path Register
                    MDP_PRINTF("DumpRegister - MdpPipeCameraPreview (Both Path)\n" );
                    m_MdpPathCameraPreviewToMemory.DumpRegister( NULL );
                    m_MapPathDisplayFromMemory.DumpRegister(NULL );
                }
                return ret;
            }
        }else
        /* SW Trigger Path*/
        {
            if( _QueueGetFreeListOrWaitBusySwTrigger( mdp_id, p_StartIndex, p_Count  ) < 0 )
                return -1;

        }

    }
    /*Zero Shutter Path*/
    else
    {
        if( m_MapPathCameraPreviewZeroShutter.QueueGetFreeListOrWaitBusy( mdp_id,p_StartIndex,p_Count  ) < 0 )
        {
            ret = -1;
        }
        return ret;
    }


    return ret;

}


/*SW Trigger*/
int MdpPipeCameraPreview::_QueueGetFreeListOrWaitBusySwTrigger( unsigned long mdp_id,unsigned long *p_StartIndex, unsigned long *p_Count  )
{
    static unsigned long rgb0_startindex = 0;
    static unsigned long rgb0_count = 0;

    unsigned long   work_buffer_index;
    unsigned long   work_buffer_count = disp_src_buffer.buffer_count;
    unsigned long   i = 0;

    switch( mdp_id )
    {
    case MID_RGB_ROT0://path 1 out put buffer, descriptor
        {

            /*1.Dequeue from 1st path, get free source buffer*/
            /*.............................................................................*/
            if( m_MdpPathCameraPreviewToMemory.QueueGetFreeListOrWaitBusy( mdp_id,p_StartIndex,p_Count  ) < 0 )
            {
                //m_MdpPathCameraPreviewToMemory.DumpRegister( NULL );
                return -1;
            }

            if( *p_Count > 1 ){
                //MDP_WARNING("Working buffer get %d frame (>1), skip %d frame\n", (int)(*p_Count), (int)(*p_Count-1) );
                MDP_WARNING("Working buffer get %d frame (>1)\n", (int)(*p_Count) );
            }

            //for( i = 0; i < *p_Count; i++ ) //Do more than 1 times
            {

            #if defined(MDP_FLAG_PROFILING)
                MdpDrv_Watch _MdpDrvWatch;
                static unsigned long tt0,tt1,tt2,tt3;//total time;
                static unsigned long fc0,fc1,fc2,fc3;//frame count;
                static unsigned long avt0,avt1,avt2,avt3;//avg_time_elapse;
            #endif
                work_buffer_index = ( *p_StartIndex + i )%work_buffer_count;

                MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_PIPE );

                //MDP_INFO_QUEUE("SW trigger working buffer index: %d/%d\n", work_buffer_index,work_buffer_count)
                _SwTriggerWorkingByIndex( work_buffer_index );

                MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_PIPE, "Preview 2nd Path", &tt0, &fc0, &avt0, 30 );
            }


            /*6. Remember index/ count for error check in mdp_hal, for compatible with hw triggerflow*/
            /*.............................................................................*/
            rgb0_startindex = *p_StartIndex;
            rgb0_count      = *p_Count;

        }
        break;
    case MID_RGB_ROT0_EX://path 1 out put buffer, descriptor
            {

                /*1.Dequeue from 1st path, get free source buffer*/
                /*.............................................................................*/
                if( m_MdpPathCameraPreviewToMemory.QueueGetFreeListOrWaitBusy( mdp_id,p_StartIndex,p_Count  ) < 0 )
                {
                    //m_MdpPathCameraPreviewToMemory.DumpRegister( NULL );
                    return -1;
                }

                if( *p_Count > 1 ){
                    //MDP_WARNING("Working buffer get %d frame (>1), skip %d frame\n", (int)(*p_Count), (int)(*p_Count-1) );
                    MDP_WARNING("Working buffer get %d frame (>1)\n", (int)(*p_Count) );
                }
#if 0
                //for( i = 0; i < *p_Count; i++ ) //Do more than 1 times
                {

        #if defined(MDP_FLAG_PROFILING)
                    MdpDrv_Watch _MdpDrvWatch;
                    static unsigned long tt0,tt1,tt2,tt3;//total time;
                    static unsigned long fc0,fc1,fc2,fc3;//frame count;
                    static unsigned long avt0,avt1,avt2,avt3;//avg_time_elapse;
        #endif
                    work_buffer_index = ( *p_StartIndex + i )%work_buffer_count;

                    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_PIPE );

                    //MDP_INFO_QUEUE("SW trigger working buffer index: %d/%d\n", work_buffer_index,work_buffer_count)
                    _SwTriggerWorkingByIndex( work_buffer_index );

                    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_PIPE, "Preview 2nd Path", &tt0, &fc0, &avt0, 30 );
                }


                /*6. Remember index/ count for error check in mdp_hal, for compatible with hw triggerflow*/
                /*.............................................................................*/
                rgb0_startindex = *p_StartIndex;
                rgb0_count      = *p_Count;
#endif
            }
            break;

    case MID_R_DMA0://Associate with working buffer in HW trigger mode
        *p_StartIndex = rgb0_startindex;
        *p_Count = rgb0_count;
        break;

    case MID_VDO_ROT0://dst 0
        disp_dst0_buffer.QueryAvailableReadBuffer( p_StartIndex, p_Count);
        MDP_INFO_QUEUE("[QUEUE]Dequeue <MID_VDO_ROT0> ROT get free list. Start = %lu Count = %lu\n",*p_StartIndex, *p_Count);
        break;
    case MID_VDO_ROT1://dst 1
        disp_dst1_buffer.QueryAvailableReadBuffer( p_StartIndex, p_Count);
        MDP_INFO_QUEUE("[QUEUE]Dequeue <MID_VDO_ROT1> ROT get free list. Start = %lu Count = %lu\n",*p_StartIndex, *p_Count);
        break;
    case MID_RGB_ROT1://dst 2
        disp_dst2_buffer.QueryAvailableReadBuffer( p_StartIndex, p_Count);
        MDP_INFO_QUEUE("[QUEUE]Dequeue <MID_RGB_ROT1> ROT get free list. Start = %lu Count = %lu\n",*p_StartIndex, *p_Count);
        break;

    default:
        MDP_ERROR("invalid mdp id= 0x%08X in camera preview pipe\n", (unsigned int)mdp_id );
        return -1;
        break;
    }

    return 0;


}



int MdpPipeCameraPreview::_SwTriggerWorkingByIndex( unsigned long index  )
{

    MdpYuvAddr      work_buffer_addr;
    int             b_eis_en;
    MdpRect         rough_crop_roi;
    MdpRect         fine_crop_roi;
    unsigned long   eis_delta_x = 0;
    unsigned long   eis_delta_y = 0;
    unsigned long   eis_x = 0;
    unsigned long   eis_y = 0;
    stTimeStamp     rgb0_timestamp;
    unsigned long   rgb0_ts_sec,rgb0_ts_us;

    unsigned long   b_trigger_last_frame = 0;



    if (index == 0xFF && en_zero_shutter_path_ == 2)
    {
        index = 0;
        b_trigger_last_frame = 1;
    }

    if( index >= (unsigned long)disp_src_buffer.buffer_count )
    {
        MDP_ERROR("index overflow.( %d >= %d )\n", (int)index, (int)disp_src_buffer.buffer_count );
        return -1;
    }

    /*Get Timestamp of RGB0,use as overall timestamp*/
    rgb0_timestamp.mdp_id = MID_RGB_ROT0;

    if( MdpDrv_GetTimeStamp( &rgb0_timestamp ) < 0 ){
        MDP_ERROR("Get time stamp error\n");
        return -1;
    }

    rgb0_ts_sec = rgb0_timestamp.sec[ index ];
    rgb0_ts_us  = rgb0_timestamp.usec[ index ];



    #if 0
    /*For Debug timestamp issue, add log here*/
    {
        MDP_INFO_PERSIST("[RGB_ROT0][TS] Sw Trig Index = %lu, sec = %lu, us = %lu\n",
            index, rgb0_ts_sec, rgb0_ts_us);


        {
            unsigned long sec,usec;

            MdpDrv_CpuClockGet( &sec, &usec );

            MDP_INFO_PERSIST("[CPUCLK][TS] sec = %lu, us = %lu\n",  sec, usec);


        }
    }
    #endif




    //work_buffer_addr = disp_src_buffer.buffer[ ((*p_StartIndex) + (*p_Count) - 1)%disp_src_buffer.buffer_count ]; //Choose the latest updated buffer
    work_buffer_addr = disp_src_buffer.buffer[ index ];
    if (b_trigger_last_frame == 1 && en_zero_shutter_path_ == 2)
    {
        work_buffer_addr = disp_src_buffer_last_preview_addr;
    }

    /*2.Get EIS Information*/
    /*.............................................................................*/
#if defined(MDP_FLAG_USE_RDMA_TO_CROP)
    if( CB_EIS_INFO_GET_ != NULL )
    {
        CB_EIS_INFO_GET_( &eis_x , &eis_y );

        MDP_INFO_PERSIST("EIS function out: 0x%04X 0x%04X\n", (unsigned int)eis_x, (unsigned int)eis_y );
        if( path_1_rotate_angle_ & 0x1 ) //90 & 270
        {
            display_from_memory_param_.b_eis = 0;//1;/*Deside to disable EIS floating part forever*/
            display_from_memory_param_.eis_float_x = eis_y & 0xFF;
            display_from_memory_param_.eis_float_y = eis_x & 0xFF;
            display_from_memory_param_.src_img_roi.x = zoom_crop_region_2nd_path_.x + ( ( eis_y & 0xFF00 ) >> 8 );
            display_from_memory_param_.src_img_roi.y = zoom_crop_region_2nd_path_.y + ( ( eis_x & 0xFF00 ) >> 8 );
            display_from_memory_param_.src_img_roi.w = zoom_crop_region_2nd_path_.w;
            display_from_memory_param_.src_img_roi.h = zoom_crop_region_2nd_path_.h;

        } else
        {
            display_from_memory_param_.b_eis = 0;//1;/*Deside to disable EIS floating part forever*/
            display_from_memory_param_.eis_float_x = eis_x & 0xFF;
            display_from_memory_param_.eis_float_y = eis_y & 0xFF;
            display_from_memory_param_.src_img_roi.x = zoom_crop_region_2nd_path_.x + ( ( eis_x & 0xFF00 ) >> 8 );
            display_from_memory_param_.src_img_roi.y = zoom_crop_region_2nd_path_.y + ( ( eis_y & 0xFF00 ) >> 8 );
            display_from_memory_param_.src_img_roi.w = zoom_crop_region_2nd_path_.w;
            display_from_memory_param_.src_img_roi.h = zoom_crop_region_2nd_path_.h;
        }
    }
    else
    {
        display_from_memory_param_.b_eis = 0;

        display_from_memory_param_.src_img_roi = zoom_crop_region_2nd_path_;
    }
#else   /* MDP_FLAG_USE_RDMA_TO_CROP */
/*Original Path*/
    if( CB_EIS_INFO_GET_ != NULL )
    {
        CB_EIS_INFO_GET_( &eis_x , &eis_y );

        MDP_INFO("EIS function out: 0x%04X 0x%04X\n", (unsigned int)eis_x, (unsigned int)eis_y );

        if( path_1_rotate_angle_ & 0x1 ) //90 & 270
        {
            display_from_memory_param_.b_eis = 1;
            display_from_memory_param_.eis_float_x = eis_y & 0xFF;
            display_from_memory_param_.eis_float_y = eis_x & 0xFF;
            display_from_memory_param_.src_img_roi.x = ( ( eis_y & 0xFF00 ) >> 8 );
            display_from_memory_param_.src_img_roi.y = ( ( eis_x & 0xFF00 ) >> 8 );

        } else
        {
            display_from_memory_param_.b_eis = 1;
            display_from_memory_param_.eis_float_x = eis_x & 0xFF;
            display_from_memory_param_.eis_float_y = eis_y & 0xFF;
            display_from_memory_param_.src_img_roi.x = ( ( eis_x & 0xFF00 ) >> 8 );
            display_from_memory_param_.src_img_roi.y = ( ( eis_y & 0xFF00 ) >> 8 );
        }
    }
    else
    {
        display_from_memory_param_.b_eis = 0;
    }
#endif  /* MDP_FLAG_USE_RDMA_TO_CROP */
    /*2.1 Seamless EIS calculation*/
    #if 0
    /*.............................................................................*/
    //Calculate rough roi , clip by RDMA which require even value
    rough_crop_roi.x = MDP_ROUND_DOWN_VALUE( display_from_memory_param_.src_img_roi.x, 1 ); //Round down to 2x
    rough_crop_roi.y = MDP_ROUND_DOWN_VALUE( display_from_memory_param_.src_img_roi.y, 1 ); //Round down to 2x

    eis_delta_x = display_from_memory_param_.src_img_roi.x - rough_crop_roi.x;
    eis_delta_y = display_from_memory_param_.src_img_roi.y - rough_crop_roi.y;

    //RDMA roi w/h also need to fit even rule.
    rough_crop_roi.w = MDP_ROUND_UP_VALUE( display_from_memory_param_.src_img_roi.w + eis_delta_x , 1 );
    rough_crop_roi.h = MDP_ROUND_UP_VALUE( display_from_memory_param_.src_img_roi.h + eis_delta_y , 1 );

    //calculate fine roi.(odd part if any)
    fine_crop_roi.x = eis_delta_x;
    fine_crop_roi.y = eis_delta_y;
    fine_crop_roi.w = display_from_memory_param_.src_img_roi.w;
    fine_crop_roi.h = display_from_memory_param_.src_img_roi.h;
    #else
    _SplitRoi( display_from_memory_param_.src_img_roi, &rough_crop_roi, &fine_crop_roi );
    #endif

    //RDMA crop is rough roi
    display_from_memory_param_.src_img_roi = rough_crop_roi;

    //Set fine roi
    display_from_memory_param_.dst0_src_img_roi = fine_crop_roi;
    display_from_memory_param_.dst1_src_img_roi = fine_crop_roi;





    /*3.Update buffer address*/
    /*.............................................................................*/
    {
        display_from_memory_param_.src_yuv_img_addr = work_buffer_addr;

        /*dst0*/
        if( disp_dst0_buffer.enable ){
            if( disp_dst0_buffer.IsEmptyForHwWrite() ){

                disp_dst0_buffer.drop_frame_count++;
                if( disp_dst0_buffer.drop_frame_count > 30 ){
                    disp_dst0_buffer.b_show_drop_frame = 0;
                }
                if( disp_dst0_buffer.b_show_drop_frame == 1 ){
                    MDP_WARNING("dst_port0 drop %d frame\n", (int)disp_dst0_buffer.drop_frame_count);
                }

                display_from_memory_param_.en_dst_port0 = 0;

            } else {
                display_from_memory_param_.en_dst_port0 = 1;
                display_from_memory_param_.dst0_yuv_img_addr = disp_dst0_buffer.WriteYuvAddrGet();
            }
        }


        /*dst1*/
        if( disp_dst1_buffer.enable ){
            if( disp_dst1_buffer.IsEmptyForHwWrite() ){

                disp_dst1_buffer.drop_frame_count++;
                if( disp_dst1_buffer.drop_frame_count > 30 ){
                    disp_dst1_buffer.b_show_drop_frame = 0;
                }
                if( disp_dst1_buffer.b_show_drop_frame == 1 ){
                    MDP_WARNING("dst_port1 drop %d frame\n", (int)disp_dst1_buffer.drop_frame_count);
                }

                display_from_memory_param_.en_dst_port1 = 0;
            } else {
                display_from_memory_param_.en_dst_port1 = 1;
                display_from_memory_param_.dst1_yuv_img_addr = disp_dst1_buffer.WriteYuvAddrGet();
            }
        }


        /*dst2*/
        if( disp_dst2_buffer.enable ){
            if( disp_dst2_buffer.IsEmptyForHwWrite() ){

                disp_dst2_buffer.drop_frame_count++;
                if( disp_dst2_buffer.drop_frame_count > 30 ){
                    disp_dst2_buffer.b_show_drop_frame = 0;
                }
                if( disp_dst2_buffer.b_show_drop_frame == 1 ){
                    MDP_WARNING("dst_port2 drop %d frame\n", (int)disp_dst2_buffer.drop_frame_count);
                }

                display_from_memory_param_.en_dst_port2 = 0;
            } else {
                display_from_memory_param_.en_dst_port2 = 1;
                display_from_memory_param_.dst2_yuv_img_addr = disp_dst2_buffer.WriteYuvAddrGet();
            }
        }

    }

    /*Dump setting before trigger*/
    MDP_INFO_PATH("[SW Trigger Display From Memory Parameters]-----------------------------------------------------------------------------\n");
    Printout_MdpPathDisplayFromMemoryParameter( &display_from_memory_param_ );

    /*4.Trigger hw to do single shot display*/
    /*.............................................................................*/
    if( m_MapPathDisplayFromMemory.Config( &display_from_memory_param_ ) < 0 ){
        MDP_ERROR("Display from memory config failed\n");
        return -1;
    }

    if( m_MapPathDisplayFromMemory.Start( NULL ) < 0 ){
        MDP_ERROR("Display from memory start failed\n");
        m_MapPathDisplayFromMemory.End( NULL );
        return -1;
    }

    m_MapPathDisplayFromMemory.WaitBusy( NULL );
    m_MapPathDisplayFromMemory.End( NULL );

    /*5. Advance Write Pointer*/
    /*.............................................................................*/
    {
        /*dst0*/
        if( display_from_memory_param_.en_dst_port0  ){
            disp_dst0_buffer.AdvanceWritePointer( rgb0_ts_sec, rgb0_ts_us );
        }

        /*dst1*/
        if( display_from_memory_param_.en_dst_port1  ){
            disp_dst1_buffer.AdvanceWritePointer( rgb0_ts_sec, rgb0_ts_us );
        }

        /*dst2*/
        if( display_from_memory_param_.en_dst_port2  ){
            disp_dst2_buffer.AdvanceWritePointer( rgb0_ts_sec, rgb0_ts_us );
        }

    }

    return 0;


}

/*N3D Path*/
int MdpPipeCameraPreview::_QueueGetFreeListOrWaitBusyN3dPath( unsigned long mdp_id,unsigned long *p_StartIndex, unsigned long *p_Count  )
{

    static unsigned long rgb0_startindex = 0;
    static unsigned long rgb0_count = 0;

    unsigned long   work_buffer_index;
    unsigned long   work_buffer_count = disp_src_buffer.buffer_count;
    unsigned long   i = 0;

    switch( mdp_id )
    {
    case MID_RGB_ROT0://TG1 Output (HW)
        if( m_MdpPathN3d1stTg1.QueueGetFreeListOrWaitBusy( mdp_id,p_StartIndex,p_Count  ) < 0 ){
            return -1;
        }

        /*Sync HW/SW buffer index*/
        /*This is a dangerous buffer management, user must dequeue only once*/
        /*This approach will crash when user dequeue twice and do nothing*/
        if( *p_Count > 0 ){

            stTimeStamp     rgb0_timestamp;
            unsigned long   rgb0_ts_sec, rgb0_ts_us;

            rgb0_timestamp.mdp_id = MID_RGB_ROT0;

            if( MdpDrv_GetTimeStamp( &rgb0_timestamp ) < 0 ){
                MDP_ERROR("Get time stamp error\n");
                return -1;
            }

            rgb0_ts_sec = rgb0_timestamp.sec[ *p_StartIndex ];
            rgb0_ts_us  = rgb0_timestamp.usec[ *p_StartIndex ];

            n3d_tg1_work_src_buffer.AdvanceWritePointer(rgb0_ts_sec, rgb0_ts_us);
        }

        if( *p_Count > 1 ){
            MDP_WARNING("TG1 Working buffer get %d frame (>1)\n", (int)(*p_Count) );
        }
        break;

    case MID_VDO_ROT0://TG2 Output (HW)
        if( m_MdpPathN3d1stTg2.QueueGetFreeListOrWaitBusy( mdp_id,p_StartIndex,p_Count  ) < 0 ){
            return -1;
        }

        /*Sync HW/SW buffer index*/
        /*This is a dangerous buffer management, user must dequeue only once*/
        /*This approach will crash when user dequeue twice and do nothing*/
        if( *p_Count > 0 ){

            stTimeStamp     vdo0_timestamp;
            unsigned long   vdo0_ts_sec, vdo0_ts_us;

            vdo0_timestamp.mdp_id = MID_VDO_ROT0;

            if( MdpDrv_GetTimeStamp( &vdo0_timestamp ) < 0 ){
                MDP_ERROR("Get time stamp error\n");
                return -1;
            }

            vdo0_ts_sec = vdo0_timestamp.sec[ *p_StartIndex ];
            vdo0_ts_us  = vdo0_timestamp.usec[ *p_StartIndex ];

            n3d_tg2_work_src_buffer.AdvanceWritePointer(vdo0_ts_sec, vdo0_ts_us);
        }

        if( *p_Count > 1 ){
            MDP_WARNING("TG2 Working buffer get %d frame (>1)\n", (int)(*p_Count) );
        }
        break;

    case MID_VDO_ROT1://N3D LR side-by-side output (SW)
        n3d_dst_buffer.QueryAvailableReadBuffer( p_StartIndex, p_Count);
        MDP_INFO_QUEUE("[QUEUE]Dequeue <MID_VDO_ROT1> ROT get free list. Start = %lu Count = %lu\n",*p_StartIndex, *p_Count);
        break;

        break;
    }



    return 0;
}

int MdpPipeCameraPreview::QueueRefill( unsigned long resource_to_refill_mask )
{
    int ret;


    //For calculate fps info
    _TickEnQueueFpsInfo( resource_to_refill_mask );

    /*one pass preview path (for 1080p support)*/
    if( en_one_pass_preview_path_ == 1 )
    {
        if( m_MdpPathCameraPreviewToMemory.QueueRefill( resource_to_refill_mask ) < 0 ){
            ret = -1;
        }
        return ret;
    }
    /*N3D Path*/
    else if( en_n3d_preview_path_ == 1 )
    {
        if( _QueueRefillN3dPath( resource_to_refill_mask  ) < 0 )
                ret = -1;
    }
    /*Normal Path*/
    else if( en_zero_shutter_path_ == 0 || en_zero_shutter_path_ == 2)
    {
        /* HW Trigger Path*/
        if( en_sw_trigger_ == 0 )
        {
            unsigned long rsc_id_mask;

            //---Path 1-1
            rsc_id_mask = m_MdpPathCameraPreviewToMemory.ResourceIdMaskGet();
            if( rsc_id_mask & resource_to_refill_mask )
            {
                if( m_MdpPathCameraPreviewToMemory.QueueRefill( resource_to_refill_mask ) < 0 )
                {
                    ret = -1;
                }
            }

            //---Path 1-2
            rsc_id_mask = m_MapPathDisplayFromMemory.ResourceIdMaskGet();
            if( rsc_id_mask & resource_to_refill_mask )
            {
                if( m_MapPathDisplayFromMemory.QueueRefill( resource_to_refill_mask  ) < 0 )
                {
                    ret = -1;
                }
            }
        }else
        /* SW Trigger Path*/
        {
            if( _QueueRefillSwTrigger( resource_to_refill_mask  ) < 0 )
                ret = -1;
        }

    }
    /*Zero Shutter Path*/
    else
    {
        if( m_MapPathCameraPreviewZeroShutter.QueueRefill( resource_to_refill_mask  ) < 0 )
        {
            ret = -1;
        }
    }

    return ret;


}

// last preview frame for zsd
int MdpPipeCameraPreview::ConfigLastPreviewFrameAndTriggerCurrentDisplay(MdpRect crop_size)
{
    int ret = 0;
    unsigned long index;

    MDP_INFO("[ZSD] ConfigLastPreviewFrameAndTriggerCurrentDisplay\n");

    // Config camera preview path, and wait for the buffer ready
    m_MdpPathCameraPreviewToMemory.ConfigZSDPreviewFrame(
                        display_From_Memory_Src_Size_Last_Preview.w,
                        display_From_Memory_Src_Size_Last_Preview.h,
                        &index,
                        true);

    _SwTriggerWorkingByIndex( index );
    _SaveLastPreviewZoomInfo(crop_size);

    MDP_INFO_MDPD("[ZSD] ConfigLastPreviewFrame X\n");
    return ret;
}
int MdpPipeCameraPreview::ConfigZSDPreviewFrame(MdpRect crop_size)
{
    int ret = 0;
    unsigned long index;

    MDP_INFO_MDPD("[ZSD] ConfigPreviewFrame E\n");

    // After this configuration, camera preview will output ZSD frame.
    m_MdpPathCameraPreviewToMemory.ConfigZSDPreviewFrame(
                        display_From_Memory_Src_Size_Last_Preview.w,
                        display_From_Memory_Src_Size_Last_Preview.h,
                        &index,
                        false);

    MDP_INFO_MDPD("[ZSD] ConfigPreviewFrame X\n");
    return ret;
}



void MdpPipeCameraPreview::TiggerLastFrameDisplay(void)
{
    MDP_INFO_PERSIST("[ZSD] TiggerLastFrameDisplay\n");
    _UpdateLastPreviewZoomInfo();
    _SwTriggerWorkingByIndex( 0xff );
}

int MdpPipeCameraPreview::_SaveLastPreviewZoomInfo(MdpRect crop_size)
{

    MDP_INFO_PERSIST("[zsd] Save Last Preview Zoom Info\n");

    if( path_1_rotate_angle_ & 0x1 ) /*90/270 rotate*/
    {
        zoom_crop_region_2nd_path_last_frame_.x = crop_size.y;
        zoom_crop_region_2nd_path_last_frame_.y = crop_size.x;
        zoom_crop_region_2nd_path_last_frame_.w = crop_size.h;
        zoom_crop_region_2nd_path_last_frame_.h = crop_size.w;
    }
    else /*No rotation needed*/
    {
        zoom_crop_region_2nd_path_last_frame_.x = crop_size.x;
        zoom_crop_region_2nd_path_last_frame_.y = crop_size.y;
        zoom_crop_region_2nd_path_last_frame_.w = crop_size.w;
        zoom_crop_region_2nd_path_last_frame_.h = crop_size.h;
    }
    return 0;
}

int MdpPipeCameraPreview::_UpdateLastPreviewZoomInfo()
{

    MDP_INFO_PERSIST("[zsd] UpdateLastPreviewZoomInfo\n");
    display_from_memory_param_.src_img_size = display_From_Memory_Src_Size_Last_Preview;
    zoom_crop_region_2nd_path_ = zoom_crop_region_2nd_path_last_frame_;

    MDP_INFO_PERSIST("[ZSD] last preview src(%d %d) crop (%d %d %d %d)\n",
        (int)display_From_Memory_Src_Size_Last_Preview.w,
        (int)display_From_Memory_Src_Size_Last_Preview.h,
        (int)zoom_crop_region_2nd_path_.x,
        (int)zoom_crop_region_2nd_path_.y,
        (int)zoom_crop_region_2nd_path_.w,
        (int)zoom_crop_region_2nd_path_.h);

    return 0;
}

int MdpPipeCameraPreview::_QueueRefillSwTrigger( unsigned long resource_to_refill_mask )
{
    int ret = 0;

    unsigned long rsc_id_mask;

    //---Path 1-1
    rsc_id_mask = m_MdpPathCameraPreviewToMemory.ResourceIdMaskGet();
    if( rsc_id_mask & resource_to_refill_mask )
    {
        if( m_MdpPathCameraPreviewToMemory.QueueRefill( resource_to_refill_mask ) < 0 )
        {
            ret = -1;
        }
    }


    if( MID_VDO_ROT0 & resource_to_refill_mask )//dst 0
    {
        if( disp_dst0_buffer.AdvanceReadPointer() == 0 ){
            MDP_INFO_QUEUE("[QUEUE]Enqueue <MID_VDO_ROT0> Refill 1. Index = %d\n",disp_dst0_buffer.index_r);
        } else
        {
            ret = -1;
        }
    }
    if( MID_VDO_ROT1 & resource_to_refill_mask )//dst 1
    {
        if( disp_dst1_buffer.AdvanceReadPointer() == 0 ){
            MDP_INFO_QUEUE("[QUEUE]Enqueue <MID_VDO_ROT1> Refill 1. Index = %d\n",disp_dst1_buffer.index_r);
        } else
        {
            ret = -1;
        }
    }
    if( MID_RGB_ROT1 & resource_to_refill_mask )//dst 2
    {
        if( disp_dst2_buffer.AdvanceReadPointer() == 0 ){
            MDP_INFO_QUEUE("[QUEUE]Enqueue <MID_RGB_ROT1> Refill 1. Index = %d\n",disp_dst2_buffer.index_r);
        } else
        {
            ret = -1;
        }
    }

    return ret;
}


int MdpPipeCameraPreview::_QueueRefillN3dPath( unsigned long resource_to_refill_mask )
{
    int ret = 0;

    unsigned long rsc_id_mask;

    //TG1
    rsc_id_mask = m_MdpPathN3d1stTg1.ResourceIdMaskGet();
    if( rsc_id_mask & resource_to_refill_mask )
    {
        if( m_MdpPathN3d1stTg1.QueueRefill( resource_to_refill_mask ) < 0 )
        {
            ret = -1;
        }

        /*Sync HW/SW buffer management, this is a dangerous way to sync*/
        if( n3d_tg1_work_src_buffer.AdvanceReadPointer() == 0 ){
            MDP_INFO_QUEUE("[QUEUE]Enqueue <MID_RGB_ROT0> Refill 1. Index = %d (SW)\n",n3d_tg1_work_src_buffer.index_r);
        } else
        {
            ret = -1;
        }
    }

    //TG2
    rsc_id_mask = m_MdpPathN3d1stTg2.ResourceIdMaskGet();
    if( rsc_id_mask & resource_to_refill_mask )
    {
        if( m_MdpPathN3d1stTg2.QueueRefill( resource_to_refill_mask ) < 0 )
        {
            ret = -1;
        }

        /*Sync HW/SW buffer management, this is a dangerous way to sync*/
        if( n3d_tg2_work_src_buffer.AdvanceReadPointer() == 0 ){
            MDP_INFO_QUEUE("[QUEUE]Enqueue <MID_VDO_ROT0> Refill 1. Index = %d (SW)\n",n3d_tg2_work_src_buffer.index_r);
        } else
        {
            ret = -1;
        }
    }

    //Output Buffer (SW Trigger)
    if( MID_VDO_ROT1 & resource_to_refill_mask )
    {
        if( n3d_dst_buffer.AdvanceReadPointer() == 0 ){
            MDP_INFO_QUEUE("[QUEUE]Enqueue <MID_VDO_ROT1> Refill 1. Index = %d\n",disp_dst1_buffer.index_r);
        } else
        {
            ret = -1;
        }
    }

    return ret;
}

/*Specific for Native 3D camera*/
int MdpPipeCameraPreview::N3dManualTrigger2ndPass( MdpRect tg1_roi, MdpRect tg2_roi )
        {
    stTimeStamp     rgb0_timestamp;
    unsigned long   rgb0_ts_sec,rgb0_ts_us;
    int ret = 0;

    /*.............................................................................
        Sanity Test
        .............................................................................*/

    if( n3d_tg1_work_src_buffer.IsEmptyForSwRead() ){
        MDP_ERROR("n3d_tg1_work_src_buffer no read buffer!\n");
        ret = -1;
    }

    if( n3d_tg2_work_src_buffer.IsEmptyForSwRead() ){
        MDP_ERROR("n3d_tg2_work_src_buffer no read buffer!\n");
        ret = -1;
    }


    if( n3d_dst_buffer.IsEmptyForHwWrite() ){
        MDP_ERROR("n3d_dst_buffer no write buffer!\n");
        ret = -1;
    }

#if 0
    /*ROI consistence test*/
    if( ( tg1_roi.w != n3d2ndpass_tg1_param_.dst_img_roi.w ) ||
        ( tg1_roi.h != n3d2ndpass_tg1_param_.dst_img_roi.h ) ||
        ( tg2_roi.w != n3d2ndpass_tg2_param_.dst_img_roi.w ) ||
        ( tg2_roi.h != n3d2ndpass_tg2_param_.dst_img_roi.h )  )
    {
        MDP_ERROR("N3D Manual Trigger ROI and dest ROI not consist! tg1_in(%d %d %d %d) tg1_dst(%d %d %d %d)/tg2_in(%d %d %d %d) tg2_dst(%d %d %d %d)\n",
        (int)tg1_roi.x, (int)tg1_roi.y, (int)tg1_roi.w, (int)tg1_roi.h,
        (int)n3d2ndpass_tg1_param_.dst_img_roi.x, (int)n3d2ndpass_tg1_param_.dst_img_roi.y, (int)n3d2ndpass_tg1_param_.dst_img_roi.w, (int)n3d2ndpass_tg1_param_.dst_img_roi.h,
        (int)tg2_roi.x, (int)tg2_roi.y, (int)tg2_roi.w, (int)tg2_roi.h,
        (int)n3d2ndpass_tg2_param_.dst_img_roi.x, (int)n3d2ndpass_tg2_param_.dst_img_roi.y, (int)n3d2ndpass_tg2_param_.dst_img_roi.w, (int)n3d2ndpass_tg2_param_.dst_img_roi.h );

        ret = -1;
    }
#endif



    if( ret < 0 )
        return ret;



     /*.............................................................................
         Tg1 Parameter Mapping
         .............................................................................*/
    {

        MdpRect         rough_crop_roi;
        MdpRect         fine_crop_roi;

        _SplitRoi( tg1_roi, &rough_crop_roi, &fine_crop_roi );

    /*Source*/
        n3d2ndpass_tg1_param_.src_img_roi       =   rough_crop_roi; //!!!Input Parameter when trigger!!!
    n3d2ndpass_tg1_param_.src_yuv_img_addr  =   n3d_tg1_work_src_buffer.ReadYuvAddrGet();  //!!!Input Parameter when trigger!!!

        /*2-level cropping*/
        n3d2ndpass_tg1_param_.dst_src_img_roi   =   fine_crop_roi;   //Use when resizer in path has cropping ability

    /*Output port*/

    n3d2ndpass_tg1_param_.dst_yuv_img_addr  =   n3d_dst_buffer.WriteYuvAddrGet();  //!!!Input Parameter when trigger!!!
    }


    /*.............................................................................
         Tg2 Parameter Mapping
         .............................................................................*/
    {

        MdpRect         rough_crop_roi;
        MdpRect         fine_crop_roi;

        _SplitRoi( tg2_roi, &rough_crop_roi, &fine_crop_roi );

    /*Source*/
        n3d2ndpass_tg2_param_.src_img_roi       =   rough_crop_roi; //!!!Input Parameter when trigger!!!
    n3d2ndpass_tg2_param_.src_yuv_img_addr  =   n3d_tg2_work_src_buffer.ReadYuvAddrGet();  //!!!Input Parameter when trigger!!!


        /*2-level cropping*/
        n3d2ndpass_tg2_param_.dst_src_img_roi   =   fine_crop_roi;   //Use when resizer in path has cropping ability

    /*Output port*/
    n3d2ndpass_tg2_param_.dst_yuv_img_addr  =   n3d_dst_buffer.WriteYuvAddrGet();  //!!!Input Parameter when trigger!!!
    }


    /*.............................................................................
         Get Tg1 RGB0 Timestamp as timestamp
         .............................................................................*/
    /*Get Timestamp of RGB0,use as overall timestamp*/
     rgb0_timestamp.mdp_id = MID_RGB_ROT0;

     if( MdpDrv_GetTimeStamp( &rgb0_timestamp ) < 0 ){
         MDP_ERROR("Get time stamp error\n");
         return -1;
        }

     rgb0_ts_sec = rgb0_timestamp.sec[ n3d_tg1_work_src_buffer.index_r ];
     rgb0_ts_us  = rgb0_timestamp.usec[ n3d_tg1_work_src_buffer.index_r ];




    /*.............................................................................
        Tg1 Trigger
        .............................................................................*/
    /*Dump setting before trigger*/
    MDP_INFO_PATH("[N3D Manual Trigger TG1 Parameters]-----------------------------------------------------------------------------\n");
    //Printout_MdpPathDisplayFromMemoryParameter( &display_from_memory_param_ );

    if( m_MdpPathN3d2nd.Config( &n3d2ndpass_tg1_param_ ) < 0 ){
        MDP_ERROR("TG1 MdpPathN3d2ndPass config failed\n");
        return -1;
    }

    if( m_MdpPathN3d2nd.Start( NULL ) < 0 ){
        MDP_ERROR("TG1 MdpPathN3d2ndPass Start failed\n");
        m_MdpPathN3d2nd.End( NULL );
        return -1;
    }


    m_MdpPathN3d2nd.WaitBusy( NULL );
    m_MdpPathN3d2nd.End( NULL );


    /*.............................................................................
        Tg2 Trigger
        .............................................................................*/
    /*Dump setting before trigger*/
    MDP_INFO_PATH("[N3D Manual Trigger TG2 Parameters]-----------------------------------------------------------------------------\n");
    //Printout_MdpPathDisplayFromMemoryParameter( &display_from_memory_param_ );

    if( m_MdpPathN3d2nd.Config( &n3d2ndpass_tg2_param_ ) < 0 ){
        MDP_ERROR("TG1 MdpPathN3d2ndPass config failed\n");
        return -1;
    }

    if( m_MdpPathN3d2nd.Start( NULL ) < 0 ){
        MDP_ERROR("TG2 MdpPathN3d2ndPass Start failed\n");
        m_MdpPathN3d2nd.End( NULL );
        return -1;
    }

    m_MdpPathN3d2nd.WaitBusy( NULL );
    m_MdpPathN3d2nd.End( NULL );




    /*.............................................................................
        Advance Write Pointer
        .............................................................................*/
    {
        /*Out buffer*/
        if( n3d_dst_buffer.AdvanceWritePointer( rgb0_ts_sec, rgb0_ts_us ) < 0 ){
            MDP_ERROR("n3d_dst_buffer.AdvanceWritePointer error\n");
            return -1;
    }
    }


    return 0;





}


int MdpPipeCameraPreview::GetTimeStamp( unsigned long mdp_id /*Dont't Care*/, stTimeStamp * p_timestamp )
{

    int ret;

    if( p_timestamp == NULL ){
        MDP_ERROR("p_timestamp == NULL\n");
        return -1;
    }

    /*one pass preview path (for 1080p support)*/
    if( en_one_pass_preview_path_ == 1 )
    {
        p_timestamp->mdp_id = ( b_is_generic_yuv_in_one_pass_preview_path_ ) ? MID_VDO_ROT0 : MID_RGB_ROT0; //1: use me_rgbrot0  0: use me_vdorot0
        return MdpDrv_GetTimeStamp( p_timestamp );
    }
    /*N3D Path*/
    else if( en_n3d_preview_path_ == 1 )
    {
        memcpy( p_timestamp, &n3d_dst_buffer.time_stamp, sizeof( stTimeStamp ) );
    }
    /*Normal Path*/
    else if( en_zero_shutter_path_ == 0 || en_zero_shutter_path_ == 2)
    {
        /* HW Trigger Path*/
        if( en_sw_trigger_ == 0 )
        {
            p_timestamp->mdp_id = MID_RGB_ROT0;
            return MdpDrv_GetTimeStamp( p_timestamp );
        }
        else
        /* SW Trigger Path*/
        {
            /*MID_VDO_ROT0:*/
            memcpy( p_timestamp, &disp_dst0_buffer.time_stamp, sizeof( stTimeStamp ) );
        }
    }
    /*Zero Shutter Path*/
    else
    {
        p_timestamp->mdp_id = MID_VDO_ROT1;
        return MdpDrv_GetTimeStamp( p_timestamp );
    }

    return 0;



    }

int MdpPipeCameraPreview::GetBufferAddress( unsigned long mdp_id, int buffer_index, MdpYuvAddr* pAddr )
    {
    if( en_n3d_preview_path_!= 1 )
        {
        MDP_ERROR("GetBufferAddress only support in Native 3D Camera currently\n");
        return -1;
    }
            switch( mdp_id )
            {
    case MID_RGB_ROT0:
        return n3d_tg1_work_src_buffer.GetAddrByIndex( buffer_index, pAddr );
        break;

            case MID_VDO_ROT0:
        return n3d_tg2_work_src_buffer.GetAddrByIndex( buffer_index, pAddr );
                break;
            case MID_VDO_ROT1:
        return n3d_dst_buffer.GetAddrByIndex( buffer_index, pAddr );
                break;

           default:
        MDP_ERROR("mdp_id = 0x%08X not found\n", (unsigned int)mdp_id );
        return -1;
        }

    return 0;

}




int MdpPipeCameraPreview::DumpRegister( void )
{

    /*one pass preview path (for 1080p support)*/
    if( en_one_pass_preview_path_ == 1 )
    {
        m_MdpPathCameraPreviewToMemory.DumpRegister( NULL );
    }
    /*N3D Path*/
    else if( en_n3d_preview_path_ == 1 )
    {
        m_MdpPathN3d1stTg1.DumpRegister( NULL );
        m_MdpPathN3d1stTg2.DumpRegister( NULL );

    }
    /*Normal Path*/
    else if( en_zero_shutter_path_ == 0 || en_zero_shutter_path_ == 2)
    {
        /*HW Trigger*/
        if( en_sw_trigger_ == 0 )
        {
            m_MdpPathCameraPreviewToMemory.DumpRegister( NULL );
            m_MapPathDisplayFromMemory.DumpRegister( NULL );
        }else
        /*SW Trigger*/
        {
            m_MdpPathCameraPreviewToMemory.DumpRegister( NULL );
        }
    }
    /*Zero Shutter Path*/
    else
    {
        m_MapPathCameraPreviewZeroShutter.DumpRegister( NULL );
    }


    return 0;
}


int MdpPipeCameraPreview::End( void* pParam )
{
    /*one pass preview path (for 1080p support)*/
    if( en_one_pass_preview_path_ == 1 )
    {
        m_MdpPathCameraPreviewToMemory.End( pParam );
    }
    /*N3D Path*/
    else if( en_n3d_preview_path_ == 1 )
    {
        m_MdpPathN3d1stTg1.End( pParam );
        m_MdpPathN3d1stTg2.End( pParam );


        n3d_tg1_work_src_buffer.UnRegisterLoopMemory();    /*loop_mem_index:0*/
        n3d_tg2_work_src_buffer.UnRegisterLoopMemory();    /*loop_mem_index:1*/
        n3d_dst_buffer.UnRegisterLoopMemory();             /*loop_mem_index:2*/
    }
    else
    /*Normal Path*/
    if( en_zero_shutter_path_ == 0 || en_zero_shutter_path_ == 2)
    {
        /*HW Trigger*/
        if( en_sw_trigger_ == 0 )
        {
            m_MdpPathCameraPreviewToMemory.End( pParam );
            m_MapPathDisplayFromMemory.End( pParam );
        } else
        /*SW Trigger*/
        {

            m_MdpPathCameraPreviewToMemory.End( pParam );
            // last preview frame for zsd
            if (en_zero_shutter_path_ == 2) {
                m_MdpPathCameraPreviewToMemory.UnAdpateLastPreviewBuffer();
            }



            disp_src_buffer.UnRegisterLoopMemory();
            disp_dst0_buffer.UnRegisterLoopMemory();
            disp_dst1_buffer.UnRegisterLoopMemory();
            disp_dst2_buffer.UnRegisterLoopMemory();
        }
    }
    /*Zero Shutter Path*/
    else
    {
        m_MapPathCameraPreviewZeroShutter.End( pParam );
    }


    return 0;
}

void MdpPipeCameraPreview::_TickEnQueueFpsInfo( unsigned long mdp_resource_mask )
{

    fps_show_info_counter_++;

    fps_eq_working_.TickReference( mdp_resource_mask );
    fps_eq_dst0_.TickReference( mdp_resource_mask );
    fps_eq_dst1_.TickReference( mdp_resource_mask );
    fps_eq_dst2_.TickReference( mdp_resource_mask );

    //CLOUDS TEST
    MDP_INFO_FPS("EnQ:w:%8dus\n", (int)fps_eq_working_.latest_time_elapse_us_ );
    MDP_INFO_FPS("EnQ:0:%8dus\n", (int)fps_eq_dst0_.latest_time_elapse_us_ );
    MDP_INFO_FPS("EnQ:1:%8dus\n", (int)fps_eq_dst1_.latest_time_elapse_us_ );
    MDP_INFO_FPS("EnQ:2:%8dus\n", (int)fps_eq_dst2_.latest_time_elapse_us_ );


    if( fps_show_info_counter_ >= 60 )
    {
        fps_show_info_counter_ = 0;

        MDP_INFO_FPS("[FPS]EnQueue FPS:\n");
        MDP_INFO_FPS("[FPS]fps_eq_working_.fps_ = %lu (avg = %6luus)\n", fps_eq_working_.fps_, fps_eq_working_.avg_interval_us_);
        MDP_INFO_FPS("[FPS]fps_eq_dst0_.fps_ = %lu (avg = %6luus)\n", fps_eq_dst0_.fps_, fps_eq_dst0_.avg_interval_us_);
        MDP_INFO_FPS("[FPS]fps_eq_dst1_.fps_ = %lu (avg = %6luus)\n", fps_eq_dst1_.fps_, fps_eq_dst1_.avg_interval_us_);
        MDP_INFO_FPS("[FPS]fps_eq_dst2_.fps_ = %lu (avg = %6luus)\n", fps_eq_dst2_.fps_, fps_eq_dst2_.avg_interval_us_);
        MDP_INFO_FPS("[FPS]DeQueue FPS:\n");
        MDP_INFO_FPS("[FPS]fps_dq_working_.fps_ = %lu (avg = %6luus)\n", fps_dq_working_.fps_, fps_dq_working_.avg_interval_us_);
        MDP_INFO_FPS("[FPS]fps_dq_dst0_.fps_ = %lu (avg = %6luus)\n", fps_dq_dst0_.fps_, fps_dq_dst0_.avg_interval_us_);
        MDP_INFO_FPS("[FPS]fps_dq_dst1_.fps_ = %lu (avg = %6luus)\n", fps_dq_dst1_.fps_, fps_dq_dst1_.avg_interval_us_);
        MDP_INFO_FPS("[FPS]fps_dq_dst2_.fps_ = %lu (avg = %6luus)\n", fps_dq_dst2_.fps_, fps_dq_dst2_.avg_interval_us_);

    }



}

void MdpPipeCameraPreview::_TickDeQueueFpsInfo( unsigned long mdp_resource_mask )
{

    fps_show_info_counter_++;

    fps_dq_working_.TickReference( mdp_resource_mask );
    fps_dq_dst0_.TickReference( mdp_resource_mask );
    fps_dq_dst1_.TickReference( mdp_resource_mask );
    fps_dq_dst2_.TickReference( mdp_resource_mask );


    //CLOUDS TEST
    MDP_INFO_FPS("DeQ:w:%8dus\n", (int)fps_dq_working_.latest_time_elapse_us_ );
    MDP_INFO_FPS("DeQ:0:%8dus\n", (int)fps_dq_dst0_.latest_time_elapse_us_ );
    MDP_INFO_FPS("DeQ:1:%8dus\n", (int)fps_dq_dst1_.latest_time_elapse_us_ );
    MDP_INFO_FPS("DeQ:2:%8dus\n", (int)fps_dq_dst2_.latest_time_elapse_us_ );

}


/*/////////////////////////////////////////////////////////////////////////////
    MdpPipeCameraPreview::BufferInfo
  /////////////////////////////////////////////////////////////////////////////*/
void MdpPipeCameraPreview::BufferInfo::Reset( int loop_mem_index )
{
    int i;

    enable = 0;
    buffer_count = 0;
    memset( buffer, 0, sizeof( buffer ) );
    index_w = 0;
    index_r = 0;
    b_empty = 0;
    memset( &time_stamp, 0, sizeof( stTimeStamp ) );

    b_show_drop_frame = 1;
    drop_frame_count = 0;

    /*init semaphore to 0*/
    sem_init( &sem_hw_finish_count_, 0, 0 );


    memset( p_loop_mem_param, 0, sizeof(p_loop_mem_param) );
    memset( p_loop_mem_obj, 0, sizeof(p_loop_mem_obj) );

    //memset( loop_mem_param, 0, sizeof(loop_mem_param) );
    //memset( loop_mem_obj, 0, sizeof(loop_mem_obj) );

    loop_mem_index_ = loop_mem_index;

    for( i = 0 ; i < MT6575RDMA_MAX_RINGBUFFER_CNT; i++ )
    {
        p_loop_mem_param[i] = &loop_mem_param[loop_mem_index_][i];
        p_loop_mem_obj[i] = &loop_mem_obj[loop_mem_index_][i];
    }



}

int MdpPipeCameraPreview::BufferInfo::IsEmptyForHwWrite( void )
{
    return b_empty;
}

int  MdpPipeCameraPreview::BufferInfo::GetAddrByIndex( int index, MdpYuvAddr *pAddr )
{
    if( ( index < 0 ) || ( index >= buffer_count ) )
    {
        MDP_ERROR("<%s> no buffer index is %d, buffer count = %d\n", name_str, index, buffer_count  );
        return -1;
    }

    if( pAddr == NULL )
    {
        MDP_ERROR("<%s> pAddr == NULL\n", name_str );
        return -1;
    }

    *pAddr = buffer[ index ];

    return 0;
}


int MdpPipeCameraPreview::BufferInfo::AdvanceWritePointer( unsigned long ts_sec, unsigned long ts_us )
{
    int     sem_val;


    /*no free buffer for hw to write*/
    if( (index_w == index_r) && (b_empty == 1 ) )
    {
        MDP_INFO_QUEUE("[QUEUE] <%s> No available buffer to write\n", name_str);
        return -1;
    }


    time_stamp.sec[index_w] = ts_sec;
    time_stamp.usec[index_w] = ts_us;


    #if 0
    /*For Debug timestamp issue, add log here*/
    if( loop_mem_index_ == 1 ) //Only print disp0 buffer timestamp
    {
        MDP_INFO_PERSIST("[TS] Buffer Type = %d, Buffer Index = %2d, sec = %lu, us = %lu\n",
            loop_mem_index_, index_w,  ts_sec, ts_us);
    }
    #endif

    /*Check Timesamp reverse*/
    {
        int b_reverse_detected = 0;
        unsigned long last_index;
        unsigned long last_s, last_us, curr_s, curr_us;

        last_index = ( index_w == 0 ) ? (buffer_count-1) : (index_w - 1);

        last_s  = time_stamp.sec[last_index];
        last_us = time_stamp.usec[last_index];
        curr_s  = time_stamp.sec[index_w];
        curr_us = time_stamp.usec[index_w];

        if( curr_s < last_s )
        {
            b_reverse_detected = 1;
        } else if( ( curr_s == last_s) &&  ( curr_us < last_us) )
        {
            b_reverse_detected = 1;
        }

        if( b_reverse_detected )
        {
            unsigned long sec,usec;

            MDP_ERROR("Timestamp Reverse! loop_index = %d , sec/us[%d]=(%lu %lu) < sec/us[%lu]=(%lu %lu)\n",
                loop_mem_index_, index_w, curr_s, curr_us, last_index, last_s, last_us );

            /*Resample timestamp*/
            MdpDrv_CpuClockGet( &sec, &usec );
            time_stamp.sec[index_w]     = sec;
            time_stamp.usec[index_w]    = usec;
            MDP_INFO_PERSIST("Timestamp recovered! ( %lu %lu )==>( %lu %lu )\n", curr_s, curr_us, sec, usec );

        }

    }


    /*write pointer + 1, if empty set flag*/
    index_w = ( index_w + 1 )%buffer_count;

    if( index_w == index_r ){
        b_empty = 1;
    }

    /*Increase semaphore*/
    sem_post( &sem_hw_finish_count_ );
    //sem_getvalue( &sem_hw_finish_count_, &sem_val );
    //MDP_INFO_QUEUE("[QUEUE] <%s> Post: sem_hw_finish_count_ = %d\n", name_str, (int)sem_val );

    return 0;
}

int MdpPipeCameraPreview::BufferInfo::AdvanceReadPointer( void )
{
    /*If hw no consume any buffer*/
    if( (index_r == index_w) && (b_empty == 0 ) )
    {
        MDP_ERROR("[QUEUE] Enqueue <%s> failed. No available buffer to read.Queue/Deque pair error\n",name_str);
        return -1;
    }

    /*read pointer + 1, if empty set flag*/
    index_r = ( index_r + 1 )%buffer_count;
    b_empty = 0;

    return 0;

}

void MdpPipeCameraPreview::BufferInfo::QueryAvailableReadBuffer( unsigned long *p_startindex, unsigned long *p_count)
{
    int     semwait_ret = 0;
    int     sem_val;
    struct  timespec timeout;


    timeout.tv_sec  = time(NULL)+5;
    timeout.tv_nsec = 0;

    semwait_ret = sem_timedwait(&sem_hw_finish_count_, &timeout);

    //sem_getvalue( &sem_hw_finish_count_, &sem_val );
    //MDP_INFO_QUEUE("[QUEUE] <%s> Wait & Wake-Up: sem_timedwait sem_hw_finish_count_ = %d\n", name_str, (int)sem_val );

    if( semwait_ret == -1 )
    {
        sem_getvalue( &sem_hw_finish_count_, &sem_val );
        MDP_ERROR("[QUEUE] <%s> sem_timedwait TIMEOUT. sem_hw_finish_count_ = %d error=%s\n", name_str, (int)sem_val,strerror(errno) );
        *p_count = 0;
        return;
    }

    /*-----------------------------------------------------------------------------

      -----------------------------------------------------------------------------*/

    *p_startindex = index_r;

    if     ( index_w > index_r ){
        *p_count = index_w - index_r;
    }
    else if( index_w < index_r ){
        *p_count = buffer_count - index_r + index_w;
    }
    else if( index_w == index_r){
        if( b_empty == 1 )
        {
            *p_count = buffer_count;
        }
        else
        {
            *p_count = 0;
        }
    }

    if( ((int)(*p_count) < 0) || (*p_count > (unsigned long)buffer_count) )
    {
        MDP_ERROR("*p_count=%d, buffer_count=%d, index_w=%d, index_r=%d, b_empty=%d", (int)*p_count, (int)buffer_count, (int)index_w, (int)index_r, (int)b_empty );
        return;
    }


    /*-----------------------------------------------------------------------------
        Consume the reset semaphore count
      -----------------------------------------------------------------------------*/
    //sem_getvalue( &sem_hw_finish_count_, &sem_val );
    //MDP_INFO_QUEUE("[QUEUE] <%s> Consume: *p_count = %d, sem_hw_finish_count_ = %d  ( index_w = %d , index_r = %d , buffer_count = %d )\n",
    //                        name_str,(int)*p_count, (int)sem_val,  (int)index_w, (int)index_r, (int)buffer_count);


    if( *p_count > 1 )
    {
        unsigned long i;


        for( i = 0 ; i < (*p_count-1); i++  )
        {
            semwait_ret = sem_trywait(&sem_hw_finish_count_ );
            if( semwait_ret == -1 )
            {
                sem_getvalue( &sem_hw_finish_count_, &sem_val );
                MDP_WARNING("[QUEUE] <%s> consume sem_timedwait failed. *p_count = %d, sem_hw_finish_count_ = %d\n", name_str,(int)*p_count, (int)sem_val );
            }
        }
        //sem_getvalue( &sem_hw_finish_count_, &sem_val );
        //MDP_INFO_QUEUE("[QUEUE] <%s> Consume Done: *p_count = %d, sem_hw_finish_count_ = %d\n", name_str,(int)*p_count, (int)sem_val );
    }

}


int MdpPipeCameraPreview::BufferInfo::RegisterLoopMemory(   int b_mem_type_input,
                                                            MdpColorFormat color_format,
                                                            MdpSize img_size,
                                                            MdpRect img_roi,
                                                            int rotate )

{
#if defined(MDP_FLAG_PROFILING)
    MdpDrv_Watch _MdpDrvWatch;
    static unsigned long tt0,tt1,tt2,tt3;//total time;
    static unsigned long fc0,fc1,fc2,fc3;//frame count;
    static unsigned long avt0,avt1,avt2,avt3;//avg_time_elapse;
    char   title_str[256];
#endif

    int ret = 0;


#if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)

    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_PIPE );

    int i;

    for( i = 0; i < buffer_count; i++ )
    {
        p_loop_mem_param[i]->mem_type      = ( b_mem_type_input == 0 )? MEM_TYPE_INPUT : MEM_TYPE_OUTPUT;
        p_loop_mem_param[i]->addr          = buffer[i].y;
        p_loop_mem_param[i]->buffer_size   = img_size.w*img_size.h*4;
        p_loop_mem_param[i]->mhal_color    = MdpColorFormatToMhal(color_format);
        p_loop_mem_param[i]->img_size      = img_size;
        p_loop_mem_param[i]->img_roi       = img_roi;
        p_loop_mem_param[i]->rotate        = rotate;


        if( Mdp_RegisterLoopMemory(MLM_CLIENT_PV2ND, p_loop_mem_param[i], p_loop_mem_obj[i] ) < 0 )
        {
            MDP_ERROR("%s register %d loop memory failed\n",name_str, i );
            ret = -1;
        }



        //MDP_DEBUG("\n");
        //MDP_DEBUG("%s register %d loop memory VA:0x%08X MVA:0x%08X\n",name_str, i , (unsigned int)buffer[i].y , (unsigned int)p_loop_mem_obj[i]->adapt_addr[0].y );
        //MDP_DEBUG("\n");

    }

    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_PIPE , "RegisterLoopMemory", &tt0, &fc0, &avt0, 30 );

#endif /*#if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)*/



    return ret;

}

int MdpPipeCameraPreview::BufferInfo::UnRegisterLoopMemory( void )
{

#if defined(MDP_FLAG_PROFILING)
    MdpDrv_Watch _MdpDrvWatch;
    static unsigned long tt0,tt1,tt2,tt3;//total time;
    static unsigned long fc0,fc1,fc2,fc3;//frame count;
    static unsigned long avt0,avt1,avt2,avt3;//avg_time_elapse;
    char   title_str[256];
#endif

    int ret = 0;

#if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)

    int i;

    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_PIPE );


    for( i = 0; i < buffer_count; i++ )
    {

        //MDP_DEBUG("\n");
        //MDP_DEBUG("%s Un-register %d loop memory 0x%08X\n",name_str, i , (unsigned int) p_loop_mem_obj[i]->calc_addr[0].y );
        //MDP_DEBUG("\n");
        if( Mdp_UnRegisterLoopMemory( MLM_CLIENT_PV2ND, p_loop_mem_obj[i] ) < 0 )
        {
            MDP_ERROR("%s un-register %d loop memory failed\n",name_str, i );
            ret = -1;
        }
    }


    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_PIPE , "UnRegisterLoopMemory", &tt0, &fc0, &avt0, 30 );

#endif /*#if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)*/


    return ret;

}





/*/////////////////////////////////////////////////////////////////////////////
    MdpPipeCameraPreview::BufferInfo
  /////////////////////////////////////////////////////////////////////////////*/
void MdpPipeCameraPreview::DmaFreqInfo::TickReference( unsigned long mdp_resource_id_mask )
{
    #define _RESET_FRAME_COUNT  (30)

    unsigned long curren_time_us;

    if( ( mdp_id_ & mdp_resource_id_mask ) == 0 )
        return;

    curren_time_us = MdpDrv_GetTimeUs();

    counter_++;
    infini_counter_++;


    if( last_timestamp_us_ != 0 )
    {
        latest_time_elapse_us_ = curren_time_us - last_timestamp_us_;
        total_time_elapse_us_ += latest_time_elapse_us_;
    }

    last_timestamp_us_ = curren_time_us;

    //Reset & calculate fps every n frame
    if( counter_ == _RESET_FRAME_COUNT )
    {
        fps_            = (1000000*counter_)/total_time_elapse_us_;
        avg_interval_us_   = total_time_elapse_us_/counter_;
        total_time_elapse_us_= 0;

        counter_ = 0;
    }


}




/*/////////////////////////////////////////////////////////////////////////////
    MdpPipeCameraCapture
  /////////////////////////////////////////////////////////////////////////////*/
int MdpPipeCameraCapture::Config( struct MdpPipeCameraCaptureParameter* p_parameter )
{
    int ret = 0;
    struct MdpPathJpgEncodeScaleParameter   mp_jesp;



    /*-----------------------------------------------------------------------------
      Parametr Sanity Test
      -----------------------------------------------------------------------------*/

    if( p_parameter->camera_out_mode != 0 )    //0:BayerRaw 1:YUV 2:JPEG
    {
        MDP_ERROR("Camera Mode = %d ,not implemented!\n\r", p_parameter->camera_out_mode);
        return -1;
    }

    if( p_parameter->camera_count != 0 )       //0:single camera 1:dual camera
    {
        MDP_ERROR("Camera Count = %d ,not implemented!\n\r", p_parameter->camera_count + 1);
        return -1;
    }


    /***Parameter Convertion***/
    /*-----------------------------------------------------------------------------
      MdpPathJpgEncodeScaleParameter
      -----------------------------------------------------------------------------*/
    /*-----Input-----*/
    mp_jesp.b_sensor_input = (p_parameter->pseudo_src_enable == 1)?0:1;

    if( mp_jesp.b_sensor_input == 0 )
    {
        mp_jesp.src_yuv_img_addr = p_parameter->pseudo_src_yuv_img_addr;
        mp_jesp.src_color_format = p_parameter->pseudo_src_color_format;
    }

    mp_jesp.src_img_size = p_parameter->src_img_size;
    mp_jesp.src_img_roi = p_parameter->src_img_roi;


     /*-----JPEG Output-----*/
     mp_jesp.b_jpg_path_disen = p_parameter->b_jpg_path_disen;
     //if( mp_jesp.b_jpg_path_disen == 0 )
     {
     mp_jesp.jpg_img_size = p_parameter->jpg_img_size;
     mp_jesp.jpg_yuv_color_format = p_parameter->jpg_yuv_color_format;
     mp_jesp.jpg_buffer_addr = p_parameter->jpg_buffer_addr;
     mp_jesp.jpg_buffer_size = p_parameter->jpg_buffer_size;
     mp_jesp.jpg_quality = p_parameter->jpg_quality;
     mp_jesp.jpg_b_add_soi = p_parameter->jpg_b_add_soi;
     }


     /*-----Quick View (RGB) Output-----*/
     mp_jesp.b_qv_path_en = p_parameter->b_qv_path_en;
     //if( mp_jesp.b_qv_path_en == 1 )
     {
         mp_jesp.qv_path_sel= p_parameter->qv_path_sel;
         mp_jesp.qv_yuv_img_addr= p_parameter->qv_yuv_img_addr;
         mp_jesp.qv_img_size= p_parameter->qv_img_size;

         mp_jesp.qv_img_roi= p_parameter->qv_img_roi;
         if( mp_jesp.qv_img_roi == MdpRect( 0, 0, 0, 0 ) ) {
            mp_jesp.qv_img_roi = MdpRect( mp_jesp.qv_img_size );
         }
         if( ( mp_jesp.qv_img_roi.x != 0 ) || ( mp_jesp.qv_img_roi.y != 0 ) )
         {
            MDP_ERROR("Not support non-0 roi offset:qv_img_roi=(%d,%d,%d,%d)\n",
                (int)mp_jesp.qv_img_roi.x, (int)mp_jesp.qv_img_roi.y, (int)mp_jesp.qv_img_roi.w, (int)mp_jesp.qv_img_roi.h );
            DumpRegister();
            return -1;
         }

         mp_jesp.qv_color_format= p_parameter->qv_color_format;
         mp_jesp.qv_flip= p_parameter->qv_flip;
         mp_jesp.qv_rotate= p_parameter->qv_rotate;
     }


     /*-----Full Frame (YUV) Output-----*/
     mp_jesp.b_ff_path_en = p_parameter->b_ff_path_en;
     //if( mp_jesp.b_ff_path_en == 1 )
     {
         mp_jesp.ff_yuv_img_addr= p_parameter->ff_yuv_img_addr;
         mp_jesp.ff_img_size= p_parameter->ff_img_size;


         mp_jesp.ff_img_roi= p_parameter->ff_img_roi;
         if( mp_jesp.ff_img_roi == MdpRect( 0, 0, 0, 0 ) ) {
            mp_jesp.ff_img_roi = MdpRect( mp_jesp.ff_img_size );
         }
         if( ( mp_jesp.ff_img_roi.x != 0 ) || ( mp_jesp.ff_img_roi.y != 0 ) )
         {
            MDP_ERROR("Not support non-0 roi offset:ff_img_roi=(%d,%d,%d,%d)\n",
                (int)mp_jesp.ff_img_roi.x, (int)mp_jesp.ff_img_roi.y, (int)mp_jesp.ff_img_roi.w, (int)mp_jesp.ff_img_roi.h );
            DumpRegister();
            return -1;
         }

         mp_jesp.ff_color_format= p_parameter->ff_color_format;
         mp_jesp.ff_flip= p_parameter->ff_flip;
         mp_jesp.ff_rotate= p_parameter->ff_rotate;
     }

     /*-----resz coeff-----*/
     mp_jesp.resz_coeff= p_parameter->resz_coeff;


    /*-----------------------------------------------------------------------------
      MdpPathJpgEncodeScale Config
      -----------------------------------------------------------------------------*/
    ret = mpath_jpg_encode_scale_.Config( &mp_jesp );

    return ret;


}


int MdpPipeCameraCapture::Start( void* pParam )
{

    return mpath_jpg_encode_scale_.Start( pParam );

}

int MdpPipeCameraCapture::WaitBusy( unsigned long rsc_to_wait_mask  )
{
    //return 0  : ok
    //return -1 : busy and timeout
    return mpath_jpg_encode_scale_.WaitBusy( rsc_to_wait_mask  );


}

int MdpPipeCameraCapture::DumpRegister( void )
{
    return mpath_jpg_encode_scale_.DumpRegister( NULL );
}


int MdpPipeCameraCapture::End( void* pParam )
{

    return mpath_jpg_encode_scale_.End( pParam );
}




/*#############################################################################
    C Version
  #############################################################################*/

#define IMAGE_TRANSFORM_ROUTE( _it_func_, _param_, _request_flag_, _property_flag_ ) \
    {\
        int __ret_val;\
        if( (_request_flag_ == NULL) || ( (_request_flag_ & _property_flag_) != NULL ) )\
        {\
        __ret_val = _it_func_( &_param_ );\
        if( __ret_val >= MDP_ERROR_CODE_OK )  {\
                    MDP_INFO_PERSIST("IT func applied is = {%s} Favor Flags = 0x%08X src=(%d, %dx%d,(%d,%d,%d,%d)) dst=(%d, %dx%d,(%d,%d,%d,%d),%d) \n", \
                #_it_func_ , (unsigned int)_request_flag_ ,\
                (int)_param_.src_color_format,\
                (int)_param_.src_img_size.w, (int)_param_.src_img_size.h,\
                (int)_param_.src_img_roi.x,(int)_param_.src_img_roi.y,(int)_param_.src_img_roi.w,(int)_param_.src_img_roi.h,\
                (int)_param_.dst_color_format,\
                (int)_param_.dst_img_size.w, (int)_param_.dst_img_size.h,\
                (int)_param_.dst_img_roi.x,(int)_param_.dst_img_roi.y,(int)_param_.dst_img_roi.w,(int)_param_.dst_img_roi.h,\
                (int)_param_.dst_rotate_angle \
                );\
            goto _EXIT_MdpPipeImageTransform_Func;\
        }\
            else if( MDP_IS_ERROR_CODE( __ret_val, MDP_ERROR_CODE_LOCK_RESOURCE_FAIL ) ){\
                MDP_INFO_PERSIST("IT func {%s} is busy.retry = %d\n", #_it_func_ , retry );\
            }\
            else {\
                MDP_ERROR("IT func {%s} return failed:%d.\n", #_it_func_, __ret_val );\
        }\
        }\
    }


int MdpPipeImageTransform_Func( struct MdpPipeImageTransformParameter* p_parameter )
{
    const int       IT_PATH_RETRY_COUNT = 10;
    int             retry = 0;
    int ret_val = 0;
    unsigned long   req_favor_flags = NULL;


//Add for workaround a hardware limitation,ISP cannot handle odd width or < 16 pixel image - 2012/1/10 -S
#if 0
    void * pWorkingBuff = NULL;
    struct MdpPathImageTransformParameter tempParam;
    unsigned long u4ResidualWidth;
    unsigned long u4Index = 0;
    unsigned long u4BPP = 0;
    unsigned long u4InitOffset = 0;
    unsigned long u4Stride = 0;
    unsigned long u4ChunkSize = 0;
    unsigned long u4Case = 0;
    unsigned long u4BuffSize = 0;

//Debug
    unsigned long u4Src , u4Dst;

#endif
//Add for workaround a hardware limitation,ISP cannot handle odd width or < 16 pixel image  - 2012/1/10 -S

    struct MdpPathImageTransformParameter image_transform_param;

    if( p_parameter == 0 )
        return -1;


    MDP_INFO_PATH("{MdpPipeImageTransform_Func}"DEBUG_STR_BEGIN"\n");

    /***Parameter Convertion***/

    image_transform_param.src_color_format                =          p_parameter->src_color_format;
    image_transform_param.src_img_size                    =          p_parameter->src_img_size;
    image_transform_param.src_img_roi                     =          p_parameter->src_img_roi;
    image_transform_param.src_yuv_img_addr                =          p_parameter->src_yuv_img_addr;

    image_transform_param.dst_color_format                =          p_parameter->dst_color_format;
    image_transform_param.dst_img_size                    =          p_parameter->dst_img_size;
    image_transform_param.dst_img_roi                     =          p_parameter->dst_img_roi;
    image_transform_param.dst_yuv_img_addr                =          p_parameter->dst_yuv_img_addr;
    image_transform_param.dst_rotate_angle                =          p_parameter->dst_rotate_angle;


    /*Resizer coeff if can apply*/
    image_transform_param.resz_up_scale_coeff               =          p_parameter->resz_up_scale_coeff;
    image_transform_param.resz_dn_scale_coeff               =          p_parameter->resz_dn_scale_coeff;
    image_transform_param.resz_ee_h_str                     =          p_parameter->resz_ee_h_str;
    image_transform_param.resz_ee_v_str                     =          p_parameter->resz_ee_v_str;


    req_favor_flags = p_parameter->favor_flags;

    /*.............................................................................
        Decide whether or not to do dithering
      .............................................................................*/
    if(     ( p_parameter->src_color_format != p_parameter->dst_color_format ) &&

            !( ( ( p_parameter->src_color_format == RGB565 ) && ( p_parameter->dst_color_format == BGR565 ) ) ||
               ( ( p_parameter->src_color_format == BGR565 ) && ( p_parameter->dst_color_format == RGB565 ) ) )
       )
    {
        if( ( p_parameter->dst_color_format == RGB565 ) || ( p_parameter->dst_color_format == BGR565 ) )
        {
            image_transform_param.dst_dither_en = 1;
        } else
        {
            image_transform_param.dst_dither_en = 0;
        }
    } else
    {
        image_transform_param.dst_dither_en = 0;
    }

    /*.............................................................................
        Special Image Transform path : Image process path
      .............................................................................*/
    if(p_parameter->do_image_process)
    {
#if 0
        //CPU loading issue, 720P content performance might be not achieved if PQ is on.
        //960x540 = 518400
        if((UYVY_Pack == p_parameter->dst_color_format) && (518400 < (p_parameter->src_img_roi.w * p_parameter->src_img_roi.h)))
        {
//            MDP_DEBUG("Run normal path");
            goto _Normal_MdpPipeImageTransform_Func;
        }

        //A workaround for HW limitation : ISP cannot process odd width , width smaller than 16 and height smaller than 16
        if(((1 & p_parameter->src_img_roi.w) || (16 > p_parameter->src_img_roi.w) || (16 > p_parameter->src_img_roi.h)) &&
        	(p_parameter->src_yuv_img_addr.y == p_parameter->dst_yuv_img_addr.y) && (image_transform_param.dst_color_format < RGBA8888))
        {
            switch(image_transform_param.dst_color_format)
            {
                case RGB888 :
                case BGR888 :
                    u4BPP = 3;
                break;
                case RGB565 :
                case BGR565 :
                    u4BPP = 2;
                break;
                case ABGR8888 :
                case ARGB8888 :
                case BGRA8888 :
                case RGBA8888 :
                    u4BPP = 4;
                break;
                default :
                    free(pWorkingBuff);
                    goto _EXIT_MdpPipeImageTransform_Func;
                break;
            }

            memcpy(&tempParam , p_parameter , sizeof(MdpPathImageTransformParameter));

            //allocates 16 x image height memory space , suppose ARGB8888
            if(16 > p_parameter->src_img_roi.w)
            {
                if(16 > p_parameter->src_img_roi.h)
                {
                    //(w < 16, h < 16) + (1 & w , w < 16, h < 16) case
                    //16x16 box
                    u4BuffSize = (u4BPP<<8);
                    u4Case = 3;

                }
                else
                {
                    //(w < 16 , h > 16 , 1 & w) + (w < 16 , h > 16 , !(1 & w)) case
                    //H residule : residulex16
                    u4BuffSize = (p_parameter->src_img_roi.h << 4)*u4BPP;
                    u4Case = 1;
                }
            }
            else
            {
                if(16 > p_parameter->src_img_roi.h)
                {
                    //(h < 16) + (h < 16 , 1 & w) case
                    //V residule : 16xresidule
                    u4BuffSize = (1 & p_parameter->src_img_roi.w ? ((p_parameter->src_img_roi.w+1) << 4) : ((p_parameter->src_img_roi.w) << 4))*u4BPP;
                    u4Case = 2;
                }
                else
                {
                    // 1 & w case
                    //H residule : residulex16
                    u4BuffSize = (p_parameter->src_img_roi.h << 4)*u4BPP;
                    u4Case = 1;
                }
            }

            pWorkingBuff = malloc(u4BuffSize);
            if(NULL == pWorkingBuff)
            {
                MDP_ERROR("No space to do odd width post process , skip this one\n");
                goto _Normal_MdpPipeImageTransform_Func;
            }
            memset(pWorkingBuff , 0 , u4BuffSize);

            //process large part first
            tempParam.src_img_roi.w = ((p_parameter->src_img_roi.w >> 4) << 4);
            tempParam.dst_img_roi.w = tempParam.src_img_roi.w;
            u4ResidualWidth = (p_parameter->src_img_roi.w - tempParam.src_img_roi.w);

            if((0 < tempParam.src_img_roi.w) && (16 < tempParam.src_img_roi.h))
            {
                ret_val = MdpPathImageProcess(&tempParam , p_parameter->do_image_process);
                if(MDP_ERROR_CODE_OK != ret_val)
                {
                    free(pWorkingBuff);
                    MDP_DEBUG("Post proc failed in making main part");
                    goto _EXIT_MdpPipeImageTransform_Func;
                }
            }

            //Copy out
            //Use memcpy to avoid no DMA resource.
            if(1 == u4Case)
            {
                //H band
                u4InitOffset = (tempParam.src_img_roi.w + tempParam.src_img_roi.x + tempParam.src_img_roi.y*tempParam.src_img_size.w)*u4BPP;
                u4Stride = (u4BPP << 4);//16 pixels
                u4ChunkSize = (u4ResidualWidth*u4BPP);
            }
            else if(2 == u4Case)
            {
                //V band
                u4InitOffset = (tempParam.src_img_roi.x + tempParam.src_img_roi.y*tempParam.src_img_size.w)*u4BPP;
                u4Stride = (1 & p_parameter->src_img_roi.w ? (p_parameter->src_img_roi.w+1) : (p_parameter->src_img_roi.w))*u4BPP;
                u4ChunkSize = (p_parameter->src_img_roi.w*u4BPP);
            }
            else
            {
                // 16x16 box
                u4InitOffset = (tempParam.src_img_roi.x + tempParam.src_img_roi.y*tempParam.src_img_size.w)*u4BPP;
                u4Stride = (u4BPP << 4);
                u4ChunkSize = (p_parameter->src_img_roi.w*u4BPP);
            }

            for(u4Index = 0 ; u4Index < p_parameter->src_img_roi.h ; u4Index += 1)
            {
                memcpy((void *)((char *)pWorkingBuff + (u4Index*u4Stride)) , (void *)((char *)p_parameter->src_yuv_img_addr.y + u4InitOffset + u4Index*p_parameter->src_img_size.w*u4BPP) , u4ChunkSize);
            }

            //Padding in H for YUV422 process
            if(1 & p_parameter->src_img_roi.w)
            {
                for(u4Index = 0 ; u4Index < p_parameter->src_img_roi.h ; u4Index += 1)
                {
                    memcpy((void *)((char *)pWorkingBuff + u4ChunkSize + u4Index*u4Stride) ,
                    	(void *)((char *)pWorkingBuff + u4ChunkSize - u4BPP +  u4Index*u4Stride) ,
                    	u4BPP);
                }
            }

            //process residule part
            //to make sure it always success, we use memcpy.
            tempParam.src_img_roi.x = 0;
            tempParam.src_img_roi.y = 0;
            tempParam.src_img_roi.w = (1 & u4Case ? 16 : p_parameter->src_img_roi.w);// 0x1 or 0x3
            tempParam.src_img_roi.w = ((1 & tempParam.src_img_roi.w) ? (tempParam.src_img_roi.w + 1) : tempParam.src_img_roi.w);//Align to even
            tempParam.src_img_roi.h = (2 & u4Case ? 16 : p_parameter->src_img_roi.h);// 0x2 or 0x3
            tempParam.src_img_size.w = tempParam.src_img_roi.w;
            tempParam.src_img_size.h = tempParam.src_img_roi.h;

            tempParam.dst_img_roi.x = 0;
            tempParam.dst_img_roi.y = 0;
            tempParam.dst_img_roi.w = tempParam.src_img_roi.w;
            tempParam.dst_img_roi.h = tempParam.src_img_roi.h;
            tempParam.dst_img_size.w = tempParam.dst_img_roi.w;
            tempParam.dst_img_size.h = tempParam.dst_img_roi.h;

            tempParam.src_yuv_img_addr.y = (unsigned long)pWorkingBuff;
            tempParam.dst_yuv_img_addr.y = (unsigned long)pWorkingBuff;

//Debug - S
#if 0
{
    FILE *fp;
    char filename [60];
    sprintf(filename , "/dump/PartInn%d_%d_%d_%d_%d.raw" , g_Cnt , tempParam.dst_img_size.w , tempParam.dst_img_roi.h , u4ChunkSize , p_parameter->src_img_roi.h);
    fp = fopen( filename, "wb");
    unsigned long size = (tempParam.dst_img_size.w*tempParam.dst_img_roi.h*u4BPP);

    if(  fp == NULL )
    {
        MDP_ERROR("Open file failed.:%s\n", strerror(errno));
        goto _EXIT_MdpPipeImageTransform_Func;
    }

    if( fwrite( (void*)pWorkingBuff , 1 , size , fp) < size )
    {
        MDP_ERROR("write file failed.:%s\n", strerror(errno));
    }

    fclose(fp);
    MDP_ERROR("Part dump %d : pitch:%d , vp:%d , w:%d , h : %d" , g_Cnt , tempParam.dst_img_size.w , tempParam.dst_img_roi.h , u4ChunkSize , p_parameter->src_img_roi.h);
}
#endif
//Debug - E

            ret_val = MdpPathImageProcess(&tempParam , p_parameter->do_image_process);
            if(MDP_ERROR_CODE_OK != ret_val)
            {
                free(pWorkingBuff);
                MDP_DEBUG("Post proc failed in making residual part");
                goto _EXIT_MdpPipeImageTransform_Func;
            }

            //Copy in
            //Use memcpy to avoid no DMA resource.
            for(u4Index = 0 ; u4Index < p_parameter->src_img_roi.h ; u4Index += 1)
            {
                memcpy((void *)((char *)p_parameter->src_yuv_img_addr.y + u4InitOffset + u4Index*p_parameter->src_img_size.w*u4BPP) , (void *)((char *)pWorkingBuff + (u4Index*u4Stride)) , u4ChunkSize);
            }

//Debug - S
#if 0
{
    FILE *fp;
    char filename [60];
    sprintf(filename , "/dump/PartOut%d_%d_%d_%d_%d.raw" , g_Cnt , tempParam.dst_img_size.w , tempParam.dst_img_roi.h , u4ChunkSize , p_parameter->src_img_roi.h);
    fp = fopen( filename, "wb");
    unsigned long size = (tempParam.dst_img_size.w*tempParam.dst_img_roi.h*u4BPP);

    if(  fp == NULL )
    {
        MDP_ERROR("Open file failed.:%s\n", strerror(errno));
        goto _EXIT_MdpPipeImageTransform_Func;
    }

    if( fwrite( (void*)pWorkingBuff , 1 , size , fp) < size )
    {
        MDP_ERROR("write file failed.:%s\n", strerror(errno));
    }

    fclose(fp);
    MDP_ERROR("Part dump %d : pitch:%d , vp:%d , w:%d , h : %d" , g_Cnt , tempParam.dst_img_size.w , tempParam.dst_img_roi.h , u4ChunkSize , p_parameter->src_img_roi.h);
    g_Cnt += 1;
}
#endif
//Debug - E

            //free memory
            free(pWorkingBuff);
        }
        else
        {
            ret_val = MdpPathImageProcess(&image_transform_param , p_parameter->do_image_process);
        }

#else
        ret_val = MdpPathImageProcess(&image_transform_param , p_parameter->do_image_process);
#endif

        goto _EXIT_MdpPipeImageTransform_Func;
    }

_Normal_MdpPipeImageTransform_Func:
    /*.............................................................................
        Normal Image Transform path
      .............................................................................*/
    switch( image_transform_param.dst_color_format )
    {


    case Y411_Pack://YUV411, 4x4 sub sample U/V plane,only ROTDMA0
    case YV16_Planar://YUV422, 2x1 subsampled U/V planes,only ROTDMA0
    case YV12_Planar://YUV420, 2x2 subsampled U/V planes,only ROTDMA0
    case ANDROID_YV12:
    case IMG_YV12:
    case Y800://Y plan only,only ROTDMA0
    case YUV420_4x4BLK://For encoder use,only ROTDMA0
    case NV12://YUV420, 2x2 subsampled , interleaved U/V plane,only ROTDMA0
    case NV21://YUV420, 2x2 subsampled , interleaved V/U plane,only ROTDMA0
        for( retry = 0; retry < IT_PATH_RETRY_COUNT; retry++ )
        {
            IMAGE_TRANSFORM_ROUTE( MdpPathImageTransformYuv_3, image_transform_param , req_favor_flags, ITFF_USE_CRZ );
            IMAGE_TRANSFORM_ROUTE( MdpPathImageTransformYuv_2, image_transform_param , req_favor_flags, NULL );
            IMAGE_TRANSFORM_ROUTE( MdpPathImageTransformYuv_1, image_transform_param , req_favor_flags, NULL );
        }

        //MDP_ERROR("No Imagetransform path for YUV Plane transform. Favor Flags = 0x%08X.\n",(unsigned int)req_favor_flags);

        MDP_ERROR("No IT path for YUV Plane transform. Favor Flags = 0x%08X src=(%d, %dx%d,(%d,%d,%d,%d)) dst=(%d, %dx%d,(%d,%d,%d,%d),%d) \n",
        (unsigned int)req_favor_flags ,
        (int)image_transform_param.src_color_format,
        (int)image_transform_param.src_img_size.w, (int)image_transform_param.src_img_size.h,
        (int)image_transform_param.src_img_roi.x,(int)image_transform_param.src_img_roi.y,(int)image_transform_param.src_img_roi.w,(int)image_transform_param.src_img_roi.h,
        (int)image_transform_param.dst_color_format,
        (int)image_transform_param.dst_img_size.w, (int)image_transform_param.dst_img_size.h,
        (int)image_transform_param.dst_img_roi.x,(int)image_transform_param.dst_img_roi.y,(int)image_transform_param.dst_img_roi.w,(int)image_transform_param.dst_img_roi.h,
        (int)image_transform_param.dst_rotate_angle
        );


        ret_val = -1;

        break;

    case RGB888:
    case BGR888:
    case RGB565:
    case BGR565:
    case ABGR8888:
    case ARGB8888:
    case BGRA8888:
    case RGBA8888:
        for( retry = 0; retry < IT_PATH_RETRY_COUNT; retry++ )
        {
            IMAGE_TRANSFORM_ROUTE( MdpPathImageTransformRgb_4, image_transform_param , req_favor_flags, ITFF_USE_CRZ );
            IMAGE_TRANSFORM_ROUTE( MdpPathImageTransformRgb_3, image_transform_param , req_favor_flags, NULL );
            IMAGE_TRANSFORM_ROUTE( MdpPathImageTransformRgb_1, image_transform_param , req_favor_flags, NULL );
        }

        //MDP_ERROR("No Imagetransform path for RGB transform. Favor Flags = 0x%08X.\n",(unsigned int)req_favor_flags);

        MDP_ERROR("No IT path for RGB Plane transform. Favor Flags = 0x%08X src=(%d, %lux%lu,(%lu,%lu,%lu,%lu)) dst=(%d, %lux%lu,(%lu,%lu,%lu,%lu),%lu) \n",
        (unsigned int)req_favor_flags ,
        image_transform_param.src_color_format,
        image_transform_param.src_img_size.w, image_transform_param.src_img_size.h,
        image_transform_param.src_img_roi.x,image_transform_param.src_img_roi.y,image_transform_param.src_img_roi.w,image_transform_param.src_img_roi.h,
        image_transform_param.dst_color_format,
        image_transform_param.dst_img_size.w, image_transform_param.dst_img_size.h,
        image_transform_param.dst_img_roi.x,image_transform_param.dst_img_roi.y,image_transform_param.dst_img_roi.w,image_transform_param.dst_img_roi.h,
        image_transform_param.dst_rotate_angle
        );

        ret_val = -1;

        break;

    default: /*Other..packed YUV*/
        for( retry = 0; retry < IT_PATH_RETRY_COUNT; retry++ )
        {
        IMAGE_TRANSFORM_ROUTE( MdpPathImageTransformRgb_3, image_transform_param , req_favor_flags, NULL );
        IMAGE_TRANSFORM_ROUTE( MdpPathImageTransformRgb_1, image_transform_param , req_favor_flags, NULL );
        IMAGE_TRANSFORM_ROUTE( MdpPathImageTransformYuv_2, image_transform_param , req_favor_flags, NULL );
        IMAGE_TRANSFORM_ROUTE( MdpPathImageTransformYuv_1, image_transform_param , req_favor_flags, NULL );


        IMAGE_TRANSFORM_ROUTE( MdpPathImageTransformRgb_4, image_transform_param , req_favor_flags, ITFF_USE_CRZ );
        IMAGE_TRANSFORM_ROUTE( MdpPathImageTransformYuv_3, image_transform_param , req_favor_flags, ITFF_USE_CRZ );
        }

        //MDP_ERROR("No Imagetransform path for ANY transform. dst color = 0x%X Favor Flags = 0x%08X.\n",
        //    (unsigned int)image_transform_param.dst_color_format, (unsigned int)req_favor_flags);


        MDP_ERROR("No IT path for YUV Pack transform. Favor Flags = 0x%08X src=(%d, %lux%lu,(%lu,%lu,%lu,%lu)) dst=(%d, %lux%lu,(%lu,%lu,%lu,%lu),%lu) \n",
        (unsigned int)req_favor_flags ,
        image_transform_param.src_color_format,
        image_transform_param.src_img_size.w, image_transform_param.src_img_size.h,
        image_transform_param.src_img_roi.x,image_transform_param.src_img_roi.y,image_transform_param.src_img_roi.w,image_transform_param.src_img_roi.h,
        image_transform_param.dst_color_format,
        image_transform_param.dst_img_size.w, image_transform_param.dst_img_size.h,
        image_transform_param.dst_img_roi.x,image_transform_param.dst_img_roi.y,image_transform_param.dst_img_roi.w,image_transform_param.dst_img_roi.h,
        image_transform_param.dst_rotate_angle
        );


        ret_val = -1;
        break;
    }



/*Exit function label*/
_EXIT_MdpPipeImageTransform_Func:


    MDP_INFO(DEBUG_STR_END"MdpPipeImageTransform_Func""\n");

    return ret_val;

}




