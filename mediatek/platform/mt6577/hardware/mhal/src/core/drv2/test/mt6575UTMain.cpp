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
#define LOG_TAG "MDP"

#include <stdio.h>



#include "mdp_element.h" //For MdpDrvInit() ( Usr Mode Driver )
#include "mhal_interface.h"//For mhal_interface
#include "mdp_hal_imp.h"    //For mdp_hal test
#include "MediaHal.h"       //For MHAL color format /mHalBltParam_t
#include "mdp_path.h"


#if !defined(MTK_M4U_SUPPORT)
#include <cutils/pmem.h>    //pmem
#endif

#include <string.h>         //memcpy
#include <errno.h>          //strerror
#include <sys/time.h>       //gettimeofday



#include "mt6575UTMain_rsc.h"   //For image resource

/*-----------------------------------------------------------------------------
    Definition
  -----------------------------------------------------------------------------*/
#define BUFFER_COUNT    (1)//(12)


/*-----------------------------------------------------------------------------
    Data Structure
  -----------------------------------------------------------------------------*/
struct ImgBuffer
{
    unsigned long   size;
    unsigned long   vir_addr;
    unsigned long   phy_addr;
    int             pmem_fd;  
};

struct ImgBuffer g_src_buffer;
struct ImgBuffer g_dst_buffer;

/*-----------------------------------------------------------------------------
    Functions
  -----------------------------------------------------------------------------*/

int SaveImage( char* filename , unsigned long addr, unsigned long size )
{
    FILE *fp;

    fp = fopen( filename, "w");

    if(  fp == NULL )
    {
        printf("Open file failed.:%s\n", strerror(errno));
        return -1;
    }

    if( fwrite( (void*)addr , 1 , size , fp) < size )
    {
       printf("write file failed.:%s\n", strerror(errno));
    }
    
    fclose(fp);
    printf("File saved : %s\n", filename );

    return 0;
}

int PrepareTestImage( void )
{
    int i;
    const int kBUFFER_COUNT = BUFFER_COUNT;
    const int kDST_BUFFER_MARGIN_FACTOR = 10;   /*is normal*/

    /*Source Buffer*/
    /*Allocate a pmem buffer to store the pre-build image in heap*/
    g_src_buffer.size = SRC_BUFFER_SIZE * kBUFFER_COUNT;

    #if defined(MTK_M4U_SUPPORT)
    if(  (g_src_buffer.vir_addr = (unsigned long)malloc(g_src_buffer.size) ) == NULL )
    #else
    if(  (g_src_buffer.vir_addr = (unsigned long)pmem_alloc_sync(g_src_buffer.size, &g_src_buffer.pmem_fd ) ) == NULL )
    #endif
    {
        printf("Can not allocate src memory\n\r");
        return -1;
    }

    #if !defined(MTK_M4U_SUPPORT)
    g_src_buffer.phy_addr = (unsigned int)pmem_get_phys( g_src_buffer.pmem_fd );
    #endif

    for( i = 0 ; i < kBUFFER_COUNT; i++ )
    {
        memcpy( (void *)(g_src_buffer.vir_addr + i*SRC_BUFFER_SIZE) , (void*)g_src_file, SRC_BUFFER_SIZE);
    }
    

    printf("SRC Img va:0x%x, pa:0x%x, size:%d, fd:%d\n\r",(unsigned int)g_src_buffer.vir_addr , (unsigned int)g_src_buffer.phy_addr , SRC_BUFFER_SIZE , g_src_buffer.pmem_fd );



    /*Destination Buffer*/
    #if 1 /*Normal case,use more memory*/
    g_dst_buffer.size = kDST_BUFFER_MARGIN_FACTOR * (DEST_WIDTH*DEST_HEIGHT*4*3) * kBUFFER_COUNT; /*Worst case: ARGB(*4) & 1-to-3 in-out(*3)*/
    #else /*For BIG image buffer*/
    g_dst_buffer.size = kDST_BUFFER_MARGIN_FACTOR * (DEST_WIDTH*DEST_HEIGHT*2) * kBUFFER_COUNT; /*Worst case: ARGB(*4) & 1-to-3 in-out(*3)*/
    #endif

    #if defined(MTK_M4U_SUPPORT)
    if( (g_dst_buffer.vir_addr = (unsigned long)malloc( g_dst_buffer.size )) == NULL )
    #else
    if( (g_dst_buffer.vir_addr = (unsigned long)pmem_alloc_sync( g_dst_buffer.size, &g_dst_buffer.pmem_fd )) == NULL )
    #endif
    {
        printf("Can not allocate dst memory\n\n\r");
        return -1;
    }
    #if !defined(MTK_M4U_SUPPORT)
    g_dst_buffer.phy_addr = (unsigned int)pmem_get_phys( g_dst_buffer.pmem_fd );
    #endif

    memset( (void*)g_dst_buffer.vir_addr , 0 , g_dst_buffer.size);


    printf("DST Img va:0x%x, pa:0x%x, size:%d, fd:%d\n\r",(unsigned int)g_dst_buffer.vir_addr , (unsigned int)g_dst_buffer.phy_addr, (int)g_dst_buffer.size, (int)g_dst_buffer.pmem_fd);


    return 0;
}


int  TestFunc_Mt6575_ImageTransform( void )
{
    MDP_SHOWFUNCTION();
    
    #if defined(MTK_M4U_SUPPORT)
    unsigned long src_addr = g_src_buffer.vir_addr;
    unsigned long dst_addr = g_dst_buffer.vir_addr;
    #else
    unsigned long src_addr = g_src_buffer.phy_addr;
    unsigned long dst_addr = g_dst_buffer.phy_addr;
    #endif
 

    /*-----------------------------------------------------------------------------
        Invoke
      -----------------------------------------------------------------------------*/
    {
        MdpPathImageTransformParameter  itparam;
        
        itparam.src_color_format = MhalColorFormatToMdp( MHAL_SRC_COLOR_FMT );
        itparam.src_img_size = MdpSize( SRC_WIDTH, SRC_HEIGHT );
        itparam.src_img_roi = MdpRect( 0, 0, CROP_WIDTH, CROP_HEIGHT );
        itparam.src_yuv_img_addr.y = src_addr;

        
        itparam.dst_color_format = IMG_YV12;//MhalColorFormatToMdp( MHAL_FORMAT_YUV_420 );
        //itparam.dst_color_format = MhalColorFormatToMdp( MHAL_FORMAT_RGB_888 );
        itparam.dst_img_size = MdpSize( DEST_WIDTH, DEST_HEIGHT );       /*Currently, "dst roi"  should be equal to "dst size"*/
        itparam.dst_img_roi = MdpRect( 0, 0, DEST_WIDTH, DEST_HEIGHT );  /*dst_img_roi is not used*/
        itparam.dst_yuv_img_addr.y = dst_addr;
        itparam.dst_rotate_angle = 0;
        itparam.dst_dither_en = 1;

        //MdpPathImageTransformYuv_3( &itparam );
        MdpPathImageTransformRgb_4( &itparam );

        #if 1
        itparam.src_color_format = IMG_YV12;
        itparam.src_img_size = MdpSize( DEST_WIDTH, DEST_HEIGHT );
        itparam.src_img_roi = MdpRect( 0, 0, DEST_WIDTH, DEST_HEIGHT ); 
        itparam.src_yuv_img_addr.y = dst_addr;

        
        itparam.dst_color_format = MhalColorFormatToMdp( MHAL_SRC_COLOR_FMT );
        itparam.dst_img_size = MdpSize( SRC_WIDTH, SRC_HEIGHT );       /*Currently, "dst roi"  should be equal to "dst size"*/
        itparam.dst_img_roi = MdpRect( 0, 0, CROP_WIDTH, CROP_HEIGHT );  /*dst_img_roi is not used*/
        itparam.dst_yuv_img_addr.y = dst_addr + DEST_WIDTH*DEST_HEIGHT*3;
        itparam.dst_rotate_angle = 0;
        itparam.dst_dither_en = 1;

        MdpPathImageTransformRgb_4( &itparam );
        
        #endif

    }
    
    SaveImage( "/data/mHalBitblt-src.raw", src_addr, DEST_WIDTH*DEST_HEIGHT*3);
    SaveImage( "/data/mHalBitblt-dst.raw", dst_addr, DEST_WIDTH*DEST_HEIGHT*3);
    SaveImage( "/data/mHalBitblt-last.raw", dst_addr + DEST_WIDTH*DEST_HEIGHT*3, DEST_WIDTH*DEST_HEIGHT*3);
    


    return 0;


    
}


int  TestFunc_Mt6575_mHalBitblt( void )
{
    int ret = 0;
    MDP_SHOWFUNCTION();
    
    #if defined(MTK_M4U_SUPPORT)
    unsigned long src_addr = g_src_buffer.vir_addr;
    unsigned long dst_addr = g_dst_buffer.vir_addr;
    #else
    unsigned long src_addr = g_src_buffer.phy_addr;
    unsigned long dst_addr = g_dst_buffer.phy_addr;
    #endif
 

    /*-----------------------------------------------------------------------------
        Invoke
      -----------------------------------------------------------------------------*/
    {
        mHalBltParam_t bltParam;
        
        bltParam.srcX = 0;
        bltParam.srcY = 0;
        bltParam.srcW = CROP_WIDTH;
        bltParam.srcWStride = SRC_WIDTH;
        bltParam.srcH = CROP_HEIGHT;
        bltParam.srcHStride = SRC_HEIGHT;
        bltParam.srcAddr = src_addr;
        bltParam.srcFormat = MHAL_SRC_COLOR_FMT;
        
        bltParam.dstW = DEST_WIDTH;
        bltParam.dstH = DEST_HEIGHT;
        bltParam.dstAddr = dst_addr;

        //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
        bltParam.dstFormat = MHAL_FORMAT_RGB_888;

        bltParam.pitch = DEST_WIDTH;

        bltParam.orientation = 0; //TODO: test 5
        
        ret = Mt6575_mHalBitblt((void *)&bltParam); /*mt6575 prefix should not add, this should be platform independent implement*/

    }
    
    SaveImage( "/data/mHalBitblt-src.raw", src_addr, DEST_WIDTH*DEST_HEIGHT*3);
    SaveImage( "/data/mHalBitblt-dst.raw", dst_addr, DEST_WIDTH*DEST_HEIGHT*3);

    printf("MDP Test Case : TestFunc_Mt6575_mHalBitblt : Result = %d\n", ret );
        
    


    return 0;


    
}

