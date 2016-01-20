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
#define LOG_TAG "isp_cctop"
//
#include <utils/Errors.h>
#include <cutils/xlog.h>
//
#include "pipe_types.h"
//
#include "cct_feature.h"
//
#include "isp_drv.h"
#include "isp_hal.h"
#include "isp_tuning_hal.h"
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
enum ISP_REG_CATEGORY
{
    EIspReg_Shading,
    EIspReg_OB,
    EIspReg_DM,
    EIspReg_DP,
    EIspReg_NR1,
    EIspReg_NR2,
    EIspReg_EE,
    EIspReg_Saturation,
    EIspReg_Contrast,
    EIspReg_Hue,
    EIspReg_CCM,
    EIspReg_Gamma
};
MBOOL  CctCtrl::updateIspRegs(MUINT32 const u4Category/*= 0xFFFFFFFF*/, MUINT32 const u4Index/*= 0xFFFFFFFF*/)
{
    MBOOL fgRet = MFALSE;

    MBOOL fgIsDynamicISPEnabled = false;
    MBOOL fgIsDynamicCCMEnabled = false;
    MBOOL fgDisableDynamic = false; //  Disable Dynamic Data.

    //  (1) Save all index.
    ISP_NVRAM_REG_INDEX_T BackupIdx = m_rISPRegsIdx;

    //  (2) Modify a specific index.
#define MY_SET_ISP_REG_INDEX(_category)\
    case EIspReg_##_category:\
        if  ( IspNvramRegMgr::NUM_##_category <= u4Index )\
            return  MFALSE;\
        m_rISPRegsIdx._category = static_cast<MUINT8>(u4Index);\
        break

    switch (u4Category)
    {
    MY_SET_ISP_REG_INDEX(Shading);
    MY_SET_ISP_REG_INDEX(OB);
    MY_SET_ISP_REG_INDEX(DM);
    MY_SET_ISP_REG_INDEX(DP);
    MY_SET_ISP_REG_INDEX(NR1);
    MY_SET_ISP_REG_INDEX(NR2);
    MY_SET_ISP_REG_INDEX(EE);
    MY_SET_ISP_REG_INDEX(Saturation);
    MY_SET_ISP_REG_INDEX(Contrast);
    MY_SET_ISP_REG_INDEX(Hue);
    MY_SET_ISP_REG_INDEX(CCM);
    MY_SET_ISP_REG_INDEX(Gamma);
    default:
        break;
    }

    //  (3) Save the current dynamic ISP flag.
    NSIspTuning::CmdArg_T cmd_GetDynamicISP;
    cmd_GetDynamicISP.eCmd        = NSIspTuning::ECmd_GetDynamicTuning;
    cmd_GetDynamicISP.pOutBuf     = &fgIsDynamicISPEnabled;
    cmd_GetDynamicISP.u4OutBufSize= sizeof(MBOOL);
    //  (4) Save the current dynamic CCM flag.
    NSIspTuning::CmdArg_T cmd_GetDynamicCCM;
    cmd_GetDynamicCCM.eCmd        = NSIspTuning::ECmd_GetDynamicCCM;
    cmd_GetDynamicCCM.pOutBuf     = &fgIsDynamicCCMEnabled;
    cmd_GetDynamicCCM.u4OutBufSize= sizeof(MBOOL);
    //  (5) Disable the dynamic ISP.
    NSIspTuning::CmdArg_T cmd_DisableDynamicISP;
    cmd_DisableDynamicISP.eCmd       = NSIspTuning::ECmd_SetDynamicTuning;
    cmd_DisableDynamicISP.pInBuf     = &fgDisableDynamic;
    cmd_DisableDynamicISP.u4InBufSize= sizeof(MBOOL);
    //  (6) Disable the dynamic CCM.
    NSIspTuning::CmdArg_T cmd_DisableDynamicCCM;
    cmd_DisableDynamicCCM.eCmd       = NSIspTuning::ECmd_SetDynamicCCM;
    cmd_DisableDynamicCCM.pInBuf     = &fgDisableDynamic;
    cmd_DisableDynamicCCM.u4InBufSize= sizeof(MBOOL);
    //  (8) Restore the dynamic ISP flag.
    NSIspTuning::CmdArg_T cmd_RestoreDynamicISP;
    cmd_RestoreDynamicISP.eCmd       = NSIspTuning::ECmd_SetDynamicTuning;
    cmd_RestoreDynamicISP.pInBuf     = &fgIsDynamicISPEnabled;
    cmd_RestoreDynamicISP.u4InBufSize= sizeof(MBOOL);
    //  (9) Restore the dynamic CCM flag.
    NSIspTuning::CmdArg_T cmd_RestoreDynamicCCM;
    cmd_RestoreDynamicCCM.eCmd       = NSIspTuning::ECmd_SetDynamicCCM;
    cmd_RestoreDynamicCCM.pInBuf     = &fgIsDynamicCCMEnabled;
    cmd_RestoreDynamicCCM.u4InBufSize= sizeof(MBOOL);

    if  (
            0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_GetDynamicISP))     //(3)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_GetDynamicCCM))     //(4)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_DisableDynamicISP)) //(5)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_DisableDynamicCCM)) //(6)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_VALIDATE_FRAME, true)                                  //(7) Validate
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_RestoreDynamicISP)) //(8)
        ||  0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd_RestoreDynamicCCM)) //(9)
        )
    {
        goto lbExit;
    }

    MY_LOG("dynamic flags:(isp, ccm)=(%d, %d)", fgIsDynamicISPEnabled, fgIsDynamicCCMEnabled);

    fgRet = MTRUE;

