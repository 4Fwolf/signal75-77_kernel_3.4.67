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

#include "mhal_interface.h"


#include "mdp_pipe.h"




/*-----------------------------------------------------------------------------
    Interface Helper Function
    (Internal Use)
  -----------------------------------------------------------------------------*/
MdpColorFormat          MhalColorFormatToMdp( MHAL_BITBLT_FORMAT_ENUM mhal_bitblt_color_format )
{
    switch( mhal_bitblt_color_format )
    {
        case MHAL_FORMAT_RGB_565 :
            return RGB565;
        case MHAL_FORMAT_BGR_565 :
            return BGR565;
        case MHAL_FORMAT_RGB_888 :
            return RGB888;
        case MHAL_FORMAT_BGR_888 :
            return BGR888;
        case MHAL_FORMAT_ARGB_8888 :
            return ARGB8888;
        case MHAL_FORMAT_ABGR_8888 :
            return ABGR8888;
        case MHAL_FORMAT_BGRA_8888:
            return BGRA8888;
        case MHAL_FORMAT_RGBA_8888:
            return RGBA8888;
        case MHAL_FORMAT_YUV_420 :
            return YV12_Planar; /*Original YV12*/
            //return ANDROID_YV12; /*Android YV12 for Test*/
        case MHAL_FORMAT_YUV_420_SP :
            return NV21;
        case MHAL_FORMAT_MTK_YUV :
            return YUV420_4x4BLK;
        case MHAL_FORMAT_YUY2 :
            return YUY2_Pack;
        case MHAL_FORMAT_UYVY :
            return UYVY_Pack;
        case MHAL_FORMAT_Y800:
            return Y800;
        case MHAL_FORMAT_YUV_422_PL://422 Planar,i.e YV16 Planar
            return YV16_Planar;
        case MHAL_FORMAT_ANDROID_YV12:   //Androdi YV12.YVU stride all 16 pixel align
            return ANDROID_YV12;
        case MHAL_FORMAT_IMG_YV12:       //Imagination YV12.YVU stride all 32 pixel align
            return IMG_YV12;
           
            

            
        default :
     	     MDP_ERROR("Unsupport mhal bitblt color format 0x%x" , mhal_bitblt_color_format );
             return (MdpColorFormat)-1;
        break;
    }

    return (MdpColorFormat)-1;
    
}


MHAL_BITBLT_FORMAT_ENUM MdpColorFormatToMhal( MdpColorFormat mdp_color )
{
    switch( mdp_color )
    {
        case RGB565:
            return MHAL_FORMAT_RGB_565 ;
        case BGR565:
            return MHAL_FORMAT_BGR_565 ;
        case RGB888:
            return MHAL_FORMAT_RGB_888 ;
        case BGR888:
            return MHAL_FORMAT_BGR_888 ;
        case ARGB8888:
            return MHAL_FORMAT_ARGB_8888 ;
        case ABGR8888:
            return MHAL_FORMAT_ABGR_8888 ;
        case BGRA8888:
            return MHAL_FORMAT_BGRA_8888;
        case RGBA8888:
            return MHAL_FORMAT_RGBA_8888;
        case YV12_Planar:
            return MHAL_FORMAT_YUV_420 ; /*Original YV12*/
            //return ANDROID_YV12; /*Android YV12 for Test*/
        case NV21:
            return MHAL_FORMAT_YUV_420_SP ;
        case YUV420_4x4BLK:
            return MHAL_FORMAT_MTK_YUV ;
        case YUY2_Pack:
            return MHAL_FORMAT_YUY2 ;
        case UYVY_Pack:
            return MHAL_FORMAT_UYVY ;
        case Y800:
            return MHAL_FORMAT_Y800;
        case YV16_Planar://422 Planar,i.e YV16 Planar
            return MHAL_FORMAT_YUV_422_PL;
        case ANDROID_YV12:   //Androdi YV12.YVU stride all 16 pixel align
            return MHAL_FORMAT_ANDROID_YV12;
        case IMG_YV12:       //Imagination YV12.YVU stride all 32 pixel align
            return MHAL_FORMAT_IMG_YV12;
           
            
        default :
     	     MDP_ERROR("Unsupport mdp color format 0x%x" , mdp_color );
             return (MHAL_BITBLT_FORMAT_ENUM)-1;
        break;
    }

    return (MHAL_BITBLT_FORMAT_ENUM)-1;
    
}



/*-----------------------------------------------------------------------------
    Interface to upper client
  -----------------------------------------------------------------------------*/

/**
    mHal should call mdp_pipe function call, which is a set of platform independent function call,
    thus, mHal implementation should be platform independent,
    no need to add mt6575 prefix.

    the mt6575 prefix is due to 73 mhal is platform dependent,
    so I add this prefix for temporary
    **/