int  TestFunc_Mt6575_mHalBitblt_cust( void )
{
    MDP_SHOWFUNCTION();
    
    #if defined(MTK_M4U_SUPPORT)
    unsigned long src_addr = g_src_buffer.vir_addr;
    unsigned long dst_addr = g_dst_buffer.vir_addr;
    #else
    unsigned long src_addr = g_src_buffer.phy_addr;
    unsigned long dst_addr = g_dst_buffer.phy_addr;
    #endif

    
    /*-----------------------------------------------------------------------------
        Invoke
      -----------------------------------------------------------------------------*/
    {
        mHalBltParam_t bltParam;

        memset( &bltParam, 0, sizeof(mHalBltParam_t));
        
        bltParam.srcX = 0;
        bltParam.srcY = 0;
        bltParam.srcW = CROP_WIDTH;
        bltParam.srcWStride = SRC_WIDTH;
        bltParam.srcH = CROP_HEIGHT;
        bltParam.srcHStride = SRC_HEIGHT;
        bltParam.srcAddr = src_addr;
        bltParam.srcFormat = MHAL_SRC_COLOR_FMT;
        
        bltParam.dstW = DEST_WIDTH;
        bltParam.dstH = DEST_HEIGHT;
        bltParam.dstAddr = dst_addr;

        //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
        //bltParam.dstFormat = MHAL_FORMAT_RGB_888;
        //bltParam.dstFormat = MHAL_FORMAT_Y800;
            //bltParam.dstFormat = MHAL_FORMAT_ABGR_8888;
            //bltParam.dstFormat = MHAL_FORMAT_ARGB_8888;
            bltParam.dstFormat = MHAL_FORMAT_BGRA_8888;
            //bltParam.dstFormat = MHAL_FORMAT_RGBA_8888;

        bltParam.pitch = DEST_WIDTH;

        bltParam.orientation = 0; //TODO: test 5

        bltParam.favor_flags = ITFF_USE_CRZ;
        
        Mt6575_mHalBitblt((void *)&bltParam); /*mt6575 prefix should not add, this should be platform independent implement*/

        #if 1

        
        memset( &bltParam, 0, sizeof(mHalBltParam_t));
        
        bltParam.srcX = 0;
        bltParam.srcY = 0;
        bltParam.srcW = DEST_WIDTH;
        bltParam.srcWStride = DEST_WIDTH;
        bltParam.srcH = DEST_HEIGHT;
        bltParam.srcHStride = DEST_HEIGHT;
        bltParam.srcAddr = dst_addr;

            //bltParam.srcFormat = MHAL_FORMAT_RGB_888;
            //bltParam.srcFormat = MHAL_FORMAT_ABGR_8888;
            //bltParam.srcFormat = MHAL_FORMAT_YUV_420;
            //bltParam.srcFormat = MHAL_FORMAT_ARGB_8888;
            bltParam.srcFormat = MHAL_FORMAT_BGRA_8888;
            //bltParam.srcFormat = MHAL_FORMAT_RGBA_8888;
            
        
        bltParam.dstW = SRC_WIDTH;
        bltParam.dstH = SRC_HEIGHT;
        bltParam.dstAddr = dst_addr + DEST_WIDTH*DEST_HEIGHT*4;

        //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
        //bltParam.dstFormat = MHAL_FORMAT_RGB_888;
            bltParam.dstFormat = MHAL_FORMAT_BGR_888;
        //bltParam.dstFormat = MHAL_FORMAT_Y800;
            //bltParam.dstFormat = MHAL_FORMAT_RGB_565;
            //bltParam.dstFormat = MHAL_FORMAT_BGR_565;

        bltParam.pitch = SRC_WIDTH;

        bltParam.orientation = 0; //TODO: test 5

        bltParam.favor_flags = ITFF_USE_CRZ;

        
        Mt6575_mHalBitblt((void *)&bltParam); /*mt6575 prefix should not add, this should be platform independent implement*/
        
        #endif

    }

    SaveImage( "/data/mHalBitblt-src.raw", src_addr, DEST_WIDTH*DEST_HEIGHT*4);
    SaveImage( "/data/mHalBitblt-dst.raw", dst_addr, DEST_WIDTH*DEST_HEIGHT*4);
    SaveImage( "/data/mHalBitblt-last.raw", dst_addr + DEST_WIDTH*DEST_HEIGHT*4, DEST_WIDTH*DEST_HEIGHT*4);
    


    return 0;


    
}


int  TestFunc_Mt6575_mHalBitblt_cust2( void )
{
    MDP_SHOWFUNCTION();
    
#if defined(MTK_M4U_SUPPORT)
    unsigned long src_addr = g_src_buffer.vir_addr;
    unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
    unsigned long src_addr = g_src_buffer.phy_addr;
    unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif
 

    /*-----------------------------------------------------------------------------
        Invoke
      -----------------------------------------------------------------------------*/
    {
        int i;
        mHalRegisterLoopMemory_t    loop_mem_param[2];
        mHalRegisterLoopMemoryObj_t loop_mem_obj[2];
        MHAL_BITBLT_FORMAT_ENUM     MHAL_DST_COLOR_FMT = MHAL_FORMAT_ABGR_8888;
        mHalBltParam_t              bltParam;


        /*.............................................................................
          1.Register 2 MDP loop memory,one for input, the other for output
          .............................................................................*/
        loop_mem_param[0].mem_type      = MHAL_MEM_TYPE_INPUT;
        loop_mem_param[0].addr          = src_addr;
        loop_mem_param[0].buffer_size   = SRC_WIDTH*SRC_HEIGHT*4;
        loop_mem_param[0].mhal_color    = MHAL_SRC_COLOR_FMT;
        loop_mem_param[0].img_size      = mHalMdpSize(SRC_WIDTH, SRC_HEIGHT);
        loop_mem_param[0].img_roi       = mHalMdpRect( 0, 0, CROP_WIDTH, CROP_HEIGHT);
        loop_mem_param[0].rotate        = 0;
        
        mHalMdp_RegisterLoopMemory( MHAL_MLM_CLIENT_MAX,&loop_mem_param[0], &loop_mem_obj[0] );


        
        loop_mem_param[1].mem_type      = MHAL_MEM_TYPE_OUTPUT;
        loop_mem_param[1].addr          = dst_addr;
        loop_mem_param[1].buffer_size   = DEST_WIDTH*DEST_HEIGHT*4; 
        loop_mem_param[1].mhal_color    = MHAL_DST_COLOR_FMT;
        loop_mem_param[1].img_size      = mHalMdpSize(DEST_WIDTH, DEST_HEIGHT);
        loop_mem_param[1].img_roi       = mHalMdpRect( 0, 0, DEST_WIDTH, DEST_HEIGHT);
        loop_mem_param[1].rotate        = 0;

        mHalMdp_RegisterLoopMemory( MHAL_MLM_CLIENT_MAX,&loop_mem_param[1], &loop_mem_obj[1] );


        
        /*.............................................................................
          2.Do MDP Bitblt fro 10 times
          .............................................................................*/

        //for( i = 0; i < 10; i++ )
        while( 1 )
        {

            memset( &bltParam, 0, sizeof(mHalBltParam_t));
            
            bltParam.srcX = 0;
            bltParam.srcY = 0;
            bltParam.srcW = CROP_WIDTH;
            bltParam.srcWStride = SRC_WIDTH;
            bltParam.srcH = CROP_HEIGHT;
            bltParam.srcHStride = SRC_HEIGHT;
            bltParam.srcAddr = src_addr;
            bltParam.srcFormat = MHAL_SRC_COLOR_FMT;
            
            bltParam.dstW = DEST_WIDTH;
            bltParam.dstH = DEST_HEIGHT;
            bltParam.dstAddr = dst_addr;

            bltParam.dstFormat = MHAL_DST_COLOR_FMT;

            bltParam.pitch = DEST_WIDTH;

            bltParam.orientation = 0; //TODO: test 5

            bltParam.favor_flags = ITFF_USE_CRZ;
            
            Mt6575_mHalBitblt((void *)&bltParam); /*mt6575 prefix should not add, this should be platform independent implement*/
        }

        
        /*.............................................................................
          2.UnRegister MDP loop memory
          .............................................................................*/
        mHalMdp_UnRegisterLoopMemory( MHAL_MLM_CLIENT_MAX, &loop_mem_obj[0] );
        mHalMdp_UnRegisterLoopMemory( MHAL_MLM_CLIENT_MAX, &loop_mem_obj[1] );

    }
    
    SaveImage( "/data/mHalBitblt-src.raw", src_addr, DEST_WIDTH*DEST_HEIGHT*4);
    SaveImage( "/data/mHalBitblt-dst.raw", dst_addr, DEST_WIDTH*DEST_HEIGHT*4);
    //SaveImage( "/data/mHalBitblt-last.raw", dst_addr + DEST_WIDTH*DEST_HEIGHT*4, DEST_WIDTH*DEST_HEIGHT*4);
    


    return 0;


    
}



