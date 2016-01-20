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

#ifndef _EIS_HAL_H_
#define _EIS_HAL_H_

#include "eis_hal_base.h"

using namespace android;

/*******************************************************************************
*
********************************************************************************/

class EisHal : public EisHalBase
{
public:
    static EisHalBase* getInstance();
    virtual MVOID destroyInstance();

private:
    EisHal();
    virtual ~EisHal();
    virtual MINT32 init();
    virtual MINT32 uninit();  
    virtual MINT32 sysram_alloc(NSIspSysram::EUser_T const eUsr, MUINT32 const u4BytesToAlloc, MVOID*& rPA, MVOID*& rVA);    
    virtual MINT32 sysram_free(NSIspSysram::EUser_T const eUsr);

public:
    virtual MVOID  enableEIS(eis_config_t a_sEisConfig, eis_factor_t &a_sEisFactor);    
    virtual MVOID  disableEIS();    
    virtual MINT32 doEIS(MINT32 &a_i4CMV_X, MINT32 &a_i4CMV_Y);
    
    virtual MVOID getHWGMV(MINT32 &a_i4GMV_X, MINT32 &a_i4GMV_Y);
    virtual MVOID getSWGMV(MINT32 &a_i4GMV_X, MINT32 &a_i4GMV_Y);
    virtual MVOID getEISStat(eis_stat_t &a_sEIS_Stat);
    

private:

    volatile MINT32 mUsers;
    mutable Mutex mLock;

    NSIspSysram::IspSysramDrv* m_pEisSysram;    
    EisDrvBase* m_pEisDrv;
    MTKEIS* m_pEisApp;

    MINT32 m_i4Path;


};

#endif