int Mt6575_mHalBitblt(void *a_pInBuffer) /*mt6575 prefix should not add, this should be platform independent implement*/
{
    int                                     ret_val = 0;
    mHalBltParam_t                          *bltParam;
    mHalBltParam_t                          param;
    MdpPipeImageTransformParameter          mdp_pipe_param;
    unsigned long _w,_h,_pitch;
    unsigned long dst_img_size_w;
    unsigned long dst_img_size_h;

    //Performance Test
    
    #if defined(MDP_FLAG_PROFILING)
    MdpDrv_Watch _MdpDrvWatch;
    static unsigned long total_time = 0;
    static unsigned long frame_count = 0;
    static unsigned long avg_time_elapse = 0;
    #endif



    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_IT );

    

    if( a_pInBuffer == 0 )
        return -1;

    /*MDP Driver initial for each process*/
    /*(or just the first time used)      */
    if( MdpDrvInit() < 0 )
    {
        MdpDrvRelease(); 
        return -1;
    }

    bltParam = (mHalBltParam_t*)a_pInBuffer;

    /*Structure Copy*/
    param = *bltParam;

    /*-----------------------------------------------------------------------------
        For backward compatible:
        Change back width and hiegh when rotate.
      -----------------------------------------------------------------------------*/
    _w      = param.dstW;
    _h      = param.dstH;
    _pitch  = param.pitch;
    
    if ( param.orientation & 0x1 ) {/*90/270 rotate*/
        param.dstW       =  _h; //set w/h as w/h before rotate
        param.dstH       =  _w;
        dst_img_size_w      =  param.dstW;
        dst_img_size_h      =  _pitch;
    } else
    {
        param.dstW       =  _w; //set w/h as w/h before rotate
        param.dstH       =  _h;
        dst_img_size_w      =  _pitch;
        dst_img_size_h      =  param.dstH;
    }

    /*-----------------------------------------------------------------------------*/

    

    /*Source Param Conversion*/
    mdp_pipe_param.src_color_format = MhalColorFormatToMdp( (MHAL_BITBLT_FORMAT_ENUM)param.srcFormat );

    mdp_pipe_param.src_img_size.w = param.srcWStride;
    mdp_pipe_param.src_img_size.h = param.srcHStride;

    mdp_pipe_param.src_img_roi.x = param.srcX;
    mdp_pipe_param.src_img_roi.y = param.srcY;
    mdp_pipe_param.src_img_roi.w = param.srcW;
    mdp_pipe_param.src_img_roi.h = param.srcH;

    mdp_pipe_param.src_yuv_img_addr.y = param.srcAddr;
    mdp_pipe_param.src_yuv_img_addr.m4u_handle  = param.m4u_handle;
        

    /*Dest 0 Param Conversion*/
    mdp_pipe_param.dst_color_format = MhalColorFormatToMdp( (MHAL_BITBLT_FORMAT_ENUM)param.dstFormat );

    mdp_pipe_param.dst_img_size.w = dst_img_size_w;
    mdp_pipe_param.dst_img_size.h = dst_img_size_h;//TODO:actually UNKNOWN from mhal interface
    
    mdp_pipe_param.dst_img_roi.x = 0; //there is no roi offeset info from mhal interface, dst starting address is the start address
    mdp_pipe_param.dst_img_roi.y = 0; //of roi region. thus roi offset is always 0
    mdp_pipe_param.dst_img_roi.w = param.dstW;
    mdp_pipe_param.dst_img_roi.h = param.dstH;
    
    mdp_pipe_param.dst_yuv_img_addr.y = param.dstAddr;
    mdp_pipe_param.dst_yuv_img_addr.m4u_handle  = param.m4u_handle;
    
    
    mdp_pipe_param.dst_rotate_angle = param.orientation;

//    mdp_pipe_param.do_image_process = (param.doImageProcess == 1) ? true : false;
    mdp_pipe_param.do_image_process = param.doImageProcess;

    mdp_pipe_param.favor_flags = param.favor_flags;

    
    mdp_pipe_param.resz_up_scale_coeff = param.resz_up_scale_coeff;    
    mdp_pipe_param.resz_dn_scale_coeff = param.resz_dn_scale_coeff;
    mdp_pipe_param.resz_ee_h_str = param.resz_ee_h_str;
    mdp_pipe_param.resz_ee_v_str = param.resz_ee_v_str;

    /*.............................................................................*/
    /*Show Info*/
    MDP_INFO_PATH("[mHalBitblt Parameters]-----------------------------------------------------------------------------\n");

    MDP_INFO("m4u_handle: 0x%08x\n", (unsigned int)mdp_pipe_param.src_yuv_img_addr.m4u_handle );

    MDP_INFO("src color: %d\n", (unsigned int)mdp_pipe_param.src_color_format );
    MDP_INFO("src w: %d    h:%d\n", (unsigned int)mdp_pipe_param.src_img_size.w , (unsigned int)mdp_pipe_param.src_img_size.h);
    MDP_INFO("src roi x: %d    y:%d\n", (unsigned int)mdp_pipe_param.src_img_roi.x , (unsigned int)mdp_pipe_param.src_img_roi.y);
    MDP_INFO("src roi w: %d    h:%d\n", (unsigned int)mdp_pipe_param.src_img_roi.w , (unsigned int)mdp_pipe_param.src_img_roi.h);
    MDP_INFO("src y address: 0x%08x\n", (unsigned int)mdp_pipe_param.src_yuv_img_addr.y );

    MDP_INFO("dst color: %d\n", (unsigned int)mdp_pipe_param.dst_color_format );
    MDP_INFO("dst w: %d    h:%d\n", (unsigned int)mdp_pipe_param.dst_img_size.w , (unsigned int)mdp_pipe_param.dst_img_size.h);
    MDP_INFO("dst roi x: %d    y:%d\n", (unsigned int)mdp_pipe_param.dst_img_roi.x , (unsigned int)mdp_pipe_param.dst_img_roi.y);
    MDP_INFO("dst roi w: %d    h:%d\n", (unsigned int)mdp_pipe_param.dst_img_roi.w , (unsigned int)mdp_pipe_param.dst_img_roi.h);
    MDP_INFO("dst y address: 0x%08x\n", (unsigned int)mdp_pipe_param.dst_yuv_img_addr.y );
    MDP_INFO("dst rotate: %d\n", (unsigned int)mdp_pipe_param.dst_rotate_angle );

    MDP_INFO("do image process: %d\n", (unsigned int)mdp_pipe_param.do_image_process );
    
    MDP_INFO("favor_flags: 0x%08X\n", (unsigned int)mdp_pipe_param.favor_flags );

    
    MDP_INFO("resz_up_scale_coeff: %d\n", (unsigned int)mdp_pipe_param.resz_up_scale_coeff );
    MDP_INFO("resz_dn_scale_coeff: %d\n", (unsigned int)mdp_pipe_param.resz_dn_scale_coeff );
    MDP_INFO("resz_ee_h_str: %d\n", (unsigned int)mdp_pipe_param.resz_ee_h_str );
    MDP_INFO("resz_ee_v_str: %d\n", (unsigned int)mdp_pipe_param.resz_ee_v_str );
    /*.............................................................................*/

    
    /*Call Pipe Function*/
    ret_val = MdpPipeImageTransform_Func( &mdp_pipe_param );

    
    MDPDRV_WATCHSTOP( MDPDRV_WATCH_LEVEL_IT, "Image Transform", &total_time, &frame_count, &avg_time_elapse, 30 );
    


    /*MDP Driver release for each process, comment out for speed up.*/
    /*ie. mdp driver will remain open.                              */
    //MdpDrvRelease();  //comment out this for performance

    return ret_val;
    
}





