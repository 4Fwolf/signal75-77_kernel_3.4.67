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

#ifndef __OVERLAY_HAL_H__
#define __OVERLAY_HAL_H__

typedef unsigned int  MUINT32;
typedef signed int    MINT32;
typedef unsigned char MUINT8;

struct fb_overlay_layer;
class MTKM4UDrv;

/*****************************************************************************
 * OverlayHal Class
 *****************************************************************************/

class OverlayHal
{
public:
    typedef enum {
        OK              = 0,
        NO_ERROR        = 0,
        UNKNOWN_ERROR   = 0x80000000,
        ALLOC_MEM_FAIL,
        OPEN_FAIL,
        IOCTL_FAIL,
        FLUSH_CACHE_FAIL,
        
    } STATUS;
    
public:
    OverlayHal();
    virtual ~OverlayHal();

    STATUS init(MUINT32 srcX, MUINT32 srcY, MUINT32 srcW, MUINT32 srcH,
                MUINT32 dstX, MUINT32 dstY, MUINT32 dstW, MUINT32 dstH);

    STATUS uninit();
    
    STATUS  setLayerEnable(MUINT32 isEnabled);
    MUINT32 getLayerEnable();

    MUINT8* getLayerBuffer();
    STATUS  clearLayerBuffer();
    STATUS  flushLayerBuffer();

private:
    typedef struct fb_overlay_layer layer_info;

    MINT32 m_fdFB;
    
#define FD_LAYER_NUM (2)
#define FD_LAYER_BPP (2)

    MINT32  m_layerMemHandle;
    MUINT32 m_layerMemSize;
    MUINT32 m_totalLayerMemSize;

    layer_info *m_pLayerInfo;

    MUINT32 m_activeBufferIdx;
    MUINT8 *m_pLayerVA[FD_LAYER_NUM];
    void   *m_pLayerPA[FD_LAYER_NUM];

#if defined(MTK_M4U_SUPPORT)
    MTKM4UDrv *m_m4uDrv;
#endif

private:
    //  M4U Dependent Functions

    void* allocate_buffer(size_t size);
    void* get_buffer_phys_addr();
    void free_buffer(void *ptr, size_t size);
    
    int register_buffer(void *ptr, size_t size);
    int unregister_buffer(void *ptr);
    
    int cache_flush_buffer(void *buffer, size_t size);
};


#endif  // __OVERLAY_HAL_H__