lbExit:
    //  (10) Restore all index.
    m_rISPRegsIdx = BackupIdx;

    return  fgRet;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 CctCtrl::CCTOPReadIspReg(MVOID *puParaIn, MVOID *puParaOut, MUINT32 *pu4RealParaOutLen)
{
    MINT32 err = CCTIF_NO_ERROR;
    PACDK_CCT_REG_RW_STRUCT pIspRegInfoIn = (PACDK_CCT_REG_RW_STRUCT)puParaIn;
    PACDK_CCT_REG_RW_STRUCT pIspRegInfoOut = (PACDK_CCT_REG_RW_STRUCT)puParaOut;
    IspDrv::reg_t rIspReg;

    MY_LOG("ACDK_CCT_OP_ISP_READ_REG\n");

    rIspReg.addr = pIspRegInfoIn->RegAddr;

    err = m_pIspDrv->readRegs(&rIspReg, 1);
    if (err < 0) {
        MY_ERR("[CCTOPReadIspReg] readIspRegs() error");
        return err;
    }

    pIspRegInfoOut->RegData = rIspReg.val;

    *pu4RealParaOutLen = sizeof(ACDK_CCT_REG_RW_STRUCT);

    MY_LOG("[CCTOPReadIspReg] regAddr = %x, regData = %x\n", (MUINT32)rIspReg.addr, (MUINT32)rIspReg.val);
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctCtrl::CCTOPWriteIspReg(MVOID *puParaIn)
{
    MINT32 err = CCTIF_NO_ERROR;
    PACDK_CCT_REG_RW_STRUCT pIspRegInfoIn = (PACDK_CCT_REG_RW_STRUCT)puParaIn;
    IspDrv::reg_t rIspReg;

    MY_LOG("ACDK_CCT_OP_ISP_WRITE_REG\n");

    rIspReg.addr = pIspRegInfoIn->RegAddr;
    rIspReg.val = pIspRegInfoIn->RegData;

    err = m_pIspDrv->writeRegs(&rIspReg, 1);
    if (err < 0) {
        MY_ERR("[CCTOPWriteIspReg]writeRegs() error");
        return err;
    }

    MY_LOG("[CCTOPWriteIspReg] regAddr = %x, regData = %x\n", (MUINT32)rIspReg.addr, (MUINT32)rIspReg.val);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
IMP_CCT_CTRL( ACDK_CCT_OP_QUERY_ISP_ID )
{
    if  ( sizeof(MUINT32) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    *reinterpret_cast<MUINT32 *>(puParaOut) = 0x65758A00;;
    *pu4RealParaOutLen = sizeof(MUINT32);

    return CCTIF_NO_ERROR;
}


IMP_CCT_CTRL( ACDK_CCT_OP_ISP_READ_REG )
{
    return CCTOPReadIspReg((MVOID *)puParaIn, (MVOID *)puParaOut, pu4RealParaOutLen);
}


IMP_CCT_CTRL( ACDK_CCT_OP_ISP_WRITE_REG )
{
    return CCTOPWriteIspReg((MVOID *)puParaIn);
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX
u4ParaInLen
    sizeof(ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX )
{
    typedef ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const                   u4Index     = reinterpret_cast<type const*>(puParaIn)->u4Index;
    ACDK_CCT_ISP_REG_CATEGORY const eCategory   = reinterpret_cast<type const*>(puParaIn)->eCategory;

#define MY_SET_TUNING_INDEX(_category)\
    case EIsp_Category_##_category:\
        if  ( IspNvramRegMgr::NUM_##_category <= u4Index )\
            return  CCTIF_BAD_PARAM;\
        m_rISPRegsIdx._category = static_cast<MUINT8>(u4Index);\
        break

    switch  (eCategory)
    {
    MY_SET_TUNING_INDEX(OB);
    MY_SET_TUNING_INDEX(DM);
    MY_SET_TUNING_INDEX(DP);
    MY_SET_TUNING_INDEX(NR1);
    MY_SET_TUNING_INDEX(NR2);
    MY_SET_TUNING_INDEX(EE);
    MY_SET_TUNING_INDEX(Saturation);
    MY_SET_TUNING_INDEX(Contrast);
    MY_SET_TUNING_INDEX(Hue);
    default:
        return  CCTIF_BAD_PARAM;
    }

    return  CCTIF_NO_ERROR;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
    MUINT32
u4ParaOutLen
    sizeof(MUINT32)
pu4RealParaOutLen
    sizeof(MUINT32)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY   i_type;
    typedef MUINT32                     o_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type*>(puParaIn);
    o_type&      rIndex    = *reinterpret_cast<o_type*>(puParaOut);


#define MY_GET_TUNING_INDEX(_category)\
    case EIsp_Category_##_category:\
        rIndex = m_rISPRegsIdx._category;\
        break

    switch  (eCategory)
    {
    MY_GET_TUNING_INDEX(OB);
    MY_GET_TUNING_INDEX(DM);
    MY_GET_TUNING_INDEX(DP);
    MY_GET_TUNING_INDEX(NR1);
    MY_GET_TUNING_INDEX(NR2);
    MY_GET_TUNING_INDEX(EE);
    MY_GET_TUNING_INDEX(Saturation);
    MY_GET_TUNING_INDEX(Contrast);
    MY_GET_TUNING_INDEX(Hue);
    default:
        return  CCTIF_BAD_PARAM;
    }

    *pu4RealParaOutLen = sizeof(o_type);

    return  CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen

puParaOut
    ACDK_CCT_ISP_GET_TUNING_PARAS
u4ParaOutLen
    sizeof(ACDK_CCT_ISP_GET_TUNING_PARAS)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ISP_GET_TUNING_PARAS)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS )
{
    typedef ACDK_CCT_ISP_GET_TUNING_PARAS o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    ACDK_CCT_ISP_NVRAM_REG& rRegs = reinterpret_cast<o_type*>(puParaOut)->stIspNvramRegs;

#define MY_GET_TUNING_PARAS(_category)\
    ::memcpy(rRegs._category, m_rISPRegs._category, sizeof(rRegs._category))

    MY_GET_TUNING_PARAS(OB);
    MY_GET_TUNING_PARAS(DM);
    MY_GET_TUNING_PARAS(DP);
    MY_GET_TUNING_PARAS(NR1);
    MY_GET_TUNING_PARAS(NR2);
    MY_GET_TUNING_PARAS(EE);
    MY_GET_TUNING_PARAS(Saturation);
    MY_GET_TUNING_PARAS(Contrast);
    MY_GET_TUNING_PARAS(Hue);

    *pu4RealParaOutLen = sizeof(o_type);

    return  CCTIF_NO_ERROR;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ACDK_CCT_ISP_SET_TUNING_PARAS
u4ParaInLen
    sizeof(ACDK_CCT_ISP_SET_TUNING_PARAS);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS )
{
    typedef ACDK_CCT_ISP_SET_TUNING_PARAS type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const                   u4Index     = reinterpret_cast<type const*>(puParaIn)->u4Index;
    ACDK_CCT_ISP_REG_CATEGORY const eCategory   = reinterpret_cast<type const*>(puParaIn)->eCategory;
    ACDK_CCT_ISP_NVRAM_REG const&   rRegs       = reinterpret_cast<type const*>(puParaIn)->stIspNvramRegs;

    ISP_REG_CATEGORY eIspRegCategory;

#define MY_SET_TUNING_PARAS(_category)\
    case EIsp_Category_##_category:\
        if  ( IspNvramRegMgr::NUM_##_category <= u4Index )\
            return  CCTIF_BAD_PARAM;\
        m_rISPRegs._category[u4Index] = rRegs._category[u4Index];\
        eIspRegCategory = EIspReg_##_category;\
        break

    switch  ( eCategory )
    {
    MY_SET_TUNING_PARAS(OB);
    MY_SET_TUNING_PARAS(DM);
    MY_SET_TUNING_PARAS(DP);
    MY_SET_TUNING_PARAS(NR1);
    MY_SET_TUNING_PARAS(NR2);
    MY_SET_TUNING_PARAS(EE);
    MY_SET_TUNING_PARAS(Saturation);
    MY_SET_TUNING_PARAS(Contrast);
    MY_SET_TUNING_PARAS(Hue);
    default:
        return  CCTIF_BAD_PARAM;
    }

    if  ( ! updateIspRegs(eIspRegCategory, u4Index) )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    return  CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE )
{
    MBOOL fgEnableDynamicTuning = false;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_SetDynamicTuning,
        pInBuf:             &fgEnableDynamicTuning,
        u4InBufSize:        sizeof(MBOOL),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    return  CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE )
{
    MBOOL fgEnableDynamicTuning = true;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_SetDynamicTuning,
        pInBuf:             &fgEnableDynamicTuning,
        u4InBufSize:        sizeof(MBOOL),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    return  CCTIF_NO_ERROR;
}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF )
{
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MBOOL fgIsDynamicTuningEnabled = false;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_GetDynamicTuning,
        pInBuf:             NULL,
        u4InBufSize:        0,
        pOutBuf:            &fgIsDynamicTuningEnabled,
        u4OutBufSize:       sizeof(MBOOL),
        u4ActualOutSize:    0
    };

    if  (
        0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd))
    ||  sizeof(MBOOL) != cmd.u4ActualOutSize
        )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    //  enable/disable       : (1, 0)
    //  enable/disable bypass: (0, 1)
    reinterpret_cast<o_type*>(puParaOut)->Enable
        = fgIsDynamicTuningEnabled ? false : true;

    *pu4RealParaOutLen = sizeof(o_type);

    return  CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_GAMMA_TABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE )
{
    typedef ACDK_CCT_GAMMA_TABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const pGamma = reinterpret_cast<o_type*>(puParaOut);

    ::memcpy(pGamma, m_rISPRegs.Gamma[0].set, sizeof(o_type));

    *pu4RealParaOutLen = sizeof(o_type);

    for (MUINT32 i = 0; i < GAMMA_STEP_NO; i++)
    {
        MY_LOG("gamma: [%d]: 0x%02X\n", i, pGamma->gamma[0][i]);
    }

    return  CCTIF_NO_ERROR;
}

/*
puParaIn
    ACDK_CCT_GAMMA_TABLE_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_GAMMA_TABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE )
{
    typedef ACDK_CCT_GAMMA_TABLE_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pGamma = reinterpret_cast<i_type*>(puParaIn);

    ::memcpy(m_rISPRegs.Gamma[0].set, pGamma, sizeof(i_type));

    for (MUINT32 i = 0; i < GAMMA_STEP_NO; i++)
    {
        MY_LOG("gamma: [%d]: 0x%02X\n", i, pGamma->gamma[0][i]);
    }

    if  ( ! updateIspRegs(EIspReg_Gamma, 0) )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    return  CCTIF_NO_ERROR;
}

/*
puParaIn
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS )
{
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    //  enable/disable       : (1, 0)
    //  enable/disable bypass: (0, 1)
    MBOOL fgEnable = reinterpret_cast<type*>(puParaIn)->Enable ? false : true;

    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_SetForceCtrl_Gamma,
        pInBuf:             &fgEnable,
        u4InBufSize:        sizeof(fgEnable),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    if  ( ! updateIspRegs() )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    return  CCTIF_NO_ERROR;
}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG )
{
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MBOOL fgEnable = MFALSE;

    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_GetForceCtrl_Gamma,
        pInBuf:             NULL,
        u4InBufSize:        0,
        pOutBuf:            &fgEnable,
        u4OutBufSize:       sizeof(fgEnable),
        u4ActualOutSize:    0
    };

    if  (
            0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd))
        ||  sizeof(fgEnable) != cmd.u4ActualOutSize
        )
    {
        MY_ERR("[ERR] (u4OutBufSize, u4ActualOutSize)=(%d, %d)", sizeof(fgEnable), cmd.u4ActualOutSize);
        return  CCTIF_INVALID_DRIVER;
    }

    //  enable/disable       : (1, 0)
    //  enable/disable bypass: (0, 1)
    reinterpret_cast<o_type*>(puParaOut)->Enable
        = fgEnable ? false : true;

    *pu4RealParaOutLen = sizeof(o_type);

    return  CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_CCM_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM )
{
    typedef ACDK_CCT_CCM_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MUINT32 const index = m_rISPRegsIdx.CCM;
    ISP_NVRAM_CCM_T& rSrc = m_rISPRegs.CCM[index];
    o_type*const     pDst = reinterpret_cast<o_type*>(puParaOut);

    pDst->M11 = rSrc.ccm1.bits.M11;
    pDst->M12 = rSrc.ccm1.bits.M12;
    pDst->M13 = rSrc.ccm2.bits.M13;
    pDst->M21 = rSrc.ccm2.bits.M21;
    pDst->M22 = rSrc.ccm3.bits.M22;
    pDst->M23 = rSrc.ccm3.bits.M23;
    pDst->M31 = rSrc.ccm4.bits.M31;
    pDst->M32 = rSrc.ccm4.bits.M32;
    pDst->M33 = rSrc.ccm5.bits.M33;

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("Current CCM Index: %d", m_rISPRegsIdx.CCM);
    MY_LOG("index to get: %d", index);

    MY_LOG("index 0x%03X", index);
    MY_LOG("M11 0x%03X", pDst->M11);
    MY_LOG("M12 0x%03X", pDst->M12);
    MY_LOG("M13 0x%03X", pDst->M13);
    MY_LOG("M21 0x%03X", pDst->M21);
    MY_LOG("M22 0x%03X", pDst->M22);
    MY_LOG("M23 0x%03X", pDst->M23);
    MY_LOG("M31 0x%03X", pDst->M31);
    MY_LOG("M32 0x%03X", pDst->M32);
    MY_LOG("M33 0x%03X", pDst->M33);
    return  CCTIF_NO_ERROR;
}

/*
puParaIn
    ACDK_CCT_CCM_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_CCM_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM )
{
    typedef ACDK_CCT_CCM_STRUCT type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const index = m_rISPRegsIdx.CCM;
    ISP_NVRAM_CCM_T& rDst = m_rISPRegs.CCM[index];
    type*const       pSrc = reinterpret_cast<type*>(puParaIn);

    rDst.ccm1.bits.M11 = pSrc->M11;
    rDst.ccm1.bits.M12 = pSrc->M12;
    rDst.ccm2.bits.M13 = pSrc->M13;
    rDst.ccm2.bits.M21 = pSrc->M21;
    rDst.ccm3.bits.M22 = pSrc->M22;
    rDst.ccm3.bits.M23 = pSrc->M23;
    rDst.ccm4.bits.M31 = pSrc->M31;
    rDst.ccm4.bits.M32 = pSrc->M32;
    rDst.ccm5.bits.M33 = pSrc->M33;

    //  write to register.
    if  ( ! updateIspRegs(EIspReg_CCM, index) )
    {
        return  CCTIF_INVALID_DRIVER;
    }
    return  CCTIF_NO_ERROR;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    MUINT32 u4Index
u4ParaInLen
    sizeof(MUINT32)
puParaOut
    ACDK_CCT_CCM_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_CCM_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM )
{
    typedef ACDK_CCT_CCM_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    typedef MUINT32             i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type const index = *reinterpret_cast<i_type*>(puParaIn);
    if  ( NVRAM_CCM_TBL_NUM <= index )
    {
        MY_ERR("[ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM] out of range: index(%d) >= NVRAM_CCM_TBL_NUM(%d)", index, NVRAM_CCM_TBL_NUM);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_CCM_T& rSrc = m_rISPRegs.CCM[index];
    o_type*const     pDst = reinterpret_cast<o_type*>(puParaOut);

    pDst->M11 = rSrc.ccm1.bits.M11;
    pDst->M12 = rSrc.ccm1.bits.M12;
    pDst->M13 = rSrc.ccm2.bits.M13;
    pDst->M21 = rSrc.ccm2.bits.M21;
    pDst->M22 = rSrc.ccm3.bits.M22;
    pDst->M23 = rSrc.ccm3.bits.M23;
    pDst->M31 = rSrc.ccm4.bits.M31;
    pDst->M32 = rSrc.ccm4.bits.M32;
    pDst->M33 = rSrc.ccm5.bits.M33;

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("Current CCM Index: %d", m_rISPRegsIdx.CCM);
    MY_LOG("index to get: %d", index);

    MY_LOG("M11 0x%03X", pDst->M11);
    MY_LOG("M12 0x%03X", pDst->M12);
    MY_LOG("M13 0x%03X", pDst->M13);
    MY_LOG("M21 0x%03X", pDst->M21);
    MY_LOG("M22 0x%03X", pDst->M22);
    MY_LOG("M23 0x%03X", pDst->M23);
    MY_LOG("M31 0x%03X", pDst->M31);
    MY_LOG("M32 0x%03X", pDst->M32);
    MY_LOG("M33 0x%03X", pDst->M33);
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
    ACDK_CCT_SET_NVRAM_CCM
u4ParaInLen
    sizeof(ACDK_CCT_SET_NVRAM_CCM)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM )
{
    typedef ACDK_CCT_SET_NVRAM_CCM type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    type& rInParam = *reinterpret_cast<type*>(puParaIn);
    MUINT32 const index = rInParam.u4Index;
    if  ( NVRAM_CCM_TBL_NUM <= index )
    {
        MY_ERR("[ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM] out of range: index(%d) >= NVRAM_CCM_TBL_NUM(%d)", index, NVRAM_CCM_TBL_NUM);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_CCM_T& rDst = m_rISPRegs.CCM[index];

    rDst.ccm1.bits.M11 = rInParam.ccm.M11;
    rDst.ccm1.bits.M12 = rInParam.ccm.M12;
    rDst.ccm2.bits.M13 = rInParam.ccm.M13;
    rDst.ccm2.bits.M21 = rInParam.ccm.M21;
    rDst.ccm3.bits.M22 = rInParam.ccm.M22;
    rDst.ccm3.bits.M23 = rInParam.ccm.M23;
    rDst.ccm4.bits.M31 = rInParam.ccm.M31;
    rDst.ccm4.bits.M32 = rInParam.ccm.M32;
    rDst.ccm5.bits.M33 = rInParam.ccm.M33;
/*
    //  For the compatibility to old ways, needn't write to register/nvram.
    //  write to register.
    if  ( ! updateIspRegs(EIspReg_CCM, index) )
    {
        return  CCTIF_INVALID_DRIVER;
    }
*/
    MY_LOG("Current CCM Index: %d", m_rISPRegsIdx.CCM);
    MY_LOG("index to set: %d", index);
    for (MUINT32 i = 0; i < ISP_NVRAM_CCM_T::COUNT; i++)
    {
        MY_LOG("CCM: [%d] 0x%06X", i, rDst.set[i]);
    }
    return  CCTIF_NO_ERROR;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_NVRAM_CCM_PARA
u4ParaOutLen
    sizeof(ACDK_CCT_NVRAM_CCM_PARA)
pu4RealParaOutLen
    sizeof(ACDK_CCT_NVRAM_CCM_PARA)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_PARA )
{
    typedef ACDK_CCT_NVRAM_CCM_PARA o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const     pDst = reinterpret_cast<o_type*>(puParaOut);

    for (MUINT32 i=0; i<NVRAM_CCM_TBL_NUM; i++)
    {
        pDst->ccm[i].M11 = m_rISPRegs.CCM[i].ccm1.bits.M11;
        pDst->ccm[i].M12 = m_rISPRegs.CCM[i].ccm1.bits.M12;
        pDst->ccm[i].M13 = m_rISPRegs.CCM[i].ccm2.bits.M13;
        pDst->ccm[i].M21 = m_rISPRegs.CCM[i].ccm2.bits.M21;
        pDst->ccm[i].M22 = m_rISPRegs.CCM[i].ccm3.bits.M22;
        pDst->ccm[i].M23 = m_rISPRegs.CCM[i].ccm3.bits.M23;
        pDst->ccm[i].M31 = m_rISPRegs.CCM[i].ccm4.bits.M31;
        pDst->ccm[i].M32 = m_rISPRegs.CCM[i].ccm4.bits.M32;
        pDst->ccm[i].M33 = m_rISPRegs.CCM[i].ccm5.bits.M33;
    }

    *pu4RealParaOutLen = sizeof(o_type);

    return  CCTIF_NO_ERROR;
}

