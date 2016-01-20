/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
//#define LOG_TAG "CMdpHal"
#define LOG_TAG "MDP"



//
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
//
#include "MediaTypes.h"
#include "mdp_hal_imp.h"

//mdp headers
#include "mdp_datatypes.h"
#include "mhal_interface.h"


//#include "mt6573_sysram.h"
//#include "jpeg_enc_hal.h"
//

/*******************************************************************************
*
********************************************************************************/

/*******************************************************************************
*
********************************************************************************/

/*******************************************************************************
* Test
********************************************************************************/

static int _SaveImage( char* filename , unsigned long addr, unsigned long size )
{
    FILE *fp;

    fp = fopen( filename, "wb");

    if(  fp == NULL )
    {
        MDP_ERROR("Open file failed.:%s\n", strerror(errno));
        return -1;
    }

    if( fwrite( (void*)addr , 1 , size , fp) < size )
    {
        MDP_ERROR("write file failed.:%s\n", strerror(errno));
    }
     
    fclose(fp);
    MDP_INFO_PERSIST("File saved : %s\n", filename );

    return 0;
}


static int TestCbFunc_EIS_INFO_GET( unsigned long *p_eis_x, unsigned long *p_eis_y )
{
    static int count = 0;
    int module;

    module = count++%4;

    #if 1

    switch( module )
    {
    case 0:
        *p_eis_x = (0x0 << 8 ) | 0x0;
        *p_eis_y = (0x0 << 8 ) | 0x0;
        break;
    case 1:
        *p_eis_x = (0x1 << 8 ) | 0x10;
        *p_eis_y = (0x1 << 8 ) | 0x10;
        break;
    case 2:
        *p_eis_x = (0x2 << 8 ) | 0x20;
        *p_eis_y = (0x2 << 8 ) | 0x20;
        break;
    case 3:
        *p_eis_x = (0x3 << 8 ) | 0x30;
        *p_eis_y = (0x3 << 8 ) | 0x30;
        break;
    }
    #else
        *p_eis_x = 0 ;
        *p_eis_y = 0 ;
    #endif

    return 0;
    
}



/*******************************************************************************
* 
********************************************************************************/
    /*
    cls:why??
    MdpHal is a base class, but its implementation invoke its descendant. why??
    */
