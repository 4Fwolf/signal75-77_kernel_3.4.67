/********************************************************************************************
 *	   LEGAL DISCLAIMER
 *
 *	   (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *	   BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *	   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *	   FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *	   ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *	   INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *	   A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *	   WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *	   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *	   ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *	   NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *	   OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *	   BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *	   RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *	   FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *	   THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *	   OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "DISPLAY_ISP_TUNING"

#include <stdlib.h>
#include <stdio.h>
#include <utils/threads.h>
#include <cutils/xlog.h>
//#include <cutils/xlog.h>
#include <cutils/properties.h>
#include "MediaTypes.h"
#include "MediaAssert.h"

#include "display_ispif_mt6575.h"
#include "isp_drv.h"
#include "isp_reg.h"
#include "isp_sysram_drv.h"
#include "display_isp_tuning_if.h"

using namespace NSDisplayIspTuning;
using namespace android;

/**************************************************************************
 *						D E F I N E S / M A C R O S 					  *
 **************************************************************************/
#define DISPLAY_ISP_TUNING_DEBUG
#ifdef DISPLAY_ISP_TUNING_DEBUG
	#define DISPLAY_ISP_TUNING_TAG	     "[DISPLAY_ISP_TUNING] "
	#define DISPLAY_ISP_TUNING_LOG(fmt, arg...)	 XLOGD(DISPLAY_ISP_TUNING_TAG fmt, ##arg)
	#define DISPLAY_ISP_TUNING_ERR(fmt, arg...)	 XLOGE(DISPLAY_ISP_TUNING_TAG "Err: %5d: "fmt, __LINE__, ##arg)
#else
	#define DISPLAY_ISP_TUNING_LOG(a,...)
	#define DISPLAY_ISP_TUNING_ERR(a,...)
#endif

/*******************************************************************************
*
********************************************************************************/
DisplayIspTuningIFBase*
DisplayIspTuningIFBase::createInstance()
{
	return DisplayIspTuningIF::getInstance();
}


