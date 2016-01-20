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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// mHalSurfaceView.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  mHalSurfaceView.cpp
//! \brief
#define LOG_TAG "mHalSurfaceView"

 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

extern "C" {
#include <linux/kd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <linux/ioctl.h>
#include <linux/videodev2.h>
#include <linux/fb.h>
#include "mtkfb.h"
}
#include "camera_custom_if.h"

#include "../inc/AcdkTypes.h"
#include "../inc/AcdkLog.h"
#include "../inc/AcdkCommon.h"
#include "../inc/AcdkErrCode.h"

#include "MediaHal.h"
#include "AcdkSurfaceView.h"
#include "mHalSurfaceView.h"

using namespace NSACDK;

#define FB_DEVICE "/dev/graphics/fb0"
#define BOOT_INFO_FILE "/sys/class/BOOT/BOOT/boot/boot_mode"
#define MEDIA_PATH "//data"

#define VIDOE_LAYER_NO    5

//#undef MTK_M4U_SUPPORT


/*******************************************************************************
*
********************************************************************************/
void   
mHalSurfaceView::
destroyInstance() 
{
     delete this; 
}


/*******************************************************************************
*
********************************************************************************/
mHalSurfaceView::mHalSurfaceView()
    :AcdkSurfaceView()
    ,mTotalLayerNum(VIDOE_LAYER_NO)
    ,mBootMode(NORMAL_MODE)
    ,mFBfd(-1) 
    ,mSurfaceWidth(0)
    ,mSurfaceHeight(0)
    ,mSurfaceOriention(0)
    ,mFBNo(0)
{
    ACDK_LOGD("[mHalSurfaceView] E \n"); 
    memset (&mVinfo, 0, sizeof(fb_var_screeninfo));
    ACDK_LOGD("[mHalSurfaceView] total layer cnt = %d\n", mTotalLayerNum); 
}

/*******************************************************************************
*
********************************************************************************/
mHalSurfaceView::~mHalSurfaceView()
{
    ACDK_LOGD("~mHalSurfaceView() E \n"); 
    
    if(mFBfd > 0) {
        uninit(); 
    }
}    
    
