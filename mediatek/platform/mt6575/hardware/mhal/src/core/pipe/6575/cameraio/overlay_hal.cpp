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

#define LOG_TAG "OvlHal"

#include <fcntl.h>

#include <linux/ioctl.h>
#include "mtkfb.h"

#include <cutils/xlog.h>

#if defined(MTK_M4U_SUPPORT)
#include "m4u_lib.h"
#endif

#include "overlay_hal.h"

// ---------------------------------------------------------------------------

#define _LOG(fmt, arg...) XLOGD(fmt, ##arg)
#define _ERR(fmt, arg...) XLOGE("Line %d, "fmt, __LINE__, ##arg)

// ---------------------------------------------------------------------------

typedef OverlayHal::STATUS Status;

// ---------------------------------------------------------------------------

OverlayHal::OverlayHal() :
    m_fdFB(-1),
    m_layerMemHandle(-1),
    m_layerMemSize(0),
    m_totalLayerMemSize(0),
    m_pLayerInfo(NULL),
    m_activeBufferIdx(0)
        
{
    memset(m_pLayerVA, 0, sizeof(m_pLayerVA));
    memset(m_pLayerPA, 0, sizeof(m_pLayerPA));

    m_pLayerInfo = new layer_info();

    if (!m_pLayerInfo) {
        _ERR("allocate layer_info failed");
    }
    
#if defined(MTK_M4U_SUPPORT)
    m_m4uDrv = new MTKM4UDrv();
#endif
}


OverlayHal::~OverlayHal()
{
    if (m_pLayerInfo) delete m_pLayerInfo;
    
#if defined(MTK_M4U_SUPPORT)
    delete m_m4uDrv;
#endif
}

// ---------------------------------------------------------------------------


Status OverlayHal::init(
    MUINT32 srcX, MUINT32 srcY, MUINT32 srcW, MUINT32 srcH,
    MUINT32 dstX, MUINT32 dstY, MUINT32 dstW, MUINT32 dstH)
{
    Status err = OK;
    MUINT8 *buffer_va = NULL;
    MUINT8 *buffer_pa = NULL;
    layer_info *pLayerInfo = m_pLayerInfo;
    MINT32 w, h;

    _LOG("OverlayHal::init(), src: (%d,%d,%d,%d), dst: (%d,%d,%d,%d)",
         srcX, srcY, srcW, srcH,
         dstX, dstY, dstW, dstH);

    w = (srcW + 15) & ~0x0F;
    h = (srcH + 15) & ~0x0F;
    
    m_layerMemSize = w * h * FD_LAYER_BPP;
    m_totalLayerMemSize = m_layerMemSize * FD_LAYER_NUM;
    
    buffer_va = (MUINT8 *) allocate_buffer(m_totalLayerMemSize);

    if (buffer_va == NULL) {
        _ERR("[init] pmem_alloc for FD layer failed");
        return ALLOC_MEM_FAIL;
    }

    buffer_pa = (MUINT8 *)get_buffer_phys_addr();
    
    memset(pLayerInfo, 0, sizeof(fb_overlay_layer));
    pLayerInfo->src_phy_addr      = buffer_pa;
    pLayerInfo->src_base_addr     = buffer_va;
    pLayerInfo->src_pitch         = srcW;
    pLayerInfo->src_offset_x      = 0;
    pLayerInfo->src_offset_y      = 0;
    pLayerInfo->src_width         = srcW;
    pLayerInfo->src_height        = srcH;
    pLayerInfo->tgt_offset_x      = dstX;
    pLayerInfo->tgt_offset_y      = dstY;
    pLayerInfo->tgt_width         = dstW;
    pLayerInfo->tgt_height        = dstH;
#warning [FIXME]KK_Migration[SeanC]
#if 0
    pLayerInfo->layer_id          = FACE_DETECTION_LAYER_ID;
#endif
    pLayerInfo->layer_enable      = 0;
    pLayerInfo->src_fmt           = MTK_FB_FORMAT_RGB565;
    pLayerInfo->src_use_color_key = 1;
    pLayerInfo->src_color_key     = 0;

    if (m_fdFB == -1) {
        m_fdFB = ::open("/dev/graphics/fb0", O_RDWR, 0);
        if (m_fdFB <= 0) {
            _ERR("[init] open /dev/graphics/fb0 failed");
            err = OPEN_FAIL;
            goto exit;
        }
    }

exit:
    if (err != OK) {
        if (buffer_va) {
            free_buffer(buffer_va, m_totalLayerMemSize);
        }
    }
    else {
        int ret = 0;
        
        memset(buffer_va, 0, m_totalLayerMemSize);
        m_activeBufferIdx = 1;  // 0 is being displayed, 1 is currently background buffer
        for (MUINT32 i = 0; i < FD_LAYER_NUM; ++ i)
        {
            m_pLayerVA[i] = buffer_va + m_layerMemSize * i;
            m_pLayerPA[i] = (void *)(buffer_pa + m_layerMemSize * i);

            ret = register_buffer(m_pLayerVA[i], m_layerMemSize);
            if (ret < 0) err = IOCTL_FAIL;
        }
    }
    
    return err;
}


