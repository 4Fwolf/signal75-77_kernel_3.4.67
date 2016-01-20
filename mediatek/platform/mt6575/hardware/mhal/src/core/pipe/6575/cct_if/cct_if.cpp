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
#define LOG_TAG "CCTIF"

//
#include <utils/Errors.h>
#include <cutils/xlog.h>
//
#include "pipe_types.h"
//
#include "cct_feature.h"
#include "sensor_drv.h"
#include "nvram_drv.h"
#include "isp_drv.h"
#include "aaa_hal_base.h"
#include "isp_hal.h"
//
#include "cct_if.h"
#include "cct_imp.h"
//


/*******************************************************************************
*
********************************************************************************/
#define MY_LOG(fmt, arg...)    XLOGD(fmt, ##arg)
#define MY_ERR(fmt, arg...)    XLOGE("Err: %5d: "fmt, __LINE__, ##arg)

/*******************************************************************************
*
********************************************************************************/
CCTIF* CCTIF::createInstance()
{
    return  new CctImp;
}

void CctImp::destroyInstance()
{
    delete this;
}

/*******************************************************************************
*
********************************************************************************/
CctImp::CctImp()
    : CCTIF()
    , m_pCctCtrl(NULL)
{
    MY_LOG("[CCTIF] E\n");
    m_pHal3AObj = NULL; 
    m_pNvramDrvObj = NvramDrvBase::createInstance();
    m_pSensorDrvObj = SensorDrv::createInstance(SENSOR_MAIN);
    m_pIspDrvObj = IspDrv::createInstance();
}