/*******************************************************************************
*
********************************************************************************/
MINT32 
mHalSurfaceView::getBootMode(MUINT32 &mode)
{
    //check if in Meta mode 
    MINT32 fd = open(BOOT_INFO_FILE, O_RDWR); 
    if (fd < 0) {
        ACDK_LOGE("fail to open %s \n", BOOT_INFO_FILE); 
        perror(""); 
        return ACDKSURFACE_API_FAIL; 
    }

    char bootMode[4]; 
    if (::read(fd, (MVOID*)bootMode, 4)== 0) {
        ACDK_LOGE("fail to read %s ", BOOT_INFO_FILE);
        perror("");
        close(fd); 
        return ACDKSURFACE_API_FAIL; 
    }
    
    if (bootMode[0] == 0x31) {
        mode = META_MODE; 
    }
    else {
        mode = NORMAL_MODE;
    }

    close(fd); 
    return ACDKSURFACE_NO_ERROR; 
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
mHalSurfaceView::init()
{
    MINT32 status; 
    ACDK_LOGD("[init] - E \n"); 
 
    //
/*
    ACDK_LOGD("[init] - Check the boot mode \n"); 
    status = getBootMode(mBootMode); 
    if (status != ACDKSURFACE_NO_ERROR) {
        ACDK_LOGD("[init] - check the boot mode fail \n"); 
        return ACDKSURFACE_API_FAIL; 
    }
*/
mBootMode = 0;
    //
    mFBfd = ::open(FB_DEVICE, O_RDWR); 
    if (!mFBfd) {
        ACDK_LOGE("[init] Can not get frame buffer deivce %s \n", FB_DEVICE); 
        return ACDKSURFACE_INVALID_DRIVER; 
    }    

    // Get Variable Screen Information  
    if (::ioctl(mFBfd, FBIOGET_VSCREENINFO, &mVinfo) < 0) {
        ACDK_LOGE("[init] Reading varaiable Infromation Fail\n");
        return ACDKSURFACE_API_FAIL;
    }    

    //
    // Check the LCD scan orientation. 
    mSurfaceOriention = NSCamCustom::getLCMPhysicalOrientation(); 
    // display driver reports the physical size
    /*	
    if (mSurfaceOriention == 90 || mSurfaceOriention == 270) {
        mSurfaceWidth = mVinfo.yres; 
        mSurfaceHeight = mVinfo.xres; 
    }
    else
    */
    {
        mSurfaceWidth = mVinfo.xres; 
        mSurfaceHeight = mVinfo.yres;         
    }        

    //Dump The infromation 
    ACDK_LOGD("Boot Mode = %d\n", mBootMode); 
    ACDK_LOGD("mVinfo.xres = %d\n", mVinfo.xres);        
    ACDK_LOGD("mVinfo.yres = %d\n", mVinfo.yres);    
    ACDK_LOGD("mVinfo.xresv = %d\n", mVinfo.xres_virtual);  
    ACDK_LOGD("mVinfo.yresv = %d\n", mVinfo.yres_virtual);   
    ACDK_LOGD("mVinfo.xoff = %d\n", mVinfo.xoffset);    
    ACDK_LOGD("mVinfo.yoff = %d\n", mVinfo.yoffset);   
    ACDK_LOGD("mVinfo.bits_per_pixel = %d\n", mVinfo.bits_per_pixel);  
    ACDK_LOGD("orientation = %d\n", mSurfaceOriention);  

    for (MUINT32 i = 0; i < mTotalLayerNum; i++) {
        memset(&mLayerInfo[i], 0, sizeof(struct fb_overlay_layer)); 
    }

    return ACDKSURFACE_NO_ERROR;     
}
    
/*******************************************************************************
*
********************************************************************************/
MINT32 
mHalSurfaceView::uninit()
{
    ACDK_LOGD("[uninit] E\n"); 
    if (mFBfd > 0 ) {
        for (MUINT32 layerNo = 0; layerNo < mTotalLayerNum; layerNo++) {
            mLayerInfo[layerNo].layer_id          = 1;
            mLayerInfo[layerNo].layer_enable      = 0;
        }
        // 
        if (ioctl(mFBfd, MTKFB_SET_VIDEO_LAYERS, &mLayerInfo) < 0)  {
            ACDK_LOGE("[uninit] ioctl(MTKFB_SET_VIDEO_LAYERS) failed");
            return ACDKSURFACE_API_FAIL; 
        }
        //
        if (ioctl(mFBfd, MTKFB_TRIG_OVERLAY_OUT, 0) < 0) {
            ACDK_LOGE("[uninit] ioctl(MTKFB_TRIG_OVERLAY_OUT) failed\n");
            return ACDKSURFACE_API_FAIL;     
        }        
        //! in meta mode, we should restore the screen back 
        if (mBootMode == META_MODE) {
            mVinfo.yoffset = 0;
            if (ioctl(mFBfd, MTKFB_META_RESTORE_SCREEN, &mVinfo) < 0) { 
                ACDK_LOGE("[uninit] Resore to Meta mode screen fail\n"); 
            }
        }
        //
        close(mFBfd); 
        mFBfd = -1; 
    }
    return ACDKSURFACE_NO_ERROR; 
}

    
/*******************************************************************************
*
********************************************************************************/
MINT32 
mHalSurfaceView::getSurfaceInfo(
            MUINT32 &width,
            MUINT32 &height, 
            MUINT32 &orientation
)
{
    width = mSurfaceWidth;
    height = mSurfaceHeight; 
    orientation = mSurfaceOriention; 
    return ACDKSURFACE_NO_ERROR; 
}
         
/*******************************************************************************
*
********************************************************************************/
MINT32 
mHalSurfaceView::setOverlayInfo(
        MUINT32 const layerNo, 
        MUINT32 const startx, 
        MUINT32 const starty, 
        MUINT32 const width, 
        MUINT32 const height, 
        MUINT32 const phyAddr, 
        MUINT32 const virtAddr,
        MUINT32 const orientation
)
{
    if (layerNo > mTotalLayerNum) {
        ACDK_LOGE("[setOverlayInfo] error layerNo = %d\n", layerNo); 
        return ACDKSURFACE_INVALID_PARA; 
    }

    //ACDK_LOGD("[setOverlayInfo] layerNo = %d\n", layerNo); 
    //ACDK_LOGD("[setOverlayInfo] startx = %d\n", startx); 
    //ACDK_LOGD("[setOverlayInfo] starty = %d\n", starty); 
    //ACDK_LOGD("[setOverlayInfo] width = %d\n", width); 
    //ACDK_LOGD("[setOverlayInfo] height = %d\n", height); 
    if (orientation == 90 || orientation == 270) { 
        mLayerInfo[layerNo].src_pitch         = height;
        mLayerInfo[layerNo].src_offset_x      = 0;
        mLayerInfo[layerNo].src_offset_y      = 0;
        mLayerInfo[layerNo].src_width         = height;
        mLayerInfo[layerNo].src_height        = width;
        mLayerInfo[layerNo].tgt_offset_x = startx;     
        mLayerInfo[layerNo].tgt_offset_y = starty; 
        mLayerInfo[layerNo].tgt_width         = height;
        mLayerInfo[layerNo].tgt_height        = width;
        mLayerInfo[layerNo].layer_id          = 1;
        mLayerInfo[layerNo].layer_enable      = 1;
        mLayerInfo[layerNo].src_fmt           = MTK_FB_FORMAT_RGB565;
        mLayerInfo[layerNo].src_use_color_key = 1;
        mLayerInfo[layerNo].src_color_key     = 0;
        mLayerInfo[layerNo].src_phy_addr = (MUINT32*)phyAddr; 
        mLayerInfo[layerNo].src_base_addr = (MUINT32*)virtAddr; 
    }
    else {
        mLayerInfo[layerNo].src_pitch         = width;
        mLayerInfo[layerNo].src_offset_x      = 0;
        mLayerInfo[layerNo].src_offset_y      = 0;
        mLayerInfo[layerNo].src_width         = width;
        mLayerInfo[layerNo].src_height        = height;
        mLayerInfo[layerNo].tgt_offset_x = startx;     
        mLayerInfo[layerNo].tgt_offset_y = starty; 
        mLayerInfo[layerNo].tgt_width         = width;
        mLayerInfo[layerNo].tgt_height        = height;
        mLayerInfo[layerNo].layer_id          = 1;
        mLayerInfo[layerNo].layer_enable      = 1;
        mLayerInfo[layerNo].src_fmt           = MTK_FB_FORMAT_RGB565;
        mLayerInfo[layerNo].src_use_color_key = 1;
        mLayerInfo[layerNo].src_color_key     = 0;
        mLayerInfo[layerNo].src_phy_addr = (MUINT32*)phyAddr; 
        mLayerInfo[layerNo].src_base_addr = (MUINT32*)virtAddr; 
    }
    return ACDKSURFACE_NO_ERROR; 
}     
                                    

/*******************************************************************************
*
********************************************************************************/
static FILE *fp = NULL; 
MINT32 
mHalSurfaceView::setOverlayBuf(
        MUINT32 const layerNo, 
        MUINT8 const *pSrcIn, 
        MUINT32 const srcFmt,  
        MUINT32 const srcW, 
        MUINT32 const srcH, 
        MUINT32 const orientation, 
        MUINT32 const hFlip, 
        MUINT32 const vFlip
)
{
    MRESULT status = MHAL_NO_ERROR;

    // It's hw convert, the buffer address must be physical address
    // ACDK_LOGD("[mtkYuvtoRgb565] pbufIn: 0x%x, pbufOut: 0x%x, width: %d, height: %d \n", (uint32_t) pbufIn, (uint32_t) pbufOut, width, height);
    
    MHalLockParam_t resource;
    resource.mode         = MHAL_MODE_BITBLT;
    resource.waitMilliSec = 1000;
    resource.waitMode     = MHAL_MODE_ALL;
    status = mHalIoCtrl(MHAL_IOCTL_LOCK_RESOURCE, &resource, sizeof(resource), NULL, 0, NULL);
    if (MHAL_NO_ERROR != status) {
        ACDK_LOGD("[setOverlayBuf] MHAL_IOCTL_LOCK_RESOURCE err: %d \n", status);
        return status;
    }

    mHalBltParam_t bltParam;
    memset(&bltParam, 0, sizeof(bltParam));
    bltParam.srcAddr   = (MUINT32)(pSrcIn);
    bltParam.srcX      = 0;
    bltParam.srcY      = 0;
    bltParam.srcW      = srcW; // width;
    bltParam.srcWStride= srcW; //width;
    bltParam.srcH      = srcH; //height;
    bltParam.srcHStride= srcH; // height;
    bltParam.srcFormat = (MHAL_BITBLT_FORMAT_ENUM) srcFmt; //MHAL_FORMAT_MTK_YUV;
    //bltParam.dstAddr   =  (MUINT32)mLayerInfo[layerNo].src_phy_addr;
    bltParam.dstAddr   =  (MUINT32)mLayerInfo[layerNo].src_base_addr;
    int bltOrientation = MHAL_BITBLT_ROT_0; 
    //Due to the android orientation should flip before rotation, 
    //Hence, if we rotate first, when rotate to 90 + h flip it will become 
    // rotate 270 + h flip. 
    switch (orientation) {
        case 0:
            bltOrientation = hFlip ? MHAL_BITBLT_FLIP_H : MHAL_BITBLT_ROT_0; 
            break; 
        case 90:
            bltOrientation = hFlip ? MHAL_BITBLT_FLIP_H |MHAL_BITBLT_ROT_270 : MHAL_BITBLT_ROT_90; 
            break; 
        case 180:
            bltOrientation = hFlip ? MHAL_BITBLT_FLIP_H |MHAL_BITBLT_ROT_180 : MHAL_BITBLT_ROT_180; 
            break; 
        case 270:
            bltOrientation = hFlip ? MHAL_BITBLT_FLIP_H|MHAL_BITBLT_ROT_90 : MHAL_BITBLT_ROT_270; 
            break; 
    }
    //
    if (orientation == 90 || orientation == 270) {
        bltParam.dstW      = srcH;
        bltParam.dstH      = srcW;
        bltParam.pitch     = srcH;
    }
    else {
        bltParam.dstW      = srcW;
        bltParam.dstH      = srcH;
        bltParam.pitch     = srcW;
    }
    bltParam.dstFormat = MHAL_FORMAT_RGB_565;      //display format 
    bltParam.orientation = bltOrientation; 

    //
    status = mHalIoCtrl(MHAL_IOCTL_BITBLT, &bltParam, sizeof(bltParam), NULL, 0, NULL);
    if (MHAL_NO_ERROR != status) {
        ACDK_LOGD("[setOverlayBuf] err: %d, can't do bitblt operation \n", status);
        return status; 
    }

    uint32_t mode = MHAL_MODE_BITBLT;
    if (MHAL_NO_ERROR != mHalIoCtrl(MHAL_IOCTL_UNLOCK_RESOURCE, &mode, sizeof(mode), NULL, 0, NULL)) {
        ACDK_LOGD("[setOverlayBuf] MHAL_IOCTL_UNLOCK_RESOURCE err: %d \n", status);
    }

#if 1 
    if (bltParam.srcFormat == MHAL_FORMAT_ABGR_8888) {
        fp = fopen ("//data//prv.raw", "wb"); 
        if (NULL == fp) {
            ACDK_LOGD("Can not open file /data/prv.raw\n"); 
        }
        fwrite((void*)mLayerInfo[layerNo].src_base_addr , 1, bltParam.srcW * bltParam.srcH  * 2, fp); 
        fclose(fp); 
    }
#endif
    
    return status;
}     
 
/*******************************************************************************
*
********************************************************************************/   
MINT32
mHalSurfaceView::resetLayer(MUINT32 const layerNo)
{
    if (layerNo > mTotalLayerNum) {
        ACDK_LOGE("[setOverlayInfo] error layerNo = %d\n", layerNo); 
        return ACDKSURFACE_INVALID_PARA;         
    }
    
    mLayerInfo[layerNo].layer_enable = 0; 

    //
    ACDK_LOGD("[vResetLayer] MTKFB_SET_VIDEO_LAYERS, layerNo = %d\n", layerNo);
    if (ioctl(mFBfd, MTKFB_SET_VIDEO_LAYERS, &mLayerInfo[0]) < 0) {
        ACDK_LOGE("[vResetLayer] ioctl(MTKFB_SET_VIDEO_LAYERS) failed");
        return ACDKSURFACE_API_FAIL; 
    }

    //
    ACDK_LOGD("[vResetLayer] MTKFB_TRIG_OVERLAY_OUT\n");
    if (ioctl(mFBfd, MTKFB_TRIG_OVERLAY_OUT, 0) < 0) {
        ACDK_LOGE("[vResetLayer] MTKFB_TRIG_OVERLAY_OUT failed!\n");
        return ACDKSURFACE_API_FAIL; 
    }
    return ACDKSURFACE_NO_ERROR;
}
    
/*******************************************************************************
*
********************************************************************************/   
MINT32 
mHalSurfaceView::refresh()
{
    //ACDK_LOGD("[refresh] MTKFB_SET_VIDEO_LAYERS");
    if (ioctl(mFBfd, MTKFB_SET_VIDEO_LAYERS, &mLayerInfo) < 0) {
        ACDK_LOGE("[refresh] ioctl(MTKFB_SET_VIDEO_LAYERS) failed");
        return ACDKSURFACE_API_FAIL;
    }

    //ACDK_LOGD("[refresh] MTKFB_TRIG_OVERLAY_OUT");
    if (ioctl(mFBfd, MTKFB_TRIG_OVERLAY_OUT, 0) < 0) {
        ACDK_LOGE("[refresh] ioctl(MTKFB_TRIG_OVERLAY_OUT) failed\n");    
        return ACDKSURFACE_API_FAIL;
    }    
    mFBNo = (mFBNo+1) % OVERLAY_BUFFER_CNT;
    
    return ACDKSURFACE_NO_ERROR; 
}
        
/*******************************************************************************
* 
*******************************************************************************/
MINT32 
mHalSurfaceView::getNumLayer(MUINT32 &numLayer)
{
    numLayer = mTotalLayerNum; 
    return ACDKSURFACE_NO_ERROR; 
}

/*******************************************************************************
* 
*******************************************************************************/
#if defined (MTK_M4U_SUPPORT)
MINT32 
mHalSurfaceView::registerBuffer(MUINT32 virtAddr, MUINT32 size)
{
    if (mFBfd < 0) {
        ACDK_LOGE("[registerBuffer] error fd = %d\n", mFBfd); 
        return ACDKSURFACE_INVALID_DRIVER; 
    } 

    int ret = 0;

    struct fb_overlay_buffer_info b;
    b.src_vir_addr = (unsigned int)virtAddr;
    b.size         = (unsigned int)size;

    ret = ::ioctl(mFBfd, MTKFB_REGISTER_OVERLAYBUFFER, &b);
    if (ret < 0) {
        ACDK_LOGE("[registerBuffer] MTKFB_REGISTER_OVERLAYBUFFER failed, err: %s, %d",
             strerror(ret), ret);
    }
    return ret; 
}

/*******************************************************************************
* 
*******************************************************************************/
MINT32 
mHalSurfaceView::unRegisterBuffer(MUINT32 virtAddr) 
{
    if (mFBfd < 0) {
        ACDK_LOGE("[registerBuffer] error fd = %d\n", mFBfd); 
        return ACDKSURFACE_INVALID_DRIVER; 
    } 
 
    MUINT32 va = virtAddr; 
    int ret = ::ioctl(mFBfd, MTKFB_UNREGISTER_OVERLAYBUFFER, &va);
    if (ret < 0) {
        ACDK_LOGE("[registerBuffer] MTKFB_UNREGISTER_OVERLAYBUFFER failed, err: %s, %d",
             strerror(ret), ret);
    }
    return ret;    
}
#endif 