Status OverlayHal::uninit()
{
    Status err = OK;
    int ret = 0;
    
    _LOG("OverlayHal::uninit()");
    
    if (m_fdFB > 0) {
        // disable layer first
        setLayerEnable(0);

        if (m_layerMemHandle != -1)
        {
            MUINT8* va = m_pLayerVA[0]; 
            for (MUINT32 i = 0; i < FD_LAYER_NUM; i++)
            {
                ret = unregister_buffer(m_pLayerVA[i]);
                if (ret < 0) err = IOCTL_FAIL;
                m_pLayerVA[i] = NULL;
                m_pLayerPA[i] = NULL;
            }
            free_buffer(va, m_totalLayerMemSize);            
        }

        ::close(m_fdFB);
        m_fdFB = -1;
    }

    return err;
}

// ---------------------------------------------------------------------------

Status OverlayHal::setLayerEnable(MUINT32 isEnabled)
{
    int ret = 0;
    layer_info *pLayerInfo = m_pLayerInfo;

    pLayerInfo->layer_enable = isEnabled;

    if (isEnabled) {
        int ret = cache_flush_buffer(pLayerInfo->src_base_addr,
                                     m_layerMemSize);
        if (ret < 0) {
            _ERR("[setLayerEnable] cache_flush_buffer failed, err: %d", ret);
            return FLUSH_CACHE_FAIL;
        }
    }
    
    ret = ::ioctl(m_fdFB, MTKFB_SET_OVERLAY_LAYER, pLayerInfo);
    if (ret < 0) {
        _ERR("[setLayerEnable] MTKFB_SET_OVERLAY_LAYER failed, err: %s, %d",
             strerror(ret), ret);
        
        return IOCTL_FAIL;
    }

    return OK;
}


MUINT32 OverlayHal::getLayerEnable()
{
    return m_pLayerInfo->layer_enable;
}


MUINT8* OverlayHal::getLayerBuffer()
{
    return m_pLayerVA[m_activeBufferIdx];
}


Status OverlayHal::clearLayerBuffer()
{
    if (m_pLayerVA[m_activeBufferIdx] != NULL) {
        memset(m_pLayerVA[m_activeBufferIdx], 0, m_layerMemSize);
    }

    return OK;
}


