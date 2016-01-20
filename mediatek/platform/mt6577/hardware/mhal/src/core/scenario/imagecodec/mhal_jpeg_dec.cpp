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

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2009
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "MediaTypes.h"
#include "mhal_jpeg.h"
#include "jpeg_hal.h"

extern "C" {
#include <pthread.h>
#include <semaphore.h>
}

#include <cutils/log.h>
#include <cutils/xlog.h>

#define xlog(...) \
        do { \
            XLOGW(__VA_ARGS__); \
        } while (0)

#undef LOG_TAG
#define LOG_TAG "mHalJpgDec"

static JpgDecHal* jpgDecoder = NULL;

static pthread_mutex_t resMutex = PTHREAD_MUTEX_INITIALIZER;

int return_false(const char msg[])
{
    if(jpgDecoder != NULL)
    {
        delete jpgDecoder;
        jpgDecoder = NULL;
    }

    xlog("%s", msg);
    
    return JPEG_ERROR_INVALID_DRIVER;
}

int mHalJpgDecParser(unsigned char* srcAddr, unsigned int srcSize)
{
    pthread_mutex_lock(&resMutex);
    
    if(jpgDecoder != NULL) 
    {
        xlog("hw decoder is busy");
        pthread_mutex_unlock(&resMutex);
        //return JPEG_ERROR_INVALID_DRIVER;
        return MHAL_INVALID_RESOURCE;
    }
    jpgDecoder = new JpgDecHal();
    
    pthread_mutex_unlock(&resMutex);

    if(!jpgDecoder->lock())
    {
        return return_false("can't lock resource");
    }

    jpgDecoder->setSrcAddr(srcAddr);
    jpgDecoder->setSrcSize(srcSize);
    
    if(!jpgDecoder->parse()) { return return_false("no support file format"); }

    return 0;
}

int mHalJpgDecGetInfo(MHAL_JPEG_DEC_INFO_OUT *decOutInfo)
{
    if(jpgDecoder == NULL) { return return_false("null decoder"); }
    
    decOutInfo->srcWidth = jpgDecoder->getJpgWidth();
    decOutInfo->srcHeight = jpgDecoder->getJpgHeight();

    return 0;
}

int mHalJpgDecStart(MHAL_JPEG_DEC_START_IN *decInParams, void* procHandler)
{
    if(jpgDecoder == NULL) { return return_false("null decoder"); }

    if(decInParams == NULL)
    {
        jpgDecoder->unlock();
        delete jpgDecoder;
        jpgDecoder = NULL;

        return 0;
    }
    if(decInParams->srcBuffer != NULL)
    {
        jpgDecoder->setSrcAddr(decInParams->srcBuffer);
        jpgDecoder->setSrcSize(decInParams->srcLength);
    }
    
    jpgDecoder->setOutWidth(decInParams->dstWidth);
    jpgDecoder->setOutHeight(decInParams->dstHeight);

    unsigned int dst_size = decInParams->dstWidth * decInParams->dstHeight;
    switch (decInParams->dstFormat)
    {
        case JPEG_OUT_FORMAT_RGB565:
            jpgDecoder->setOutFormat(JpgDecHal::kRGB_565_Format);
            dst_size *= 2;
            break;

        case JPEG_OUT_FORMAT_RGB888:
            jpgDecoder->setOutFormat(JpgDecHal::kRGB_888_Format);
            dst_size *= 3;
            break;

        case JPEG_OUT_FORMAT_ARGB8888:
            jpgDecoder->setOutFormat(JpgDecHal::kARGB_8888_Format);
            dst_size *= 4;
            break;

        case JPEG_OUT_FORMAT_YUY2:
            jpgDecoder->setOutFormat(JpgDecHal::kYUY2_Pack_Format);
            dst_size *= 2;
            break;

        case JPEG_OUT_FORMAT_UYVY:
            jpgDecoder->setOutFormat(JpgDecHal::kUYVY_Pack_Format);
            dst_size *= 2;
            break;
            
        default:
            jpgDecoder->setOutFormat(JpgDecHal::kRGB_565_Format);
            dst_size *= 2;
            break;
    }

    jpgDecoder->setDstAddr(decInParams->dstVirAddr);
    jpgDecoder->setDstSize(dst_size);
    jpgDecoder->setProcHandler(procHandler);
    
    if(decInParams->doDithering == 0)
        jpgDecoder->setDither(false);
    else
        jpgDecoder->setDither(true);

    if(decInParams->doRangeDecode == 1)
    {
        jpgDecoder->setRangeDecode(decInParams->rangeLeft, decInParams->rangeTop, 
                                   decInParams->rangeRight, decInParams->rangeBottom);
    }
    
    if(!jpgDecoder->start())
    {
        jpgDecoder->unlock();
        delete jpgDecoder;
        jpgDecoder = NULL;
        return return_false("decode failed~~~");
    }

    jpgDecoder->unlock();
    delete jpgDecoder;
    jpgDecoder = NULL;
    
    return 0;
}

