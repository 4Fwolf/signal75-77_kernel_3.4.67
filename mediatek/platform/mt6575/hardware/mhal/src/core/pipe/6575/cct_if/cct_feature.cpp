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
#define LOG_TAG "cct_feature"
//
#include <utils/Errors.h>
#include <cutils/xlog.h>
//
#include "pipe_types.h"
//
#include "cct_feature.h"

#include "sensor_drv.h"
#include "aaa_hal_base.h"
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
MINT32 CctImp::sensorCCTFeatureControl(MUINT32 a_u4Ioctl,
                                       MUINT8 *puParaIn,
                                       MUINT32 u4ParaInLen,
                                       MUINT8 *puParaOut,
                                       MUINT32 u4ParaOutLen,
                                       MUINT32 *pu4RealParaOutLen
)
{
    MINT32 err = CCTIF_NO_ERROR;
    MUINT32 *pu32In = (MUINT32*)puParaIn;
    MUINT32 *pu32Out = (MUINT32 *)puParaOut;

    switch (a_u4Ioctl) {
    case ACDK_CCT_OP_READ_SENSOR_REG:
        err = CCTOReadSensorReg(puParaIn, puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_WRITE_SENSOR_REG:
        err = CCTOPWriteSensorReg(puParaIn);
        break;
    case ACDK_CCT_OP_QUERY_SENSOR:
        err = CCTOPQuerySensor(puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION:
        err = CCTOPGetSensorRes(puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION:
        err = CCTOPGetLSCSensorRes(puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT:
        err = CCTOPGetEngSensorGroupCount(pu32Out, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA:
        err = CCTOPGetEngSensorGroupPara(*pu32In, puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_GET_ENG_SENSOR_PARA:
        err = CCTOPGetEngSensorPara(puParaIn, puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_SET_ENG_SENSOR_PARA:
        err = CCTOPSetEngSensorPara(puParaIn );
        break;
    case ACDK_CCT_OP_GET_SENSOR_PREGAIN:
        err = CCTOPGetSensorPregain(puParaIn, puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_SET_SENSOR_PREGAIN:
        err = CCTOPSetSensorPregain(puParaIn);
        break;
    case ACDK_CCT_OP_GET_SENSOR_INFO:
        err = CCTOPGetSensorInfo(puParaIn, puParaOut, pu4RealParaOutLen);
        break;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::aaaCCTFeatureControl(MUINT32 a_u4Ioctl,
                                    MUINT8 *puParaIn ,
                                    MUINT32 u4ParaInLen,
                                    MUINT8 *puParaOut,
                                    MUINT32 u4ParaOutLen,
                                    MUINT32 *pu4RealParaOutLen
)
{
    MINT32 err = CCTIF_NO_ERROR;
    if (m_pHal3AObj == NULL) {
        IMAGE_SENSOR_TYPE type = m_pSensorDrvObj->getCurrentSensorType();
        if (type == IMAGE_SENSOR_TYPE_RAW) {
            //FIXME, device should depended on the sensor,
            m_pHal3AObj = Hal3ABase::createInstance(HAL3A_SENSOR_TYPE_RAW, SENSOR_MAIN);
            if (m_pHal3AObj == NULL)    {
                MY_LOG("[aaaCCTFeatureControl]: NULL 3A Obj \n");
                return CCTIF_INVALID_DRIVER;
            }
        }
        else if(type == IMAGE_SENSOR_TYPE_YUV ||type == IMAGE_SENSOR_TYPE_YCBCR){
            MY_LOG("[aaaCCTFeatureControl]: aaa yuv  Obj \n");
            m_pHal3AObj = Hal3ABase::createInstance(HAL3A_SENSOR_TYPE_YUV, SENSOR_MAIN);
            //m_pHal3AObj = Hal3ABase::createInstance(HAL3A_SENSOR_TYPE_YUV, SENSOR_SUB);
            MY_LOG("[aaaCCTFeatureControl]: m_pHal3AObj=%p \n", m_pHal3AObj);            
            if (m_pHal3AObj == NULL)    {
                MY_LOG("[aaaCCTFeatureControl]: NULL 3A Obj \n");
                return CCTIF_INVALID_DRIVER;
            }
        }
        else {
            m_pHal3AObj = NULL;
            MY_LOG("[aaaCCTFeatureControl]: Unsupport sensor type\n");
            return CCTIF_UNSUPPORT_SENSOR_TYPE;
        }
    }

    MINT32 *i32In = (MINT32 *)puParaIn;
    MUINT32 *u32In = (MUINT32 *)puParaIn;

    switch (a_u4Ioctl)
    {
    // AE
    case ACDK_CCT_OP_AE_ENABLE:
        err = m_pHal3AObj->CCTOPAEEnable();
        break;
    case ACDK_CCT_OP_AE_DISABLE:
        err = m_pHal3AObj->CCTOPAEDisable();
        break;
    case ACDK_CCT_OP_AE_GET_ENABLE_INFO:
        err = m_pHal3AObj->CCTOPAEGetEnableInfo((MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE:
        err = m_pHal3AObj->CCTOPAESetAEMode(*i32In);
        break;
    case ACDK_CCT_OP_DEV_AE_GET_INFO:
        err = m_pHal3AObj->CCTOPAEGetNVRAMParam((VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AE_GET_SCENE_MODE:
        err = m_pHal3AObj->CCTOPAEGetAEMode((MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AE_SET_METERING_MODE:
        err = m_pHal3AObj->CCTOPAESetMeteringMode(*i32In);
        break;
    case ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO:
        err = m_pHal3AObj->CCTOPAEApplyExpParam((VOID *)puParaIn);
        break;
    case ACDK_CCT_V2_OP_AE_SELECT_BAND:
        err = m_pHal3AObj->CCTOPAESetFlickerMode(*i32In);
        break;
    case ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA:
        err = m_pHal3AObj->CCTOPAEGetExpParam((VOID *)puParaIn, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AE_GET_BAND:
        err = m_pHal3AObj->CCTOPAEGetFlickerMode((MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AE_GET_METERING_RESULT:
        err = m_pHal3AObj->CCTOPAEGetMeteringMode((MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_DEV_AE_APPLY_INFO:
        err = m_pHal3AObj->CCTOPAEApplyNVRAMParam((VOID *)puParaIn);
        break;
    case ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM:
        err = m_pHal3AObj->CCTOPAESaveNVRAMParam();
        break;
    case ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION:
        err = m_pHal3AObj->CCTOPAEGetCurrentEV((MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING:
        err = m_pHal3AObj->CCTOPAELockExpSetting();
        break;
    case ACDK_CCT_OP_AE_UNLOCK_EXPOSURE_SETTING:
        err = m_pHal3AObj->CCTOPAEUnLockExpSetting();
        break;
    case ACDK_CCT_OP_AE_GET_ISP_OB:
        err = m_pHal3AObj->CCTOPAEGetIspOB((MUINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_AE_SET_ISP_OB:
        err = m_pHal3AObj->CCTOPAESetIspOB(*u32In);
        break;
    case ACDK_CCT_OP_AE_GET_ISP_RAW_GAIN:
        err = m_pHal3AObj->CCTOPAEGetIspRAWGain((MUINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_AE_SET_ISP_RAW_GAIN:
        err = m_pHal3AObj->CCTOPAESetIspRAWGain(*u32In);
        break;
    case ACDK_CCT_OP_AE_SET_SENSOR_EXP_TIME:
        err = m_pHal3AObj->CCTOPAESetSensorExpTime(*u32In);
        break;
    case ACDK_CCT_OP_AE_SET_SENSOR_EXP_LINE:
        err = m_pHal3AObj->CCTOPAESetSensorExpLine(*u32In);
        break;
    case ACDK_CCT_OP_AE_SET_SENSOR_GAIN:
        err = m_pHal3AObj->CCTOPAESetSensorGain(*u32In);
        break;
    case ACDK_CCT_OP_AE_CAPTURE_MODE:
        err = m_pHal3AObj->CCTOPAESetCaptureMode(*u32In);
    	 break;
    // AWB
    case ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN:
        err = m_pHal3AObj->CCTOPAWBEnable();
        break;
    case ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN:
        err = m_pHal3AObj->CCTOPAWBDisable();
        break;
    case ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO:
        err = m_pHal3AObj->CCTOPAWBGetEnableInfo((MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AWB_GET_GAIN:
        err = m_pHal3AObj->CCTOPAWBGetAWBGain((VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AWB_SET_GAIN:
        err = m_pHal3AObj->CCTOPAWBSetAWBGain((VOID *)puParaIn);
        break;
    case ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2:
        err = m_pHal3AObj->CCTOPAWBApplyNVRAMParam((VOID *)puParaIn);
        break;
    case ACDK_CCT_V2_OP_AWB_GET_AWB_PARA:
        err = m_pHal3AObj->CCTOPAWBGetNVRAMParam((VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA:
        err = m_pHal3AObj->CCTOPAWBSaveNVRAMParam();
        break;
    case ACDK_CCT_OP_AWB_SET_AWB_MODE:
        err = m_pHal3AObj->CCTOPAWBSetAWBMode(*i32In);
        break;
    case ACDK_CCT_OP_AWB_GET_AWB_MODE:
        err = m_pHal3AObj->CCTOPAWBGetAWBMode((MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_AWB_GET_LIGHT_PROB:
        err = m_pHal3AObj->CCTOPAWBGetLightProb((VOID *)puParaOut, pu4RealParaOutLen);
        break;

    // AF
    case ACDK_CCT_V2_OP_AF_OPERATION:
        err = m_pHal3AObj->CCTOPAFOpeartion();
        break;
    case ACDK_CCT_V2_OP_MF_OPERATION:
        err = m_pHal3AObj->CCTOPMFOpeartion(*i32In);
        break;
    case ACDK_CCT_V2_OP_GET_AF_INFO:
        err = m_pHal3AObj->CCTOPAFGetAFInfo((VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AF_GET_BEST_POS:
        err = m_pHal3AObj->CCTOPAFGetBestPos((MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AF_CALI_OPERATION:
        err = m_pHal3AObj->CCTOPAFCaliOperation((VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AF_SET_RANGE:
        err = m_pHal3AObj->CCTOPAFSetFocusRange((VOID *)puParaIn);
        break;
    case ACDK_CCT_V2_OP_AF_GET_RANGE:
        err = m_pHal3AObj->CCTOPAFGetFocusRange((VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM:
        err = m_pHal3AObj->CCTOPAFSaveNVRAMParam();
        break;
    case ACDK_CCT_V2_OP_AF_READ:
        err = m_pHal3AObj->CCTOPAFGetNVRAMParam((VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AF_APPLY:
        err = m_pHal3AObj->CCTOPAFApplyNVRAMParam((VOID *)puParaIn);
        break;
    case ACDK_CCT_V2_OP_AF_GET_FV:
        err = m_pHal3AObj->CCTOPAFGetFV((VOID *)puParaIn, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_AF_ENABLE:
        err = m_pHal3AObj->CCTOPAFEnable();
        break;
    case ACDK_CCT_OP_AF_DISABLE:
        err = m_pHal3AObj->CCTOPAFDisable();
        break;
    case ACDK_CCT_OP_AF_GET_ENABLE_INFO:
        err = m_pHal3AObj->CCTOPAFGetEnableInfo((VOID *)puParaOut, pu4RealParaOutLen);
        break;

    // Strobe
    case ACDK_CCT_OP_FLASH_ENABLE:
        err = m_pHal3AObj->CCTOPFlashEnable();
        break;
    case ACDK_CCT_OP_FLASH_DISABLE:
        err = m_pHal3AObj->CCTOPFlashDisable();
        break;
    case ACDK_CCT_OP_FLASH_GET_INFO:
        err = m_pHal3AObj->CCTOPFlashGetEnableInfo((MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_FLASH_CONTROL:
        err = m_pHal3AObj->CCTOPFlashControl((VOID *)puParaIn);
        break;
    case ACDK_CCT_OP_STROBE_RATIO_TUNING : //cotta-- added for strobe ratio tuning
        err = m_pHal3AObj->CCTOPStrobeRatioTuning(*u32In, (MUINT32 *)puParaOut);
        break;
    }
    return err;
}


/*******************************************************************************
* ISP
********************************************************************************/
MINT32 CctImp::ispCCTFeatureControl(MUINT32 a_u4Ioctl,
                                    MUINT8 *puParaIn,
                                    MUINT32 u4ParaInLen,
                                    MUINT8 *puParaOut,
                                    MUINT32 u4ParaOutLen,
                                    MUINT32 *pu4RealParaOutLen
)
{
    if  ( ! m_pCctCtrl )
        return  CCTIF_NOT_INIT;
    return  m_pCctCtrl->cctFeatureCtrl_isp(a_u4Ioctl, puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen);
}


MINT32
CctCtrl::
cctFeatureCtrl_isp(
    MUINT32 const a_u4Ioctl,
    MUINT8*const puParaIn,
    MUINT32 const u4ParaInLen,
    MUINT8*const puParaOut,
    MUINT32 const u4ParaOutLen,
    MUINT32*const pu4RealParaOutLen
)
{
#define DO_CCT_CTRL(ctl_cocde)  \
    case ctl_cocde: \
        err = doCctFeatureCtrl<ctl_cocde>(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); \
        break

    MINT32 err = CCTIF_NO_ERROR;

    switch (a_u4Ioctl)
    {
    // ISP
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_READ_REG );
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_WRITE_REG );
    DO_CCT_CTRL( ACDK_CCT_OP_QUERY_ISP_ID );

    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS );

    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF );

    // GAMMA
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG );

    // CCM
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_PARA );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM );
    DO_CCT_CTRL( ACDK_CCT_OP_SET_CCM_MODE );
    DO_CCT_CTRL( ACDK_CCT_OP_GET_CCM_MODE );

    // ISP Common Control
    DO_CCT_CTRL( ACDK_CCT_OP_SET_ISP_ON );
    DO_CCT_CTRL( ACDK_CCT_OP_SET_ISP_OFF );
    DO_CCT_CTRL( ACDK_CCT_OP_GET_ISP_ON_OFF );

    // NVRAM
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM );
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM );
    // Shading Table NVRAM
    DO_CCT_CTRL( ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM );
    DO_CCT_CTRL( ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM );

    //  PCA
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_TABLE );
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_TABLE );
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_PARA );
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_PARA );

    // Shading/Defect
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA);
    case ACDK_CCT_V2_ISP_DEFECT_TABLE_ON:
    case ACDK_CCT_V2_ISP_DEFECT_TABLE_OFF:
    case ACDK_CCT_OP_SET_CALI_MODE:
        break;

    default:
        err = CCTIF_BAD_CTRL_CODE;
        break;
    }

    return err;
}


/*******************************************************************************
* NVRAM
********************************************************************************/
MINT32 CctImp::nvramCCTFeatureControl(MUINT32 a_u4Ioctl,
                                      MUINT8 *puParaIn,
                                      MUINT32 u4ParaInLen,
                                      MUINT8 *puParaOut,
                                      MUINT32 u4ParaOutLen,
                                      MUINT32 *pu4RealParaOutLen
)
{
    MY_LOG("CctImp::nvramCCTFeatureControl \n");
    if  ( ! m_pCctCtrl )
        return  CCTIF_NOT_INIT;
    return  m_pCctCtrl->cctFeatureCtrl_nvram(a_u4Ioctl, puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen);
}


MINT32
CctCtrl::
cctFeatureCtrl_nvram(
    MUINT32 const a_u4Ioctl,
    MUINT8*const puParaIn,
    MUINT32 const u4ParaInLen,
    MUINT8*const puParaOut,
    MUINT32 const u4ParaOutLen,
    MUINT32*const pu4RealParaOutLen
)
{
    MINT32 err = CCTIF_NO_ERROR;
    switch (a_u4Ioctl)
    {
//    DO_CCT_CTRL( ACDK_CCT_OP_LOAD_FROM_NVRAM );
//    DO_CCT_CTRL( ACDK_CCT_OP_SAVE_TO_NVRAM );
    default:
        err = CCTIF_BAD_CTRL_CODE;
        break;
    }
    return  err;
}

