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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
//#include <syslog.h>
#include <string.h>

#include "mdp_datatypes.h"
#include "mdp_drv.h"
#include "mhal_interface.h"
#include "MediaHal.h"       //For MHAL color format /mHalBltParam_t




typedef struct
{
    unsigned long       src_addr;
    unsigned long       src_size;
    MDPK_BITBLT_MEMTYPE src_memtype;
    unsigned long       dst_addr;
    unsigned long       dst_size;
    MDPK_BITBLT_MEMTYPE dst_memtype;

} MdpdUserBufferInfo;

/*Data structure to remember buffer user space address taht map from kernel space*/
MdpdUserBufferInfo  g_MdpdUsrBuffAddr[ MDPKBITBLIT_CHNL_COUNT ];

static MHAL_BITBLT_FORMAT_ENUM   MdpkColorToMhalColor( MDPK_BITBLT_FORMAT_ENUM mdpk_color )
{
    switch( mdpk_color )
    {
        case MDPK_FORMAT_RGB_565 :
            return MHAL_FORMAT_RGB_565;
        break;
        case MDPK_FORMAT_RGB_888 :
            return MHAL_FORMAT_RGB_888;
        break;
        case MDPK_FORMAT_ARGB_8888 :
            return MHAL_FORMAT_ARGB_8888;
        break;
        case MDPK_FORMAT_ABGR_8888 :
            return MHAL_FORMAT_ABGR_8888;
        break;
        case MDPK_FORMAT_YUV_420 :
            return MHAL_FORMAT_YUV_420; 
        break;        
        case MDPK_FORMAT_YUV_420_SP :
            return MHAL_FORMAT_YUV_420_SP;
        break;
        case MDPK_FORMAT_MTK_YUV :
            return MHAL_FORMAT_MTK_YUV;
        break;        
        case MDPK_FORMAT_YUY2 :
            return MHAL_FORMAT_YUY2;
        case MDPK_FORMAT_UYVY :
            return MHAL_FORMAT_UYVY;
        default :
     	     MDP_ERROR("Unsupport mdpk bitblt color format 0x%x" , mdpk_color );
             return (MHAL_BITBLT_FORMAT_ENUM)-1;
        break;
    }

    return (MHAL_BITBLT_FORMAT_ENUM)-1;
    
}



void Do_MdpdInit( void )
{
    memset( g_MdpdUsrBuffAddr, 0, sizeof( g_MdpdUsrBuffAddr ) );
}

