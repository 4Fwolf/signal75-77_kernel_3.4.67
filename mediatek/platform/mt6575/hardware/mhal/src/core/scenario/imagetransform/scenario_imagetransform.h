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

#ifndef _Scenario_ImageTransform_H_
#define _Scenario_ImageTransform_H_

/*
#define MUINT32 MHAL_UINT32
typedef unsigned int MHAL_UINT32;

typedef enum
{
    MHAL_NO_ERROR = 0,                  ///< The function work successfully
    MHAL_INVALID_DRIVER,                ///< Error due to invalid driver
    MHAL_INVALID_CTRL_CODE,             ///< Error due to invalid control code
    MHAL_INVALID_PARA,                  ///< Error due to invalid parameter
    MHAL_INVALID_MEMORY,                ///< Error due to invalid memory
    MHAL_INVALID_FORMAT,                ///< Error due to invalid file format
    MHAL_INVALID_RESOURCE,              ///< Error due to invalid resource, like IDP

    MHAL_UNKNOWN_ERROR = 0x80000000,    ///< Unknown error
    MHAL_ALL = 0xFFFFFFFF
} MHAL_ERROR_ENUM;

typedef enum
{
    MHAL_FORMAT_RGB_565 = 0,
    MHAL_FORMAT_RGB_888,
    MHAL_FORMAT_ARGB_8888,
    MHAL_FORMAT_YUV_420,
    MHAL_FORMAT_MTK_YUV,
    MHAL_FORMAT_ERROR,
    MHAL_FORMAT_ALL = 0xFFFFFFFF
} MHAL_BITBLT_FORMAT_ENUM;

typedef enum
{
    MHAL_BITBLT_ROT_0 = 0,
    MHAL_BITBLT_ROT_90,
    MHAL_BITBLT_ROT_180,
    MHAL_BITBLT_ROT_270,
    MHAL_BITBLT_ALL = 0xFFFFFFFF
} MHAL_BITBLT_ROT_ENUM;

typedef struct mHalBltParam_s
{
    MUINT32 srcX;
    MUINT32 srcY;
    MUINT32 srcW;
    MUINT32 srcWStride;
    MUINT32 srcH;
    MUINT32 srcHStride;
    MUINT32 srcAddr;
    MUINT32 srcFormat;

    MUINT32 dstW;
    MUINT32 dstH;
    MUINT32 dstAddr;
    MUINT32 dstFormat;
    MUINT32 pitch;

    MUINT32 orientation;
} mHalBltParam_t;
*/
int mHalBitblt(void *a_pInBuffer);

typedef struct mHalImgPostProcess_s
{
    unsigned short srcX;
    unsigned short srcY;
    unsigned short srcW;
    unsigned short srcWStride;
    unsigned short srcH;
    unsigned short srcHStride;
    unsigned long srcAddr;
    unsigned short srcFormat;
    unsigned short dstX;
    unsigned short dstY;    
    unsigned short dstW;
    unsigned short dstWStride;
    unsigned short dstH;
    unsigned long dstAddr;
    unsigned short dstFormat;
    unsigned short orientation;
    //Color //TODO : color process parameters
} mHalImgPostProcess_t;

int mHal_ImagePostProcess(mHalImgPostProcess_t * a_stParam);

#endif
