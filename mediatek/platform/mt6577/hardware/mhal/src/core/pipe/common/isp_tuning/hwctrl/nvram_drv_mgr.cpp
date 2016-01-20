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
#define LOG_TAG "NSIspTuning::NvramDrvMgr"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/xlog.h>
//
#include "debug.h"
//
#include "nvram_drv.h"
#include "sensor_drv.h"
#include "isp_tuning.h"
//
#include "nvram_drv_mgr.h"
//
using namespace NSIspTuning;


/*******************************************************************************
* NVRAM Driver Manager Context
*******************************************************************************/
class NvramDrvMgrCtx : public NvramDrvMgr
{
    friend class NvramDrvMgr;
protected:
    NvramDrvMgrCtx();
    ~NvramDrvMgrCtx();

protected:  ////    Data Members.
    NvramDrvBase*                   m_pNvramDrv;

    MUINT32                         m_u4SensorID;
    CAMERA_DUAL_CAMERA_SENSOR_ENUM  m_eSensorEnum;

public:     ////    Interfaces.

    virtual MERROR_ENUM     init(
        ESensorType_T const eSensorType, 
        ESensorRole_T const eSensorRole
    );

    virtual MERROR_ENUM     uninit();

private:    ////
    template <class Buf_T>
    Buf_T*  getRefBuf() const
    {
        NSNvram::BufIF<Buf_T>*const pBufIF = m_pNvramDrv->getBufIF<Buf_T>();
        if  ( ! pBufIF )
        {
            return  NULL;
        }
        return  pBufIF->getRefBuf(m_eSensorEnum, m_u4SensorID);
    }

public:     ////    Interfaces.

    virtual MVOID   getRefBuf(NVRAM_CAMERA_ISP_PARAM_STRUCT*& rpBuf) const;
    virtual MVOID   getRefBuf(NVRAM_CAMERA_SHADING_STRUCT*& rpBuf) const;
    virtual MVOID   getRefBuf(NVRAM_CAMERA_DEFECT_STRUCT*& rpBuf) const;

};


NvramDrvMgr&
NvramDrvMgr::
getInstance()
{
    static NvramDrvMgrCtx singleton;
    return singleton;
}


NvramDrvMgrCtx::
NvramDrvMgrCtx()
    : NvramDrvMgr()
    , m_pNvramDrv(NULL)
    , m_u4SensorID(0)
    , m_eSensorEnum(DUAL_CAMERA_NONE_SENSOR)
{
}


NvramDrvMgrCtx::
~NvramDrvMgrCtx()
{
    uninit();
}


MERROR_ENUM
NvramDrvMgrCtx::
init(ESensorType_T const eSensorType, ESensorRole_T const eSensorRole)
{
    MERROR_ENUM err = MERR_UNKNOWN;

    //  Sensor driver.
    SensorDrv*const pSensorDrv = SensorDrv::createInstance(SENSOR_MAIN|SENSOR_SUB);
    if  ( ! pSensorDrv )
    {
        MY_ERR("[init] Cannnot create Sensor driver");
        err = MERR_BAD_SENSOR_DRV;
        goto lbExit;
    }

    //  Query sensor ID & sensor enum.
    switch  ( eSensorRole )
    {
    case ESensorRole_Main:
        m_eSensorEnum = DUAL_CAMERA_MAIN_SENSOR;
        m_u4SensorID = pSensorDrv->getMainSensorID();
        break;
    case ESensorRole_Sub:
        m_eSensorEnum = DUAL_CAMERA_SUB_SENSOR;
        m_u4SensorID = pSensorDrv->getSubSensorID();
        break;
    default:    //  Shouldn't happen.
        MY_ERR("[init] Invalid sensor role (%d)!", eSensorRole);
        err = MERR_BAD_PARAM;
        goto lbExit;
    }

    //  Nvram driver.
    if  ( ! m_pNvramDrv )
        m_pNvramDrv = NvramDrvBase::createInstance();
    if  ( ! m_pNvramDrv )
    {
        MY_ERR("[init] Cannnot create NVRAM driver");
        err = MERR_BAD_NVRAM_DRV;
        goto lbExit;
    }

    err = MERR_OK;

lbExit:
    if  ( pSensorDrv )
        pSensorDrv->destroyInstance();

    return  err;
}


MERROR_ENUM
NvramDrvMgrCtx::
uninit()
{
    if  ( m_pNvramDrv )
    {
        m_pNvramDrv->destroyInstance();
        m_pNvramDrv = NULL;
    }

    m_u4SensorID = 0;
    m_eSensorEnum = DUAL_CAMERA_NONE_SENSOR;

    return  MERR_OK;    
}


MVOID
NvramDrvMgrCtx::
getRefBuf(NVRAM_CAMERA_ISP_PARAM_STRUCT*& rpBuf) const
{
    rpBuf = getRefBuf<NVRAM_CAMERA_ISP_PARAM_STRUCT>();
}


MVOID
NvramDrvMgrCtx::
getRefBuf(NVRAM_CAMERA_SHADING_STRUCT*& rpBuf) const
{
    rpBuf = getRefBuf<NVRAM_CAMERA_SHADING_STRUCT>();
}


MVOID
NvramDrvMgrCtx::
getRefBuf(NVRAM_CAMERA_DEFECT_STRUCT*& rpBuf) const
{
    rpBuf = getRefBuf<NVRAM_CAMERA_DEFECT_STRUCT>();
}

