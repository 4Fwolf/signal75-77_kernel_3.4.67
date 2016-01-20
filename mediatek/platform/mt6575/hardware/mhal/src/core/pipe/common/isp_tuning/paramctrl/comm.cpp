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
#define LOG_TAG "NSIspTuning::ParamctrlComm"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
#include "paramctrl_comm.h"
//
#include "ispdrvmgr.h"
//
using namespace android;
using namespace NSIspTuning;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MERROR_ENUM
ParamctrlComm::
validate(MBOOL const fgForce)
{
    MERROR_ENUM err = MERR_UNKNOWN;
    MBOOL const fgRet = ( MERR_OK == (err = validateFrameless()) )
                    &&  ( MERR_OK == (err = validatePerFrame(fgForce)) )
                        ;
    return  err;
}


MERROR_ENUM
ParamctrlComm::
validateFrameless()
{
    MERROR_ENUM err = MERR_UNKNOWN;

    MY_LOG("[+validateFrameless]");

    Mutex::Autolock lock(m_Lock);

    //  (1) reinit isp driver manager
    if  ( IspDrvMgr::MERR_OK != IspDrvMgr::getInstance().reinit() )
    {
        err = MERR_BAD_ISP_DRV;
        goto lbExit;
    }

    //  (2)
    if  ( ! prepareHw_Frameless_All() )
    {
        err = MERR_PREPARE_HW;
        goto lbExit;
    }

    //  (3)
    if  ( ! applyToHw_Frameless_All() )
    {
        err = MERR_APPLY_TO_HW;
        goto lbExit;
    }

    //  (4) Force validatePerFrame() to run.
    m_u4ParamChangeCount++;

    err = MERR_OK;

lbExit:
#if ENABLE_MY_ERR
    if  ( MERR_OK != err )
    {
        MY_ERR("[-validateFrameless]err(%X)", err);
    }
#endif
    MY_LOG("[-validateFrameless]err(%X)", err);
    return  err;
}


MERROR_ENUM
ParamctrlComm::
validatePerFrame(MBOOL const fgForce)
{
    MERROR_ENUM err = MERR_UNKNOWN;
#if 0
    MY_LOG("[+validatePerFrame]");
#endif
    Mutex::Autolock lock(m_Lock);

    //  (0) Make sure it's really needed to apply.
    if  (
            0 == getParamChangeCount()  //  no params change
        && ! fgForce                    //  not force to apply
        )
    {
        err = MERR_OK;
        goto lbExit;
    }

#if 0
    MY_LOG  (
        "[validatePerFrame](ParamChangeCount, fgForce)=(%d, %d)"
        , getParamChangeCount(), fgForce
    );
#endif

    //  (1) Do something.
    err = do_validatePerFrame();
    if  ( MERR_OK != err )
    {
        MY_LOG("[validatePerFrame]do_validatePerFrame returns err(%d)", err);
        goto lbExit;
    }

    //  (2) reset to 0 since all params have been applied.
    resetParamChangeCount();

    err = MERR_OK;

lbExit:
    return  err;
}


MERROR_ENUM
ParamctrlComm::
setEnable_Meta_Gamma(MBOOL const fgForceEnable)
{
    MY_LOG(
        "[+setEnable_Meta_Gamma] (fgForceEnable, m_fgForceEnable_Meta_Gamma)=(%d, %d)"
        , fgForceEnable, m_fgForceEnable_Meta_Gamma
    );

    Mutex::Autolock lock(m_Lock);

    checkParamChange(m_fgForceEnable_Meta_Gamma, fgForceEnable);

    m_fgForceEnable_Meta_Gamma = fgForceEnable;

    return  MERR_OK;
}


MERROR_ENUM
ParamctrlComm::
queryExifDebugInfo(NSIspExifDebug::IspExifDebugInfo_T& rExifDebugInfo) const
{
    return  MERR_UNSUPPORT;
}