int Mt6575_mHalCameraPreview( halIDPParam_t *phalIDPParam )
{
    
#define _CPP_VERSION__

    int retval;
    MdpPipeCameraPreviewParameter mdp_pipe_param;

    #ifdef _CPP_VERSION__
    MdpPipeCameraPreview    mdp_pipe_camera_preview;
    #endif


    /*------------------------------------------------*/
    /*Thread 1 : MdpPathCameraPreviewToMemoryParameter*/
    /*------------------------------------------------*/
    mdp_pipe_param.pseudo_src_enable =   phalIDPParam->test_pseudo_src_enable; //Use RDMA0 to take place of camera sensor as pseudo source          
    mdp_pipe_param.pseudo_src_color_format = phalIDPParam->test_pseudo_src_color_format;

    mdp_pipe_param.pseudo_src_yuv_img_addr.y = phalIDPParam->test_pseudo_src_yuv_img_addr;
    

    mdp_pipe_param.src_img_size.w = phalIDPParam->src_size_w;
    mdp_pipe_param.src_img_size.h = phalIDPParam->src_size_h;
    
    mdp_pipe_param.src_img_roi.x = 0;
    mdp_pipe_param.src_img_roi.y = 0;
    mdp_pipe_param.src_img_roi.w = mdp_pipe_param.src_img_size.w;
    mdp_pipe_param.src_img_roi.h = mdp_pipe_param.src_img_size.h;
    

    mdp_pipe_param.dst_color_format = RGB888;

    mdp_pipe_param.dst_img_size.w = phalIDPParam->src_size_w;
    mdp_pipe_param.dst_img_size.h = phalIDPParam->src_size_h;

    mdp_pipe_param.dst_img_roi.x = 0;
    mdp_pipe_param.dst_img_roi.y = 0; 
    mdp_pipe_param.dst_img_roi.w = mdp_pipe_param.dst_img_size.w;
    mdp_pipe_param.dst_img_roi.h = mdp_pipe_param.dst_img_size.h;

    #if 0 /*Normal Pipe( Thread 1 + Thread 2 ) Setting*/
    mdp_pipe_param.dst_yuv_img_addr.y = phalIDPParam->working_buffer_addr;
    #else/*Test Path , Thread 2 Only */
    mdp_pipe_param.dst_yuv_img_addr.y = phalIDPParam->test_pseudo_src_yuv_img_addr;
    #endif
    


    mdp_pipe_param.dst_rotate_angle = 0;
    
    /*------------------------------------------------*/
    /*Thread 2 : MdpPathDisplayFromMemory*/
    /*------------------------------------------------*/
    /*For Display : NV21*/
    mdp_pipe_param.dst0_color_format = phalIDPParam->dst_port0.color_format;

    mdp_pipe_param.dst0_img_size.w = phalIDPParam->dst_port0.size_w;       /*Currently, "dst roi"  should be equal to "dst size"*/
    mdp_pipe_param.dst0_img_size.h = phalIDPParam->dst_port0.size_h;

    mdp_pipe_param.dst0_img_roi.x = 0;        /*dst_img_roi is not used*/
    mdp_pipe_param.dst0_img_roi.y = 0;
    mdp_pipe_param.dst0_img_roi.w = mdp_pipe_param.dst0_img_size.w;
    mdp_pipe_param.dst0_img_roi.h = mdp_pipe_param.dst0_img_size.h;
    
    mdp_pipe_param.dst0_yuv_img_addr.y = phalIDPParam->dst_port0.buffer_addr;
    
    
    mdp_pipe_param.dst0_rotate_angle = phalIDPParam->dst_port0.rotate;

    /*For Encode : YV12*/
    mdp_pipe_param.dst1_color_format = phalIDPParam->dst_port1.color_format ;

    mdp_pipe_param.dst1_img_size.w = phalIDPParam->dst_port1.size_w;       /*Currently, "dst roi"  should be equal to "dst size"*/
    mdp_pipe_param.dst1_img_size.h = phalIDPParam->dst_port1.size_h;
    
    mdp_pipe_param.dst1_img_roi.x = 0;        /*dst_img_roi is not used*/
    mdp_pipe_param.dst1_img_roi.y = 0;
    mdp_pipe_param.dst1_img_roi.w = mdp_pipe_param.dst1_img_size.w;
    mdp_pipe_param.dst1_img_roi.h = mdp_pipe_param.dst1_img_size.h;
    
    mdp_pipe_param.dst1_yuv_img_addr.y = phalIDPParam->dst_port1.buffer_addr;
    
    mdp_pipe_param.dst1_rotate_angle = phalIDPParam->dst_port1.rotate;

    /*For FD : RGB*/
    mdp_pipe_param.dst2_color_format = phalIDPParam->dst_port2.color_format ;

    mdp_pipe_param.dst2_img_size.w = phalIDPParam->dst_port2.size_w;       /*Currently, "dst roi"  should be equal to "dst size"*/
    mdp_pipe_param.dst2_img_size.h = phalIDPParam->dst_port2.size_h;

    mdp_pipe_param.dst2_img_roi.x = 0;        /*dst_img_roi is not used*/
    mdp_pipe_param.dst2_img_roi.y = 0;
    mdp_pipe_param.dst2_img_roi.w = mdp_pipe_param.dst2_img_size.w;
    mdp_pipe_param.dst2_img_roi.h = mdp_pipe_param.dst2_img_size.h;
    
    mdp_pipe_param.dst2_yuv_img_addr.y = phalIDPParam->dst_port2.buffer_addr;
        
    mdp_pipe_param.dst2_rotate_angle  = phalIDPParam->dst_port2.rotate;



    /*Show Info*/
    MDP_INFO("src y address = 0x%08x\n", (unsigned int)mdp_pipe_param.pseudo_src_yuv_img_addr.y );
    MDP_INFO("working y address = 0x%08x\n", (unsigned int)mdp_pipe_param.dst_yuv_img_addr.y );
    MDP_INFO("dst 0 y address = 0x%08x\n", (unsigned int)mdp_pipe_param.dst0_yuv_img_addr.y );
    MDP_INFO("dst 1 y address = 0x%08x\n", (unsigned int)mdp_pipe_param.dst1_yuv_img_addr.y );
    MDP_INFO("dst 2 y address = 0x%08x\n", (unsigned int)mdp_pipe_param.dst2_yuv_img_addr.y );


    /*Call Pipe Function*/
    
    #ifdef _CPP_VERSION__

    MDP_INFO("C++ Version\n");

    retval = mdp_pipe_camera_preview.Config( &mdp_pipe_param );
    retval = mdp_pipe_camera_preview.Start( NULL);
    retval = mdp_pipe_camera_preview.WaitBusy( NULL);
    retval = mdp_pipe_camera_preview.End( NULL);

    #else

    retval = MdpPipeCameraPreview_Func( &mdp_pipe_param );
    
    #endif




    return retval;
    
}


