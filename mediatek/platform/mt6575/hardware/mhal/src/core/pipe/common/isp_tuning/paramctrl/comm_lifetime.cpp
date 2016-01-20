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
#define LOG_TAG "NSIspTuning"
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
using namespace NSIspTuning;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ParamctrlComm::
ParamctrlComm(ESensorRole_T const eSensorRole, IspCamInfo*const pIspCamInfo)
    : ParamctrlIF()
    , m_Lock()
    , m_eSensorRole(eSensorRole)
    , m_eSensorMode(ESensorMode_Preview)
    , m_eOperMode(EOperMode_Normal)
    , m_rIspCamInfo(*pIspCamInfo)
    , m_u4ParamChangeCount(0)
    , m_fgDynamicTuning(MTRUE)
    , m_fgDynamicCCM(MTRUE)
    , m_fgShadingNVRAMdataChange(MTRUE)
    //
    , m_eIdx_Effect(MEFFECT_OFF)
    //
    //ISP End-User-Define Tuning Index.
    //
    , m_IspUsrSelectLevel()
    //
    //ISP Force Enable/Disable
    , m_fgForceEnable_Meta_Gamma(MTRUE)
    //
    //
{
}


MERROR_ENUM
ParamctrlComm::
construct()
{
    return  MERR_OK;
}


MERROR_ENUM
ParamctrlComm::
init()
{
    MERROR_ENUM err = MERR_UNKNOWN;

    //  (1) Force to assume all params have chagned and different.
    m_u4ParamChangeCount = 1;

    //  (2) Init ISP driver manager.
    if  ( IspDrvMgr::MERR_OK != IspDrvMgr::getInstance().init() )
    {
        err = MERR_BAD_ISP_DRV;
        goto lbExit;
    }

    //  (3) Do something. (Template Method Design Pattern)
    err = do_init();
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }
    
    //  (4) validateFrameless() is invoked 
    //  when init() or status change, like Camera Mode.
    err = validateFrameless();
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

    //  (5) however, is it needed to invoke validatePerFrame() in init()?
    //  or just invoke it only when a frame comes.
    err = validatePerFrame(MTRUE);
    if  ( MERR_OK != err )
    {
        goto lbExit;
    }

    err = MERR_OK;

lbExit:
    if  ( MERR_OK != err )
    {
        uninit();
    }
    MY_LOG("[-ParamctrlComm::init]err(%X)", err);
    return  err;
}


MERROR_ENUM
ParamctrlComm::
uninit()
{
    //  Uninit ISP driver manager.
    IspDrvMgr::getInstance().uninit();

    return  MERR_OK;
}

