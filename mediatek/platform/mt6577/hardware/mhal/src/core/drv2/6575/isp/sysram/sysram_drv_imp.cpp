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
#define LOG_TAG "IspSysramDrvImp"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/xlog.h>
//
#include "debug.h"
#include "drv_types.h"
//
#include "isp_sysram_drv.h"
#include "sysram_mgr.h"
#include "sysram_drv_imp.h"
//
using namespace NSIspSysram;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspSysramDrv*
IspSysramDrv::
createInstance()
{
    IspSysramMgr::Autolock lock();

    IspSysramDrvImp* pDrv = new IspSysramDrvImp;
    if  ( pDrv )
    {
        //  register this driver.
        if  ( ! IspSysramMgr::getInstance().registerDrv(pDrv) )
        {
            pDrv->destroyInstance();
            pDrv = NULL;
        }
    }

    return  pDrv;
}


void
IspSysramDrvImp::
destroyInstance()
{
    IspSysramMgr::Autolock lock();

    //  (1) Unregister this driver from the manager.
    if  ( IspSysramMgr::getInstance().unregisterDrv(this) )
    {
        //  (2) Release all users belongs to this driver.
        for (MUINT32 i = 0; ! isNoUsr(); i++)
        {
            if  ( isAliveUsr(i) )
            {
                IspSysramMgr::getInstance().free(this, static_cast<EUser_T>(i));
            }
        }

        delete this;
    }
    else
    {   //  Shouldn't happen unless re-entrant.
        //  I believe "this" has become a bad pointer.
        MY_LOG("[destroyInstance] this:%p should be unregistered before", this);
    }
}


IspSysramDrvImp::
IspSysramDrvImp()
    : IspSysramDrv()
    , UsrInfo()
    , m_pDrvNext(NULL)
{
}


MERROR_ENUM
IspSysramDrvImp::
init()
{
    IspSysramMgr::Autolock lock();
    return  IspSysramMgr::getInstance().init();
}


MERROR_ENUM
IspSysramDrvImp::
uninit()
{
    IspSysramMgr::Autolock lock();
    return  IspSysramMgr::getInstance().uninit();
}


MVOID*
IspSysramDrvImp::
getPhyAddr(EUser_T const eUsr)
{
    IspSysramMgr::Autolock lock();
    return  IspSysramMgr::getInstance().getPhyAddr(this, eUsr);
}


MVOID*
IspSysramDrvImp::
getVirAddr(EUser_T const eUsr)
{
    IspSysramMgr::Autolock lock();
    return  IspSysramMgr::getInstance().getVirAddr(this, eUsr);
}


MERROR_ENUM
IspSysramDrvImp::
free(EUser_T const eUsr)
{
    IspSysramMgr::Autolock lock();
    return  IspSysramMgr::getInstance().free(this, eUsr);
}


MERROR_ENUM
IspSysramDrvImp::
alloc(
    EUser_T const eUsr, 
    MUINT32 const u4BytesToAllocate, 
    MUINT32&      ru4BytesAllocated
)
{
    IspSysramMgr::Autolock lock();
    return  IspSysramMgr::getInstance().alloc(
        this, eUsr, u4BytesToAllocate, ru4BytesAllocated, 0
    );
}


MERROR_ENUM
IspSysramDrvImp::
alloc(
    EUser_T const eUsr, 
    MUINT32 const u4BytesToAllocate, 
    MUINT32&      ru4BytesAllocated,
    MUINT32 const TimeoutMS 
)
{
    IspSysramMgr::Autolock lock();
    return  IspSysramMgr::getInstance().alloc(
        this, eUsr, u4BytesToAllocate, ru4BytesAllocated, TimeoutMS
    );
}


MVOID
IspSysramDrvImp::
dump()  const
{
    MY_LOG(
        "[dump]: "
        "(this, mask, m_pDrvNext)=(%p, 0x%08X, %p)"
        , this, m_u4UsrMask, m_pDrvNext
    );
}

