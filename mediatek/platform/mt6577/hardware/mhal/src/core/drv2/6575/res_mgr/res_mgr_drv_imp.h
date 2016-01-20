/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef RES_MGR_DRV_IMP_H
#define RES_MGR_DRV_IMP_H
//----------------------------------------------------------------------------
#include "res_mgr_drv.h"
#include <utils/threads.h>
#include <cutils/xlog.h>
//----------------------------------------------------------------------------
using namespace android;
//----------------------------------------------------------------------------
#define LOG_MSG(fmt, arg...)    XLOGD("[%s]"fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    XLOGW("[%s]Warning(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    XLOGE("[%s]Err(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
//----------------------------------------------------------------------------
#define RES_MGR_DRV_DEVNAME             "/dev/camera-resmgr"
#define RES_MGR_DRV_DEVNAME_SYSRAM      "/dev/camera-sysram"
#define RES_MGR_DRV_DEVNAME_TVOUT       "/dev/TV-out"
#define RES_MGR_DRV_DEVNAME_BWC         "/sys/bus/platform/drivers/mem_bw_ctrl/concurrency_scenario"
#define RES_MGR_DRV_DEVNAME_HDMITX      "/dev/hdmitx"
#define RES_MGR_DRV_INIT_MAX            5

#define RES_MGR_DRV_BWC_PREVIEW_ON      "CON_SCE_CAM_PREVIEW ON"
#define RES_MGR_DRV_BWC_PREVIEW_OFF     "CON_SCE_CAM_PREVIEW OFF"
#define RES_MGR_DRV_BWC_CAPTURE_ON      "CON_SCE_CAM_CAP ON"
#define RES_MGR_DRV_BWC_CAPTURE_OFF     "CON_SCE_CAM_CAP OFF"
#define RES_MGR_DRV_BWC_VIDEO_REC_ON    "CON_SCE_VIDEO_REC ON"
#define RES_MGR_DRV_BWC_VIDEO_REC_OFF   "CON_SCE_VIDEO_REC OFF"
//----------------------------------------------------------------------------
class ResMgrDrvImp : public ResMgrDrv
{
    protected:
        ResMgrDrvImp();
        ~ResMgrDrvImp();
    //
    public:
        static ResMgrDrv* GetInstance(void);
        virtual void    DestroyInstance(void);
        virtual MBOOL   Init(void);
        virtual MBOOL   Uninit(void);
        virtual MBOOL   LockRes(RES_MGR_DRV_LOCK_STRUCT* pLockInfo);
        virtual MBOOL   UnlockRes(MUINT32 ResMask);
        virtual MBOOL   CheckRes(RES_MGR_DRV_CEHCK_STRUCT* pCheckInfo);
        virtual MBOOL   SetSysramSwitchbank(MBOOL En);
        virtual MBOOL   CloseTvout(MBOOL En);
        virtual MBOOL   SetBWC(RES_MGR_DRV_MODE_ENUM Mode);
        virtual MBOOL   CloseHdmi(MBOOL En);
        virtual MBOOL   ForceHdmiFullscreen(MBOOL En);
        virtual MBOOL   EnableLog(MUINT32 LogMask);
        virtual MBOOL   DisableLog(MUINT32 LogMask);
    //
    private:
        mutable Mutex mLock;
        volatile MINT32 mInitCount;
        MINT32 mFd;
        MINT32 mFdSysram;
        MINT32 mFdTvOut;
        MINT32 mFdBwc;
        MINT32 mFdHdmiTx;
};
//----------------------------------------------------------------------------
#endif

