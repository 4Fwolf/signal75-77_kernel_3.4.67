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
#define LOG_TAG "aaa_cctop"

#include <stdlib.h>
#include <stdio.h>
#include <cutils/xlog.h>
#include "MTK3A.h"
#include "aaa_hal.h"
#include "MediaAssert.h"
#include "sensor_drv.h"
#include "isp_reg.h"
#include "nvram_drv.h"
#include "isp_drv.h"
#include "mcu_drv.h"
#include "strobe_drv.h"
//#include "AppFeature.h"
#include "cct_feature.h"
#include "eeprom_drv.h"

/*******************************************************************************
*
********************************************************************************/
#define AAA_CCTOP_DEBUG

#ifdef AAA_CCTOP_DEBUG
#define AAA_CCTOP_LOG(fmt, arg...)    XLOGD(" "fmt, ##arg)
#define AAA_CCTOP_ERR(fmt, arg...)    XLOGE("Err: %5d: "fmt, __LINE__, ##arg)
#else
#define AAA_CCTOP_LOG(a,...)
#define AAA_CCTOP_ERR(a,...)
#endif

static MUINT32 g_u4CaptureFlareOffset = 0;

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEEnable(
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_AE_ENABLE]\n");

    enableAE();

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEDisable(
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_AE_DISABLE]\n");

    disableAE();

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEGetEnableInfo(
    MINT32 *a_pEnableAE,
    MUINT32 *a_pOutLen
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_AE_GET_ENABLE_INFO]\n");

    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    *a_pEnableAE = (MINT32)m_pmtk3A->isAEEnable();

    *a_pOutLen = sizeof(MINT32);

    AAA_CCTOP_LOG("[AE Enable] = %d\n", *a_pEnableAE);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAESetAEMode(
    MINT32 a_AEMode
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE]\n");

    m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, a_AEMode);

    AAA_CCTOP_LOG("[AE Mode] = %d\n", a_AEMode);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEGetAEMode(
    MINT32 *a_pAEMode,
    MUINT32 *a_pOutLen
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AE_GET_SCENE_MODE]\n");

    m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, a_pAEMode);

    *a_pOutLen = sizeof(MINT32);

    AAA_CCTOP_LOG("[AE Mode] = %d\n", *a_pAEMode);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAESetMeteringMode(
    MINT32 a_AEMeteringMode
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AE_SET_METERING_MODE]\n");

    m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_METERING_MODE, a_AEMeteringMode);

    AAA_CCTOP_LOG("[AE Metering Mode] = %d\n", a_AEMeteringMode);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEApplyExpParam(
    MVOID *a_pAEExpParam
)
{
    ACDK_AE_MODE_CFG_T *pAEExpParam = (ACDK_AE_MODE_CFG_T *)a_pAEExpParam;
    MUINT32 u4AFEGain = 0, u4IspGain = 1024, u4BinningRatio = 1;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO]\n");

    // Set exposure time
    m_r3AOutput.rAEOutput.rPreviewMode.u4Eposuretime = pAEExpParam->u4Eposuretime;
    setSensorExpTime(m_r3AOutput.rAEOutput.rPreviewMode.u4Eposuretime);

    // Set sensor gain
    if (pAEExpParam->u4GainMode == 0) { // AfeGain and isp gain
        u4AFEGain = pAEExpParam->u4AfeGain;
        u4IspGain = pAEExpParam->u4IspGain;
    }
    else { // ISO
        u4AFEGain = (pAEExpParam->u4ISO * 1024) / m_r3ANVRAMData.rAENVRAM.rDevicesInfo.u4MiniISOGain;
    }

    m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain = u4AFEGain;
    setSensorGain(m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain);
    m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain = u4IspGain;
    setIspRAWGain(m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain>>3, FALSE);

    // Set flare
    m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlare_B = pAEExpParam->uFlareValue;
    m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlare_G = pAEExpParam->uFlareValue;
    m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlare_R = pAEExpParam->uFlareValue;
    m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlareBGain = (MUINT8)(128 * 128 /(128 - pAEExpParam->uFlareValue));
    m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlareGGain = (MUINT8)(128 * 128 /(128 - pAEExpParam->uFlareValue));
    m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlareRGain = (MUINT8)(128 * 128 /(128 - pAEExpParam->uFlareValue));
