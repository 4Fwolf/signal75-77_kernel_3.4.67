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

#include <stdio.h>
#include <cutils/log.h>
#include <cutils/pmem.h>
#include <cutils/memutil.h>
#include "MediaHal.h"
#include "jpeg_dec_hal.h"
#include "MT6573MDPDrv.h"
#include "scenario_imagetransform.h"
/********************************************************************************/
#define xlog(...) \
        do { \
            LOGD(__VA_ARGS__); \
        } while (0)

#undef LOG_TAG
#define LOG_TAG "jpeg_test"

//Try bitblt hal
extern unsigned char src_file[];

//#define BUFFER_SIZE 12801
#define BUFFER_SIZE (720*480*4)
#define OUTW 720
#define OUTH 480
int main()
{
    mHalBltParam_t stParam;
    mHalImgPostProcess_t stImgPostParam;
    unsigned int dst_size;
    unsigned char *src_va;
    unsigned char *dst_va;
    int src_ID;
    int dst_ID;
    unsigned long src_pa;
    unsigned long dst_pa;

    //PMEM
//    src_va = (unsigned char *) pmem_alloc_sync(BUFFER_SIZE , &src_ID);
//    src_pa = (unsigned long)pmem_get_phys(src_ID);
    src_va = (unsigned char *)malloc(BUFFER_SIZE);
xlog("Source VA is : %d\n" , src_va);
    if(src_va == NULL)
    {
        xlog("Can not allocate memory\n");
        return -1;
    }

    dst_size = (OUTW*OUTH*4);
//    dst_va = (unsigned char *) pmem_alloc_sync(dst_size , &dst_ID);
//    dst_pa = (unsigned long)pmem_get_phys(dst_ID);    
    dst_va = (unsigned char *)malloc(dst_size);
xlog("Dest VA is : %d\n" , dst_va);
    memset(dst_va , 0 , dst_size);

    FILE *fp;
    unsigned long index;
    fp = fopen("/data/Before.raw", "r");
    fread(src_va , 1 , BUFFER_SIZE , fp);
    fclose(fp);
//
//memset(src_va , 0 , BUFFER_SIZE);

    memset(&stParam, 0, sizeof(stParam));
    stParam.srcX = 0;
    stParam.srcY = 0;
    stParam.srcW = 720; //106
    stParam.srcWStride = 720; //112
    stParam.srcH = 480; //200
    stParam.srcHStride = 480; //208
    stParam.srcAddr = (MUINT32)(src_va);//TODO
    stParam.srcFormat = MHAL_FORMAT_ARGB_8888;
    stParam.dstW = 720;//479
    stParam.dstH = 480;//254
    stParam.dstAddr = (MUINT32)dst_va;
    stParam.dstFormat = MHAL_FORMAT_ARGB_8888;//TODO
    stParam.pitch = 720;// 479
    stParam.orientation = 0;// 1

    stImgPostParam.srcX = 0;
    stImgPostParam.srcY = 0;
    stImgPostParam.srcW = 3200;
    stImgPostParam.srcWStride = 3200;
    stImgPostParam.srcH = 2400;
    stImgPostParam.srcHStride = 2400;
    stImgPostParam.srcAddr = (unsigned long)src_va;
    stImgPostParam.srcFormat = MHAL_FORMAT_YUY2;
    stImgPostParam.dstX = 0;
    stImgPostParam.dstY = 0;
    stImgPostParam.dstW = 3200;
    stImgPostParam.dstWStride = 3200;
    stImgPostParam.dstH = 2400;
    stImgPostParam.dstAddr = (unsigned long)dst_va;
    stImgPostParam.dstFormat = MHAL_FORMAT_YUY2;
    stImgPostParam.orientation = 0;

struct timeval t1 , t2;
gettimeofday(&t1,NULL);
    if(MHAL_NO_ERROR != mHalBitblt((void *)&stParam))
//    if(MHAL_NO_ERROR != mHal_ImagePostProcess(&stImgPostParam))
    {
        xlog("Test failed!!");
    }
    else
    {
gettimeofday(&t2,NULL);
xlog("Time cost : %d" , (t2.tv_sec - t1.tv_sec)*1000000 + (t2.tv_usec - t1.tv_usec));
        xlog("Test success!!");
    }

//Save
#if 1
//sprintf(filename , "/data/ttt%d.raw" , index2);
//    fp = fopen(filename, "w");
    fp = fopen("/data/ttt.raw", "w");
    for(index = 0 ; index < dst_size ; index++)
    {
        fprintf(fp, "%c", dst_va[index]);
    }
    fclose(fp);
#endif

//    free(src_va);
//    free(dst_va);
    pmem_free(src_va , BUFFER_SIZE , src_ID);
    pmem_free(dst_va , dst_size , dst_ID);
    
    return 0;
}

