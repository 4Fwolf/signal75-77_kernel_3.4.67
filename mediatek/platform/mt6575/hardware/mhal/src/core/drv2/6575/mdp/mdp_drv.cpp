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

#include "mdp_drv.h"
#include "mdp_sysram.h"


#ifdef MDP_FLAG_0_PLATFORM_LINUX
//open syscall
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//close syscall
#include <unistd.h>
//mmap syscall
#include <sys/mman.h>

//strerror
#include <string.h>
#include <errno.h>

//mutex
#include <pthread.h>

//gettimeofday
#include <sys/time.h>       

//Property
#include <cutils/properties.h>

//Callstack
#include <utils/CallStack.h>



//Kernel Includes
#include "mt_mdp.h" //For kernel structure

    #ifdef MDP_FLAG_1_SUPPORT_M4U
    //For M4U
    #include    "mdp_m4u.h" 
    #endif /*MDP_FLAG_1_SUPPORT_M4U*/

#endif /*MDP_FLAG_0_PLATFORM_LINUX*/

/*/////////////////////////////////////////////////////////////////////////////
    Macro Re-Definition
  /////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////
    Global Variable
        - Logcat message switch
  /////////////////////////////////////////////////////////////////////////////*/
#define MDP_PROPERTY_VERSION_NONE   (0xFF)
/*This version number should be advanced by one, each time 
  the property format is change.( default value...etc..)
  note that, it's independ from android version*/
#define MDP_PROPERTY_VERSION        (0xA2)
/*
    History:
    0xA1    :   Profile default build-in and turn on ( g_b_profile = 1 , g_profile_level = 4->0 )
    0xA2    :   g_b_profile = 1->0 (for reduce log amount)
 */

int g_b_show_info_resource  = 0;    /*1*/
int g_b_show_info_fps       = 0;    /*2*/
int g_b_show_info_queue     = 0;    /*3*/
int g_b_show_info_mem       = 0;    /*4*/

int g_b_show_info_path      = 0;    /*5*/
int g_b_show_info_mdpd      = 1;    /*6*/
int g_b_show_info           = 0;    /*7*//*verbose*/

int g_b_profile             = 0;    /*8*/

int g_profile_level         = 0;    /*9*//*print profile information which level <= n ; greater n provide more detail info*/
                                         /*n = 0 - 4*/
                                    





/*/////////////////////////////////////////////////////////////////////////////
    MDP Platform Driver Function
  /////////////////////////////////////////////////////////////////////////////*/
static int _MdpDrvUpdateProperty( void )
{
#ifdef MDP_FLAG_0_PLATFORM_LINUX /*Actually,this is for android platform*/
    unsigned int    prop_ver             = 0x0;
    int             b_different_version = 0;
    char value[PROPERTY_VALUE_MAX];
    char default_value[PROPERTY_VALUE_MAX];


    sprintf( default_value, "%x", MDP_PROPERTY_VERSION_NONE );

    property_get("persist.mdp_msg_ver", value, default_value);

    sscanf( value, "%x", &prop_ver );

    if( prop_ver != MDP_PROPERTY_VERSION ){
        MDP_WARNING("MDP Property Different Version Detect (0x%04X (current) != 0x%04X)\n", MDP_PROPERTY_VERSION, prop_ver );
        b_different_version = 1;
    }

    sprintf( value, "%x", MDP_PROPERTY_VERSION );

    property_set("persist.mdp_msg_ver", value);

    //-----------------------------------------------------------------------------//
    //-----------------------------------------------------------------------------//

    
                           /*1  2  3  4  5  6  7  8  9*/
    sprintf( default_value, "%d %d %d %d %d %d %d %d %d",
            g_b_show_info_resource, g_b_show_info_fps,  g_b_show_info_queue, g_b_show_info_mem, 
            g_b_show_info_path,     g_b_show_info_mdpd, g_b_show_info   ,    g_b_profile ,
            g_profile_level );

    if( b_different_version )
    {
        
        property_set("persist.mdp_msg", default_value);

        //MDP_PRINTF("[MDP] persist.mdp_msg = %s\n", value );
    }
    else
    {
        property_get("persist.mdp_msg", value, default_value);
                      /*1  2  3  4  5  6  7  8  9*/
        sscanf( value, "%d %d %d %d %d %d %d %d %d", 
                &g_b_show_info_resource, &g_b_show_info_fps,  &g_b_show_info_queue, &g_b_show_info_mem, 
                &g_b_show_info_path,     &g_b_show_info_mdpd, &g_b_show_info   ,    &g_b_profile,
                &g_profile_level);
        

        property_set("persist.mdp_msg", value);

        //MDP_PRINTF("[MDP] persist.mdp_msg = %s\n", value );
    }

    
#endif
    return 0;
}





static int              g_mdp_device_driver_fd = -1;    //MDP device driver node file descriptor
static unsigned long    g_mdp_drv_ref_count = 0;        //MDP driver initial reference count


#ifdef MDP_FLAG_0_PLATFORM_LINUX

pthread_mutex_t mutex_mdpdrv = PTHREAD_MUTEX_INITIALIZER;

static void ENTER_CRITICAL_MDPDRV( void )
{
    pthread_mutex_lock ( &mutex_mdpdrv );
    //MDP_INFO("MDPDRV MUTEX LOCK\n");
}

static void EXIT_CRITICAL_MDPDRV( void )
{
    //MDP_INFO("MDPDRV MUTEX UN-LOCK\n");
    pthread_mutex_unlock ( &mutex_mdpdrv );
}

#endif


int MdpDrvInit( void )      //Open mdp device driver node
{
    
#ifdef MDP_FLAG_0_PLATFORM_LINUX

    int ret_val = 0;

    ENTER_CRITICAL_MDPDRV(); /*Critical Section Enter--------------------------------------------*/

    //MDP_INFO("UID=%d\n",getuid());
    
    /*Increase reference count*/
    g_mdp_drv_ref_count++;
        
    if(  g_mdp_drv_ref_count == 1 ) /*First time initial for this process*/
    {

        //MDP_INFO("MDP Device Driver Initial\n");

        /*Update property only the first time for this process*/
        _MdpDrvUpdateProperty();

        /*=============================================================================
            MDP Device Driver
          =============================================================================*/
        if( g_mdp_device_driver_fd == -1 )
        {
            g_mdp_device_driver_fd = open("/dev/mt-mdp", O_RDWR);

            if(-1 == g_mdp_device_driver_fd)
            {
                MDP_ERROR("Open MDP driver file failed.:%s\n", strerror(errno));
                ret_val = -1;
            }
        }




        #ifdef MDP_FLAG_1_SUPPORT_M4U
        /*=============================================================================
            M4U Device Driver
          =============================================================================*/
        if( MdpM4u_Init() != 0 )
        {
            MDP_ERROR("MDP M4U driver initial failed.\n");
            ret_val = -1;
        }
        #endif /*MDP_FLAG_1_SUPPORT_M4U*/
        

        /*=============================================================================
            SYSRAM Device Driver
          =============================================================================*/
        if( MdpSysram_Init() != 0 )
        {
            ret_val = -1;
        }

    }


    
    EXIT_CRITICAL_MDPDRV(); /*Critical Section Exit--------------------------------------------*/



    return ret_val;
    
#endif /*MDP_FLAG_0_PLATFORM_LINUX*/



}


int MdpDrvRelease( void )   //Close mdp device driver node
{
    
#ifdef MDP_FLAG_0_PLATFORM_LINUX

    int ret = 0;
    

    ENTER_CRITICAL_MDPDRV(); /*Critical Section Enter--------------------------------------------*/
    
    /*Decrease reference count*/
    g_mdp_drv_ref_count--;
    
    if(  g_mdp_drv_ref_count == 0 ) /*Last time driver release for this process*/
    {
        
        //MDP_INFO("MDP Device Driver Release\n");
        
        /*=============================================================================
            MDP Device Driver
          =============================================================================*/
        if( close( g_mdp_device_driver_fd ) != 0 )
            ret = -1;
        g_mdp_device_driver_fd = -1;

        /*=============================================================================
            M4U Device Driver
          =============================================================================*/
        #ifdef MDP_FLAG_1_SUPPORT_M4U
        if( MdpM4u_Release() != 0 )
        {
            MDP_ERROR("MDP M4U driver release failed.\n");
            ret = -1;
        }
        #endif /*MDP_FLAG_1_SUPPORT_M4U*/

        /*=============================================================================
            SYSRAM Device Driver
          =============================================================================*/
        if( MdpSysram_Release() < 0 )
        {
            ret = -1;
        }
    }
    
    EXIT_CRITICAL_MDPDRV(); /*Critical Section Exit--------------------------------------------*/

    return ret;

#endif /*MDP_FLAG_0_PLATFORM_LINUX*/


}

int MdpDrvFd( void )
{
    
    if( g_mdp_device_driver_fd == -1 )
    {
        /*Show error message when referenced*/
        MDP_ERROR("MDP device driver is not opened!\n");
    }
    
    return g_mdp_device_driver_fd;
}