int Do_MdpdBitblt( void )
{
    int ret = 0;
    int channel;
    int bitblt_ret = 0;
    MDPIOCTL_MdpkBitbltConfig   config;
    unsigned long   new_usr_src_addr;
    unsigned long   new_usr_dst_addr;

    /*I. Wait request*/   
    MDP_INFO_MDPD("MDPD Wait Request.....\n");
    if( MdpDrv_Mdpk_BitbltWaitRequest( &config ) != 0 )
    {
        MDP_ERROR("Wait Request Failed\n");
        return -1;
    }

    channel = config.out_channel;

    if( channel >= MDPKBITBLIT_CHNL_COUNT ){
        MDP_ERROR("MDPD Channel(%d) exceed (%d).\n", channel, MDPKBITBLIT_CHNL_COUNT-1 );
        return -1;
    }

#if 0        
    /*II. mmap address if dirty*/
    /*mmap src address*/
    if( config.out_b_src_addr_dirty )
    {
        //1.Unmap previous user address if necessary
        if( ( g_MdpdUsrBuffAddr[channel].src_addr != 0 ) && ( g_MdpdUsrBuffAddr[channel].src_memtype == MDPK_MEMTYPE_PHYMEM ) ){
            if( MdpDrv_Mdpk_MunMapKernelMemory( g_MdpdUsrBuffAddr[channel].src_addr, g_MdpdUsrBuffAddr[channel].src_size ) != 0 ){
                MDP_ERROR("MDPD unmap kernel src address failed (0x%08X)\n", (unsigned int)g_MdpdUsrBuffAddr[channel].src_addr );
            }
            g_MdpdUsrBuffAddr[channel].src_addr = 0;
        }

        //2.Map new user address if necessary
        if( config.out_config.srcMemType == MDPK_MEMTYPE_PHYMEM ){
            new_usr_src_addr = MdpDrv_Mdpk_MmapKernelMemory( config.out_config.srcAddr, config.out_config.srcBufferSize  );

            if( new_usr_src_addr == -1 )
            {
                MDP_ERROR("MDPD map kernel src address failed (0x%08X)\n", (unsigned int)config.out_config.srcAddr );
                return -1;
            }
        } else {
            new_usr_src_addr = config.out_config.srcAddr;
        }

        //3.Remember new address
        g_MdpdUsrBuffAddr[channel].src_addr   = new_usr_src_addr;
        g_MdpdUsrBuffAddr[channel].src_size   = config.out_config.srcBufferSize;
        g_MdpdUsrBuffAddr[channel].src_memtype= config.out_config.srcMemType;
        
    }
    

    /*mmap dst address*/
    if( config.out_b_dst_addr_dirty )
    {
        //1.Unmap previous user address if necessary
        if( ( g_MdpdUsrBuffAddr[channel].dst_addr != 0 ) && ( g_MdpdUsrBuffAddr[channel].dst_memtype == MDPK_MEMTYPE_PHYMEM ) ){
            if( MdpDrv_Mdpk_MunMapKernelMemory( g_MdpdUsrBuffAddr[channel].dst_addr, g_MdpdUsrBuffAddr[channel].dst_size ) != 0 ){
                MDP_ERROR("MDPD unmap kernel dst address failed (0x%08X)\n", (unsigned int)g_MdpdUsrBuffAddr[channel].dst_addr );
            }
            g_MdpdUsrBuffAddr[channel].dst_addr = 0;
        }

        //2.Map new user address if necessary
        if( config.out_config.dstMemType == MDPK_MEMTYPE_PHYMEM ){
            new_usr_dst_addr = MdpDrv_Mdpk_MmapKernelMemory( config.out_config.dstAddr, config.out_config.dstBufferSize  );

            if( new_usr_dst_addr == -1 )
            {
                MDP_ERROR("MDPD map kernel dst address failed (0x%08X)\n", (unsigned int)config.out_config.dstAddr );
                return -1;
            }
        } else {
            new_usr_dst_addr = config.out_config.dstAddr;
        }

        //3.Remember new address
        g_MdpdUsrBuffAddr[channel].dst_addr   = new_usr_dst_addr;
        g_MdpdUsrBuffAddr[channel].dst_size   = config.out_config.dstBufferSize;
        g_MdpdUsrBuffAddr[channel].dst_memtype= config.out_config.dstMemType;
        
    }
    
#endif
        /* Show Info efore do bitblt*/
        {
            MDP_INFO_PATH("[Mdpk Bitblt Request]----------------------------------\n");
            MDP_INFO("channel = %d\n", (int)config.out_channel );
            MDP_INFO("src dirty = %d\n", (int)config.out_b_src_addr_dirty );
            MDP_INFO("dst dirty = %d\n", (int)config.out_b_dst_addr_dirty );
            
            MDP_INFO("srcX = %d\n", (int)config.out_config.srcX);
            MDP_INFO("srcY = %d\n", (int)config.out_config.srcY);
            MDP_INFO("srcW = %d\n", (int)config.out_config.srcW);
            MDP_INFO("srcWStride = %d\n", (int)config.out_config.srcWStride);
            MDP_INFO("srcH = %d\n", (int)config.out_config.srcH);
            MDP_INFO("srcHStride = %d\n", (int)config.out_config.srcHStride);
            MDP_INFO("srcAddr = 0x%08X(kernel) => 0x%08X(usr)\n", (unsigned int)config.out_config.srcAddr, (unsigned int)g_MdpdUsrBuffAddr[channel].src_addr);
            MDP_INFO("srcFormat = %d\n", (int)config.out_config.srcFormat);
            MDP_INFO("srcBufferSize = %d\n", (int)config.out_config.srcBufferSize); //Note:Kernel Mdpk Api Only
            MDP_INFO("srcMemType = %d\n", (int)config.out_config.srcMemType); //Note:Kernel Mdpk Api Only

            MDP_INFO("dstW = %d\n", (int)config.out_config.dstW);
            MDP_INFO("dstH = %d\n", (int)config.out_config.dstH);
            MDP_INFO("dstAddr = 0x%08X(kernel) => 0x%08X(usr)\n", (unsigned int)config.out_config.dstAddr, (unsigned int)g_MdpdUsrBuffAddr[channel].dst_addr);
            MDP_INFO("dstFormat = %d\n", (int)config.out_config.dstFormat);
            MDP_INFO("pitch = %d\n", (int)config.out_config.pitch);
            MDP_INFO("dstBufferSize = %d\n", (int)config.out_config.dstBufferSize); //Note:Kernel Mdpk Api Only
            MDP_INFO("dstMemType = %d\n", (int)config.out_config.dstMemType); //Note:Kernel Mdpk Api Only

            MDP_INFO("orientation = %d\n", (int)config.out_config.orientation);
            MDP_INFO("doImageProcess = %d\n", (int)config.out_config.doImageProcess);

            MDP_INFO("u4SrcOffsetXFloat = %d\n", (int)config.out_config.u4SrcOffsetXFloat);//0x100 stands for 1, 0x40 stands for 0.25 , etc...
            MDP_INFO("u4SrcOffsetYFloat = %d\n", (int)config.out_config.u4SrcOffsetYFloat);//0x100 stands for 1, 0x40 stands for 0.25 , etc...
            
        }


    /*III. Invoke user mode MDP bitblt*/
    {
        mHalBltParam_t bltParam;

        memset( &bltParam, 0, sizeof(bltParam) );
        
        bltParam.srcX = config.out_config.srcX;
        bltParam.srcY = config.out_config.srcY;
        bltParam.srcW = config.out_config.srcW;
        bltParam.srcWStride = config.out_config.srcWStride;
        bltParam.srcH = config.out_config.srcH;
        bltParam.srcHStride = config.out_config.srcHStride;
        //bltParam.srcAddr = g_MdpdUsrBuffAddr[channel].src_addr; //Note!Use use space address
        bltParam.srcAddr = config.out_config.srcAddr;
        bltParam.srcFormat = MdpkColorToMhalColor( config.out_config.srcFormat );
        
        bltParam.dstW = config.out_config.dstW;
        bltParam.dstH = config.out_config.dstH;
        //bltParam.dstAddr = g_MdpdUsrBuffAddr[channel].dst_addr; //Note!Use use space address;
        bltParam.dstAddr = config.out_config.dstAddr;

        //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
        bltParam.dstFormat = MdpkColorToMhalColor( config.out_config.dstFormat );

        bltParam.pitch = config.out_config.pitch;

        bltParam.orientation    = config.out_config.orientation; //TODO: test 5
        bltParam.doImageProcess = config.out_config.doImageProcess;

        bitblt_ret = Mt6575_mHalBitblt( &bltParam );
        //bitblt_ret = mHalIoCtrl( MHAL_IOCTL_BITBLT, &bltParam, sizeof(mHalBltParam_t), NULL, 0, NULL); 
 
        if( MHAL_NO_ERROR != bitblt_ret ) 
        {
            MDP_ERROR("MDPD bitblt() cannot do bitblt operation\n");
            ret = -1;
        }
    
    }

    /*IV. Inform Done*/
    {
        MDPIOCTL_MdpkBitbltInformDone done;

        done.in_channel = channel;
        done.in_ret_val = bitblt_ret;
        
        if( MdpDrv_Mdpk_BitbltInformDone( &done ) < 0 )
        {
            MDP_ERROR("MDPD inform done failed\n");
            ret = -1;
        }
    }


    return ret;

}


int main(void) 
{

    /* Our process ID and Session ID */
    pid_t pid, sid;

    #if 0
    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
            MDP_ERROR("Fork error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0) {
            MDP_INFO("I'm parent.Child PID = %d , Go to exit\n",pid );
            exit(EXIT_SUCCESS);
    }

    MDP_INFO("I'm child.PID = %d\n",pid );
    #endif

    /* Change the file mode mask */
    umask(0);
            
    /* Open any logs here */        

    #if 0
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
            /* Log the failure */
            MDP_ERROR("setsid error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
    }
    #endif



    /* Change the current working directory */
    if ((chdir("/")) < 0) {
            /* Log the failure */
            MDP_ERROR("chdir error:%s\n", strerror(errno));
            exit(EXIT_FAILURE);
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    /* Daemon-specific initialization goes here */

    /*Initial*/
    Do_MdpdInit();


    /* The Big Loop */
    MdpDrvInit();
    while (1) 
    {
       /* Do some task here ... */
       Do_MdpdBitblt();
       
       //MDP_INFO("Sleep...\n");
       //sleep(30); /* wait 30 seconds */
    }
    MdpDrvRelease();
    
    exit(EXIT_SUCCESS);
    
}    