Status OverlayHal::flushLayerBuffer()
{
    int ret = 0;
    layer_info *pLayerInfo = m_pLayerInfo;

    pLayerInfo->src_phy_addr = m_pLayerPA[m_activeBufferIdx];
    pLayerInfo->src_base_addr = m_pLayerVA[m_activeBufferIdx];

    ret = cache_flush_buffer(pLayerInfo->src_base_addr,
                             m_layerMemSize);

    if (ret < 0) {
        _ERR("[flushLayerBuffer] cache_flush_buffer failed, err: %d", ret);
        return FLUSH_CACHE_FAIL;
    }

    ret = ::ioctl(m_fdFB, MTKFB_SET_OVERLAY_LAYER, pLayerInfo);
    if (ret < 0) {
        _ERR("[flushLayerBuffer] MTKFB_SET_OVERLAY_LAYER failed, err: %s, %d",
             strerror(ret), ret);
        return IOCTL_FAIL;
    }
    
    m_activeBufferIdx = (m_activeBufferIdx + 1) & 0x01;

    return OK;
}

// ---------------------------------------------------------------------------
//
//  M4U Dependent Function Implementations
//
// ---------------------------------------------------------------------------

#if defined(MTK_M4U_SUPPORT)

void* OverlayHal::allocate_buffer(size_t size)
{
    void *ptr = malloc(size);
    m_layerMemHandle = (int)ptr;
    return ptr;
}

void* OverlayHal::get_buffer_phys_addr()
{
    return (void *)m_layerMemHandle;
}

void OverlayHal::free_buffer(void *ptr, size_t size)
{
    free(ptr);
    m_layerMemHandle = -1;
}

int OverlayHal::register_buffer(void *ptr, size_t size)
{
    int ret = 0;
    int retry = 3; 

    struct fb_overlay_buffer_info b;
    b.src_vir_addr = (unsigned int)ptr;
    b.size         = (unsigned int)size;
 
    //@Sean, FIXME, sometimes register will fail, but retry can success. 
    //           Should find the root cause why register fail. 
    do {
        ret = ::ioctl(m_fdFB, MTKFB_REGISTER_OVERLAYBUFFER, &b);
    
        _LOG("OverlayHal::register_buffer(%d, 0x%x, %d), ret: %d", 
             m_fdFB, b.src_vir_addr, b.size, ret);
        
        if (ret < 0) {
            _ERR("[Overlay_hal] MTKFB_REGISTER_OVERLAYBUFFER failed, err: %s, %d",
                 strerror(ret), ret);
        }
    } while (--retry > 0 && ret != 0); 
    return ret;    
}

int OverlayHal::unregister_buffer(void *ptr)
{
    int ret = ::ioctl(m_fdFB, MTKFB_UNREGISTER_OVERLAYBUFFER, &ptr);

    _LOG("OverlayHal::unregister_buffer(%d, 0x%p), ret: %d", m_fdFB, ptr, ret);

    if (ret < 0) {
        _ERR("[Overlay_hal] MTKFB_UNREGISTER_OVERLAYBUFFER failed, err: %s, %d",
             strerror(ret), ret);
    }
    return ret;    
}

int OverlayHal::cache_flush_buffer(void *buffer, size_t size)
{
    M4U_STATUS_ENUM ret = M4U_STATUS_OK;

    if (!m_m4uDrv) return -1;

    ret = m_m4uDrv->m4u_cache_sync(M4U_CLNTMOD_LCDC,
                                   M4U_CACHE_FLUSH_BEFORE_HW_READ_MEM,
                                   (unsigned int)buffer, size);

    return (ret == M4U_STATUS_OK) ? 0 : -1;
}

#else   // use PMEM instead

void* OverlayHal::allocate_buffer(size_t size)
{
    return pmem_alloc(size, &m_layerMemHandle);
}

void* OverlayHal::get_buffer_phys_addr()
{
    return pmem_get_phys(m_layerMemHandle);
}

void OverlayHal::free_buffer(void *ptr, size_t size)
{
    pmem_free(ptr, size, m_layerMemHandle);
    m_layerMemHandle = -1;
}

int OverlayHal::register_buffer(void *ptr, size_t size)
{
    return 0;
}

int OverlayHal::unregister_buffer(void *ptr)
{
    return 0;
}

int OverlayHal::cache_flush_buffer(void *buffer, size_t size)
{
    return 0;
}

#endif