int  TestFunc_Mt6575_mHalBitblt_cust_PerformanceTest( void )
{
    const int EXE_COUNT = 100;
    struct timeval  tv0,tv1;
    struct timezone tz0,tz1;
    unsigned long time_elapse;
    unsigned long single_time_elapse;
    unsigned long max_single,min_single;
    unsigned long data_throughput;
    int i;

    
    MDP_SHOWFUNCTION();
    
    #if defined(MTK_M4U_SUPPORT)
    unsigned long src_addr = g_src_buffer.vir_addr;
    unsigned long dst_addr = g_dst_buffer.vir_addr;
    #else
    unsigned long src_addr = g_src_buffer.phy_addr;
    unsigned long dst_addr = g_dst_buffer.phy_addr;
    #endif

    //int gettimeofday(struct timeval *tv, struct timezone *tz); 

    
    
    /*-----------------------------------------------------------------------------
        Invoke
      -----------------------------------------------------------------------------*/
    //----!RGB Bitblt
    
    time_elapse = 0;
    max_single = 0x0;
    min_single = 0xFFFFFFFF;
    for( i = 0; i < EXE_COUNT; i++ )
    {
        mHalBltParam_t bltParam;
        
        memset( &bltParam, 0 , sizeof(bltParam) );
        
        bltParam.srcX = 0;
        bltParam.srcY = 0;
        bltParam.srcW = CROP_WIDTH;
        bltParam.srcWStride = SRC_WIDTH;
        bltParam.srcH = CROP_HEIGHT;
        bltParam.srcHStride = SRC_HEIGHT;
        bltParam.srcAddr = src_addr;
        bltParam.srcFormat = MHAL_SRC_COLOR_FMT;
        
        bltParam.dstW = DEST_WIDTH;
        bltParam.dstH = DEST_HEIGHT;
        bltParam.dstAddr = dst_addr;

        //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
        bltParam.dstFormat = MHAL_FORMAT_RGB_888;

        bltParam.pitch = DEST_WIDTH;

        bltParam.orientation = 0; //TODO: test 5
        
        gettimeofday( &tv0, &tz0 );
        
        Mt6575_mHalBitblt((void *)&bltParam); /*mt6575 prefix should not add, this should be platform independent implement*/

    gettimeofday( &tv1, &tz1 );

        single_time_elapse = (tv1.tv_sec - tv0.tv_sec)*1000000 + (tv1.tv_usec - tv0.tv_usec);
        max_single = ( single_time_elapse > max_single ) ? single_time_elapse:max_single;
        min_single = ( single_time_elapse < min_single ) ? single_time_elapse:min_single;
    
        time_elapse += single_time_elapse;

    }

    data_throughput = ( ( SRC_WIDTH*SRC_HEIGHT*3 ) + ( DEST_WIDTH*DEST_HEIGHT*3)  ) * EXE_COUNT;
    data_throughput = data_throughput/time_elapse; /*B/us*/
    data_throughput = data_throughput; /*MB/s*/

    printf("time_elapse = %dus\n",time_elapse);
    printf("AVG time_elapse(RGB888->RGB888) = %dus\n",time_elapse/EXE_COUNT);
    printf("min/MAX time_elapse = %dus/%dus\n",min_single, max_single );
    printf("Throughput = %dMB/s\n",data_throughput);
    

    
    //SaveImage( "/data/mHalBitblt-src.raw", src_addr, DEST_WIDTH*DEST_HEIGHT*3);
    SaveImage( "/data/mHalBitblt-dst.raw", dst_addr, DEST_WIDTH*DEST_HEIGHT*3);
    

    //----!YUV Bitblt
    
    time_elapse = 0;
    max_single = 0x0;
    min_single = 0xFFFFFFFF;
    for( i = 0; i < EXE_COUNT; i++ )
    {
        mHalBltParam_t bltParam;
        
        memset( &bltParam, 0 , sizeof(bltParam) );
        
        bltParam.srcX = 0;
        bltParam.srcY = 0;
        bltParam.srcW = CROP_WIDTH;
        bltParam.srcWStride = SRC_WIDTH;
        bltParam.srcH = CROP_HEIGHT;
        bltParam.srcHStride = SRC_HEIGHT;
        bltParam.srcAddr = src_addr;
        bltParam.srcFormat = MHAL_SRC_COLOR_FMT;
        
        bltParam.dstW = DEST_WIDTH;
        bltParam.dstH = DEST_HEIGHT;
        bltParam.dstAddr = dst_addr;

        bltParam.dstFormat = MHAL_FORMAT_YUV_420;
        //bltParam.dstFormat = MHAL_FORMAT_RGB_888;

        bltParam.pitch = DEST_WIDTH;

        bltParam.orientation = 0; //TODO: test 5
        
        
        gettimeofday( &tv0, &tz0 );
        
        Mt6575_mHalBitblt((void *)&bltParam); /*mt6575 prefix should not add, this should be platform independent implement*/

        gettimeofday( &tv1, &tz1 );
        
        single_time_elapse = (tv1.tv_sec - tv0.tv_sec)*1000000 + (tv1.tv_usec - tv0.tv_usec);
        max_single = ( single_time_elapse > max_single ) ? single_time_elapse:max_single;
        min_single = ( single_time_elapse < min_single ) ? single_time_elapse:min_single;

        time_elapse += single_time_elapse;

    }
    
    data_throughput = ( ( SRC_WIDTH*SRC_HEIGHT*3 ) + ( DEST_WIDTH*DEST_HEIGHT*1.5)  ) * EXE_COUNT;
    data_throughput = data_throughput/time_elapse; /*B/us*/
    data_throughput = data_throughput; /*MB/s*/

    printf("time_elapse = %dus\n",time_elapse);
    printf("AVG time_elapse(RGB888->YUV420) = %dus\n",time_elapse/EXE_COUNT);
    printf("min/MAX time_elapse = %dus/%dus\n",min_single, max_single );
    printf("Throughput = %dMB/s\n",data_throughput);
    
    
    //SaveImage( "/data/mHalBitblt-src.raw", src_addr, DEST_WIDTH*DEST_HEIGHT*3);
    SaveImage( "/data/mHalBitblt-dst.raw", dst_addr, DEST_WIDTH*DEST_HEIGHT*2);

#if 1
        //----!RGB IPC Bitblt

    time_elapse = 0;
    max_single = 0x0;
    min_single = 0xFFFFFFFF;
    for( i = 0; i < EXE_COUNT; i++ )
    {
        mHalBltParam_t bltParam;

        memset( &bltParam, 0 , sizeof(bltParam) );
        
        bltParam.srcX = 0;
        bltParam.srcY = 0;
        bltParam.srcW = CROP_WIDTH;
        bltParam.srcWStride = SRC_WIDTH;
        bltParam.srcH = CROP_HEIGHT;
        bltParam.srcHStride = SRC_HEIGHT;
        bltParam.srcAddr = src_addr;
        bltParam.srcFormat = MHAL_SRC_COLOR_FMT;
        
        bltParam.dstW = DEST_WIDTH;
        bltParam.dstH = DEST_HEIGHT;
        bltParam.dstAddr = dst_addr;

        //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
        bltParam.dstFormat = MHAL_FORMAT_RGB_888;

        bltParam.pitch = DEST_WIDTH;

        bltParam.orientation = 0; //TODO: test 5

        
        gettimeofday( &tv0, &tz0 );
        
        mHalMdpIpc_BitBlt(&bltParam); /*mt6575 prefix should not add, this should be platform independent implement*/

        gettimeofday( &tv1, &tz1 );
        
        single_time_elapse = (tv1.tv_sec - tv0.tv_sec)*1000000 + (tv1.tv_usec - tv0.tv_usec);
        max_single = ( single_time_elapse > max_single ) ? single_time_elapse:max_single;
        min_single = ( single_time_elapse < min_single ) ? single_time_elapse:min_single;

        time_elapse += single_time_elapse;

    }
    
    data_throughput = ( ( SRC_WIDTH*SRC_HEIGHT*3 ) + ( DEST_WIDTH*DEST_HEIGHT*3)  ) * EXE_COUNT;
    data_throughput = data_throughput/time_elapse; /*B/us*/
    data_throughput = data_throughput; /*MB/s*/

    printf("time_elapse = %dus\n",time_elapse);
    printf("AVG time_elapse(IPC RGB888->RGB888) = %dus\n",time_elapse/EXE_COUNT);
    printf("min/MAX time_elapse = %dus/%dus\n",min_single, max_single );
    printf("Throughput = %dMB/s\n",data_throughput);
    

    
    //SaveImage( "/data/mHalBitblt-src.raw", src_addr, DEST_WIDTH*DEST_HEIGHT*3);
    SaveImage( "/data/mHalBitblt-dst.raw", dst_addr, DEST_WIDTH*DEST_HEIGHT*3);


    //----!YUV IPC Bitblt
    
    time_elapse = 0;
    max_single = 0x0;
    min_single = 0xFFFFFFFF;
    for( i = 0; i < EXE_COUNT; i++ )
    {
        mHalBltParam_t bltParam;

        memset( &bltParam, 0 , sizeof(bltParam) );
        
        bltParam.srcX = 0;
        bltParam.srcY = 0;
        bltParam.srcW = CROP_WIDTH;
        bltParam.srcWStride = SRC_WIDTH;
        bltParam.srcH = CROP_HEIGHT;
        bltParam.srcHStride = SRC_HEIGHT;
        bltParam.srcAddr = src_addr;
        bltParam.srcFormat = MHAL_SRC_COLOR_FMT;
        
        bltParam.dstW = DEST_WIDTH;
        bltParam.dstH = DEST_HEIGHT;
        bltParam.dstAddr = dst_addr;

        bltParam.dstFormat = MHAL_FORMAT_YUV_420;
        //bltParam.dstFormat = MHAL_FORMAT_RGB_888;

        bltParam.pitch = DEST_WIDTH;

        bltParam.orientation = 0; //TODO: test 5

        
        gettimeofday( &tv0, &tz0 );
        
        mHalMdpIpc_BitBlt(&bltParam); /*mt6575 prefix should not add, this should be platform independent implement*/

    gettimeofday( &tv1, &tz1 );

        single_time_elapse = (tv1.tv_sec - tv0.tv_sec)*1000000 + (tv1.tv_usec - tv0.tv_usec);
        max_single = ( single_time_elapse > max_single ) ? single_time_elapse:max_single;
        min_single = ( single_time_elapse < min_single ) ? single_time_elapse:min_single;

        time_elapse += single_time_elapse;

    }
    
    data_throughput = ( ( SRC_WIDTH*SRC_HEIGHT*3 ) + ( DEST_WIDTH*DEST_HEIGHT*1.5)  ) * EXE_COUNT;
    data_throughput = data_throughput/time_elapse; /*B/us*/
    data_throughput = data_throughput; /*MB/s*/

    printf("time_elapse = %dus\n",time_elapse);
    printf("AVG time_elapse(IPC RGB888->YUV420) = %dus\n",time_elapse/EXE_COUNT);
    printf("min/MAX time_elapse = %dus/%dus\n",min_single, max_single );
    printf("Throughput = %dMB/s\n",data_throughput);
    
    
    //SaveImage( "/data/mHalBitblt-src.raw", src_addr, DEST_WIDTH*DEST_HEIGHT*3);
    SaveImage( "/data/mHalBitblt-dst.raw", dst_addr, DEST_WIDTH*DEST_HEIGHT*2);
    
#endif


    return 0;


    
}





int  TestFunc_Mt6575_mHalBitbltSlice( void )
{
    MDP_SHOWFUNCTION();
    
    #if defined(MTK_M4U_SUPPORT)
    unsigned long src_addr = g_src_buffer.vir_addr;
    unsigned long dst_addr = g_dst_buffer.vir_addr;
    #else
    unsigned long src_addr = g_src_buffer.phy_addr;
    unsigned long dst_addr = g_dst_buffer.phy_addr;
    #endif

    int     i;
    char    str[512];
 

    /*-----------------------------------------------------------------------------
        Invoke
      -----------------------------------------------------------------------------*/
    
    sprintf( str, "/data/Mdp_BitbltSlice-src-%dx%d.bin", SRC_WIDTH, SRC_HEIGHT);
    SaveImage( str, src_addr, DEST_WIDTH*DEST_HEIGHT*2);

    for( i = 0; i <= 7; i++ )
    {
        mHalBltParam_t bltParam;
        
        bltParam.srcX = 0;
        bltParam.srcY = 0;
        bltParam.srcW = CROP_WIDTH;
        bltParam.srcWStride = SRC_WIDTH;
        bltParam.srcH = CROP_HEIGHT;
        bltParam.srcHStride = SRC_HEIGHT;
        bltParam.srcAddr = src_addr;
        bltParam.srcFormat = MHAL_SRC_COLOR_FMT;
        
        bltParam.dstW = DEST_WIDTH;
        bltParam.dstH = DEST_HEIGHT;
        bltParam.dstAddr = dst_addr;

        //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
        bltParam.dstFormat = MHAL_FORMAT_RGB_888;

        bltParam.pitch = (i%2==1)?DEST_HEIGHT:DEST_WIDTH;

        bltParam.orientation = i;//0; //TODO: test 5
        
        Mdp_BitbltSlice((void *)&bltParam); /*mt6575 prefix should not add, this should be platform independent implement*/

        sprintf( str, "/data/Mdp_BitbltSlice-dst-%d.bin", i );
        SaveImage( str, dst_addr, DEST_WIDTH*DEST_HEIGHT*3);

    }
    
    //SaveImage( "/data/mHalBitblt-src.raw", src_addr, DEST_WIDTH*DEST_HEIGHT*3);
    //SaveImage( "/data/mHalBitblt-dst.raw", dst_addr, DEST_WIDTH*DEST_HEIGHT*3);
    


    return 0;


    
}