MdpHal* 
MdpHal::createInstance()
{
    MDP_SHOWFUNCTION();
    
    return MdpHalImp::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
    /*
    cls:why??
    If you create a MdpHalImp instance here, then who is the MdpHalImp instance create it????
    Should this change to just ruturn "this"?
    */

MdpHal*
MdpHalImp::
getInstance()
{
    MDP_INFO_PERSIST("[MdpHal] getInstance \n");
    static MdpHalImp singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void   
MdpHalImp::
destroyInstance() 
{
    MDP_INFO_PERSIST("[MdpHal] destroyInstance \n");
}

/*******************************************************************************
*
********************************************************************************/
MdpHalImp::MdpHalImp()
{
    
    MDP_INFO_PERSIST("[MdpHal] MdpHalImp (Open device driver)\n");

    //
    if( MdpDrvInit() == -1 )     //Open mdp device driver node
    {
        MDP_ERROR("Open mdp device driver failed\n!");
    } else
    {
        MDP_INFO_PERSIST("Open mdp device driver Success!\n");
    }
        
    memset(&mMDPParam, 0, sizeof(halIDPParam_t));
    
}

/******************************************************************************
*
********************************************************************************/
MdpHalImp::~MdpHalImp()
{
    MDP_INFO_PERSIST("[MdpHal] MdpHalImp (Close device driver)\n");

    //
    if( MdpDrvRelease() == -1 )  //Close mdp device driver node
    {
        MDP_ERROR("Close mdp device driver failed\n!");
    } else
    {
        MDP_INFO_PERSIST("Close mdp device driver Success\n!");
    }
    //

}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpHalImp::dumpReg()
{
    MINT32 ret = 0;


    ret = m_mdp_pipe_camera_preview.DumpRegister();

    
    
    return ret;   
}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpHalImp::init()
{
    MINT32 ret = 0;

    MDP_INFO_PERSIST("[MdpHal] init\n");

    memset( &mMDPParam, 0, sizeof(halIDPParam_t) );
    //
    memset( &m_mdp_pipe_camera_preview_param, 0, sizeof(MdpPipeCameraPreviewParameter) );

    return ret;   
}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpHalImp::uninit()
{
    MINT32 ret = 0;

    MDP_INFO_PERSIST("[MdpHal] uninit\n");
    
    return ret;   
}


/*******************************************************************************
*
********************************************************************************/
MINT32 MdpHalImp::start()
{
    #if defined(MDP_FLAG_PROFILING)
    MdpDrv_Watch _MdpDrvWatch;
    static unsigned long tt0,tt1,tt2,tt3;//total time;
    static unsigned long fc0,fc1,fc2,fc3;//frame count;
    static unsigned long avt0,avt1,avt2,avt3;//avg_time_elapse;
    char   title_str[256];
    #endif

    int retval = 0;
    
    MDP_INFO_PERSIST("[MdpHal] start (mode=%d)\n", mMDPParam.mode );

    /*
    1.Allocate Sysram (if element is included in resource)
    2.Enable MDP element (if element is included in resource)
    */

    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_IT );

    switch( mMDPParam.mode )
    {
    case MDP_MODE_PRV_YUV:
        /*Lock BRZ to prevent jpeg decode when preview*/
        #if 0 //Leave BRZ lock to upper layer
        if( m_mdp_path_dummy_brz.Config((struct MdpPathDummyBrzParameter *)NULL) < 0 )
        {
            MDP_ERROR("m_mdp_path_dummy_brz condif error!\n");
            retval = -1;
            break;
        }
        
        if( m_mdp_path_dummy_brz.Start( NULL ) < 0 )
        {
            m_mdp_path_dummy_brz.End( NULL );//Release resource
            MDP_ERROR("m_mdp_path_dummy_brz start error!\n");
            retval = -1;
            break;
        }
        #endif
        
        /*Assign EIS call back function*/
        m_mdp_pipe_camera_preview.Config_CallBackFunction_EisInfoGet( mMDPParam.CB_EIS_INFO_GET_FUNC );
        
        if( m_mdp_pipe_camera_preview.Config( &m_mdp_pipe_camera_preview_param ) < 0 )
        {
            //m_mdp_path_dummy_brz.End( NULL );//Release resource//Leave BRZ lock to upper layer
            MDP_ERROR("m_mdp_pipe_camera_preview config error!\n");
            retval = -1;
            break;
        }

        if( m_mdp_pipe_camera_preview.Start( NULL) < 0 )
        {
            //m_mdp_path_dummy_brz.End( NULL );//Release resource//Leave BRZ lock to upper layer
            m_mdp_pipe_camera_preview.End( NULL );//Release resource
            MDP_ERROR("m_mdp_pipe_camera_preview start error!\n");
            retval = -1;
            break;
        }
        break;
    case MDP_MODE_CAP_JPG:
        /*.............................................................................*/
        /*Show Information*/        
        MDP_INFO_PERSIST("[MdpHalImp Capture Parameters]-----------------------------------------------------------------------------\n");

        MDP_INFO_PERSIST("camera out mode: %d\n",mMDPParam.Capture.camera_out_mode);
        MDP_INFO_PERSIST("camera count: %d\n",mMDPParam.Capture.camera_count);
        MDP_INFO_PERSIST("pseudo src:%d\n",mMDPParam.Capture.pseudo_src_enable);
        if( mMDPParam.Capture.pseudo_src_enable )
        {
            MDP_INFO_PERSIST("pseudo srcy color = %d\n",(int)mMDPParam.Capture.pseudo_src_color_format );
            MDP_INFO_PERSIST("pseudo srcy address = 0x%08x\n",(unsigned int)mMDPParam.Capture.pseudo_src_yuv_img_addr.y );
        }
        MDP_INFO_PERSIST("src w: %d    h: %d\n", (unsigned int)mMDPParam.Capture.src_img_size.w, (unsigned int)mMDPParam.Capture.src_img_size.h);
        MDP_INFO_PERSIST("roi x: %d    y: %d\n", (unsigned int)mMDPParam.Capture.src_img_roi.x, (unsigned int)mMDPParam.Capture.src_img_roi.y);
        MDP_INFO_PERSIST("roi w: %d    h: %d\n", (unsigned int)mMDPParam.Capture.src_img_roi.w, (unsigned int)mMDPParam.Capture.src_img_roi.h);

        MDP_INFO_PERSIST("jpg en=%d\n",mMDPParam.Capture.b_jpg_path_disen==0?1:0);
        MDP_INFO_PERSIST("jpg img w: %d    h: %d\n", (unsigned int)mMDPParam.Capture.jpg_img_size.w, (unsigned int)mMDPParam.Capture.jpg_img_size.h);
        MDP_INFO_PERSIST("jpg yuv color: %d\n", (unsigned int)mMDPParam.Capture.jpg_yuv_color_format );
        MDP_INFO_PERSIST("jpg img buffer addr: 0x%08X\n", (unsigned int)mMDPParam.Capture.jpg_buffer_addr );
        MDP_INFO_PERSIST("jpg img buffer size: %d\n", (unsigned int)mMDPParam.Capture.jpg_buffer_size );
        MDP_INFO_PERSIST("jpg img quality: %d\n", (unsigned int)mMDPParam.Capture.jpg_quality );
        MDP_INFO_PERSIST("jpg add soi: %d\n", (unsigned int)mMDPParam.Capture.jpg_b_add_soi );

        MDP_INFO_PERSIST("qv en=%d\n",mMDPParam.Capture.b_qv_path_en);
        MDP_INFO_PERSIST("qv path=%d\n",mMDPParam.Capture.qv_path_sel);
        MDP_INFO_PERSIST("qv y address = 0x%08x\n", (unsigned int)mMDPParam.Capture.qv_yuv_img_addr.y );
        MDP_INFO_PERSIST("qv color = %d\n", (unsigned int)mMDPParam.Capture.qv_color_format );
        MDP_INFO_PERSIST("qv roi x: %d   y: %d    w: %d    h: %d\n", (unsigned int)mMDPParam.Capture.qv_img_roi.x, (unsigned int)mMDPParam.Capture.qv_img_roi.y, (unsigned int)mMDPParam.Capture.qv_img_roi.w, (unsigned int)mMDPParam.Capture.qv_img_roi.h );
        MDP_INFO_PERSIST("qv w: %d    h: %d\n", (unsigned int)mMDPParam.Capture.qv_img_size.w, (unsigned int)mMDPParam.Capture.qv_img_size.h);
        MDP_INFO_PERSIST("qv flip: %d    rotate: %d\n", (unsigned int)mMDPParam.Capture.qv_flip, (unsigned int)mMDPParam.Capture.qv_rotate );
        
        MDP_INFO_PERSIST("ff en=%d\n",mMDPParam.Capture.b_ff_path_en);
        MDP_INFO_PERSIST("ff y address = 0x%08x\n", (unsigned int)mMDPParam.Capture.ff_yuv_img_addr.y );
        MDP_INFO_PERSIST("ff color = %d\n", (unsigned int)mMDPParam.Capture.ff_color_format );
        MDP_INFO_PERSIST("ff roi x: %d   y: %d    w: %d    h: %d\n", (unsigned int)mMDPParam.Capture.ff_img_roi.x, (unsigned int)mMDPParam.Capture.ff_img_roi.y, (unsigned int)mMDPParam.Capture.ff_img_roi.w, (unsigned int)mMDPParam.Capture.ff_img_roi.h );
        MDP_INFO_PERSIST("ff w: %d    h: %d\n", (unsigned int)mMDPParam.Capture.ff_img_size.w, (unsigned int)mMDPParam.Capture.ff_img_size.h);
        MDP_INFO_PERSIST("ff flip: %d    rotate: %d\n", (unsigned int)mMDPParam.Capture.ff_flip, (unsigned int)mMDPParam.Capture.ff_rotate );
        
        MDP_INFO_PERSIST("capture resz coeff:\n");
        MDP_INFO_PERSIST("crz_up_scale_coeff=%d\n", mMDPParam.Capture.resz_coeff.crz_up_scale_coeff );
        MDP_INFO_PERSIST("crz_dn_scale_coeff=%d\n", mMDPParam.Capture.resz_coeff.crz_dn_scale_coeff );
        MDP_INFO_PERSIST("prz0_up_scale_coeff=%d\n", mMDPParam.Capture.resz_coeff.prz0_up_scale_coeff );
        MDP_INFO_PERSIST("prz0_dn_scale_coeff=%d\n", mMDPParam.Capture.resz_coeff.prz0_dn_scale_coeff );
        MDP_INFO_PERSIST("prz0_ee_h_str=%d\n", mMDPParam.Capture.resz_coeff.prz0_ee_h_str );
        MDP_INFO_PERSIST("prz0_ee_v_str=%d\n", mMDPParam.Capture.resz_coeff.prz0_ee_v_str );
        MDP_INFO_PERSIST("prz1_up_scale_coeff=%d\n", mMDPParam.Capture.resz_coeff.prz1_up_scale_coeff );
        MDP_INFO_PERSIST("prz1_dn_scale_coeff=%d\n", mMDPParam.Capture.resz_coeff.prz1_dn_scale_coeff );
        MDP_INFO_PERSIST("prz1_ee_h_str=%d\n", mMDPParam.Capture.resz_coeff.prz1_ee_h_str );
        MDP_INFO_PERSIST("prz1_ee_v_str=%d\n", mMDPParam.Capture.resz_coeff.prz1_ee_v_str );
        MDP_INFO_PERSIST("\n");
        /*.............................................................................*/

        if( m_mdp_pipe_camera_capture.Config( &(mMDPParam.Capture) )  < 0 )
        {
            MDP_ERROR("m_mdp_pipe_camera_capture config error!\n");
            retval = -1;
            break;
        }
        
        if( m_mdp_pipe_camera_capture.Start( NULL) < 0 )
        {
            m_mdp_pipe_camera_capture.End( NULL );//Release resource
            MDP_ERROR("m_mdp_pipe_camera_capture start error!\n");
            retval = -1;
            break;
        }
        break;
    default:
        MDP_ERROR("No available operation mode!\n");
        retval = -1;
        break;
    }

    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_IT , "MdpHalImp::start()", &tt0, &fc0, &avt0, 30 );


    return retval;

}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpHalImp::stop()
{
    MINT32 retval = 0;


    MDP_INFO_PERSIST("[MdpHal] stop (mode=%d)\n", mMDPParam.mode );

    
    /*
    1.Free Sysram (if element is included in resource)
    2.Disable MDP element (if element is included in resource)
    3.unLock resource
    */

    
    switch( mMDPParam.mode )
    {
    case MDP_MODE_PRV_YUV:
                 //m_mdp_path_dummy_brz.End( NULL );//Leave BRZ lock to upper layer
                 
        retval = m_mdp_pipe_camera_preview.End( NULL );
        break;
    case MDP_MODE_CAP_JPG:
        retval = m_mdp_pipe_camera_capture.End( NULL );;
        break;
    default:
        retval = -1;
        break;
    }

    return retval;


}

/******************************************************************************
*
*******************************************************************************/
MINT32 MdpHalImp::calCropRect(
    rect_t rSrc,
    rect_t rDst,
    rect_t *prCrop,
    MUINT32 zoomRatio
)
{
    rect_t rCrop;
    MUINT32 u4ZoomRatio = zoomRatio; 
    //
    if (u4ZoomRatio > 800) {
        MDP_INFO("[calCropRegion] Zoom Raio reach max value %d" , u4ZoomRatio);
        u4ZoomRatio = 800;
    }

    MDP_INFO("[calCropRegion] src: %d/%d, dst: %d/%d, zoom: %d \n", 
        rSrc.w, rSrc.h, rDst.w, rDst.h, zoomRatio); 

    // srcW/srcH < dstW/dstH 
    if (rSrc.w * rDst.h < rDst.w * rSrc.h) {
        rCrop.w = rSrc.w; 
        rCrop.h = rSrc.w * rDst.h / rDst.w;     
    }
    else if (rSrc.w * rDst.h > rDst.w * rSrc.h) { //srcW/srcH > dstW/dstH
        rCrop.w = rSrc.h * rDst.w / rDst.h; 
        rCrop.h = rSrc.h; 
    }
    else {
        rCrop.w = rSrc.w; 
        rCrop.h = rSrc.h; 
    }
    // 
    //
    rCrop.w =  (rCrop.w * 100 / zoomRatio);   
    rCrop.h =  (rCrop.h * 100 / zoomRatio);
    // make it 2x
    rCrop.w = (rCrop.w >> 1) << 1;
    rCrop.h = (rCrop.h >> 1) << 1;
    //
    rCrop.x = (rSrc.w - rCrop.w) / 2;
    rCrop.y = (rSrc.h - rCrop.h) / 2;
    //
    // make it 2x
    rCrop.x = (rCrop.x >> 1) << 1;
    rCrop.y = (rCrop.y >> 1) << 1;
    *prCrop = rCrop;
    MDP_INFO("[calCropRegion] Crop region: %d, %d, %d, %d \n", rCrop.x, rCrop.y, rCrop.w, rCrop.h); 
    
    return 0;
}

