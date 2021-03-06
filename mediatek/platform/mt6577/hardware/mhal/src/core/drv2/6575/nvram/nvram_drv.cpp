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
#include <utils/Errors.h>
#include <cutils/xlog.h>
#include <fcntl.h>
#include "nvram_drv.h"
#include "nvram_drv_imp.h"
#include "libnvram.h"
#include "CFG_file_lid.h"
#include "camera_custom_msdk.h"
#include "aaa_param.h"

/*******************************************************************************
*
********************************************************************************/

/*******************************************************************************
*
********************************************************************************/
#undef LOG_TAG
#define LOG_TAG "NvramDrv"

#define NVRAM_DRV_LOG(fmt, arg...)    XLOGD(LOG_TAG " "fmt, ##arg)
#define NVRAM_DRV_ERR(fmt, arg...)    XLOGE(LOG_TAG "Err: %5d: "fmt, __LINE__, ##arg)

#define INVALID_HANDLE_VALUE (-1)

/*******************************************************************************
*
********************************************************************************/

extern UINT32 cameraCustomInit();
extern UINT32 LensCustomInit();

/*******************************************************************************
*
********************************************************************************/
static unsigned long const g_u4NvramDataSize[CAMERA_DATA_TYPE_NUM] = 
{
    sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
    sizeof(NVRAM_CAMERA_3A_STRUCT),
    sizeof(NVRAM_CAMERA_SHADING_STRUCT),
    sizeof(NVRAM_CAMERA_DEFECT_STRUCT),
    0,
    sizeof(NVRAM_LENS_PARA_STRUCT),
    sizeof(AAA_PARAM_T),
    sizeof(AAA_STAT_CONFIG_PARAM_T)
};