int  TestFunc_Mt6575_mHalMdp_BitBltEx( void )
{
    MDP_SHOWFUNCTION();

    int ret = 0;
    
    #if defined(MTK_M4U_SUPPORT)
    unsigned long src_addr = g_src_buffer.vir_addr;
    unsigned long dst_addr = g_dst_buffer.vir_addr;
    #else
    unsigned long src_addr = g_src_buffer.phy_addr;
    unsigned long dst_addr = g_dst_buffer.phy_addr;
    #endif

    int     i;
    char    str[512];
 

    /*-----------------------------------------------------------------------------
        Invoke
      -----------------------------------------------------------------------------*/

    {

        mHalBltParamEx_t param;
        unsigned long hdmi_out_start;
        unsigned long disp_out_start;

        hdmi_out_start = dst_addr;
        disp_out_start = dst_addr + DEST_HEIGHT*DEST_HEIGHT*6;
        
        

        memset( &param, 0, sizeof(param));
        
        
        param.src_color_format = MHAL_SRC_COLOR_FMT;
        
        param.src_img_size_w = SRC_WIDTH;
        param.src_img_size_h = SRC_HEIGHT;

        param.src_img_roi_x = 0;
        param.src_img_roi_y = 0;
        param.src_img_roi_w = SRC_WIDTH;
        param.src_img_roi_h = SRC_HEIGHT;
        
        param.src_yuv_img_addr = src_addr;
        

        /*For HDMI : RGB*/
        param.en_hdmi_port = 1;           //enable hdmi port
            param.hdmi_color_format = MHAL_FORMAT_UYVY;

            param.hdmi_img_size_w = DEST_WIDTH;
            param.hdmi_img_size_h = DEST_HEIGHT;

            
            param.hdmi_img_roi_x = 0;
            param.hdmi_img_roi_y = 0;
            param.hdmi_img_roi_w = DEST_WIDTH;
            param.hdmi_img_roi_h = DEST_HEIGHT;
            
            param.hdmi_yuv_img_addr = hdmi_out_start;
            param.hdmi_rotate_angle = 1;

        /*For Display : 422 Pack*/
        param.en_disp_port = 1;           //enable disp port
            param.disp_color_format = MHAL_FORMAT_UYVY;;
            
            param.disp_img_size_w = DEST_WIDTH;
            param.disp_img_size_h = DEST_HEIGHT;
            
            param.disp_img_roi_x = 0;
            param.disp_img_roi_y = 0;
            param.disp_img_roi_w = DEST_WIDTH;
            param.disp_img_roi_h = DEST_HEIGHT/2;
            
            param.disp_yuv_img_addr = disp_out_start + (DEST_WIDTH/2)*2;
            param.disp_rotate_angle = 1;

        

        ret = Mdp_BitBltEx(&param); 
        
        sprintf( str, "/data/BitBltEx-hdmi-%dx%d.bin", DEST_WIDTH, DEST_HEIGHT );
        SaveImage( str, hdmi_out_start, DEST_WIDTH*DEST_HEIGHT*3);

        sprintf( str, "/data/BitBltEx-disp-%dx%d.bin", DEST_WIDTH, DEST_HEIGHT );
        SaveImage( str, disp_out_start, DEST_WIDTH*DEST_HEIGHT*3);
        
    }

    
    printf("MDP Test Case : TestFunc_Mt6575_mHalMdp_BitBltEx : Result = %d\n", ret );

    


    return 0;


    
}


int TestFunc_mHalIoCtrl_MHAL_IOCTL_BITBLT( void )
{
    int  ret = 0;

    MDP_SHOWFUNCTION();

#if defined(MTK_M4U_SUPPORT)
    unsigned long src_addr = g_src_buffer.vir_addr;
    unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
    unsigned long src_addr = g_src_buffer.phy_addr;
    unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif
 

    /*-----------------------------------------------------------------------------
        Invoke
      -----------------------------------------------------------------------------*/
    {
        mHalBltParam_t bltParam;
        
        bltParam.srcX = 0;
        bltParam.srcY = 0;
        bltParam.srcW = CROP_WIDTH;
        bltParam.srcWStride = SRC_WIDTH;
        bltParam.srcH = CROP_HEIGHT;
        bltParam.srcHStride = SRC_HEIGHT;
        bltParam.srcAddr = src_addr;
        bltParam.srcFormat = MHAL_SRC_COLOR_FMT;
        
        bltParam.dstW = DEST_WIDTH;
        bltParam.dstH = DEST_HEIGHT;
        bltParam.dstAddr = dst_addr;

        //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
        bltParam.dstFormat = MHAL_FORMAT_RGB_888;

        bltParam.pitch = DEST_WIDTH;

        bltParam.orientation = 0; //TODO: test 5
    
 
        if( MHAL_NO_ERROR != mHalIoCtrl( MHAL_IOCTL_BITBLT, &bltParam, sizeof(mHalBltParam_t), NULL, 0, NULL) ) 
        {
            printf("bitblt() can't do bitblt operation\n");
            ret = -1;
        }

    }

    return ret;
    
}




int  TestFunc_Mt6575_CameraPreview( void )
{
  
#if defined(MTK_M4U_SUPPORT)
      unsigned long src_addr = g_src_buffer.vir_addr;
      unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
      unsigned long src_addr = g_src_buffer.phy_addr;
      unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif

    MDP_SHOWFUNCTION();
    /*-----------------------------------------------------------------------------
        Invoke
      -----------------------------------------------------------------------------*/
{
        halIDPParam_t mahlparam;

        /*Because no camera, use rdma0 to take place of it!!*/
        /*These extra three parameters is designed for pueudo source test only*/
        mahlparam.test_pseudo_src_enable = 1;
        mahlparam.test_pseudo_src_color_format = SRC_COLOR_FMT;
        mahlparam.test_pseudo_src_yuv_img_addr = src_addr;


                
        mahlparam.src_size_w = SRC_WIDTH; //Input: sensor output after crop & filter
        mahlparam.src_size_h = SRC_HEIGHT;
            
        mahlparam.working_buffer_addr = dst_addr; //Same with input size

        #if 1        
        mahlparam.en_dst_port0 = 1;
        mahlparam.dst_port0.color_format = YV12_Planar;//MHAL_FORMAT_YUV_420;  /*For Display : NV21*/
        mahlparam.dst_port0.size_w = SRC_WIDTH;
        mahlparam.dst_port0.size_h = SRC_HEIGHT;
        mahlparam.dst_port0.buffer_addr = dst_addr + 1*(SRC_WIDTH*SRC_HEIGHT*2);
        mahlparam.dst_port0.buffer_count = 1;
        mahlparam.dst_port0.rotate = 0;
        mahlparam.dst_port0.flip = 0;
        mahlparam.dst_port0.up_coef = 0;
        mahlparam.dst_port0.down_coef = 0;
        mahlparam.dst_port0.zoom_ratio = 0;

        mahlparam.en_dst_port0 = 1;
        mahlparam.dst_port1.color_format = YV12_Planar;//MHAL_FORMAT_YUV_420;  /*For Encode : YV12*/
        mahlparam.dst_port1.size_w = SRC_WIDTH;
        mahlparam.dst_port1.size_h = SRC_HEIGHT;
        mahlparam.dst_port1.buffer_addr = dst_addr + 2*(SRC_WIDTH*SRC_HEIGHT*2);
        mahlparam.dst_port1.buffer_count = 1;
        mahlparam.dst_port1.rotate = 0;
        mahlparam.dst_port1.flip = 0;
        mahlparam.dst_port1.up_coef = 0;
        mahlparam.dst_port1.down_coef = 0;
        mahlparam.dst_port1.zoom_ratio = 0;

        mahlparam.en_dst_port0 = 1;
        mahlparam.dst_port2.color_format = RGB888;//MHAL_FORMAT_RGB_888;  /*For FD : RGB*/
        mahlparam.dst_port2.size_w = SRC_WIDTH;
        mahlparam.dst_port2.size_h = SRC_HEIGHT;
        mahlparam.dst_port2.buffer_addr = dst_addr + 3*(SRC_WIDTH*SRC_HEIGHT*2);
        mahlparam.dst_port2.buffer_count = 1;
        mahlparam.dst_port2.rotate = 0;
        mahlparam.dst_port2.flip = 0;
        mahlparam.dst_port2.up_coef = 0;
        mahlparam.dst_port2.down_coef = 0;
        mahlparam.dst_port2.zoom_ratio = 0;

        #endif


        Mt6575_mHalCameraPreview( &mahlparam );

    }



    return 0;


    
    }



int TestFunc_MdpHal_Preview( void )
{
    #if defined(MTK_M4U_SUPPORT)
    unsigned long src_addr = g_src_buffer.vir_addr;
    unsigned long dst_addr = g_dst_buffer.vir_addr;
    #else
    unsigned long src_addr = g_src_buffer.phy_addr;
    unsigned long dst_addr = g_dst_buffer.phy_addr;
    #endif

    char filename[256];

    MDP_SHOWFUNCTION();


    /*-----------------------------------------------------------------------------
        Invoke
      -----------------------------------------------------------------------------*/
    {
        
        MdpHalImp       mdp_mhal_imp;
        halIDPParam_t   mahlparam;

        /*C++ version use mode to distinguish between Preview and Capture mode*/
        mahlparam.mode = MDP_MODE_PRV_YUV;

        /*Because no camera, use rdma0 to take place of it!!*/
        /*These extra three parameters is designed for pueudo source test only*/
        mahlparam.test_pseudo_src_enable = 1; 
            mahlparam.test_pseudo_src_color_format = SRC_COLOR_FMT;
            mahlparam.test_pseudo_src_yuv_img_addr = src_addr;

        mahlparam.debug_preview_single_frame_enable = 1;

                
        mahlparam.src_size_w = SRC_WIDTH; //Input: sensor output after crop & filter
        mahlparam.src_size_h = SRC_HEIGHT;

        mahlparam.en_zero_shutter_path = 1; //0:Normal 1:Zero Shutter
            
        mahlparam.working_buffer_addr = dst_addr; //Same with input size

    #if 1
        mahlparam.en_dst_port0 = 1;
        mahlparam.dst_port0.color_format = NV21;//YV12_Planar;  /*For Display : NV21*/
        mahlparam.dst_port0.size_w = 640;//SRC_WIDTH;
        mahlparam.dst_port0.size_h = 480;//SRC_HEIGHT;
        mahlparam.dst_port0.buffer_addr = dst_addr + 1*(SRC_WIDTH*SRC_HEIGHT*2);
        mahlparam.dst_port0.buffer_count = 1;
        mahlparam.dst_port0.rotate = 0;
        mahlparam.dst_port0.flip = 0;
        mahlparam.dst_port0.up_coef = 0;
        mahlparam.dst_port0.down_coef = 0;
        mahlparam.dst_port0.zoom_ratio = 0;

        mahlparam.en_dst_port1 = 0;
        mahlparam.dst_port1.color_format = YV12_Planar;  /*For Encode : YV12*/
        mahlparam.dst_port1.size_w = SRC_WIDTH;
        mahlparam.dst_port1.size_h = SRC_HEIGHT;
        mahlparam.dst_port1.buffer_addr = dst_addr + 2*(SRC_WIDTH*SRC_HEIGHT*2);
        mahlparam.dst_port1.buffer_count = 1;
        mahlparam.dst_port1.rotate = 2;
        mahlparam.dst_port1.flip = 0;
        mahlparam.dst_port1.up_coef = 0;
        mahlparam.dst_port1.down_coef = 0;
        mahlparam.dst_port1.zoom_ratio = 0;

        mahlparam.en_dst_port2 = 0;
        mahlparam.dst_port2.color_format = RGB888;  /*For FD : RGB*/
        mahlparam.dst_port2.size_w = SRC_WIDTH;
        mahlparam.dst_port2.size_h = SRC_HEIGHT;
        mahlparam.dst_port2.buffer_addr = dst_addr + 3*(SRC_WIDTH*SRC_HEIGHT*2);
        mahlparam.dst_port2.buffer_count = 1;
        mahlparam.dst_port2.rotate = 1;
        mahlparam.dst_port2.flip = 0;
        mahlparam.dst_port2.up_coef = 0;
        mahlparam.dst_port2.down_coef = 0;
        mahlparam.dst_port2.zoom_ratio = 0;

    #endif
    
            

        mdp_mhal_imp.setConf( &mahlparam );
        mdp_mhal_imp.start();
        mdp_mhal_imp.waitDone(NULL);
        //mdp_mhal_imp.dumpReg();
        mdp_mhal_imp.stop();


        #if 0
        sprintf( filename,"/data/port0_%lux%lu.raw", mahlparam.dst_port0.size_w,mahlparam.dst_port0.size_h);
        SaveImage( filename, mahlparam.dst_port0.buffer_addr, mahlparam.dst_port0.size_w*mahlparam.dst_port0.size_h*3);
        #else
        sprintf( filename,"/data/port0_%lux%lu.raw", 640,480);
        SaveImage( filename, mahlparam.dst_port0.buffer_addr, 640*480*3);
        #endif

        
        sprintf( filename,"/data/port1_%lux%lu.raw", mahlparam.dst_port1.size_w,mahlparam.dst_port1.size_h);
        SaveImage( filename, mahlparam.dst_port1.buffer_addr, mahlparam.dst_port1.size_w*mahlparam.dst_port1.size_h*3);

        
        sprintf( filename,"/data/port2_%lux%lu.raw", mahlparam.dst_port2.size_w,mahlparam.dst_port2.size_h);
        SaveImage( filename, mahlparam.dst_port2.buffer_addr, mahlparam.dst_port2.size_w*mahlparam.dst_port2.size_h*3);

    }

    return 1;



}

