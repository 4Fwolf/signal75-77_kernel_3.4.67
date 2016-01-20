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

//----------------------------------------------------------------------------
#ifndef VIDEO_SNAPSHOT_IMP_H
#define VIDEO_SNAPSHOT_IMP_H
//----------------------------------------------------------------------------
#include <cutils/xlog.h>
#include <utils/threads.h>
#include "VideoSnapshot.h"
#include "mdp_hal.h"
#include "ICameraIO.h"
#include <mhal/inc/MediaHal.h>
//----------------------------------------------------------------------------
using namespace android;
//----------------------------------------------------------------------------
#define LOG_MSG(fmt, arg...)    XLOGD(""          fmt,           ##arg)
#define LOG_WRN(fmt, arg...)    XLOGW("WRN(%5d):" fmt, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    XLOGE("ERR(%5d):" fmt, __LINE__, ##arg)
#define LOG_DMP(fmt, arg...)    XLOGE(""          fmt,           ##arg)
//----------------------------------------------------------------------------
#define VIDEO_SNAPSHOT_BUF_OFST_EXIF    (0)
#define VIDEO_SNAPSHOT_BUF_OFST_JPG     (VIDEO_SNAPSHOT_BUF_OFST_EXIF+2*1024)
#define VIDEO_SNAPSHOT_BUF_OFST_VSS     (VIDEO_SNAPSHOT_BUF_OFST_JPG+512*1024)
#define VIDEO_SNAPSHOT_JPG_SOI_LEN      (2)
#define VIDEO_SNAPSHOT_HW_JPG_SUPPORT   (0)
//----------------------------------------------------------------------------
typedef enum
{
    VIDEO_SNAPSHOT_THREAD_STATE_NONE,
    VIDEO_SNAPSHOT_THREAD_STATE_CREATE,
    VIDEO_SNAPSHOT_THREAD_STATE_END
}VIDEO_SNAPSHOT_THREAD_STATE_ENUM;
//----------------------------------------------------------------------------
class VideoSnapshotImp : public VideoSnapshot
{
    protected:
        VideoSnapshotImp();
        ~VideoSnapshotImp();
    //
    public:
        static VideoSnapshot* GetInstance(void);
        virtual void    DestroyInstance(void);
        virtual MINT32  Init(void);
        virtual MINT32  Uninit(void);
        virtual MINT32  Config(VIDEO_SNAPSHOT_CONFIG_STRUCT* pConfig);
        virtual MINT32  Release(void);
        virtual MINT32  TakePic(MUINT32 Rotation);
        virtual MINT32  SetFrame(MUINT32 BufAddr);
        virtual MINT32  ProcessImg(void);
        virtual MUINT32 GetAddr(VIDEO_SNAPSHOT_BUF_TYPE_ENUM BufType);
        virtual MINT32  AddExif(void);
    //
    private:
        volatile int    mUsers;
        mutable Mutex   mLock;
        MBOOL           mIsFirstFrame;
        MBOOL           mIsConfig;
        MBOOL           mIsTakePic;
        MUINT32         mBufVirAddr;
        MUINT32         mFrameVirAddr;
        MUINT32         mJpgSize;
        MUINT32         mRotation;
        ICameraIO*      mpCameraIOObj;
        ICameraIO::BuffInfo_t           mBufInfo;
        VIDEO_SNAPSHOT_CONFIG_STRUCT    mConfig;
        mHalRegisterLoopMemoryObj_t     mRegMemObj;
};
//----------------------------------------------------------------------------
#endif

