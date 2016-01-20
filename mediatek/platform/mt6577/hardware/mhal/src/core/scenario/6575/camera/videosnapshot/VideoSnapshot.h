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

#ifndef VIDEO_SNAPSHOT_H
#define VIDEO_SNAPSHOT_H
//-----------------------------------------------------------------------------
#include <mhal/inc/camera/types.h>
#include <cam_types.h>
#include <aaa_hal_base.h>
//-----------------------------------------------------------------------------
using namespace NSCamera;
//-----------------------------------------------------------------------------
#define VIDEO_SNAPSHOT_SKIA_WIDTH       (1280)
//-----------------------------------------------------------------------------
typedef enum
{
    VIDEO_SNAPSHOT_BUF_TYPE_EXIF,
    VIDEO_SNAPSHOT_BUF_TYPE_JPG,
    VIDEO_SNAPSHOT_BUF_TYPE_VSS
}VIDEO_SNAPSHOT_BUF_TYPE_ENUM;

typedef struct
{
    MUINT32         Width;
    MUINT32         Height;
    ESensorType     SensorType;
    EDeviceId       DeviceId;
    Hal3ABase*      pHal3AObj;
    mhalCamParam_s* pCamParam;
    mHalCamObserver CB;
}VIDEO_SNAPSHOT_CONFIG_STRUCT;
//----------------------------------------------------------------------------
class VideoSnapshot
{
    protected:
        virtual ~VideoSnapshot() {};
    //
    public:
        static VideoSnapshot* CreateInstance(void);
        virtual void    DestroyInstance(void) = 0;
        virtual MINT32  Init(void) = 0;
        virtual MINT32  Uninit(void) = 0;
        virtual MINT32  Config(VIDEO_SNAPSHOT_CONFIG_STRUCT* pConfig) = 0;
        virtual MINT32  Release(void) = 0;
        virtual MINT32  TakePic(MUINT32 Rotation) = 0;
        virtual MINT32  SetFrame(MUINT32 BufAddr) = 0;
        virtual MUINT32 GetAddr(VIDEO_SNAPSHOT_BUF_TYPE_ENUM BufType) = 0;
};
//-----------------------------------------------------------------------------
#endif