int TestFunc_MdpHal_Capture( void )
{
#if defined(MTK_M4U_SUPPORT)
        unsigned long src_addr = g_src_buffer.vir_addr;
        unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
        unsigned long src_addr = g_src_buffer.phy_addr;
        unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif
    unsigned long dst_size = g_dst_buffer.size;

    MDP_SHOWFUNCTION();
    

    /*-----------------------------------------------------------------------------
        Invoke
      -----------------------------------------------------------------------------*/
    {
        
        MdpHalImp       mdp_mhal_imp;
        halIDPParam_t   mahlparam;
        int             jpg_size = 0;
        
        /*C++ version use mode to distinguish between Preview and Capture mode*/
        mahlparam.mode = MDP_MODE_CAP_JPG;

        /*-----------------------------------------------------------------------------
            Config Parameter
          -----------------------------------------------------------------------------*/
        mahlparam.Capture.camera_out_mode = 0;
        mahlparam.Capture.camera_count = 0;
        
        mahlparam.Capture.pseudo_src_enable = 1;         /*0:input from memory (rdma0) 1:input from sensor*/
            if( mahlparam.Capture.pseudo_src_enable == 1 )
            {
                mahlparam.Capture.pseudo_src_yuv_img_addr.y = src_addr;          //Omit if pseudo_src_enable = 1
                mahlparam.Capture.pseudo_src_color_format = SRC_COLOR_FMT;     //Omit if pseudo_src_enable = 1
            }

        mahlparam.Capture.src_img_size.w = SRC_WIDTH;
        mahlparam.Capture.src_img_size.h = SRC_HEIGHT;
        
        mahlparam.Capture.src_img_roi.x = 0;
        mahlparam.Capture.src_img_roi.y = 0;
        mahlparam.Capture.src_img_roi.w = CROP_WIDTH;
        mahlparam.Capture.src_img_roi.h = CROP_HEIGHT;
        

        mahlparam.Capture.jpg_img_size.w = DEST_WIDTH;       
        mahlparam.Capture.jpg_img_size.h = DEST_HEIGHT; 
        
        mahlparam.Capture.jpg_yuv_color_format = 422;   //integer : 411, 422, 444, 400, 410 etc...
        mahlparam.Capture.jpg_buffer_addr = dst_addr;
        mahlparam.Capture.jpg_buffer_size = dst_size;
        mahlparam.Capture.jpg_quality = 50;    //39~90
        mahlparam.Capture.jpg_b_add_soi = 1;  //1:Add EXIF 0:none
        
        mahlparam.Capture.b_qv_path_en = 1;   /*1:enable quick view path*/
            if( mahlparam.Capture.b_qv_path_en == 1 )
            {
            mahlparam.Capture.qv_path_sel = 0;//0;    /*0:auto select quick view path 1:QV_path_1 2:QV_path_2*/

            mahlparam.Capture.qv_yuv_img_addr.y = dst_addr + DEST_WIDTH*DEST_HEIGHT*2;
            
            mahlparam.Capture.qv_img_size.w = DEST_WIDTH;        //Omit if b_qv_path_en = 0
            mahlparam.Capture.qv_img_size.h = DEST_HEIGHT;

            mahlparam.Capture.qv_color_format = RGB888;    //Omit if b_qv_path_en = 0

            mahlparam.Capture.qv_flip = 0;
            mahlparam.Capture.qv_rotate = 0;
            }
            
         

         /*-----------------------------------------------------------------------------
             Execute MDP Mhal Capture
           -----------------------------------------------------------------------------*/
        mdp_mhal_imp.setConf( &mahlparam );
        mdp_mhal_imp.start();
        mdp_mhal_imp.waitDone(NULL);
        mdp_mhal_imp.sendCommand(CMD_GET_JPEG_FILE_SIZE, (int)(&jpg_size), 0, 0);
        mdp_mhal_imp.dumpReg();
        mdp_mhal_imp.stop();

        MDP_INFO("JPEG File size = %d\n\r", jpg_size );

    }

    return 1;



}



void TestFunc_Stnr( void )
{
#if defined(MTK_M4U_SUPPORT)
        unsigned long src_addr = g_src_buffer.vir_addr;
        unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
        unsigned long src_addr = g_src_buffer.phy_addr;
        unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif
    

    MDP_SHOWFUNCTION();

    /*-----------------------------------------------------------------------------
        Operation
      -----------------------------------------------------------------------------*/
    {
        struct MdpPathStnrParameter param;
        MdpPathStnr mdppath_stnr;


        param.b_crz_use_line_buffer  = 0;
        param.b_prz0_use_line_buffer = 0;
        param.b_prz1_use_line_buffer = 0;

        mdppath_stnr.Config(&param);    //Config
        mdppath_stnr.Start(NULL);       //Execute

                /*Do what you want to do between start() & end()*/
                mdppath_stnr.DumpRegister(NULL);

        mdppath_stnr.End(NULL); //Release resource
                
    }


    
    
}


void TestFunc_MdpPathJpgEncScale( void )
{
#if defined(MTK_M4U_SUPPORT)
        unsigned long src_addr = g_src_buffer.vir_addr;
        unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
        unsigned long src_addr = g_src_buffer.phy_addr;
        unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif

    MDP_SHOWFUNCTION();

    {

        struct MdpPathJpgEncodeScaleParameter   param;
        MdpPathJpgEncodeScale                   mdp_path_jpg_encode_scale;

        unsigned int jpg_encode_size;
        char filename[512];


        /*-----------------------------------------------------------------------------
            Config Parameter
          -----------------------------------------------------------------------------*/
        param.b_sensor_input = 0;         /*0:input from memory (rdma0) 1:input from sensor*/
            if( param.b_sensor_input == 0 )
            {
            param.src_yuv_img_addr.y = src_addr;            //Omit if b_sensor_input = 1
            param.src_color_format = SRC_COLOR_FMT;     //Omit if b_sensor_input = 1
            }

        param.src_img_size.w = SRC_WIDTH;
        param.src_img_size.h = SRC_HEIGHT;
        
        param.src_img_roi.x = 0;
        param.src_img_roi.y = 0;
        param.src_img_roi.w = CROP_WIDTH;
        param.src_img_roi.h = CROP_HEIGHT;

        param.b_jpg_path_disen = 1;
            if( param.b_jpg_path_disen == 0 )
            {
            param.jpg_img_size.w = DEST_WIDTH;       
            param.jpg_img_size.h = DEST_HEIGHT; 
            
            param.jpg_yuv_color_format = 422;   //integer : 411, 422, 444, 400, 410 etc...
            param.jpg_buffer_addr = dst_addr;
            param.jpg_buffer_size = DEST_WIDTH*DEST_HEIGHT*2;
            param.jpg_quality = 50;    //39~90
            param.jpg_b_add_soi = 1;  //1:Add EXIF 0:none
            }
        
        param.b_qv_path_en = 1;   /*1:enable quick view path*/
            if( param.b_qv_path_en == 1 )
            {
            param.qv_path_sel = 2;//0;    /*0:auto select quick view path 1:QV_path_1 2:QV_path_2*/

            param.qv_yuv_img_addr.y = dst_addr + DEST_WIDTH*DEST_HEIGHT*2;

            param.qv_img_roi = MdpRect( 0, 0, DEST_WIDTH, DEST_HEIGHT );
                
            param.qv_img_size.w = DEST_WIDTH;        //Omit if b_qv_path_en = 0
            param.qv_img_size.h = DEST_HEIGHT;

            param.qv_color_format = RGB888;    //Omit if b_qv_path_en = 0

            param.qv_flip = 0;
            param.qv_rotate = 0;
            }

        
        param.b_ff_path_en = 0;   
            if( param.b_ff_path_en == 1 )
            {
            param.ff_yuv_img_addr.y = dst_addr + 2*DEST_WIDTH*DEST_HEIGHT*2;

            
            param.ff_img_roi = MdpRect( 0, 0, DEST_WIDTH, DEST_HEIGHT );        //Omit if b_qv_path_en = 0
            
            param.ff_img_size.w = DEST_WIDTH;        //Omit if b_qv_path_en = 0
            param.ff_img_size.h = DEST_HEIGHT;

            param.ff_color_format = UYVY_Pack;    //Omit if b_qv_path_en = 0

            param.ff_flip = 0;
            param.ff_rotate = 0;
            }
            
        
        /*-----------------------------------------------------------------------------
            Execute
          -----------------------------------------------------------------------------*/
        mdp_path_jpg_encode_scale.Config( &param );
        mdp_path_jpg_encode_scale.Start( NULL );
        mdp_path_jpg_encode_scale.WaitBusy( NULL );
        mdp_path_jpg_encode_scale.End( NULL );

        

        /*-----------------------------------------------------------------------------
            Verify
          -----------------------------------------------------------------------------*/
        jpg_encode_size = mdp_path_jpg_encode_scale.jpg_encode_size();

        if( param.b_jpg_path_disen == 0 )
        {
            sprintf( filename,"/data/port_jpg_%dx%d.jpg", (int)DEST_WIDTH, (int)DEST_HEIGHT);
            SaveImage( filename, param.jpg_buffer_addr, jpg_encode_size);
        }

        
        if( param.b_qv_path_en == 1 )
        {
            sprintf( filename,"/data/port_qv_%lux%lu.raw", DEST_WIDTH,DEST_HEIGHT);
            SaveImage( filename, param.qv_yuv_img_addr.y, DEST_WIDTH*DEST_HEIGHT*3);
        }

        if( param.b_ff_path_en == 1 )
        {
            sprintf( filename,"/data/port_ff_%lux%lu.raw", DEST_WIDTH,DEST_HEIGHT);
            SaveImage( filename, param.ff_yuv_img_addr.y, DEST_WIDTH*DEST_HEIGHT*2);
        }
               
        

    }
    
}