/*******************************************************************************
*
********************************************************************************/
NvramDrvBase*
NvramDrvBase::createInstance()
{
    return NvramDrv::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
NvramDrvBase*
NvramDrv::getInstance()
{
    static NvramDrv singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
NvramDrv::destroyInstance()
{
}

/*******************************************************************************
*
********************************************************************************/
NvramDrv::NvramDrv()
    : NvramDrvBase()
{
}

/*******************************************************************************
*
********************************************************************************/
NvramDrv::~NvramDrv()
{
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::readNvram(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    unsigned long a_u4SensorID,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	void *a_pNvramData,
	unsigned long a_u4NvramDataSize
)
{
    int err = NVRAM_NO_ERROR;

    NVRAM_DRV_LOG("[readNvram] sensor type = %d; NVRAM data type = %d\n", a_eSensorType, a_eNvramDataType);

	if ((a_eSensorType > DUAL_CAMERA_SUB_SENSOR) ||
		(a_eSensorType < DUAL_CAMERA_MAIN_SENSOR) ||
		(a_eNvramDataType > CAMERA_DATA_3A_STAT_CONFIG_PARA) ||
		(a_eNvramDataType < CAMERA_NVRAM_DATA_ISP) ||
		(a_pNvramData == NULL) ||
		(a_u4NvramDataSize != g_u4NvramDataSize[a_eNvramDataType])) {
        return NVRAM_READ_PARAMETER_ERROR;
    }

    Mutex::Autolock lock(mLock);

    switch(a_eNvramDataType) {
    case CAMERA_NVRAM_DATA_ISP:
    case CAMERA_NVRAM_DATA_3A:
    case CAMERA_NVRAM_DATA_SHADING:
    case CAMERA_NVRAM_DATA_DEFECT:
	case CAMERA_NVRAM_DATA_LENS:
		err = readNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);
        if (err != NVRAM_NO_ERROR) {
		    NVRAM_DRV_ERR("readNvramData() error ==> readDefaultData()\n");
            err = readDefaultData(a_u4SensorID, a_eNvramDataType, a_pNvramData);
            if (err != NVRAM_NO_ERROR) {
		        NVRAM_DRV_ERR("readDefaultData() error\n");
	        }
            break;
	    }

		if (checkDataVersion(a_eNvramDataType, a_pNvramData) != NVRAM_NO_ERROR) {
			err = readDefaultData(a_u4SensorID, a_eNvramDataType, a_pNvramData);
			if (err != NVRAM_NO_ERROR) {
		        NVRAM_DRV_ERR("readDefaultData() error\n");
	        }
		}
 
        break;
    case CAMERA_DATA_3A_PARA:
    case CAMERA_DATA_3A_STAT_CONFIG_PARA:
        err = readDefaultData(a_u4SensorID, a_eNvramDataType, a_pNvramData);
	    if (err != NVRAM_NO_ERROR) {
		    NVRAM_DRV_ERR("readDefaultData() error\n");
	    }

        break;
    case CAMERA_NVRAM_DATA_SENSOR:
    default:
        break;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::writeNvram(
    CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    unsigned long a_u4SensorID,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	void *a_pNvramData,
	unsigned long a_u4NvramDataSize
)
{
    int err = NVRAM_NO_ERROR;

    NVRAM_DRV_LOG("[writeNvram] sensor type = %d; NVRAM data type = %d\n", a_eSensorType, a_eNvramDataType);

	if ((a_eSensorType > DUAL_CAMERA_SUB_SENSOR) ||
		(a_eSensorType < DUAL_CAMERA_MAIN_SENSOR) ||
		(a_eNvramDataType > CAMERA_DATA_3A_STAT_CONFIG_PARA) ||
		(a_eNvramDataType < CAMERA_NVRAM_DATA_ISP) ||
		(a_pNvramData == NULL) ||
		(a_u4NvramDataSize != g_u4NvramDataSize[a_eNvramDataType])) {
        return NVRAM_WRITE_PARAMETER_ERROR;
    }

    Mutex::Autolock lock(mLock);

    err = writeNvramData(a_eSensorType, a_eNvramDataType, a_pNvramData);
   
    return err;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::checkDataVersion(
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	void *a_pNvramData
)
{
    int err = NVRAM_NO_ERROR;

    NVRAM_DRV_LOG("[checkDataVersion]\n");

    if (a_eNvramDataType == CAMERA_NVRAM_DATA_ISP) { // ISP
        PNVRAM_CAMERA_ISP_PARAM_STRUCT pCameraNvramData = (PNVRAM_CAMERA_ISP_PARAM_STRUCT)a_pNvramData;

        NVRAM_DRV_LOG("[ISP] NVRAM data version = %d; F/W data version = %d\n", pCameraNvramData->Version, NVRAM_CAMERA_PARA_FILE_VERSION);

		if (pCameraNvramData->Version != NVRAM_CAMERA_PARA_FILE_VERSION) {
			err = NVRAM_DATA_VERSION_ERROR;
	    }
	}
	else if (a_eNvramDataType == CAMERA_NVRAM_DATA_3A) { // 3A
		PNVRAM_CAMERA_3A_STRUCT p3ANvramData = (PNVRAM_CAMERA_3A_STRUCT)a_pNvramData;

        NVRAM_DRV_LOG("[3A] NVRAM data version = %d; F/W data version = %d\n", p3ANvramData->u4Version, NVRAM_CAMERA_3A_FILE_VERSION);

		if (p3ANvramData->u4Version != NVRAM_CAMERA_3A_FILE_VERSION) {
			err = NVRAM_DATA_VERSION_ERROR;
	    }
	}
	else if (a_eNvramDataType == CAMERA_NVRAM_DATA_SHADING) { // Shading
		PNVRAM_CAMERA_SHADING_STRUCT pShadingNvramData = (PNVRAM_CAMERA_SHADING_STRUCT)a_pNvramData;

        NVRAM_DRV_LOG("[Shading] NVRAM data version = %d; F/W data version = %d\n", pShadingNvramData->Shading.Version, NVRAM_CAMERA_SHADING_FILE_VERSION);

		if (pShadingNvramData->Shading.Version != NVRAM_CAMERA_SHADING_FILE_VERSION) {
			err = NVRAM_DATA_VERSION_ERROR;
	    }
	}
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_DEFECT) { // Defect
		PNVRAM_CAMERA_DEFECT_STRUCT pDefectNvramData = (PNVRAM_CAMERA_DEFECT_STRUCT)a_pNvramData;

        NVRAM_DRV_LOG("[Defect] NVRAM data version = %d; F/W data version = %d\n", pDefectNvramData->Defect.Version, NVRAM_CAMERA_DEFECT_FILE_VERSION);

		if (pDefectNvramData->Defect.Version != NVRAM_CAMERA_DEFECT_FILE_VERSION) {
			err = NVRAM_DATA_VERSION_ERROR;
	    }
	}
    else if (a_eNvramDataType == CAMERA_NVRAM_DATA_LENS) { // Lens
		PNVRAM_LENS_PARA_STRUCT pLensNvramData = (PNVRAM_LENS_PARA_STRUCT)a_pNvramData;

        NVRAM_DRV_LOG("[Lens] NVRAM data version = %d; F/W data version = %d\n", pLensNvramData->Version, NVRAM_CAMERA_LENS_FILE_VERSION);

		if (pLensNvramData->Version != NVRAM_CAMERA_LENS_FILE_VERSION) {
			err = NVRAM_DATA_VERSION_ERROR;
		}
    }
    else {
		NVRAM_DRV_ERR("checkDataVersion(): incorrect data type\n");
}

    return err;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::readNvramData(
	CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	void *a_pNvramData
)
{
	F_ID rNvramFileID;
	int i4FileInfo;
	int i4RecSize;
    int i4RecNum;

    NVRAM_DRV_LOG("[readNvramData] sensor type = %d; NVRAM data type = %d\n", a_eSensorType, a_eNvramDataType);

	switch (a_eNvramDataType) {
	case CAMERA_NVRAM_DATA_ISP:
		i4FileInfo = AP_CFG_RDCL_CAMERA_PARA_LID;
		break;
	case CAMERA_NVRAM_DATA_3A:
		i4FileInfo = AP_CFG_RDCL_CAMERA_3A_LID;
		break;
	case CAMERA_NVRAM_DATA_SHADING:
		i4FileInfo = AP_CFG_RDCL_CAMERA_SHADING_LID;
		break;
	case CAMERA_NVRAM_DATA_DEFECT:
		i4FileInfo = AP_CFG_RDCL_CAMERA_DEFECT_LID;
		break;
	case CAMERA_NVRAM_DATA_LENS:
		i4FileInfo = AP_CFG_RDCL_CAMERA_LENS_LID;
		break;
	default :
	    NVRAM_DRV_ERR("readNvramData(): incorrect data type\n");
		return NVRAM_READ_PARAMETER_ERROR;
		break;
	}

	rNvramFileID = NVM_GetFileDesc(i4FileInfo, &i4RecSize, &i4RecNum, ISREAD);
	if (rNvramFileID.iFileDesc == INVALID_HANDLE_VALUE) {
		NVRAM_DRV_ERR("readNvramData(): create NVRAM file fail\n");
		return NVRAM_CAMERA_FILE_ERROR;
	}

    if (a_eSensorType == DUAL_CAMERA_SUB_SENSOR) {
	    lseek(rNvramFileID.iFileDesc, i4RecSize, SEEK_SET);
	}

	read(rNvramFileID.iFileDesc, a_pNvramData, i4RecSize);

	NVM_CloseFileDesc(rNvramFileID);

    return NVRAM_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::writeNvramData(
	CAMERA_DUAL_CAMERA_SENSOR_ENUM a_eSensorType,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	void *a_pNvramData
)
{
	F_ID rNvramFileID;
	int i4FileInfo;
	int i4RecSize;
    int i4RecNum;

    NVRAM_DRV_LOG("[writeNvramData] sensor type = %d; NVRAM data type = %d\n", a_eSensorType, a_eNvramDataType);

	switch (a_eNvramDataType) {
	case CAMERA_NVRAM_DATA_ISP:
		i4FileInfo = AP_CFG_RDCL_CAMERA_PARA_LID;
		break;
	case CAMERA_NVRAM_DATA_3A:
		i4FileInfo = AP_CFG_RDCL_CAMERA_3A_LID;
		break;
	case CAMERA_NVRAM_DATA_SHADING:
		i4FileInfo = AP_CFG_RDCL_CAMERA_SHADING_LID;
		break;
	case CAMERA_NVRAM_DATA_DEFECT:
		i4FileInfo = AP_CFG_RDCL_CAMERA_DEFECT_LID;
		break;
	case CAMERA_NVRAM_DATA_LENS:
		i4FileInfo = AP_CFG_RDCL_CAMERA_LENS_LID;
		break;
	default:
	    NVRAM_DRV_ERR("writeNvramData(): incorrect data type\n");
		return NVRAM_WRITE_PARAMETER_ERROR;
		break;
	}

    rNvramFileID = NVM_GetFileDesc(i4FileInfo, &i4RecSize, &i4RecNum, ISWRITE);
	if (rNvramFileID.iFileDesc == INVALID_HANDLE_VALUE) {
	    NVRAM_DRV_ERR("writeNvramData(): create NVRAM file fail\n");
		return NVRAM_CAMERA_FILE_ERROR;
	}

	if (a_eSensorType == DUAL_CAMERA_SUB_SENSOR) {
	    lseek(rNvramFileID.iFileDesc, i4RecSize, SEEK_SET);
	}

    write(rNvramFileID.iFileDesc, a_pNvramData, i4RecSize);

	NVM_CloseFileDesc(rNvramFileID);

    return NVRAM_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
int
NvramDrv::readDefaultData(
	unsigned long a_u4SensorID,
    CAMERA_DATA_TYPE_ENUM a_eNvramDataType,
	void *a_pNvramData
)
{
    static bool bCustomInit = 0;
    NVRAM_DRV_LOG("[readDefaultData] sensor ID = %ld; NVRAM data type = %d\n", a_u4SensorID, a_eNvramDataType);

    if (!bCustomInit) {
        cameraCustomInit();
	    LensCustomInit();
		bCustomInit = 1;
	}

	switch (a_eNvramDataType) {
	case CAMERA_NVRAM_DATA_ISP:
		GetCameraDefaultPara(a_u4SensorID, (PNVRAM_CAMERA_ISP_PARAM_STRUCT)a_pNvramData,NULL,NULL,NULL,NULL,NULL);
		break;
	case CAMERA_NVRAM_DATA_3A:
		GetCameraDefaultPara(a_u4SensorID, NULL,(PNVRAM_CAMERA_3A_STRUCT)a_pNvramData,NULL,NULL,NULL,NULL);
		break;
	case CAMERA_NVRAM_DATA_SHADING:
		GetCameraDefaultPara(a_u4SensorID, NULL,NULL,(PNVRAM_CAMERA_SHADING_STRUCT)a_pNvramData,NULL,NULL,NULL);
		break;
	case CAMERA_NVRAM_DATA_DEFECT:
		GetCameraDefaultPara(a_u4SensorID, NULL,NULL,NULL,(PNVRAM_CAMERA_DEFECT_STRUCT)a_pNvramData,NULL,NULL);
		break;
	case CAMERA_NVRAM_DATA_SENSOR:
		break;
	case CAMERA_NVRAM_DATA_LENS:
		GetLensDefaultPara((PNVRAM_LENS_PARA_STRUCT)a_pNvramData);
		{
			PNVRAM_LENS_PARA_STRUCT pLensNvramData = (PNVRAM_LENS_PARA_STRUCT)a_pNvramData;
			pLensNvramData->Version = NVRAM_CAMERA_LENS_FILE_VERSION;
		}
		break;
	case CAMERA_DATA_3A_PARA:
		GetCameraDefaultPara(a_u4SensorID, NULL,NULL,NULL,NULL,(P3A_PARAM_T)a_pNvramData,NULL);
		break;
	case CAMERA_DATA_3A_STAT_CONFIG_PARA:
		GetCameraDefaultPara(a_u4SensorID, NULL,NULL,NULL,NULL,NULL,(P3A_STAT_CONFIG_PARAM_T)a_pNvramData);
		break;
	default:
		break;
	}

    return NVRAM_NO_ERROR;
}