/*******************************************************************************
*CAM->CRZ->IPP->OVL->ROTDMA0
*                                   ->VRZ->ROTDMA2
*Always occupied
********************************************************************************/
MINT32 MdpHalImp::setPrv(halIDPParam_t *phalIDPParam)
{

    MDP_SHOWFUNCTION();

    MdpPipeCameraPreviewParameter* p_pipe = &m_mdp_pipe_camera_preview_param;
    MdpSize max_output_size;
    MdpSize work_img_size;

    
    /*------------------------------------------------*/
    /* Parameters Sanity Check                        */
    /*------------------------------------------------*/
    if( phalIDPParam->en_n3d_preview_path != 1 )
    {
    if( (phalIDPParam->en_zero_shutter_path == 0) && ( phalIDPParam->working_buffer_count == 0 ) )
    {
        MDP_ERROR("working_buffer_count = 0\n");
        return -1;
    }

    if( phalIDPParam->en_dst_port0 && (phalIDPParam->dst_port0.buffer_count == 0) )
    {
        MDP_ERROR("dst_port0.buffer_count = 0\n");
        return -1;
    }

    if( phalIDPParam->en_dst_port1 && (phalIDPParam->dst_port1.buffer_count == 0) )
    {
        MDP_ERROR("dst_port1.buffer_count = 0\n");
        return -1;
    }

    if( phalIDPParam->en_dst_port2 && (phalIDPParam->dst_port2.buffer_count == 0) )
    {
        MDP_ERROR("dst_port2.buffer_count = 0\n");
        return -1;
    }
    }

    if( (   ( ( phalIDPParam->en_one_pass_preview_path > 0 ) ? 1 : 0 ) + 
            ( ( phalIDPParam->en_n3d_preview_path > 0 ) ? 1 : 0 ) +
            ( ( phalIDPParam->en_zero_shutter_path > 0 ) ? 1 : 0 )  )  > 1  )
    {
        MDP_ERROR("More than one exclusive option has been turned on.(%d,%d,%d)\n" , 
            phalIDPParam->en_one_pass_preview_path, 
            phalIDPParam->en_n3d_preview_path,
            phalIDPParam->en_zero_shutter_path );
        return -1;
    }

    
    /*------------------------------------------------*/
    /* Preview Option Select                          */
    /*------------------------------------------------*/
    /*Switch one pass preview path*/
    p_pipe->en_one_pass_preview_path = phalIDPParam->en_one_pass_preview_path;
    
    /*Switch HW trigger/SW trigger*/
    p_pipe->en_sw_trigger = phalIDPParam->en_sw_trigger;

    /*Switch zsd or normal path*/
    p_pipe->en_zero_shutter_path = phalIDPParam->en_zero_shutter_path;

    /*Switch N3D path*/
    p_pipe->en_n3d_preview_path = phalIDPParam->en_n3d_preview_path;
    p_pipe->n3d_working_buff_layout = phalIDPParam->n3d_working_buff_layout;


    
    /*------------------------------------------------*/
    /* Decide Working Buffer Size                     */
    /*------------------------------------------------*/

    if( p_pipe->en_one_pass_preview_path == 1 )
    /*.............................................................................
        I.[One Pass Preview Path (for 1080p)]
      .............................................................................*/
      /*use user define working buffer size directly*/
    {
        
        if( phalIDPParam->working_rotate & 0x1 )/*90 & 270 rotate*/
        {
            /*mhal is after rotate, change to MDP drv before rotate*/
            work_img_size.w = phalIDPParam->working_img_size.h;
            work_img_size.h = phalIDPParam->working_img_size.w;
            
        }
        else
        {
            work_img_size = phalIDPParam->working_img_size;
        }
        
    }


    else if( p_pipe->en_zero_shutter_path == 1 )
    /*.............................................................................
        II.[ZSD]
      .............................................................................*/
        
    {
        /*If ZSD, no working buffer is needed*/
        work_img_size.w = 0;
        work_img_size.h = 0;
    }
    

    else if( p_pipe->en_n3d_preview_path == 1 )
    /*.............................................................................
        III.[N3D] 
      .............................................................................*/
      /*use user defined working buffer size*/
    {
        //TG1 working buffer
        if( phalIDPParam->work_tg1.working_rotate & 0x1 )/*90 & 270 rotate*/
        {
            /*mhal is after rotate, change to MDP drv before rotate*/
            work_img_size.w = phalIDPParam->work_tg1.working_img_size.h;
            work_img_size.h = phalIDPParam->work_tg1.working_img_size.w;
            
        }
        else
        {
            work_img_size = phalIDPParam->work_tg1.working_img_size;
        }

        //TG2 working buffer:IGNORE!! only use TG1's setting!!!

    }


    else /*if( p_pipe->en_zero_shutter_path == 0 || p_pipe->en_zero_shutter_path == 2)*/
    /*.............................................................................
        IV.[Normal Preview] 
      .............................................................................*/
    /*Normal preview & ZSD ver.2 path : calculate optimised working buffer size*/
    {
        /*HW Trigger, decide the optimized working buffer size*/
        if( p_pipe->en_sw_trigger == 0 )
        {
            #if 1
            /*Find out the optimised sized of working buffer image size:
                If downscale:
                        working image size is the max size of three output image size.
                if upscale:
                        working image size is just the input size.
            */
            max_output_size.w = MDP_MAX3( phalIDPParam->dst_port0.size_w, phalIDPParam->dst_port1.size_w, phalIDPParam->dst_port2.size_w);
            max_output_size.h = MDP_MAX3( phalIDPParam->dst_port0.size_h, phalIDPParam->dst_port1.size_h, phalIDPParam->dst_port2.size_h);

                    //Down scale 
            if( (phalIDPParam->src_size_w > max_output_size.w) && (phalIDPParam->src_size_h > max_output_size.h) )
            {
                work_img_size.w = max_output_size.w;
                work_img_size.h = max_output_size.h;

            } else  //Up Scale
            {
                work_img_size.w = phalIDPParam->src_size_w;
                work_img_size.h = phalIDPParam->src_size_h;
            }
            #else
            work_img_size.w = phalIDPParam->src_size_w;
            work_img_size.h = phalIDPParam->src_size_h;
            #endif
        } 
        /*SW Trigger, use user define working buffer size directly*/
        else 
        {
            if( phalIDPParam->working_rotate & 0x1 )/*90 & 270 rotate*/
            {
                /*mhal is after rotate, change to MDP drv before rotate*/
                work_img_size.w = phalIDPParam->working_img_size.h;
                work_img_size.h = phalIDPParam->working_img_size.w;
                
            }
            else
            {
                work_img_size = phalIDPParam->working_img_size;
            }
        }
    }



    /*------------------------------------------------*/
    /*Parameters Mapping                              */
    /*------------------------------------------------*/
    /*------------------------------------------------*/
    /*Thread 1 : MdpPathCameraPreviewToMemoryParameter*/
    /*------------------------------------------------*/
    p_pipe->pseudo_src_enable =   phalIDPParam->test_pseudo_src_enable; //Use RDMA0 to take place of camera sensor as pseudo source          

    if( p_pipe->pseudo_src_enable )
    {
        /*pseduo source use the same buffer count as dst buffer*/
        p_pipe->pseudo_src_color_format = phalIDPParam->test_pseudo_src_color_format;
        p_pipe->pseudo_src_yuv_img_addr.y = phalIDPParam->test_pseudo_src_yuv_img_addr;
    }

    p_pipe->debug_preview_single_frame_enable = phalIDPParam->debug_preview_single_frame_enable;


    p_pipe->src_img_size.w = phalIDPParam->src_size_w;
    p_pipe->src_img_size.h = phalIDPParam->src_size_h;

    
    p_pipe->src_tg2_img_size.w = phalIDPParam->src_tg2_size_w;
    p_pipe->src_tg2_img_size.h = phalIDPParam->src_tg2_size_h;


    
    #if defined (MDP_FLAG_USE_RDMA_TO_CROP)
    p_pipe->src_img_roi = phalIDPParam->src_roi;
    p_pipe->src_tg2_img_roi = phalIDPParam->src_tg2_roi;
    #else
    p_pipe->src_img_roi         = MdpRect( 0, 0, p_pipe->src_img_size.w, p_pipe->src_img_size.h);
    p_pipe->src_img_tg2_roi.x   = MdpRect( 0, 0, p_pipe->src_img_tg2_size.w, p_pipe->src_img_tg2_size.h);
    #endif


    /*Working Buffer ( = dst image ) Setting */
    p_pipe->dst_buffer_count   = phalIDPParam->working_buffer_count;
    p_pipe->dst_yuv_img_addr.y = phalIDPParam->working_buffer_addr;
    p_pipe->dst_img_size       = work_img_size;


    
    /*HW Trigger Path/ZSD (although no use these*/
    if( 
        (
         ( p_pipe->en_one_pass_preview_path == 0 ) &&
         ( p_pipe->en_zero_shutter_path != 1 ) &&
         ( p_pipe->en_n3d_preview_path == 0 ) &&
         ( p_pipe->en_sw_trigger == 0 )
        )
        || 
        (
           p_pipe->en_zero_shutter_path == 1 
        )
      )
    {
        p_pipe->dst_color_format = RGB565; //RGB888;

        /*no scale*/
        p_pipe->dst_img_roi.x = 0;
        p_pipe->dst_img_roi.y = 0; 
        p_pipe->dst_img_roi.w = p_pipe->dst_img_size.w;
        p_pipe->dst_img_roi.h = p_pipe->dst_img_size.h;

        p_pipe->dst_rotate_angle = 0; //HW trigger, no rotate in phase 1
    }
    else 
    {
        p_pipe->dst_color_format = phalIDPParam->working_color_format;

        if( phalIDPParam->working_rotate & 0x1 )/*90 & 270 rotate*/
        {
            /*mhal is after rotate, change to MDP drv before rotate*/
            p_pipe->dst_img_roi.x = phalIDPParam->working_img_roi.y;
            p_pipe->dst_img_roi.y = phalIDPParam->working_img_roi.x;
            p_pipe->dst_img_roi.w = phalIDPParam->working_img_roi.h;
            p_pipe->dst_img_roi.h = phalIDPParam->working_img_roi.w;
        }
        else
        {
            p_pipe->dst_img_roi = phalIDPParam->working_img_roi;
        }

        p_pipe->dst_rotate_angle = ( (phalIDPParam->working_flip & 0x1)<<2 ) |phalIDPParam->working_rotate;;
        if ( p_pipe->en_zero_shutter_path == 2 )
        {
             p_pipe->dst_yuv_img_addr_last_preview_.y = phalIDPParam->working_buffer_addr_last_preview_;
             p_pipe->dst_buffer_count_zsd = phalIDPParam->working_buffer_count_zsd;
        }
    } 


    /*N3D Preview Path*/
    if( p_pipe->en_n3d_preview_path == 1 )
    {
        p_pipe->working_tg1_buffer_count = phalIDPParam->work_tg1.working_buffer_count;
        p_pipe->working_tg1_buffer_addr.y= phalIDPParam->work_tg1.working_buffer_addr; 
        p_pipe->working_tg1_color_format = phalIDPParam->work_tg1.working_color_format;
        p_pipe->working_tg1_img_size     = phalIDPParam->work_tg1.working_img_size;
        p_pipe->working_tg1_img_roi      = phalIDPParam->work_tg1.working_img_roi; 
        p_pipe->working_tg1_rotate_angle = ( (phalIDPParam->work_tg1.working_flip & 0x1)<<2 ) |phalIDPParam->work_tg1.working_rotate;

        p_pipe->working_tg2_buffer_count = phalIDPParam->work_tg2.working_buffer_count;                                                
        p_pipe->working_tg2_buffer_addr.y= phalIDPParam->work_tg2.working_buffer_addr;                                                
        p_pipe->working_tg2_color_format = phalIDPParam->work_tg2.working_color_format;                                               
        p_pipe->working_tg2_img_size     = phalIDPParam->work_tg2.working_img_size;                                                   
        p_pipe->working_tg2_img_roi      = phalIDPParam->work_tg2.working_img_roi;                                                    
        p_pipe->working_tg2_rotate_angle = ( (phalIDPParam->work_tg2.working_flip & 0x1)<<2 ) |phalIDPParam->work_tg2.working_rotate; 
    }


    /*------------------------------------------------*/
    /*Thread 2 : MdpPathDisplayFromMemory*/
    /*------------------------------------------------*/
    /*For Display : NV21*/
    p_pipe->en_dst_port0 = phalIDPParam->en_dst_port0;
    
    p_pipe->dst0_buffer_count = phalIDPParam->dst_port0.buffer_count;
    p_pipe->dst0_color_format = phalIDPParam->dst_port0.color_format;

    p_pipe->dst0_img_size.w = phalIDPParam->dst_port0.size_w;       /*Currently, "dst roi"  should be equal to "dst size"*/
    p_pipe->dst0_img_size.h = phalIDPParam->dst_port0.size_h;

    p_pipe->dst0_img_roi.x = 0;        /*dst_img_roi is not used*/
    p_pipe->dst0_img_roi.y = 0;
    p_pipe->dst0_img_roi.w = p_pipe->dst0_img_size.w;
    p_pipe->dst0_img_roi.h = p_pipe->dst0_img_size.h;
    
    p_pipe->dst0_yuv_img_addr.y = phalIDPParam->dst_port0.buffer_addr;
    
    
    p_pipe->dst0_rotate_angle = ( (phalIDPParam->dst_port0.flip & 0x1)<<2 ) |phalIDPParam->dst_port0.rotate;

    /*For Encode : YV12*/
    p_pipe->en_dst_port1 = phalIDPParam->en_dst_port1;

    p_pipe->dst1_buffer_count = phalIDPParam->dst_port1.buffer_count;
    p_pipe->dst1_color_format = phalIDPParam->dst_port1.color_format;

    p_pipe->dst1_img_size.w = phalIDPParam->dst_port1.size_w;       /*Currently, "dst roi"  should be equal to "dst size"*/
    p_pipe->dst1_img_size.h = phalIDPParam->dst_port1.size_h;
    
    p_pipe->dst1_img_roi.x = 0;        /*dst_img_roi is not used*/
    p_pipe->dst1_img_roi.y = 0;
    p_pipe->dst1_img_roi.w = p_pipe->dst1_img_size.w;
    p_pipe->dst1_img_roi.h = p_pipe->dst1_img_size.h;
    
    p_pipe->dst1_yuv_img_addr.y = phalIDPParam->dst_port1.buffer_addr;
    
    p_pipe->dst1_rotate_angle = ( (phalIDPParam->dst_port1.flip & 0x1)<<2 ) |phalIDPParam->dst_port1.rotate;

    /*For FD : RGB*/
    p_pipe->en_dst_port2 = phalIDPParam->en_dst_port2;

    p_pipe->dst2_buffer_count = phalIDPParam->dst_port2.buffer_count;
    p_pipe->dst2_color_format = phalIDPParam->dst_port2.color_format;

    p_pipe->dst2_img_size.w = phalIDPParam->dst_port2.size_w;       /*Currently, "dst roi"  should be equal to "dst size"*/
    p_pipe->dst2_img_size.h = phalIDPParam->dst_port2.size_h;

    p_pipe->dst2_img_roi.x = 0;        /*dst_img_roi is not used*/
    p_pipe->dst2_img_roi.y = 0;
    p_pipe->dst2_img_roi.w = p_pipe->dst2_img_size.w;
    p_pipe->dst2_img_roi.h = p_pipe->dst2_img_size.h;
    
    p_pipe->dst2_yuv_img_addr.y = phalIDPParam->dst_port2.buffer_addr;
        
    p_pipe->dst2_rotate_angle  = ( (phalIDPParam->dst_port2.flip & 0x1)<<2 ) | phalIDPParam->dst_port2.rotate;


    /*resz coeff*/
    p_pipe->resz_coeff = phalIDPParam->prv_resz_coeff;

    /*.............................................................................*/
    /*Show Info*/
    /*.............................................................................*/
    MDP_INFO_PERSIST("[MdpHalImp Preview Parameters]-----------------------------------------------------------------------------\n");
    MDP_INFO_PERSIST("pseudo src:%d\n",p_pipe->pseudo_src_enable);
    if( p_pipe->pseudo_src_enable )
    {
        MDP_INFO_PERSIST("pseudo srcy address = 0x%08x\n",(unsigned int)p_pipe->pseudo_src_yuv_img_addr.y );
    }
    MDP_INFO_PERSIST("src y address = 0x%08x\n", (unsigned int)p_pipe->pseudo_src_yuv_img_addr.y );
    MDP_INFO_PERSIST("src w: %d    h: %d\n", (unsigned int)p_pipe->src_img_size.w, (unsigned int)p_pipe->src_img_size.h);
    #if defined (MDP_FLAG_USE_RDMA_TO_CROP)
    MDP_INFO_PERSIST("src roi x: %d    y: %d    w: %d    h: %d\n",(unsigned int)p_pipe->src_img_roi.x, (unsigned int)p_pipe->src_img_roi.y, (unsigned int)p_pipe->src_img_roi.w, (unsigned int)p_pipe->src_img_roi.h);
    #endif
    MDP_INFO_PERSIST("tg2 src w: %d    h: %d\n", (unsigned int)p_pipe->src_tg2_img_size.w, (unsigned int)p_pipe->src_tg2_img_size.h);
    #if defined (MDP_FLAG_USE_RDMA_TO_CROP)
    MDP_INFO_PERSIST("tg2 src roi x: %d    y: %d    w: %d    h: %d\n",(unsigned int)p_pipe->src_tg2_img_roi.x, (unsigned int)p_pipe->src_tg2_img_roi.y, (unsigned int)p_pipe->src_tg2_img_roi.w, (unsigned int)p_pipe->src_tg2_img_roi.h);
    #endif

    
    MDP_INFO_PERSIST("en_one_pass_preview_path = %d\n", p_pipe->en_one_pass_preview_path);
    MDP_INFO_PERSIST("en_zero_shutter_path = %d\n", p_pipe->en_zero_shutter_path);
    MDP_INFO_PERSIST("en_sw_trigger = %d\n", p_pipe->en_sw_trigger);
    MDP_INFO_PERSIST("en_n3d_preview_path = %d\n", p_pipe->en_n3d_preview_path);
    MDP_INFO_PERSIST("n3d_working_buff_layout = %d\n", p_pipe->n3d_working_buff_layout);
    MDP_INFO_PERSIST("\n");
    
    MDP_INFO_PERSIST("working(dst) y address = 0x%08x\n", (unsigned int)p_pipe->dst_yuv_img_addr.y );
    MDP_INFO_PERSIST("work w: %d    h: %d\n", (unsigned int)p_pipe->dst_img_size.w, (unsigned int)p_pipe->dst_img_size.h);
    MDP_INFO_PERSIST("work roi x: %d    y: %d    w: %d    h: %d\n",(unsigned int)p_pipe->dst_img_roi.x, (unsigned int)p_pipe->dst_img_roi.y, (unsigned int)p_pipe->dst_img_roi.w, (unsigned int)p_pipe->dst_img_roi.h);
    MDP_INFO_PERSIST("work rotate: %d    color: %d\n", (unsigned int)p_pipe->dst_rotate_angle, (unsigned int)p_pipe->dst_color_format );
    MDP_INFO_PERSIST("work buffer count: %d\n", (unsigned int)p_pipe->dst_buffer_count );
    MDP_INFO_PERSIST("\n");

    MDP_INFO_PERSIST("tg1 working(dst) y address = 0x%08x\n", (unsigned int)p_pipe->working_tg1_buffer_addr.y );
    MDP_INFO_PERSIST("tg1 work w: %d    h: %d\n", (unsigned int)p_pipe->working_tg1_img_size.w, (unsigned int)p_pipe->working_tg1_img_size.h);
    MDP_INFO_PERSIST("tg1 work roi x: %d    y: %d    w: %d    h: %d\n",(unsigned int)p_pipe->working_tg1_img_roi.x, (unsigned int)p_pipe->working_tg1_img_roi.y, (unsigned int)p_pipe->working_tg1_img_roi.w, (unsigned int)p_pipe->working_tg1_img_roi.h);
    MDP_INFO_PERSIST("tg1 work rotate: %d    color: %d\n", (unsigned int)p_pipe->working_tg1_rotate_angle, (unsigned int)p_pipe->working_tg1_color_format );
    MDP_INFO_PERSIST("tg1 work buffer count: %d\n", (unsigned int)p_pipe->working_tg1_buffer_count );
    MDP_INFO_PERSIST("\n");
    
    MDP_INFO_PERSIST("tg2 working(dst) y address = 0x%08x\n", (unsigned int)p_pipe->working_tg2_buffer_addr.y );
    MDP_INFO_PERSIST("tg2 work w: %d    h: %d\n", (unsigned int)p_pipe->working_tg2_img_size.w, (unsigned int)p_pipe->working_tg2_img_size.h);
    MDP_INFO_PERSIST("tg2 work roi x: %d    y: %d    w: %d    h: %d\n",(unsigned int)p_pipe->working_tg2_img_roi.x, (unsigned int)p_pipe->working_tg2_img_roi.y, (unsigned int)p_pipe->working_tg2_img_roi.w, (unsigned int)p_pipe->working_tg2_img_roi.h);
    MDP_INFO_PERSIST("tg2 work rotate: %d    color: %d\n", (unsigned int)p_pipe->working_tg2_rotate_angle, (unsigned int)p_pipe->working_tg2_color_format );
    MDP_INFO_PERSIST("tg2 work buffer count: %d\n", (unsigned int)p_pipe->working_tg2_buffer_count );
    MDP_INFO_PERSIST("\n");
    
    MDP_INFO_PERSIST("dst 0 en=%d\n",p_pipe->en_dst_port0);
    MDP_INFO_PERSIST("dst 0 y address = 0x%08x\n", (unsigned int)p_pipe->dst0_yuv_img_addr.y );
    MDP_INFO_PERSIST("dst 0 w: %d    h: %d\n", (unsigned int)p_pipe->dst0_img_size.w, (unsigned int)p_pipe->dst0_img_size.h);
    MDP_INFO_PERSIST("dst 0 rotate: %d    color: %d\n", (unsigned int)p_pipe->dst0_rotate_angle, (unsigned int)p_pipe->dst0_color_format );
    MDP_INFO_PERSIST("dst 0 buffer count: %d\n", (unsigned int)p_pipe->dst0_buffer_count );
    MDP_INFO_PERSIST("\n");
    MDP_INFO_PERSIST("dst 1 en=%d\n",p_pipe->en_dst_port1);
    MDP_INFO_PERSIST("dst 1 y address = 0x%08x\n", (unsigned int)p_pipe->dst1_yuv_img_addr.y );
    MDP_INFO_PERSIST("dst 1 w: %d    h: %d\n", (unsigned int)p_pipe->dst1_img_size.w, (unsigned int)p_pipe->dst1_img_size.h);
    MDP_INFO_PERSIST("dst 1 rotate: %d    color: %d\n", (unsigned int)p_pipe->dst1_rotate_angle, (unsigned int)p_pipe->dst1_color_format );
    MDP_INFO_PERSIST("dst 1 buffer count: %d\n", (unsigned int)p_pipe->dst1_buffer_count );
    MDP_INFO_PERSIST("\n");
    MDP_INFO_PERSIST("dst 2 en=%d\n",p_pipe->en_dst_port2);
    MDP_INFO_PERSIST("dst 2 y address = 0x%08x\n", (unsigned int)p_pipe->dst2_yuv_img_addr.y );
    MDP_INFO_PERSIST("dst 2 w: %d    h: %d\n", (unsigned int)p_pipe->dst2_img_size.w, (unsigned int)p_pipe->dst2_img_size.h);
    MDP_INFO_PERSIST("dst 2 rotate: %d    color: %d\n", (unsigned int)p_pipe->dst2_rotate_angle, (unsigned int)p_pipe->dst2_color_format );
    MDP_INFO_PERSIST("dst 2 buffer count: %d\n", (unsigned int)p_pipe->dst2_buffer_count );
    MDP_INFO_PERSIST("\n");
    MDP_INFO_PERSIST("CB_EIS_INFO_GET_FUNC: 0x%08X\n", (unsigned int)phalIDPParam->CB_EIS_INFO_GET_FUNC );
    MDP_INFO_PERSIST("\n");
    MDP_INFO_PERSIST("preview resz coeff:\n");
    MDP_INFO_PERSIST("crz_up_scale_coeff=%d\n", p_pipe->resz_coeff.crz_up_scale_coeff );
    MDP_INFO_PERSIST("crz_dn_scale_coeff=%d\n", p_pipe->resz_coeff.crz_dn_scale_coeff );
    MDP_INFO_PERSIST("prz0_up_scale_coeff=%d\n", p_pipe->resz_coeff.prz0_up_scale_coeff );
    MDP_INFO_PERSIST("prz0_dn_scale_coeff=%d\n", p_pipe->resz_coeff.prz0_dn_scale_coeff );
    MDP_INFO_PERSIST("prz0_ee_h_str=%d\n", p_pipe->resz_coeff.prz0_ee_h_str );
    MDP_INFO_PERSIST("prz0_ee_v_str=%d\n", p_pipe->resz_coeff.prz0_ee_v_str );
    MDP_INFO_PERSIST("prz1_up_scale_coeff=%d\n", p_pipe->resz_coeff.prz1_up_scale_coeff );
    MDP_INFO_PERSIST("prz1_dn_scale_coeff=%d\n", p_pipe->resz_coeff.prz1_dn_scale_coeff );
    MDP_INFO_PERSIST("prz1_ee_h_str=%d\n", p_pipe->resz_coeff.prz1_ee_h_str );
    MDP_INFO_PERSIST("prz1_ee_v_str=%d\n", p_pipe->resz_coeff.prz1_ee_v_str );
    MDP_INFO_PERSIST("\n");
    
    
    /*.............................................................................*/
    

     
    return 0;
    

}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpHalImp::setCapJpg(halIDPParam_t *phalIDPParam)
{
    /*Nothing to be implemented.Parameter is already saved.*/
     
    return 0;

}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpHalImp::setConf(halIDPParam_t *phalIDPParam)
{
    MINT32 ret = 0;

    //
    memcpy(&mMDPParam, phalIDPParam, sizeof(halIDPParam_t));
    //
    switch (mMDPParam.mode) {
    case MDP_MODE_PRV_YUV:
        MDP_INFO_PERSIST("Preview Mode.\n");
        ret = setPrv( &mMDPParam );
        break;
    case MDP_MODE_CAP_JPG:
        MDP_INFO_PERSIST("Capture Mode.\n");
        ret = setCapJpg( &mMDPParam );
        break;
    default:
        ret = -1;
        break;
    }
    if (ret < 0) {
        MDP_ERROR("Unknown operation mode!\n");
        return ret;
    }
        
    return ret;
}