void TestFunc_MdpPathJpgEncScale2( void )
{
#if defined(MTK_M4U_SUPPORT)
        unsigned long src_addr = g_src_buffer.vir_addr;
        unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
        unsigned long src_addr = g_src_buffer.phy_addr;
        unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif

    MDP_SHOWFUNCTION();

    {

        struct MdpPathJpgEncodeScaleParameter   param;
        MdpPathJpgEncodeScale                   mdp_path_jpg_encode_scale;

        unsigned int jpg_encode_size;
        char filename[512];


        /*-----------------------------------------------------------------------------
            Config Parameter
          -----------------------------------------------------------------------------*/
        param.b_sensor_input = 0;         /*0:input from memory (rdma0) 1:input from sensor*/
            if( param.b_sensor_input == 0 )
            {
            param.src_yuv_img_addr.y = src_addr;            //Omit if b_sensor_input = 1
            param.src_color_format = SRC_COLOR_FMT;     //Omit if b_sensor_input = 1
            }

        param.src_img_size.w = SRC_WIDTH;
        param.src_img_size.h = SRC_HEIGHT;
        
        param.src_img_roi.x = 0;
        param.src_img_roi.y = 0;
        param.src_img_roi.w = CROP_WIDTH;
        param.src_img_roi.h = CROP_HEIGHT;

        param.b_jpg_path_disen = 1;
            if( param.b_jpg_path_disen == 0 )
            {
            param.jpg_img_size.w = DEST_WIDTH;       
            param.jpg_img_size.h = DEST_HEIGHT; 
            
            param.jpg_yuv_color_format = 422;   //integer : 411, 422, 444, 400, 410 etc...
            param.jpg_buffer_addr = dst_addr;
            param.jpg_buffer_size = DEST_WIDTH*DEST_HEIGHT*2;
            param.jpg_quality = 50;    //39~90
            param.jpg_b_add_soi = 1;  //1:Add EXIF 0:none
            }
        
        param.b_qv_path_en = 1;   /*1:enable quick view path*/
            if( param.b_qv_path_en == 1 )
            {
            param.qv_path_sel = 2;//0;    /*0:auto select quick view path 1:QV_path_1 2:QV_path_2*/

            param.qv_yuv_img_addr.y = dst_addr + DEST_WIDTH*DEST_HEIGHT*2;

            param.qv_img_roi = MdpRect( 0, 0, DEST_WIDTH/2, DEST_HEIGHT );
            //param.qv_img_roi = MdpRect( 0, 0, DEST_WIDTH, DEST_HEIGHT );
                
            param.qv_img_size.w = DEST_WIDTH;        //Omit if b_qv_path_en = 0
            param.qv_img_size.h = DEST_HEIGHT;

            //param.qv_color_format = RGB888;    //Omit if b_qv_path_en = 0
            param.qv_color_format = UYVY_Pack;    //Omit if b_qv_path_en = 0

            param.qv_flip = 0;
            param.qv_rotate = 0;
            }

        
        param.b_ff_path_en = 0;   
            if( param.b_ff_path_en == 1 )
            {
            param.ff_yuv_img_addr.y = dst_addr + 2*DEST_WIDTH*DEST_HEIGHT*2;

            
            param.ff_img_roi = MdpRect( 0, 0, DEST_WIDTH, DEST_HEIGHT );        //Omit if b_qv_path_en = 0
            
            param.ff_img_size.w = DEST_WIDTH;        //Omit if b_qv_path_en = 0
            param.ff_img_size.h = DEST_HEIGHT;

            param.ff_color_format = UYVY_Pack;    //Omit if b_qv_path_en = 0

            param.ff_flip = 0;
            param.ff_rotate = 0;
            }
            
        
        /*-----------------------------------------------------------------------------
            Execute
          -----------------------------------------------------------------------------*/
        mdp_path_jpg_encode_scale.Config( &param );
        mdp_path_jpg_encode_scale.Start( NULL );
        mdp_path_jpg_encode_scale.WaitBusy( NULL );
        mdp_path_jpg_encode_scale.End( NULL );

        #if 1
        /*-----------------------------------------------------------------------------
            2nd Execute side-by-side test
          -----------------------------------------------------------------------------*/
                param.qv_yuv_img_addr.y = dst_addr + DEST_WIDTH*DEST_HEIGHT*2 + 2*(DEST_WIDTH/2);
            
        
        mdp_path_jpg_encode_scale.Config( &param );
        mdp_path_jpg_encode_scale.Start( NULL );
        mdp_path_jpg_encode_scale.WaitBusy( NULL );
        mdp_path_jpg_encode_scale.End( NULL );

        #endif

        

        /*-----------------------------------------------------------------------------
            Verify
          -----------------------------------------------------------------------------*/
        jpg_encode_size = mdp_path_jpg_encode_scale.jpg_encode_size();

        if( param.b_jpg_path_disen == 0 )
        {
            sprintf( filename,"/data/port_jpg_%dx%d.jpg", (int)DEST_WIDTH, (int)DEST_HEIGHT);
            SaveImage( filename, param.jpg_buffer_addr, jpg_encode_size);
        }

        
        if( param.b_qv_path_en == 1 )
        {
            sprintf( filename,"/data/port_qv_%lux%lu.raw", DEST_WIDTH,DEST_HEIGHT);
            //SaveImage( filename, param.qv_yuv_img_addr.y, DEST_WIDTH*DEST_HEIGHT*3);
            SaveImage( filename, dst_addr + DEST_WIDTH*DEST_HEIGHT*2, DEST_WIDTH*DEST_HEIGHT*3);
        }

        if( param.b_ff_path_en == 1 )
        {
            sprintf( filename,"/data/port_ff_%lux%lu.raw", DEST_WIDTH,DEST_HEIGHT);
            SaveImage( filename, param.ff_yuv_img_addr.y, DEST_WIDTH*DEST_HEIGHT*2);
        }
               
        

    }
    
}



void TestFunc_MdpPathN3d2ndPass( void )
{
#if defined(MTK_M4U_SUPPORT)
        unsigned long src_addr = g_src_buffer.vir_addr;
        unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
        unsigned long src_addr = g_src_buffer.phy_addr;
        unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif

    MDP_SHOWFUNCTION();

    {

        struct MdpPathN3d2ndPassParameter   param1;
        struct MdpPathN3d2ndPassParameter   param2;
        MdpPathN3d2ndPass                   mdp_path;

        
        /*-----------------------------------------------------------------------------
            Parameter
          -----------------------------------------------------------------------------*/
        param1.src_buffer_count = 1;
        param1.src_color_format = SRC_COLOR_FMT;
        param1.src_img_size = MdpSize( SRC_WIDTH ,SRC_HEIGHT );
        param1.src_img_roi = MdpRect( 0, 0, SRC_WIDTH ,SRC_HEIGHT );
        param1.src_yuv_img_addr.y = src_addr;


        /*Output port*/
        param1.dst_buffer_count = 1;
        param1.dst_yuv_img_addr.y = dst_addr;    
        param1.dst_img_size = MdpSize( 2*SRC_WIDTH ,SRC_HEIGHT );        //image stride
        param1.dst_img_roi = MdpRect( 0, 0, SRC_WIDTH ,SRC_HEIGHT );     //if roi=0, default value is (0,0,size.w, size.h)
        param1.dst_color_format = YUV420_Planar;    
        param1.dst_flip = 0;       
        param1.dst_rotate = 0; 

        /*resizer coeff*/
        //param1.resz_coeff;

        /*param2*/
        param2 = param1;
        param2.dst_img_roi = MdpRect( SRC_WIDTH, 0, SRC_WIDTH ,SRC_HEIGHT ); 
            

       
        
        /*-----------------------------------------------------------------------------
            Execute
          -----------------------------------------------------------------------------*/
        mdp_path.Config( &param1 );
        mdp_path.Start( NULL );
        mdp_path.WaitBusy( NULL );
        mdp_path.End( NULL );

        
        mdp_path.Config( &param2 );
        mdp_path.Start( NULL );
        mdp_path.WaitBusy( NULL );
        mdp_path.End( NULL );

        

        /*-----------------------------------------------------------------------------
            Verify
          -----------------------------------------------------------------------------*/
        {
            char    filename[512]; 

            sprintf( filename,"/data/side_by_side_%lux%lu.raw", 2*SRC_WIDTH,SRC_HEIGHT);
            SaveImage( filename, param1.dst_yuv_img_addr.y,  2*SRC_WIDTH*SRC_HEIGHT*4 );
        }
               
        

    }
    
}



void TestFunc_MdpPathDisplayFromMemoryHdmi( void )
{
#if defined(MTK_M4U_SUPPORT)
        unsigned long src_addr = g_src_buffer.vir_addr;
        unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
        unsigned long src_addr = g_src_buffer.phy_addr;
        unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif

    MDP_SHOWFUNCTION();

    {

        struct MdpPathDisplayFromMemoryHdmiParameter   param;
        MdpPathDisplayFromMemoryHdmi                   mdp_path;

        
        /*-----------------------------------------------------------------------------
            Parameter
          -----------------------------------------------------------------------------*/
        /*Source*/
        param.src_color_format   = SRC_COLOR_FMT;
        param.src_img_size = MdpSize( SRC_WIDTH ,SRC_HEIGHT );
        param.src_img_roi  = MdpRect( 0, 0, SRC_WIDTH ,SRC_HEIGHT );
        param.src_yuv_img_addr.y = src_addr;
        
        
        /*For HDMI : RGB*/
        param.en_hdmi_port = 1;           //enable hdmi port
            param.hdmi_color_format = YUV422_Pack;
            param.hdmi_img_size = MdpSize( DEST_WIDTH ,DEST_HEIGHT );      //Stride       
            param.hdmi_img_roi  = MdpRect( 0, 0, DEST_WIDTH ,DEST_HEIGHT ); //image size to be scaled
            param.hdmi_yuv_img_addr.y = dst_addr;
            param.hdmi_rotate_angle = 0;
        
        /*For Display : 422 Pack*/
        param.en_disp_port = 1;           //enable disp port
            param.disp_color_format = YUV422_Pack;
            param.disp_img_size = MdpSize( DEST_WIDTH ,DEST_HEIGHT );       /*Currently, "dst roi"  should be equal to "dst size"*/
            param.disp_img_roi  = MdpRect( 0, 0, DEST_WIDTH ,DEST_HEIGHT );       /*dst_img_roi is not used*/
            param.disp_yuv_img_addr.y = dst_addr + DEST_WIDTH*DEST_HEIGHT*4;
            param.disp_rotate_angle = 0;
        
        /*resizer coeff*/
        //param.resz_coeff;
            
        
        /*-----------------------------------------------------------------------------
            Execute
          -----------------------------------------------------------------------------*/
        mdp_path.Config( &param );
        mdp_path.Start( NULL );
        mdp_path.WaitBusy( NULL );
        mdp_path.End( NULL );


        /*-----------------------------------------------------------------------------
            Verify
          -----------------------------------------------------------------------------*/
        {
            char    filename[512]; 

            sprintf( filename,"/data/hdmi_%lux%lu.raw", DEST_WIDTH,DEST_HEIGHT);
            SaveImage( filename, param.hdmi_yuv_img_addr.y,  DEST_WIDTH*DEST_HEIGHT*4 );

            sprintf( filename,"/data/disp_%lux%lu.raw", DEST_WIDTH,DEST_HEIGHT);
            SaveImage( filename, param.disp_yuv_img_addr.y,  DEST_WIDTH*DEST_HEIGHT*4 );
        }
               
        

    }
    
}

