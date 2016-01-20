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
#define LOG_TAG "ResMgrDrv"
//
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cutils/atomic.h>
#include <sys/ioctl.h>
#include <camera_res_mgr.h>

#warning [FIXME]KK_Migration[Marx]
#if 0
#include <tv_out.h>
#endif

#include <camera_sysram.h>
#include <linux/hdmitx.h>
//
#include "res_mgr_drv_imp.h"
//----------------------------------------------------------------------------
ResMgrDrvImp::ResMgrDrvImp()
{
    //LOG_MSG("");
    mInitCount = 0;
    mFd = -1;
    mFdSysram = -1;
    mFdTvOut = -1;
    mFdBwc = -1;
    mFdHdmiTx = -1;
}
//----------------------------------------------------------------------------
ResMgrDrvImp::~ResMgrDrvImp()
{
    //LOG_MSG("");
}
//-----------------------------------------------------------------------------
ResMgrDrv* ResMgrDrv::CreateInstance(void)
{
    return ResMgrDrvImp::GetInstance();
}
//-----------------------------------------------------------------------------
ResMgrDrv* ResMgrDrvImp::GetInstance(void)
{
    static ResMgrDrvImp Singleton;
    //
    //LOG_MSG("");
    //
    return &Singleton;
}
//----------------------------------------------------------------------------
void ResMgrDrvImp::DestroyInstance(void) 
{
}
//----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::Init(void)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mInitCount >= RES_MGR_DRV_INIT_MAX)
    {
        LOG_MSG("mInitCount(%d)",mInitCount);
    }
    //
    if(mInitCount > 0)
    {
        android_atomic_inc(&mInitCount);
        goto EXIT;
    }
    //ResMgr
    if(mFd < 0)
    {
        mFd = open(RES_MGR_DRV_DEVNAME, O_RDWR, 0);
        if(mFd < 0)
        {
            LOG_ERR("ResMgr kernel open fail, errno(%d):%s",errno,strerror(errno));
            Result = MFALSE;
            goto EXIT;
        }
    }
    else
    {
        LOG_MSG("ResMgr kernel is opened already");
    }
    //
    android_atomic_inc(&mInitCount);
    //
    EXIT:
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::Uninit(void)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mInitCount >= RES_MGR_DRV_INIT_MAX)
    {
        LOG_MSG("mInitCount(%d)",mInitCount);
    }
    //
    if(mInitCount <= 0)
    {
        goto EXIT;
    }
    //
    android_atomic_dec(&mInitCount);
    //
    if(mInitCount > 0)
    {
        goto EXIT;
    }
    //ResMgr
    if(mFd >= 0)
    {
        close(mFd);
        mFd = -1;
    }
    //SYSRAM
    if(mFdSysram >= 0)
    {
        close(mFdSysram);
        mFdSysram = -1;
    }
    //TV Out
    if(mFdTvOut >= 0)
    {
        close(mFdTvOut);
        mFdTvOut = -1;
    }
    //BWC
    if(mFdBwc >= 0)
    {
        close(mFdBwc);
        mFdBwc = -1;
    }
    //HDMI TX
    if(mFdHdmiTx >= 0)
    {
        close(mFdHdmiTx);
        mFdHdmiTx = -1;
    }
    //
    EXIT:
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::LockRes(RES_MGR_DRV_LOCK_STRUCT* pLockInfo)
{
    MBOOL Result = MTRUE;
    RES_MGR_RES_LOCK_STRUCT ResLock;
    //
    //LOG_MSG("ModeMask(0x%08X), Timeount(%d)",pLockInfo->ResMask,pLockInfo->Timeout);
    //
    if(mInitCount <= 0)
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mFd >= 0)
    {
        ResLock.ResMask = pLockInfo->ResMask;
        ResLock.PermanentMask = pLockInfo->PermanentMask;
        ResLock.Timeout = pLockInfo->Timeout;
        if(ioctl(mFd, RES_MGR_RES_LOCK, &ResLock) == 0)
        {
            LOG_MSG("Lock OK, ModeMask(0x%08X), Timeount(%d)",pLockInfo->ResMask,pLockInfo->Timeout);
        }
        else
        {
            LOG_ERR("Lock fail, ModeMask(0x%08X), Timeount(%d)",pLockInfo->ResMask,pLockInfo->Timeout);
            Result = MFALSE;
        }
    }
    else
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::UnlockRes(MUINT32 ResMask)
{
    MBOOL Result = MTRUE;
    //
    //LOG_MSG("ModeMask(0x%08X)",ResMask);
    //
    if(mInitCount <= 0)
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mFd >= 0)
    {
        if(ioctl(mFd, RES_MGR_RES_UNLOCK, &ResMask) == 0)
        {
            LOG_MSG("Unlock OK, ModeMask(0x%08X)",ResMask);
        }
        else
        {
            LOG_ERR("Unlock fail, ModeMask(0x%08X)",ResMask);
            Result = MFALSE;
        }
    }
    else
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::CheckRes(RES_MGR_DRV_CEHCK_STRUCT* pCheckInfo)
{
    MBOOL Result = MTRUE;
    RES_MGR_RES_CHECK_STRUCT Check;
    //
    //LOG_MSG("");
    //
    if(mInitCount <= 0)
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mFd >= 0)
    {
        if(ioctl(mFd, RES_MGR_RES_CHECK, &Check) == 0)
        {
            //LOG_MSG("Check OK, ResMask(0x%08X), Permanent(0x%08X)",Check.ResLockMask,Check.PermanentMask);
            pCheckInfo->ResLockMask = Check.ResLockMask;
            pCheckInfo->PermanentMask = Check.PermanentMask;
        }
        else
        {
            LOG_ERR("Check fail");
            Result = MFALSE;
        }
    }
    else
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::SetSysramSwitchbank(MBOOL En)
{
    MBOOL Result = MTRUE;
    //
    LOG_MSG("En(%d)",En);
    //
    if(mInitCount <= 0)
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mFdSysram < 0)
    {
        mFdSysram = open(RES_MGR_DRV_DEVNAME_SYSRAM, O_RDWR, 0);
        if(mFdSysram < 0)
        {
            LOG_WRN("Sysram kernel open fail, errno(%d):%s",errno,strerror(errno));
        }
    }
    //
    if(mFdSysram >= 0)
    {
        if(ioctl(mFdSysram, SYSRAM_S_SWITCH_BANK, &En) < 0)
        {
            LOG_ERR("Sysram ioctl error");
            Result = MFALSE;
        }
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::CloseTvout(MBOOL En)
{
    MBOOL Result = MTRUE;
    //
    LOG_MSG("En(%d)",En);
    //
    if(mInitCount <= 0)
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
        goto EXIT;
    }
    //
#warning [FIXME]KK_Migration[Marx]
#if 0
    if(mFdTvOut < 0)
    {
        mFdTvOut = open(RES_MGR_DRV_DEVNAME_TVOUT, O_RDONLY, 0);
        if(mFdTvOut < 0)
        {
            LOG_WRN("TvOut kernel open fail, errno(%d):%s",errno,strerror(errno));
        }
    }
#endif
    //
    if(mFdTvOut >= 0)
    {
        if(En)
        {
#warning [FIXME]KK_Migration[SeanC]
#if 0 
            if(ioctl(mFdTvOut, TVOUT_FORCE_CLOSE, 0) < 0)
            {
                LOG_ERR("TvOut ioctl error");
                Result = MFALSE;
            }
#endif 
        }
        else
       {
#warning [FIXME]KK_Migration[SeanC]
#if 0
            if(ioctl(mFdTvOut, TVOUT_RESTORE_OPEN, 0) < 0)
            {
                LOG_ERR("TvOut ioctl error");
                Result = MFALSE;
            }
#endif
        }
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::SetBWC(RES_MGR_DRV_MODE_ENUM Mode)
{
    MBOOL Result = MTRUE;
    const char* pBwcString = NULL;
    //
    LOG_MSG("Mode(%d)",Mode);
    //
    if(mInitCount <= 0)
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
        goto EXIT;
    }
    //
    switch(Mode)
    {
        case RES_MGR_DRV_MODE_NONE:
        {
            //Do nothing.
            pBwcString = NULL;
            break;
        }
        case RES_MGR_DRV_MODE_PREVIEW_OFF:
        {
            pBwcString = RES_MGR_DRV_BWC_PREVIEW_OFF;
            break;
        }
        case RES_MGR_DRV_MODE_PREVIEW_ON:
        {
            pBwcString = RES_MGR_DRV_BWC_PREVIEW_ON;
            break;
        }
        case RES_MGR_DRV_MODE_CAPTURE_OFF:
        {
            pBwcString = RES_MGR_DRV_BWC_CAPTURE_OFF;
            break;
        }
        case RES_MGR_DRV_MODE_CAPTURE_ON:
        {
            pBwcString = RES_MGR_DRV_BWC_CAPTURE_ON;
            break;
        }
        case RES_MGR_DRV_MODE_VIDEO_REC_OFF:
        {
            pBwcString = RES_MGR_DRV_BWC_VIDEO_REC_OFF;
            break;
        }
        case RES_MGR_DRV_MODE_VIDEO_REC_ON:
        {
            pBwcString = RES_MGR_DRV_BWC_VIDEO_REC_ON;
            break;
        }
        default:
        {
            LOG_ERR("Unknown Mode(%d)",Mode);
            pBwcString = NULL;
            Result = MFALSE;
            break;
        }
    }
    //
    if(pBwcString == NULL)
    {
        goto EXIT;
    }
    //
    if(mFdBwc < 0)
    {
        mFdBwc = open(RES_MGR_DRV_DEVNAME_BWC, O_RDWR, 0);
        if(mFdBwc < 0)
        {
            LOG_WRN("BWC open fail, errno(%d):%s",errno,strerror(errno));
        }
    }
    //
    if(mFdBwc >= 0)
    {
        if(write(mFdBwc, pBwcString, strlen(pBwcString)) < 0)
        {
            LOG_ERR("BWC write error");
            Result = MFALSE;
        }
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::CloseHdmi(MBOOL En)
{
    MBOOL Result = MTRUE;
    //
    LOG_MSG("En(%d)",En);
    //
    if(mInitCount <= 0)
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mFdHdmiTx < 0)
    {      
        mFdHdmiTx = open(RES_MGR_DRV_DEVNAME_HDMITX, O_RDONLY, 0);
        if(mFdHdmiTx < 0)
        {
            LOG_WRN("Hdmi Tx kernel open fail, errno(%d):%s",errno,strerror(errno));
        } 
    }
    //
    if(mFdHdmiTx >= 0)
    {
        if(En)
        {
            if(ioctl(mFdHdmiTx, MTK_HDMI_FORCE_CLOSE, 0) < 0)
            {
                LOG_ERR("Hdmi Tx ioctl error");
                Result = MFALSE;
            }
        }
        else
        {
            if(ioctl(mFdHdmiTx, MTK_HDMI_FORCE_OPEN, 0) < 0)
            {
                LOG_ERR("Hdmi Tx ioctl error");
                Result = MFALSE;
            }
        }
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::ForceHdmiFullscreen(MBOOL En)
{
    MBOOL Result = MTRUE;
    //
    LOG_MSG("En(%d)",En);
    //
    if(mInitCount <= 0)
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mFdHdmiTx < 0)
    {
        mFdHdmiTx = open(RES_MGR_DRV_DEVNAME_HDMITX, O_RDONLY, 0);
        if(mFdHdmiTx < 0)
        {
            LOG_WRN("Hdmi Tx kernel open fail, errno(%d):%s",errno,strerror(errno));
        }
    }
    //
    if(mFdHdmiTx >= 0)
    {
        if(En)
        {
            if(ioctl(mFdHdmiTx, MTK_HDMI_FORCE_FULLSCREEN_ON, 0) < 0)
            {
                LOG_ERR("Hdmi Tx ioctl error");
                Result = MFALSE;
            }
        }
        else
        {
            if(ioctl(mFdHdmiTx, MTK_HDMI_FORCE_FULLSCREEN_OFF, 0) < 0)
            {
                LOG_ERR("Hdmi Tx ioctl error");
                Result = MFALSE;
            }
        }
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::EnableLog(MUINT32 LogMask)
{
    MBOOL Result = MTRUE;
    //
    //LOG_MSG("LogMask(0x%08X)",LogMask);
    //
    if(mInitCount <= 0)
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mFd >= 0)
    {
        if(ioctl(mFd, RES_MGR_LOG_ENABLE, &LogMask) == 0)
        {
            LOG_MSG("Enable Log OK, LogMask(0x%08X)",LogMask);
        }
        else
        {
            LOG_ERR("Enable Log fail, LogMask(0x%08X)",LogMask);
            Result = MFALSE;
        }
    }
    else
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrDrvImp::DisableLog(MUINT32 LogMask)
{
    MBOOL Result = MTRUE;
    //
    //LOG_MSG("LogMask(0x%08X)",LogMask);
    //
    if(mInitCount <= 0)
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mFd >= 0)
    {
        if(ioctl(mFd, RES_MGR_LOG_DISABLE, &LogMask) == 0)
        {
            LOG_MSG("Disable log OK, LogMask(0x%08X)",LogMask);
        }
        else
        {
            LOG_ERR("Disable log fail, LogMask(0x%08X)",LogMask);
            Result = MFALSE;
        }
    }
    else
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//----------------------------------------------------------------------------