MINT32 MdpHalImp::getConf(halIDPParam_t *phalIDPParam)
{
    if( phalIDPParam == NULL )
    {
        MDP_ERROR("phalIDPParam is NULL\n");
        return -1;
    }

    
    memcpy(phalIDPParam, &mMDPParam, sizeof(halIDPParam_t));
    return 0;
}


MINT32 MdpHalImp::dequeueBuff(halMdpOutputPort_e e_Port , halMdpBufInfo_t * a_pstBuffInfo)
{
    
#if defined(MDP_FLAG_PROFILING)
        MdpDrv_Watch _MdpDrvWatch;
        static unsigned long tt0,tt1,tt2,tt3;//total time;
        static unsigned long fc0,fc1,fc2,fc3;//frame count;
        static unsigned long avt0,avt1,avt2,avt3;//avg_time_elapse;
        char   title_str[256];
#endif

    int             ret_val = 0;
    unsigned long   mdp_id, mdp_id_ex;
    stTimeStamp     time_stamp;
    int             i;


    a_pstBuffInfo->hwIndex = 0;
    a_pstBuffInfo->fillCnt = 0;
    
    _MapHalportToMdpId( e_Port, &mdp_id, &mdp_id_ex );

    
    switch( mMDPParam.mode )
    {
    case MDP_MODE_PRV_YUV:
        if( mdp_id != 0 ) 
        {
            
            MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_IT );
            /*1.1 Get free buffer of main mdp element*/
            if( m_mdp_pipe_camera_preview.QueueGetFreeListOrWaitBusy( mdp_id, 
                                                                      (unsigned long*)&(a_pstBuffInfo->hwIndex), 
                                                                      (unsigned long*)&(a_pstBuffInfo->fillCnt)) < 0 ){
                ret_val = -1;
            }

            /*1.2 Get free buffer of sub mdp element if exist*/
            if( mdp_id_ex != 0 ) 
            {
                unsigned long _start,_count;

                if( m_mdp_pipe_camera_preview.QueueGetFreeListOrWaitBusy( mdp_id_ex, &_start, &_count ) < 0 ){
                    ret_val = -1;
                }

                if( _start != a_pstBuffInfo->hwIndex ){
                    MDP_ERROR("connetced 2 mdp do not have the same start pointer.(0x%08X=%d)(0x%08X=%d)\n",
                        (unsigned int)mdp_id, (int)a_pstBuffInfo->hwIndex , (unsigned int)mdp_id_ex , (int)_start);
                    ret_val = -1;
                }

                /*Use thie minum value of free count*/
                a_pstBuffInfo->fillCnt = _count < a_pstBuffInfo->fillCnt? _count : a_pstBuffInfo->fillCnt;
            }

            /*1.3 Update time stamp*/
            //m_mdp_pipe_camera_preview.GetTimeStamp(MID_RGB_ROT0 , &time_stamp);/*Combine display port and video encode port*/
            m_mdp_pipe_camera_preview.GetTimeStamp( (unsigned long)NULL , &time_stamp);/*Combine display port and video encode port*/

            for (i = 0 ; i < MDPHAL_MAX_BUFFER_COUNT ; i += 1) {
                a_pstBuffInfo->timeStamp.timeStampS[i] = time_stamp.sec[i];
                a_pstBuffInfo->timeStamp.timeStampUs[i] = time_stamp.usec[i];

            }

            #if 0
            /*For Time Stamp Debug*/
            if( e_Port == DISP_PORT )
            {
                MDP_INFO_PERSIST("[MdpHalImp][TS] Buffer Type = DISP_PORT, Buffer Index = %2d, sec = %lu, us = %lu\n",
                    a_pstBuffInfo->hwIndex,  
                    (unsigned long )a_pstBuffInfo->timeStamp.timeStampS[a_pstBuffInfo->hwIndex], 
                    (unsigned long )a_pstBuffInfo->timeStamp.timeStampUs[a_pstBuffInfo->hwIndex]);
            }
            #endif
            

            
            #if defined(MDP_FLAG_PROFILING)
            sprintf(title_str, "MdpHalImp::dequeueBuff(%s)",_MapHalportToStr(e_Port));
            #endif

            MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_IT , title_str, &tt0, &fc0, &avt0, 30 );



            #if 0 /*Debug*/
            if( mdp_id == MID_VDO_ROT1 )
            {
                SaveBufferImage( 0 , 30 );
            }
            #endif

            
        }
        break;
        
    case MDP_MODE_CAP_JPG:
        MDP_ERROR("Invalid operation\n");
        break;
    }



     
    return ret_val;


}