//    setIspFlare(m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG);

    // Update capture exposure time/gain/flare
    if(m_r3ANVRAMData.rAENVRAM.rDevicesInfo.u4Cap2PreRatio <= 300) {
    	u4BinningRatio = 4;
    } else if(m_r3ANVRAMData.rAENVRAM.rDevicesInfo.u4Cap2PreRatio <= 450){
    	u4BinningRatio = 3;
    } else if(m_r3ANVRAMData.rAENVRAM.rDevicesInfo.u4Cap2PreRatio <= 768){
    	u4BinningRatio = 2;
    } else {
    	u4BinningRatio = 1;
    }

    m_r3AOutput.rAEOutput.rCaptureMode[0].u4Eposuretime = (pAEExpParam->u4Eposuretime)*u4BinningRatio;
    m_r3AOutput.rAEOutput.rCaptureMode[0].u4AfeGain = u4AFEGain;
    m_r3AOutput.rAEOutput.rCaptureMode[0].u4IspGain = u4IspGain;
    m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_B = pAEExpParam->uCaptureFlareValue;
    m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_G = pAEExpParam->uCaptureFlareValue;
    m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_R = pAEExpParam->uCaptureFlareValue;
    m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlareBGain = (MUINT8)(128 * 128 /(128 - pAEExpParam->uCaptureFlareValue));
    m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlareGGain = (MUINT8)(128 * 128 /(128 - pAEExpParam->uCaptureFlareValue));
    m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlareRGain = (MUINT8)(128 * 128 /(128 - pAEExpParam->uCaptureFlareValue));

    // backup the capture parameters.
    m_r3AOutput.rAEOutput.rCaptureMode[2].u4Eposuretime = m_r3AOutput.rAEOutput.rCaptureMode[0].u4Eposuretime;
    m_r3AOutput.rAEOutput.rCaptureMode[2].u4AfeGain = m_r3AOutput.rAEOutput.rCaptureMode[0].u4AfeGain;
    m_r3AOutput.rAEOutput.rCaptureMode[2].u4IspGain = m_r3AOutput.rAEOutput.rCaptureMode[0].u4IspGain;
    m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlare_B = m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_B;
    m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlare_G = m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_G;
    m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlare_R = m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_R;
    m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlareBGain = m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlareBGain;
    m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlareGGain = m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlareGGain;
    m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlareRGain = m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlareRGain;

    m_bFlareAuto = pAEExpParam->bFlareAuto;
    m_u4PreviewFlareOffset = (MUINT32)pAEExpParam->uFlareValue;
    g_u4CaptureFlareOffset = m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_G;

    setIspFlare(m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG);

    AAA_CCTOP_LOG("[Exp Time] = %d\n", pAEExpParam->u4Eposuretime);
    AAA_CCTOP_LOG("[ISO] = %d\n", pAEExpParam->u4ISO);
    AAA_CCTOP_LOG("[AFE Gain] = %d\n", m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain);
    AAA_CCTOP_LOG("[Isp Gain] = %d\n", m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain);
    AAA_CCTOP_LOG("[PV Flare] = %d\n", pAEExpParam->uFlareValue);
    AAA_CCTOP_LOG("[PV Flare Gain] = %d\n", pAEExpParam->uFlareGain);
    AAA_CCTOP_LOG("[CAP Flare] = %d\n", pAEExpParam->uCaptureFlareValue);
    AAA_CCTOP_LOG("[CAP Flare Gain] = %d\n", pAEExpParam->uCaptureFlareGain);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAESetFlickerMode(
    MINT32 a_AEFlickerMode
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AE_SELECT_BAND]\n");

    m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_MODE, a_AEFlickerMode);

    AAA_CCTOP_LOG("[AE Flicker Mode] = %d\n", a_AEFlickerMode);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEGetExpParam(
    MVOID *a_pAEExpParamIn,
    MVOID *a_pAEExpParamOut,
    MUINT32 *a_pOutLen
)
{
    ACDK_AE_MODE_CFG_T *pAEExpParamIn = (ACDK_AE_MODE_CFG_T *)a_pAEExpParamIn;
    ACDK_AE_MODE_CFG_T *pAEExpParamOut = (ACDK_AE_MODE_CFG_T *)a_pAEExpParamOut;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA]\n");

    pAEExpParamOut->u4GainMode = pAEExpParamIn->u4GainMode;
    pAEExpParamOut->u4AfeGain = m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain;
    pAEExpParamOut->u4IspGain = m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain;
    pAEExpParamOut->u4ISO = m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain*(((m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain * m_r3ANVRAMData.rAENVRAM.rDevicesInfo.u4MiniISOGain) + 512) / 1024) / 1024;

    pAEExpParamOut->u4Eposuretime = m_r3AOutput.rAEOutput.rPreviewMode.u4Eposuretime;

    if(m_bFlareAuto)
    {
        pAEExpParamOut->uFlareValue = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlare_G;
        pAEExpParamOut->uFlareGain = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlareGGain;
        pAEExpParamOut->uCaptureFlareValue = m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_G;
        pAEExpParamOut->uCaptureFlareGain = m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlareGGain;
    }
    else
    {
        pAEExpParamOut->uFlareValue = (MUINT8)m_u4PreviewFlareOffset;
        pAEExpParamOut->uFlareGain = (MUINT8)(128 * 128 /(128 - m_u4PreviewFlareOffset));
        pAEExpParamOut->uCaptureFlareValue = (MUINT8)g_u4CaptureFlareOffset;
        pAEExpParamOut->uCaptureFlareGain = (MUINT8)(128 * 128 /(128 - g_u4CaptureFlareOffset));
    }

    pAEExpParamOut->bFlareAuto = m_bFlareAuto;

    *a_pOutLen = sizeof(ACDK_AE_MODE_CFG_T);

    AAA_CCTOP_LOG("[Exp Time] = %d\n", pAEExpParamOut->u4Eposuretime);
    AAA_CCTOP_LOG("[ISO] = %d\n", pAEExpParamOut->u4ISO);
    AAA_CCTOP_LOG("[AFE Gain] = %d\n", pAEExpParamOut->u4AfeGain);
    AAA_CCTOP_LOG("[Isp Gain] = %d\n", pAEExpParamOut->u4IspGain);
    AAA_CCTOP_LOG("[PV Flare] = %d\n", pAEExpParamOut->uFlareValue);
    AAA_CCTOP_LOG("[PV Flare Gain] = %d\n", pAEExpParamOut->uFlareGain);
    AAA_CCTOP_LOG("[CAP Flare] = %d\n", pAEExpParamOut->uCaptureFlareValue);
    AAA_CCTOP_LOG("[CAP Flare Gain] = %d\n", pAEExpParamOut->uCaptureFlareGain);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEGetFlickerMode(
    MINT32 *a_pAEFlickerMode,
    MUINT32 *a_pOutLen
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AE_GET_BAND]\n");

    m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_MODE, a_pAEFlickerMode);

    *a_pOutLen = sizeof(MINT32);

    AAA_CCTOP_LOG("[AE Flicker Mode] = %d\n", *a_pAEFlickerMode);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEGetMeteringMode(
    MINT32 *a_pAEMEteringMode,
    MUINT32 *a_pOutLen
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AE_GET_METERING_RESULT]\n");

    m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_METERING_MODE, a_pAEMEteringMode);

    *a_pOutLen = sizeof(MINT32);

    AAA_CCTOP_LOG("[AE Metering Mode] = %d\n", *a_pAEMEteringMode);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEApplyNVRAMParam(
    MVOID *a_pAENVRAM
)
{
    AE_NVRAM_T *pAENVRAM = (AE_NVRAM_T *)a_pAENVRAM;

    AAA_CCTOP_LOG("[ACDK_CCT_OP_DEV_AE_APPLY_INFO]\n");

    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->applyAEParam(*pAENVRAM);

    m_r3ANVRAMData.rAENVRAM = *pAENVRAM;

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEGetNVRAMParam(
    MVOID *a_pAENVRAM,
    MUINT32 *a_pOutLen
)
{
    MINT32 err = MHAL_NO_ERROR;
    AE_NVRAM_T *pAENVRAM = (AE_NVRAM_T *)a_pAENVRAM;

    AAA_CCTOP_LOG("[ACDK_CCT_OP_DEV_AE_GET_INFO]\n");

    MHAL_ASSERT(m_pNvramDrvObj != NULL, "m_pNvramDrvObj is NULL");

    err = m_pNvramDrvObj->readNvram(m_eSensorType, m_u4SensorID, CAMERA_NVRAM_DATA_3A, (void *)&m_r3ANVRAMData, sizeof(NVRAM_CAMERA_3A_STRUCT));
    if (err < 0) {
        AAA_CCTOP_ERR("writeNvram() error: CAMERA_NVRAM_DATA_3A");
        return err;
    }

    *pAENVRAM = m_r3ANVRAMData.rAENVRAM;

    *a_pOutLen = sizeof(AE_NVRAM_T);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAESaveNVRAMParam(
)
{
    MINT32 err = MHAL_NO_ERROR;

    AAA_CCTOP_LOG("[ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM]\n");

    MHAL_ASSERT(m_pNvramDrvObj != NULL, "m_pNvramDrvObj is NULL");

    err = m_pNvramDrvObj->writeNvram(m_eSensorType, m_u4SensorID, CAMERA_NVRAM_DATA_3A, (void *)&m_r3ANVRAMData, sizeof(NVRAM_CAMERA_3A_STRUCT));
    if (err < 0) {
        AAA_CCTOP_ERR("writeNvram() error: CAMERA_NVRAM_DATA_3A");
        return err;
    }

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEGetCurrentEV(
    MINT32 *a_pAECurrentEV,
    MUINT32 *a_pOutLen
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION]\n");

    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    *a_pAECurrentEV = m_pmtk3A->getSceneBV();

    *a_pOutLen = sizeof(MINT32);

    AAA_CCTOP_LOG("[AE Current EV] = %d\n", *a_pAECurrentEV);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAELockExpSetting(
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING]\n");

    m_bLockExposureSetting = TRUE;

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEUnLockExpSetting(
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_AE_UNLOCK_EXPOSURE_SETTING]\n");

    m_bLockExposureSetting = FALSE;

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEGetIspOB(
    MUINT32 *a_pIspOB,
    MUINT32 *a_pOutLen
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_AE_GET_ISP_OB]\n");

    *a_pIspOB = ISP_BITS(m_pIspReg, CAM_RGBOFF, OFF00);

    *a_pOutLen = sizeof(MUINT32);

    AAA_CCTOP_LOG("[OB] = %d\n", *a_pIspOB);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAESetIspOB(
    MUINT32 a_IspOB
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_AE_SET_ISP_OB]\n");

    NSNvram::BufIF<NVRAM_CAMERA_ISP_PARAM_STRUCT>*const pBufIF = m_pNvramDrvObj->getBufIF< NVRAM_CAMERA_ISP_PARAM_STRUCT>();

    NVRAM_CAMERA_ISP_PARAM_STRUCT* pIspParam = pBufIF->getRefBuf(DUAL_CAMERA_MAIN_SENSOR, m_u4SensorID);

    ISP_NVRAM_OB_T* pOB = &pIspParam->ISPRegs.OB[0];

    pOB->rgboff.bits.OFF11 = a_IspOB;
    pOB->rgboff.bits.S11 = 1; // negative
    pOB->rgboff.bits.OFF10 = a_IspOB;
    pOB->rgboff.bits.S10 = 1; // negative
	pOB->rgboff.bits.OFF01 = a_IspOB;
    pOB->rgboff.bits.S01 = 1; // negative
    pOB->rgboff.bits.OFF00 = a_IspOB;
    pOB->rgboff.bits.S00 = 1; // negative

    AAA_CCTOP_LOG("[OB] = %d\n", a_IspOB);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAEGetIspRAWGain(
    MUINT32 *a_pIspRawGain,
    MUINT32 *a_pOutLen
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_AE_GET_ISP_RAW_GAIN]\n");

    *a_pIspRawGain = ISP_BITS(m_pIspReg, CAM_RAWGAIN0, RAW_RGAIN);

    *a_pOutLen = sizeof(MUINT32);

    AAA_CCTOP_LOG("[RAW Gain] = %d\n", *a_pIspRawGain);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAESetIspRAWGain(
    MUINT32 a_IspRAWGain
)
{
    MINT32 err;

    struct {
        cam_rawgain0_t rCtrl1; // 0x016C
        cam_rawgain1_t rCtrl2; // 0x0170
    } rIspRAWGain;

    AAA_CCTOP_LOG("[ACDK_CCT_OP_AE_SET_ISP_RAW_GAIN]\n");
    AAA_CCTOP_LOG("ISP RAW Gain = %d\n", a_IspRAWGain);

    memset(&rIspRAWGain, 0, sizeof(rIspRAWGain));

    rIspRAWGain.rCtrl1.u.bits.RAW_RGAIN = a_IspRAWGain;
    rIspRAWGain.rCtrl1.u.bits.RAW_GRGAIN = a_IspRAWGain;
    rIspRAWGain.rCtrl2.u.bits.RAW_GBGAIN = a_IspRAWGain;
    rIspRAWGain.rCtrl2.u.bits.RAW_BGAIN = a_IspRAWGain;

    err = writeIspRegs(0x016C, 2, (MUINT32 *)&rIspRAWGain);
    if (err < 0) {
        AAA_CCTOP_ERR("writeIspRegs() error");
        return err;
    }

    AAA_CCTOP_LOG("[RAW Gain] = %d\n", a_IspRAWGain);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAESetSensorExpTime(
    MUINT32 a_ExpTime
)
{
    MINT32 err;

    AAA_CCTOP_LOG("[ACDK_CCT_OP_AE_SET_SENSOR_EXP_TIME]\n");
    AAA_CCTOP_LOG("Exposure Time = %d\n", a_ExpTime);

    // Set exposure time
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_EXP_TIME, &a_ExpTime, NULL, NULL);
    MHAL_ASSERT(err == MHAL_NO_ERROR, "Err HAL_SENSOR_PARAM_SET_EXP_TIME");

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAESetSensorExpLine(
    MUINT32 a_ExpLine
) const
{
    MINT32 err;

    AAA_CCTOP_LOG("[ACDK_CCT_OP_AE_SET_SENSOR_EXP_LINE]\n");
    AAA_CCTOP_LOG("Exposure Line = %d\n", a_ExpLine);

    // Set exposure line
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_EXP_LINE, &a_ExpLine, NULL, NULL);
    MHAL_ASSERT(err == MHAL_NO_ERROR, "Err HAL_SENSOR_PARAM_SET_EXP_LINE");

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAESetSensorGain(
    MUINT32 a_SensorGain
) const
{
    MINT32 err;

    AAA_CCTOP_LOG("[ACDK_CCT_OP_AE_SET_SENSOR_GAIN]\n");
    AAA_CCTOP_LOG("Sensor Gain = %d\n", a_SensorGain);

    // Set sensor gain
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_GAIN, &a_SensorGain, NULL, NULL);
    MHAL_ASSERT(err == MHAL_NO_ERROR, "Err HAL_SENSOR_PARAM_SET_GAIN");

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAESetCaptureMode(
    MUINT32 a_CaptureMode
)
{
    MINT32 err;

    AAA_CCTOP_LOG("[ACDK_CCT_OP_AE_CAPTURE_MODE]\n");
    AAA_CCTOP_LOG("Capture mode = %d\n", a_CaptureMode);

    if(a_CaptureMode == 0) {      // preview mode
        m_r3AOutput.rAEOutput.rCaptureMode[0].u4Eposuretime = m_r3AOutput.rAEOutput.rPreviewMode.u4Eposuretime;
        m_r3AOutput.rAEOutput.rCaptureMode[0].u4AfeGain = m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain;
        m_r3AOutput.rAEOutput.rCaptureMode[0].u4IspGain = m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain;
        m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_B = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlare_B;
        m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_G = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlare_G;
        m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_R = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlare_R;
        m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlareBGain = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlareBGain;
        m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlareGGain = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlareGGain;
        m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlareRGain = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlareRGain;
        m_r3AOutput.rAEOutput.rCaptureMode[1].u4Eposuretime = m_r3AOutput.rAEOutput.rPreviewMode.u4Eposuretime;
        m_r3AOutput.rAEOutput.rCaptureMode[1].u4AfeGain = m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain;
        m_r3AOutput.rAEOutput.rCaptureMode[1].u4IspGain = m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain;
        m_r3AOutput.rAEOutput.rCaptureMode[1].FlareCFG.uFlare_B = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlare_B;
        m_r3AOutput.rAEOutput.rCaptureMode[1].FlareCFG.uFlare_G = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlare_G;
        m_r3AOutput.rAEOutput.rCaptureMode[1].FlareCFG.uFlare_R = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlare_R;
        m_r3AOutput.rAEOutput.rCaptureMode[1].FlareCFG.uFlareBGain = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlareBGain;
        m_r3AOutput.rAEOutput.rCaptureMode[1].FlareCFG.uFlareGGain = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlareGGain;
        m_r3AOutput.rAEOutput.rCaptureMode[1].FlareCFG.uFlareRGain = m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG.uFlareRGain;
   } else if(a_CaptureMode == 3) {   // capture mode
        m_r3AOutput.rAEOutput.rCaptureMode[0].u4Eposuretime = m_r3AOutput.rAEOutput.rCaptureMode[2].u4Eposuretime;
        m_r3AOutput.rAEOutput.rCaptureMode[0].u4AfeGain = m_r3AOutput.rAEOutput.rCaptureMode[2].u4AfeGain;
        m_r3AOutput.rAEOutput.rCaptureMode[0].u4IspGain = m_r3AOutput.rAEOutput.rCaptureMode[2].u4IspGain;
        m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_B = m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlare_B;
        m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_G = m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlare_G;
        m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_R = m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlare_R;
        m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlareBGain = m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlareBGain;
        m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlareGGain = m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlareGGain;
        m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlareRGain = m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlareRGain;
        m_r3AOutput.rAEOutput.rCaptureMode[1].u4Eposuretime = m_r3AOutput.rAEOutput.rCaptureMode[2].u4Eposuretime;
        m_r3AOutput.rAEOutput.rCaptureMode[1].u4AfeGain = m_r3AOutput.rAEOutput.rCaptureMode[2].u4AfeGain;
        m_r3AOutput.rAEOutput.rCaptureMode[1].u4IspGain = m_r3AOutput.rAEOutput.rCaptureMode[2].u4IspGain;
        m_r3AOutput.rAEOutput.rCaptureMode[1].FlareCFG.uFlare_B = m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlare_B;
        m_r3AOutput.rAEOutput.rCaptureMode[1].FlareCFG.uFlare_G = m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlare_G;
        m_r3AOutput.rAEOutput.rCaptureMode[1].FlareCFG.uFlare_R = m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlare_R;
        m_r3AOutput.rAEOutput.rCaptureMode[1].FlareCFG.uFlareBGain = m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlareBGain;
        m_r3AOutput.rAEOutput.rCaptureMode[1].FlareCFG.uFlareGGain = m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlareGGain;
        m_r3AOutput.rAEOutput.rCaptureMode[1].FlareCFG.uFlareRGain = m_r3AOutput.rAEOutput.rCaptureMode[2].FlareCFG.uFlareRGain;
        m_u4PreviewFlareOffset = g_u4CaptureFlareOffset;
    }
    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAWBEnable(
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN]\n");

    enableAWB();

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAWBDisable(
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN]\n");

    disableAWB();

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAWBGetEnableInfo(
    MINT32 *a_pEnableAWB,
    MUINT32 *a_pOutLen
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO]\n");

    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    *a_pEnableAWB = (MINT32)m_pmtk3A->isAWBEnable();

    *a_pOutLen = sizeof(MINT32);

    AAA_CCTOP_LOG("AWB Enable = %d\n", *a_pEnableAWB);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAWBGetAWBGain(
    MVOID *a_pAWBGain,
    MUINT32 *a_pOutLen
)
{
    AWB_GAIN_T *pAWBGain = (AWB_GAIN_T *)a_pAWBGain;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AWB_GET_GAIN]\n");

    *pAWBGain = m_r3AOutput.rAWBOutput.rPreviewAWBGain;

    *a_pOutLen = sizeof(AWB_GAIN_T);

    AAA_CCTOP_LOG("[RGain] = %d\n", m_r3AOutput.rAWBOutput.rPreviewAWBGain.u4R);
    AAA_CCTOP_LOG("[GGain] = %d\n", m_r3AOutput.rAWBOutput.rPreviewAWBGain.u4G);
    AAA_CCTOP_LOG("[BGain] = %d\n", m_r3AOutput.rAWBOutput.rPreviewAWBGain.u4B);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAWBSetAWBGain(
    MVOID *a_pAWBGain
)
{
    AWB_GAIN_T *pAWBGain = (AWB_GAIN_T *)a_pAWBGain;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AWB_SET_GAIN]\n");

    m_r3AOutput.rAWBOutput.rPreviewAWBGain = *pAWBGain;
    m_r3AOutput.rAWBOutput.rCaptureAWBGain = *pAWBGain;

    setIspAWBGain(*pAWBGain, TRUE);

    AAA_CCTOP_LOG("[RGain] = %d\n", m_r3AOutput.rAWBOutput.rPreviewAWBGain.u4R);
    AAA_CCTOP_LOG("[GGain] = %d\n", m_r3AOutput.rAWBOutput.rPreviewAWBGain.u4G);
    AAA_CCTOP_LOG("[BGain] = %d\n", m_r3AOutput.rAWBOutput.rPreviewAWBGain.u4B);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAWBApplyNVRAMParam(
    MVOID *a_pAWBNVRAM
)
{
    AWB_NVRAM_T *pAWBNVRAM = (AWB_NVRAM_T *)a_pAWBNVRAM;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2]\n");

    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->applyAWBParam(*pAWBNVRAM,
                            m_r3AStatConfig.rAWBStatConfig);

    setIsp3AStatConfig(m_r3AStatConfig);

    m_r3ANVRAMData.rAWBNVRAM = *pAWBNVRAM;

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAWBGetNVRAMParam(
    MVOID *a_pAWBNVRAM,
    MUINT32 *a_pOutLen
)
{
    MINT32 err = MHAL_NO_ERROR;
    AWB_NVRAM_T *pAWBNVRAM = (AWB_NVRAM_T *)a_pAWBNVRAM;
    EepromDrvBase *m_pEepromDrvObj = EepromDrvBase::createInstance();
    GET_SENSOR_CALIBRATION_DATA_STRUCT GetSensorCalData;
    MUINT32 result = 0xff;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AWB_GET_AWB_PARA]\n");

    MHAL_ASSERT(m_pNvramDrvObj != NULL, "m_pNvramDrvObj is NULL");

    err = m_pNvramDrvObj->readNvram(m_eSensorType, m_u4SensorID, CAMERA_NVRAM_DATA_3A, (void *)&m_r3ANVRAMData, sizeof(NVRAM_CAMERA_3A_STRUCT));
    if (err < 0) {
        AAA_CCTOP_ERR("writeNvram() error: CAMERA_NVRAM_DATA_3A");
        return err;
    }

    result= m_pEepromDrvObj->GetEepromCalData(m_eSensorType, m_u4SensorID, CAMERA_EEPROM_DATA_PREGAIN, (void *)&GetSensorCalData);
    if (result&EEPROM_ERR_NO_PREGAIN)
    {
        AAA_CCTOP_ERR("No Pregain data\n 0x%x\n",result);
    }
    else
    {
        m_r3ANVRAMData.rAWBNVRAM.rCalData.rCalGain.u4R    = GetSensorCalData.rCalGain.u4R;
        m_r3ANVRAMData.rAWBNVRAM.rCalData.rCalGain.u4G    = GetSensorCalData.rCalGain.u4G;
        m_r3ANVRAMData.rAWBNVRAM.rCalData.rCalGain.u4B    = GetSensorCalData.rCalGain.u4B;
        AAA_CCTOP_LOG("rCalGain.u4R = %d\n",GetSensorCalData.rCalGain.u4R);
        AAA_CCTOP_LOG("rCalGain.u4G = %d\n",GetSensorCalData.rCalGain.u4G);
        AAA_CCTOP_LOG("rCalGain.u4B = %d\n",GetSensorCalData.rCalGain.u4B);
    }

    *pAWBNVRAM = m_r3ANVRAMData.rAWBNVRAM;

    *a_pOutLen = sizeof(AWB_NVRAM_T);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAWBSaveNVRAMParam(
 )
{
    MINT32 err = MHAL_NO_ERROR;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA]\n");

    MHAL_ASSERT(m_pNvramDrvObj != NULL, "m_pNvramDrvObj is NULL");

    err = m_pNvramDrvObj->writeNvram(m_eSensorType, m_u4SensorID, CAMERA_NVRAM_DATA_3A, (void *)&m_r3ANVRAMData, sizeof(NVRAM_CAMERA_3A_STRUCT));
    if (err < 0) {
        AAA_CCTOP_ERR("writeNvram() error: CAMERA_NVRAM_DATA_3A");
        return err;
    }

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAWBSetAWBMode(
    MINT32 a_AWBMode
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_AWB_SET_AWB_MODE]\n");

    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->send3ACmd(LIB3A_AWB_CMD_ID_SET_AWB_MODE, a_AWBMode);

    AAA_CCTOP_LOG("[AWB Mode] = %d\n", a_AWBMode);


    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAWBGetAWBMode(
    MINT32 *a_pAWBMode,
    MUINT32 *a_pOutLen
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_AWB_GET_AWB_MODE]\n");

    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->get3ACmd(LIB3A_AWB_CMD_ID_SET_AWB_MODE, a_pAWBMode);

    *a_pOutLen = sizeof(MINT32);

    AAA_CCTOP_LOG("[AWB Mode] = %d\n", *a_pAWBMode);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAWBGetLightProb(
    MVOID *a_pAWBLightProb,
    MUINT32 *a_pOutLen
)
{
    MINT32 err = MHAL_NO_ERROR;
    AWB_LIGHT_PROBABILITY_T *pAWBLightProb = (AWB_LIGHT_PROBABILITY_T *)a_pAWBLightProb;

    AAA_CCTOP_LOG("[ACDK_CCT_OP_AWB_GET_LIGHT_PROB]\n");

    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->getAWBLightProb(*pAWBLightProb);

    *a_pOutLen = sizeof(AWB_LIGHT_PROBABILITY_T);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAFOpeartion(
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AF_OPERATION]\n");

    m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_AFS);

    m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_METER, LIB3A_AF_METER_SPOT);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPMFOpeartion(
    MINT32 a_MFpos
)
{
    MINT32 i4TimeOutCnt = 0;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_MF_OPERATION]\n");

    m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_MF);

    setMFPos(a_MFpos);

    while (!isAFFinish()) {
        usleep(5000); // 5ms
        i4TimeOutCnt++;

        if (i4TimeOutCnt > 100) {
            break;
        }
    }

    AAA_CCTOP_LOG("[MF]pos:%d, value:%d\n", a_MFpos, getAFValue());

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAFGetAFInfo(
    MVOID *a_pAFInfo,
    MUINT32 *a_pOutLen
)
{
    ACDK_AF_INFO_T *pAFInfo = (ACDK_AF_INFO_T *)a_pAFInfo;
    FOCUS_INFO_T rFocusInfo;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_GET_AF_INFO]\n");

    m_pmtk3A->get3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, &pAFInfo->i4AFMode);

    m_pmtk3A->get3ACmd(LIB3A_AF_CMD_ID_SET_AF_METER, &pAFInfo->i4AFMeter);

    if (getFocusInfo(rFocusInfo) == MHAL_NO_ERROR) {
        pAFInfo->i4CurrPos = rFocusInfo.i4CurrentPos;
    }
    else {
        pAFInfo->i4CurrPos = 0;
    }

    *a_pOutLen = sizeof(ACDK_AF_INFO_T);

    AAA_CCTOP_LOG("[AF Mode] = %d\n", pAFInfo->i4AFMode);
    AAA_CCTOP_LOG("[AF Meter] = %d\n", pAFInfo->i4AFMeter);
    AAA_CCTOP_LOG("[AF Current Pos] = %d\n", pAFInfo->i4CurrPos);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAFGetBestPos(
    MINT32 *a_pAFBestPos,
    MUINT32 *a_pOutLen
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AF_GET_BEST_POS]\n");

    *a_pAFBestPos = getAFBestPos();

    *a_pOutLen = sizeof(MINT32);

    AAA_CCTOP_LOG("[AF Best Pos] = %d\n", *a_pAFBestPos);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAFCaliOperation(
    MVOID *a_pAFCaliData,
    MUINT32 *a_pOutLen
)
{
    ACDK_AF_CALI_DATA_T *pAFCaliData = (ACDK_AF_CALI_DATA_T *)a_pAFCaliData;
    AAA_DEBUG_INFO_T *p_r3ADebugInfo;
    MUINT32 aaaDebugSize;
    MINT32 i4TimeOutCnt = 0;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AF_CALI_OPERATION]\n");

    m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_AFS);

    usleep(500000);	// 500ms

    m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_CALIBRATION);

    while(!isAFFinish()) {
        usleep(10000); // 10ms
        i4TimeOutCnt++;

        if (i4TimeOutCnt > 2000) {
             break;
        }
    }

    getDebugInfo((void **) &p_r3ADebugInfo, &aaaDebugSize);

    pAFCaliData->i4Gap = (MINT32)p_r3ADebugInfo->rAFDebugInfo.Tag[2].u4FieldValue;

    for (MINT32 i = 0; i < 512; i++) {
        if (p_r3ADebugInfo->rAFDebugInfo.Tag[i+2].u4FieldValue != 0) {
            pAFCaliData->u4Vlu[i] = p_r3ADebugInfo->rAFDebugInfo.Tag[i+3].u4FieldValue;
            pAFCaliData->i4Num = i+1;
        }
        else {
            break;
        }
    }

    pAFCaliData->i4BestPos = getAFBestPos();

    AAA_CCTOP_LOG("[AFCaliData] Num = %d\n", pAFCaliData->i4Num);
    AAA_CCTOP_LOG("[AFCaliData] Gap = %d\n", pAFCaliData->i4Gap);
    AAA_CCTOP_LOG("[AFCaliData] Pos = %d\n", pAFCaliData->i4BestPos);

    m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_AFS);

    *a_pOutLen = sizeof(MINT32);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAFSetFocusRange(
    MVOID *a_pFocusRange
)
{
    FOCUS_RANGE_T *pFocusRange = (FOCUS_RANGE_T *)a_pFocusRange;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AF_SET_RANGE]\n");

    if (m_pMcuDrvObj) {
        m_pMcuDrvObj->setMCUInfPos(pFocusRange->i4InfPos);
        m_pMcuDrvObj->setMCUMacroPos(pFocusRange->i4MacroPos);
    }

    m_rAFNVRAMData.rFocusRange = *pFocusRange;

    AAA_CCTOP_LOG("[Inf Pos] = %d\n", pFocusRange->i4InfPos);
    AAA_CCTOP_LOG("[Marco Pos] = %d\n", pFocusRange->i4MacroPos);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAFGetFocusRange(
    MVOID *a_pFocusRange,
    MUINT32 *a_pOutLen
)
{
    FOCUS_RANGE_T *pFocusRange = (FOCUS_RANGE_T *)a_pFocusRange;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AF_GET_RANGE]\n");

    *pFocusRange = m_rAFNVRAMData.rFocusRange;

    *a_pOutLen = sizeof(FOCUS_RANGE_T);

    AAA_CCTOP_LOG("[Inf Pos] = %d\n", pFocusRange->i4InfPos);
    AAA_CCTOP_LOG("[Marco Pos] = %d\n", pFocusRange->i4MacroPos);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAFGetNVRAMParam(
    MVOID *a_pAFNVRAM,
    MUINT32 *a_pOutLen
)
{
    MINT32 err = MHAL_NO_ERROR;
    NVRAM_LENS_PARA_STRUCT *pAFNVRAM = (NVRAM_LENS_PARA_STRUCT *)a_pAFNVRAM;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AF_READ]\n");

    MHAL_ASSERT(m_pNvramDrvObj != NULL, "m_pNvramDrvObj is NULL");

    err = m_pNvramDrvObj->readNvram(m_eSensorType, m_u4SensorID, CAMERA_NVRAM_DATA_LENS, (void *)&m_rAFNVRAMData, sizeof(NVRAM_LENS_PARA_STRUCT));
    if (err < 0) {
        AAA_CCTOP_ERR("writeNvram() error: CAMERA_NVRAM_DATA_LENS");
        return err;
    }

    *pAFNVRAM = m_rAFNVRAMData;

    *a_pOutLen = sizeof(NVRAM_LENS_PARA_STRUCT);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAFApplyNVRAMParam(
    MVOID *a_pAFNVRAM
)
{
    NVRAM_LENS_PARA_STRUCT *pAFNVRAM = (NVRAM_LENS_PARA_STRUCT *)a_pAFNVRAM;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AF_APPLY]\n");

    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->applyAFParam(*pAFNVRAM);

    if (m_pMcuDrvObj) {
        m_pMcuDrvObj->setMCUInfPos(pAFNVRAM->rFocusRange.i4InfPos);
        m_pMcuDrvObj->setMCUMacroPos(pAFNVRAM->rFocusRange.i4MacroPos);
    }

    m_rAFNVRAMData = *pAFNVRAM;

    return MHAL_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAFSaveNVRAMParam(
)
{
    MINT32 err = MHAL_NO_ERROR;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM]\n");

    MHAL_ASSERT(m_pNvramDrvObj != NULL, "m_pNvramDrvObj is NULL");

    err = m_pNvramDrvObj->writeNvram(m_eSensorType, m_u4SensorID, CAMERA_NVRAM_DATA_LENS, (void *)&m_rAFNVRAMData, sizeof(NVRAM_LENS_PARA_STRUCT));
    if (err < 0) {
        AAA_CCTOP_ERR("writeNvram() error: CAMERA_NVRAM_DATA_LENS");
        return err;
    }

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAFGetFV(
    MVOID *a_pAFPosIn,
    MVOID *a_pAFValueOut,
    MUINT32 *a_pOutLen
)
{
    ACDK_AF_POS_T *pAFPos = (ACDK_AF_POS_T *) a_pAFPosIn;
    ACDK_AF_VLU_T *pAFValue = (ACDK_AF_VLU_T *) a_pAFValueOut;
    MINT32 i4TimeOutCnt = 0;

    AAA_CCTOP_LOG("[ACDK_CCT_V2_OP_AF_GET_FV]\n");

    pAFValue->i4Num = pAFPos->i4Num;

    m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_AFS);

    usleep(500000); // 500ms

    m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_MF);

    for (MINT32 i = 0; i < pAFValue->i4Num; i++) {
        i4TimeOutCnt = 0;

        setMFPos(pAFPos->i4Pos[i]);

        while (!isAFFinish()) {
            usleep(5000);       // 5ms
            i4TimeOutCnt++;

            if (i4TimeOutCnt > 100) {
                break;
            }
        }

        pAFValue->u4Vlu[i] = getAFValue();

        AAA_CCTOP_LOG("[FV]pos = %d, value = %d\n", pAFPos->i4Pos[i], pAFValue->u4Vlu[i]);
    }

    m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_AFS);

    *a_pOutLen = sizeof(ACDK_AF_VLU_T);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAFEnable(
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_AF_ENABLE]\n");

    enableAF();

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAFDisable(
)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_AF_DISABLE]\n");

    disableAF();

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::CCTOPAFGetEnableInfo(
    MVOID *a_pEnableAF,
    MUINT32 *a_pOutLen
)
{
    MINT32 *pEnableAF = (MINT32 *)a_pEnableAF;

    AAA_CCTOP_LOG("[ACDK_CCT_OP_AF_GET_ENABLE_INFO]\n");

    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    *pEnableAF = (MINT32)m_pmtk3A->isAFEnable();

    *a_pOutLen = sizeof(MINT32);

    AAA_CCTOP_LOG("[AF Enable] = %d\n", *pEnableAF);

    return MHAL_NO_ERROR;
}