int MdpDrvIsPmem( unsigned long addr, unsigned long size, unsigned long *p_phy_addr ) //Return 1 if addr is pmem
{
    
#ifdef MDP_FLAG_0_PLATFORM_LINUX
    stPMEMRange PMEMRange;

    PMEMRange.u4StartAddr = addr;
    PMEMRange.u4Size = size;
    
    if( ioctl( MdpDrvFd(), MT_MDP_G_PMEMRANGE , &PMEMRange ) )
    {
        MDP_ERROR("pmem check memory range failed in ioctl:%s\n", strerror(errno) );
        return -1; /*Error*/
    }

    if( PMEMRange.u4Result == 1 ) 
    {
        *p_phy_addr = PMEMRange.u4StartAddr;
        return 1;   //It's PMem
    }
    return 0;  //It's not PMem
#endif

}



int MdpDrvLockResource( unsigned long mdp_resource_mask, int b_time_shared, unsigned long time_out_ms, const char* path_name_str  )
{
    
#ifdef MDP_FLAG_0_PLATFORM_LINUX
    int ret_value;
    int b_lock_fail_show_error = 0;
    stLockResParam  lock_param;
    unsigned long   time_elapse;
    struct timeval  tv0,tv1;
    struct timezone tz0,tz1;

    lock_param.u4LockResTable   = mdp_resource_mask;
    lock_param.u4IsTimeShared   = b_time_shared;
    lock_param.u4TimeOutInms    = time_out_ms;
    lock_param.out_Table_status_in_use      = 0xFFFFFFFF;
    lock_param.out_Table_status_in_occupied = 0xFFFFFFFF;

    gettimeofday( &tv0, &tz0 );

    /*-----------------------------------------------------------------------------
        Check Path name
      -----------------------------------------------------------------------------*/
    if( path_name_str == NULL )
    {
        b_lock_fail_show_error = 0; /*Current ImageTransform and Tzu-Meng's Jpeg path do not pass path name*/
    } else
    {
        b_lock_fail_show_error = 1;
    }

    
    /*-----------------------------------------------------------------------------
        Lock resource IO control
      -----------------------------------------------------------------------------*/
    ret_value = ioctl( MdpDrvFd(), MT_MDP_T_LOCKRESOURCE , &lock_param );
    if( ret_value < 0 )
    {
        gettimeofday( &tv1, &tz1 );
        time_elapse = (tv1.tv_sec - tv0.tv_sec)*1000000 + (tv1.tv_usec - tv0.tv_usec);

        if( (b_time_shared == 0) || (b_lock_fail_show_error == 1) )/*If this is a non-share path, lock fail is an error*/
        {
        MDP_ERROR("[Resource] Lock Fail: Req:0x%08X Share:%d TimeOut:%dms tbl_in_use:0x%08X tbl_in_occu:0x%08X time_elapse:%luus\n",
                  (unsigned int)lock_param.u4LockResTable,(int) lock_param.u4IsTimeShared ,  (int)lock_param.u4TimeOutInms, 
                  (unsigned int)lock_param.out_Table_status_in_use, (unsigned int)lock_param.out_Table_status_in_occupied,
                  time_elapse);
        }
        else
        {
        MDP_INFO_RESOURCE("[Resource] Lock Fail: Req:0x%08X Share:%d TimeOut:%dms tbl_in_use:0x%08X tbl_in_occu:0x%08X time_elapse:%luus\n",
                  (unsigned int)lock_param.u4LockResTable,(int) lock_param.u4IsTimeShared ,  (int)lock_param.u4TimeOutInms, 
                  (unsigned int)lock_param.out_Table_status_in_use, (unsigned int)lock_param.out_Table_status_in_occupied,
                  time_elapse);
        }
        return -1; 
    }
    gettimeofday( &tv1, &tz1 );
    time_elapse = (tv1.tv_sec - tv0.tv_sec)*1000000 + (tv1.tv_usec - tv0.tv_usec);
    
    
    MDP_INFO_RESOURCE("[Resource] Lock OK: Req:0x%08X Share:%d TimeOut:%dms tbl_in_use:0x%08X tbl_in_occu:0x%08X time_elapse:%luus\n",
              (unsigned int)lock_param.u4LockResTable,(int) lock_param.u4IsTimeShared ,  (int)lock_param.u4TimeOutInms, 
              (unsigned int)lock_param.out_Table_status_in_use, (unsigned int)lock_param.out_Table_status_in_occupied,
              time_elapse);

    return 0; 
#endif

}


int MdpDrvUnLockResource( unsigned long mdp_resource_mask )
{
    
#ifdef MDP_FLAG_0_PLATFORM_LINUX
    MDP_INFO_RESOURCE("[Resource] Release : Req:0x%08X\n", (unsigned int)mdp_resource_mask );

    if( ioctl( MdpDrvFd(), MT_MDP_T_UNLOCKRESOURCE , mdp_resource_mask ) < 0 )
    {
        MDP_ERROR("[Resource] Release Fail: Req:0x%08X\n", (unsigned int)mdp_resource_mask );
        return -1; 
    }

    
    return 0; 
#endif

}

int MdpDrv_DumpRegister( void )
{
    
#ifdef MDP_FLAG_0_PLATFORM_LINUX
    if( ioctl( MdpDrvFd(), MT_MDP_T_DUMPREG , NULL ) < 0 )
    {
        return -1; 
    }
    return 0; 
#endif

}



/*Define the MDP element resources that support wait INT done*/
#if defined (MDP_KERNEL_FLAG_0_FPGA)

static const unsigned long kRES_SUPPORT_INT_DONE = 
    MID_RGB_ROT0 | MID_RGB_ROT1 | MID_RGB_ROT2 |
    MID_VDO_ROT0 | MID_VDO_ROT1 ;


#else /*Normal INT support list*/

#if 0
static const unsigned long kRES_SUPPORT_INT_DONE = 
    MID_RGB_ROT0 | MID_RGB_ROT1 | MID_RGB_ROT2 |
    MID_VDO_ROT0 | MID_VDO_ROT1 |
    MID_JPEG_DMA |
    MID_OVL |
    MID_R_DMA0 | MID_R_DMA1 |
    MID_BRZ |
    MID_CRZ | MID_PRZ0 | MID_PRZ1 |
    MID_VRZ0 | MID_VRZ1 ;
#else
    
static const unsigned long kRES_SUPPORT_INT_DONE = 
    MID_RGB_ROT0 | MID_RGB_ROT1 | MID_RGB_ROT2 |
    MID_VDO_ROT0 | MID_VDO_ROT1 |
    MID_JPEG_DMA |
    MID_OVL |
    MID_R_DMA0 | MID_R_DMA1 |
    /*MID_BRZ |*/
    MID_CRZ | MID_PRZ0 | MID_PRZ1 |
    MID_VRZ0 | MID_VRZ1 ;

#endif
    
#endif
    

int MdpDrvWaitIntDone( unsigned long mdp_rsc_id_mask , unsigned long time_out_ms )
{
    int ret = 0;
    
#ifdef MDP_FLAG_0_PLATFORM_LINUX

    stWaitIrqParam aParam;



    /*Not a support resource, return success,no need to wait*/
    mdp_rsc_id_mask &= kRES_SUPPORT_INT_DONE;
    if( mdp_rsc_id_mask == 0 )
    {
        return 0;
    }


    aParam.u4IrqNo = mdp_rsc_id_mask;
    aParam.u4TimeOutInms = time_out_ms;

    
    ret = ioctl( MdpDrvFd(), MT_MDP_X_WAITIRQ , &aParam);

    
#endif

    return ret;
    
}


int MdpDrvClearIntDone( unsigned long mdp_rsc_id_mask )
{
    int ret = 0;
    
#ifdef MDP_FLAG_0_PLATFORM_LINUX

    ret = ioctl( MdpDrvFd(), MT_MDP_T_CLRIRQ , mdp_rsc_id_mask);
    
#endif

    return ret;
    
}

int MdpDrvColorFormatInfoPrintOut( MdpDrvColorInfo_t* p_ci )
{
    
    MDP_SHOW_VAR_D(p_ci->byte_per_pixel);
    MDP_SHOW_VAR_D(p_ci->b_is_generic_yuv);
    MDP_SHOW_VAR_D(p_ci->b_is_uv_swap);
    MDP_SHOW_VAR_D(p_ci->b_is_uv_interleave);

    //Stride Align: 0:None 1:2 align 2:4 align 3:8 align 4:16 align
    MDP_SHOW_VAR_D(p_ci->y_stride_align);
    MDP_SHOW_VAR_D(p_ci->uv_stride_align);
    

    /*Register setting*/
    MDP_SHOW_VAR_D(p_ci->in_format);    //For RDMA used
    MDP_SHOW_VAR_D(p_ci->swap);         //For RDMA used: 01:Byte Swap 10:RGB Swap 3:Byte&RGB Swap
    
    MDP_SHOW_VAR_D(p_ci->out_format);   //For ROTDMA used

    //Sampling Period. b00:0 b01:1 b10:2 b11:4
    MDP_SHOW_VAR_D(p_ci->yh);   
    MDP_SHOW_VAR_D(p_ci->yv);
    MDP_SHOW_VAR_D(p_ci->uvh);
    MDP_SHOW_VAR_D(p_ci->uvv);

    return 0;
    
}



