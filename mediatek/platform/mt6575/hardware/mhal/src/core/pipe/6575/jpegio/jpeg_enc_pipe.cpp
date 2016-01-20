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
 
#include <stdio.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "mdp_element.h"
#include "mdp_datatypes.h"
#include "mdp_path.h"
#include "jpeg_enc_hal.h"
#include "jpeg_hal.h"

#include <cutils/xlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "JpgEncHal"

#define JPG_LOG(fmt, arg...)    XLOGW(LOG_TAG fmt, ##arg)
#define JPG_DBG(fmt, arg...)    XLOGD(LOG_TAG fmt, ##arg)

JpgEncHal::JpgEncHal()
{
    JPG_DBG("JpgEncHal::JpgEncHal");

    fIsAddSOI = true;
    fROIWidth = fROIHeight = 0;
}

JpgEncHal::~JpgEncHal()
{
    JPG_DBG("JpgEncHal::~JpgEncHal");
}

bool JpgEncHal::lock()
{
    JPG_DBG("JpgEncHal::lock");
    if(JPEG_ENC_STATUS_OK != jpegEncLockEncoder(&encID))
    {
        return false;
    }
    return true;
}

bool JpgEncHal::unlock()
{
    JPG_DBG("JpgEncHal::unlock");
    jpegEncUnlockEncoder(encID);
    
    return true;
}

void JpgEncHal::setROI(UINT32 x, UINT32 y, UINT32 width, UINT32 height)
{
    fROIX = x;
    fROIY = y;
    fROIWidth = width;
    fROIHeight = height;
}

bool JpgEncHal::config_mdp(bool reScale)
{
    bool ret = true;
        
    //MDP Drv operation
    int             mdp_element_count = reScale ? 4 : 3;
    MDPELEMENT_I*   mdp_element_list[4];
    
    //MDP Elements    
    RDMA0           me_rdma0;
    MOUT            me_mout;
    JPEGDMA         me_jpegdma;

    CRZ             me_crz;
    IPP             me_ipp;
 
    MdpSize         src_size;
    MdpRect         src_rect;

    src_rect.w = src_size.w = fSrcWidth;
    src_rect.h = src_size.h = fSrcHeight;
    src_rect.x = src_rect.y = 0;

    if(fROIWidth != 0 || fROIHeight != 0 )
    {
        src_rect.x = (long)fROIX;
        src_rect.y = (long)fROIY;
        src_rect.w = (unsigned long)fROIWidth;
        src_rect.h = (unsigned long)fROIHeight;
    }
    
    me_rdma0.uInBufferCnt = 1;
    me_rdma0.src_img_yuv_addr[0].y = (unsigned long)fSrcAddr;                                 
    me_rdma0.src_img_size = src_size;
    me_rdma0.src_img_roi = src_rect;

    switch (fSrcFormat)
    {
        case kRGB_565_Format:
            me_rdma0.src_color_format = RGB565;
            break;

        case kRGB_888_Format:
            me_rdma0.src_color_format = RGB888;
            break;

        case kARGB_8888_Format:
            me_rdma0.src_color_format = ARGB8888;
            break;

        case kABGR_8888_Format:
            me_rdma0.src_color_format = ABGR8888;
            break;
            
        case kYUY2_Pack_Format:
            me_rdma0.src_color_format = YUY2_Pack;
            break;
            
        case kUYVY_Pack_Format:
            me_rdma0.src_color_format = UYVY_Pack;
            break;
            
        case kYVU9_Planar_Format:
            me_rdma0.src_color_format = Y411_Pack;
            break;
            
        case kYV16_Planar_Format:
            me_rdma0.src_color_format = YV16_Planar;
            break;

        case kYV12_Planar_Format:
            me_rdma0.src_color_format = YV12_Planar;
            break;

        case kNV12_Format:
            me_rdma0.src_color_format = NV12;
            break;
            
        case kNV21_Format:
            me_rdma0.src_color_format = NV21;
            break;
            
        default :
            JPG_LOG("Unsupport Input Format : %d", fSrcFormat);
            return false;
    }    

    me_rdma0.bEISEn = 0;
    me_rdma0.u4EISCON = 0;//EIS setting
    //RDMA0
    if(reScale)
    {
        me_rdma0.to_cam = 1;
        me_rdma0.to_mout = 0;
    }
    else
    {
        me_rdma0.to_cam = 0;
        me_rdma0.to_mout = 1;
    }

    me_rdma0.to_ovl = 0;
    me_rdma0.trigger_src = 0;
    
    if(reScale)
    {
        // config crz
        src_size.w = src_rect.w;
        src_size.h = src_rect.h;
        src_rect.x = src_rect.y = 0;
        me_crz.src_sel = 0;
        me_crz.src_img_size = src_size;
        me_crz.src_img_roi = src_rect;
        me_crz.dst_img_size.w = fDstWidth;
        me_crz.dst_img_size.h = fDstHeight;

        // config ipp
        me_ipp.bBypass = 1;
        me_ipp.to_jpg_dma = 1;
        me_ipp.src_sel = 1; //0 : OVL, 1:CRZ
    }
    else
    {
        // config mout
        me_mout.to_jpg_dma = 1;
        me_mout.src_sel = 0;// 0-R_DMA0_MOUT, 1-OVL_DMA_MIMO
    }

    me_jpegdma.src_img_size.w = fDstWidth;
    me_jpegdma.src_img_size.h = fDstHeight;
    me_jpegdma.src_sel = reScale ? 0 : 1;// 0-IPP_MOUT, 1-MOUT

    //integer : 411, 422, 444, 400, 410 etc...
    switch (fEncFormat)
    {
        case kYUV_400_Format:
            me_jpegdma.output_format = 400;
            break;
            
        case kYUV_420_Format:
            me_jpegdma.output_format = 420;
            break;
            
        case kYUV_422_Format:
            me_jpegdma.output_format = 422;
            break;
            
        case kYUV_411_Format:
            me_jpegdma.output_format = 411;
            break;

        case kYUV_444_Format:
            me_jpegdma.output_format = 444;
            break;
            
        default :
            JPG_LOG("Unsupport Encoder Format : %d", fEncFormat);
            return false;
    }

    /*Resource List*/
    if(reScale)
    {
        mdp_element_list[0] = (MDPELEMENT_I*)&me_rdma0;
        mdp_element_list[1] = (MDPELEMENT_I*)&me_crz;
        mdp_element_list[2] = (MDPELEMENT_I*)&me_ipp;
        mdp_element_list[3] = (MDPELEMENT_I*)&me_jpegdma;
    }
    else
    {
        mdp_element_list[0] = (MDPELEMENT_I*)&me_rdma0;
        mdp_element_list[1] = (MDPELEMENT_I*)&me_mout;
        mdp_element_list[2] = (MDPELEMENT_I*)&me_jpegdma;    
    }
    MdpDrvInit();
    if(0 != _MdpPathTriggerHw(mdp_element_list , mdp_element_count))
    {
        JPG_LOG("Trigger MDP Path Fail");
        MdpDrvRelease();
        return false;
    }
    MdpDrvRelease();
    return true;
}

