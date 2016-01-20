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
#ifndef _SYSRAM_MGR_H_
#define _SYSRAM_MGR_H_
namespace NSIspSysram
{


/*******************************************************************************
*   
*******************************************************************************/
class IspSysramDrv;
class IspSysramDrvImp;


/*******************************************************************************
*   User Info.
*******************************************************************************/
class UsrInfo
{
public:

    inline MBOOL    addNewUser(EUser_T const eUsr)
    {
        MBOOL fgRet = MFALSE;
        if  ( isDeadUsr(eUsr) )
        {
            m_u4UsrMask |= (1<<eUsr);
            fgRet = MTRUE;
        }
        return  fgRet;
    }

    inline MBOOL    removeUser(EUser_T const eUsr)
    {
        MBOOL fgRet = MFALSE;
        if  ( isAliveUsr(eUsr) )
        {
            m_u4UsrMask &= ~(1<<eUsr);
            fgRet = MTRUE;
        }
        return  fgRet;
    }

    inline MBOOL    isAliveUsr(MUINT32 const u4Usr) const
    {
        return 0 != ((1<<u4Usr) & m_u4UsrMask);
    }

    inline MBOOL    isDeadUsr(MUINT32 const u4Usr) const
    {
        return ! isAliveUsr(u4Usr);
    }

    inline MBOOL    isNoUsr() const
    {
        return  (0 == m_u4UsrMask);
    }

    inline MUINT32  getUsrMask() const
    {
        return m_u4UsrMask;
    }

public:     ////
    UsrInfo()
        : m_u4UsrMask(0)
    {}
protected:  ////    
    //  (1<<EUser_xxx) indicates whether the EUser_xxx is using.
    MUINT32         m_u4UsrMask;
};


/*******************************************************************************
*   Isp Sysram Manager.
********************************************************************************/
class IspSysramMgr : private UsrInfo
{
public:
    static IspSysramMgr&    getInstance();

protected:  ////
    IspSysramMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    MVOID           dump() const;

    MVOID*          getPhyAddr(IspSysramDrvImp*const pDrv, EUser_T const eUsr);
    MVOID*          getVirAddr(IspSysramDrvImp*const pDrv, EUser_T const eUsr);

    MERROR_ENUM     free(IspSysramDrvImp*const pDrv, EUser_T const eUsr);
    MERROR_ENUM     alloc(
        IspSysramDrvImp*const pDrv, 
        EUser_T const eUsr, 
        MUINT32 const u4BytesToAllocate, 
        MUINT32&      ru4BytesAllocated,
        MUINT32 const TimeoutMS
    );

    MERROR_ENUM     init();
    MERROR_ENUM     uninit();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Memory Operation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

    MERROR_ENUM     evaluateVA(MVOID*const pPhyAddr, MVOID*& rpVirAddr);
    MERROR_ENUM     allocPA(
        EUser_T const eUsr, 
        MUINT32 const u4BytesToAllocate, 
        MVOID*&       rpPhyAddr,
        MUINT32 const TimeoutMS
    );
    MERROR_ENUM     freePA(EUser_T const eUsr);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Autolock
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    class Autolock
    {
    public:
        Autolock()
            : m_pMutex(&IspSysramMgr::getInstance().m_mutex)
        {
            ::pthread_mutex_lock(m_pMutex);
        }
        Autolock(IspSysramMgr*const pThis)
            : m_pMutex(&pThis->m_mutex)
        {
            ::pthread_mutex_lock(m_pMutex);
        }
        ~Autolock()
        {
            ::pthread_mutex_unlock(m_pMutex);
        }
    private:
        pthread_mutex_t*const   m_pMutex;
    };

    friend  class Autolock;

private:
    pthread_mutex_t     m_mutex;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Driver Linked List
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MBOOL   registerDrv(IspSysramDrvImp*const pDrv);
    MBOOL   unregisterDrv(IspSysramDrvImp*const pDrv);

private:
    IspSysramDrvImp*    m_pDrvHead;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Reference Counting.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MERROR_ENUM     addRef();
    MERROR_ENUM     delRef();

private:
    MUINT32         m_u4RefCount;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Kernel Space.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

    MERROR_ENUM     syncSysramKDrv();
    MERROR_ENUM     mmapVA();
    MERROR_ENUM     unmapVA();

private:

    //  File descriptor of kernel-space sysram driver.
    int             m_fdSysramKDrv;

    MUINT32 const   m_u4SysramSize; // Total size in bytes of sysram space.
    MUINT8*const    m_pu8PhyBase;   // Pointer to the physical base address of sysram.
    MUINT8*         m_pu8VirBase;   // Pointer to the virtual base address of sysram.

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Memory Info.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

    struct MemInfo
    {
        MUINT32     u4SizeInBytes;
        MVOID*      pvPA;           //  Physical Address.
        MVOID*      pvVA;           //  Virtual Address.

        MemInfo()
            : u4SizeInBytes(0)
            , pvPA(NULL)
            , pvVA(NULL)
        {}
        MVOID dump() const;
    };

private:
    MemInfo     m_aMemInfo[ENumOfUsr];

};


/*******************************************************************************
*
*******************************************************************************/
};  //  NSIspSysram
#endif // _SYSRAM_MGR_H_