void TestFunc_MdpPipeCameraCapture( void )
{
#if defined(MTK_M4U_SUPPORT)
        unsigned long src_addr = g_src_buffer.vir_addr;
        unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
        unsigned long src_addr = g_src_buffer.phy_addr;
        unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif
    unsigned long dst_size = g_dst_buffer.size;


    MDP_SHOWFUNCTION();

    {

        struct MdpPipeCameraCaptureParameter   param;
        MdpPipeCameraCapture                   mdp_pipe_camera_capture;


        /*-----------------------------------------------------------------------------
            Config Parameter
          -----------------------------------------------------------------------------*/
        param.camera_out_mode = 0;
        param.camera_count = 0;
        
        param.pseudo_src_enable = 1;         /*0:input from memory (rdma0) 1:input from sensor*/
        if( param.pseudo_src_enable == 1 )
        {
            param.pseudo_src_yuv_img_addr.y = src_addr;          //Omit if pseudo_src_enable = 1
            param.pseudo_src_color_format = SRC_COLOR_FMT;     //Omit if pseudo_src_enable = 1
        }

        param.src_img_size.w = SRC_WIDTH;
        param.src_img_size.h = SRC_HEIGHT;
        
        param.src_img_roi.x = 0;
        param.src_img_roi.y = 0;
        param.src_img_roi.w = CROP_WIDTH;
        param.src_img_roi.h = CROP_HEIGHT;
        

        param.jpg_img_size.w = DEST_WIDTH;       
        param.jpg_img_size.h = DEST_HEIGHT; 
        
        param.jpg_yuv_color_format = 422;   //integer : 411, 422, 444, 400, 410 etc...
        param.jpg_buffer_addr = dst_addr;
        param.jpg_buffer_size = dst_size;
        param.jpg_quality = 50;    //39~90
        param.jpg_b_add_soi = 1;  //1:Add EXIF 0:none
        
        param.b_qv_path_en = 1;   /*1:enable quick view path*/
            if( param.b_qv_path_en == 1 )
            {
            param.qv_path_sel = 0;//0;    /*0:auto select quick view path 1:QV_path_1 2:QV_path_2*/

            param.qv_yuv_img_addr.y = dst_addr + DEST_WIDTH*DEST_HEIGHT*2;
            
            param.qv_img_size.w = DEST_WIDTH;        //Omit if b_qv_path_en = 0
            param.qv_img_size.h = DEST_HEIGHT;

            param.qv_color_format = RGB888;    //Omit if b_qv_path_en = 0

            param.qv_flip = 0;
            param.qv_rotate = 0;
            }
            
        
        /*-----------------------------------------------------------------------------
            Execute
          -----------------------------------------------------------------------------*/
        mdp_pipe_camera_capture.Config( &param );
        mdp_pipe_camera_capture.Start( NULL );
        mdp_pipe_camera_capture.WaitBusy( NULL );
        mdp_pipe_camera_capture.End( NULL );

    }
    
}



void TestFunc_MdpPathCamera2PreviewToMemory( void )
{
#if defined(MTK_M4U_SUPPORT)
        unsigned long src_addr = g_src_buffer.vir_addr;
        unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
        unsigned long src_addr = g_src_buffer.phy_addr;
        unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif

    MDP_SHOWFUNCTION();

    {

        struct MdpPathCamera2PreviewToMemoryParameter   param;
        MdpPathCamera2PreviewToMemory                   mdp_path_camera_2_preview_to_memory;


        /*-----------------------------------------------------------------------------
            Config Parameter
          -----------------------------------------------------------------------------*/
        param.pseudo_src_enable = 1;         
            if( param.pseudo_src_enable == 1 )
            {
            param.pseudo_src_yuv_img_addr.y = src_addr;           
            param.pseudo_src_color_format = SRC_COLOR_FMT;     
            }
            
        param.debug_preview_single_frame_enable = 1;
        param.b_hw_trigger_out = 0;

        param.src_img_size.w = SRC_WIDTH;
        param.src_img_size.h = SRC_HEIGHT;
        
        param.src_img_roi.x = 0;
        param.src_img_roi.y = 0;
        param.src_img_roi.w = CROP_WIDTH;
        param.src_img_roi.h = CROP_HEIGHT;

        


        param.dst_color_format = YUV420_Planar;

        param.dst_img_size.w = DEST_WIDTH;   
        param.dst_img_size.h = DEST_HEIGHT;
        
        param.dst_img_roi.x = 0;
        param.dst_img_roi.y = 0;
        param.dst_img_roi.w = param.dst_img_size.w;
        param.dst_img_roi.h = param.dst_img_size.h;

        param.dst_yuv_img_addr.y = dst_addr;
        param.dst_rotate_angle = 0;
           
            
        
        /*-----------------------------------------------------------------------------
            Execute
          -----------------------------------------------------------------------------*/
        mdp_path_camera_2_preview_to_memory.Config( &param );
        mdp_path_camera_2_preview_to_memory.Start( NULL );
        mdp_path_camera_2_preview_to_memory.WaitBusy( NULL );
        mdp_path_camera_2_preview_to_memory.End( NULL );

    }
    
}


void TestFunc_MdpPathCameraPreviewToMemory( void )
{
#if defined(MTK_M4U_SUPPORT)
        unsigned long src_addr = g_src_buffer.vir_addr;
        unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
        unsigned long src_addr = g_src_buffer.phy_addr;
        unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif

    MDP_SHOWFUNCTION();

    {

        struct MdpPathCameraPreviewToMemoryParameter   param;
        MdpPathCameraPreviewToMemory                   mdp_path_camera_preview_to_memory;


        /*-----------------------------------------------------------------------------
            Config Parameter
          -----------------------------------------------------------------------------*/
        param.pseudo_src_enable = 1;         
            if( param.pseudo_src_enable == 1 )
            {
            param.pseudo_src_buffer_count = BUFFER_COUNT;
            param.pseudo_src_yuv_img_addr.y = src_addr;           
            param.pseudo_src_color_format = SRC_COLOR_FMT;     
            
            }
            
        param.debug_preview_single_frame_enable = 1;
        param.b_hw_trigger_out = 0;

        param.src_img_size.w = SRC_WIDTH;
        param.src_img_size.h = SRC_HEIGHT;
        
        param.src_img_roi.x = 0;
        param.src_img_roi.y = 0;
        param.src_img_roi.w = CROP_WIDTH;
        param.src_img_roi.h = CROP_HEIGHT;

        

        param.dst_buffer_count = BUFFER_COUNT;
        param.dst_color_format = YUV420_Planar;//RGB565;

        param.dst_img_size.w = DEST_WIDTH;   
        param.dst_img_size.h = DEST_HEIGHT;
        
        param.dst_img_roi.x = 0;
        param.dst_img_roi.y = 0;
        param.dst_img_roi.w = param.dst_img_size.w;
        param.dst_img_roi.h = param.dst_img_size.h;

        param.dst_yuv_img_addr.y = dst_addr;
        param.dst_rotate_angle = 0;

          
            
        
        /*-----------------------------------------------------------------------------
            Execute
          -----------------------------------------------------------------------------*/
        mdp_path_camera_preview_to_memory.Config( &param );
        mdp_path_camera_preview_to_memory.Start( NULL );
        mdp_path_camera_preview_to_memory.WaitBusy( NULL );
        mdp_path_camera_preview_to_memory.End( NULL );


        {
            char str[512];
            sprintf( str, "/data/MdpPathCameraPreviewToMemory-%dx%d_%d.bin",DEST_WIDTH,DEST_HEIGHT,YUV420_Planar);
            SaveImage( str, dst_addr, DEST_WIDTH*DEST_HEIGHT*3);
        }

    }
    
}


void TestFunc_MdpPathCameraPreviewToMemory_ZOOM( void )
{
#if defined(MTK_M4U_SUPPORT)
        unsigned long src_addr = g_src_buffer.vir_addr;
        unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
        unsigned long src_addr = g_src_buffer.phy_addr;
        unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif
    int i,j;

    MDP_SHOWFUNCTION();

    {

        struct MdpPathCameraPreviewToMemoryParameter   param;
        MdpPathCameraPreviewToMemory                   mdp_path_camera_preview_to_memory;


        /*-----------------------------------------------------------------------------
            Config Parameter
          -----------------------------------------------------------------------------*/
        param.pseudo_src_enable = 1;         
            if( param.pseudo_src_enable == 1 )
            {
            param.pseudo_src_buffer_count = BUFFER_COUNT;
            param.pseudo_src_yuv_img_addr.y = g_src_buffer.vir_addr;//src_addr;           
            param.pseudo_src_color_format = SRC_COLOR_FMT;     
            
            }
            
        param.debug_preview_single_frame_enable = 1;
        param.b_hw_trigger_out = 0;

        param.src_img_size.w = SRC_WIDTH;
        param.src_img_size.h = SRC_HEIGHT;
        
        param.src_img_roi.x = 0;
        param.src_img_roi.y = 0;
        param.src_img_roi.w = CROP_WIDTH;
        param.src_img_roi.h = CROP_HEIGHT;

        

        param.dst_buffer_count = BUFFER_COUNT;
        param.dst_color_format = RGB565;

        param.dst_img_size.w = DEST_WIDTH;   
        param.dst_img_size.h = DEST_HEIGHT;
        
        param.dst_img_roi.x = 0;
        param.dst_img_roi.y = 0;
        param.dst_img_roi.w = param.dst_img_size.w;
        param.dst_img_roi.h = param.dst_img_size.h;

        param.dst_yuv_img_addr.y = g_dst_buffer.vir_addr;//dst_addr;
        param.dst_rotate_angle = 0;

           
            
        
        /*-----------------------------------------------------------------------------
            Execute
          -----------------------------------------------------------------------------*/
        mdp_path_camera_preview_to_memory.Config( &param );
        mdp_path_camera_preview_to_memory.Start( NULL );
        

        #if 0
        for( i = 0; i < BUFFER_COUNT*10; i++ )
        {
            mdp_path_camera_preview_to_memory.WaitBusy( NULL );

            mdp_path_camera_preview_to_memory.QueueRefill( MID_R_DMA0 | MID_RGB_ROT0 );
        }
        #else
        
        for( i = 0; i < 201; i++ )
        //for( i = 1; i < 0xFFFFFFFF ; i++ )
        {
            static int     bReverse = 0;
            MdpRect crop_size;
            unsigned long free_start, free_count;


            //----
            if( mdp_path_camera_preview_to_memory.QueueGetFreeListOrWaitBusy( MID_RGB_ROT0, &free_start, &free_count  ) < 0 )
            {
                printf("Error:ROT no free buffer!\n\r");
            }
            
            for( j = free_start; j < free_start+ free_count ; j++ )
            {
                mdp_path_camera_preview_to_memory.QueueRefill( MID_RGB_ROT0 );
            }
            
            //---
            if( mdp_path_camera_preview_to_memory.QueueGetFreeListOrWaitBusy( MID_R_DMA0, &free_start, &free_count  ) < 0 )
            {
                printf("Error:RDMA no free buffer!\n\r");
            }
            
            for( j = free_start; j < free_start+ free_count ; j++ )
            {
                mdp_path_camera_preview_to_memory.QueueRefill( MID_R_DMA0 );
            }

            #if 1
            //--
            if( ( i % 100 ) == 0 )
            {

                if( bReverse == 0 )
                {
                    bReverse = 1;

                    {
                        FILE *fp;

                        fp = fopen("/data/zoom-no.raw", "w");
                        fwrite((void*) param.dst_yuv_img_addr.y , 1 , DEST_WIDTH*DEST_HEIGHT*2 , fp);
                        fclose(fp);
                        printf("File saved\n");
                    }
                    
                    crop_size.x = SRC_WIDTH/4;
                    crop_size.y = SRC_HEIGHT/4;
                    crop_size.w = SRC_WIDTH/2;
                    crop_size.h = SRC_HEIGHT/2;

                    
                    
                } 
                else
                {
                    bReverse = 0;

                    {
                        FILE *fp;

                        fp = fopen("/data/zoom-yes.raw", "w");
                        fwrite( (void*)param.dst_yuv_img_addr.y , 1 , DEST_WIDTH*DEST_HEIGHT*2 , fp);
                        fclose(fp);
                        printf("File saved\n");
                    }
                    
                    crop_size.x = 0;
                    crop_size.y = 0;
                    crop_size.w = SRC_WIDTH;
                    crop_size.h = SRC_HEIGHT;
                }
                printf("Zoom Crop:%d , %d, %d, %d\n", crop_size.x, crop_size.y, crop_size.w, crop_size.h );

                
                mdp_path_camera_preview_to_memory.ConfigZoom( crop_size  );
            }
            #endif


            
            
        }
        #endif


        mdp_path_camera_preview_to_memory.End( NULL );

    }
    
}