int MdpDrvColorFormatInfoGet( MdpColorFormat color, MdpDrvColorInfo_t* p_ci )
{
    
    if( p_ci == NULL ) {
        MDP_ERROR("pointer is null\n");
        return -1;
    }

    //Clear structure
    memset( p_ci, 0, sizeof( MdpDrvColorInfo_t ) );
    
    
    
    switch( color )
    {
        case RGB888 :
            p_ci->byte_per_pixel = 3;
            p_ci->in_format = 0x1;
            p_ci->swap = 0;
            p_ci->out_format = 0x01;
            break;
        case BGR888 :
            p_ci->byte_per_pixel = 3;
            p_ci->in_format = 0x1;
            p_ci->swap = 2;
            p_ci->out_format = 0x09;
            break;
        case RGB565 :
            p_ci->byte_per_pixel = 2;
            p_ci->in_format = 0x2;
            p_ci->swap = 0;
            p_ci->out_format = 0x02;
            break;
        case BGR565 :
            p_ci->byte_per_pixel = 2;
            p_ci->in_format = 0x2;
            p_ci->swap = 2;
            p_ci->out_format = 0x0A;
            break;
        case ARGB8888 :
            p_ci->byte_per_pixel = 4;
            p_ci->in_format = 0x0;
            p_ci->swap = 0;
            p_ci->out_format = 0x00;
            break;
        case ABGR8888 :
            p_ci->byte_per_pixel = 4;
            p_ci->in_format = 0x0;
            p_ci->swap = 2;
            p_ci->out_format = 0x08;
            break;
        case RGBA8888 :
            p_ci->byte_per_pixel = 4;
            p_ci->in_format = 0x0;
            p_ci->swap = 3;
            p_ci->out_format = 0x48;
            break;
        case BGRA8888 :
            p_ci->byte_per_pixel = 4;
            p_ci->in_format = 0x0;
            p_ci->swap = 1;
            p_ci->out_format = 0x40;
            break;
        case UYVY_Pack ://UYVY, 
            p_ci->byte_per_pixel = 2;
            p_ci->in_format = 0x4;
            p_ci->out_format = 0x03;
            p_ci->yh = 1; 
            p_ci->yv = 1;
            p_ci->uvh = 2;
            p_ci->uvv = 1;
            break;
        case YUY2_Pack ://YUYV
            p_ci->byte_per_pixel = 2;
            p_ci->in_format = 0x6;
            p_ci->out_format = 0x83;
            p_ci->yh = 1; 
            p_ci->yv = 1;
            p_ci->uvh = 2;
            p_ci->uvv = 1;
            break;
        case Y411_Pack ://YUV410, 4x4 sub sample U/V plane
            p_ci->byte_per_pixel = 1;
            p_ci->in_format = 0x5;
            p_ci->out_format = 0x07;
            p_ci->yh = 1; 
            p_ci->yv = 1;
            p_ci->uvh = 3;
            p_ci->uvv = 1;
            break;
        case YV16_Planar ://YUV422, 2x1 subsampled U/V planes
            p_ci->b_is_generic_yuv = 1;
            p_ci->byte_per_pixel = 1;
            p_ci->in_format = 0x5;
            p_ci->out_format = 0x07;
            p_ci->yh = 1; 
            p_ci->yv = 1;
            p_ci->uvh = 2;
            p_ci->uvv = 1;
            break;
        case YV12_Planar ://YUV420, 2x2 subsampled U/V planes,only ROTDMA0
            p_ci->b_is_generic_yuv = 1;
            p_ci->byte_per_pixel = 1;
            p_ci->in_format = 0x5;
            p_ci->out_format = 0x07;
            p_ci->yh = 1; 
            p_ci->yv = 1;
            p_ci->uvh = 2;
            p_ci->uvv = 2;
            break;
        case Y800 ://Y plan only,only ROTDMA0
            p_ci->b_is_generic_yuv = 1;
            p_ci->byte_per_pixel = 1;
            p_ci->in_format = 0x5;
            p_ci->out_format = 0x07;
            p_ci->yh = 1; 
            p_ci->yv = 1;
            p_ci->uvh = 0;
            p_ci->uvv = 0;
            break;
        case NV12 ://YUV420, 2x2 subsampled , interleaved U/V plane,only ROTDMA0
            p_ci->b_is_generic_yuv = 1;
            p_ci->byte_per_pixel = 1;
            p_ci->b_is_uv_interleave = 1;
            p_ci->in_format = 0xD;
            p_ci->out_format = 0x06;
            p_ci->yh = 1; 
            p_ci->yv = 1;
            p_ci->uvh = 2;
            p_ci->uvv = 2;
            break;
        case NV21 ://YUV420, 2x2 subsampled , interleaved V/U plane,only ROTDMA0
            p_ci->b_is_generic_yuv = 1;
            p_ci->byte_per_pixel = 1;
            p_ci->b_is_uv_interleave = 1;
            p_ci->in_format = 0xF;
            p_ci->out_format = 0x0E;
            p_ci->yh = 1; 
            p_ci->yv = 1;
            p_ci->uvh = 2;
            p_ci->uvv = 2;
            break;
        case ANDROID_YV12://YUV420, 2x2 subsampled V/U planes , 16x stride
            p_ci->b_is_generic_yuv = 1;
            p_ci->byte_per_pixel = 1;
            p_ci->b_is_uv_swap = 1;
            p_ci->in_format = 0x5;
            p_ci->out_format = 0x07;
            p_ci->yh = 1; 
            p_ci->yv = 1;
            p_ci->uvh = 2;
            p_ci->uvv = 2;
            p_ci->y_stride_align = 4;//Y stride 16x align
            p_ci->uv_stride_align = 4;//UV stride 16x align
            break;
        case IMG_YV12://YUV420, 2x2 subsampled V/U planes , 32x stride
            p_ci->b_is_generic_yuv = 1;
            p_ci->byte_per_pixel = 1;
            p_ci->b_is_uv_swap = 1;
            p_ci->in_format = 0x5;
            p_ci->out_format = 0x07;
            p_ci->yh = 1; 
            p_ci->yv = 1;
            p_ci->uvh = 2;
            p_ci->uvv = 2;
            p_ci->y_stride_align = 5;//Y stride 32x align
            p_ci->uv_stride_align = 4;//UV stride 16x align
            break;
        case RAW:
            p_ci->b_is_generic_yuv = 0;
            p_ci->byte_per_pixel = 1;
            p_ci->out_format = 0x04;
        default :
            MDP_ERROR("failed -- unsupport format : %d !!" , color );
            return -1;
        break;
    }

    return 0;

}




#ifdef MDP_FLAG_1_SUPPORT_M4U
/*/////////////////////////////////////////////////////////////////////////////
    MDP Platform Driver Functions
        - M4U Support functions
  /////////////////////////////////////////////////////////////////////////////*/
/*-----------------------------------------------------------------------------
    color_component
         0: Y   1: U    2: V
  -----------------------------------------------------------------------------*/
int _MdpDrv_MdpId2M4uId( unsigned long mdp_id, int color_component,M4U_MODULE_ID_ENUM* p_module_id, M4U_PORT_ID_ENUM* p_port_id )
{
    /*Setup Module ID*/
    /*Setup ROTDMA port ID*/
    switch( mdp_id )
    {
    case MID_R_DMA0:
        *p_module_id    = M4U_CLNTMOD_RDMA0;
        break;
    case MID_R_DMA1:
        *p_module_id    = M4U_CLNTMOD_RDMA1;
        break;

    case MID_RGB_ROT0:/*DMA1_OUT*/
        *p_module_id    = M4U_CLNTMOD_ROT1;
        *p_port_id      = M4U_PORT_ROT_DMA1_OUT0;
        break;
    case MID_RGB_ROT1:/*DMA2_OUT*/
        *p_module_id    = M4U_CLNTMOD_ROT2;
        *p_port_id      = M4U_PORT_ROT_DMA2_OUT0;
        break;
    case MID_RGB_ROT2:/*DMA4_OUT*/
        *p_module_id    = M4U_CLNTMOD_ROT4;
        *p_port_id      = M4U_PORT_ROT_DMA4_OUT0;
        break;
    case MID_VDO_ROT0:/*DMA0_OUT*/
        *p_module_id    = M4U_CLNTMOD_ROT0;
        *p_port_id      = M4U_PORT_ROT_DMA0_OUT0;
        break;
    case MID_VDO_ROT1:/*DMA3_OUT*/
        *p_module_id    = M4U_CLNTMOD_ROT3;
        *p_port_id      = M4U_PORT_ROT_DMA3_OUT0;
        break;
        
#if defined(MDP_FLAG_2_M4U_SUPPORT_QUERY)
    case MID_RDMA_GENERAL:
        *p_module_id    = M4U_CLNTMOD_RDMA_GENERAL;
        break;

    case MID_ROTDMA_GENERAL:
        *p_module_id    = M4U_CLNTMOD_ROT_GENERAL;
        break;
#endif
        
     default:
        MDP_ERROR("Unknown MDP id:0x%08X\n", (unsigned int) mdp_id );
        return -1;

    }


    /*Setup RDMA port ID*/
    if( mdp_id == MID_R_DMA0 )
    {
        switch( color_component )
        {
        case 0:/*Y*/
            *p_port_id      = M4U_PORT_R_DMA0_OUT0;
            break;
        case 1:/*U*/
            *p_port_id      = M4U_PORT_R_DMA0_OUT1;
            break;
        case 2:/*V*/
            *p_port_id      = M4U_PORT_R_DMA0_OUT2;
            break;
        default:
            MDP_ERROR("Unknown color_component:%d\n", (int) color_component );
            return -1;
        }
    }

    if( mdp_id == MID_R_DMA1 )
    {
        switch( color_component )
        {
        case 0:/*Y*/
            *p_port_id      = M4U_PORT_R_DMA1_OUT0;
            break;
        case 1:/*U*/
            *p_port_id      = M4U_PORT_R_DMA1_OUT1;
            break;
        case 2:/*V*/
            *p_port_id      = M4U_PORT_R_DMA1_OUT2;
            break;
        default:
            MDP_ERROR("Unknown color_component:%d\n", (int) color_component );
            return -1;
        }
    }

    return 0;
    
}