/*-----------------------------------------------------------------------------
    Lock/UnLock CRZ Line Buffer
  -----------------------------------------------------------------------------*/
static struct MdpPathStnrParameter  g_stnr_param;
static MdpPathStnr                  g_mdppath_stnr;


int Mt6575_mHalCrzLbLock( void )
{
    int ret = 0;
    
    /*MDP Driver initial for each process*/
    /*(or just the first time used)      */
    if( MdpDrvInit() < 0 )
    {
        MdpDrvRelease(); 
        return -1;
    }

    
    g_stnr_param.b_crz_use_line_buffer  = 0;
    g_stnr_param.b_prz0_use_line_buffer = 1;
    g_stnr_param.b_prz1_use_line_buffer = 1;
    
    if( g_mdppath_stnr.Config(&g_stnr_param) < 0 )//Config
    {
        return -1;
    }
    
    if( g_mdppath_stnr.Start(NULL) < 0 )//Execute
    {
        return -1;
    }
    
            /*Do what you want to do between start() & end()*/
            //g_mdppath_stnr.DumpRegister(NULL);

    return 0;
    
    
}

int Mt6575_mHalCrzLbUnLock( void )
{

    
    g_mdppath_stnr.End(NULL); //Release resource
    
    /*MDP Driver release for each process, comment out for speed up.*/
    /*ie. mdp driver will remain open.                              */
    //MdpDrvRelease();  //comment out this for performance

    return 0;
}



static void mHalMdp_PrintOut_RegisterLoopMemory_t( RegisterLoopMemory_t* p_param )
{
    if( p_param == NULL )   return;

    MDP_PRINTF("================================================\n");
    MDP_PRINTF("<%s>:\n","RegisterLoopMemory_t" );   
    MDP_PRINTF("================================================\n");

    MDP_DUMP_VAR_D( p_param->mem_type);
    MDP_DUMP_VAR_H( p_param->addr);
    MDP_DUMP_VAR_H( p_param->buffer_size);
    MDP_DUMP_VAR_D( p_param->mhal_color);

    MDP_DUMP_VAR_D( p_param->img_size.w);
    MDP_DUMP_VAR_D( p_param->img_size.h);

    MDP_DUMP_VAR_D( p_param->img_roi.x);
    MDP_DUMP_VAR_D( p_param->img_roi.y);
    MDP_DUMP_VAR_D( p_param->img_roi.w);
    MDP_DUMP_VAR_D( p_param->img_roi.h);

    MDP_DUMP_VAR_D( p_param->rotate); 

    
}


static void mHalMdp_PrintOut_RegisterLoopMemoryObj_t( RegisterLoopMemoryObj_t* p_param )
{
    if( p_param == NULL )   return;

    
    MDP_PRINTF("================================================\n");
    MDP_PRINTF("<%s>:\n","RegisterLoopMemoryObj_t" );   
    MDP_PRINTF("================================================\n");

    MDP_DUMP_VAR_H( p_param->mdp_id);

    MDP_DUMP_VAR_H( p_param->calc_addr[0].y);
    MDP_DUMP_VAR_H( p_param->calc_addr[0].u);
    MDP_DUMP_VAR_H( p_param->calc_addr[0].v);

    MDP_DUMP_VAR_H( p_param->adapt_addr[0].y);
    MDP_DUMP_VAR_H( p_param->adapt_addr[0].u);
    MDP_DUMP_VAR_H( p_param->adapt_addr[0].v);

    MDP_DUMP_VAR_D( p_param->adapt_m4u_flag_bit);
    MDP_DUMP_VAR_D( p_param->alloc_mva_flag_bit);
}