/*******************************************************************************
* Author : Cotta
* Functionality :
********************************************************************************/

MINT32 Hal3A::CCTOPFlashEnable()
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_FLASH_ENABLE]\n");

    m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_STROBE_MODE, LIB3A_AE_STROBE_MODE_AUTO);


    /*cotta-- added for strobe ratio CCT tuning

    if (m_pStrobeDrvObj)
    {
        m_pStrobeDrvObj->setState(1);       // state  = FLASHLIGHTDRV_STATE_STILL
        m_pStrobeDrvObj->setLevel(level);   // set level
        m_pStrobeDrvObj->setFire(1);        // turn on strobe
    }
    else
    {
        AAA_CCTOP_ERR("[m_pStrobeDrvObj == NULL]\n");
        return MHAL_UNKNOWN_ERROR;
    }

    --cotta*/

    return MHAL_NO_ERROR;
}


/*******************************************************************************
* Author : Cotta
* Functionality : turn off strobe
********************************************************************************/

MINT32 Hal3A::CCTOPFlashDisable()
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_FLASH_DISABLE]\n");

    m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_STROBE_MODE, LIB3A_AE_STROBE_MODE_FORCE_OFF);


     /*
    if (m_pStrobeDrvObj)
    {
        m_pStrobeDrvObj->setFire(0);    // turn off strobe
    }
    else
    {
        AAA_CCTOP_ERR("[m_pStrobeDrvObj == NULL]\n");
        return MHAL_UNKNOWN_ERROR;
    }
    */

    return MHAL_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3A::CCTOPFlashGetEnableInfo(MINT32 *a_pEnableFlash,MUINT32 *a_pOutLen)
{
    MINT32 i4StrobeMode;

    AAA_CCTOP_LOG("[ACDK_CCT_OP_FLASH_GET_INFO]\n");

    m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_STROBE_MODE, &i4StrobeMode);

    if (i4StrobeMode == LIB3A_AE_STROBE_MODE_AUTO)
    {
        *a_pEnableFlash = TRUE; // enable
    }
    else if (i4StrobeMode == LIB3A_AE_STROBE_MODE_FORCE_OFF)
    {
        *a_pEnableFlash = FALSE; // disable
    }
    else
    {
        AAA_CCTOP_ERR("[Unsupported strobe mode]\n");
        return MHAL_UNKNOWN_ERROR;
    }

    *a_pOutLen = sizeof(MINT32);

    AAA_CCTOP_LOG("[Flash Enable] = %d\n", *a_pEnableFlash);

    return MHAL_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3A::CCTOPFlashControl(MVOID *a_pflashCtrl)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_FLASH_CONTROL]\n");
	ACDK_FLASH_CONTROL *pflashCtrl = (ACDK_FLASH_CONTROL *)a_pflashCtrl;

    if (m_pStrobeDrvObj)
    {
        //m_pStrobeDrvObj->setLevel(pflashCtrl->level);

        AAA_CCTOP_LOG("CCTOPFlashControl fire 300 sleep 300\n");
        m_pStrobeDrvObj->setLevel(12);
        m_pStrobeDrvObj->setFire(1);
        usleep(300000);
        m_pStrobeDrvObj->setFire(0);
        usleep(300000);
    }
    else {
        AAA_CCTOP_ERR("[m_pStrobeDrvObj == NULL]\n");
        return MHAL_UNKNOWN_ERROR;
    }

    return MHAL_NO_ERROR;
}


