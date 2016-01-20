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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <cutils/log.h>

#include "mt6575_spc.h"


#undef LOG_TAG
#define LOG_TAG "[spc_ut]"

#define MTKM4UDEBUG
#ifdef MTKM4UDEBUG
  #define SPCDBG(string, args...) printf("[M4U_N]"string,##args)
#else
  #define SPCDBG(string, args...)
#endif

//#define M4U_MEM_USE_NEW
#define M4U_MEM_USE_PMEM



void monitor_all()
{

    int fd;
    SPC_cfg spc_struct;
    spc_struct.RegionID = 1;       
    spc_struct.Enable = 1;         

    spc_struct.EngineMaskLARB0 = LARB0_ALL_EN;
    spc_struct.EngineMaskLARB1 = LARB1_ALL_EN; 
    spc_struct.EngineMaskLARB2 = LARB2_ALL_EN;
    spc_struct.EngineMaskLARB3 = LARB3_ALL_EN;

    spc_struct.ReadEn = 1;         
    spc_struct.WriteEn = 1;        
    spc_struct.StartAddr = MM_SYSRAM_BASE_PA;      
    spc_struct.EndAddr = MM_SYSRAM_BASE_PA + MM_SYSRAM_SIZE -1;

    fd = open("/dev/spc", O_RDWR);
    if(fd < 0)
    {
        perror("open /dev/spc failed in monitor_all\n");
        return;
    }

    if(ioctl(fd, MT6575SPC_CONFIG, &spc_struct)<0)
    {
        perror("open /dev/spc failed in monitor_all\n");
        return;
    }

    return;
}


void monitor_none()
{
    int fd;
    SPC_cfg spc_struct;
    
    spc_struct.RegionID = 1;       
    spc_struct.Enable = 0;         

    spc_struct.EngineMaskLARB0 = 0;
    spc_struct.EngineMaskLARB1 = 0; 
    spc_struct.EngineMaskLARB2 = 0;
    spc_struct.EngineMaskLARB3 = 0;

    spc_struct.ReadEn = 1;         
    spc_struct.WriteEn = 1;        
    spc_struct.StartAddr = MM_SYSRAM_BASE_PA;      
    spc_struct.EndAddr = MM_SYSRAM_BASE_PA + MM_SYSRAM_SIZE -1;
    fd = open("/dev/spc", O_RDWR);
    if(fd < 0)
    {
        perror("open /dev/spc failed in monitor_none\n");
        return;
    }

    if(ioctl(fd, MT6575SPC_CONFIG, &spc_struct)<0)
    {
        perror("open /dev/spc failed in monitor_none\n");
        return;
    }

}

void dump_reg()
{

    int fd;
    SPC_cfg spc_struct;

    fd = open("/dev/spc", O_RDWR);
    if(fd < 0)
    {
        perror("open /dev/spc failed in dump reg\n");
        return;
    }

    if(ioctl(fd, MT6575SPC_DUMP_REG, &spc_struct)<0)
    {
        perror("open /dev/spc failed in dump reg\n");
        return;
    }
}


void status_check()
{

    int fd;
    SPC_cfg spc_struct;

    fd = open("/dev/spc", O_RDWR);
    if(fd < 0)
    {
        perror("open /dev/spc failed in dump reg\n");
        return;
    }

    if(ioctl(fd, MT6575SPC_STATUS_CHECK, &spc_struct)<0)
    {
        perror("open /dev/spc failed in dump reg\n");
        return;
    }
}

int main (int argc, char *argv[])
{
    SPCDBG("enter m4u_ut main \n");
         //spc_register_isr();
    int ch;


    while( (ch=getopt(argc, argv, "andsh") )!= -1 )
    {
        switch(ch)
        {
            case ('a'): 
                printf("monitor all\n");
                monitor_all();
                break;
            case ('n'):
                printf("monitor none\n");
                monitor_none();
                break;
            case ('d'):
                printf("dump reg\n");
                dump_reg();
                break;

            case ('s'):
                printf("status check\n");
                status_check();
                break;

            case ('h'):
                printf("usage: spc_util [options]\n");
                printf(" -a : monitor all\n");
                printf(" -n : monitor none\n");
                printf(" -d : dump registers\n");
                printf(" -s : status check\n");
                break;

            default:
                printf("error argumnet\n");
                break;
        }

    }
 
    return 0;
}


