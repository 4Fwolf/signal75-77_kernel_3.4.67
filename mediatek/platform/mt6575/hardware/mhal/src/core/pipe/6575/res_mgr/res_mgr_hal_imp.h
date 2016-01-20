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
#ifndef RES_MGR_HAL_IMP_H
#define RES_MGR_HAL_IMP_H
//----------------------------------------------------------------------------
#include "res_mgr_hal.h"
#include <cutils/xlog.h>
#include <utils/threads.h>
#include "mdp_drv.h"
#include "mdp_path.h"
#include "res_mgr_drv.h"
//----------------------------------------------------------------------------
using namespace android;
//----------------------------------------------------------------------------
#define LOG_MSG(fmt, arg...)    XLOGD("[%s]"fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    XLOGW("[%s]Warning(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    XLOGE("[%s]Err(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
//----------------------------------------------------------------------------
#define RES_MGR_HAL_LOCK_TIMEOUT    10000
//----------------------------------------------------------------------------
class ResMgrHalImp : public ResMgrHal
{
    protected:
        ResMgrHalImp();
        ~ResMgrHalImp();
    //
    public:
        static ResMgrHal* GetInstance(void);
        virtual void    DestroyInstance(void);
        virtual MBOOL   Init(void);
        virtual MBOOL   Uninit(void);
        virtual MBOOL   SetMode(RES_MGR_HAL_MODE_STRUCT* pModeInfo);
        virtual MBOOL   LockMdpCrz(MBOOL Lock);
        virtual MBOOL   LockMdpBrz(MBOOL Lock);
    //
    private:
        mutable Mutex mLock;
        volatile MINT32 mInitCount;
        RES_MGR_HAL_MODE_STRUCT mModeInfo;
        ResMgrDrv*  mpDrv;
        MdpPathStnr mMdpPathStnr;
        MdpPathDummyBrz mMdpDummyBrz;
        MBOOL   mLockMdpCrz;
        MBOOL   mLockMdpBrz;
};
//----------------------------------------------------------------------------
#endif

