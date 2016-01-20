/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/* 
 * data path: virtual memory -> m4u -> LCDC_R
 * LCD_R read BufAddr through M4U, then LCD_W write the data to PMEM PA 
 * test APP dump PMEM_VA image to verify
 */

#include "stdio.h"
#include "errno.h"
#include "fcntl.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <cutils/log.h>
#include "MediaHal.h"
#include <m4u_lib.h>

#undef LOG_TAG
#define LOG_TAG "[m4u_ut]"

#define MTKM4UDEBUG
#ifdef MTKM4UDEBUG
  #define M4UDBG(string, args...) printf("[M4U_N]"string,##args)
#else
  #define M4UDBG(string, args...)
#endif

void MAUDebugCommand(char* cmd)
{
    FILE* fp_dbg = fopen("/d/mau", "w");
    if (fp_dbg)
    {
        fprintf(fp_dbg, "%s", cmd);
        fclose(fp_dbg);
    }
}

void DebugCommand(int i)
{
    FILE* fp_dbg = fopen("/d/mau", "w");
    if (fp_dbg)
    {
        fprintf(fp_dbg, "m4u_debug:%d", i);
        fclose(fp_dbg);
    }
}

int main (int argc, char *argv[])
{
    M4UDBG("enter m4u gcov ut main \n");
    
    unsigned int i;
    MTKM4UDrv CM4u;
    M4UDBG("argc=%d \n", argc);
    for(i=0;i<argc;i++)
    {
    	  M4UDBG("argv[%d]=%s \n", i, argv[i]);
    }
        
    unsigned int BufSize = 4096;
    unsigned int BufAddr = (unsigned int)new unsigned char[BufSize];
    unsigned int BufMVA;

    FILE* fp_dbg = fopen("/d/mau", "w");
    if (!fp_dbg)
    {
        M4UDBG("Cannot open /d/mau\r\n");
        return 0;
    }
    MAUDebugCommand("m4u_log:on");
    MAUDebugCommand("m4u_monitor:on");
    DebugCommand(0);
    DebugCommand(1);
    DebugCommand(2);
    DebugCommand(5);
    DebugCommand(7);

    CM4u.m4u_monitor_start(M4U_PORT_DEFECT);
    M4UDBG("Now run multimedia test cases manually... ");
    scanf("%d", &i);
    M4UDBG("Done.\r\n");
    DebugCommand(9);
    DebugCommand(8);
    for (i=10; i<20; i++)
        DebugCommand(i);

    pid_t pid;
    pid = fork();
    if (pid == 0)
    {
        mHalBltParam_t stParam;
        char* src_va = new char[400*400*3];
        char* dst_va = new char[400*400*3];
        memset(&stParam, 0, sizeof(mHalBltParam_t));
        stParam.srcX = 0;
        stParam.srcY = 0;
        stParam.srcW = 400;
        stParam.srcWStride = 400;
        stParam.srcH = 400;
        stParam.srcHStride = 400;
        stParam.srcAddr = (MUINT32)src_va;
        stParam.srcFormat = MHAL_FORMAT_RGB_888;
        stParam.dstW = 400;
        stParam.dstH = 400;
        stParam.dstAddr = (MUINT32)dst_va;
        stParam.dstFormat = MHAL_FORMAT_RGB_888;
        stParam.pitch = 400;
        stParam.orientation = 1;
        stParam.doImageProcess = 1;
        mHalIoCtrl(MHAL_IOCTL_BITBLT, &stParam, sizeof(stParam), NULL, 0, NULL);
        return 0;
    }
    // allocate MVA
    CM4u.m4u_power_on(M4U_CLNTMOD_DEFECT);
    // allocate MVA buffer for LCDC module                    
    M4UDBG("Allocate MVA... ");
    for (i=0; i<(int)M4U_CLNTMOD_MAX; i++)
    {
        CM4u.m4u_alloc_mva((M4U_MODULE_ID_ENUM)i,     // Module ID
                           BufAddr,              // buffer virtual start address
                           BufSize,              // buffer size
                           &BufMVA);             // return MVA address
        CM4u.m4u_manual_insert_entry((M4U_MODULE_ID_ENUM)i, 
                                 BufMVA,   // MVA address
                                 true);    // lock the entry for circuling access
        CM4u.m4u_manual_insert_entry((M4U_MODULE_ID_ENUM)i, 
                                 BufMVA,   // MVA address
                                 true);    // lock the entry for circuling access
    }
    M4UDBG("Invalid tlb... ");
    CM4u.m4u_invalid_tlb_range(M4U_CLNTMOD_DEFECT, BufMVA, BufMVA+BufSize-1);
    CM4u.m4u_invalid_tlb_all(M4U_CLNTMOD_DEFECT);
    M4UDBG("Done.\r\n");
    CM4u.m4u_alloc_mva(M4U_CLNTMOD_DEFECT,     // Module ID
                           BufAddr,              // buffer virtual start address
                           BufSize,              // buffer size
                           &BufMVA);             // return MVA address
    M4UDBG("Done BufMVA=0x%x \r\n", BufMVA);
    // cache sync
    M4UDBG("Cache sync... ");
    CM4u.m4u_cache_sync(M4U_CLNTMOD_DEFECT, M4U_CACHE_FLUSH_BEFORE_HW_READ_MEM, BufAddr, BufSize);
    CM4u.m4u_cache_sync(M4U_CLNTMOD_DEFECT, M4U_CACHE_FLUSH_BEFORE_HW_WRITE_MEM, BufAddr, BufSize);
    CM4u.m4u_cache_sync(M4U_CLNTMOD_DEFECT, M4U_CACHE_CLEAN_BEFORE_HW_READ_MEM, BufAddr, BufSize);
    CM4u.m4u_cache_sync(M4U_CLNTMOD_DEFECT, M4U_CACHE_INVALID_AFTER_HW_WRITE_MEM, BufAddr, BufSize);
    CM4u.m4u_cache_flush_all(M4U_CLNTMOD_DEFECT);
    M4UDBG("Done.\r\n");
    
    // insert tlb range and tlb entry
    // manual insert MVA start page address
    M4UDBG("Done.\r\n");

    // insert TLB uni-update range
    M4UDBG("Insert TLB range... ");
    CM4u.m4u_insert_tlb_range(M4U_CLNTMOD_DEFECT, 
                              BufMVA,                // range start MVA
                              BufMVA + BufSize - 1,  // range end MVA
                              RT_RANGE_HIGH_PRIORITY,
                              1);
    M4UDBG("Done.\r\n");
    M4UDBG("Insert TLB wrapped range... ");
    CM4u.m4u_insert_wrapped_range(M4U_CLNTMOD_DEFECT, 
                                  M4U_PORT_DEFECT, 
                                  BufMVA, 
                                  BufMVA + BufSize - 1);
    M4UDBG("Done.\r\n");

    M4UDBG("Invalid tlb... ");
    CM4u.m4u_invalid_tlb_range(M4U_CLNTMOD_DEFECT, BufMVA, BufMVA+BufSize-1);
    CM4u.m4u_invalid_tlb_all(M4U_CLNTMOD_DEFECT);
    M4UDBG("Done.\r\n");


    M4UDBG("Dump info... ");
    CM4u.m4u_dump_reg(M4U_CLNTMOD_DEFECT);
    CM4u.m4u_dump_info(M4U_CLNTMOD_DEFECT);
    CM4u.m4u_dump_pagetable(M4U_CLNTMOD_DEFECT,     // Module ID
                       BufAddr,              // buffer virtual start address
                       BufSize,              // buffer size
                       BufMVA);
    M4UDBG("Done.\r\n");

    // config port                       
    M4UDBG("Config port... ");
    M4U_PORT_STRUCT M4uPort;
    M4uPort.ePortID = M4U_PORT_DEFECT;
    M4uPort.Virtuality = 1;						   
    M4uPort.Security = 0;
    M4uPort.Distance = 1;
    M4uPort.Direction = 0;
    M4uPort.ePortID = M4U_PORT_DEFECT;
    CM4u.m4u_config_port(&M4uPort);

    M4U_PORT_STRUCT_ROTATOR rot_config;
    rot_config.ePortID = M4U_PORT_TV_ROT_OUT0;
    rot_config.Virtuality = 0;						   
    rot_config.Security = 0;
    rot_config.angle = (M4U_ROTATOR_ENUM)0;
    CM4u.m4u_config_port_rotator(&rot_config);
    rot_config.Virtuality = 1;						   
    CM4u.m4u_config_port_rotator(&rot_config);
    rot_config.angle = (M4U_ROTATOR_ENUM)90;
    CM4u.m4u_config_port_rotator(&rot_config);
    M4UDBG("Done.\r\n");

    CM4u.m4u_monitor_stop(M4U_PORT_DEFECT);

    CM4u.m4u_power_off(M4U_CLNTMOD_DEFECT);

    MAUDebugCommand("m4u_log:off");
    MAUDebugCommand("m4u_monitor:off");

    M4UDBG("Following steps:");
    M4UDBG("1. Plug out USB. Suspend target and resume. ");
    M4UDBG("2. Run rmmod /system/modules/m4u.ko ");
    return 0;
}


