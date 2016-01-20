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
#define LOG_TAG "IspSysramMgr"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/xlog.h>
#include <fcntl.h>
#include <sys/mman.h>
//
#include "debug.h"
#include "drv_types.h"
//
#include "isp_sysram_drv.h"
#include "sysram_mgr.h"
#include "sysram_drv_imp.h"
//
#include "camera_sysram.h"
//
//  Refer to H/W spec.
#define MM_SYSRAM_BASE_PA       (0xC2000000)    //  Sysram Physical Base Address.
#define MM_SYSRAM_SIZE          (0x00040000)    //  256 KB
//Default timeout
#define SYSRAM_MGR_TIMEOUT_DEFAULT          500
#define SYSRAM_MGR_TIMEOUT_FLICKER          SYSRAM_MGR_TIMEOUT_DEFAULT
#define SYSRAM_MGR_TIMEOUT_PCA              SYSRAM_MGR_TIMEOUT_DEFAULT
#define SYSRAM_MGR_TIMEOUT_EIS              SYSRAM_MGR_TIMEOUT_DEFAULT
#define SYSRAM_MGR_TIMEOUT_LCE0             SYSRAM_MGR_TIMEOUT_DEFAULT
#define SYSRAM_MGR_TIMEOUT_LCE1             SYSRAM_MGR_TIMEOUT_DEFAULT
#define SYSRAM_MGR_TIMEOUT_SHADING          SYSRAM_MGR_TIMEOUT_DEFAULT
#define SYSRAM_MGR_TIMEOUT_DEFECT           SYSRAM_MGR_TIMEOUT_DEFAULT
//
using namespace NSIspSysram;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace
{


//  Device Name of Kernel Driver for Sysram.
#define DEVNAME_KD_SYSRAM   "/dev/camera-sysram"
//  Invalid File Descriptor Handle.
enum { INVALID_FD_HANDLE = (int)-1 };


MERROR_ENUM
getSysramKDrvUserParam(
    stSysramParam& rSysramParam, 
    EUser_T const eUsr, 
    MUINT32 const u4Bytes,
    MUINT32 const TimeoutMS
)
{
    MERROR_ENUM err = MERR_OK;

    rSysramParam.u4Alignment    = 4;
    rSysramParam.u4Size         = u4Bytes;
    rSysramParam.u4Owner        = ESysramUser_None;
    rSysramParam.u4Addr         = 0;
    rSysramParam.u4TimeoutInMS  = TimeoutMS;

    switch  (eUsr)
    {
    case EUsr_Flicker:
        rSysramParam.u4Owner = ESysramUser_FLICKER;
        rSysramParam.u4Alignment= 32;
        if(TimeoutMS == 0)
        {
            rSysramParam.u4TimeoutInMS = SYSRAM_MGR_TIMEOUT_FLICKER;
        }
        break;
    case EUsr_PCA:
        rSysramParam.u4Owner = ESysramUser_PCA;
        rSysramParam.u4Alignment= 32;
        if(TimeoutMS == 0)
        {
            rSysramParam.u4TimeoutInMS = SYSRAM_MGR_TIMEOUT_PCA;
        }
        break;
    case EUsr_EIS:
        rSysramParam.u4Owner = ESysramUser_EIS;
        rSysramParam.u4Alignment= 128;
        if(TimeoutMS == 0)
        {
            rSysramParam.u4TimeoutInMS = SYSRAM_MGR_TIMEOUT_EIS;
        }
        break;
    case EUsr_LCE0:
        rSysramParam.u4Owner = ESysramUser_LCE0;
        rSysramParam.u4Alignment= 8;
        if(TimeoutMS == 0)
        {
            rSysramParam.u4TimeoutInMS = SYSRAM_MGR_TIMEOUT_LCE0;
        }
        break;
    case EUsr_LCE1:
        rSysramParam.u4Owner = ESysramUser_LCE1;
        rSysramParam.u4Alignment= 8;
        if(TimeoutMS == 0)
        {
            rSysramParam.u4TimeoutInMS = SYSRAM_MGR_TIMEOUT_LCE1;
        }
        break;
    case EUsr_Shading_Preview:
    case EUsr_Shading_Capture:
        rSysramParam.u4Owner = ESysramUser_SHADING;
        rSysramParam.u4Alignment= 16;
        if(TimeoutMS == 0)
        {
            rSysramParam.u4TimeoutInMS = SYSRAM_MGR_TIMEOUT_SHADING;
        }
        break;
    case EUsr_Defect:
        rSysramParam.u4Owner = ESysramUser_DEFECT;
        rSysramParam.u4Alignment= 32;
        if(TimeoutMS == 0)
        {
            rSysramParam.u4TimeoutInMS = SYSRAM_MGR_TIMEOUT_DEFECT;
        }
        break;
    default:    //Shoundn't happen.
        MY_ERR("[getSysramKDrvUserParam] Invalid user = %d", eUsr);
        err = MERR_BAD_PARAM;
    }

    return err;
}


};  //  namespace


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspSysramMgr&
IspSysramMgr::
getInstance()
{
    static IspSysramMgr singleton;
    return singleton;
}


