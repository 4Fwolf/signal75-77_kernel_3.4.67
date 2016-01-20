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
#include "mt_mdp.h"     //For MDP ID
#include "mdp_reg.h"   //For local test memory layout
#include <sys/mman.h>



#ifdef MDP_FLAG_0_PLATFORM_LINUX
//open syscall
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//close syscall
#include <unistd.h>
//strerror
#include <string.h>
#include <errno.h>
#endif


#ifdef MDP_FLAG_1_SUPPORT_SYSRAM
#include "camera_sysram.h"
#endif

/*/////////////////////////////////////////////////////////////////////////////
    Macro Re-Definition
  /////////////////////////////////////////////////////////////////////////////*/



/*/////////////////////////////////////////////////////////////////////////////
    MUTEX
  /////////////////////////////////////////////////////////////////////////////*/
#ifdef MDP_FLAG_1_SUPPORT_SYSRAM

static int g_sysram_device_driver_fd = -1; //MDP device driver node file descriptor

#if 0 /*Do not need this mutext anymore*/
pthread_mutex_t mutex_g_sysram_device_driver_fd = PTHREAD_MUTEX_INITIALIZER;

static void ENTER_CRITICAL_SYSRAM_FD( void )
{
    pthread_mutex_lock ( &mutex_g_sysram_device_driver_fd );
    MDP_INFO("SYSRAM FD MUTEX LOCK\n\r");
}

static void EXIT_CRITICAL_SYSRAM_FD( void )
{
    MDP_INFO("SYSRAM FD MUTEX UN-LOCK\n\r");
    pthread_mutex_unlock ( &mutex_g_sysram_device_driver_fd );
}
#endif

#endif


/*/////////////////////////////////////////////////////////////////////////////
    Local Function
  /////////////////////////////////////////////////////////////////////////////*/
#ifdef MDP_FLAG_1_SUPPORT_SYSRAM
static ESysramUser_T _MdpId_to_ESysramUser( unsigned long mdp_id , MDPSYSRAM_SUBCAT sub_category )
{
    /*-----------------------------------------------------------------------------
        for Normal SYSRAM Allocate    
      -----------------------------------------------------------------------------*/
    if( sub_category == MDPSYSRAM_SUBCAT_NORMAL)
    {
        switch( mdp_id )
        {
        case MID_R_DMA0:
            return ESysramUser_RDMA0;
            break;
            
        case MID_VRZ0:
            return ESysramUser_VRZ0;
            break;
            
        case MID_VRZ1:
            return ESysramUser_VRZ1;
            break;

        case MID_RGB_ROT0:
            return ESysramUser_RGB_ROT0;
            break;
            
        case MID_RGB_ROT1:
            return ESysramUser_RGB_ROT1;
            break;
        case MID_RGB_ROT2:
            return ESysramUser_RGB_ROT2;
            break;
//#ifdef MDP_DRIVER_DVT_SUPPORT      
        case MID_VDO_ROT0:
        		MDP_WARNING("VDO_ROT0 share the same sysram with VDO_ROT1");
//#endif    
        case MID_VDO_ROT1:
            return ESysramUser_VDO_ROT1;
            break;

        case MID_BRZ:
            return ESysramUser_BRZ;
            break;

        case MID_JPEG_DMA:
            return ESysramUser_JPEG_DMA;
            break;

        default:
            MDP_ERROR("unsupport ESysramUser_T for <0x%X>\n", (unsigned int)mdp_id );
            return ESysramUser_None;
        }

    }
    
    /*-----------------------------------------------------------------------------
        for Descriptor mode SYSRAM Allocate       
      -----------------------------------------------------------------------------*/
    else if( sub_category == MDPSYSRAM_SUBCAT_DESCRIPTOR )
    {
        switch( mdp_id )
        {
        case MID_R_DMA0:
            return ESysramUser_DESC_RDMA0;
            break;
        case MID_R_DMA1:
            return ESysramUser_DESC_RDMA1;
            break;
        case MID_RGB_ROT0:
            return ESysramUser_DESC_RGB_ROT0;
            break;
        case MID_RGB_ROT1:
            return ESysramUser_DESC_RGB_ROT1;
            break;
        case MID_RGB_ROT2:
            return ESysramUser_DESC_RGB_ROT2;
            break;
        case MID_VDO_ROT0:
            return ESysramUser_DESC_VDO_ROT0;
            break;
        case MID_VDO_ROT1:
            return ESysramUser_DESC_VDO_ROT1;
            break;

        default:
            MDP_ERROR("unsupport Descriptor ESysramUser_T for <0x%X>\n", (unsigned int)mdp_id );
            return ESysramUser_None;
        
        }
    }
    /*-----------------------------------------------------------------------------
        for ROT sub sysram
    -----------------------------------------------------------------------------*/
    else if( sub_category == MDPSYSRAM_SUBCAT_SUBROTSYSRAM )
    {
        switch( mdp_id )
        {
        case MID_VDO_ROT1:
            return ESysramUser_VDO_ROT1_SUB;
            break;

        default:
            MDP_ERROR("unsupport ROT sub sysram ESysramUser_T for <0x%X>\n", (unsigned int)mdp_id );
            return ESysramUser_None;
        
        }
    }
    else
    {
        MDP_ERROR("unsupport sub_category %d\n", sub_category );
        return ESysramUser_None;
    }
    
    
}
#endif //MDP_FLAG_1_SUPPORT_SYSRAM

