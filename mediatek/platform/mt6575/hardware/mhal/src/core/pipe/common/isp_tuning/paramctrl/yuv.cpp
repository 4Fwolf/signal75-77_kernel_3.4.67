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
#define LOG_TAG "NSIspTuning::ParamctrlYUV"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
#include "paramctrl_yuv.h"
//
#include "sensor_drv.h"
#include "kd_camera_feature.h"
//
using namespace android;
using namespace NSIspTuning;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ParamctrlYUV*
ParamctrlYUV::
createInstance(ESensorRole_T const eSensorRole)
{
    return new ParamctrlYUV(eSensorRole);
}


void
ParamctrlYUV::
destroyInstance()
{
    delete this;
}


ParamctrlYUV::
ParamctrlYUV(ESensorRole_T const eSensorRole)
    : Parent_t(eSensorRole, &m_IspCamInfo)
    , m_IspCamInfo()
{
}


ParamctrlYUV::
~ParamctrlYUV()
{
}


MERROR_ENUM
ParamctrlYUV::
do_validatePerFrame()
{
    return  MERR_OK;
}


MERROR_ENUM
ParamctrlYUV::
setSceneMode(EIndex_Scene_T const eScene)
{
    MERROR_ENUM err = MERR_UNKNOWN;

    err = Parent_t::setSceneMode(eScene);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

    err = apply(FID_SCENE_MODE, m_IspCamInfo.eIdx_Scene);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

lbExit:
    return  err;
}


MERROR_ENUM
ParamctrlYUV::
setEffect(EIndex_Effect_T const eEffect)
{
    MERROR_ENUM err = MERR_UNKNOWN;

    err = Parent_t::setEffect(eEffect);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

    err = apply(FID_COLOR_EFFECT, getEffect());
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

lbExit:
    return  err;
}

MERROR_ENUM
ParamctrlYUV::
setIspUserIdx_Edge(EIndex_Isp_Edge_T const eIndex)
{
    MERROR_ENUM err = MERR_UNKNOWN;
    
    err = Parent_t::setIspUserIdx_Edge(eIndex);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }
    err = apply(FID_ISP_EDGE, getIspUsrSelectLevel().eIdx_Edge);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

lbExit:
    return  err;
}

MERROR_ENUM
ParamctrlYUV::
setIspUserIdx_Hue(EIndex_Isp_Hue_T const eIndex)
{
    MERROR_ENUM err = MERR_UNKNOWN;
    
    err = Parent_t::setIspUserIdx_Hue(eIndex);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }
    err = apply(FID_ISP_HUE, getIspUsrSelectLevel().eIdx_Hue);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

lbExit:
    return  err;
}

MERROR_ENUM
ParamctrlYUV::
setIspUserIdx_Sat(EIndex_Isp_Saturation_T const eIndex)
{
    MERROR_ENUM err = MERR_UNKNOWN;
    
    err = Parent_t::setIspUserIdx_Sat(eIndex);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }
    err = apply(FID_ISP_SAT, getIspUsrSelectLevel().eIdx_Sat);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

lbExit:
    return  err;
}

MERROR_ENUM
ParamctrlYUV::
setIspUserIdx_Bright(EIndex_Isp_Brightness_T const eIndex)
{
    MERROR_ENUM err = MERR_UNKNOWN;
    
    err = Parent_t::setIspUserIdx_Bright(eIndex);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }
    err = apply(FID_ISP_BRIGHT, getIspUsrSelectLevel().eIdx_Bright);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

lbExit:
    return  err;
}

MERROR_ENUM
ParamctrlYUV::
setIspUserIdx_Contrast(EIndex_Isp_Contrast_T const eIndex)
{
    MERROR_ENUM err = MERR_UNKNOWN;
    
    err = Parent_t::setIspUserIdx_Contrast(eIndex);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }
    err = apply(FID_ISP_CONTRAST, getIspUsrSelectLevel().eIdx_Contrast);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

lbExit:
    return  err;
}


MERROR_ENUM
ParamctrlYUV::
apply(MUINT32 u4Fid, MUINT32 u4Val)
{
    MERROR_ENUM err = MERR_UNKNOWN;

    MINT32  i4Err = 0;

    //  (1) Open the sensor driver.
    SensorDrv*const pSensorDrv = SensorDrv::createInstance(
        ESensorRole_ATV == getSensorRole()
            ?   SENSOR_ATV
            :   SENSOR_MAIN|SENSOR_SUB
    );
    if  ( ! pSensorDrv )
    {
        MY_ERR("[apply] pSensorDrv==NULL");
        err = MERR_BAD_SENSOR_DRV;
        goto lbExit;
    }

    //  (2) Send command to the sensor driver.
    i4Err = pSensorDrv->sendCommand(CMD_SENSOR_SET_YUV_CMD, &u4Fid, &u4Val);
    if  ( 0 > i4Err )
    {
        MY_ERR("[apply][CMD_SENSOR_SET_YUV_CMD] i4Err(%d)", i4Err);
        err = MERR_BAD_SENSOR_DRV;
        goto lbExit;
    }

    err = MERR_OK;

lbExit:
    if  ( pSensorDrv )
    {
        pSensorDrv->destroyInstance();
    }

    return  err;    
}