IspSysramMgr::
IspSysramMgr()
    : UsrInfo()
    , m_mutex()
    , m_pDrvHead(NULL)
    , m_u4RefCount(0)
    , m_fdSysramKDrv(INVALID_FD_HANDLE)
    , m_u4SysramSize(MM_SYSRAM_SIZE)
    , m_pu8PhyBase(reinterpret_cast<MUINT8*>(MM_SYSRAM_BASE_PA))
    , m_pu8VirBase(NULL)
    , m_aMemInfo()
{
    ::pthread_mutex_init(&m_mutex, NULL);
}


MERROR_ENUM
IspSysramMgr::
init()
{
    MERROR_ENUM err = MERR_OK;
    err = addRef();
    if  ( MERR_OK == err )
    {
        err = syncSysramKDrv();
        if  ( MERR_OK != err )
        {
            delRef();
        }
    }
    return  err;
}


MERROR_ENUM
IspSysramMgr::
uninit()
{
    MERROR_ENUM err = MERR_OK;
    err = delRef();
    if  ( MERR_OK == err )
    {
        err = syncSysramKDrv();
    }
    return  err;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
IspSysramMgr::
addRef()
{
    if  ( 0xFFFFFFFF > m_u4RefCount )
    {
        m_u4RefCount++;
        return  MERR_OK;
    }
    return  MERR_OVER_REF;
}


MERROR_ENUM
IspSysramMgr::
delRef()
{
    if  ( 0 < m_u4RefCount )
    {
        m_u4RefCount--;
        return  MERR_OK;
    }
    return  MERR_UNDER_REF;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
IspSysramMgr::
syncSysramKDrv()
{
    MERROR_ENUM err = MERR_OK;
    if  ( 0 == m_u4RefCount )
    {
        //  We can close it since: no user has initialized the driver.
        if  ( INVALID_FD_HANDLE != m_fdSysramKDrv )
        {
            unmapVA();
            ::close(m_fdSysramKDrv);
            m_fdSysramKDrv = INVALID_FD_HANDLE;
        }
    }
    else
    {
        //  We must open it since: someone has initialized the driver (i.e. m_u4RefCount>0).
        //  Try to open the kernel-space sysram driver if needed.
        if  ( INVALID_FD_HANDLE == m_fdSysramKDrv )
        {
            //  1. Kernel driver.
            m_fdSysramKDrv = ::open(DEVNAME_KD_SYSRAM, O_RDWR);
            if  ( INVALID_FD_HANDLE == m_fdSysramKDrv )
            {
                MY_ERR(
                    "[syncSysramKDrv]Cannot open kernel-space sysram driver: "
                    "%d %s", errno, strerror(errno)
                );
                err = MERR_BAD_SYSRAM_DRV;
                goto lbExit;
            }
            //  2. mmap virtual base address.
            err = mmapVA();
            if  ( MERR_OK != err )
            {
                goto lbExit;
            }
        }
    }

lbExit:
#if 0
    MY_LOG(
        "[syncSysramKDrv]:"
        "(ec, m_u4RefCount, m_u4UsrMask, m_fdSysramKDrv, m_pu8VirBase)=(0x%08X, %u, 0x%08X, %d, %p)"
        , err, m_u4RefCount, m_u4UsrMask, m_fdSysramKDrv, m_pu8VirBase
    );
#endif
    return  err;
}


MERROR_ENUM
IspSysramMgr::
mmapVA()
{
    MERROR_ENUM err = MERR_OK;

    m_pu8VirBase = reinterpret_cast<MUINT8*>(::mmap(
        NULL, 
        m_u4SysramSize, 
        (PROT_READ | PROT_WRITE), 
        MAP_SHARED, 
        m_fdSysramKDrv, 
        reinterpret_cast<off_t>(m_pu8PhyBase)
    ));

    if  ( MAP_FAILED == m_pu8VirBase )
    {
        MY_ERR(
            "[mmapVA][mmap](size, PA, errno, strerror(errno))"
            "=(0x%08X, %p, %d, %s)"
            , m_u4SysramSize, m_pu8PhyBase, errno, strerror(errno)
        );
        m_pu8VirBase = NULL;
        err = MERR_BAD_SYSRAM_DRV;
    }

    return  err;
}


MERROR_ENUM
IspSysramMgr::
unmapVA()
{
    MERROR_ENUM err = MERR_OK;

    if  ( m_pu8VirBase )
    {
        int const iRet = ::munmap(m_pu8VirBase, m_u4SysramSize);
        if  ( 0 != iRet )
        {
            MY_ERR("[unmapVA][munmap] return %d", iRet);
            err = MERR_BAD_SYSRAM_DRV;
        }
        m_pu8VirBase = NULL;
    }

    return  err;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
IspSysramMgr::
evaluateVA(MVOID*const pPhyAddr, MVOID*& rpVirAddr)
{
    MERROR_ENUM err = MERR_UNKNOWN;
    if  ( pPhyAddr >= m_pu8PhyBase && m_pu8VirBase && m_pu8PhyBase )
    {
        err = MERR_OK;
        rpVirAddr = m_pu8VirBase + (reinterpret_cast<MUINT8*>(pPhyAddr) - m_pu8PhyBase);
#if 0
        MY_LOG("[evaluateVA] (PA, VA)=(%p, %p)", pPhyAddr, rpVirAddr);
#endif
    }
    return  err;
}


MERROR_ENUM
IspSysramMgr::
allocPA(EUser_T const eUsr, MUINT32 const u4BytesToAllocate, MVOID*& rpPhyAddr, MUINT32 const TimeoutMS)
{
    MERROR_ENUM err = MERR_OK;

    //  (1) Try to get the sysram info for the user.
    stSysramParam a_SysramParam;
    err = getSysramKDrvUserParam(a_SysramParam, eUsr, u4BytesToAllocate, TimeoutMS);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

    //  (2) Try to allocate this user.
    if  (
            0 != ::ioctl(m_fdSysramKDrv, SYSRAM_X_USRALLOC_TIMEOUT, &a_SysramParam)
        ||  0 == a_SysramParam.u4Addr
        )
    {
        MY_ERR("[allocPA] ioctl(SYSRAM_X_USRALLOC_TIMEOUT) fail");
        MY_ERR(
            "stSysramParam:(u4Owner, u4Alignmen, u4Size, u4Addr, TimeoutMS)="
            "(%d, %lu, 0x%08lX, 0x%08lX, %lu)"
            , a_SysramParam.u4Owner, a_SysramParam.u4Alignment
            , a_SysramParam.u4Size, a_SysramParam.u4Addr, a_SysramParam.u4TimeoutInMS
        );
        err = MERR_NO_MEM;
        goto lbExit;
    }

    //  (3) Setup the output.
    rpPhyAddr = reinterpret_cast<MVOID*>(a_SysramParam.u4Addr);

    err = MERR_OK;

lbExit:
    return  err;
}


MERROR_ENUM
IspSysramMgr::
freePA(EUser_T const eUsr)
{
    MERROR_ENUM err = MERR_OK;

    //  (1) Try to get the sysram info for the user.
    stSysramParam a_SysramParam;
    err = getSysramKDrvUserParam(a_SysramParam, eUsr, 0, 0);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

    //  (2) Try to release sysram from kernel.
    {
        int iRet = 0;
        iRet = ::ioctl(m_fdSysramKDrv, SYSRAM_S_USRFREE, &a_SysramParam);
        if  ( 0 != iRet )
        {
            MY_ERR("[freePA]ioctl(SYSRAM_S_USRFREE) return %d", iRet);
        }
    }

    err = MERR_OK;

lbExit:
#if 0
    MY_LOG("[-freePA] (eUsr, err)=(%d, %X)", eUsr, err);
#endif
    return  err;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspSysramMgr::
dump() const
{
    MY_LOG("[dump]");
    MY_LOG("(fd, m_u4UsrMask)=(%d, 0x%08X)", m_fdSysramKDrv, m_u4UsrMask);
    MY_LOG("(m_u4SysramSize, m_pu8PhyBase, m_pu8VirBase)=(0x%08X, %p, %p)"
        , m_u4SysramSize, m_pu8PhyBase, m_pu8VirBase
    );
    for (MUINT32 u4Usr = EUsr_Begin; u4Usr < ENumOfUsr; u4Usr++)
    {
        MemInfo const& rMemInfo = m_aMemInfo[u4Usr];
        MY_LOG("usr[%d]:", u4Usr);
        rMemInfo.dump();
    }

    MY_LOG("[linked list:]");
    for (IspSysramDrvImp* p = m_pDrvHead; NULL != p; p = p->m_pDrvNext )
    {
        p->dump();
    }
}


MVOID
IspSysramMgr::
MemInfo::
dump() const
{
    MY_LOG("[MemInfo::dump] (size, PA, VA)=(0x%08X, %p, %p)", u4SizeInBytes, pvPA, pvVA);
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspSysramMgr::
registerDrv(IspSysramDrvImp*const pDrv)
{
    if  ( ! m_pDrvHead )
    {
        m_pDrvHead = pDrv;
        goto lbExit;
    }
    else
    {
        pDrv->m_pDrvNext = m_pDrvHead;
        m_pDrvHead = pDrv;
    }

lbExit:
    return  MTRUE;
}


MBOOL
IspSysramMgr::
unregisterDrv(IspSysramDrvImp*const pDrv)
{
    MBOOL fgRet = MFALSE;

#if 0
    MY_LOG("[+unregisterDrv]");
    pDrv->dump();
    dump();
#endif

    //  (1) Remove this driver from the linked list of the manager.

    //  (1.1) Check to see whether it is the head.
    if  ( pDrv == m_pDrvHead )
    {
        m_pDrvHead = m_pDrvHead->m_pDrvNext;
        fgRet = MTRUE;
        goto lbExit;
    }

    //  (1.2) Here, it is not the head.
    for (IspSysramDrvImp* p = m_pDrvHead; NULL != p->m_pDrvNext; p = p->m_pDrvNext )
    {
        if  ( pDrv == p->m_pDrvNext )
        {   //  hit and then remove.
            p->m_pDrvNext = p->m_pDrvNext->m_pDrvNext;
            fgRet = MTRUE;
            break;
        }
    }

lbExit:
    return  fgRet;
}


MVOID*
IspSysramMgr::
getPhyAddr(IspSysramDrvImp*const pDrv, EUser_T const eUsr)
{
    MVOID* pvPA = NULL;

    if  ( isAliveUsr(eUsr) && pDrv->isAliveUsr(eUsr) )
    {
        pvPA = m_aMemInfo[eUsr].pvPA;
    }

    return  pvPA;
}


MVOID*
IspSysramMgr::
getVirAddr(IspSysramDrvImp*const pDrv, EUser_T const eUsr)
{
    MVOID* pvVA = NULL;

    if  ( isAliveUsr(eUsr) && pDrv->isAliveUsr(eUsr) )
    {
        pvVA = m_aMemInfo[eUsr].pvVA;
    }

    return  pvVA;
}


MERROR_ENUM
IspSysramMgr::
free(IspSysramDrvImp*const pDrv, EUser_T const eUsr)
{
    MERROR_ENUM err = MERR_OK;

    if(eUsr == EUsr_None)
    {
        MY_ERR("[free]eUsr(%d) is invalid.",eUsr);
        return MERR_BAD_PARAM;
    }

    MemInfo& rMemInfo = m_aMemInfo[eUsr];

    //  (1) Check to see whether this manager knows the user.
    if  ( isDeadUsr(eUsr) )
    {
        err = MERR_NOT_ALLOCATED;
        goto lbExit;
    }

    //  (2) Check to see whether the user belongs to this driver.
    if  ( pDrv->isDeadUsr(eUsr) )
    {
        err = MERR_NOT_ALLOCATED;
        goto lbExit;
    }
    //  Try to remove the user from the driver.
    pDrv->removeUser(eUsr);

    //  (3) Try to release PA.
    if  ( MERR_OK != freePA(eUsr) )
    {
        dump();
    }

    //  (4) Here, everything is ok.
    removeUser(eUsr);
    rMemInfo.u4SizeInBytes = 0;
    rMemInfo.pvPA = NULL;
    rMemInfo.pvVA = NULL;

    //  (5) Delete the reference.
    err = delRef();
    if  ( MERR_OK != err )
    {   //  Shouldn't happen.
        goto lbExit;
    }

    err = MERR_OK;

lbExit:
    syncSysramKDrv();
#if 0
    MY_LOG("[-free] (eUsr, err)=(%d, %X)", eUsr, err);
    pDrv->dump();
    dump();
#endif
    return  err;
}


MERROR_ENUM
IspSysramMgr::
alloc(
    IspSysramDrvImp*const pDrv, 
    EUser_T const eUsr, 
    MUINT32 const u4BytesToAllocate, 
    MUINT32&      ru4BytesAllocated,
    MUINT32 const TimeoutMS
)
{
    MERROR_ENUM err = MERR_OK;

    if(eUsr == EUsr_None)
    {
        MY_ERR("[alloc]eUsr(%d) is invalid.",eUsr);
        return MERR_BAD_PARAM;
    }

    MemInfo& rMemInfo = m_aMemInfo[eUsr];

    MVOID* pPhyAddr = NULL; //  Physical Address.
    MVOID* pVirAddr = NULL; //  Virtual Address.

    //  (1) Check to see whether the size is valid or not.
    if  ( 0 == u4BytesToAllocate )
    {
        MY_ERR("[alloc] Invalid size to alloc = %d", u4BytesToAllocate);
        err = MERR_BAD_PARAM;
        goto lbExit;
    }

    ru4BytesAllocated = 0;

    //  (2) Check to see whether this manager has known the user.
    if  ( isAliveUsr(eUsr) )
    {
        MY_LOG(
            "[alloc] This user(%d) has been allocated before. "
            "m_fdSysramKDrv(%d); masks:(mgr, drv)=(0x%08X, 0x%08X)"
            , eUsr, m_fdSysramKDrv, m_u4UsrMask, pDrv->getUsrMask()
        );
        rMemInfo.dump();

        //  Here, this manager indeed knows the user.
        //  Now check to see whether the user belongs to this driver.
        if  ( pDrv->isAliveUsr(eUsr) )
        {   //  The user belongs to this driver.
            MY_LOG("And this user belongs to the manager and the driver.");
            err = MERR_HAS_ALLOCATED_BY_MYSELF;
            ru4BytesAllocated = rMemInfo.u4SizeInBytes;
        }
        else
        {   //  The user does not belong to this driver.
            MY_LOG("And this user belongs to the manager but not the driver.");
            MY_LOG("The driver has no authority to alloc this user again.");
            err = MERR_HAS_ALLOCATED_BY_OTHERS;
            ru4BytesAllocated = 0;
        }
        goto lbExit;
    }

    //  (3) Try to add reference before using.
    err = addRef();
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

    //  (4) Try to sync. kernel-space driver.
    err = syncSysramKDrv();
    if  ( MERR_OK != err )
    {
        //  Make sure to invoke delRef() if failure.
        delRef();
        goto lbExit;
    }

    //  (5) Try to allocate PA for this user.
    err = allocPA(eUsr, u4BytesToAllocate, pPhyAddr, TimeoutMS);
    if  ( MERR_OK != err )
    {
        //  Make sure to invoke delRef() if failure.
        delRef();
        goto lbExit;
    }

    //  (6) Try to evaluate VA for this user.
    err = evaluateVA(pPhyAddr, pVirAddr);
    if  ( MERR_OK != err )
    {   //  Shouldn't happen.
        //  Make sure to invoke delRef() if failure.
        delRef();
        goto lbExit;
    }

    //  (7) Here, everything is ok.
    pDrv->addNewUser(eUsr);
    addNewUser(eUsr);
    rMemInfo.u4SizeInBytes = u4BytesToAllocate;
    rMemInfo.pvPA = pPhyAddr;
    rMemInfo.pvVA = pVirAddr;

    //  (8) Assign the output.
    ru4BytesAllocated = rMemInfo.u4SizeInBytes;

    err = MERR_OK;

lbExit:
    syncSysramKDrv();
#if 0
    MY_LOG("[-alloc] (eUsr, err)=(%d, %X)", eUsr, err);
    pDrv->dump();
    dump();
#endif
    return  err;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