int Mdp_RegisterLoopMemory( MVALOOPMEM_CLIENT client_id, RegisterLoopMemory_t* p_param, RegisterLoopMemoryObj_t* p_out_obj )
{
    
#if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)

    //unsigned long   mdp_id;
    unsigned long   calc_buffer_size = 0;
    MdpColorFormat  mdp_color;
    //MdpYuvAddr      calc_addr[1];
    //MdpYuvAddr      adapt_addr[1];
    //unsigned long   adapt_m4u_flag_bit;
    int             rotate;
    
    


    if( p_param == NULL ){        MDP_ERROR("p_param = NULL@ register\n");     return -1;    }
    if( p_out_obj == NULL ){      MDP_ERROR("p_out_obj = NULL@ register\n");   return -1;    }

    if( (mdp_color = MhalColorFormatToMdp( p_param->mhal_color )) == (MdpColorFormat)-1 ){
        MDP_ERROR("unknow color when register loop memory.\n");
        mHalMdp_PrintOut_RegisterLoopMemory_t( p_param );
        
        return -1;
    }

    MDP_INFO_PERSIST("mHalMdp_RegisterLoopMemory(): client id:%d VA:0x%08X size:0x%X color:%d size:(%dx%d) roi:(%d %d %d %d) rotate:%d\n", 
        (int)client_id,
        (unsigned int)p_param->addr,
        (unsigned int)p_param->buffer_size,
        (int)mdp_color,
        (int)p_param->img_size.w,(int)p_param->img_size.h,
        (int)p_param->img_roi.x,(int)p_param->img_roi.y,(int)p_param->img_roi.w,(int)p_param->img_roi.h,
        (int)p_param->rotate );
        

    
    /*MDP Driver initial for each process*/
    /*(or just the first time used)      */
    if( MdpDrvInit() < 0 )
    {
        MdpDrvRelease(); 
        
        return -1;
    }
    

    p_out_obj->mdp_id = (p_param->mem_type == MEM_TYPE_INPUT)? MID_RDMA_GENERAL : MID_ROTDMA_GENERAL;

    rotate = (p_param->rotate == MEM_TYPE_INPUT)? 0 : p_param->rotate;

    p_out_obj->calc_addr[0].y = p_param->addr;

    
    /*-----------------------------------------------------------------------------
        Calculate buffer size and uv address
      -----------------------------------------------------------------------------*/
 
    if( MdpDrv_CalImgBufferArraySizeAndFillIn( 
                        mdp_color, 
                        p_out_obj->calc_addr,
                        1,
                        p_param->img_size,
                        p_param->img_roi,
                        rotate, 
                        &calc_buffer_size ) < 0 )
    {
        MDP_ERROR("Calc  memory size error.\n");
        mHalMdp_PrintOut_RegisterLoopMemory_t( p_param );
        
        return -1;
    }


    if( calc_buffer_size > p_param->buffer_size  )
    {
        MDP_ERROR("Calc buffer size(0x%08X) > buffer_size(0x%08X)\n",(unsigned int)calc_buffer_size, (unsigned int)p_param->buffer_size );
        mHalMdp_PrintOut_RegisterLoopMemory_t( p_param );
        
        return -1;
    }else if( calc_buffer_size < p_param->buffer_size )
    {
        MDP_WARNING("Calc buffer size(0x%08X) < buffer_size(0x%08X)\n",(unsigned int)calc_buffer_size, (unsigned int)p_param->buffer_size );
    }



    /*-----------------------------------------------------------------------------
        Adapt source image address
      -----------------------------------------------------------------------------*/
      
    if( MdpDrv_AdaptMdpYuvAddrArray(    p_out_obj->mdp_id, 
                                        p_out_obj->calc_addr, 1, //Original user input address  
                                        p_out_obj->adapt_addr,   //Output adapted address
                                        &p_out_obj->adapt_m4u_flag_bit,             //If address has been adapted with m4u mva, corresponding bit will set to 1
                                        &p_out_obj->alloc_mva_flag_bit ) != 0 )    //Corresponding bit will set to 1 if the mva is new allocated
    {
        MDP_ERROR("Adapt Memory error.\n");
        mHalMdp_PrintOut_RegisterLoopMemory_t( p_param );
        
        return -1;
    }

    
    MDP_INFO_PERSIST("mHalMdp_RegisterLoopMemory():MVA Obj Create:client id:%d 0x%08X(VA) 0x%08X(MVA) 0x%08X(M4U flag) 0x%08X(alloc MVA flag)\n",
        (int)client_id,
        (unsigned int)p_out_obj->calc_addr[0].y ,(unsigned int)p_out_obj->adapt_addr[0].y ,
        (unsigned int)p_out_obj->adapt_m4u_flag_bit,(unsigned int)p_out_obj->adapt_m4u_flag_bit );


    if( p_out_obj->adapt_m4u_flag_bit != 1 )
    {
        MDP_ERROR("Pool Memory register failed\n");
        mHalMdp_PrintOut_RegisterLoopMemory_t( p_param );
        
        return -1;
    }

    //Track
    MdpDrv_MvaLoopMemTrack( client_id, MLMT_ACTION_ALLOC, 
        p_out_obj->adapt_addr[0].y_buffer_size + p_out_obj->adapt_addr[0].u_buffer_size + p_out_obj->adapt_addr[0].v_buffer_size );
    

    #endif /*#if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)*/

    
    return 0;
    
    
}

int Mdp_UnRegisterLoopMemory( MVALOOPMEM_CLIENT client_id, RegisterLoopMemoryObj_t* p_obj )
{
    
#if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)

    int ret = 0;
    
    if( p_obj == NULL ){      MDP_ERROR("p_obj = NULL@ un-register\n");     return -1;    }

    
    MDP_INFO_PERSIST("mHalMdp_UnRegisterLoopMemory():MVA Obj Delete:client id:%d 0x%08X(VA) 0x%08X(MVA) 0x%08X(M4U flag) 0x%08X(alloc MVA flag)\n",
        (int)client_id,
        (unsigned int)p_obj->calc_addr[0].y ,(unsigned int)p_obj->adapt_addr[0].y ,
        (unsigned int)p_obj->adapt_m4u_flag_bit,(unsigned int)p_obj->adapt_m4u_flag_bit );

    
    /*UnAdapt MdpYuvAddress*/
    ret = MdpDrv_UnAdaptMdpYuvAddrArray(    p_obj->mdp_id,
                                            p_obj->calc_addr, 1,            //Original user input address  
                                            p_obj->adapt_addr,              //Adapted address
                                            p_obj->adapt_m4u_flag_bit,      //Corresponding bit will set to 1 if address had been adapted with m4u mva
                                            p_obj->alloc_mva_flag_bit );    //Corresponding bit will set to 1 if the mva is new allocated
    if( ret < 0 ){
        MDP_ERROR("UnAdapt Memory error.\n");
        mHalMdp_PrintOut_RegisterLoopMemoryObj_t( p_obj );        
        
        return -1;
    }
    
    MdpDrv_MvaLoopMemTrack( client_id, MLMT_ACTION_FREE, 
        p_obj->adapt_addr[0].y_buffer_size + p_obj->adapt_addr[0].u_buffer_size + p_obj->adapt_addr[0].v_buffer_size );
    
#endif

    return 0;
    
}