#endif /*MDP_FLAG_1_SUPPORT_M4U*/


/*-----------------------------------------------------------------------------
    Adapt address base on the situation of pmem or m4u enabled.
    This function may "DO CHANGE" to the address input (MdpYuvAddr[])
    return 1 if change happened.
    return 0 if unchanged.

    Descision Making:
        0.Regard input address as VA
        1.Check If PMem
            1.1 Yes.It's PMem : address ==> PA
            1.2 NO.
                1.2.1 M4U On : address ==> MVA (turn on M4U simultaneously)
                1.2.2 M4U Off: address unchange (maybe it's physcal address)

    color_component:
             0: Y   1: U    2: V

    Return
         2: Adapt M4U MVA by cache
         1: Adapt M4U MVA and turn on M4U
         0: Adapt PMEM physical addr or already is physical addr
        -1: Error.
  -----------------------------------------------------------------------------*/
int _MdpDrv_AdaptAddr( unsigned long mdp_id, int color_component, unsigned long* p_addr, unsigned long size , unsigned long m4u_handle )
{
    int             b_is_pmem;
    unsigned long   pmem_phy_addr;
    unsigned long   org_addr = *p_addr;
    const char      color_comp[] = { 'Y', 'U', 'V' };


    if( size == 0 )
    {
        MDP_ERROR("Buffer %c : 0x%x size is 0\n", color_comp[color_component], (unsigned int)*p_addr );
        return -1;  /*Error, nothing happened*/
    }

    /*1.Check If PMem*/        
    b_is_pmem = MdpDrvIsPmem( *p_addr, size, &pmem_phy_addr ); //Return 0 if addr is pmem , and return physical address as well 

    /*1.1 Yes.It's PMem : address ==> PA*/
    if( b_is_pmem == 1 )
    {
        if( pmem_phy_addr == NULL ){
            MDP_ERROR("phy addr of pmem (0x%08X) is NULL\n", (unsigned int)*p_addr );
            return -1;
        }
        
        *p_addr = pmem_phy_addr;
        MDP_INFO_MEM("[MEM][PMEM VA->PA] %c:0x%08X -> 0x%08X(size=0x%x)\n",color_comp[color_component],(unsigned int)org_addr, (unsigned int)pmem_phy_addr,(unsigned int)size);

        if( *p_addr == 0 )
        {
            MDP_ERROR("Buffer address is NULL!\n");
            return -1;
        }

        return 0;
    }
    /*1.2 NO.Not a pmem*/
    else
    {
        /*1.2.1 M4U On : address ==> MVA (turn on M4U simultaneously)*/
        #ifdef MDP_FLAG_1_SUPPORT_M4U
        {
            unsigned long       mva_addr;
            M4U_MODULE_ID_ENUM  module_id;
            M4U_PORT_ID_ENUM    port_id;
            int                 b_is_alloc_mva;

            if( _MdpDrv_MdpId2M4uId( mdp_id, color_component, &module_id, &port_id ) != 0 )
            {
                MDP_ERROR("M4U ID mapping error\n");
                return -1;
            }

            
            if( MdpM4u_Start(   module_id, port_id,
                                *p_addr, size, 
                                &mva_addr , &b_is_alloc_mva ,
                                m4u_handle ) != 0 )
            {
                MDP_ERROR("M4U Start error\n");
                return -1;
            }

            #if defined(MDP_FLAG_2_M4U_SUPPORT_PREALLOC)
            if( ( mdp_id != MID_RDMA_GENERAL ) && ( mdp_id != MID_ROTDMA_GENERAL ) )
            {
                if( mva_addr == NULL ){
                    MDP_ERROR("MVA addr of VA (0x%08X) is NULL\n", (unsigned int)*p_addr );
                    return -1;
                }
            }
            #else
            if( mva_addr == NULL ){
                MDP_ERROR("MVA addr of VA (0x%08X) is NULL\n", (unsigned int)*p_addr );
                return -1;
            }
            #endif
            
            *p_addr = mva_addr;


            if( b_is_alloc_mva )
            {

                MDP_INFO_MEM("[MEM][M4U VA->MVA] %c:0x%08X -> 0x%08X(size=0x%x)\n",color_comp[color_component],(unsigned int)org_addr, (unsigned int)mva_addr,(unsigned int)size);
                return 1;   /*Adapt MVA by allocate new MVA*/
            } else
            {
                MDP_INFO_MEM("[MEM][M4U VA->MVA] %c:0x%08X -> 0x%08X(size=0x%x) - Cached\n",color_comp[color_component],(unsigned int)org_addr, (unsigned int)mva_addr,(unsigned int)size);
                return 2;   /*Adapt MVA by cache*/
            }
        
        }
        #endif /*MDP_FLAG_1_SUPPORT_M4U*/
        
        /*1.2.2 M4U Off: address unchange (maybe it's physcal address)*/
        /*Do nothing*/
    }
    

    MDP_INFO_MEM("[MEM][N/A(PA)] %c:0x%08X = 0x%08X(size=0x%x)\n",color_comp[color_component],(unsigned int)org_addr, (unsigned int)org_addr,(unsigned int)size);
        
    return 0;   /*Adapt PMEM physical addr*/
}

int MdpDrv_AdaptMdpYuvAddrArray(   unsigned long mdp_id, 
                                    MdpYuvAddr* in_addr_list, int addr_count,   //Original user input address  
                                    MdpYuvAddr* out_adapt_addr_list,            //Output adapted address
                                    unsigned long *p_adapt_m4u_flag_bit,        //If address has been adapted with m4u mva, corresponding bit will set to 1
                                    unsigned long *p_alloc_mva_flag_bit )       //If the mva is new allocate, corresponding bit will set to 1
{
    int i;
    int ret = 0;
    int ret_of_y = 0;

    *p_adapt_m4u_flag_bit = 0;
    *p_alloc_mva_flag_bit = 0;

    for( i = 0; i < addr_count; i++ )
    {
        MDP_INFO_MEM("Adapted Buffer %d/%d:\n", i+1, addr_count );

        //Duplicate in_addr to out_addr
        out_adapt_addr_list[i] = in_addr_list[i];

        //Adapt Y Address
        ret = _MdpDrv_AdaptAddr(    mdp_id, 0 /*Y*/, 
                                    &(out_adapt_addr_list[i].y), 
                                    out_adapt_addr_list[i].y_buffer_size,
                                    out_adapt_addr_list[i].m4u_handle );

        
        if( ret < 0 ){
            return -1;
        }

        switch( ret )
        {
            case 1: /*1:means mva allocate*/
                *p_alloc_mva_flag_bit |= ( 0x1 << i );//This MVA is allocated not by cache
            case 2: /*2:means mva is from cache*/
                *p_adapt_m4u_flag_bit |= ( 0x1 << i );//This memory has been adapted m4u
                break;
        }

        ret_of_y = ret;

       
        //Adapt U,V Address
        //If address u = 0 or v = 0, ignore
        if( in_addr_list[i].u != 0 )
        {
            if( ( ret = _MdpDrv_AdaptAddr(  mdp_id, 1 /*U*/, 
                                            &(out_adapt_addr_list[i].u), 
                                            out_adapt_addr_list[i].u_buffer_size,
                                            out_adapt_addr_list[i].m4u_handle) ) < 0 )
                return -1;

            if( ret == 1 ){
                /*To prevent MVA memory leak*/                
                *p_alloc_mva_flag_bit |= ( 0x1 << i );//This MVA is allocated not by cache
            }

            if( ret_of_y != ret ){
                MDP_ERROR("Return value of U (%d) != return value of Y(%d)\n",ret, ret_of_y );
                return -1;
            }
                
                
        }

        if( in_addr_list[i].v != 0 )
        {
            
            if( ( ret = _MdpDrv_AdaptAddr(  mdp_id, 2 /*V*/, 
                                            &(out_adapt_addr_list[i].v), 
                                            out_adapt_addr_list[i].v_buffer_size,
                                            out_adapt_addr_list[i].m4u_handle  ) ) < 0 )
                return -1;

            if( ret == 1 ){
                /*To prevent MVA memory leak*/                
                *p_alloc_mva_flag_bit |= ( 0x1 << i );//This MVA is allocated not by cache
            }
            
            if( ret_of_y != ret ){
                MDP_ERROR("Return value of V (%d) != return value of Y(%d)\n",ret, ret_of_y );
                return -1;
            }
        }

    }

    return 0;
}

