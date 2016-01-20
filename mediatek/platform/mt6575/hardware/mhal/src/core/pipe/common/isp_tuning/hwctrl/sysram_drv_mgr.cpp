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
#define LOG_TAG "NSIspTuning::IspSysramDrvMgr"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/xlog.h>
//
#include "debug.h"
#include "isp_tuning.h"
#include "sysram_drv_mgr.h"
//
using namespace NSIspTuning;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspSysramDrvMgr::
IspSysramDrvMgr()
    : m_pIspSysramDrv(NULL)
{
}


IspSysramDrvMgr::
~IspSysramDrvMgr()
{
    uninit();
}


MERROR_ENUM
IspSysramDrvMgr::
init()
{
    MERROR_ENUM err = MERR_OK;

    //  Create isp sysram driver.
    if  ( m_pIspSysramDrv )
    {
        m_pIspSysramDrv->destroyInstance();
    }
    m_pIspSysramDrv = NSIspSysram::IspSysramDrv::createInstance();
    if  ( ! m_pIspSysramDrv )
    {
        MY_ERR("[init][m_pIspSysramDrv==NULL]");
        err = MERR_BAD_SYSRAM_DRV;
    }

    MY_LOG("[-init]err(%X)", err);
    return  err;
}


MERROR_ENUM
IspSysramDrvMgr::
uninit()
{
    MERROR_ENUM err = MERR_OK;

    MY_LOG("[+uninit]m_pIspSysramDrv(%p)", m_pIspSysramDrv);

    if  ( m_pIspSysramDrv )
    {
        //  destroy
        m_pIspSysramDrv->destroyInstance();
        m_pIspSysramDrv = NULL;
    }

    MY_LOG("[-uninit]err(%X)", err);
    return  err;
}


MERROR_ENUM
IspSysramDrvMgr::
autoAlloc(
    NSIspSysram::EUser_T const eUsr, 
    MUINT32 const u4BytesToAlloc, 
    MVOID*& rPA, 
    MVOID*& rVA
)
{
    MERROR_ENUM err = MERR_OK;

    //  (1) Make sure the sysram driver is open.
    if  ( ! m_pIspSysramDrv )
    {
        err = MERR_BAD_SYSRAM_DRV;
        goto lbExit;
    }

    //  (2) Try to alloc the sysram.
    {
        MUINT32 u4BytesAllocated = 0;
        NSIspSysram::MERROR_ENUM_T err_sysram = m_pIspSysramDrv->alloc(
            eUsr, u4BytesToAlloc, u4BytesAllocated
        );
        if  (
                NSIspSysram::MERR_OK != err_sysram
            ||  u4BytesToAlloc != u4BytesAllocated
            ||  NULL == m_pIspSysramDrv->getPhyAddr(eUsr)
            )
        {   //  Free it and re-alloc.
            free(eUsr);
            err_sysram = m_pIspSysramDrv->alloc(
                eUsr, u4BytesToAlloc, u4BytesAllocated
            );
            //  Again, check to see whether it is successful or not.
            if  (
                    NSIspSysram::MERR_OK != err_sysram
                ||  u4BytesToAlloc != u4BytesAllocated
                ||  NULL == m_pIspSysramDrv->getPhyAddr(eUsr)
                )
            {
                MY_ERR(
                    "[autoAlloc] fail: "
                    "(err_sysram, eUsr, u4BytesToAlloc, u4BytesAllocated, PA)=(%X %d, 0x%08X, 0x%08X, %p)"
                    , err_sysram, eUsr, u4BytesToAlloc, u4BytesAllocated, m_pIspSysramDrv->getPhyAddr(eUsr)
                );
                err = MERR_NO_SYSRAM_MEM;
                goto lbExit;
            }
        }
    }

    //  (3) Here, alloc must be successful.
    rPA = m_pIspSysramDrv->getPhyAddr(eUsr);
    rVA = m_pIspSysramDrv->getVirAddr(eUsr);
    err = MERR_OK;

    MY_LOG(
        "[-autoAlloc] (eUsr, u4BytesToAlloc, rPA, rVA)=(%d, 0x%08X, %p, %p)"
        , eUsr, u4BytesToAlloc, rPA, rVA
    );

lbExit:
    if  ( MERR_OK != err )
    {
        free(eUsr);
    }

    return  err;
}


MERROR_ENUM
IspSysramDrvMgr::
free(NSIspSysram::EUser_T const eUsr)
{
    MY_LOG("[+free] eUsr=%d", eUsr);
    m_pIspSysramDrv->free(eUsr);
    return  MERR_OK;
}