/*
puParaIn
    ACDK_CCT_NVRAM_CCM_PARA
u4ParaInLen
    sizeof(ACDK_CCT_NVRAM_CCM_PARA)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA )
{
    typedef ACDK_CCT_NVRAM_CCM_PARA i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const       pSrc = reinterpret_cast<i_type*>(puParaIn);

    for (MUINT32 i=0; i<NVRAM_CCM_TBL_NUM; i++)
    {
        m_rISPRegs.CCM[i].ccm1.bits.M11 = pSrc->ccm[i].M11;
        m_rISPRegs.CCM[i].ccm1.bits.M12 = pSrc->ccm[i].M12;
        m_rISPRegs.CCM[i].ccm2.bits.M13 = pSrc->ccm[i].M13;
        m_rISPRegs.CCM[i].ccm2.bits.M21 = pSrc->ccm[i].M21;
        m_rISPRegs.CCM[i].ccm3.bits.M22 = pSrc->ccm[i].M22;
        m_rISPRegs.CCM[i].ccm3.bits.M23 = pSrc->ccm[i].M23;
        m_rISPRegs.CCM[i].ccm4.bits.M31 = pSrc->ccm[i].M31;
        m_rISPRegs.CCM[i].ccm4.bits.M32 = pSrc->ccm[i].M32;
        m_rISPRegs.CCM[i].ccm5.bits.M33 = pSrc->ccm[i].M33;
    }

    return  CCTIF_NO_ERROR;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM )
{
    MBOOL fgEnableDynamicCCM = true;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_SetDynamicCCM,
        pInBuf:             &fgEnableDynamicCCM,
        u4InBufSize:        sizeof(MBOOL),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    return  CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM )
{
    MBOOL fgEnableDynamicCCM = false;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_SetDynamicCCM,
        pInBuf:             &fgEnableDynamicCCM,
        u4InBufSize:        sizeof(MBOOL),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    return  CCTIF_NO_ERROR;
}

/*
puParaIn
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaInLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS )
{
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MBOOL fgEnableDynamicCCM = reinterpret_cast<i_type*>(puParaIn)->Enable;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_SetDynamicCCM,
        pInBuf:             &fgEnableDynamicCCM,
        u4InBufSize:        sizeof(MBOOL),
        pOutBuf:            NULL,
        u4OutBufSize:       0,
        u4ActualOutSize:    0
    };

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd)) )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    return  CCTIF_NO_ERROR;
}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS )
{
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    MBOOL fgIsDynamicCCMEnabled = false;
    NSIspTuning::CmdArg_T cmd = {
        eCmd:               NSIspTuning::ECmd_GetDynamicCCM,
        pInBuf:             NULL,
        u4InBufSize:        0,
        pOutBuf:            &fgIsDynamicCCMEnabled,
        u4OutBufSize:       sizeof(MBOOL),
        u4ActualOutSize:    0
    };

    if  (
        0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&cmd))
    ||  sizeof(MBOOL) != cmd.u4ActualOutSize
        )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    reinterpret_cast<o_type*>(puParaOut)->Enable = fgIsDynamicCCMEnabled;

    *pu4RealParaOutLen = sizeof(o_type);

    return  CCTIF_NO_ERROR;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
puParaIn
    ACDK_CCT_ISP_ACCESS_NVRAM_REG_INDEX
u4ParaInLen
    sizeof(MUINT32);
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_SET_CCM_MODE )
{
    typedef MUINT32 i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    MUINT32 const u4Index = *reinterpret_cast<i_type const*>(puParaIn);

    MY_LOG("[ACDK_CCT_OP_SET_CCM_MODE] CCM Index: (old, new)=(%d, %d)", m_rISPRegsIdx.CCM, u4Index);

    if  ( IspNvramRegMgr::NUM_CCM <= u4Index )
    {
        return  CCTIF_BAD_PARAM;
    }

    m_rISPRegsIdx.CCM = static_cast<MUINT8>(u4Index);
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
u4ParaInLen
puParaOut
    MUINT32
u4ParaOutLen
    sizeof(MUINT32)
pu4RealParaOutLen
    sizeof(MUINT32)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_GET_CCM_MODE )
{
    typedef MUINT32 o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    *reinterpret_cast<o_type*>(puParaOut) = m_rISPRegsIdx.CCM;
    *pu4RealParaOutLen = sizeof(o_type);
    MY_LOG("[ACDK_CCT_OP_GET_CCM_MODE] Current CCM Index: %d", m_rISPRegsIdx.CCM);
    return  CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
CctCtrl::
dumpIspReg(MUINT32 const u4Addr) const
{
#if 1
    MY_LOG("[dumpIspReg] isp reg:%04X = 0x%08X", u4Addr, m_pIspDrv->readReg(u4Addr));
#endif
}


MVOID
CctCtrl::
setIspOnOff_OB(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.OB;
    if  (fgOn)
    {
        if  ( ! m_fgEnabled_OB )
        {
            //  OB: from DISABLE to ENABLE
            //  restore the backup.
            m_rISPRegs.OB[u4Index].set[0] = m_u4Backup_OB;
        }
        m_fgEnabled_OB = MTRUE;
    }
    else
    {
        if  ( m_fgEnabled_OB )
        {
            //  OB: from ENABLE to DISABLE.
            //  (1) backup the enabled OB.
            m_u4Backup_OB = m_rISPRegs.OB[u4Index].set[0];
            //  (2) disable OB by assigning 0.
            m_rISPRegs.OB[u4Index].set[0] = 0;
        }
        m_fgEnabled_OB = MFALSE;
    }
}


MVOID
CctCtrl::
setIspOnOff_DP(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.DP;
    ISP_NR1_CTRL_T& rDPCtrl  = m_rISPRegs.DP[u4Index].ctrl.bits;
    rDPCtrl.DP_EN   = fgOn;
}


MVOID
CctCtrl::
setIspOnOff_NR1(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.NR1;
    ISP_NR1_CTRL_T&  rNR1Ctrl  = m_rISPRegs.NR1[u4Index].ctrl.bits;
    rNR1Ctrl.CT_EN  = fgOn;
    rNR1Ctrl.NR_EN  = fgOn;
}


MVOID
CctCtrl::
setIspOnOff_NR2(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.NR2;
    ISP_NR2_CTRL_T& rNR2Ctrl = m_rISPRegs.NR2[u4Index].ctrl.bits;
    rNR2Ctrl.ENC    = fgOn;
    rNR2Ctrl.ENY    = fgOn;
}


MVOID
CctCtrl::
setIspOnOff_EE(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.EE;
    ISP_EE_CTRL_T&  rEECtrl = m_rISPRegs.EE[u4Index].ee_ctrl.bits;
    rEECtrl.YEDGE_EN = fgOn;
    rEECtrl.RGBEDGE_EN = 0;//unused
}


MVOID
CctCtrl::
setIspOnOff_Saturation(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.Saturation;
    ISP_YCCGO_CTRL_T& rSatCtrl = m_rISPRegs.Saturation[u4Index].ctrl.bits;
    rSatCtrl.ENC3 = fgOn;
}


MVOID
CctCtrl::
setIspOnOff_Contrast(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.Contrast;
    ISP_YCCGO_CTRL_T& rContrastCtrl = m_rISPRegs.Contrast[u4Index].ctrl.bits;
    rContrastCtrl.ENY3 = fgOn;
}


MVOID
CctCtrl::
setIspOnOff_Hue(MBOOL const fgOn)
{
    MUINT32 const u4Index = m_rISPRegsIdx.Hue;
    ISP_YCCGO_CTRL_T& rHueCtrl = m_rISPRegs.Hue[u4Index].ctrl.bits;
    rHueCtrl.ENC2 = fgOn;
}


MBOOL
CctCtrl::
getIspOnOff_OB() const
{
    return  m_fgEnabled_OB;
}


MBOOL
CctCtrl::
getIspOnOff_DP() const
{
    MUINT32 const u4Index = m_rISPRegsIdx.DP;
    ISP_NR1_CTRL_T const& rCtrl = m_rISPRegs.DP[u4Index].ctrl.bits;
    return  (0 != rCtrl.DP_EN);
}


MBOOL
CctCtrl::
getIspOnOff_NR1() const
{
    MUINT32 const u4Index = m_rISPRegsIdx.NR1;
    ISP_NR1_CTRL_T const& rCtrl = m_rISPRegs.NR1[u4Index].ctrl.bits;
    return  (0 != rCtrl.CT_EN)
        ||  (0 != rCtrl.NR_EN);
}


MBOOL
CctCtrl::
getIspOnOff_NR2() const
{
    MUINT32 const u4Index = m_rISPRegsIdx.NR2;
    ISP_NR2_CTRL_T const& rCtrl = m_rISPRegs.NR2[u4Index].ctrl.bits;
    return  (0 != rCtrl.ENC)
        ||  (0 != rCtrl.ENY);
}


MBOOL
CctCtrl::
getIspOnOff_EE() const
{
    MUINT32 const u4Index = m_rISPRegsIdx.EE;
    ISP_EE_CTRL_T const& rCtrl = m_rISPRegs.EE[u4Index].ee_ctrl.bits;
    return  (0 != rCtrl.YEDGE_EN)
        ||  (0 != rCtrl.RGBEDGE_EN);
}


MBOOL
CctCtrl::
getIspOnOff_Saturation() const
{
    MUINT32 const u4Index = m_rISPRegsIdx.Saturation;
    ISP_YCCGO_CTRL_T const& rCtrl = m_rISPRegs.Saturation[u4Index].ctrl.bits;
    return  (0 != rCtrl.ENC3);
}


MBOOL
CctCtrl::
getIspOnOff_Contrast() const
{
    MUINT32 const u4Index = m_rISPRegsIdx.Contrast;
    ISP_YCCGO_CTRL_T const& rCtrl = m_rISPRegs.Contrast[u4Index].ctrl.bits;
    return  (0 != rCtrl.ENY3);
}


MBOOL
CctCtrl::
getIspOnOff_Hue() const
{

    MUINT32 const u4Index = m_rISPRegsIdx.Hue;
    ISP_YCCGO_CTRL_T const& rCtrl = m_rISPRegs.Hue[u4Index].ctrl.bits;
    return  (0 != rCtrl.ENC2);
}


MINT32
CctCtrl::
setIspOnOff(MUINT32 const u4Category, MBOOL const fgOn)
{
#define SET_ISP_ON_OFF(_u4Addr, _category)\
    case EIsp_Category_##_category:\
        setIspOnOff_##_category(fgOn);\
        if  ( ! updateIspRegs() )\
        {\
            return  CCTIF_INVALID_DRIVER;\
        }\
        MY_LOG("[setIspOnOff] < %s >", #_category);\
        dumpIspReg(_u4Addr);\
        break

    switch  ( u4Category )
    {
    SET_ISP_ON_OFF(0x0090, OB);
    SET_ISP_ON_OFF(0x0550, DP);
    SET_ISP_ON_OFF(0x0550, NR1);
    SET_ISP_ON_OFF(0x0500, NR2);
    SET_ISP_ON_OFF(0x05A0, EE);
    SET_ISP_ON_OFF(0x0600, Saturation);
    SET_ISP_ON_OFF(0x0600, Contrast);
    SET_ISP_ON_OFF(0x0600, Hue);
    default:
        MY_ERR("[setIspOnOff] Unsupported Category(%d)", u4Category);
        return  CCTIF_BAD_PARAM;
    }

    return  CCTIF_NO_ERROR;
}


MINT32
CctCtrl::
getIspOnOff(MUINT32 const u4Category, MBOOL& rfgOn) const
{
#define GET_ISP_ON_OFF(_u4Addr, _category)\
    case EIsp_Category_##_category:\
        MY_LOG("[getIspOnOff] < %s >", #_category);\
        dumpIspReg(_u4Addr);\
        rfgOn = getIspOnOff_##_category();\
        break

    switch  ( u4Category )
    {
    GET_ISP_ON_OFF(0x0090, OB);
    GET_ISP_ON_OFF(0x0550, DP);
    GET_ISP_ON_OFF(0x0550, NR1);
    GET_ISP_ON_OFF(0x0500, NR2);
    GET_ISP_ON_OFF(0x05A0, EE);
    GET_ISP_ON_OFF(0x0600, Saturation);
    GET_ISP_ON_OFF(0x0600, Contrast);
    GET_ISP_ON_OFF(0x0600, Hue);
    default:
        MY_ERR("[getIspOnOff] Unsupported Category(%d)", u4Category);
        return  CCTIF_BAD_PARAM;
    }
    return  CCTIF_NO_ERROR;
}


/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_SET_ISP_ON )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type const*>(puParaIn);

    MINT32 const err = setIspOnOff(eCategory, 1);

    MY_LOG("[-ACDK_CCT_OP_SET_ISP_ON] eCategory(%d), err(%x)", eCategory, err);
    return  err;
}

/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_SET_ISP_OFF )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type const*>(puParaIn);

    MINT32 const err = setIspOnOff(eCategory, 0);

    MY_LOG("[-ACDK_CCT_OP_SET_ISP_OFF] eCategory(%d), err(%x)", eCategory, err);
    return  err;
}

/*
puParaIn
    ACDK_CCT_ISP_REG_CATEGORY
u4ParaInLen
    sizeof(ACDK_CCT_ISP_REG_CATEGORY)
puParaOut
    ACDK_CCT_FUNCTION_ENABLE_STRUCT
u4ParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
pu4RealParaOutLen
    sizeof(ACDK_CCT_FUNCTION_ENABLE_STRUCT)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_GET_ISP_ON_OFF )
{
    typedef ACDK_CCT_ISP_REG_CATEGORY       i_type;
    typedef ACDK_CCT_FUNCTION_ENABLE_STRUCT o_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    i_type const eCategory = *reinterpret_cast<i_type*>(puParaIn);
    MBOOL&       rfgEnable = reinterpret_cast<o_type*>(puParaOut)->Enable;

    MINT32 const err = getIspOnOff(eCategory, rfgEnable);

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("[-ACDK_CCT_OP_GET_ISP_ON_OFF] (eCategory, rfgEnable)=(%d, %d)", eCategory, rfgEnable);
    return  err;
}


/*******************************************************************************
*
********************************************************************************/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM )
{
    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM )");

    err = m_rBufIf_ISP.refresh(m_eSensorEnum, m_u4SensorID);
    if  ( CCTIF_NO_ERROR != err )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM] m_rBufIf_ISP.refresh() fail (0x%x)\n", err);
        return  err;
    }
    //
    return  err;
}


IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM )
{
    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM )");

    err = m_rBufIf_ISP.flush(m_eSensorEnum, m_u4SensorID);
    if  ( CCTIF_NO_ERROR != err )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_SAVE_TO_NVRAM] m_rBufIf_ISP.flush() fail (0x%x)\n", err);
        return  err;
    }
    //
    return  err;
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE
u4ParaInLen
    sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_TABLE )
{
    typedef ACDK_CCT_ACCESS_NVRAM_PCA_TABLE type;
    if  ( sizeof(type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    type*const pAccess = reinterpret_cast<type*>(puParaIn);

    MUINT32 const u4Offset = pAccess->u4Offset;
    MUINT32 const u4Count = pAccess->u4Count;
    MUINT8  const u8ColorTemperature = pAccess->u8ColorTemperature;

    if  (
            u4Offset >= PCA_BIN_NUM
        ||  u4Count  == 0
        ||  u4Count  > (PCA_BIN_NUM-u4Offset)
        )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_SET_PCA_TABLE] bad (PCA_BIN_NUM, u4Count, u4Offset)=(%d, %d, %d)\n", PCA_BIN_NUM, u4Count, u4Offset);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_PCA_BIN_T* pBuf_pc = &pAccess->buffer[u4Offset];
    ISP_NVRAM_PCA_BIN_T* pBuf_fw = NULL;
    switch (u8ColorTemperature)
    {
    case 0:
        pBuf_fw = &m_rISPPca.PCA_LUTs.lut_lo[u4Offset];
        break;
    case 1:
        pBuf_fw = &m_rISPPca.PCA_LUTs.lut_md[u4Offset];
        break;
    case 2:
        pBuf_fw = &m_rISPPca.PCA_LUTs.lut_hi[u4Offset];
        break;
    default:
        MY_ERR("[ACDK_CCT_OP_ISP_SET_PCA_TABLE] bad u8ColorTemperature(%d)\n", u8ColorTemperature);
        return  CCTIF_BAD_PARAM;
    }

    ::memcpy(pBuf_fw, pBuf_pc, u4Count*sizeof(ISP_NVRAM_PCA_BIN_T));

    return  CCTIF_NO_ERROR;
}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_ACCESS_NVRAM_PCA_TABLE
u4ParaOutLen
    sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ACCESS_NVRAM_PCA_TABLE)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_TABLE )
{
    typedef ACDK_CCT_ACCESS_NVRAM_PCA_TABLE type;
    if  ( sizeof(type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    type*const pAccess = reinterpret_cast<type*>(puParaOut);

    MUINT32 const u4Offset = pAccess->u4Offset;
    MUINT32 const u4Count = pAccess->u4Count;
    MUINT8  const u8ColorTemperature = pAccess->u8ColorTemperature;

    if  (
            u4Offset >= PCA_BIN_NUM
        ||  u4Count  == 0
        ||  u4Count  > (PCA_BIN_NUM-u4Offset)
        )
    {
        MY_ERR("[ACDK_CCT_OP_ISP_GET_PCA_TABLE] bad (PCA_BIN_NUM, u4Count, u4Offset)=(%d, %d, %d)\n", PCA_BIN_NUM, u4Count, u4Offset);
        return  CCTIF_BAD_PARAM;
    }

    ISP_NVRAM_PCA_BIN_T* pBuf_pc = &pAccess->buffer[u4Offset];
    ISP_NVRAM_PCA_BIN_T* pBuf_fw = NULL;
    switch (u8ColorTemperature)
    {
    case 0:
        pBuf_fw = &m_rISPPca.PCA_LUTs.lut_lo[u4Offset];
        break;
    case 1:
        pBuf_fw = &m_rISPPca.PCA_LUTs.lut_md[u4Offset];
        break;
    case 2:
        pBuf_fw = &m_rISPPca.PCA_LUTs.lut_hi[u4Offset];
        break;
    default:
        MY_ERR("[ACDK_CCT_OP_ISP_GET_PCA_TABLE] bad u8ColorTemperature(%d)\n", u8ColorTemperature);
        return  CCTIF_BAD_PARAM;
    }

    ::memcpy(pBuf_pc, pBuf_fw, u4Count*sizeof(ISP_NVRAM_PCA_BIN_T));
    *pu4RealParaOutLen = sizeof(type);

    return  CCTIF_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
    ACDK_CCT_ACCESS_PCA_CONFIG
u4ParaInLen
    sizeof(ACDK_CCT_ACCESS_PCA_CONFIG)
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_PARA )
{
    typedef ACDK_CCT_ACCESS_PCA_CONFIG i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pAccess = reinterpret_cast<i_type*>(puParaIn);

    m_rISPPca.Config.ctrl.bits.EN = pAccess->EN;

    if  ( ! updateIspRegs() )
    {
        return  CCTIF_INVALID_DRIVER;
    }

    dumpIspReg(0x0630);
    dumpIspReg(0x0634);
    dumpIspReg(0x0638);
    MY_LOG("-[ACDK_CCT_OP_ISP_SET_PCA_PARA] PCA_EN(%d)\n", pAccess->EN);
    return  CCTIF_NO_ERROR;
}

/*
puParaIn
u4ParaInLen
puParaOut
    ACDK_CCT_ACCESS_PCA_CONFIG
u4ParaOutLen
    sizeof(ACDK_CCT_ACCESS_PCA_CONFIG)
pu4RealParaOutLen
    sizeof(ACDK_CCT_ACCESS_PCA_CONFIG)
*/
IMP_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_PARA )
{
    typedef ACDK_CCT_ACCESS_PCA_CONFIG o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const pAccess = reinterpret_cast<o_type*>(puParaOut);

    pAccess->EN = m_rISPPca.Config.ctrl.bits.EN;
    *pu4RealParaOutLen = sizeof(o_type);

    dumpIspReg(0x0630);
    dumpIspReg(0x0634);
    dumpIspReg(0x0638);
    MY_LOG("-[ACDK_CCT_OP_ISP_GET_PCA_PARA] PCA_EN(%d)\n", pAccess->EN);
    return  CCTIF_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF )
{
    MUINT32 u4Mode;
    MBOOL fgUpdateShadingNVRAMData = true;
    typedef ACDK_CCT_MODULE_CTRL_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pShadingPara = reinterpret_cast<i_type*>(puParaIn);

    switch (pShadingPara->Mode)
    {
        case CAMERA_TUNING_PREVIEW_SET:
            u4Mode = 0;
            break;
        case CAMERA_TUNING_CAPTURE_SET:
            u4Mode = 1;
            break;
        default:
            u4Mode = 2;
            break;
    }

    m_rISPRegs.Shading[u4Mode].shading_ctrl1.bits.SHADING_EN = pShadingPara->Enable;

    NSIspTuning::CmdArg_T Cmd_ModShadingNVRAMData;
      Cmd_ModShadingNVRAMData.eCmd       = NSIspTuning::ECmd_ModShadingNVRAMData;
      Cmd_ModShadingNVRAMData.pInBuf     = &fgUpdateShadingNVRAMData;
      Cmd_ModShadingNVRAMData.u4InBufSize= sizeof(MBOOL);

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&Cmd_ModShadingNVRAMData))  )
    {
        MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF]"
            " (Update Shading register setting from NVRAM fail)\n");
        return  CCTIF_UNKNOWN_ERROR;
    }

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF]"
        " (Shading mode pre/cap, on/off)=(%d,%d)\n"
        , pShadingPara->Mode
        , m_rISPRegs.Shading[u4Mode].shading_ctrl1.bits.SHADING_EN );

    return  CCTIF_NO_ERROR;
}


IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF )
{
    MUINT32 u4Mode;
    typedef ACDK_CCT_MODULE_CTRL_STRUCT o_type;
    if  ( sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut )
        return  CCTIF_BAD_PARAM;

    o_type*const pShadingPara = reinterpret_cast<o_type*>(puParaOut);

    switch (pShadingPara->Mode)
    {
        case CAMERA_TUNING_PREVIEW_SET:
            u4Mode = 0;
            break;
        case CAMERA_TUNING_CAPTURE_SET:
            u4Mode = 1;
            break;
        default:
            u4Mode = 2;
            break;
    }

   //  enable/disable       : (1, 0)
    pShadingPara->Enable
        = m_rISPRegs.Shading[u4Mode].shading_ctrl1.bits.SHADING_EN ? true : false;

    *pu4RealParaOutLen = sizeof(o_type);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF]"
        " (Shading mode pre/cap, on/off)=(%d, %d)\n"
        , pShadingPara->Mode
        , pShadingPara->Enable
        );

    return  CCTIF_NO_ERROR;
}
/*******************************************************************************
* Because CCT tool is working on preview mode (Lsc_mgr.m_u4Mode = 0)
* 1.
*    Capture parameters will not be update to isp register
*    (since Lsc_mgr.m_u4Mode doesn't changed) till capture command
* 2.
*     Preivew parameters will be updated to reigster immediately at "prepareHw_PerFrame_Shading()"
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA )
{
    MUINT32 u4Mode;
    MBOOL fgUpdateShadingNVRAMData = true;
    typedef ACDK_CCT_SHADING_COMP_STRUCT i_type;
    if  ( sizeof(i_type) != u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pShadingPara = reinterpret_cast<i_type*>(puParaIn);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA] (Shading mode pre/cap)=(%d)\n", pShadingPara->SHADING_MODE);

    switch (pShadingPara->SHADING_MODE)
    {
        case CAMERA_TUNING_PREVIEW_SET:
            u4Mode = 0;
            break;
        case CAMERA_TUNING_CAPTURE_SET:
            u4Mode = 1;
            break;
        default:
            u4Mode = 2;
            break;
    }

    m_rISPRegs.Shading[u4Mode].shading_ctrl1.bits.SDBLK_TRIG = pShadingPara->pShadingComp->SDBLK_TRIG;
    m_rISPRegs.Shading[u4Mode].shading_ctrl1.bits.SHADING_EN = pShadingPara->pShadingComp->SHADING_EN;
    m_rISPRegs.Shading[u4Mode].shading_ctrl2.bits.SDBLK_XNUM = pShadingPara->pShadingComp->SHADINGBLK_XNUM - 1;
    m_rISPRegs.Shading[u4Mode].shading_ctrl2.bits.SDBLK_YNUM = pShadingPara->pShadingComp->SHADINGBLK_YNUM - 1;
    m_rISPRegs.Shading[u4Mode].shading_ctrl2.bits.SDBLK_WIDTH = pShadingPara->pShadingComp->SHADINGBLK_WIDTH;
    m_rISPRegs.Shading[u4Mode].shading_ctrl2.bits.SDBLK_HEIGHT = pShadingPara->pShadingComp->SHADINGBLK_HEIGHT;
    //m_rISPRegs.Shading[u4Mode].shading_read_addr.bits.SHADING_RADDR =  g_ShadingTblAddr;
    m_rISPRegs.Shading[u4Mode].shading_last_blk.bits.SDBLK_LWIDTH = pShadingPara->pShadingComp->SD_LWIDTH;
    m_rISPRegs.Shading[u4Mode].shading_last_blk.bits.SDBLK_LHEIGHT = pShadingPara->pShadingComp->SD_LHEIGHT;
    m_rISPRegs.Shading[u4Mode].shading_ratio_cfg.bits.SDBLK_RATIO00 = pShadingPara->pShadingComp->SDBLK_RATIO00;
    m_rISPRegs.Shading[u4Mode].shading_ratio_cfg.bits.SDBLK_RATIO01 = pShadingPara->pShadingComp->SDBLK_RATIO01;
    m_rISPRegs.Shading[u4Mode].shading_ratio_cfg.bits.SDBLK_RATIO10 = pShadingPara->pShadingComp->SDBLK_RATIO10;
    m_rISPRegs.Shading[u4Mode].shading_ratio_cfg.bits.SDBLK_RATIO11 = pShadingPara->pShadingComp->SDBLK_RATIO11;

    NSIspTuning::CmdArg_T Cmd_ModShadingNVRAMData;
      Cmd_ModShadingNVRAMData.eCmd       = NSIspTuning::ECmd_ModShadingNVRAMData;
      Cmd_ModShadingNVRAMData.pInBuf     = &fgUpdateShadingNVRAMData;
      Cmd_ModShadingNVRAMData.u4InBufSize= sizeof(MBOOL);

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&Cmd_ModShadingNVRAMData))  )
    {
        MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA]"
            " (Update Shading register setting from NVRAM fail)\n");
        return  CCTIF_UNKNOWN_ERROR;
    }

       // log nvram data
    MY_LOG("ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA, mode = %d \n", pShadingPara->SHADING_MODE);
    MY_LOG("SDBLK_TRIG:%d\n", m_rISPRegs.Shading[u4Mode].shading_ctrl1.bits.SDBLK_TRIG);
    MY_LOG("SHADING_EN:%d\n", m_rISPRegs.Shading[u4Mode].shading_ctrl1.bits.SHADING_EN);
    MY_LOG("SHADINGBLK_XNUM:%d\n", m_rISPRegs.Shading[u4Mode].shading_ctrl2.bits.SDBLK_XNUM);
    MY_LOG("SHADINGBLK_YNUM:%d\n", m_rISPRegs.Shading[u4Mode].shading_ctrl2.bits.SDBLK_YNUM);
    MY_LOG("SHADINGBLK_WIDTH:%d\n",  m_rISPRegs.Shading[u4Mode].shading_ctrl2.bits.SDBLK_WIDTH);
    MY_LOG("SHADINGBLK_HEIGHT:%d\n", m_rISPRegs.Shading[u4Mode].shading_ctrl2.bits.SDBLK_HEIGHT);
    MY_LOG("SHADINGBLK_ADDRESS(can not modify by user):%d\n", m_rISPRegs.Shading[u4Mode].shading_read_addr.bits.SHADING_RADDR);
    MY_LOG("SD_LWIDTH:%d\n", m_rISPRegs.Shading[u4Mode].shading_last_blk.bits.SDBLK_LWIDTH);
    MY_LOG("SD_LHEIGHT:%d\n", m_rISPRegs.Shading[u4Mode].shading_last_blk.bits.SDBLK_LHEIGHT);
    MY_LOG("SDBLK_RATIO00:%d\n", m_rISPRegs.Shading[u4Mode].shading_ratio_cfg.bits.SDBLK_RATIO00);
    MY_LOG("SDBLK_RATIO01:%d\n", m_rISPRegs.Shading[u4Mode].shading_ratio_cfg.bits.SDBLK_RATIO01);
    MY_LOG("SDBLK_RATIO10:%d\n", m_rISPRegs.Shading[u4Mode].shading_ratio_cfg.bits.SDBLK_RATIO10);
    MY_LOG("SDBLK_RATIO11:%d\n", m_rISPRegs.Shading[u4Mode].shading_ratio_cfg.bits.SDBLK_RATIO11);

    return  CCTIF_NO_ERROR;
}


IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA )
{
    MUINT8 *pCompMode=reinterpret_cast<MUINT8*>(puParaIn);
    MUINT32 u4Mode;
    typedef ACDK_CCT_SHADING_COMP_STRUCT o_type;
    //if  (! puParaIn || sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut)
    if  (! puParaIn || sizeof(o_type) != u4ParaOutLen  || ! puParaOut)
        return  CCTIF_BAD_PARAM;

    o_type*const pShadingPara = reinterpret_cast<o_type*>(puParaOut);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA] (Shading mode pre/cap)=(%d)\n", *pCompMode);

    switch (*pCompMode)
    {
        case CAMERA_TUNING_PREVIEW_SET:
            u4Mode = 0;
            break;
        case CAMERA_TUNING_CAPTURE_SET:
            u4Mode = 1;
            break;
        default:
            u4Mode = 2;
            break;
    }

    pShadingPara->pShadingComp->SDBLK_TRIG = m_rISPRegs.Shading[u4Mode].shading_ctrl1.bits.SDBLK_TRIG;
    pShadingPara->pShadingComp->SHADING_EN = m_rISPRegs.Shading[u4Mode].shading_ctrl1.bits.SHADING_EN;
    pShadingPara->pShadingComp->SHADINGBLK_XNUM = m_rISPRegs.Shading[u4Mode].shading_ctrl2.bits.SDBLK_XNUM +1;
    pShadingPara->pShadingComp->SHADINGBLK_YNUM =  m_rISPRegs.Shading[u4Mode].shading_ctrl2.bits.SDBLK_YNUM + 1;
    pShadingPara->pShadingComp->SHADINGBLK_WIDTH = m_rISPRegs.Shading[u4Mode].shading_ctrl2.bits.SDBLK_WIDTH;
    pShadingPara->pShadingComp->SHADINGBLK_HEIGHT = m_rISPRegs.Shading[u4Mode].shading_ctrl2.bits.SDBLK_HEIGHT;
    pShadingPara->pShadingComp->SHADING_RADDR = m_rISPRegs.Shading[u4Mode].shading_read_addr.bits.SHADING_RADDR;
    pShadingPara->pShadingComp->SD_LWIDTH = m_rISPRegs.Shading[u4Mode].shading_last_blk.bits.SDBLK_LWIDTH;
    pShadingPara->pShadingComp->SD_LHEIGHT = m_rISPRegs.Shading[u4Mode].shading_last_blk.bits.SDBLK_LHEIGHT;
    pShadingPara->pShadingComp->SDBLK_RATIO00 = m_rISPRegs.Shading[u4Mode].shading_ratio_cfg.bits.SDBLK_RATIO00;
    pShadingPara->pShadingComp->SDBLK_RATIO01 = m_rISPRegs.Shading[u4Mode].shading_ratio_cfg.bits.SDBLK_RATIO01;
    pShadingPara->pShadingComp->SDBLK_RATIO10 = m_rISPRegs.Shading[u4Mode].shading_ratio_cfg.bits.SDBLK_RATIO10;
    pShadingPara->pShadingComp->SDBLK_RATIO11 = m_rISPRegs.Shading[u4Mode].shading_ratio_cfg.bits.SDBLK_RATIO11;


    // log nvram data
    MY_LOG("ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA, mode = %d \n", (*pCompMode));
    MY_LOG("SDBLK_TRIG:%d\n", pShadingPara->pShadingComp->SDBLK_TRIG);
    MY_LOG("SHADING_EN:%d\n", pShadingPara->pShadingComp->SHADING_EN );
    MY_LOG("SHADINGBLK_XNUM:%d\n", pShadingPara->pShadingComp->SHADINGBLK_XNUM);
    MY_LOG("SHADINGBLK_YNUM:%d\n", pShadingPara->pShadingComp->SHADINGBLK_YNUM);
    MY_LOG("SHADINGBLK_WIDTH:%d\n",  pShadingPara->pShadingComp->SHADINGBLK_WIDTH);
    MY_LOG("SHADINGBLK_HEIGHT:%d\n", pShadingPara->pShadingComp->SHADINGBLK_HEIGHT);
    MY_LOG("SHADINGBLK_ADDRESS(can not modify by user):%d\n", pShadingPara->pShadingComp->SHADING_RADDR);
    MY_LOG("SD_LWIDTH:%d\n", pShadingPara->pShadingComp->SD_LWIDTH);
    MY_LOG("SD_LHEIGHT:%d\n", pShadingPara->pShadingComp->SD_LHEIGHT);
    MY_LOG("SDBLK_RATIO00:%d\n", pShadingPara->pShadingComp->SDBLK_RATIO00);
    MY_LOG("SDBLK_RATIO01:%d\n", pShadingPara->pShadingComp->SDBLK_RATIO01);
    MY_LOG("SDBLK_RATIO10:%d\n", pShadingPara->pShadingComp->SDBLK_RATIO10);
    MY_LOG("SDBLK_RATIO11:%d\n", pShadingPara->pShadingComp->SDBLK_RATIO11);

    return  CCTIF_NO_ERROR;
}
/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX )
{
    MBOOL fgUpdateShadingNVRAMData = true;
    if  ( ! puParaIn )
        return  CCTIF_BAD_PARAM;
    MUINT8 *pShadingIndex  = reinterpret_cast<MUINT8*> (puParaIn);
    MUINT32 u4CCT = 0;

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX] "
        "(Set Shading table index to)=(%d)\n", *pShadingIndex);

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SET_SHADING_IDX, *pShadingIndex) )
    {
        MY_LOG("[ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX] "
            "(Set Shading table index fail\n");
        return  CCTIF_INVALID_DRIVER;
    }

    NSIspTuning::CmdArg_T Cmd_ModShadingNVRAMData;
      Cmd_ModShadingNVRAMData.eCmd       = NSIspTuning::ECmd_ModShadingNVRAMData;
      Cmd_ModShadingNVRAMData.pInBuf     = &fgUpdateShadingNVRAMData;
      Cmd_ModShadingNVRAMData.u4InBufSize= sizeof(MBOOL);

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&Cmd_ModShadingNVRAMData))  )
    {
        MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3]"
            " (Update Shading register setting from NVRAM fail)\n");
        return  CCTIF_UNKNOWN_ERROR;
    }

    return  CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX )
{
    if  ( ! puParaOut )
        return  CCTIF_BAD_PARAM;
    //MUINT8 *pShadingIndex = reinterpret_cast<MUINT8*> puParaOut;

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_GET_SHADING_IDX, reinterpret_cast<int> (puParaOut)) )
    {
        MY_LOG("[ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX] "
            "(Set Shading table index fail\n");
        return  CCTIF_INVALID_DRIVER;
    }

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX] "
        "(Get Shading table index to)=(%d)\n", *puParaOut);

    return  CCTIF_NO_ERROR;
}
/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3 )
{
    MBOOL fgUpdateShadingNVRAMData = true;
    typedef ACDK_CCT_TABLE_SET_STRUCT i_type;
    if  ( sizeof (i_type) !=  u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pShadingtabledata  = reinterpret_cast<i_type*> (puParaIn);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3]"
        " (Shading mode pre/cap, ColorTemp_Idx)=(%d, %d)\n"
        ,pShadingtabledata->Mode
        ,pShadingtabledata->ColorTemp
        );


    MUINT8* p_cct_input_address = (MUINT8 *)(pShadingtabledata->pBuffer);
    MUINT32 length;

    length = pShadingtabledata->Length;

    if(length==0 || length > MAX_SVD_SHADING_SIZE || p_cct_input_address == NULL)
    //if(length==0 || length > 4096 || p_cct_input_address == NULL)
    {
        MY_LOG("[Set Shading Table V3]"
            "length param is wrong or table buffer is null\n");
        return CCTIF_BAD_PARAM;
    }

    if (pShadingtabledata->ColorTemp > 4 )
    {
        MY_LOG("[Set Shading Table V3]"
            "Color tempature  is out of range\n");
        return CCTIF_BAD_PARAM;
    }

    switch (pShadingtabledata->Mode)
    {
        case CAMERA_TUNING_PREVIEW_SET:
            memcpy(&m_rBuf_SD.Shading.PreviewSVDTable[pShadingtabledata->ColorTemp][0], p_cct_input_address, MAX_SVD_SHADING_SIZE);
            break;
        case CAMERA_TUNING_CAPTURE_SET:
            memcpy(&m_rBuf_SD.Shading.CaptureSVDTable[pShadingtabledata->ColorTemp][0], p_cct_input_address, MAX_SVD_SHADING_SIZE);
            break;
        default:
            MY_LOG("[Set Shading Table V3]"
                "Camera mode not support shading table\n");
            return CCTIF_BAD_PARAM;
            break;
    }

    NSIspTuning::CmdArg_T Cmd_ModShadingNVRAMData;
      Cmd_ModShadingNVRAMData.eCmd       = NSIspTuning::ECmd_ModShadingNVRAMData;
      Cmd_ModShadingNVRAMData.pInBuf     = &fgUpdateShadingNVRAMData;
      Cmd_ModShadingNVRAMData.u4InBufSize= sizeof(MBOOL);

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&Cmd_ModShadingNVRAMData))  )
    {
        MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3]"
            " (Update Shading register setting from NVRAM fail)\n");
        return  CCTIF_UNKNOWN_ERROR;
    }

    return  CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3 )
{
    typedef ACDK_CCT_TABLE_SET_STRUCT o_type;
    if  (sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut)
        return  CCTIF_BAD_PARAM;

    o_type*const pShadingtabledata  = reinterpret_cast<o_type*> (puParaOut);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3]"
        " (Shading mode pre/cap, ColorTemp_Idx)=(%d, %d)\n"
        ,pShadingtabledata->Mode
        ,pShadingtabledata->ColorTemp
        );

    MUINT8* p_cct_output_address = (MUINT8 *)(pShadingtabledata->pBuffer);

    if (pShadingtabledata->ColorTemp > 4 )
    {
        MY_LOG("[Get Shading Table V3]"
            "Color tempature  is out of range\n");
        return CCTIF_BAD_PARAM;
    }

    switch (pShadingtabledata->Mode)
    {
        case CAMERA_TUNING_PREVIEW_SET:
            memcpy(p_cct_output_address, &m_rBuf_SD.Shading.PreviewSVDTable[pShadingtabledata->ColorTemp][0], MAX_SVD_SHADING_SIZE);
            break;
        case CAMERA_TUNING_CAPTURE_SET:
            memcpy(p_cct_output_address, &m_rBuf_SD.Shading.CaptureSVDTable[pShadingtabledata->ColorTemp][0], MAX_SVD_SHADING_SIZE);
            break;
        default:
            MY_LOG("[Get Shading Table V3]"
                "Camera mode not support shading table\n");
            return CCTIF_BAD_PARAM;
            break;
    }

    return  CCTIF_NO_ERROR;
}
/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF )
{
    MBOOL fgUpdateShadingNVRAMData = true;
    typedef ACDK_CCT_TABLE_SET_STRUCT i_type;
    if  ( sizeof (i_type) !=  u4ParaInLen || ! puParaIn )
        return  CCTIF_BAD_PARAM;

    i_type*const pShadingtabledata  = reinterpret_cast<i_type*> (puParaIn);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF]"
        " (Shading mode pre/cap, ColorTemp_Idx)=(%d, %d)\n"
        ,pShadingtabledata->Mode
        ,pShadingtabledata->ColorTemp
        );


    MUINT8* p_cct_input_address = (MUINT8 *)(pShadingtabledata->pBuffer);
    MUINT32 length;

    length = pShadingtabledata->Length;

    if(p_cct_input_address == NULL)
    {
        MY_LOG("[Set Shading Table Poly Coef]"
            "length param is wrong or table buffer is null\n");
        return CCTIF_BAD_PARAM;
    }

    if (pShadingtabledata->ColorTemp > 4 )
    {
        MY_LOG("[Set Shading Table Poly Coef]"
            "Color tempature  is out of range\n");
        return CCTIF_BAD_PARAM;
    }

    switch (pShadingtabledata->Mode)
    {
        case CAMERA_TUNING_PREVIEW_SET:
            memcpy(&m_rBuf_SD.Shading.PreviewTable[pShadingtabledata->ColorTemp][0], p_cct_input_address
                , MAX_SHADING_Preview_SIZE*sizeof(m_rBuf_SD.Shading.PreviewTable[pShadingtabledata->ColorTemp][0]));
            break;
        case CAMERA_TUNING_CAPTURE_SET:
            memcpy(&m_rBuf_SD.Shading.CaptureTable[pShadingtabledata->ColorTemp][0], p_cct_input_address
                , MAX_SHADING_Capture_SIZE*sizeof(m_rBuf_SD.Shading.CaptureTable[pShadingtabledata->ColorTemp][0]));
            break;
        default:
            MY_LOG("[Set Shading Table Poly Coef]"
                "Camera mode not support shading table\n");
            return CCTIF_BAD_PARAM;
            break;
    }

    NSIspTuning::CmdArg_T Cmd_ModShadingNVRAMData;
      Cmd_ModShadingNVRAMData.eCmd       = NSIspTuning::ECmd_ModShadingNVRAMData;
      Cmd_ModShadingNVRAMData.pInBuf     = &fgUpdateShadingNVRAMData;
      Cmd_ModShadingNVRAMData.u4InBufSize= sizeof(MBOOL);

    if  ( 0 != m_pIspHal->sendCommand(ISP_CMD_SEND_TUNING_CMD, reinterpret_cast<int>(&Cmd_ModShadingNVRAMData))  )
    {
        MY_LOG("[+ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF]"
            " (Update Shading register setting from NVRAM fail)\n");
        return  CCTIF_UNKNOWN_ERROR;
    }

    return  CCTIF_NO_ERROR;
}

IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF )
{
    typedef ACDK_CCT_TABLE_SET_STRUCT o_type;
    if  (sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut)
        return  CCTIF_BAD_PARAM;

    o_type*const pShadingtabledata  = reinterpret_cast<o_type*> (puParaOut);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF]"
        " (Shading mode pre/cap, ColorTemp_Idx)=(%d, %d)\n"
        ,pShadingtabledata->Mode
        ,pShadingtabledata->ColorTemp
        );

    MUINT8* p_cct_output_address = (MUINT8 *)(pShadingtabledata->pBuffer);

    if (pShadingtabledata->ColorTemp > 4 )
    {
        MY_LOG("[Get Shading Table Poly Coef]"
            "Color tempature  is out of range\n");
        return CCTIF_BAD_PARAM;
    }

    switch (pShadingtabledata->Mode)
    {
        case CAMERA_TUNING_PREVIEW_SET:
            memcpy(p_cct_output_address, &m_rBuf_SD.Shading.PreviewTable[pShadingtabledata->ColorTemp][0]
                , MAX_SHADING_Preview_SIZE*sizeof(m_rBuf_SD.Shading.PreviewTable[pShadingtabledata->ColorTemp][0]));
            break;
        case CAMERA_TUNING_CAPTURE_SET:
            memcpy(p_cct_output_address, &m_rBuf_SD.Shading.CaptureTable[pShadingtabledata->ColorTemp][0]
                , MAX_SHADING_Capture_SIZE*sizeof(m_rBuf_SD.Shading.CaptureTable[pShadingtabledata->ColorTemp][0]));
            break;
        default:
            MY_LOG("[Get Shading Table Poly Coef]"
                "Camera mode not support shading table\n");
            return CCTIF_BAD_PARAM;
            break;
    }

    return  CCTIF_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
/*
puParaIn
u4ParaInLen
puParaOut
u4ParaOutLen
pu4RealParaOutLen
*/
IMP_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA )
{
    typedef ACDK_CCT_NVRAM_SET_STRUCT o_type;
    if  (sizeof(o_type) != u4ParaOutLen || ! pu4RealParaOutLen || ! puParaOut)
        return  CCTIF_BAD_PARAM;

    o_type*const pCAMERA_NVRAM_DATA  = reinterpret_cast<o_type*> (puParaOut);

    MY_LOG("[+ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA]"
        "Mode is %d"
        , pCAMERA_NVRAM_DATA->Mode
        );

    MUINT8* p_cct_output_address = (MUINT8 *)(pCAMERA_NVRAM_DATA->pBuffer);

    switch (pCAMERA_NVRAM_DATA->Mode)
    {
        case CAMERA_NVRAM_DEFECT_STRUCT:
            break;
        case CAMERA_NVRAM_SHADING_STRUCT:
            memcpy(p_cct_output_address, &m_rBuf_SD.Shading
                , sizeof(ISP_SHADING_STRUCT));
            *pu4RealParaOutLen = sizeof(ISP_SHADING_STRUCT);
            break;
        case CAMERA_NVRAM_3A_STRUCT:
            break;
        case CAMERA_NVRAM_ISP_PARAM_STRUCT:
            break;
        default:
            MY_LOG("[Get Camera NVRAM data]"
                "Not support NVRAM structure\n");
            return CCTIF_BAD_PARAM;
            break;
    }

    MY_LOG("PreviewSize :%d\n",  m_rBuf_SD.Shading.PreviewSize);
    MY_LOG("CaptureSize :%d\n", m_rBuf_SD.Shading.CaptureSize);
    MY_LOG("Pre SVD Size :%d\n", m_rBuf_SD.Shading.PreviewSVDSize);
    MY_LOG("Cap SVD Size :%d\n", m_rBuf_SD.Shading.CaptureSVDSize);
    MY_LOG("NVRAM Data :%d\n", m_rBuf_SD.Shading.PreviewTable[0][0]);

    return  CCTIF_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM )
{
    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM )");

    err = m_rBufIf_SD.refresh(m_eSensorEnum, m_u4SensorID);
    if  ( CCTIF_NO_ERROR != err )
    {
        MY_ERR("[ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM] m_rBufIf_SD.refresh() fail (0x%x)\n", err);
        return  err;
    }
    //
    return  err;
}


IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM )
{
    MINT32 err = CCTIF_NO_ERROR;

    MY_LOG("IMP_CCT_CTRL( ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM )");

    err = m_rBufIf_SD.flush(m_eSensorEnum, m_u4SensorID);
    if  ( CCTIF_NO_ERROR != err )
    {
        MY_ERR("[ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM] m_rBufIf_SD.flush() fail (0x%x)\n", err);
        return  err;
    }
    //
    return  err;
}

