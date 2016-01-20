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
#define LOG_TAG "NSIspTuning::ParamctrlComm_user"
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
using namespace android;
using namespace NSIspTuning;


MERROR_ENUM
ParamctrlComm::
setIspUserIdx_Edge(EIndex_Isp_Edge_T const eIndex)
{
    Mutex::Autolock lock(m_Lock);

#if ENABLE_MY_LOG
    MY_LOG(
        "[+setIspUserIdx_Edge] (old, new)=(%d, %d)"
        , m_IspUsrSelectLevel.eIdx_Edge, eIndex
    );
#endif

    if  ( checkParamChange(m_IspUsrSelectLevel.eIdx_Edge, eIndex) )
        m_IspUsrSelectLevel.eIdx_Edge = eIndex;

    return  MERR_OK;
}


MERROR_ENUM
ParamctrlComm::
setIspUserIdx_Hue(EIndex_Isp_Hue_T const eIndex)
{
    Mutex::Autolock lock(m_Lock);

#if ENABLE_MY_LOG
    MY_LOG(
        "[+setIspUserIdx_Hue] (old, new)=(%d, %d)"
        , m_IspUsrSelectLevel.eIdx_Hue, eIndex
    );
#endif

    if  ( checkParamChange(m_IspUsrSelectLevel.eIdx_Hue, eIndex) )
        m_IspUsrSelectLevel.eIdx_Hue = eIndex;

    return  MERR_OK;
}


MERROR_ENUM
ParamctrlComm::
setIspUserIdx_Sat(EIndex_Isp_Saturation_T const eIndex)
{
    Mutex::Autolock lock(m_Lock);

#if ENABLE_MY_LOG
    MY_LOG(
        "[+setIspUserIdx_Sat] (old, new)=(%d, %d)", 
        m_IspUsrSelectLevel.eIdx_Sat, eIndex
    );
#endif

    if  ( checkParamChange(m_IspUsrSelectLevel.eIdx_Sat, eIndex) )
        m_IspUsrSelectLevel.eIdx_Sat = eIndex;

    return  MERR_OK;
}


MERROR_ENUM
ParamctrlComm::
setIspUserIdx_Bright(EIndex_Isp_Brightness_T const eIndex)
{
    Mutex::Autolock lock(m_Lock);

#if ENABLE_MY_LOG
    MY_LOG(
        "[+setIspUserIdx_Bright] (old, new)=(%d, %d)"
        , m_IspUsrSelectLevel.eIdx_Bright, eIndex
    );
#endif

    if  ( checkParamChange(m_IspUsrSelectLevel.eIdx_Bright, eIndex) )
        m_IspUsrSelectLevel.eIdx_Bright = eIndex;

    return  MERR_OK;
}


MERROR_ENUM
ParamctrlComm::
setIspUserIdx_Contrast(EIndex_Isp_Contrast_T const eIndex)
{
    Mutex::Autolock lock(m_Lock);

#if ENABLE_MY_LOG
    MY_LOG(
        "[+setIspUserIdx_Contrast] (old, new)=(%d, %d)"
        , m_IspUsrSelectLevel.eIdx_Contrast, eIndex
    );
#endif

    if  ( checkParamChange(m_IspUsrSelectLevel.eIdx_Contrast, eIndex) )
        m_IspUsrSelectLevel.eIdx_Contrast = eIndex;

    return  MERR_OK;
}

