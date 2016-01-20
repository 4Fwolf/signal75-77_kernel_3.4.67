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

#define LOG_TAG "IspDisp"
//----------------------------------------------------------------------------
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include "isp_drv.h"
#include "isp_disp_imp.h"
#include <sys/ioctl.h>
#include <camera_res_mgr.h>
//-----------------------------------------------------------------------------
IspDispImp::IspDispImp()
{
    //LOG_MSG("");
    mpIspDrv = NULL;
    mpResMgrDrv = NULL;
    mState = ISP_DISP_STATE_CLOSE;
}
//-----------------------------------------------------------------------------
IspDispImp::~IspDispImp()
{
    //LOG_MSG("");
}
//-----------------------------------------------------------------------------
IspDisp* IspDisp::CreateInstance(void)
{
    return IspDispImp::GetInstance();
}
//-----------------------------------------------------------------------------
IspDisp* IspDispImp::GetInstance(void)
{
    static IspDispImp Singleton;
    //
    //LOG_MSG("");
    //
    return &Singleton;
}
//-----------------------------------------------------------------------------
void IspDispImp::DestroyInstance(void) 
{
    //LOG_MSG("");
}
//-----------------------------------------------------------------------------
MBOOL IspDispImp::CheckSize(
    MUINT32     Width,
    MUINT32     Height)
{
    if( Width < ISP_DISP_WIDTH_MIN ||
        Width > ISP_DISP_WIDTH_MAX ||
        Height < ISP_DISP_HEIGHT_MIN ||
        Height > ISP_DISP_HEIGHT_MAX ||
        (Width%2) != 0)
    {
        LOG_MSG("W(%d) or H(%d) is un-supported",Width,Height);
        return MFALSE;
    }
    //
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL IspDispImp::Lock(MUINT32 Timeout)
{
    MBOOL Result = MTRUE;
    RES_MGR_DRV_LOCK_STRUCT ResMgrLock;
    RES_MGR_DRV_CEHCK_STRUCT ResMgrCheck;
    //
    Mutex::Autolock lock(mLock);
    //
    if(mpResMgrDrv == NULL)
    {
        mpResMgrDrv = ResMgrDrv::CreateInstance();
        if(!mpResMgrDrv)
        {
            LOG_ERR("ResMgrDrv createInstance fail");
            Result = MFALSE;
            goto EXIT;
        }
    }
    //
    if(!mpResMgrDrv->Init())
    {
        LOG_ERR("ResMgrDrv init fail");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mpResMgrDrv->CheckRes(&ResMgrCheck))
    {
        //LOG_MSG("Check OK, ResMask(0x%08X), Permanent(0x%08X)",ResMgrCheck.ResMask,ResMgrCheck.PermanentMask);
        if( (ResMgrCheck.ResLockMask & RES_MGR_RES_ISP) &&
            (ResMgrCheck.PermanentMask & RES_MGR_RES_ISP))
        {
            //LOG_MSG("ISP has been occupied permanently");
            mpResMgrDrv->Uninit();
            //mpResMgrDrv->DestroyInstance();
            //mpResMgrDrv = NULL;
            Result = MFALSE;
            goto EXIT;
        }
    }
    else
    {
        LOG_ERR("Check fail");
        mpResMgrDrv->Uninit();
        //mpResMgrDrv->DestroyInstance();
        //mpResMgrDrv = NULL;
        Result = MFALSE;
        goto EXIT;
    }
    //
    ResMgrLock.ResMask = RES_MGR_RES_ISP;
    ResMgrLock.PermanentMask = 0;
    ResMgrLock.Timeout = Timeout;
    if(mpResMgrDrv->LockRes(&ResMgrLock))
    {
        //LOG_MSG("Lock OK, Timeout(%d)",Timeout);
    }
    else
    {
        LOG_ERR("Lock fail, Timeout(%d)",Timeout);
        mpResMgrDrv->Uninit();
        //mpResMgrDrv->DestroyInstance();
        //mpResMgrDrv = NULL;
        Result = MFALSE;
        goto EXIT;
    }
    //
    EXIT:
    //
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL IspDispImp::Unlock(void)
{
    MBOOL Result = MTRUE;
    //
    if(mpIspDrv != NULL)
    {
        LOG_ERR("please close ISP first");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mpResMgrDrv != NULL)
    {
        if(mpResMgrDrv->UnlockRes(RES_MGR_RES_ISP))
        {
            //LOG_MSG("Unlock OK");
        }
        else
        {
            LOG_ERR("Unlock fail");
            Result = MFALSE;
            goto EXIT;
        }
        mpResMgrDrv->Uninit();
        //mpResMgrDrv->DestroyInstance();
        //mpResMgrDrv = NULL;
    }
    //
    EXIT:
    //
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL IspDispImp::Init(void)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    //LOG_MSG("mInitCount(%d)",mInitCount);
    //
    if(mInitCount > 0)
    {
        android_atomic_inc(&mInitCount);
        goto EXIT;
    }
    //
    if(mpIspDrv == NULL)
    {
        mpIspDrv = IspDrv::createInstance();
        if(!mpIspDrv)
        {
            LOG_ERR("IspDrv createInstance fail\n");
            Result = MFALSE;
            goto EXIT;
        }
        //
        mpIspDrv->setLogMask(ISP_DRV_LOG_ERR); 
    }
    else
    {
        LOG_ERR("IspDrv exist\n");
    }
    //
    mState = ISP_DISP_STATE_OPEN;
    android_atomic_inc(&mInitCount);
    //
    EXIT:
    if(!Result)
    {
        if(mpIspDrv != NULL)
        {
            mpIspDrv->uninitPQ();
            mpIspDrv->setLogMask(ISP_DRV_LOG_MSG|ISP_DRV_LOG_WRN|ISP_DRV_LOG_ERR);
            mpIspDrv->destroyInstance();
            mpIspDrv = NULL;
        }
    }
    //
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL IspDispImp::Uninit(void)
{
    MBOOL Result = MTRUE;
    MUINT32 i;
    //
    Mutex::Autolock lock(mLock);
    //
    //LOG_MSG("mInitCount(%d)",mInitCount);
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
    //
    if(mpIspDrv != NULL)
    {
        mpIspDrv->uninitPQ();
        mpIspDrv->setLogMask(ISP_DRV_LOG_MSG|ISP_DRV_LOG_WRN|ISP_DRV_LOG_ERR);
        mpIspDrv->destroyInstance();
        mpIspDrv = NULL;
    }
    //
    EXIT:
    mState = ISP_DISP_STATE_CLOSE;
    //
    return Result;
}
//-----------------------------------------------------------------------------
MBOOL IspDispImp::Config(ISP_DISP_CONFIG_STRUCT* pConfig)
{
    MBOOL Result = MTRUE;
    //
    if(mState != ISP_DISP_STATE_OPEN)
    {
        LOG_ERR("Error state(%d)",mState);
        Result = MFALSE;
        goto EXIT;
    }
    //
    //LOG_MSG("W(%d), H(%d)",pConfig->InputSize.Width,pConfig->InputSize.Height);
    //
    if(mpIspDrv->initPQ() < 0)
    {
        LOG_ERR("mpIspDrv->initPQ() fail");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mpIspDrv->reset() < 0)
    {
        LOG_ERR("mpIspDrv->reset() fail");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mpIspDrv->sendCommand(ISP_DRV_CMD_RESET_BUF) < 0)
    {
        LOG_ERR("mpIspDrv reset buffer fail");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mpIspDrv->setTgGrabRange(0, pConfig->InputSize.Width-1, 1, pConfig->InputSize.Height) < 0)
    {
        LOG_ERR("mpIspDrv->setTgGrabRange() fail");
        Result = MFALSE;
        goto EXIT;
    }
    //

    if(mpIspDrv->setCamModulePath(2, 1, 0, 0, ISP_DRV_OUTPUT_FORMAT_YUV422, 0, 1) < 0)
    {
        LOG_ERR("mpIspDrv->setCamModulePath() fail");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mpIspDrv->setViewFinderMode(1 , 0) < 0)
    {
        LOG_ERR("mpIspDrv->setViewFinderMode() fail");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(mpIspDrv->setSensorModeCfg(0 , 0) < 0)
    {
       LOG_ERR("mpIspDrv->setSensorModeCfg() fail");
       Result = MFALSE;
       goto EXIT;
    }
    //
    mpIspDrv->setSubsampleWidth(MFALSE, 0, 0);
    mpIspDrv->setSubsampleHeight(MFALSE, 0, 0);
    //
    mState = ISP_DISP_STATE_CONFIG;
    //
    EXIT:
    //
    return Result;
}
//----------------------------------------------------------------------------
MBOOL IspDispImp::Run(MBOOL En)
{
    MBOOL Result = MTRUE;
    //
    if(En)
    {
        if(mState != ISP_DISP_STATE_CONFIG)
        {
            LOG_ERR("Error state(%d)",mState);
            Result = MFALSE;
            goto EXIT;
        }
    }
    else
    {
        if(mState != ISP_DISP_STATE_CONFIG)
        {
            goto EXIT;
        }
    }
    //
    LOG_MSG("En(%d)",En);
    //
    mpIspDrv->control(En);
    //
    EXIT:
    //
    return Result;
}
//----------------------------------------------------------------------------
void IspDispImp::DumpReg(void)
{
    //LOG_MSG("");
    //
    if(mpIspDrv != NULL)
    {
        mpIspDrv->dumpReg();
    }
    else
    {
        LOG_ERR("mpIspDrv is NULL.");
    }
}
//----------------------------------------------------------------------------

