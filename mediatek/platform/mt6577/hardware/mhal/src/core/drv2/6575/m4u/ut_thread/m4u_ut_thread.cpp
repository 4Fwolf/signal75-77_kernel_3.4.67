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

#include <m4u_lib.h>

#undef LOG_TAG
#define LOG_TAG "[m4u_ut]"

#define MTKM4UDEBUG
#ifdef MTKM4UDEBUG
  #define M4UDBG(string, args...) printf("[M4U_N]"string,##args)
#else
  #define M4UDBG(string, args...)
#endif

//#define M4U_MEM_USE_NEW
#define M4U_MEM_USE_PMEM

#define BufSize 1024*1024
unsigned int BufAddr;
unsigned int mva;
MTKM4UDrv *CM4u;

unsigned int BufAddr2;
unsigned int mva2;

void* alloc_thread(void* Data)
{
    CM4u->m4u_alloc_mva(M4U_CLNTMOD_CAM, BufAddr, BufSize, &mva);
    printf("alloc in thread: mva=0x%x, tid=%d\n", mva, pthread_self());
    return NULL;
}

void* register_thread(void* Data)
{
    CM4u->m4u_register_buffer(M4U_CLNTMOD_CAM, BufAddr2, BufSize, &mva);
    printf("register in thread: tid=%d\n", pthread_self());
    return NULL;
}


void * func1(void *)
{
    volatile int i,j = 0;
    for(i=0; i<10000; i++)
        j++;
    return NULL;
}


/* allocate_mva in main thread
 * query_mva in child thread
 */
void test1(void)
{

    printf("test1: allocate & query in multi-thread\n");
    unsigned int query_mva;

    pthread_t tid;

    pthread_create(&tid, NULL, alloc_thread, NULL);

    pthread_join(tid,NULL);

    for(int i=0; i<50; i++)
    {
        pthread_create(&tid, NULL, func1, NULL);
        pthread_join(tid,NULL);
    }


    sleep(20);

    CM4u->m4u_query_mva(M4U_CLNTMOD_CAM, BufAddr, BufSize, &query_mva);
    if(query_mva == 0)
    {
        printf("query_mva returns NULL!!\n");
        return;
    }
    else
    {
        printf("query_mva = 0x%x\n", query_mva);
    }

    CM4u->m4u_dealloc_mva(M4U_CLNTMOD_CAM, BufAddr, BufSize, query_mva);
}


/* allocate_mva in main thread
 * query_mva in child thread
 */
void test2(void)
{

    printf("test2: register & query in multi-thread\n");
    unsigned int query_mva;

    pthread_t tid;

    pthread_create(&tid, NULL, register_thread, NULL);

    pthread_join(tid,NULL);

    for(int i=0; i<50; i++)
    {
        pthread_create(&tid, NULL, func1, NULL);
        pthread_join(tid,NULL);
    }


    sleep(20);

    CM4u->m4u_query_mva(M4U_CLNTMOD_CAM, BufAddr2, BufSize, &query_mva);

    if(query_mva == 0)
    {
        printf("query_mva returns NULL!!\n");
        return ;
    }
    else
    {
        printf("query_mva = 0x%x\n", query_mva);
    }

    CM4u->m4u_dealloc_mva(M4U_CLNTMOD_CAM, BufAddr2, BufSize, query_mva);
}






int main (int argc, char *argv[])
{
    BufAddr = (unsigned int) new char[BufSize];
    BufAddr2 = (unsigned int) new char[BufSize];
    CM4u = new MTKM4UDrv;
    
    if(argc == 1)
        test1();
    else if(argc == 2)
        test2();
    else
        printf("please add some args!!\n");

    return 0;

}


