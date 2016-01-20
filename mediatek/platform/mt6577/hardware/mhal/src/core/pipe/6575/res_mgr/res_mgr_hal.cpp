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
#define LOG_TAG "ResMgrHal"
//
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cutils/atomic.h>
//
#include "res_mgr_hal_imp.h"
//----------------------------------------------------------------------------
ResMgrHalImp::ResMgrHalImp()
{
    LOG_MSG("");
    mInitCount = 0;
    mpDrv = NULL;
    mLockMdpCrz = MFALSE;
    mLockMdpBrz = MFALSE;
}
//----------------------------------------------------------------------------
ResMgrHalImp::~ResMgrHalImp()
{
    //LOG_MSG("");
}
//-----------------------------------------------------------------------------
ResMgrHal* ResMgrHal::CreateInstance(void)
{
    return ResMgrHalImp::GetInstance();
}
//-----------------------------------------------------------------------------
ResMgrHal* ResMgrHalImp::GetInstance(void)
{
    static ResMgrHalImp Singleton;
    //
    //LOG_MSG("");
    //
    return &Singleton;
}
//----------------------------------------------------------------------------
void ResMgrHalImp::DestroyInstance(void) 
{
}
//----------------------------------------------------------------------------
MBOOL ResMgrHalImp::Init(void)
{
    MBOOL Result = MTRUE;
    RES_MGR_DRV_LOCK_STRUCT LockInfo;
    //
    Mutex::Autolock lock(mLock);
    //
    LOG_MSG("mInitCount(%d)",mInitCount);
    //
    if(mInitCount > 0)
    {
        android_atomic_inc(&mInitCount);
        goto EXIT;
    }
    //
    if(mpDrv == NULL)
    {
        mpDrv = ResMgrDrv::CreateInstance();
    }
    //
    if(mpDrv != NULL)
    {
        if(mpDrv->Init())
        {
            LockInfo.ResMask = RES_MGR_DRV_RES_ISP;
            LockInfo.PermanentMask = LockInfo.ResMask;
            LockInfo.Timeout = RES_MGR_HAL_LOCK_TIMEOUT;
            if(mpDrv->LockRes(&LockInfo))
            {
                LOG_MSG("lock ISP OK");
            }
            else
            {
                LOG_ERR("lock ISP fail");
                Result = MFALSE;
            }
        }
        else
        {
            LOG_ERR("mpDrv init fai");
            Result = MFALSE;
        }
    }
    else
    {
        LOG_ERR("mpDrv is NULL");
        Result = MFALSE;
    }
    //
    if(MdpDrvInit() < 0)
    {
        LOG_ERR("MdpDrvInit() fail");
        MdpDrvRelease(); 
        Result = MFALSE;
    }
    //
    mModeInfo.Mode = RES_MGR_HAL_MODE_NONE;
    mModeInfo.ModeSub = RES_MGR_HAL_MODE_SUB_NONE;
    mModeInfo.Dev = RES_MGR_HAL_DEV_NONE;
    //
    android_atomic_inc(&mInitCount);
    //
    EXIT:
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrHalImp::Uninit(void)
{
    MBOOL Result = MTRUE;
    //
    Mutex::Autolock lock(mLock);
    //
    LOG_MSG("mInitCount(%d)",mInitCount);
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
    if(MdpDrvRelease() < 0)
    {
        LOG_ERR("MdpDrvRelease() fail");
    }
    //
    if(mpDrv != NULL)
    {
        LOG_MSG("unlock ISP");
        mpDrv->UnlockRes(RES_MGR_DRV_RES_ISP);
        mpDrv->Uninit();
        mpDrv->DestroyInstance();
        mpDrv = NULL;
    }
    //
    EXIT:
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrHalImp::SetMode(RES_MGR_HAL_MODE_STRUCT* pModeInfo)
{
    MBOOL Result = MTRUE;
    struct MdpPathStnrParameter StnrParam;
    //
    Mutex::Autolock lock(mLock);
    //
    LOG_MSG("Mode(%d), ModeSub(%d), Dev(%d)",pModeInfo->Mode,pModeInfo->ModeSub,pModeInfo->Dev);
    //
    if(mInitCount <= 0)
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if( mModeInfo.Mode == pModeInfo->Mode &&
        mModeInfo.ModeSub == pModeInfo->ModeSub &&
        mModeInfo.Dev == pModeInfo->Dev)
    {
        LOG_MSG("Current Mode(%d), ModeSub(%d), Dev(%d) are already set.",mModeInfo.Mode,mModeInfo.ModeSub,mModeInfo.Dev);
        goto EXIT;
    }
    //ResMgrDrv
    if( pModeInfo->Dev == RES_MGR_HAL_DEV_CAM ||
        pModeInfo->Dev == RES_MGR_HAL_DEV_VT)
    {
        if(mModeInfo.Mode != pModeInfo->Mode)
        {
            if(!mpDrv->SetBWC((RES_MGR_DRV_MODE_ENUM)(pModeInfo->Mode)))
            {
                Result = MFALSE;
                goto EXIT;
            }
            //
            switch(pModeInfo->Mode)
            {
                case RES_MGR_HAL_MODE_NONE:
                {
                    Result = mpDrv->SetSysramSwitchbank(MTRUE);
                    if(!Result)
                    {
                        goto EXIT;
                    }
                    //
                    Result = mpDrv->CloseTvout(MFALSE);
                    if(!Result)
                    {
                        goto EXIT;
                    }
                    //
                    Result = mpDrv->CloseHdmi(MFALSE);
                    if(!Result)
                    {
                        goto EXIT;
                    }
                    //
                    Result = LockMdpBrz(MFALSE);
                    if(!Result)
                    {
                        goto EXIT;   
                    }
                    break;
                }
                case RES_MGR_HAL_MODE_PREVIEW_OFF:
                case RES_MGR_HAL_MODE_PREVIEW_ON:
                case RES_MGR_HAL_MODE_CAPTURE_OFF:
                case RES_MGR_HAL_MODE_CAPTURE_ON:
                case RES_MGR_HAL_MODE_VIDEO_REC_OFF:
                case RES_MGR_HAL_MODE_VIDEO_REC_ON:
                {
                    Result = mpDrv->SetSysramSwitchbank(MFALSE);
                    if(!Result)
                    {
                        goto EXIT;
                    }
                    //
                    Result = mpDrv->CloseTvout(MTRUE);
                    if(!Result)
                    {
                        goto EXIT;
                    }
                    //
                    Result = mpDrv->CloseHdmi(MTRUE);
                    if(!Result)
                    {
                        goto EXIT;
                    }
                    //
                    Result = LockMdpBrz(MTRUE);
                    if(!Result)
                    {
                        goto EXIT;   
                    }
                    break;
                }
                default:
                {
                    LOG_ERR("Unknown Mode(%d)",pModeInfo->Mode);
                    Result = MFALSE;
                    goto EXIT;
                }
            }
        }
        //
        if(mModeInfo.ModeSub != pModeInfo->ModeSub)
        {
            switch(pModeInfo->ModeSub)
            {
                case RES_MGR_HAL_MODE_SUB_NONE:
                {
                    Result = LockMdpCrz(MFALSE);
                    if(!Result)
                    {
                        goto EXIT;   
                    }
                    break;
                }
                case RES_MGR_HAL_MODE_SUB_ISP2MEM:
                {
                    switch(pModeInfo->Mode)
                    {
                        case RES_MGR_HAL_MODE_CAPTURE_ON:
                        {
                            Result = LockMdpCrz(MTRUE);
                            if(!Result)
                            {
                                goto EXIT;   
                            }
                            break;
                        }
                        
                        case RES_MGR_HAL_MODE_NONE:
                        case RES_MGR_HAL_MODE_PREVIEW_OFF:
                        case RES_MGR_HAL_MODE_PREVIEW_ON:
                        case RES_MGR_HAL_MODE_CAPTURE_OFF:
                        case RES_MGR_HAL_MODE_VIDEO_REC_OFF:
                        case RES_MGR_HAL_MODE_VIDEO_REC_ON:
                        {
                            Result = LockMdpCrz(MFALSE);
                            if(!Result)
                            {
                                goto EXIT;   
                            }
                            break;
                        }
                        default:
                        {
                            LOG_ERR("Unknown Mode(%d)",pModeInfo->Mode);
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    LOG_ERR("Unknown ModeSub(%d)",pModeInfo->ModeSub);
                    Result = MFALSE;
                    goto EXIT;
                }
            }
        }
    }
    else
    if(pModeInfo->Dev == RES_MGR_HAL_DEV_ATV)
    {
        if(mModeInfo.Mode != pModeInfo->Mode)
        {
            //Lock BRZ
            switch(pModeInfo->Mode)
            {
                case RES_MGR_HAL_MODE_NONE:
                {
                    Result = LockMdpBrz(MFALSE);
                    if(!Result)
                    {
                        goto EXIT;   
                    }
                    //
                    Result = mpDrv->CloseHdmi(MFALSE);
                    if(!Result)
                    {
                        goto EXIT;
                    }
                    break;
                }
                case RES_MGR_HAL_MODE_PREVIEW_OFF:
                case RES_MGR_HAL_MODE_PREVIEW_ON:
                case RES_MGR_HAL_MODE_CAPTURE_OFF:
                case RES_MGR_HAL_MODE_CAPTURE_ON:
                case RES_MGR_HAL_MODE_VIDEO_REC_OFF:
                case RES_MGR_HAL_MODE_VIDEO_REC_ON:
                {
                    Result = LockMdpBrz(MTRUE);
                    if(!Result)
                    {
                        goto EXIT;   
                    }
                    //
                    Result = mpDrv->CloseHdmi(MTRUE);
                    if(!Result)
                    {
                        goto EXIT;
                    }
                    break;
                }
                default:
                {
                    LOG_ERR("Unknown Mode(%d)",pModeInfo->Mode);
                    Result = MFALSE;
                    goto EXIT;
                }
            }
        }
    }
    //
    mModeInfo.Mode = pModeInfo->Mode;
    mModeInfo.ModeSub = pModeInfo->ModeSub;
    mModeInfo.Dev = pModeInfo->Dev;
    EXIT:
    LOG_MSG("Result(%d)",Result);
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrHalImp::LockMdpCrz(MBOOL Lock)
{
    MBOOL Result = MTRUE;
    struct MdpPathStnrParameter StnrParam;
    //
    //LOG_MSG("Lock(%d)",Lock);
    //
    if(mInitCount <= 0)
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(Lock)
    {
        if(mLockMdpCrz)
        {
            LOG_MSG("CRZ has been locked");
        }
        else
        {
            StnrParam.b_crz_use_line_buffer  = 0;
            StnrParam.b_prz0_use_line_buffer = 1;
            StnrParam.b_prz1_use_line_buffer = 1;
            //
            if(mMdpPathStnr.Config(&StnrParam) < 0)//Config
            {
                LOG_ERR("CRZ:Config() fail");
                Result = MFALSE;
                goto EXIT;
            }
            //
            if(mMdpPathStnr.Start(NULL) < 0)//Execute
            {
                LOG_ERR("CRZ:Start() fail");
                Result = MFALSE;
                goto EXIT;
            }
            mLockMdpCrz = MTRUE;
            LOG_MSG("CRZ locked OK");
        }
    }
    else
    {
        if(mLockMdpCrz)
        {
            if(mMdpPathStnr.End(NULL) < 0) //Release resource
            {
                LOG_ERR("CRZ:End() fail");
            }
            mLockMdpCrz = MFALSE;
            LOG_MSG("CRZ unlocked OK");
        }
        else
        {
            LOG_MSG("CRZ has been unlocked");
        }
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//----------------------------------------------------------------------------
MBOOL ResMgrHalImp::LockMdpBrz(MBOOL Lock)
{
    MBOOL Result = MTRUE;
    //
    //LOG_MSG("Lock(%d)",Lock);
    //
    if(mInitCount <= 0)
    {
        LOG_ERR("Please init first!");
        Result = MFALSE;
        goto EXIT;
    }
    //
    if(Lock)
    {
        if(mLockMdpBrz)
        {
            LOG_MSG("BRZ has been locked");
        }
        else
        {
            if(mMdpDummyBrz.Config((struct MdpPathDummyBrzParameter *)NULL) < 0)
            {
                LOG_ERR("BRZ:Config() fail");
                Result = MFALSE;
                goto EXIT;
            }
            if(mMdpDummyBrz.Start(NULL) < 0)
            {
                LOG_ERR("BRZ:Start() fail");
                mMdpDummyBrz.End(NULL);//Release resource
                Result = MFALSE;
                goto EXIT;
            }
            mLockMdpBrz = MTRUE;
            LOG_MSG("BRZ locked OK");
        }
    }
    else
    {
        if(mLockMdpBrz)
        {
            if(mMdpDummyBrz.End(NULL) < 0) //Release resource
            {
                LOG_ERR("BRZ:End() fail");
            }
            mLockMdpBrz = MFALSE;
            LOG_MSG("BRZ unlocked OK");
        }
        else
        {
            LOG_MSG("BRZ has been unlocked");
        }
    }
    //
    EXIT:
    //LOG_MSG("Result(%d)",Result);
    return Result;
}
//----------------------------------------------------------------------------