MINT32 MdpHalImp::enqueueBuff(halMdpOutputPort_e e_Port)
{
    
#if defined(MDP_FLAG_PROFILING)
    MdpDrv_Watch _MdpDrvWatch;
    static unsigned long tt0,tt1,tt2,tt3;//total time;
    static unsigned long fc0,fc1,fc2,fc3;//frame count;
    static unsigned long avt0,avt1,avt2,avt3;//avg_time_elapse;
    char   title_str[256];
#endif
    MINT32  ret;
    unsigned long mdp_id, mdp_id_ex;

    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_IT );
    

    _MapHalportToMdpId( e_Port, &mdp_id, &mdp_id_ex );

    //Test    
    //SaveBufferImage( 0, 60 );
    //SaveBufferImage( 3, 60 );
    //SaveBufferImage( 1, 60 ); //Save zsd video encode port

    ret = (MINT32)m_mdp_pipe_camera_preview.QueueRefill( mdp_id | mdp_id_ex );

    #if defined(MDP_FLAG_PROFILING)
    sprintf(title_str, "MdpHalImp::enqueueBuff(%s)",_MapHalportToStr(e_Port));
    #endif
    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_IT , title_str, &tt0, &fc0, &avt0, 30 );

    return ret;


   
}

/*******************************************************************************
*
********************************************************************************/
MINT32 MdpHalImp::waitDone(MINT32 mode)
{
    int retval = -1;
    
    MDP_SHOWFUNCTION();

    /*
        1.==>Use check busy for temparary
        */

    switch( mMDPParam.mode )
    {
    case MDP_MODE_PRV_YUV:
        retval = m_mdp_pipe_camera_preview.WaitBusy( NULL );
        break;
    case MDP_MODE_CAP_JPG:
        retval = m_mdp_pipe_camera_capture.WaitBusy( NULL );
        break;
    default:
        retval = -1;
        break;
    }
    return retval;

}

