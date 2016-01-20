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
 
#ifndef __JPEG_HAL_H__
#define __JPEG_HAL_H__

#ifndef MTK_M4U_SUPPORT
#define USE_PMEM
#endif

#ifndef USE_PMEM
#include "m4u_lib.h"
#endif
/*******************************************************************************
*
********************************************************************************/
#ifndef UINT32
typedef unsigned int UINT32;
#endif

#ifndef INT32
typedef int INT32;
#endif

/*******************************************************************************
* class JpgDecHal
********************************************************************************/
class JpgDecHal {
public:
    JpgDecHal();
    virtual ~JpgDecHal();

    enum ColorFormat {
        kRGB_565_Format,
        kRGB_888_Format,
        kBGR_888_Format,
        kARGB_8888_Format,
        kABGR_8888_Format,
        kYUY2_Pack_Format,      // YUYV
        kUYVY_Pack_Format       // UYVY        
    };
    
    bool lock();
    bool unlock();
    bool parse();
    bool start();

    UINT32 getJpgWidth() { return fJpgWidth; }
    UINT32 getJpgHeight() { return fJpgHeight; }
    
    void setOutWidth(UINT32 width) { fOutWidth = width; }
    void setOutHeight(UINT32 height) { fOutHeight = height; }
    void setOutFormat(ColorFormat format) { fOutFormat = format; }
    void setSrcAddr(unsigned char *srcAddr) { fSrcAddr = srcAddr; }
    void setDstAddr(unsigned char *dstAddr) { fDstAddr = dstAddr; }
    void setSrcSize(UINT32 srcSize) { fSrcSize = srcSize; }
    void setDstSize(UINT32 dstSize) { fDstSize = dstSize; }
    void setDither(bool doDither) { isDither = doDither; }
    void setRangeDecode(UINT32 left, UINT32 top, UINT32 right, UINT32 bottom);
    void setProcHandler(void* h) { fProcHandler = h; }  
    
private:
    bool onStart();
    bool checkParam();
    bool islock;
    bool isDither;
    bool isRangeDecode;

    UINT32 fJpgWidth;
    UINT32 fJpgHeight;
    UINT32 fOutWidth;
    UINT32 fOutHeight;

    UINT32 fLeft;
    UINT32 fRight;
    UINT32 fTop;
    UINT32 fBottom;
    
    ColorFormat fOutFormat;

    unsigned char *fSrcAddr;
    unsigned char *fDstAddr;

    UINT32 fSrcSize;    
    UINT32 fDstSize;

    UINT32 fSrcConfigAddr;
    UINT32 fDstConfigAddr;

    void* fProcHandler;
#ifdef USE_PMEM
    unsigned char *fSrcPmemVA;
    unsigned char *fDstPmemVA;
    int fSrcPmemFD;
    int fDstPmemFD;
#else
    MTKM4UDrv *pM4uDrv;
    unsigned int fSrcMVA;
#endif
    int decID;
};


/*******************************************************************************
* class JpgEncHal
********************************************************************************/
class JpgEncHal {
public:
    JpgEncHal();
    virtual ~JpgEncHal();

    enum SrcFormat {
        kRGB_565_Format,
        kRGB_888_Format,
        kARGB_8888_Format,
        kABGR_8888_Format,
        kYUY2_Pack_Format,      // YUYV
        kUYVY_Pack_Format,      // UYVY
        kYVU9_Planar_Format,    // YUV411, 4x4 sub sample U/V plane
        kYV16_Planar_Format,    // YUV422, 2x1 subsampled U/V planes
        kYV12_Planar_Format,    // YUV420, 2x2 subsampled U/V planes
        kNV12_Format,           // YUV420, 2x2 subsampled , interleaved U/V plane
        kNV21_Format,           // YUV420, 2x2 subsampled , interleaved V/U plane
        
        kSrcFormatCount
    };

    enum EncFormat {
        kYUV_444_Format,
        kYUV_422_Format,
        kYUV_411_Format,
        kYUV_420_Format,
        kYUV_400_Format,
        
        kEncFormatCount
    };
    
    bool lock();
    bool unlock();
    bool start(UINT32 *encSize);
    void setROI(UINT32 x, UINT32 y, UINT32 width, UINT32 height);

    void setSrcWidth(UINT32 width) { fSrcWidth = width; }
    void setSrcHeight(UINT32 height) { fSrcHeight = height; }
    void setDstWidth(UINT32 width) { fDstWidth = width; }
    void setDstHeight(UINT32 height) { fDstHeight = height; }
    void setQuality(UINT32 quality) { fQuality = quality; }
    void setSrcFormat(SrcFormat srcformat) { fSrcFormat = srcformat; }
    void setEncFormat(EncFormat encformat) { fEncFormat = encformat; }
    void setSrcAddr(void *srcAddr) { fSrcAddr = srcAddr; }
    void setDstAddr(void *dstAddr) { fDstAddr = dstAddr; }
    void setDstSize(UINT32 size) { fDstSize = size; }
    void enableSOI(bool b) { fIsAddSOI = b; }

     
private:
    bool config_mdp(bool reScale);
    
    UINT32 fSrcWidth;
    UINT32 fSrcHeight;
    UINT32 fDstWidth;
    UINT32 fDstHeight;
    UINT32 fQuality;
    UINT32 fROIX;
    UINT32 fROIY;
    UINT32 fROIWidth;
    UINT32 fROIHeight;
    SrcFormat fSrcFormat;
    EncFormat fEncFormat;
    void *fSrcAddr;
    void *fDstAddr;
    int fDstSize;
    bool fIsAddSOI;

    int encID;
    unsigned long fResTable;
};

#endif 