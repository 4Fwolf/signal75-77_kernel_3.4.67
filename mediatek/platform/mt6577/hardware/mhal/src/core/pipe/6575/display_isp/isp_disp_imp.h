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
#ifndef ISP_DISP_IMP_H
#define ISP_DISP_IMP_H
//----------------------------------------------------------------------------
#include "isp_disp.h"
#include <cutils/xlog.h>
#include "res_mgr_drv.h"
//-----------------------------------------------------------------------------
using namespace android;
//----------------------------------------------------------------------------
#define LOG_MSG(fmt, arg...)    XLOGD("[%s]"fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    XLOGW("[%s]Warning(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    XLOGE("[%s]Err(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
//----------------------------------------------------------------------------
#define ISP_DISP_WIDTH_MAX      (4092)
#define ISP_DISP_WIDTH_MIN      (16)
#define ISP_DISP_HEIGHT_MAX     (3000)
#define ISP_DISP_HEIGHT_MIN     (16)
//----------------------------------------------------------------------------
typedef enum
{
    ISP_DISP_STATE_CLOSE,
    ISP_DISP_STATE_OPEN,
    ISP_DISP_STATE_CONFIG
}ISP_DISP_STATE_ENUM;
//----------------------------------------------------------------------------
class IspDispImp : public IspDisp
{
    protected:
        IspDispImp();
        ~IspDispImp();
    //
    public:
        static IspDisp* GetInstance(void);
        virtual void    DestroyInstance(void);
        virtual MBOOL   CheckSize(
            MUINT32     Width,
            MUINT32     Height);
        virtual MBOOL   Lock(MUINT32 Timeout);
        virtual MBOOL   Unlock(void);
        virtual MBOOL   Init(void);
        virtual MBOOL   Uninit(void);
        virtual MBOOL   Config(ISP_DISP_CONFIG_STRUCT* pConfig);
        virtual MBOOL   Run(MBOOL En);
        virtual void    DumpReg(void);
        //
    private:
        mutable Mutex       mLock;
        volatile MINT32     mInitCount;
        IspDrv*             mpIspDrv;
        ResMgrDrv*          mpResMgrDrv;
        ISP_DISP_STATE_ENUM mState;
};
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------