/*******************************************************************************
*
********************************************************************************/


MINT32 MdpHalImp::sendCommand(int cmd, int parg1, int parg2, int parg3)
{
    MINT32  ret = 0;
    rect_t  *pRect;
    MdpRect crop_size;
    MdpRect real_crop_size;

    /*
        1.Special command
        */
     
    MDP_INFO("[MdpHal] sendCommand  cmd: 0x%x \n", cmd);

    switch (cmd) 
    {
    case CMD_SET_ZOOM_RATIO:
        pRect = (rect_t *) parg1;

        /*Config ZOOM only under preview mode*/
        if( mMDPParam.mode != MDP_MODE_PRV_YUV ){
            MDP_ERROR("Config ZOOM not in preview mode\n");
            ret = -1;
            break;
        }


        crop_size.x = pRect->x;
        crop_size.y = pRect->y;
        crop_size.w = pRect->w;
        crop_size.h = pRect->h;
        
        if( m_mdp_pipe_camera_preview.ConfigZoom( crop_size , &real_crop_size ) < 0 )
        {
            MDP_ERROR("Camera Preivew Pipe config zoom fail!\n");
            ret = -1;
        }

        /*If this is zero shutter path, update the video port dimension.
          This may not be a good approach...*/
        if( mMDPParam.en_zero_shutter_path == 0x1)
        {
            mMDPParam.dst_port1.size_w = real_crop_size.w;
            mMDPParam.dst_port1.size_h = real_crop_size.h;
        }

        if( (parg2 != 0) && (parg3 != 0 ) )
        {
            *((unsigned long *)parg2) = real_crop_size.w;
            *((unsigned long *)parg3) = real_crop_size.h;
        }

        MDP_INFO_PERSIST("Zoom Crop      = x: %d y: %d, w: %d, h: %d \n", (int)crop_size.x, (int)crop_size.y, (int)crop_size.w, (int)crop_size.h);
        MDP_INFO_PERSIST("Zoom Real Crop = x: %d y: %d, w: %d, h: %d \n", (int)real_crop_size.x, (int)real_crop_size.y, (int)real_crop_size.w, (int)real_crop_size.h);

        break;
        
    case CMD_GET_JPEG_FILE_SIZE:
        *((int *)parg1) = m_mdp_pipe_camera_capture.jpg_encode_size();
        break;

    // last preview frame for zsd
    case CMD_SET_ZSD_LAST_PREVIEW_FRAME:
        pRect = (rect_t *) parg1;

        crop_size.x = pRect->x;
        crop_size.y = pRect->y;
        crop_size.w = pRect->w;
        crop_size.h = pRect->h;

        MDP_INFO_PERSIST(" CMD_SET_ZSD_LAST_PREVIEW_FRAME\n ");
        m_mdp_pipe_camera_preview.ConfigLastPreviewFrameAndTriggerCurrentDisplay(crop_size);
        break;
    case CMD_SET_ZSD_PREVIEW_FRAME:
        pRect = (rect_t *) parg1;

        crop_size.x = pRect->x;
        crop_size.y = pRect->y;
        crop_size.w = pRect->w;
        crop_size.h = pRect->h;

        MDP_INFO_PERSIST(" CMD_SET_ZSD_PREVIEW_FRAME\n ");
        m_mdp_pipe_camera_preview.ConfigZSDPreviewFrame(crop_size);
        break;
        
    case CMD_TRIGGER_LAST_FRAME_DISPLAY:
        m_mdp_pipe_camera_preview.TiggerLastFrameDisplay();
        break;

    case CMD_N3D_MANUAL_TRIGGER_2ND_PASS:
        {
            /*parameter remapping*/
            MdpRect*    p_tg1roi = (MdpRect*)parg1;
            MdpRect*    p_tg2roi = (MdpRect*)parg2;
            /*~parameter remapping*/

            if( m_mdp_pipe_camera_preview.N3dManualTrigger2ndPass( *p_tg1roi, *p_tg2roi ) < 0 )
            {
                ret = -1;
            }

        }
        break;

    case CMD_GET_BUFFER_ADDRESS:
        {
            /*parameter remapping*/
            halMdpOutputPort_e  mhal_mdp_port       =   (halMdpOutputPort_e) parg1;
            int                 buffer_index        =   (int)                parg2;
            unsigned long       *p_buffer_address   =   (unsigned long*)     parg3;
            /*~parameter remapping*/

            unsigned long   mdp_id, mdp_id_ex;
            MdpYuvAddr      mdp_yuv_addr;
    
            _MapHalportToMdpId( mhal_mdp_port, &mdp_id, &mdp_id_ex );

            if( m_mdp_pipe_camera_preview.GetBufferAddress( mdp_id, buffer_index, &mdp_yuv_addr ) < 0 )
            {
                MDP_ERROR("m_mdp_pipe_camera_preview.GetBufferAddress() error\n");
                ret = -1;
            } else {
                *p_buffer_address = mdp_yuv_addr.y;
            }
        }
        
        break;

    default:
        ret = -1;
        break;
    }

    
    return ret;

}