/*******************************************************************************
* Author : Cotta
* Functionality : strobe high current - low current ratio tuning
********************************************************************************/
MINT32 Hal3A::CCTOPStrobeRatioTuning(MUINT32 level, MUINT32 *dataArray)
{
    AAA_CCTOP_LOG("[ACDK_CCT_OP_STROBE_RATIO_TUNING]\n");

    AAA_STAT_T low_current_3AStat;   // 3A statistic of low current
    AAA_STAT_T high_current_3AStat;  // 3A statistic of high current

    MUINT32 arrayLen = 25;
    MUINT32 lowCurrentAvg = 0;  // to calculate Y value of low current
    MUINT32 highCurrentAvg = 0; // to calculate Y value of high current

    ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;

    WaitIrq.Mode = ISP_DRV_IRQ_CLEAR_WAIT|ISP_DRV_IRQ_VSYNC;
    WaitIrq.Timeout = 1000;

    if (m_pStrobeDrvObj)
    {
        m_pStrobeDrvObj->setState(1);   // state  = FLASHLIGHTDRV_STATE_STILL

        // ====== low current ======

        m_pStrobeDrvObj->setLevel(level);  // level of low current

        m_pIspDrvObj->waitIrq(&WaitIrq);    // wait VD to start

        m_pStrobeDrvObj->setFire(1);    // turn on strobe

        m_pIspDrvObj->waitIrq(&WaitIrq);    // wait one
        m_pIspDrvObj->waitIrq(&WaitIrq);    // wait two
        m_pIspDrvObj->waitIrq(&WaitIrq);    // wait three
        m_pIspDrvObj->waitIrq(&WaitIrq);    // wait four

        m_pStrobeDrvObj->setFire(0);    //turn off strobe

        getIsp3AStat(low_current_3AStat);

        for(MUINT32 i = 0; i < arrayLen; ++i)
        {
            dataArray[i] = low_current_3AStat.rAEStat.u4AEWindowInfo[i];     // pass AE date
            //lowCurrentAvg += low_current_3AStat.rAEStat.u4AEWindowInfo[i];
        }

        //wait four frame
        m_pIspDrvObj->waitIrq(&WaitIrq);
        m_pIspDrvObj->waitIrq(&WaitIrq);
        m_pIspDrvObj->waitIrq(&WaitIrq);
        m_pIspDrvObj->waitIrq(&WaitIrq);

        // ====== high current ======

        m_pStrobeDrvObj->setLevel(255);  // level of high current

        m_pStrobeDrvObj->sendCommand(CMD_STROBE_SET_CAP_DELAY, 0, NULL, NULL); //set capture delay of sensor to the strobe kernel driver

        MBOOL Enable = MTRUE;
        m_pIspDrvObj->sendCommand(ISP_DRV_CMD_SET_VD_PROC,(MINT32)&Enable,0,0);  // force turn on VD interrupt signal

        m_pStrobeDrvObj->setFire(1);    // turn on strobe

        m_pIspDrvObj->waitIrq(&WaitIrq);    // wait one
        m_pIspDrvObj->waitIrq(&WaitIrq);    // wait two
        m_pIspDrvObj->waitIrq(&WaitIrq);    // wait three
        m_pIspDrvObj->waitIrq(&WaitIrq);    // wait four

        getIsp3AStat(high_current_3AStat);

        m_pStrobeDrvObj->setFire(0);    // turn off strobe

        for(MUINT32 i = 0; i < arrayLen; ++i)
        {
            dataArray[i + arrayLen] = high_current_3AStat.rAEStat.u4AEWindowInfo[i];     // pass AE date
            //highCurrentAvg += high_current_3AStat.rAEStat.u4AEWindowInfo[i];
        }

        // === calculate Y value ===

        for(MUINT32 i = 0; i < arrayLen; ++i)
        {
            dataArray[i]            = 4 * dataArray[i] / low_current_3AStat.rAEStat.u4AEBlockCnt;
            dataArray[i + arrayLen] = 4 * dataArray[i + arrayLen] / high_current_3AStat.rAEStat.u4AEBlockCnt;
        }
    }
    else
    {
        AAA_CCTOP_ERR("[m_pStrobeDrvObj == NULL]\n");
        return MHAL_UNKNOWN_ERROR;
    }

    return MHAL_NO_ERROR;
}