int Mdp_BitBltEx( mHalBltParamEx_t* p_param  )
{
    {
        
        mHalBltParamEx_t                                mhal_param;
        struct MdpPathDisplayFromMemoryHdmiParameter    param;
        MdpPathDisplayFromMemoryHdmi                    mdp_path;


        memcpy( &mhal_param, p_param, sizeof( mHalBltParamEx_t ) );
        memset( &param, 0 , sizeof(param) );

        
        /*-----------------------------------------------------------------------------
            For backward compatible:
            Change back width and hiegh when rotate.
          -----------------------------------------------------------------------------*/
        {
            #define _SWAP_VAR( _type_, _a_ , _b_ ) {    _type_ temp; temp = _a_; _a_= _b_; _b_= temp; }

            if( mhal_param.hdmi_rotate_angle & 0x01 ) /*90/270 rotate*/
            {
                _SWAP_VAR( unsigned long, mhal_param.hdmi_img_size_w , mhal_param.hdmi_img_size_h );
                _SWAP_VAR( unsigned long, mhal_param.hdmi_img_roi_x  , mhal_param.hdmi_img_roi_y  );
                _SWAP_VAR( unsigned long, mhal_param.hdmi_img_roi_w  , mhal_param.hdmi_img_roi_h  );
            }

            if( mhal_param.disp_rotate_angle & 0x01 ) /*90/270 rotate*/
            {
                _SWAP_VAR( unsigned long, mhal_param.disp_img_size_w , mhal_param.disp_img_size_h );
                _SWAP_VAR( unsigned long, mhal_param.disp_img_roi_x  , mhal_param.disp_img_roi_y  );
                _SWAP_VAR( unsigned long, mhal_param.disp_img_roi_w  , mhal_param.disp_img_roi_h  );
            }
        }
        
        /*-----------------------------------------------------------------------------*/

        
        /*-----------------------------------------------------------------------------
            Parameter
          -----------------------------------------------------------------------------*/
        
        /*Source*/
        param.src_color_format   = MhalColorFormatToMdp( mhal_param.src_color_format );
        param.src_img_size = MdpSize( mhal_param.src_img_size_w ,mhal_param.src_img_size_h );
        param.src_img_roi  = MdpRect( mhal_param.src_img_roi_x, mhal_param.src_img_roi_y, mhal_param.src_img_roi_w ,mhal_param.src_img_roi_h );
        param.src_yuv_img_addr.y = mhal_param.src_yuv_img_addr;
        
        
        /*For HDMI : RGB*/
        param.en_hdmi_port = mhal_param.en_hdmi_port;           //enable hdmi port
            param.hdmi_color_format = MhalColorFormatToMdp(mhal_param.hdmi_color_format);
            param.hdmi_img_size = MdpSize( mhal_param.hdmi_img_size_w ,mhal_param.hdmi_img_size_h );      //Stride       
            param.hdmi_img_roi  = MdpRect( mhal_param.hdmi_img_roi_x, mhal_param.hdmi_img_roi_y, mhal_param.hdmi_img_roi_w ,mhal_param.hdmi_img_roi_h ); //image size to be scaled
            param.hdmi_yuv_img_addr.y = mhal_param.hdmi_yuv_img_addr;
            param.hdmi_rotate_angle = mhal_param.hdmi_rotate_angle;
        
        /*For Display : 422 Pack*/
        param.en_disp_port = mhal_param.en_disp_port;           //enable disp port
            param.disp_color_format = MhalColorFormatToMdp(mhal_param.disp_color_format);
            param.disp_img_size = MdpSize( mhal_param.disp_img_size_w ,mhal_param.disp_img_size_h );       /*Currently, "dst roi"  should be equal to "dst size"*/
            param.disp_img_roi  = MdpRect( mhal_param.disp_img_roi_x, mhal_param.disp_img_roi_y, mhal_param.disp_img_roi_w ,mhal_param.disp_img_roi_h );       /*dst_img_roi is not used*/
            param.disp_yuv_img_addr.y = mhal_param.disp_yuv_img_addr;
            param.disp_rotate_angle = mhal_param.disp_rotate_angle;
        
        /*resizer coeff*/
        //param.resz_coeff;
        
        /*.............................................................................*/
        /*Show Info*/
        MDP_INFO_PATH("[Mdp_BitBltEx Parameters]-----------------------------------------------------------------------------\n");

        
        //MDP_INFO("m4u_handle: 0x%08x\n", (unsigned int)param.src_yuv_img_addr.m4u_handle );
       
        MDP_INFO("src color: %d\n", (unsigned int)param.src_color_format );
        MDP_INFO("src w: %d    h:%d\n", (unsigned int)param.src_img_size.w , (unsigned int)param.src_img_size.h);
        MDP_INFO("src roi x: %d    y:%d\n", (unsigned int)param.src_img_roi.x , (unsigned int)param.src_img_roi.y);
        MDP_INFO("src roi w: %d    h:%d\n", (unsigned int)param.src_img_roi.w , (unsigned int)param.src_img_roi.h);
        MDP_INFO("src y address: 0x%08x\n", (unsigned int)param.src_yuv_img_addr.y );

        
        MDP_INFO("en_hdmi_port: %d\n", (unsigned int)param.en_hdmi_port );
        MDP_INFO("hdmi color: %d\n", (unsigned int)param.hdmi_color_format );
        MDP_INFO("hdmi w: %d    h:%d\n", (unsigned int)param.hdmi_img_size.w , (unsigned int)param.hdmi_img_size.h);
        MDP_INFO("hdmi roi x: %d    y:%d\n", (unsigned int)param.hdmi_img_roi.x , (unsigned int)param.hdmi_img_roi.y);
        MDP_INFO("hdmi roi w: %d    h:%d\n", (unsigned int)param.hdmi_img_roi.w , (unsigned int)param.hdmi_img_roi.h);
        MDP_INFO("hdmi y address: 0x%08x\n", (unsigned int)param.hdmi_yuv_img_addr.y );
        MDP_INFO("hdmi rotate: %d\n", (unsigned int)param.hdmi_rotate_angle );
        
        MDP_INFO("en_disp_port: %d\n", (unsigned int)param.en_disp_port );
        MDP_INFO("disp color: %d\n", (unsigned int)param.disp_color_format );
        MDP_INFO("disp w: %d    h:%d\n", (unsigned int)param.disp_img_size.w , (unsigned int)param.disp_img_size.h);
        MDP_INFO("disp roi x: %d    y:%d\n", (unsigned int)param.disp_img_roi.x , (unsigned int)param.disp_img_roi.y);
        MDP_INFO("disp roi w: %d    h:%d\n", (unsigned int)param.disp_img_roi.w , (unsigned int)param.disp_img_roi.h);
        MDP_INFO("disp y address: 0x%08x\n", (unsigned int)param.disp_yuv_img_addr.y );
        MDP_INFO("disp rotate: %d\n", (unsigned int)param.disp_rotate_angle );
        
        /*.............................................................................*/
            
        
        /*-----------------------------------------------------------------------------
            Execute
          -----------------------------------------------------------------------------*/
        if( mdp_path.Config( &param ) < 0 ){
            MDP_ERROR("Mdp_BitBltEx config error\n");
            return -1;
        }
        
        if( mdp_path.Start( NULL ) < 0 )
        {
            mdp_path.End( NULL );
            MDP_ERROR("Mdp_BitBltEx start error\n");
            return -1;
        }
        
        if( mdp_path.WaitBusy( NULL ) < 0 )
        {
            mdp_path.End( NULL );
            MDP_ERROR("Mdp_BitBltEx wait error\n");
            return -1;
        }
        
        if( mdp_path.End( NULL ) < 0 )
        {
            MDP_ERROR("Mdp_BitBltEx end error\n");
            return -1;
        }

    }

    return 0;

    
    
}