int MdpHalImp::SaveBufferImage( int port_num , int frame_skip )
{
    char            filename[256];
    int*            p_buffer_count = NULL;
    unsigned long   w,h;
    float           bpp;
    MdpColorFormat  color;
    unsigned long   address;
    static  int buffer_count[4] = { 0, 0, 0, 0 };
    static  int i = 0;


    p_buffer_count = &buffer_count[port_num];

    switch( port_num )
    {
    case 0:
        w = mMDPParam.dst_port0.size_w;
        h = mMDPParam.dst_port0.size_h;
        color = mMDPParam.dst_port0.color_format;
        address = mMDPParam.dst_port0.buffer_addr;
        break;
        
    case 1:
        w = mMDPParam.dst_port1.size_w;
        h = mMDPParam.dst_port1.size_h;
        color = mMDPParam.dst_port1.color_format;
        address = mMDPParam.dst_port1.buffer_addr;
        
        break;
    case 2:
        w = mMDPParam.dst_port2.size_w;
        h = mMDPParam.dst_port2.size_h;
        color = mMDPParam.dst_port2.color_format;
        address = mMDPParam.dst_port2.buffer_addr;
        
        break;
    case 3: //Work ing buffer
        w = m_mdp_pipe_camera_preview_param.dst_img_size.w;
        h = m_mdp_pipe_camera_preview_param.dst_img_size.h;
        color = m_mdp_pipe_camera_preview_param.dst_color_format;
        address = m_mdp_pipe_camera_preview_param.dst_yuv_img_addr.y;
        
        break;


    case 4: //TG1 buffer
        w = m_mdp_pipe_camera_preview_param.working_tg1_img_size.w;
        h = m_mdp_pipe_camera_preview_param.working_tg1_img_size.h;
        color = m_mdp_pipe_camera_preview_param.working_tg1_color_format;
        address = m_mdp_pipe_camera_preview_param.working_tg1_buffer_addr.y;
        break;
        
    case 5: //TG2 buffer
        w = m_mdp_pipe_camera_preview_param.working_tg2_img_size.w;
        h = m_mdp_pipe_camera_preview_param.working_tg2_img_size.h;
        color = m_mdp_pipe_camera_preview_param.working_tg2_color_format;
        address = m_mdp_pipe_camera_preview_param.working_tg2_buffer_addr.y;
        break;

    
    default:
        return -1;
    }

    switch( color )
    {
    /*-----------------------------------------------------------------------------
    RGB
    -----------------------------------------------------------------------------*/
    case RGB888:
    case BGR888:
        bpp = 3;
        break;
    case RGB565:
    case BGR565:
        bpp = 2;
        break;
    case ABGR8888:
    case ARGB8888:
    case BGRA8888:
    case RGBA8888:
        bpp = 4;
        break;

    /*-----------------------------------------------------------------------------
    YUV Packed
    -----------------------------------------------------------------------------*/
    case UYVY_Pack: //YUV422_Pack
    case YUYV_Pack: //YUY2_Pack,YUV422_2_Pack
        bpp = 2;
        break;

    /*-----------------------------------------------------------------------------
    YUV Plane
    -----------------------------------------------------------------------------*/
    case Y411_Pack:  
        bpp = 1.0f + 1.0f/2;
        break;

    //YUV422, 2x1 subsampled U/V planes,only ROTDMA0
    case YV16_Planar:   //YUV422_Planar
        bpp = 1 + 1;
        break;

    //YUV420, 2x2 subsampled U/V planes,only ROTDMA0
    case YV12_Planar:   //YUV420_Planar
    case ANDROID_YV12:  //ANDROID_YV12 YUV420, 2x2 subsampled U/V planes , 16x stride
        bpp = 1.0f + 1.0f/2;
        break;

    //Y plan only,only ROTDMA0
    case Y800:  //Y8 GREY
        bpp = 1;
        break;

    /*-----------------------------------------------------------------------------
    YUV Interleave
    -----------------------------------------------------------------------------*/
    //YUV420, 2x2 subsampled , interleaved U/V plane,only ROTDMA0
    case NV12:  //YUV420_Inter
    //YUV420, 2x2 subsampled , interleaved V/U plane,only ROTDMA0
    case NV21:  //YVU420_Inter
        bpp = 1.0f + 1.0f/2;
        break;


    /*-----------------------------------------------------------------------------
    Misc
    -----------------------------------------------------------------------------*/
    //Output the same data with input,use when jpeg sensor output jpg bitstream.
    //Please note that this is different from sensor's bayer pattern raw data.
    case RAW:
        bpp = 1;
        break;

    //For encoder use,only ROTDMA0 (So called "MTK YUV")
    case YUV420_4x4BLK: //MTK_YUV420
    default:
        MDP_ERROR("Unsupport color format:%d\n", color );
        return -1;
    }



    (*p_buffer_count)++;

    if( ( *p_buffer_count % frame_skip == 0)  )
    {
        sprintf( filename,"/data/PV%d_%lux%lu_%d-%d.raw",port_num, w, h, color, i );
        _SaveImage( filename, address, w*h*bpp );
        MDP_INFO_PERSIST("PV debug image save : %s\n",  filename);
        i++;
        i %= 3;
    }

    return 0;
}