/*******************************************************************************
*
********************************************************************************/
DisplayIspTuningIFBase*
DisplayIspTuningIF::
getInstance()
{
//	DISPLAY_ISP_TUNING_LOG("getInstance \n");
	static DisplayIspTuningIF singleton;

	return &singleton;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
DisplayIspTuningIF::
destroyInstance()
{

}


/*******************************************************************************
*
********************************************************************************/
DisplayIspTuningIF::DisplayIspTuningIF()
    : DisplayIspTuningIFBase()
    , m_rParam(getDisplayIspParam())
    , m_rPcaLut()
    , m_pIspDrv(NULL)
    , m_pIspReg(NULL)
    , m_pSysramDrv(NULL)
    , m_PcaPA(NULL)
    , m_PcaVA(NULL)
    , m_bIsPcaSkinLutIdxChanged(MTRUE)
    , m_bIsPcaGrassLutIdxChanged(MTRUE)
    , m_bIsPcaSkyLutIdxChanged(MTRUE)
    , m_Users(0)
{

}


/*******************************************************************************
*
********************************************************************************/
DisplayIspTuningIF::~DisplayIspTuningIF()
{

}


/*******************************************************************************
*
********************************************************************************/
MINT32
DisplayIspTuningIF::init()
{
	//DISPLAY_ISP_TUNING_LOG("[%s()] - E. m_Users: %d \n", __FUNCTION__, m_Users);

	Mutex::Autolock lock(m_Lock);

	if (m_Users > 0)
	{
		DISPLAY_ISP_TUNING_LOG("%d has created \n", m_Users);
		android_atomic_inc(&m_Users);
		return MHAL_NO_ERROR;
	}

	// Create IspDrv instance.
	m_pIspDrv = IspDrv::createInstance();
	if (!m_pIspDrv)
	{
		DISPLAY_ISP_TUNING_ERR("IspDrv::createInstance fail \n");
		goto create_fail_exit;
	}

	// Init IspDrv object.
	if (m_pIspDrv->initPQ() < 0)
	{
		DISPLAY_ISP_TUNING_ERR("IspDrv::init fail \n");
		goto create_fail_exit;
	}

	// Get ISP HW addr.
	if (m_pIspDrv->sendCommand(CMD_GET_ISP_ADDR, (MINT32)&m_pIspReg) < 0)
	{
		DISPLAY_ISP_TUNING_ERR("CMD_GET_ISP_ADDR fail \n");
		goto create_fail_exit;
	}

        //Set PQ param here
        ISP_DRV_PQIndex_STRUCT stPQIndex;
        getISPParamIndex(stPQIndex.u4SkinToneIndex , stPQIndex.u4GrassToneIndex , stPQIndex.u4SkyToneIndex , 
            stPQIndex.u4YCCGOIndex , stPQIndex.u4EEIndex);
        stPQIndex.u4Reset = 0;
        if (m_pIspDrv->sendCommand(ISP_DRV_CMD_SET_PQ_PARAM , (MINT32)&stPQIndex) < 0)
        {
            DISPLAY_ISP_TUNING_ERR("CMD_SET_PQ_PARAM fail \n");
            goto create_fail_exit;
        }

    // Create SysramDrv instance
    m_pSysramDrv = NSIspSysram::IspSysramDrv::createInstance();

    if (!m_pSysramDrv) {
        DISPLAY_ISP_TUNING_ERR("IspSysramDrv::createInstance fail ");
        goto create_fail_exit;
    }

	android_atomic_inc(&m_Users);

	return MHAL_NO_ERROR;

create_fail_exit:

	// Init failed, destroy IspDrv instance.
	if (m_pIspDrv)
	{
		m_pIspDrv->destroyInstance();
		m_pIspDrv = NULL;
	}

	return MHAL_INVALID_DRIVER;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
DisplayIspTuningIF::deinit()
{
//	DISPLAY_ISP_TUNING_LOG("[%s()] - E. m_Users: %d \n", __FUNCTION__, m_Users);

	Mutex::Autolock lock(m_Lock);

	// If no more users, return directly and do nothing.
	if (m_Users <= 0)
	{
		return MHAL_NO_ERROR;
	}

	// More than one user, so decrease one User.
	android_atomic_dec(&m_Users);

	if (m_Users == 0) // There is no more User after decrease one User, then destroy IspDrv instance.
	{
		if (m_pIspDrv)
		{
		    m_pIspDrv->uninitPQ();
			m_pIspDrv->destroyInstance();
			m_pIspDrv = NULL;
		}

        if (m_pSysramDrv) {
            m_pSysramDrv->destroyInstance();
            m_pSysramDrv = NULL;
        }

		m_pIspReg = NULL;
	}
	else	// There are still some users.
	{
		DISPLAY_ISP_TUNING_LOG("Still %d users \n", m_Users);
	}

	return MHAL_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
const PRZ_T&
DisplayIspTuningIF::getPRZParam()
{
    return (reinterpret_cast<const PRZ_T&>(m_rParam.rPRZ[m_rParam.rIndex.PRZ]));
}

/*******************************************************************************
*
********************************************************************************/
MINT32
DisplayIspTuningIF::loadISPParam()
{
    //DISPLAY_ISP_TUNING_LOG("[%s()]\n", __FUNCTION__);

    //  Make sure the ISP driver is open.
    if (!m_pIspDrv)
    {
        DISPLAY_ISP_TUNING_ERR("[DisplayIspTuningIF::loadISPParam()] m_pIspDrv = NULL.");
        return  MHAL_INVALID_DRIVER;
    }

    ISP_DRV_PQIndex_STRUCT stPQIndex;
    if (m_pIspDrv->sendCommand(ISP_DRV_CMD_GET_PQ_PARAM , (MINT32)&stPQIndex) < 0)
    {
        DISPLAY_ISP_TUNING_ERR("CMD_SET_PQ_PARAM fail \n");
        return  MHAL_INVALID_CTRL_CODE;
    }
    m_rParam.rIndex.PcaSkinLut = stPQIndex.u4SkinToneIndex;
    m_rParam.rIndex.PcaGrassLut = stPQIndex.u4GrassToneIndex;
    m_rParam.rIndex.PcaSkyLut = stPQIndex.u4SkyToneIndex;
    m_rParam.rIndex.YCCGO = stPQIndex.u4YCCGOIndex;
    m_rParam.rIndex.PRZ = stPQIndex.u4EEIndex;

    // PCA
    if (isPCAEnabled())
    {
        if (allocSysram(NSIspSysram::EUsr_PCA, getPCALutSize(), m_PcaPA, m_PcaVA) != MHAL_NO_ERROR)
        {
            DISPLAY_ISP_TUNING_ERR("[DisplayIspTuningIF::loadISPParam()] fail to alloc sysram.");
            return MHAL_INVALID_RESOURCE;
        }

        mergePCALut();
        loadPCALutToSysram();
        loadSysramLutToISP();
        loadPcaParam();
    }

    // EE
    loadEEParam();

    // NR2
    loadNR2Param();

    // YCCGO
    loadYCCGOParam();

    // PCA
    if (isPCAEnabled())
    {
        //DISPLAY_ISP_TUNING_LOG("before isPCALutLoadBusy()\n");
        while (isPCALutLoadBusy())
        {
            usleep(500);
        }
        //DISPLAY_ISP_TUNING_LOG("after isPCALutLoadBusy()\n");
    }

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
DisplayIspTuningIF::unloadISPParam()
{
//    DISPLAY_ISP_TUNING_LOG("[%s()]\n", __FUNCTION__);

    // PCA
    if (isPCAEnabled())
    {
        disablePCA();
        freeSysram(NSIspSysram::EUsr_PCA);
    }

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
DisplayIspTuningIF::checkISPParamEffectiveness()
{
    return (isPCAEnabled() ||
            isNR2Enabled() ||
            isYCCGOEnabled());
}

/*******************************************************************************
*
********************************************************************************/
MINT32 DisplayIspTuningIF::setISPParamIndex(MUINT32 u4PcaSkinLutIdx,
                                            MUINT32 u4PcaGrassLutIdx,
                                            MUINT32 u4PcaSkyLutIdx,
                                            MUINT32 u4YCCGOIdx,
                                            MUINT32 u4PRZIdx)
{
    if ((u4PcaSkinLutIdx >= DISPLAY_ISP_PCA_SKIN_TBL_NUM)   ||
        (u4PcaGrassLutIdx >= DISPLAY_ISP_PCA_GRASS_TBL_NUM) ||
        (u4PcaSkyLutIdx >= DISPLAY_ISP_PCA_SKY_TBL_NUM)     ||
        (u4YCCGOIdx >= DISPLAY_ISP_YCCGO_TBL_NUM) ||
        (u4PRZIdx >= DISPLAY_ISP_PRZ_TBL_NUM)) {
        DISPLAY_ISP_TUNING_ERR("[DisplayIspTuningIF::setISPParamIndex()] Index out of range: u4PcaSkinLutIdx = %d, u4PcaGrassLutIdx = %d, u4PcaSkyLutIdx = %d, u4YCCGOIdx = %d, u4PRZIdx = %d",
            u4PcaSkinLutIdx, u4PcaGrassLutIdx, u4PcaSkyLutIdx, u4YCCGOIdx, u4PRZIdx);
        return MHAL_INVALID_PARA;
    }

    if (m_rParam.rIndex.PcaSkinLut != u4PcaSkinLutIdx) {
        m_rParam.rIndex.PcaSkinLut = u4PcaSkinLutIdx;
        m_bIsPcaSkinLutIdxChanged = MTRUE;
        DISPLAY_ISP_TUNING_LOG("[setISPParamIndex()] m_rParam.rIndex.PcaSkinLut = %d", m_rParam.rIndex.PcaSkinLut);
}

    if (m_rParam.rIndex.PcaGrassLut != u4PcaGrassLutIdx) {
        m_rParam.rIndex.PcaGrassLut = u4PcaGrassLutIdx;
        m_bIsPcaGrassLutIdxChanged = MTRUE;
        DISPLAY_ISP_TUNING_LOG("[setISPParamIndex()] m_rParam.rIndex.PcaGrassLut = %d", m_rParam.rIndex.PcaGrassLut);
}

    if (m_rParam.rIndex.PcaSkyLut != u4PcaSkyLutIdx) {
        m_rParam.rIndex.PcaSkyLut = u4PcaSkyLutIdx;
        m_bIsPcaSkyLutIdxChanged = MTRUE;
        DISPLAY_ISP_TUNING_LOG("[setISPParamIndex()] m_rParam.rIndex.PcaSkyLut = %d", m_rParam.rIndex.PcaSkyLut);
    }

    m_rParam.rIndex.YCCGO = u4YCCGOIdx;
    m_rParam.rIndex.PRZ = u4PRZIdx;

    ISP_DRV_PQIndex_STRUCT stPQIndex;
    stPQIndex.u4SkinToneIndex = u4PcaSkinLutIdx;
    stPQIndex.u4GrassToneIndex = u4PcaGrassLutIdx;
    stPQIndex.u4SkyToneIndex = u4PcaSkyLutIdx;
    stPQIndex.u4YCCGOIndex = u4YCCGOIdx;
    stPQIndex.u4EEIndex = u4PRZIdx;
    stPQIndex.u4Reset = 1;
    if(NULL == m_pIspDrv)
    {
        m_pIspDrv = IspDrv::createInstance();
        if(NULL == m_pIspDrv)
        {
            DISPLAY_ISP_TUNING_ERR("ISP driver obj is NULL \n");
            return MHAL_UNKNOWN_ERROR;
        }
    }

    if (m_pIspDrv->sendCommand(ISP_DRV_CMD_SET_PQ_PARAM , (MINT32)&stPQIndex) < 0)
    {
        DISPLAY_ISP_TUNING_ERR("CMD_SET_PQ_PARAM fail \n");
        return MHAL_INVALID_PARA;
    }

    return  MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 DisplayIspTuningIF::getISPParamIndex(MUINT32& u4PcaSkinLutIdx,
                                            MUINT32& u4PcaGrassLutIdx,
                                            MUINT32& u4PcaSkyLutIdx,
                                            MUINT32& u4YCCGOIdx,
                                            MUINT32& u4PRZIdx)
{
    u4PcaSkinLutIdx = m_rParam.rIndex.PcaSkinLut;
    u4PcaGrassLutIdx = m_rParam.rIndex.PcaGrassLut;
    u4PcaSkyLutIdx = m_rParam.rIndex.PcaSkyLut;
    u4YCCGOIdx = m_rParam.rIndex.YCCGO;
    u4PRZIdx = m_rParam.rIndex.PRZ;

    return  MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
DisplayIspTuningIF::allocSysram(
    NSIspSysram::EUser_T const eUsr,
    MUINT32 const u4BytesToAlloc,
    MVOID*& rPA,
    MVOID*& rVA
)
{
    MINT32 err = MHAL_NO_ERROR;

    //  (1) Make sure the sysram driver is open.
    if (!m_pSysramDrv)
    {
        err = MHAL_INVALID_DRIVER;
        goto lbExit;
    }

    //  (2) Try to alloc the sysram.
    {
        MUINT32 u4BytesAllocated = 0;
        NSIspSysram::MERROR_ENUM_T err_sysram = m_pSysramDrv->alloc(
            eUsr, u4BytesToAlloc, u4BytesAllocated
        );
        if  (
                NSIspSysram::MERR_OK != err_sysram
            ||  u4BytesToAlloc != u4BytesAllocated
            ||  NULL == m_pSysramDrv->getPhyAddr(eUsr)
            )
        {   //  Free it and re-alloc.
            freeSysram(eUsr);
            err_sysram = m_pSysramDrv->alloc(
                eUsr, u4BytesToAlloc, u4BytesAllocated
            );
            //  Again, check to see whether it is successful or not.
            if  (
                    NSIspSysram::MERR_OK != err_sysram
                ||  u4BytesToAlloc != u4BytesAllocated
                ||  NULL == m_pSysramDrv->getPhyAddr(eUsr)
                )
            {
                DISPLAY_ISP_TUNING_ERR("[sysram alloc] fail: (err_sysram, eUsr, u4BytesToAlloc, u4BytesAllocated, PA)=(%X %d, 0x%08X, 0x%08X, %p)"
                    , err_sysram, eUsr, u4BytesToAlloc, u4BytesAllocated, m_pSysramDrv->getPhyAddr(eUsr));
                err = MHAL_INVALID_DRIVER;
                goto lbExit;
            }
        }
    }

    //  (3) Here, alloc must be successful.
    rPA = m_pSysramDrv->getPhyAddr(eUsr);
    rVA = m_pSysramDrv->getVirAddr(eUsr);
    err = MHAL_NO_ERROR;

    //DISPLAY_ISP_TUNING_LOG("[sysram alloc] (eUsr, u4BytesToAlloc, rPA, rVA)=(%d, 0x%08X, %p, %p)", eUsr, u4BytesToAlloc, rPA, rVA);

lbExit:
    if (err != MHAL_NO_ERROR)
    {
        freeSysram(eUsr);
    }

    return err;
}


MINT32
DisplayIspTuningIF::freeSysram(NSIspSysram::EUser_T const eUsr)
{
//    DISPLAY_ISP_TUNING_LOG("[sysram free] eUsr=%d", eUsr);

    if (m_pSysramDrv)
        m_pSysramDrv->free(eUsr);

    return  MHAL_NO_ERROR;
}