typedef struct 
{
    int             n_of_slices;        //number of slices
    unsigned int    rotation;           //rotation
    unsigned int    flip;               //flip
    unsigned int    src_width;          //src image width
    unsigned int    src_height;         //src image height
    unsigned int    max_process_width;  //max slice width
    unsigned int    last_slice_width;   //last slice width
    unsigned int    dst_pitch;
    MdpColorFormat  dst_color;
    
    
} _bbs_INFO_t;

static int _bbs_get_src_roi( int index, _bbs_INFO_t *p_info, MdpRect *p_ROI )
{
    int M       = p_info->max_process_width;
    int W       = p_info->src_width;        //Image width (not include stride)
    int H       = p_info->src_height;       //Image Heigh
    int N       = p_info->n_of_slices;      //Number of slices
    int Last_W  = p_info->last_slice_width; //Width of Last slices
    int Rotation= p_info->rotation;         //Rotation angle
    int Flip    = p_info->flip;             //Flip

    if( index >= N )  {
        MDP_ERROR("Index overflow:%d/%d\n", index, N );
        return -1;
    }

    p_ROI->x = index * M;
    p_ROI->y = 0;
    p_ROI->w = ( index == (N-1) ) ? Last_W : M;
    p_ROI->h = H;


    return 0;
    
    
}

static int _bbs_get_dst_roi( int index, _bbs_INFO_t *p_info, MdpRect *p_ROI  )
{
    
    int M       = p_info->max_process_width;
    int W       = p_info->src_width;        //Image width (not include stride)
    int H       = p_info->src_height;       //Image Heigh
    int N       = p_info->n_of_slices;      //Number of slices
    int Last_W  = p_info->last_slice_width; //Width of Last slices
    int Rotation= p_info->rotation;         //Rotation angle
    int Flip    = p_info->flip;             //Flip

    if( index >= N )  {
        MDP_ERROR("Index overflow:%d/%d\n", index, N );
        return -1;
    }


    /*.............................................................................
        0 / no-flip & 180 / flip
      .............................................................................*/
    if( ( ( Rotation == 0 ) && ( Flip == 0 ) ) ||
        ( ( Rotation == 2 ) && ( Flip == 1 ) )   )
    {
        p_ROI->x = index * M;
        p_ROI->y = 0;
        p_ROI->w = ( index == (N-1) ) ? Last_W : M;
        p_ROI->h = H;
        
    } else
    /*.............................................................................
        0 / flip & 180 / no-flip
      .............................................................................*/
    if( ( ( Rotation == 0 ) && ( Flip == 1 ) ) ||
        ( ( Rotation == 2 ) && ( Flip == 0 ) )   )
    {
        if( index != (N-1) )
        {
            p_ROI->x = W - ( index + 1 ) * M ;
            p_ROI->y = 0;
        }
        else
        {
            p_ROI->x = 0;
            p_ROI->y = 0;
        }

        p_ROI->w = ( index == (N-1) ) ? Last_W : M;
        p_ROI->h = H;
            
    } else
    /*.............................................................................
        90
      .............................................................................*/
    if( Rotation == 1 )
    {
        p_ROI->x = 0;
        p_ROI->y = index * M;

        p_ROI->w = H;
        p_ROI->h = ( index == (N-1) ) ? Last_W : M;
            
    } else
    /*.............................................................................
        270
      .............................................................................*/
    if( Rotation == 3 )
    {
        if( index != (N-1) )
        {
            p_ROI->x = 0;
            p_ROI->y = W - ( index + 1 ) * M ;
        }
        else
        {
            p_ROI->x = 0;
            p_ROI->y = 0;
        }

        p_ROI->w = H;
        p_ROI->h = ( index == (N-1) ) ? Last_W : M;
            
    } else
    /*.............................................................................
        Error
      .............................................................................*/
    {
        MDP_ERROR("Unknown combination Rotation=%d Flip=%d\n", Rotation, Flip );
        return -1;
    }

    return 0;
    
    
}


static int _bbs_get_dst_offset( MdpRect *p_ROI, _bbs_INFO_t *p_info, unsigned long *p_offset )
{
    MdpColorFormat      color;
    MdpDrvColorInfo_t   ci;
    
    int M       = p_info->max_process_width;
    int W       = p_info->src_width;        //Image width (not include stride)
    int H       = p_info->src_height;       //Image Heigh
    int N       = p_info->n_of_slices;      //Number of slices
    int Last_W  = p_info->last_slice_width; //Width of Last slices
    int Rotation= p_info->rotation;         //Rotation angle
    int Flip    = p_info->flip;             //Flip
    unsigned int pitch = p_info->dst_pitch;



    color = p_info->dst_color;

    MdpDrvColorFormatInfoGet( color, &ci );

    switch( color )
    {
    /*supported format*/
    case RGB888:
    case BGR888:
    case RGB565:
    case BGR565:
    case ABGR8888:
    case ARGB8888:
    case BGRA8888:
    case RGBA8888:
    case UYVY_Pack:
    case YUYV_Pack:
    case Y411_Pack:
    case Y800:
        break;
        
    default:
        MDP_ERROR("Unsupport Mdp_BitbltSlice MdpColorFormat 0x%x" , (unsigned int)color );
        return -1;
    }


    *p_offset = ( p_ROI->x +  ( p_ROI->y * pitch ) ) * ci.byte_per_pixel;

    return 0;

    
    

}



