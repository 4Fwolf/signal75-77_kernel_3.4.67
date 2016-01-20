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
#define LOG_TAG "NSIspTuning::Instance"
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
//
#include "paramctrl_raw.h"
#include "paramctrl_yuv.h"
//
#include "isp_tuning_hal.h"
#include "isp_tuning_bridge.h"
//
using namespace NSIspTuning;


IspTuningHal*
IspTuningHal::
createInstance(ESensorType_T const eSensorType, ESensorRole_T const eSensorRole)
{
    IspTuningHal*pIspTuningHal = NULL;

    ParamctrlIF* pParamctrl = NULL;
    switch  (eSensorType)
    {
    case ESensorType_RAW:
        if  ( ESensorRole_ATV == eSensorRole )
        {
            MY_LOG("[createInstance] RAW sensor does not support ATV");
            break;
        }
        pParamctrl = ParamctrlRAW::createInstance(eSensorRole);
        break;
    case ESensorType_YUV:
        pParamctrl = ParamctrlYUV::createInstance(eSensorRole);
        break;
    }
    if  ( ! pParamctrl )
    {
        MY_LOG(
            "[createInstance] Cannot new ParamctrlIF:"
            "(type, role)=(%d, %d)", eSensorType, eSensorRole
        );
        return  NULL;
    }

    pIspTuningHal = new IspTuningBridge(pParamctrl);
    if  ( ! pIspTuningHal || MERR_OK != pIspTuningHal->construct() )
    {   //  Fail.
        pParamctrl->destroyInstance();
        pParamctrl = NULL;
        delete  pIspTuningHal;
        pIspTuningHal = NULL;
        MY_LOG  (
            "[createInstance][type:%d]"
            "Cannot new or construct IspTuningHal", eSensorType
        );
    }
    return  pIspTuningHal;
}


void
IspTuningBridge::
destroyInstance()
{
    delete  this;
}

