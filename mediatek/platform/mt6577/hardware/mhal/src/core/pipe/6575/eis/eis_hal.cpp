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
#include <stdlib.h>
#include <stdio.h>
#include <utils/threads.h>
#include <cutils/xlog.h>

#include "MTKEIS.h"
#include "isp_sysram_drv.h"
#include "eis_drv_base.h"
#include "eis_hal.h"
#include "camera_custom_eis.h"

/*******************************************************************************
*
********************************************************************************/
#define EIS_DEBUG

#ifdef EIS_DEBUG
#define EIS_HAL_TAG             "[EIShal]"
#define EIS_LOG(fmt, arg...)    XLOGD(EIS_HAL_TAG fmt, ##arg)
#define EIS_ERR(fmt, arg...)    XLOGE(EIS_HAL_TAG "Err: %5d: "fmt, __LINE__, ##arg)
#else
#define EIS_LOG(a,...)
#define EIS_ERR(a,...)
#endif

/*******************************************************************************
*
********************************************************************************/
EisHalBase*
EisHalBase::createInstance()
{
    return EisHal::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
EisHalBase*
EisHal::
getInstance()
{
    EIS_LOG("getInstance \n");
    static EisHal singleton;

    if (singleton.init() != 0)  {
        EIS_LOG("singleton.init() fail \n");        
        return NULL;
    }    
        
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
MVOID   
EisHal::
destroyInstance() 
{    
	uninit();
}

/*******************************************************************************
*
********************************************************************************/
EisHal::EisHal():EisHalBase()
{
    mUsers = 0;
	m_pEisDrv = NULL;
    m_pEisSysram = NULL;
    m_pEisApp = NULL;
    m_i4Path = 0;
}

/*******************************************************************************
*
********************************************************************************/
EisHal::~EisHal()
{
	
}

/*******************************************************************************
*
********************************************************************************/
MINT32
EisHal::init()
{
    EIS_LOG("init - mUsers: %d \n", mUsers);

    MVOID * rPA = NULL; 
    MVOID * rVA = NULL; 

    Mutex::Autolock lock(mLock);

    if (mUsers > 0) {
        EIS_LOG("%d has created \n", mUsers);
        android_atomic_inc(&mUsers);
        return EIS_NO_ERROR;
    }

    // ---------------------------------------------------
    m_pEisDrv = EisDrvBase::createInstance();
    
    if (!m_pEisDrv) {
        EIS_ERR("EisDrv::createInstance fail \n");
        goto create_fail_exit;
    }

    // ---------------------------------------------------
    //  Create isp sysram driver.
    m_pEisSysram = NSIspSysram::IspSysramDrv::createInstance();
    
    if (!m_pEisSysram) {
        EIS_ERR("EisDrv::createInstance IspSysramDrv fail ");
        goto create_fail_exit;
    }

    sysram_alloc(NSIspSysram::EUsr_EIS, 184, rPA, rVA);

    m_pEisDrv->setStatAddr((MINT32)rPA, (MINT32)rVA);

    // ---------------------------------------------------
    m_pEisApp = MTKEIS::createInstance();

    if (!m_pEisApp) {
        EIS_ERR("EisApp::createInstance fail \n");
        goto create_fail_exit;
    }
    
    android_atomic_inc(&mUsers);

    m_pEisApp->setSensitivity((EIS_SENSI)get_EIS_Sensitivity());

    return EIS_NO_ERROR;

create_fail_exit:	

    if (m_pEisDrv) {
        m_pEisDrv->destroyInstance();
        m_pEisDrv = NULL;
    }

    if (m_pEisSysram) {
        m_pEisSysram->destroyInstance();
        m_pEisSysram = NULL;
    }

    if (m_pEisApp) {
        m_pEisApp->destroyInstance();
        m_pEisApp = NULL;
    }

    return EIS_INVALID_DRIVER;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
EisHal::uninit()
{    
    EIS_LOG("uninit - mUsers: %d \n", mUsers);

    Mutex::Autolock lock(mLock);

    if (mUsers <= 0) {
        // No more users
        return EIS_NO_ERROR;
    }
    // More than one user
    android_atomic_dec(&mUsers);
    //
    if (mUsers == 0) {

    	if (m_pEisDrv) {
        	m_pEisDrv->destroyInstance();
        	m_pEisDrv = NULL;
    	}

        sysram_free(NSIspSysram::EUsr_EIS);

        if (m_pEisSysram) {
            m_pEisSysram->destroyInstance();
            m_pEisSysram = NULL;
        } 

        if (m_pEisApp) {
            m_pEisApp->destroyInstance();
            m_pEisApp = NULL;
        }        
    }
    else {
        EIS_LOG("Still %d users \n", mUsers);
    }

    return EIS_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
EisHal::sysram_alloc(
    NSIspSysram::EUser_T const eUsr, 
    MUINT32 const u4BytesToAlloc, 
    MVOID*& rPA, 
    MVOID*& rVA
)
{
    MINT32 err = EIS_NO_ERROR;

    //  (1) Make sure the sysram driver is open.
    if (!m_pEisSysram)
    {
        err = EIS_INVALID_DRIVER;
        goto lbExit;
    }

    //  (2) Try to alloc the sysram.
    {
        MUINT32 u4BytesAllocated = 0;
        NSIspSysram::MERROR_ENUM_T err_sysram = m_pEisSysram->alloc(
            eUsr, u4BytesToAlloc, u4BytesAllocated
        );
        if  (
                NSIspSysram::MERR_OK != err_sysram
            ||  u4BytesToAlloc != u4BytesAllocated
            ||  NULL == m_pEisSysram->getPhyAddr(eUsr)
            )
        {   //  Free it and re-alloc.
            sysram_free(eUsr);
            err_sysram = m_pEisSysram->alloc(
                eUsr, u4BytesToAlloc, u4BytesAllocated
            );
            //  Again, check to see whether it is successful or not.
            if  (
                    NSIspSysram::MERR_OK != err_sysram
                ||  u4BytesToAlloc != u4BytesAllocated
                ||  NULL == m_pEisSysram->getPhyAddr(eUsr)
                )
            {
                EIS_ERR("[sysram alloc] fail: (err_sysram, eUsr, u4BytesToAlloc, u4BytesAllocated, PA)=(%X %d, 0x%08X, 0x%08X, %p)"
                    , err_sysram, eUsr, u4BytesToAlloc, u4BytesAllocated, m_pEisSysram->getPhyAddr(eUsr));
                err = EIS_INVALID_DRIVER;
                goto lbExit;
            }
        }
    }

    //  (3) Here, alloc must be successful.
    rPA = m_pEisSysram->getPhyAddr(eUsr);
    rVA = m_pEisSysram->getVirAddr(eUsr);
    err = EIS_NO_ERROR;

    EIS_LOG("[sysram alloc] (eUsr, u4BytesToAlloc, rPA, rVA)=(%d, 0x%08X, %p, %p)", eUsr, u4BytesToAlloc, rPA, rVA);

lbExit:
    if (err != EIS_NO_ERROR)
    {
        sysram_free(eUsr);
    }

    return err;
}


MINT32
EisHal::sysram_free(NSIspSysram::EUser_T const eUsr)
{
    EIS_LOG("[sysram free] eUsr=%d", eUsr);
    m_pEisSysram->free(eUsr);
    return  EIS_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
EisHal::enableEIS(eis_config_t a_sEisConfig, eis_factor_t &a_sEisFactor)
{
    app_eis_config_t sEisConfig;
    app_eis_factor_t sEisFactor;

    sEisConfig.bForce_ISP_Path = a_sEisConfig.bForce_ISP_Path;
    sEisConfig.i4CrzSize_X = a_sEisConfig.i4CrzSize_X;
    sEisConfig.i4CrzSize_Y = a_sEisConfig.i4CrzSize_Y;
    sEisConfig.i4IspSize_X = a_sEisConfig.i4IspSize_X;
    sEisConfig.i4IspSize_Y = a_sEisConfig.i4IspSize_Y;
    sEisConfig.i4TarSize_X = a_sEisConfig.i4TarSize_X;
    sEisConfig.i4TarSize_Y = a_sEisConfig.i4TarSize_Y;
    
    m_pEisApp->enableEIS(sEisConfig, sEisFactor);

    if (sEisConfig.bForce_ISP_Path == TRUE)
    {    
        m_i4Path = 0;                
        m_pEisDrv->set0024H(1, sEisConfig.i4IspSize_X, sEisConfig.i4IspSize_Y);
    }
    else
    {
        if (a_sEisConfig.i4TarSize_Y > 260)   {m_i4Path = 1;}
        else                                  {m_i4Path = 0;}
        
        m_pEisDrv->set0024H(0, 0, 0);        
    }

    if (m_i4Path == 0)
    {
        // Path: before CRZ    
        m_pEisDrv->AutoConfig(0, a_sEisConfig.i4IspSize_X, a_sEisConfig.i4IspSize_Y);
    }
    else
    {
        // Path: after CRZ
        m_pEisDrv->AutoConfig(1, a_sEisConfig.i4CrzSize_X, a_sEisConfig.i4CrzSize_Y);
    }

    m_pEisDrv->enableEIS(1);
    m_pEisDrv->dumpReg();      

    //a_sEisFactor = (eis_factor_t)sEisFactor;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
EisHal::disableEIS()
{
    m_pEisDrv->enableEIS(0);
    m_pEisApp->disableEIS();
}

/*******************************************************************************
*
********************************************************************************/
MINT32
EisHal::doEIS(MINT32 &a_i4CMV_X, MINT32 &a_i4CMV_Y)
{
    MINT32 err = EIS_NO_ERROR;

    MINT32 i4DIV_H, i4DIV_V;
    eis_stat_t sEIS_Stat;
    eis_ori_stat_t sEIS_OriStat;

    m_pEisDrv->getStat(sEIS_Stat);
    m_pEisDrv->getOriStat(sEIS_OriStat);
    m_pEisDrv->getDIVinfo(i4DIV_H, i4DIV_V);

    m_pEisApp->setEISStat(sEIS_Stat);
    m_pEisApp->setEISOriStat(sEIS_OriStat);
    m_pEisApp->setDIVinfo(i4DIV_H, i4DIV_V);    

    err = m_pEisApp->handleEIS(a_i4CMV_X, a_i4CMV_Y);
        
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
EisHal::getHWGMV(MINT32 &a_i4GMV_X, MINT32 &a_i4GMV_Y)
{
    m_pEisDrv->getHW_GMV(a_i4GMV_X, a_i4GMV_Y);
}

/*******************************************************************************
*
********************************************************************************/
MVOID
EisHal::getSWGMV(MINT32 &a_i4GMV_X, MINT32 &a_i4GMV_Y)
{
    m_pEisApp->getSWGMV(a_i4GMV_X, a_i4GMV_Y);
}

/*******************************************************************************
*
********************************************************************************/
MVOID
EisHal::getEISStat(eis_stat_t &a_sEIS_Stat)
{
    m_pEisDrv->getStat(a_sEIS_Stat);
}