/*Big size image rotation*/
/*Due to SYSRAM not enough, split image into smaler size*/
/*
    Limitation:
        1.No scale
        2.No Crop
        3.No YUV Planar format
 */
int Mdp_BitbltSlice(void *a_pInBuffer)
{
    //#define MDP_ROT_MAX_WIDTH   (1280)
    //#define MDP_ROT_MAX_WIDTH   (310)

    int MDP_ROT_MAX_WIDTH = 1280;
    int i = 0;
    int M;  //Max process width of MDP rotation
    int W;  //Image width (not include stride)
    int H;  //Image Heigh
    int N;  //Number of slices
    int Last_W; //Width of Last slices
    int Rotation;   //Rotation angle
    int Flip;       //Flip
    
    MdpRect     src_roi;
    MdpRect     dst_roi;
    _bbs_INFO_t bbs_info;

    

    
    mHalBltParam_t  *bltParam;
    mHalBltParam_t  mdpbltparam;
    unsigned long   orig_dst_start_addr;

    bltParam = (mHalBltParam_t*)a_pInBuffer;
    orig_dst_start_addr = bltParam->dstAddr;

    
    /*-----------------------------------------------------------------------------
        Sanity Test
      -----------------------------------------------------------------------------*/

    if( ( bltParam->srcX != 0 ) || ( bltParam->srcY != 0 ) )
    {
        MDP_ERROR("Mdp_BitbltSlice dont support Cropping. x=%lu y=%lu \n",(unsigned long)bltParam->srcX, (unsigned long)bltParam->srcY );
        return -1;
    }

    
    /*-----------------------------------------------------------------------------
        Decide MAX slice width
      -----------------------------------------------------------------------------*/
    MDP_ROT_MAX_WIDTH = 1280;
    if( ( bltParam->srcW % MDP_ROT_MAX_WIDTH ) < 3 )
        MDP_ROT_MAX_WIDTH = 1260;


    /*-----------------------------------------------------------------------------
        Variables
      -----------------------------------------------------------------------------*/
    M = MDP_ROT_MAX_WIDTH;  //Max process width of MDP rotation
    W = bltParam->srcW;
    H = bltParam->srcH;
    N = ( W / M ) + ( ( (W % M) > 0) ? 1 : 0 );
    Last_W = W - ( N - 1 ) * M;
    Rotation = (bltParam->orientation & 0x3);
    Flip = ( bltParam->orientation >> 2 ) & 0x1;


    
    /*-----------------------------------------------------------------------------
        Start Slice Process
      -----------------------------------------------------------------------------*/

    bbs_info.n_of_slices        = N;        //number of slices
    bbs_info.rotation           = Rotation; //rotation
    bbs_info.flip               = Flip;     //flip
    bbs_info.src_width          = W;        //src image width
    bbs_info.src_height         = H;         //src image height
    bbs_info.max_process_width  = M;        //max slice width
    bbs_info.last_slice_width   = Last_W;   //last slice width
    bbs_info.dst_pitch          = bltParam->pitch;
    bbs_info.dst_color          = MhalColorFormatToMdp( (MHAL_BITBLT_FORMAT_ENUM)bltParam->dstFormat );

    memcpy( &mdpbltparam, a_pInBuffer, sizeof(mHalBltParam_t) );

    for( i = 0; i < N; i++ )
    {
        unsigned long   offset = 0;

        
        _bbs_get_src_roi( i, &bbs_info, &src_roi );
        _bbs_get_dst_roi( i, &bbs_info, &dst_roi  );
        _bbs_get_dst_offset( &dst_roi, &bbs_info, &offset );
        
        mdpbltparam.srcX = src_roi.x;                 
        mdpbltparam.srcY = src_roi.y;                 
        mdpbltparam.srcW = src_roi.w;                 
        //mdpbltparam.srcWStride;           
        //mdpbltparam.srcH;                 
        //mdpbltparam.srcHStride;           
        //mdpbltparam.srcAddr;              
        //mdpbltparam.srcFormat;            
                                          
        mdpbltparam.dstW    = dst_roi.w;                 
        mdpbltparam.dstH    = dst_roi.h;                 
        mdpbltparam.dstAddr = orig_dst_start_addr + offset;              
        //mdpbltparam.dstFormat;            
        //mdpbltparam.pitch;                
                                          
        //mdpbltparam.orientation;          
        //mdpbltparam.doImageProcess;       
        //mdpbltparam.favor_flags;    
                                          
                                          
        //mdpbltparam.u4SrcOffsetXFloat;
        //mdpbltparam.u4SrcOffsetYFloat;
                                          
        //mdpbltparam.resz_up_scale_coeff;  
        //mdpbltparam.resz_dn_scale_coeff;  
        //mdpbltparam.resz_ee_h_str;        
        //mdpbltparam.resz_ee_v_str;        

        
        MDP_INFO_PERSIST("[BBS]index=%d/%d, MaxWidth = %d , src_roi=(%d,%d,%d,%d) , dst_roi=(%d,%d,%d,%d) , Offset=0x%08X\n",
                    i, (int)N, (int)M,
                    (int)src_roi.x, (int)src_roi.y, (int)src_roi.w, (int)src_roi.h,
                    (int)dst_roi.x, (int)dst_roi.y, (int)dst_roi.w, (int)dst_roi.h,
                    (unsigned int)offset );
                    
                    
                    
        
        

        //if( i == 2 )
            
        Mt6575_mHalBitblt( &mdpbltparam );

        
    }
    

    return 0;

    

    

}