int MdpDrv_UnAdaptMdpYuvAddrArray( unsigned long mdp_id, 
                                    MdpYuvAddr* in_addr_list, int addr_count,   //Original user input address  
                                    MdpYuvAddr* in_adapt_addr_list,             //Adapted address
                                    unsigned long adapt_m4u_flag_bit,           //Corresponding bit will set to 1 if address had been adapted with m4u mva
                                    unsigned long alloc_mva_flag_bit_ )         //Corresponding bit will set to 1 if the mva is new allocated
{
    
    int ret = 0;

#ifdef MDP_FLAG_1_SUPPORT_M4U
    {
        int i;
        M4U_MODULE_ID_ENUM  module_id;
        M4U_PORT_ID_ENUM    port_id;
        int b_is_alloc_mva = 0;
        


        for( i = 0; i < addr_count; i++ )
        {
            int color_component;
            unsigned long va_addr;
            unsigned long va_size;
            unsigned long mva_addr;
            unsigned long m4u_handle;

            if( adapt_m4u_flag_bit & (0x1 << i ) )
            {
                b_is_alloc_mva =  ( (alloc_mva_flag_bit_ & (0x1 << i )) == 0 ) ? 0 : 1;

                m4u_handle = in_addr_list[i].m4u_handle;
                
                //UnAdapt Y address
                color_component = 0; /*0:Y 1:U 2:V*/
                va_addr = in_addr_list[i].y;
                va_size = in_addr_list[i].y_buffer_size;
                mva_addr = in_adapt_addr_list[i].y;

                
                if( _MdpDrv_MdpId2M4uId( mdp_id, color_component, &module_id, &port_id ) != 0 )
                {
                    MDP_ERROR("M4U ID mapping error\n");
                    return -1;
                }
        
                MdpM4u_End( module_id, port_id, va_addr, va_size, mva_addr , b_is_alloc_mva , m4u_handle );

                //UnAdapt U,V address
                //If address y=u=v , ignore
                if( ( in_addr_list[i].y != in_addr_list[i].u ) &&
                    ( in_addr_list[i].u != 0 )  && (  in_addr_list[i].v != 0 ) )
                {
                    //U
                    color_component = 1; /*0:Y 1:U 2:V*/
                    va_addr = in_addr_list[i].u;
                    va_size = in_addr_list[i].u_buffer_size;
                    mva_addr = in_adapt_addr_list[i].u;

                    if( _MdpDrv_MdpId2M4uId( mdp_id, color_component, &module_id, &port_id ) != 0 )
                    {
                    MDP_ERROR("M4U ID mapping error\n");
                    return -1;
                    }

                    MdpM4u_End( module_id, port_id, va_addr, va_size, mva_addr , b_is_alloc_mva , m4u_handle );

                    //V
                    color_component = 2; /*0:Y 1:U 2:V*/
                    va_addr = in_addr_list[i].v;
                    va_size = in_addr_list[i].v_buffer_size;
                    mva_addr = in_adapt_addr_list[i].v;

                    if( _MdpDrv_MdpId2M4uId( mdp_id, color_component, &module_id, &port_id ) != 0 )
                    {
                    MDP_ERROR("M4U ID mapping error\n");
                    return -1;
                    }

                    MdpM4u_End( module_id, port_id, va_addr, va_size, mva_addr , b_is_alloc_mva , m4u_handle );
                }
            }
        }
    }

#endif /*MDP_FLAG_1_SUPPORT_M4U*/

    

    return ret;
    
}
/*
 Adapt MVA for ZSD NEW path's last preview buffer
 */


int MdpDrv_AdaptM4uForZSD(unsigned long va_addr, unsigned long va_size,
                    unsigned long* p_mva_addr    )
{
    
#ifdef MDP_FLAG_1_SUPPORT_M4U
    return MdpM4u_ZSD_Start(M4U_CLNTMOD_ROT1, va_addr, va_size, p_mva_addr);
#else
    return 0;
#endif
}

int MdpDrv_UnAdaptM4uForZSD(unsigned long va_addr, unsigned long va_size, unsigned long mva_addr )
{
    
#ifdef MDP_FLAG_1_SUPPORT_M4U
    return MdpM4u_ZSD_End (M4U_CLNTMOD_ROT1, va_addr, va_size, mva_addr );
#else
    return 0;
#endif
}


/*
    This function only take followings as input:
        p_yuv_addr->y
        color_format
        (img_size)
        (img_roi)

    and help user calculate out (and fill it as well) these fields:
        p_yuv_addr->u
        p_yuv_addr->v
        p_yuv_addr->y_buffer_size
        p_yuv_addr->u_buffer_size
        p_yuv_addr->v_buffer_size
 */