/*/////////////////////////////////////////////////////////////////////////////
    Local SYSRAM Allocate System ( Only for test)
  /////////////////////////////////////////////////////////////////////////////*/
static unsigned long _LocalSysram_Alloc_Test( unsigned long id , unsigned long u4Size, MDPSYSRAM_SUBCAT sub_category )
{
    unsigned long u4Result = 0;

    if( sub_category == MDPSYSRAM_SUBCAT_NORMAL )
    {
        switch(id)
        {
            case MID_JPEG_DEC:
            case MID_VDO_ROT0 :
                //u4Result = (u4Size > (MT6575_BANK0_SIZE >> 1) ? 0 : MT6575_BANK0_ADDR);
                //u4Result = (MT6575_BANK2_ADDR + 0x7000); //CLS:GIPI Original
                u4Result = (MT6575_BANK0_ADDR + 0x14000);    //CLS: Use memory after 80K
            break;

            case MID_RGB_ROT0 :
                u4Result = (MT6575_BANK2_ADDR);
    		break;

            case MID_VRZ1 :
                MDP_WARNING("CLS:IMPLEMENTATION NEED TO BE CONFIRMED!!!\n\r");
            case MID_VRZ0 :
                //u4Result = MT6575_BANK1_ADDR;     //CLS:Gipi version
                u4Result = (MT6575_BANK0_ADDR + 0x14000) + 0x2000;     //CLS:Gipi version,off set 2K
                
    		break;

            case MID_VDO_ROT1 :
            case MID_JPEG_DMA:
            case MID_BRZ:
            case MID_RGB_ROT1 :
            case MID_RGB_ROT2 :
    		case MID_TV_ROT :
                u4Result = MT6575_BANK0_ADDR;
            break;

            default :
                u4Result = 0;
            break;
        }

        MDP_INFO("Owner %lu use %lu sysram from address 0x%x-0x%x \n\r" , (unsigned long)id , u4Size , (unsigned int)u4Result, (unsigned int)(u4Result+u4Size-1));
        
    }
    else if( sub_category == MDPSYSRAM_SUBCAT_DESCRIPTOR )
    {
        #define Base_Bank2_Temp              (0xC2000000 + 0x18000 + 0x8000 )
        
        switch( id )
        {
            case MID_R_DMA0:
                u4Result = Base_Bank2_Temp + 0xC0 * 0;
                break;
            case MID_R_DMA1:
                u4Result = Base_Bank2_Temp + 0xC0 * 1;
                break;
            case MID_RGB_ROT0:
                u4Result = Base_Bank2_Temp + 0xC0 * 2;
                break;
            case MID_RGB_ROT1:
                u4Result = Base_Bank2_Temp + 0xC0 * 3;
                break;
            case MID_RGB_ROT2:
                u4Result = Base_Bank2_Temp + 0xC0 * 4;
                break;
            case MID_VDO_ROT0:
                u4Result = Base_Bank2_Temp + 0xC0 * 5;
                break;
            case MID_VDO_ROT1:
                u4Result = Base_Bank2_Temp + 0xC0 * 6;
                break;
            default :
                u4Result = 0;
        }

        
    }
    else
    {
        MDP_ERROR("Sub rot sysram allocate is not implement in test mode\n");
    }

    return u4Result;
}

static int _LocalSysram_Free_Test( unsigned long id , MDPSYSRAM_SUBCAT sub_category )
{
    return 0;
}



/*/////////////////////////////////////////////////////////////////////////////
    SYSRAM Device Driver API
  /////////////////////////////////////////////////////////////////////////////*/

int MdpSysram_Init( void )
{
    int ret_val = 0;
    
    
#ifdef MDP_FLAG_1_SUPPORT_SYSRAM
    /*Call by MdpDrvInit, already in critical section*/
    //ENTER_CRITICAL_SYSRAM_FD(); /*Critical Section Enter--------------------------------------------*/

    if( g_sysram_device_driver_fd == -1 )
    {
        g_sysram_device_driver_fd = open("/dev/camera-sysram", O_RDWR);

        if(-1 == g_sysram_device_driver_fd)
        {
            MDP_ERROR("Open SYSRAM driver file failed :%s\n\r", strerror(errno));
            ret_val = -1;
        }
    }

    //EXIT_CRITICAL_SYSRAM_FD(); /*Critical Section Exit--------------------------------------------*/
#endif //MDP_FLAG_1_SUPPORT_SYSRAM

    return ret_val;
    
}