void TestFunc_MdpPathCameraPreviewToMemory_Cust( void )
{
    unsigned long i,j;
#if defined(MTK_M4U_SUPPORT)
        unsigned long src_addr = g_src_buffer.vir_addr;
        unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
        unsigned long src_addr = g_src_buffer.phy_addr;
        unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif

    MDP_SHOWFUNCTION();

    {

        struct MdpPathCameraPreviewToMemoryParameter   param;
        MdpPathCameraPreviewToMemory                   mdp_path_camera_preview_to_memory;


        /*-----------------------------------------------------------------------------
            Config Parameter
          -----------------------------------------------------------------------------*/
        param.pseudo_src_enable = 1;         
            if( param.pseudo_src_enable == 1 )
            {
            param.pseudo_src_buffer_count = BUFFER_COUNT;
            param.pseudo_src_yuv_img_addr.y = g_src_buffer.vir_addr;//src_addr;           
            param.pseudo_src_color_format = SRC_COLOR_FMT;     
            
            }
            
        param.debug_preview_single_frame_enable = 1;
        param.b_hw_trigger_out = 0;

        param.src_img_size.w = SRC_WIDTH;
        param.src_img_size.h = SRC_HEIGHT;
        
        param.src_img_roi.x = 0;
        param.src_img_roi.y = 0;
        param.src_img_roi.w = CROP_WIDTH;
        param.src_img_roi.h = CROP_HEIGHT;

        

        param.dst_buffer_count = BUFFER_COUNT;
        param.dst_color_format = RGB565;

        param.dst_img_size.w = DEST_WIDTH;   
        param.dst_img_size.h = DEST_HEIGHT;
        
        param.dst_img_roi.x = 0;
        param.dst_img_roi.y = 0;
        param.dst_img_roi.w = param.dst_img_size.w;
        param.dst_img_roi.h = param.dst_img_size.h;

        param.dst_yuv_img_addr.y = g_dst_buffer.vir_addr;//dst_addr;
        param.dst_rotate_angle = 0;

           
            
        
        /*-----------------------------------------------------------------------------
            Execute
          -----------------------------------------------------------------------------*/
        mdp_path_camera_preview_to_memory.Config( &param );
        mdp_path_camera_preview_to_memory.Start( NULL );
        

        #if 0
        for( i = 0; i < BUFFER_COUNT*10; i++ )
        {
            mdp_path_camera_preview_to_memory.WaitBusy( NULL );

            mdp_path_camera_preview_to_memory.QueueRefill( MID_R_DMA0 | MID_RGB_ROT0 );
        }
        #else
        
        for( i = 0; i < 201; i++ )
        //for( i = 1; i < 0xFFFFFFFF ; i++ )
        {
            static int     bReverse = 0;
            MdpRect crop_size;
            unsigned long free_start, free_count;


            //----
            if( mdp_path_camera_preview_to_memory.QueueGetFreeListOrWaitBusy( MID_RGB_ROT0, &free_start, &free_count  ) < 0 )
            {
                printf("Error:ROT no free buffer!\n\r");
            }
            
            for( j = free_start; j < free_start+ free_count ; j++ )
            {
                mdp_path_camera_preview_to_memory.QueueRefill( MID_RGB_ROT0 );
            }
            
            //---
            if( mdp_path_camera_preview_to_memory.QueueGetFreeListOrWaitBusy( MID_R_DMA0, &free_start, &free_count  ) < 0 )
            {
                printf("Error:RDMA no free buffer!\n\r");
            }
            
            for( j = free_start; j < free_start+ free_count ; j++ )
            {
                mdp_path_camera_preview_to_memory.QueueRefill( MID_R_DMA0 );
            }


            
            

           
            
        }
        #endif
        //Test
                    mdp_path_camera_preview_to_memory.DumpRegister( NULL );


        mdp_path_camera_preview_to_memory.End( NULL );

    }
    
}

/*-----------------------------------------------------------------------------
    Special DVT test case for Ryan Wang
  -----------------------------------------------------------------------------*/
int TestFunc_MdpPathCameraPreviewToMemory_NoBypass_CB_BeforeStart( void *param )
{
    unsigned long reg_val;

    printf("\nI am CB before Start Call Back!\n");

       
    reg_val = MDPELEMENT_I::RegisterRead32_Mmsys1( 0x100 );

    printf("0x100 = 0x%08X ==>", (unsigned int)reg_val );

    reg_val &= ~( 0x1 << 16 );  //Clear to 0 as No Bypass

    MDPELEMENT_I::RegisterWrite32_Mmsys1( 0x100, reg_val);

    printf("0x%08X \n\n", (unsigned int)reg_val );
    

    return 0;
}

void TestFunc_MdpPathCameraPreviewToMemory_NoBypass( void )
{
    unsigned long i,j;
#if defined(MTK_M4U_SUPPORT)
        unsigned long src_addr = g_src_buffer.vir_addr;
        unsigned long dst_addr = g_dst_buffer.vir_addr;
#else
        unsigned long src_addr = g_src_buffer.phy_addr;
        unsigned long dst_addr = g_dst_buffer.phy_addr;
#endif

    MDP_SHOWFUNCTION();

    {

        struct MdpPathCameraPreviewToMemoryParameter   param;
        MdpPathCameraPreviewToMemory                   mdp_path_camera_preview_to_memory;


        /*-----------------------------------------------------------------------------
            Config Parameter
          -----------------------------------------------------------------------------*/
        param.pseudo_src_enable = 1;         
            if( param.pseudo_src_enable == 1 )
            {
            param.pseudo_src_buffer_count = BUFFER_COUNT;
            param.pseudo_src_yuv_img_addr.y = src_addr;           
            param.pseudo_src_color_format = SRC_COLOR_FMT;     
            
            }
            
        param.debug_preview_single_frame_enable = 1;
        param.b_hw_trigger_out = 0;

        param.src_img_size.w = SRC_WIDTH;
        param.src_img_size.h = SRC_HEIGHT;
        
        param.src_img_roi.x = 0;
        param.src_img_roi.y = 0;
        param.src_img_roi.w = CROP_WIDTH;
        param.src_img_roi.h = CROP_HEIGHT;

        

        param.dst_buffer_count = BUFFER_COUNT;
        param.dst_color_format = RGB565;

        param.dst_img_size.w = DEST_WIDTH;   
        param.dst_img_size.h = DEST_HEIGHT;
        
        param.dst_img_roi.x = 0;
        param.dst_img_roi.y = 0;
        param.dst_img_roi.w = param.dst_img_size.w;
        param.dst_img_roi.h = param.dst_img_size.h;

        param.dst_yuv_img_addr.y = dst_addr;
        param.dst_rotate_angle = 0;


        /*-----------------------------------------------------------------------------
            Extra Config
          -----------------------------------------------------------------------------*/
        mdp_path_camera_preview_to_memory.Config_CallBackFunction_BeforeStart( TestFunc_MdpPathCameraPreviewToMemory_NoBypass_CB_BeforeStart );
           
            
        
        /*-----------------------------------------------------------------------------
            Execute
          -----------------------------------------------------------------------------*/
        mdp_path_camera_preview_to_memory.Config( &param );
        mdp_path_camera_preview_to_memory.Start( &mdp_path_camera_preview_to_memory );
        mdp_path_camera_preview_to_memory.WaitBusy( NULL );
        mdp_path_camera_preview_to_memory.End( NULL );

    }
    
}

void TestFunc_FactoryMode( void )
{
    
    extern int mHalFactory(void *a_pInBuffer);

    printf("Invoke factory mode entry function\n");
    mHalFactory( NULL );
    printf("Finish factory mode function\n");
}


int main( int argc, char** argv )
{
    printf("Hi!I am MDP Unit Test App\n\r");

    
    /*-----------------------------------------------------------------------------
        Open device driver
      -----------------------------------------------------------------------------*/
    if( MdpDrvInit() == -1 )     //Open mdp device driver node
    {
        printf("Open mdp device driver failed\n\r!");
    } else
    {
        printf("Open mdp device driver Success!\n\r");
    }
    printf("MDP device driver fd = %d\n\r", MdpDrvFd() );


    /*-----------------------------------------------------------------------------
        Prepare Test Buffer
      -----------------------------------------------------------------------------*/
    PrepareTestImage();

    /*-----------------------------------------------------------------------------
        Test Function
      -----------------------------------------------------------------------------*/

    if( (argc >=2)  && (strcmp( argv[1] , "all" ) == 0 )  )
    {
        TestFunc_Mt6575_mHalBitblt();
        TestFunc_mHalIoCtrl_MHAL_IOCTL_BITBLT();
        TestFunc_Mt6575_CameraPreview();
        //TestFunc_MdpHal_Preview();
        TestFunc_MdpHal_Capture();
        TestFunc_Stnr();
        TestFunc_MdpPathJpgEncScale();
        TestFunc_MdpPipeCameraCapture();
        //TestFunc_MdpPathCamera2PreviewToMemory();
    }

    if( (argc >=2)  && (strcmp( argv[1] , "1" ) == 0 )  )
    {
        TestFunc_Mt6575_mHalBitblt();
    }

    if( (argc >=2)  && (strcmp( argv[1] , "2" ) == 0 )  )
    {
        TestFunc_MdpPathCameraPreviewToMemory_ZOOM();
    }
    
    /*No start up paremeters*/
    if( argc < 2 )
    {
        //TestFunc_Mt6575_ImageTransform();
        //TestFunc_Mt6575_mHalBitblt();
        //TestFunc_Mt6575_mHalBitblt_cust();
        //TestFunc_Mt6575_mHalBitblt_cust2();
        //TestFunc_Mt6575_mHalBitblt_cust_PerformanceTest();
        //TestFunc_Mt6575_mHalBitbltSlice();
        TestFunc_Mt6575_mHalMdp_BitBltEx();
        //TestFunc_mHalIoCtrl_MHAL_IOCTL_BITBLT();
        //TestFunc_Mt6575_CameraPreview();
        //TestFunc_MdpHal_Preview();
        //TestFunc_MdpHal_Capture();
        //TestFunc_Stnr();
        //TestFunc_MdpPathJpgEncScale();
        //TestFunc_MdpPathJpgEncScale2();
        //TestFunc_MdpPathN3d2ndPass();
        //TestFunc_MdpPathDisplayFromMemoryHdmi();
        //TestFunc_MdpPipeCameraCapture();
        //TestFunc_MdpPathCamera2PreviewToMemory();
        //TestFunc_MdpPathCameraPreviewToMemory();
        //TestFunc_MdpPathCameraPreviewToMemory_ZOOM();
        //TestFunc_MdpPathCameraPreviewToMemory_Cust();
        //TestFunc_MdpPathCameraPreviewToMemory_NoBypass();//For Ryan Wang
        //TestFunc_FactoryMode();
        
    }

     


    
    /*-----------------------------------------------------------------------------
        Close device driver
      -----------------------------------------------------------------------------*/
    if( MdpDrvRelease() == -1 )  //Close mdp device driver node
    {
        printf("Close mdp device driver failed\n\r!");
    } else
    {
        printf("Close mdp device driver Success\n\r!");
    }
    
    
    return 0;
}

