/*******************************************************************************
* Common member function
********************************************************************************/
void MdpHalImp::_MapHalportToMdpId( halMdpOutputPort_e e_Port, unsigned long *p_mdp_id, unsigned long *p_mdp_id_ex )
{
    MdpDrvColorInfo_t   color_info;

    /*Ex for Working Buffer*/
    *p_mdp_id_ex= 0;

    switch( e_Port )
    {
    case DISP_PORT:
        *p_mdp_id       = MID_VDO_ROT0;
        break;
    case VDOENC_PORT:
        *p_mdp_id       = MID_VDO_ROT1;
        break;
    case FD_PORT:
        *p_mdp_id       = MID_RGB_ROT1;
        break;
    case QUICKVIEW_PORT:    /*Not support anymore?*/
        break;
    case JPEG_PORT:         /*Not support anymore?*/
        break;
    case THUMBNAIL_PORT:    /*Not support anymore?*/
        break;
    case PREVIEW_WORKING_BUFFER:
        if( mMDPParam.en_one_pass_preview_path == 1 )
        {
            
            //Decide use me_rgbrot0 or me_vdorot0
            MdpDrvColorFormatInfoGet( mMDPParam.working_color_format , &color_info );
            
            *p_mdp_id = ( color_info.b_is_generic_yuv ) ? MID_VDO_ROT0 : MID_RGB_ROT0; //1: use me_rgbrot0  0: use me_vdorot0
            *p_mdp_id_ex    = 0;
        } 
        else if( mMDPParam.en_zero_shutter_path == 0 || mMDPParam.en_zero_shutter_path == 2)
        {
            /*
            *p_mdp_id       = MID_R_DMA0;
            *p_mdp_id_ex    = MID_RGB_ROT0;
            */
            *p_mdp_id       = MID_RGB_ROT0;
            *p_mdp_id_ex    = MID_R_DMA0;
        }else
        {
            *p_mdp_id       = 0;
            *p_mdp_id_ex    = 0;
        }
        break;
    case ZSD_PORT:
        if( mMDPParam.en_zero_shutter_path == 2)
        {
            *p_mdp_id       = MID_RGB_ROT0_EX;
        }
        break;


    /*Native 3D Camera Support*/
    case N3D_PREVIEW_WORKING_BUFFER_TG1:
        *p_mdp_id = MID_RGB_ROT0;
        break;
    case N3D_PREVIEW_WORKING_BUFFER_TG2:
        *p_mdp_id = MID_VDO_ROT0;
        break;
    case N3D_OUTPUT_PORT:
        *p_mdp_id = MID_VDO_ROT1;   /*TOCHECK:Someting wrong!!This dequeue must perform twice*/
        break;


        
    default:
        *p_mdp_id       = 0;
        *p_mdp_id_ex    = 0;
        MDP_ERROR("halMdpOutputPort_e(%d) map no mdp id\n", (int) e_Port );
        break;
        
    }
    
    
}

const char* MdpHalImp::_MapHalportToStr( halMdpOutputPort_e e_Port )
{

    /*Ex for Working Buffer*/
    switch( e_Port )
    {
    case DISP_PORT:
        return "DISP_PORT";
        break;
    case VDOENC_PORT:
        return "VDOENC_PORT";
        break;
    case FD_PORT:
        return "FD_PORT";
        break;
    case QUICKVIEW_PORT:
        return "QUICKVIEW_PORT";
        break;
    case JPEG_PORT:
        return "JPEG_PORT";
        break;
    case THUMBNAIL_PORT:
        return "THUMBNAIL_PORT";
        break;
    case PREVIEW_WORKING_BUFFER:
        return "PREVIEW_WORKING_BUFFER";
        break;
    case ZSD_PORT:
        return "ZSD_PORT";
        break;
        
        /*N3D Buffer Management*/
    case N3D_PREVIEW_WORKING_BUFFER_TG1: //For native 3d camera support
        return "N3D_PREVIEW_WORKING_BUFFER_TG1";
        break;
    case N3D_PREVIEW_WORKING_BUFFER_TG2: //For native 3d camera support
        return "N3D_PREVIEW_WORKING_BUFFER_TG2";
        break;
    case N3D_OUTPUT_PORT:                 //For native 3d, output L-R side-by-side image
        return "N3D_OUTPUT_PORT";
        break;
    default:
        return "UNKNOWN";
        break;
        
    }
    
    
}