unsigned long   MdpSysram_Alloc( unsigned long id , unsigned long u4Size , MDPSYSRAM_SUBCAT sub_category  )
{
#ifdef MDP_FLAG_1_SUPPORT_SYSRAM
    #if defined(MDP_FLAG_PROFILING)
            MdpDrv_Watch _MdpDrvWatch;
            static unsigned long tt0;//total time;
            static unsigned long fc0;//frame count;
            static unsigned long avt0;//avg_time_elapse;
    #endif

    stSysramParam SysramPara;

    //SysramPara.u4Alignment      = 4;
    SysramPara.u4Alignment      = 8;
    SysramPara.u4TimeoutInMS    = 100;
    SysramPara.u4Size           = u4Size;
    SysramPara.u4Addr           = 0;
    SysramPara.u4Owner          = _MdpId_to_ESysramUser( id, sub_category );

    
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_DRV );

    ioctl( g_sysram_device_driver_fd, SYSRAM_X_USRALLOC_TIMEOUT, &SysramPara);

    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_DRV, "sysram alloc", &tt0, &fc0, &avt0, 30 );

    if( SysramPara.u4Addr == NULL ){
        MDP_ERROR("Sysram allocate failed.<0x%08X:%d> try to allocate size = %lu\n", (unsigned int)id, sub_category, u4Size);
    }

    
    MDP_INFO_MEM("[SYSRAM]<0x%08X:%d> allocate sysram size = %lu\n", (unsigned int)id, sub_category, u4Size );

    

    return SysramPara.u4Addr;

#else /*MDP_FLAG_1_SUPPORT_SYSRAM*/
    
    return _LocalSysram_Alloc_Test( id , u4Size , sub_category );

#endif /*!MDP_FLAG_1_SUPPORT_SYSRAM*/
}




int             MdpSysram_Free(  unsigned long id , unsigned long addr, MDPSYSRAM_SUBCAT sub_category )
{

#ifdef MDP_FLAG_1_SUPPORT_SYSRAM
    #if defined(MDP_FLAG_PROFILING)
            MdpDrv_Watch _MdpDrvWatch;
            static unsigned long tt0;//total time;
            static unsigned long fc0;//frame count;
            static unsigned long avt0;//avg_time_elapse;
    #endif

    stSysramParam SysramPara;
    int ret_val = 0;

    SysramPara.u4Alignment      = 4;
    SysramPara.u4TimeoutInMS    = 100;
    SysramPara.u4Size           = 0;
    SysramPara.u4Addr           = addr;
    SysramPara.u4Owner          = _MdpId_to_ESysramUser( id, sub_category );

    
    MDPDRV_WATCHSTART( MDPDRV_WATCH_LEVEL_DRV );

    if( addr != 0 )
    {
        ret_val = ioctl(g_sysram_device_driver_fd, SYSRAM_S_USRFREE, &SysramPara);
    }
    
    MDPDRV_WATCHSTOP(  MDPDRV_WATCH_LEVEL_DRV, "sysram free", &tt0, &fc0, &avt0, 30 );

    return ret_val;

    
    
#else /*MDP_FLAG_1_SUPPORT_SYSRAM*/

    return _LocalSysram_Free_Test( id , sub_category );

#endif /*!MDP_FLAG_1_SUPPORT_SYSRAM*/

}



int MdpSysram_Release( void )
{
    int ret_val = 0;

#ifdef MDP_FLAG_1_SUPPORT_SYSRAM
    /*Call by MdpDrvRelease, already in critical section*/
    //ENTER_CRITICAL_SYSRAM_FD(); /*Critical Section Enter--------------------------------------------*/
        close(g_sysram_device_driver_fd);
        g_sysram_device_driver_fd = -1;
    //EXIT_CRITICAL_SYSRAM_FD(); /*Critical Section Exit--------------------------------------------*/

#endif //MDP_FLAG_1_SUPPORT_SYSRAM

    return ret_val;
}

#define SYSRAM_BASE             (0xF2000000)
#define MM_SYSRAM_BASE_PA       (0xC2000000)
#define MM_SYSRAM_SIZE          (0x40000)

unsigned long MdpSysram_Remap( void )
{
    int ret_val = 0;
    unsigned long remap_addr = 0;

#ifdef MDP_FLAG_1_SUPPORT_SYSRAM

    if (g_sysram_device_driver_fd == -1)
    {
        MDP_ERROR("[SYSRAM] Remap failed\n");
        return 0;
    }

    remap_addr = (unsigned long)(::mmap(
        NULL,
        MM_SYSRAM_SIZE,
        (PROT_READ | PROT_WRITE),
        MAP_SHARED,
        g_sysram_device_driver_fd,
        (off_t)(MM_SYSRAM_BASE_PA)
    ));

    if  ( 0 == remap_addr )
    {
        MDP_ERROR("[SYSRAM] Remap failed\n");
        //remap_addr = 0;
    }
#endif //MDP_FLAG_1_SUPPORT_SYSRAM

    return  remap_addr;
}
