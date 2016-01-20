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
#define LOG_TAG "NSIspTuning::IspDrvMgrCtx"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/xlog.h>
//
#include "debug.h"
//
#include "isp_drv.h"
#include "isp_reg.h"
#include "ispdrvmgr.h"
//
using namespace NSIspTuning;


/*******************************************************************************
* ISP Driver Manager Context
*******************************************************************************/
class IspDrvMgrCtx : public IspDrvMgr
{
    friend  IspDrvMgr& IspDrvMgr::getInstance();
protected:  ////    Data Members.
    IspDrv*         m_pIspDrv;
    isp_reg_t*      m_pIspReg;

private:    ////    Ctor/Dtor
    IspDrvMgrCtx();
    ~IspDrvMgrCtx();

public:     ////    Interfaces.
    virtual volatile void*  getIspReg() const   { return m_pIspReg; }
    virtual MBOOL           readRegs(reg_t*const pRegs, MUINT32 const count);
    virtual MBOOL           writeRegs(reg_t*const pRegs, MUINT32 const count);
    virtual MERROR_ENUM_T   init();
    virtual MERROR_ENUM_T   uninit();
    virtual MERROR_ENUM_T   reinit();

};


IspDrvMgr&
IspDrvMgr::
getInstance()
{
    static IspDrvMgrCtx singleton;
    return singleton;
}


IspDrvMgrCtx::
IspDrvMgrCtx()
    : IspDrvMgr()
    , m_pIspDrv(NULL)
    , m_pIspReg(NULL)
{
}


IspDrvMgrCtx::
~IspDrvMgrCtx()
{
}


IspDrvMgr::MERROR_ENUM_T
IspDrvMgrCtx::
init()
{
    m_pIspDrv = IspDrv::createInstance();
    return  (NULL != m_pIspDrv)
        ?   IspDrvMgr::MERR_OK
        :   IspDrvMgr::MERR_BAD_ISP_DRV;
}


IspDrvMgr::MERROR_ENUM_T
IspDrvMgrCtx::
uninit()
{
    if  ( m_pIspDrv )
    {
        m_pIspDrv->destroyInstance();
        m_pIspDrv = NULL;
    }
    return  IspDrvMgr::MERR_OK;
}


IspDrvMgr::MERROR_ENUM_T
IspDrvMgrCtx::
reinit()
{
    IspDrvMgr::MERROR_ENUM_T err = IspDrvMgr::MERR_OK;

	int const ec = m_pIspDrv->sendCommand   (
	    CMD_GET_ISP_ADDR, (int)&m_pIspReg
    );
    if  ( ec < 0 || ! m_pIspReg )
    {
        err = IspDrvMgr::MERR_BAD_ISP_ADDR;
        MY_LOG(
            "[reinit][IspDrv][CMD_GET_ISP_ADDR]"
            "(m_pIspDrv, m_pIspReg, ec)=(%p, %p, %d)"
            , m_pIspDrv, m_pIspReg, ec
        );
        goto lbExit;
    }

    err = IspDrvMgr::MERR_OK;
lbExit:
    return  err;
}


MBOOL
IspDrvMgrCtx::
readRegs(reg_t*const pRegs, MUINT32 const count)
{
    if  ( ! m_pIspDrv )
        return  MFALSE;
    return  (m_pIspDrv->readRegs(pRegs, count) < 0) ? MFALSE : MTRUE;
}


MBOOL
IspDrvMgrCtx::
writeRegs(reg_t*const pRegs, MUINT32 const count)
{
    if  ( ! m_pIspDrv )
        return  MFALSE;
    return  (m_pIspDrv->writeRegs(pRegs, count) < 0) ? MFALSE : MTRUE;
}