bool JpgEncHal::start(UINT32 *encSize)
{
    JPEG_ENC_HAL_IN inJpgEncParam;

    JPG_DBG("JpgEncHal::start -> config jpeg encoder");
    JPG_DBG("Encoder Src Addr:0x%x, width/height:[%u, %u], format:%u", (unsigned int)fSrcAddr, fSrcWidth, fSrcHeight, fSrcFormat);
    JPG_DBG("Encoder Dst Addr:0x%x, width/height:[%u, %u], format:%u", (unsigned int)fDstAddr, fDstWidth, fDstHeight, fEncFormat);
    JPG_DBG("Encoder Quality:%u, Buffer Size:%u, Need add SOI:%d", fQuality, fDstSize, fIsAddSOI);
    JPG_DBG("Encoder ROI:(%d, %d, %d, %d)", fROIX, fROIY, fROIWidth, fROIHeight);
    // config jpeg encoder
    inJpgEncParam.dstBufferAddr = (unsigned char*)fDstAddr;
    inJpgEncParam.dstBufferSize = fDstSize;
    inJpgEncParam.dstWidth = fDstWidth;
    inJpgEncParam.dstHeight = fDstHeight;
    if(fQuality > 90)
    {
        inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q95;
    } 
    else if(fQuality > 60)
    {
        inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q80;
    }
    else if(fQuality > 40)
    {
        inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q60;
    }
    else
    {
        inJpgEncParam.dstQuality = JPEG_ENCODE_QUALITY_Q39;
    }
    
    inJpgEncParam.isPhyAddr = 0;

    if(fIsAddSOI)
        inJpgEncParam.enableEXIF = 0;
    else
        inJpgEncParam.enableEXIF = 1;

    switch (fEncFormat)
    {
        case kYUV_400_Format:
            inJpgEncParam.dstFormat = JPEG_SAMPLING_FORMAT_GRAYLEVEL;
            break;
            
        case kYUV_420_Format:
            inJpgEncParam.dstFormat = JPEG_SAMPLING_FORMAT_YUV420;
            break;
            
        case kYUV_422_Format:
            inJpgEncParam.dstFormat = JPEG_SAMPLING_FORMAT_YUV422;
            break;
            
        case kYUV_411_Format:
            inJpgEncParam.dstFormat = JPEG_SAMPLING_FORMAT_YUV411;
            break;

        case kYUV_444_Format:
            inJpgEncParam.dstFormat = JPEG_SAMPLING_FORMAT_YUV444;
            break;
            
        default :
            JPG_LOG("Unsupport Encoder Format : %d", fEncFormat);
            return false;
    }
    
    if(JPEG_ENC_STATUS_OK != jpegEncConfigEncoder(encID, inJpgEncParam))
    {
        JPG_LOG("Config Encoder Fail");
        return false;
    }

    JPG_DBG("JpgEncHal::start -> config MDP Path");
    bool reScale = false;

    if((fSrcWidth != fDstWidth) || (fSrcHeight != fDstHeight)) reScale = true;
    if(fROIWidth != 0 || fROIHeight != 0 ) reScale = true;
    

    // Trigger Jpeg Encoder
    if(JPEG_ENC_STATUS_OK != jpegEncStart(encID))
    {
        JPG_LOG("Trigger Encoder Fail");
        return false;
    }
    
    if(!config_mdp(reScale))
    {
        JPG_LOG("Config MDP Fail");
        return false;        
    }

    JPEG_ENC_RESULT_ENUM result;
    if(JPEG_ENC_STATUS_OK != jpegEncWaitIRQ(encID, 3000, encSize, &result))
    {
        JPG_LOG("Wait IRQ Fail");
        return false;
    }

    JPG_DBG("jpeg encoder result:%d, size:%u", result, *encSize);
    
    if(result != JPEG_ENC_RESULT_DONE)
        return false;

    return true;
}
    