int MdpDrv_CalImgBufferSizeAndFillIn( 
                MdpColorFormat  color_format, 
                MdpYuvAddr*     p_yuv_addr,
                MdpSize         img_size,
                MdpRect         img_roi,
                int             rotate )
{
    MdpDrvColorInfo_t   ci;
    int     bpp_of_y_plane;     /*bits per pixel for "y of yuv planar" or "yuv of yuv pack" and "rgb"*/
    int     yh,yv;              /*horizontal and vertical sample period of y*/
    int     uvh,uvv;            /*horizontal and vertical sample period of u,v*/
    int     b_is_plane_yuv;
    int     b_is_uv_interleave = 0;

    int     min_sample_period_h = 0;
    int     min_sample_period_v = 0;

    //dimension after rotate
    unsigned long rot_y_stride = 0; 
    unsigned long rot_uv_stride= 0;
    unsigned long rot_y_height, rot_uv_height;
    unsigned long rot_roi_x, rot_roi_y, rot_roi_w, rot_roi_h;

    /*-----------------------------------------------------------------------------
        Sanity Check
      -----------------------------------------------------------------------------*/
    if( p_yuv_addr->y == 0 )
    {
        MDP_ERROR("Buffer address is NULL!\n");
        return -1;
    }
    
    
    /*-----------------------------------------------------------------------------*/  


    
    bpp_of_y_plane = 0;
    yh  = 0;     yv = 0;
    uvh = 0;    uvv = 0;

    b_is_plane_yuv = 0;

    
    if( MdpDrvColorFormatInfoGet( color_format, &ci ) < 0 )
    {
        MDP_ERROR("Unsupport color format:%d\n", color_format );
        return -1;
    }


    b_is_plane_yuv      = ci.b_is_generic_yuv;
    b_is_uv_interleave  = ci.b_is_uv_interleave;
    bpp_of_y_plane      = ci.byte_per_pixel*8; /*bits*/  
    yh      = ci.yh;     
    yv      = ci.yv;
    uvh     = ci.uvh;    
    uvv     = ci.uvv;



    /*-----------------------------------------------------------------------------
        Calculate buffer size
        Check rotation
      -----------------------------------------------------------------------------*/
    //By 73's definition, x,y of roi is after rotation
    rot_roi_x = img_roi.x;
    rot_roi_y = img_roi.y;

    
    if( ( rotate & 0x01) == 0 )  { //0,180
        rot_y_stride = img_size.w;
        rot_y_height = img_size.h;

        rot_roi_w = img_roi.w;
        rot_roi_h = img_roi.h;
    }
    else  {//90,270
        rot_y_stride = img_size.h;
        rot_y_height = img_size.w;

        rot_roi_w = img_roi.h;
        rot_roi_h = img_roi.w;
    }

    
    /*-----------------------------------------------------------------------------
        Calculate buffer size
        - Calculate buffer size
      -----------------------------------------------------------------------------*/
#if 0
    if( color_format != ANDROID_YV12 )
    {
#endif

        if( ( b_is_plane_yuv == 0 ) || ( uvh == 0 || uvv == 0 ) )
        {
            #if 1 /*take roi into considerision*/
            p_yuv_addr->y_buffer_size = rot_y_stride * ( rot_roi_y + rot_roi_h - 1 ) + ( rot_roi_x + rot_roi_w );
            p_yuv_addr->y_buffer_size = p_yuv_addr->y_buffer_size * bpp_of_y_plane / 8;
            #else
            p_yuv_addr->y_buffer_size = rot_y_stride * rot_y_height * bpp_of_y_plane / 8;
            #endif
            
            p_yuv_addr->u = p_yuv_addr->v = 0;
            p_yuv_addr->u_buffer_size= p_yuv_addr->v_buffer_size= 0;

        }
        else
        {
            min_sample_period_h = yh > uvh ? uvh : yh;
            min_sample_period_v = yv > uvv ? uvv : yv;
            
            p_yuv_addr->y_buffer_size   = rot_y_stride * rot_y_height * bpp_of_y_plane 
                                        *  min_sample_period_h/yh  *  min_sample_period_v/yv 
                                        / 8;
            
            rot_uv_stride = rot_y_stride * min_sample_period_h/uvh;
            MDP_ROUND_UP( rot_uv_stride, ci.uv_stride_align );//Round uv stride to 16x align
            
            p_yuv_addr->u_buffer_size   = rot_uv_stride * rot_y_height * bpp_of_y_plane 
                                          *  min_sample_period_v/uvv 
                                        / 8;


            #if 1 /*take roi into considerision*/
            p_yuv_addr->v_buffer_size = rot_uv_stride * ( rot_roi_y + rot_roi_h - 1 ) + ( rot_roi_x + rot_roi_w ) * min_sample_period_h/uvh;
            p_yuv_addr->v_buffer_size = p_yuv_addr->v_buffer_size * bpp_of_y_plane 
                                        *  min_sample_period_v/uvv 
                                        / 8;
            #else
            p_yuv_addr->v_buffer_size = p_yuv_addr->u_buffer_size;
            #endif

            p_yuv_addr->u = p_yuv_addr->y + p_yuv_addr->y_buffer_size;
            p_yuv_addr->v = p_yuv_addr->u + p_yuv_addr->u_buffer_size;

            if( b_is_uv_interleave )
            {
                #if 1 /*take roi into considerision*/
                p_yuv_addr->u_buffer_size = p_yuv_addr->v_buffer_size * 2; /*Maybe with roi in v*/
                p_yuv_addr->v_buffer_size = 0;
                p_yuv_addr->v = 0;
                #else
                p_yuv_addr->u_buffer_size *= 2;
                p_yuv_addr->v_buffer_size = 0;
                p_yuv_addr->v = 0;
                #endif
            }
            
            if( ci.b_is_uv_swap )
            {
                unsigned long temp;

                #define _SWAP( _a, _b, _temp )  _temp = _a; _a = _b; _b = _temp;
                _SWAP( p_yuv_addr->u , p_yuv_addr->v, temp );
                _SWAP( p_yuv_addr->u_buffer_size, p_yuv_addr->v_buffer_size, temp );
                
        }
            
        }

        #if 1
        MDP_INFO_MEM("\nCalImgBufferSizeAndFillIn:\n");
        MDP_INFO_MEM("\tcolor format:%d\n",(int)color_format);
        MDP_INFO_MEM("\tci.b_is_uv_swap:%d\n",(int)ci.b_is_uv_swap);
        MDP_INFO_MEM("\tCal buffer addr Y:0x%08X U:0x%08X V:0x%08X\n", (unsigned int)p_yuv_addr->y, (unsigned int)p_yuv_addr->u, (unsigned int)p_yuv_addr->v);
        MDP_INFO_MEM("\tCal buffer size Y:0x%X U:0x%X V:0x%X\n", (unsigned int)p_yuv_addr->y_buffer_size, (unsigned int)p_yuv_addr->u_buffer_size, (unsigned int)p_yuv_addr->v_buffer_size);
        MDP_INFO_MEM("\trot_y_stride = %lu rot_uv_stride = %lu\n", rot_y_stride,rot_uv_stride );
        MDP_INFO_MEM("\timg_size = %lu %lu\n", img_size.w , img_size.h );
        MDP_INFO_MEM("\timg_roi = %lu %lu %lu %lu\n", img_roi.x, img_roi.y, img_roi.w , img_roi.h );
        MDP_INFO_MEM("\trotate = %d\n", rotate );

        MDP_INFO_MEM("\tmin_sample_period_h = %d  v = %d\n", min_sample_period_h,min_sample_period_v );
        
        MDP_INFO_MEM("\tyh = %d yv = %d uvh = %d uvv = %d\n", yh , yv, uvh, uvv);
        
        MDP_INFO_MEM("\n");
        #endif
        
   
    

    //MDP_INFO("Calculated color format:%d\n",(int)color_format);
    //MDP_INFO("Calculated buffer addr Y:0x%08X U:0x%08X V:0x%08X\n", (unsigned int)p_yuv_addr->y, (unsigned int)p_yuv_addr->u, (unsigned int)p_yuv_addr->v);
    //MDP_INFO("Calculated buffer size Y:0x%X U:0x%X V:0x%X\n", (unsigned int)p_yuv_addr->y_buffer_size, (unsigned int)p_yuv_addr->u_buffer_size, (unsigned int)p_yuv_addr->v_buffer_size);

    return 0;
    
    
}


int MdpDrv_CalImgBufferArraySizeAndFillIn( 
                MdpColorFormat  color_format, 
                MdpYuvAddr*     yuv_addr_array,
                int             count,
                MdpSize         img_size,
                MdpRect         img_roi,
                int             rotate,
                unsigned long*  p_total_size )
{
    int i;
    unsigned long   m4u_handle;
    
    if( ( p_total_size == NULL) || ( yuv_addr_array == NULL ) )
    {
        MDP_ERROR("pointer is null\n");
        return -1;
    }
    
    m4u_handle = yuv_addr_array[0].m4u_handle;

    *p_total_size = 0;
    
    for ( i = 0 ; i < count; i++ )
    {
        //MDP_INFO("Buffer %d:\n", i );
        /*.............................................................................
            Use address[0] as the start address of these sequence of buffer.
            thus, address[1] = address[0] + address[0].size

            Skip the address[0], it is given by user.
          .............................................................................*/
        if( i != 0 )
        {
            yuv_addr_array[i].y =   yuv_addr_array[i-1].y + 
                                    yuv_addr_array[i-1].y_buffer_size +
                                    yuv_addr_array[i-1].u_buffer_size +
                                    yuv_addr_array[i-1].v_buffer_size;

            yuv_addr_array[i].m4u_handle = m4u_handle;
        }
        
        
        /*.............................................................................
            Here calculate the u,v address and buffer size base on y address
          .............................................................................*/
        if( MdpDrv_CalImgBufferSizeAndFillIn( 
                        color_format, 
                        &(yuv_addr_array[i]),
                        img_size,
                        img_roi,
                        rotate ) != 0 )
        {
            
            MDP_ERROR("Calc %dth/%d memory size error.\n", i, count );
            return -1;
        }

        *p_total_size +=    yuv_addr_array[i].y_buffer_size + 
                            yuv_addr_array[i].u_buffer_size + 
                            yuv_addr_array[i].v_buffer_size;
        
    }

    return 0;
    
}



int MdpDrv_CacheSync( unsigned long mdp_id, MDPDRV_CACHE_SYNC_OP op, unsigned long start_addr, unsigned long size)
{

#ifdef MDP_FLAG_1_SUPPORT_M4U
    #if !defined(MDP_FLAG_2_CACHE_USE_FLUSH_ALL) /*If use flush all,flush range api is null function*/
    
    /*                                                           *
     * Use cache sync function which is implement in M4U module  *
     *                                                           */

    M4U_MODULE_ID_ENUM      module_id;
    M4U_PORT_ID_ENUM        port_id;    /*dummy*/
    M4U_CACHE_SYNC_ENUM     eCacheSync;


    if( _MdpDrv_MdpId2M4uId( mdp_id, 0 /*Dont't care*/, &module_id, &port_id ) != 0 )
    {
        MDP_ERROR("Cache Syc OP:M4U ID mapping error\n");
        return -1;
    }

    switch( op )
    {
        case MDPDRV_CACHE_SYNC_CLEAN_BEFORE_HW_READ_MEM:
            eCacheSync = M4U_CACHE_CLEAN_BEFORE_HW_READ_MEM; //There is a unsolved issue with p15 clean operation
            //eCacheSync = M4U_CACHE_FLUSH_BEFORE_HW_READ_MEM;

            break;
        case MDPDRV_CACHE_SYNC_INVALID_BEFORE_HW_WRITE_MEM:
            eCacheSync = M4U_CACHE_FLUSH_BEFORE_HW_WRITE_MEM;
            break;
        default:
            MDP_ERROR("Cache Syc OP:Unsupport operation:%d\n", (int)op );
            return -1;
    }


    if( MdpM4u_CacheSync(   module_id, eCacheSync,  start_addr, size) < 0 )
    {
        MDP_ERROR("Cache Sync error\n");
        return -1;
    }

    //MDP_INFO("Cache Sync OP(%d) ID(0x%08X) Addr: 0x%08X(0x%X)\n", (int)op, (unsigned int)mdp_id, (unsigned int)start_addr, (unsigned int)size);
    
    #endif /*#if !defined(MDP_FLAG_2_CACHE_USE_FLUSH_ALL)*/

    return 0;
#else
    return 0;
#endif
    
}

int MdpDrv_CacheFlushAll( void )
{
#if defined(MDP_FLAG_2_CACHE_USE_FLUSH_ALL)

    
    #ifdef  MDP_FLAG_1_SUPPORT_M4U
    if( MdpM4u_CacheFlushAll() < 0 )
    {
        MDP_ERROR("Cache flush all error\n");
        return -1;
    }
    #endif

    return 0;

#else
    return 0;
#endif
}



int MdpDrv_ConfigZoom( MdpDrvConfigZoom_Param* pParam )
{

#ifdef MDP_FLAG_0_PLATFORM_LINUX
    stZoomSetting   ioctlparam;

    if( pParam == NULL )    return -1;

    ioctlparam.mdp_id     = pParam->mdp_id;
    ioctlparam.reg_CFG    = pParam->reg_CFG;
    ioctlparam.reg_SRCSZ  = pParam->reg_SRCSZ;
    ioctlparam.reg_CROPLR = pParam->reg_CROPLR;
    ioctlparam.reg_CROPTB = pParam->reg_CROPTB;
    ioctlparam.reg_HRATIO = pParam->reg_HRATIO;
    ioctlparam.reg_VRATIO = pParam->reg_VRATIO;

    
    if( ioctl( MdpDrvFd(), MT_MDP_T_SETZOOM , &ioctlparam) < 0 )
    {
        MDP_ERROR("<0x%08X> MDP drv config zoom failed\n", (unsigned int)ioctlparam.mdp_id );
        return -1;
    }

    return 0;

#endif
    
}

int MdpDrv_ConfigZSDPreviewFrame( MdpDrvConfigZSDPreview_Param* pParam)
{
#ifdef MDP_FLAG_0_PLATFORM_LINUX
    stZSDPreviewReg ioctlparam;
    int ret;

    if( NULL == pParam )
    {
        MDP_ERROR("[ZSD] error param\n");
        return -1;
    }
    ioctlparam.bStopAfterZSDDone = pParam->bStopAfterZSDDone;
//CRZ
    ioctlparam.u4CRZLBMAX = pParam->u4CRZLBMAX;
    ioctlparam.u4CRZSrcSZ = pParam->u4CRZSrcSZ;
    ioctlparam.u4CRZTarSZ = pParam->u4CRZTarSZ;
    ioctlparam.u4CRZHRatio = pParam->u4CRZHRatio;
    ioctlparam.u4CRZVRatio = pParam->u4CRZVRatio;
    ioctlparam.u4CRZCropLR = pParam->u4CRZCropLR;
    ioctlparam.u4CRZCropTB = pParam->u4CRZCropTB;
//RGBROT0 descriptor
    for (int i=0; i<16; i++) {
        ioctlparam.u4RGB0Seg1[i] = pParam->u4RGB0Seg1[i];//y dst addr
    }
    ioctlparam.u4RGB0Seg4 = pParam->u4RGB0Seg4;//SrcW+SrcH
    ioctlparam.u4RGB0Seg5 = pParam->u4RGB0Seg5;//ClipW+ClipH
    ioctlparam.u4RGB0Seg6 = pParam->u4RGB0Seg6;//ClipX+ClipY
    ioctlparam.u4RGB0Seg7 = pParam->u4RGB0Seg7;//DstW in Bytes
    ioctlparam.u4RGB0Seg8 = pParam->u4RGB0Seg8;
    ioctlparam.u4RGB0Seg9 = pParam->u4RGB0Seg9;

    ret = ioctl( MdpDrvFd(), MT_MDP_T_SETZSDPREVIEWFRAME , &ioctlparam);

    if( ret < 0 )
    {
        MDP_ERROR("[ZSD] MDP drv config ZSD Last preview failed\n");
        return -1;
    }

    pParam->u4DescRemain = ioctlparam.u4DescRemain;
    pParam->u4DescCnt = ioctlparam.u4DescCnt;
    pParam->u4DescHwReadIndex= ioctlparam.u4DescHwReadIndex;
    pParam->u4DescHwWriteIndex= ioctlparam.u4DescHwWriteIndex;
    pParam->u4DescUpdateZone= ioctlparam.u4DescUpdateZone;

    return ret;
#endif
}

int MdpDrv_ConfigZSDZoom( MdpDrvConfigZSDZoom_Param* pParam )
{

#ifdef MDP_FLAG_0_PLATFORM_LINUX
    stZSDZoomReg ioctlparam;

    if( NULL == pParam )    return -1;

    ioctlparam.u4CRZLBMAX = pParam->u4CRZLBMAX;
    ioctlparam.u4CRZSrcSZ = pParam->u4CRZSrcSZ;
    ioctlparam.u4CRZTarSZ = pParam->u4CRZTarSZ;
    ioctlparam.u4CRZHRatio = pParam->u4CRZHRatio;
    ioctlparam.u4CRZVRatio = pParam->u4CRZVRatio;
    ioctlparam.u4CRZCropLR = pParam->u4CRZCropLR;
    ioctlparam.u4CRZCropTB = pParam->u4CRZCropTB;
//PRZ0
    ioctlparam.u4PRZ0LBMAX = pParam->u4PRZ0LBMAX;//+0x0 PRZ0_CFG
    ioctlparam.u4PRZ0SrcSZ = pParam->u4PRZ0SrcSZ;//+0x10 PRZ0_SRCSZ
    ioctlparam.u4PRZ0TarSZ = pParam->u4PRZ0TarSZ;//+0x14 PRZ0_TARSZ
    ioctlparam.u4PRZ0HRatio = pParam->u4PRZ0HRatio;//+0x18 PRZ0_HRATIO
    ioctlparam.u4PRZ0VRatio = pParam->u4PRZ0VRatio;//+0x1C PRZ0_VRATIO
//VRZ
    ioctlparam.u4VRZSrcSZ = pParam->u4VRZSrcSZ;//+0x10 VRZ_SRCSZ
    ioctlparam.u4VRZTarSZ = pParam->u4VRZTarSZ;//+0x14 VRZ_TARSZ
    ioctlparam.u4VRZHRatio = pParam->u4VRZHRatio;//+0x18 VRZ_HRATIO
    ioctlparam.u4VRZVRatio = pParam->u4VRZVRatio;//+0x1C VRZ_VRATIO
    ioctlparam.u4VRZHRes = pParam->u4VRZHRes;//+0x20 VRZ_HRES
    ioctlparam.u4VRZVRes = pParam->u4VRZVRes;//+0x24 VRZ_VRES
//VDOROT1 descriptor
    ioctlparam.u4VDO1Seg4 = pParam->u4VDO1Seg4;//SrcW+SrcH
    ioctlparam.u4VDO1Seg5 = pParam->u4VDO1Seg5;//ClipW+ClipH
    ioctlparam.u4VDO1Seg6 = pParam->u4VDO1Seg6;//ClipX+ClipY
    ioctlparam.u4VDO1Seg7 = pParam->u4VDO1Seg7;//DstW in Bytes

    if( ioctl( MdpDrvFd(), MT_MDP_T_SETZSDZOOM , &ioctlparam) < 0 )
    {
        MDP_ERROR("MDP drv config ZSD zoom failed\n");
        return -1;
    }

    return 0;

#endif
    
}


int MdpDrv_GetTimeStamp( stTimeStamp * p_timestamp )
{

#ifdef MDP_FLAG_0_PLATFORM_LINUX
    if( p_timestamp == NULL )    return -1;
    
    if( ioctl( MdpDrvFd(), MT_MDP_G_TIMESTAMP , p_timestamp) < 0 )
    {
        MDP_ERROR("Get timestamp  failed\n");
        return -1;
    }

    return 0;

#endif
    return -1;
    
}

/*
    float_offset :
        0x100 stands for 1, 0x40 stands for 0.25 , etc...
*/
static unsigned long _EisFloatToRegisterValue( unsigned long float_offset )
{
    //Sanity check
//    if(100 <= a_u4Offset)
    if(224 < float_offset)
    {
        MDP_ERROR("float_offset(%lu) overflow!\n", float_offset );
        return 3; //return max value
    }

    if(32 > float_offset)
    {
        //0~12.5
        return 0;
    }
    else if(96 > float_offset)
    {
        //12.5~37.5
        return 1;
    }
    else if(160 > float_offset)
    {
        //37.5~62.5
        return 2;
    }
    else
    {
        //62.5~87.5
        return 3;
    }
    	
    return 0;
}


unsigned long MdpDrv_RDMA_EIS_CON_Value( unsigned long eis_float_x, unsigned long eis_float_y )
{
    unsigned long reg_value;

    reg_value = ( _EisFloatToRegisterValue(eis_float_x) << 8 ) |
                ( _EisFloatToRegisterValue(eis_float_y) << 12);

    if( reg_value ){
        reg_value |= 0x11;
    }

    return reg_value;
}


int MdpDrv_Mdpk_BitbltWaitRequest( MDPIOCTL_MdpkBitbltConfig* pConfig )
{
    
#ifdef MDP_FLAG_0_PLATFORM_LINUX
        if( pConfig == NULL )    return -1;
        
        if( ioctl( MdpDrvFd(), MDPK_BITBLT_G_WAIT_REQUEST , pConfig) < 0 )
        {
            MDP_ERROR("IOCTL: MDPK_BITBLT_G_WAIT_REQUEST failed\n");
            return -1;
        }
    
        return 0;
#endif
        return -1;
        
    
}


int MdpDrv_Mdpk_BitbltInformDone( MDPIOCTL_MdpkBitbltInformDone* pDone )
{
    
#ifdef MDP_FLAG_0_PLATFORM_LINUX
        if( pDone == NULL )    return -1;
        
        if( ioctl( MdpDrvFd(), MDPK_BITBLT_T_INFORM_DONE , pDone) < 0 )
        {
            MDP_ERROR("IOCTL: MDPK_BITBLT_T_INFORM_DONE failed\n");
            return -1;
        }
    
        return 0;
#endif
        return -1;
    
}


unsigned long MdpDrv_Mdpk_MmapKernelMemory( unsigned long kernel_vmalloc_addr, unsigned long size )
{

#ifdef MDP_FLAG_0_PLATFORM_LINUX
    unsigned long user_address;

    user_address = (unsigned long)mmap( (void *)NULL , 
                                        (size_t)size, 
                                        (PROT_READ | PROT_WRITE) , 
                                        MAP_SHARED , 
                                        MdpDrvFd() , 
                                        (off_t)kernel_vmalloc_addr
                                        );

    return user_address;
#else
    return 0;
#endif


}

int MdpDrv_Mdpk_MunMapKernelMemory( unsigned long user_address, unsigned long size )
{
    
#ifdef MDP_FLAG_0_PLATFORM_LINUX
    if( munmap( (void*) user_address, (size_t) size ) != 0 )
    {
        MDP_ERROR("unmap kernel vmalloc address failed.\n");
        return -1;
    }
    return 0;
#else
    return 0;
#endif
}



static struct timeval  g_tv0[MDPDRV_WATCH_MAX_LEVEL],g_tv1[MDPDRV_WATCH_MAX_LEVEL];
static struct timezone g_tz0[MDPDRV_WATCH_MAX_LEVEL],g_tz1[MDPDRV_WATCH_MAX_LEVEL];

void MdpDrv_WatchStart( int level )
{
    gettimeofday( &g_tv0[level], &g_tz0[level] );
}

void MdpDrv_WatchStop(  int level,
                        const char* title_str,
                        unsigned long* p_total_time, 
                        unsigned long* p_frame_count,
                        unsigned long* p_avg_elapse_time,
                        unsigned long  reset_frame_count )
{
    unsigned long time_elapse;
    unsigned long avg_time_elapse;
    int   i;
    char  indent[MDPDRV_WATCH_MAX_LEVEL*2];
    
    gettimeofday( &g_tv1[level], &g_tz1[level] );


    indent[0] = '\0';
    for( i = 0; i < level; i++ ) {
        indent[i]='\t';
    }
    indent[i] = '\0';
    
    time_elapse = (g_tv1[level].tv_sec - g_tv0[level].tv_sec)*1000000 + (g_tv1[level].tv_usec - g_tv0[level].tv_usec);

    

    MDP_PROFILE("%s%s elapse time = %8luus/(avg = %luus)\n",indent,title_str, time_elapse, *p_avg_elapse_time );

    (*p_total_time) += time_elapse;
    (*p_frame_count)++;

    if( (*p_frame_count) == reset_frame_count )
    {
        (*p_avg_elapse_time) = (*p_total_time)/(*p_frame_count);
        (*p_total_time)  = 0;
        (*p_frame_count) = 0;
    }
}



unsigned long MdpDrv_GetTimeUs( void )
{
    struct timeval  tv;
    struct timezone tz;
    
    gettimeofday( &tv, &tz );

    return ( tv.tv_sec*1000000 + tv.tv_usec );
}


/*-----------------------------------------------------------------------------
    C++ version mdp drv watch
  -----------------------------------------------------------------------------*/
#if defined(MDP_FLAG_PROFILING)  

void MdpDrv_Watch::Start( int level )
{
    if( level > g_profile_level )
        return;
    
    gettimeofday( &tv0_, &tz0_ );
}

void MdpDrv_Watch::Stop(    int level, 
                            const char* title_str,
                            unsigned long* p_total_time, 
                            unsigned long* p_frame_count,
                            unsigned long* p_avg_elapse_time,
                            unsigned long  reset_frame_count )

{
    unsigned long time_elapse;
    unsigned long avg_time_elapse;
    int   i;
    char  indent[MDPDRV_WATCH_MAX_LEVEL*2];

    if( level > g_profile_level )
        return;
    
    
    gettimeofday( &tv1_, &tz1_ );


    indent[0] = '\0';
    for( i = 0; i < level; i++ ) {
        indent[i]='\t';
    }
    indent[i] = '\0';
    
    time_elapse = (tv1_.tv_sec - tv0_.tv_sec)*1000000 + (tv1_.tv_usec - tv0_.tv_usec);
    

    MDP_PROFILE("%s%s elapse time = %8luus/(avg = %luus)\n",indent,title_str, time_elapse, *p_avg_elapse_time );

    (*p_total_time) += time_elapse;
    (*p_frame_count)++;

    if( (*p_frame_count) == reset_frame_count )
    {
        (*p_avg_elapse_time) = (*p_total_time)/(*p_frame_count);
        (*p_total_time)  = 0;
        (*p_frame_count) = 0;
    }
}



#endif

void MdpDrv_DumpCallStack( const char* prefix_str )
{
    android::CallStack  stack;
        
    stack.update();

#if 0
    if( prefix_str != NULL )    {
        stack.dump( prefix_str );
    }
    else    {
        stack.dump("MDP");
    }
#endif
}

void MdpDrv_MvaLoopMemTrack( int client_id, MLMT_ACTION action, unsigned long size )
{
    MDPIOCTL_MvaLoopMemTrack param;

    param.in_client_id  = (int)client_id;   /*client id of register loop memory*/
    param.in_action     = action;      /*alloc or free?*/
    param.in_size       = size;
    param.in_tgid       = 0;

    
    if( ioctl( MdpDrvFd(), MT_MDP_T_MVALOOPMEMTRACK , &param) < 0 )
    {
        MDP_ERROR("IOCTL: MT_MDP_T_MVALOOPMEMTRACK failed\n");
        return;
    }
    
    int             in_client_id;   /*client id of register loop memory*/
    MLMT_ACTION     in_action;      /*alloc or free?*/
    unsigned long   in_size;
    unsigned long   in_tgid;

    long   out_statistic_alloc_count;
    long   out_statistic_free_count;
    long   out_statistic_count_balance;

    long   out_statistic_alloc_size;
    long   out_statistic_free_size;
    long   out_statistic_size_balance;


    
    MDP_INFO_MEM("[MVA Loop Mem]Client=%d action=%d size=%d alloc_#=%d free_#=%d bal_#=%d alloc_z=0x%08X free_z=0x%08X bal_z=0x%08X last_tgid=%d\n",
                    param.in_client_id,
                    
                    (int)param.in_action,
                    (int)param.in_size,
        
                    (int)param.out_statistic_alloc_count,
                    (int)param.out_statistic_free_count,
                    (int)param.out_statistic_count_balance,
                    (int)param.out_statistic_alloc_size,
                    (unsigned int)param.out_statistic_free_size,
                    (unsigned int)param.out_statistic_size_balance,
                    (int)param.in_tgid
                );
    
}


void MdpDrv_CpuClockGet( unsigned long *p_sec, unsigned long *p_usec )
{
    
    MDPIOCTL_CpuClockGet param;
    
    if( ioctl( MdpDrvFd(), MDPIO_T_CPUCLOCKGET  , &param) < 0 )
    {
        MDP_ERROR("IOCTL: MDPIO_T_CPUCLOCKGET  failed\n");
        return;
    }

    *p_sec = (unsigned long)param.sec;
    *p_usec  = (unsigned long)param.usec;

    
}


int MdpDrv_GetTaskStruct( unsigned long *p_m4u_handle, unsigned long *p_adj )
{
    
#ifdef  MDP_FLAG_1_SUPPORT_M4U

    unsigned long m4u_handle;
    
    if( MdpM4u_Init() < 0 ){
        MDP_ERROR("M4U Driver initial error\n");
        return -1;
    }

    
    return MdpM4u_GetTaskStruct( p_m4u_handle, p_adj  );


    //No need to release m4u driver
    //MdpM4u_Release( void )

#else

    return 0;
   
#endif
}



int MdpDrv_PutTaskStruct( unsigned long adj )
{
    
#ifdef  MDP_FLAG_1_SUPPORT_M4U
    
    unsigned long m4u_handle;

    if( MdpM4u_Init() < 0 ){
        MDP_ERROR("M4U Driver initial error\n");
        return -1;
    }


    return MdpM4u_PutTaskStruct( adj  );


    //No need to release m4u driver
    //MdpM4u_Release( void )

#else

    return 0;
       
#endif
}