/*******************************************************************************
*
********************************************************************************/
CctImp::~CctImp()
{
    MY_LOG("[~CCTIF] E\n");

    if (m_pHal3AObj) {
        m_pHal3AObj->destroyInstance();
        m_pHal3AObj = NULL;
    }

    if (m_pNvramDrvObj) {
        m_pNvramDrvObj->destroyInstance();
        m_pNvramDrvObj = NULL;
    }

    if (m_pSensorDrvObj) {
        m_pSensorDrvObj->destroyInstance();
        m_pSensorDrvObj = NULL;
    }

    if (m_pIspDrvObj) {
        m_pIspDrvObj->destroyInstance();
        m_pIspDrvObj = NULL;
    }
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::init(IspHal*const pIspHal, MINT32 sensorType)
{
    if  ( m_pCctCtrl )
    {
        MY_ERR("[CctImp::init] m_pCctCtrl != NULL before init()\n");
        return  -1;
    }
    //
    //  FIXME: DUAL_CAMERA_MAIN_SENSOR
    m_pCctCtrl = CctCtrl::createInstance((CAMERA_DUAL_CAMERA_SENSOR_ENUM)sensorType, pIspHal);
    if  ( ! m_pCctCtrl )
    {
        MY_ERR("[CctImp::init] m_pCctCtrl == NULL\n");
        return  -1;
    }
    return  0;
}

MINT32 CctImp::uninit()
{
    if  ( m_pCctCtrl )
    {
        m_pCctCtrl->destroyInstance();
        m_pCctCtrl = NULL;
    }
    return  0;
}

/*******************************************************************************
*
********************************************************************************/
static
MUINT32
getSensorID(SensorDrv& rSensorDrv, CAMERA_DUAL_CAMERA_SENSOR_ENUM const eSensorEnum)
{
    switch  ( eSensorEnum )
    {
    case DUAL_CAMERA_MAIN_SENSOR:
        return  rSensorDrv.getMainSensorID();
    case DUAL_CAMERA_SUB_SENSOR:
        return  rSensorDrv.getSubSensorID();
    default:
        break;
    }
    return  -1;
}

CctCtrl*
CctCtrl::
createInstance(CAMERA_DUAL_CAMERA_SENSOR_ENUM const eSensorEnum, IspHal*const pIspHal)
{
    if  ( ! pIspHal )
    {
        MY_ERR("[CctCtrl::createInstance] pIspHal == NULL");
        return  NULL;
    }

    CctCtrl* pCctCtrl = NULL;
    //
    //  Driver
    NvramDrvBase*const  pNvramDrv   = NvramDrvBase::createInstance();
    IspDrv*const        pIspDrv     = IspDrv::createInstance();
    SensorDrv*const     pSensorDrv  = SensorDrv::createInstance(SENSOR_MAIN|SENSOR_SUB);
    //
    //  Buf I/F
    NSNvram::BufIF<NVRAM_CAMERA_ISP_PARAM_STRUCT>*const
        pBufIf_ISP = pNvramDrv->getBufIF<NVRAM_CAMERA_ISP_PARAM_STRUCT>();
    NSNvram::BufIF<NVRAM_CAMERA_SHADING_STRUCT>*const
        pBufIf_SD  = pNvramDrv->getBufIF<NVRAM_CAMERA_SHADING_STRUCT>();
    NSNvram::BufIF<NVRAM_CAMERA_DEFECT_STRUCT>*const
        pBufIf_DF  = pNvramDrv->getBufIF<NVRAM_CAMERA_DEFECT_STRUCT>();
    //
    if  (
            pNvramDrv
        &&  pIspDrv
        &&  pSensorDrv
        &&  pBufIf_ISP
        &&  pBufIf_SD
        &&  pBufIf_DF
        )
    {
        MUINT32 const u4SensorID = getSensorID(*pSensorDrv, eSensorEnum);
        //
        //  Buffers.
        NVRAM_CAMERA_ISP_PARAM_STRUCT*  pBuf_ISP = pBufIf_ISP->getRefBuf(eSensorEnum, u4SensorID);
        NVRAM_CAMERA_SHADING_STRUCT*    pBuf_SD  = pBufIf_SD ->getRefBuf(eSensorEnum, u4SensorID);
        NVRAM_CAMERA_DEFECT_STRUCT*     pBuf_DF  = pBufIf_DF ->getRefBuf(eSensorEnum, u4SensorID);
        //
        pCctCtrl = new CctCtrl(
            eSensorEnum, u4SensorID, 
            pNvramDrv, pSensorDrv, pIspDrv, 
            pIspHal, 
            pBufIf_ISP, pBufIf_SD, pBufIf_DF, 
            pBuf_ISP, pBuf_SD, pBuf_DF
        );
    }

    if  ( ! pCctCtrl )
    {   //  Fail. Free all resources.
        if  ( pNvramDrv )
            pNvramDrv->destroyInstance();
        if  ( pIspDrv )
            pIspDrv->destroyInstance();
        if  ( pSensorDrv )
            pSensorDrv->destroyInstance();
    }

    return  pCctCtrl;
}

void
CctCtrl::
destroyInstance()
{
    delete  this;
}

CctCtrl::
CctCtrl(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM const eSensorEnum, 
    MUINT32 const u4SensorID, 
    NvramDrvBase*const  pNvramDrv, 
    SensorDrv*const     pSensorDrv, 
    IspDrv*const        pIspDrv, 
    IspHal*const        pIspHal, 
    NSNvram::BufIF<NVRAM_CAMERA_ISP_PARAM_STRUCT>*const pBufIf_ISP, 
    NSNvram::BufIF<NVRAM_CAMERA_SHADING_STRUCT>*const   pBufIf_SD, 
    NSNvram::BufIF<NVRAM_CAMERA_DEFECT_STRUCT>*const    pBufIf_DF, 
    NVRAM_CAMERA_ISP_PARAM_STRUCT*const pBuf_ISP, 
    NVRAM_CAMERA_SHADING_STRUCT*const   pBuf_SD, 
    NVRAM_CAMERA_DEFECT_STRUCT*const    pBuf_DF
)
    : m_eSensorEnum(eSensorEnum)
    , m_u4SensorID(u4SensorID)
    //
    , m_pNvramDrv(pNvramDrv)
    , m_pSensorDrv(pSensorDrv)
    , m_pIspDrv(pIspDrv)
    //
    , m_pIspHal(pIspHal)
    //
    //
    , m_rBufIf_ISP(*pBufIf_ISP)
    , m_rBufIf_SD (*pBufIf_SD)
    , m_rBufIf_DF (*pBufIf_DF)
    //
    //
    , m_rBuf_ISP(*pBuf_ISP)
    ////
    , m_rISPComm(m_rBuf_ISP.ISPComm)
    , m_rISPRegs(m_rBuf_ISP.ISPRegs)
    , m_rISPRegsIdx(m_rBuf_ISP.ISPRegs.Idx)
    , m_rISPPca (m_rBuf_ISP.ISPPca)
    //
    , m_fgEnabled_OB(MTRUE)
    , m_u4Backup_OB(0)
    //
    ////
    , m_rBuf_SD (*pBuf_SD)
    //
    ////
    , m_rBuf_DF (*pBuf_DF)
{
}

CctCtrl::
~CctCtrl()
{
    if (m_pNvramDrv) {
        m_pNvramDrv->destroyInstance();
        m_pNvramDrv = NULL;
    }

    if (m_pSensorDrv) {
        m_pSensorDrv->destroyInstance();
        m_pSensorDrv = NULL;
    }

    if (m_pIspDrv) {
        m_pIspDrv->destroyInstance();
        m_pIspDrv = NULL;
    }
    
    if (m_pIspHal) {
//        m_pIspHal->destroyInstance();
        m_pIspHal = NULL;
    }
}

