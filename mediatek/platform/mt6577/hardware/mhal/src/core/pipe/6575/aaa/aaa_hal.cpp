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
#define LOG_TAG "aaa_hal"

#include <stdlib.h>
#include <stdio.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>
#include <linux/rtpm_prio.h>
#include <pthread.h>
#include <semaphore.h>
#include "camera_feature.h"
#include "aaa_hal.h"
#include "MediaAssert.h"
#include "sensor_drv.h"
#include "isp_reg.h"
#include "nvram_drv.h"
#include "isp_drv.h"
#include "mcu_drv.h"
#include "strobe_drv.h"
//#include "AppFeature.h"
#include "MTKDetection.h"
//#include "AcdkFeature.h"
//#include "AcdkCCTFeature.h"
#include "camera_custom_flashlight.h"
#include "camera_custom_af.h"

//#include "kd_camera_feature.h"
//#include "camera_feature.h"
#include "MTK3A.h"
#include "eeprom_drv.h"
#include "flicker_hal_base.h"
#include "camera_custom_if.h"
#include "faces.h"
#include "math.h"

using namespace NSFeature;
using namespace android;

/*******************************************************************************
*
********************************************************************************/
static MINT32 AAA_HAL_DBG_OPTION = 0;

#define AAA_ERR(fmt, arg...) XLOGE("Err: %5d: "fmt, __LINE__, ##arg)
#define AAA_LOG(fmt, arg...) XLOGD(fmt, ##arg)

enum {
    AAA_HAL_DBG_OPTION_ALL_OFF = 0,
    AAA_HAL_DBG_OPTION_AAA_STAT_CONFIG = 1,
    AAA_HAL_DBG_OPTION_AE_STAT = 2,
    AAA_HAL_DBG_OPTION_AF_STAT = 3,
    AAA_HAL_DBG_OPTION_AWB_STAT = 4,
    AAA_HAL_DBG_OPTION_ISP = 5,
    AAA_HAL_DBG_OPTION_SENSOR = 6,
    AAA_HAL_DBG_OPTION_MCU = 7,
    AAA_HAL_DBG_OPTION_PARAM = 8,
    AAA_HAL_DBG_OPTION_ALL_ON = 9,
    AAA_HAL_DBG_OPTION_AF_LOG = 10,
    AAA_HAL_DBG_OPTION_AE_BRIGHTNESS = 11
};

/*******************************************************************************
*
********************************************************************************/
static MUINT32 g_u4CaptureCnt = 0;
static MBOOL   g_bBestShotEnable = FALSE;
static MUINT32 g_u4BestShotValue = 0;
static MUINT16 g_PreFrameRate = 0;
static MUINT16 g_SensorFrameRate = 0;
static MUINT32 g_BaseBandClk = 133000000;
static MBOOL g_bHDRUpdate = FALSE;
static Hal3A*  strobeHal3AObj = NULL;    //cotta--added for strobe protection
static MUINT32 sensorDelayValue = 0;    //cotta-- added for auto-set sensor capture delay mechanism
static const MUINT32 strobeDelayCount = 2; // cotta-- added for sync strobe protection and capture delay
static MUINT32 g_AEStatCnt = 0;
static MBOOL g_bBypassSensorSetting = FALSE;  // for busrt mode performance improve

// Sensor Info
static ACDK_SENSOR_INFO_STRUCT g_rSensorInfo;
static ACDK_SENSOR_CONFIG_STRUCT g_rSensorCfgData;

// AF thread
static pthread_t AFThread;
static Hal3A*  g_pHal3AObj = NULL;
static MINT32  g_bAFThreadLoop = 0;

MUINT32 g_u4PreviewShutterValue = 0;

void Hal3A::FocusThread(MINT32 enable)
{
    if (enable)  {

        if (g_bAFThreadLoop == 0)
        {
            for (MINT32 i=0; i<9; i++)
            {
                m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[i].uLeft = 0;
                m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[i].uRight = 0;
                m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[i].uUp = 0;
                m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[i].uBottom = 0;
            }

             m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uLeft = 0;
             m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uRight = 10;
             m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uUp = 0;
             m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uBottom = 10;
             setIspAFWinConfig(m_r3AStatConfig);      // set AF window config parameter

            // create AF thread
            AAA_LOG("[AFlog][Thread] Create");
            g_bAFThreadLoop = 1;
            pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_RR, RTPM_PRIO_CAMERA_COMPRESS};
            pthread_create(&AFThread, &attr, AFThreadFunc, NULL);
        }
    }
    else   {

        if (g_bAFThreadLoop == 1)
        {
            g_bAFThreadLoop = 0;
            pthread_join(AFThread, NULL);
            AAA_LOG("[AFlog][Thread] Delete");
        }
    }
}

void *
Hal3A::AFThreadFunc(
    void *arg
)
{
    AAA_LOG("[AFlog][Thread] tid: %d \n", gettid());

    while (g_bAFThreadLoop) {

        g_pHal3AObj->doAF();
    }

    AAA_LOG("[AFlog][Thread] End \n");

    return NULL;
}

/////////////////////////////////////////////////////////////////////////
//
// doAF () -
//! \brief
//
/////////////////////////////////////////////////////////////////////////
void Hal3A::doAF()
{
    MINT32 err = MHAL_NO_ERROR;
    ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;
    WaitIrq.Mode = ISP_DRV_IRQ_CLEAR_WAIT|ISP_DRV_IRQ_AFDONE;
    WaitIrq.Timeout = 200;
    static MINT32 pos = 0;

    AAA_FRAME_INPUT_PARAM_T r3AInput;

    //AAA_LOG("[AFlog][Thread] doAF \n");

    if (!m_pIspDrvObj) {
        AAA_LOG("[AFlog][Thread] m_pIspDrvObj null\n");
        return;
    }

    if (m_pIspDrvObj->waitIrq(&WaitIrq) >= 0) // success
    {
        if (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AF_LOG)  {
        AAA_LOG("[AFlog][Thread] ---- AF irq ------------\n");
        }

        if (!m_pmtk3A) {
            AAA_LOG("[AFlog][Thread] m_pmtk3A null\n");
            return;
        }

        getIsp3AStat(r3AInput.r3AStat);        // Get 3A statistics
        getFocusInfo(r3AInput.sFocusInfo);     // Get focus info
        getEZoomInfo(r3AInput.sEZoom);         // Get zoom info
        getFDInfo(r3AInput.sFDInfo);           // Get FD info

        m_pmtk3A->handleAF(r3AInput,m_r3AOutput);

        if (m_r3AOutput.bUpdateAF) {

            if (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AF_LOG)  {
            AAA_LOG("[AFlog][Thread] UpdateAFPos %d\n", m_r3AOutput.rAFOutput.i4FocusPos);
            }

            if (pos != m_r3AOutput.rAFOutput.i4FocusPos)
            {
                pos = m_r3AOutput.rAFOutput.i4FocusPos;
                setFocusPos(pos);
            }
        }

        #ifndef MTK_ZSD_AF_ENHANCE
        if (m_r3AOutput.bUpdateAFStatConfig) {
        #endif
            if (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AF_LOG)  {
            AAA_LOG("[AFlog][Thread][Pos]%d [Stat]%d %d %d %d [HWTH]%d [fgStat]%d\n", m_r3AOutput.rAFOutput.i4FocusPos, m_r3AOutput.rAFOutput.sAFStatConfig.sAFWin.sWin[0].uLeft, m_r3AOutput.rAFOutput.sAFStatConfig.sAFWin.sWin[0].uRight, m_r3AOutput.rAFOutput.sAFStatConfig.sAFWin.sWin[0].uUp, m_r3AOutput.rAFOutput.sAFStatConfig.sAFWin.sWin[0].uBottom, m_r3AOutput.rAFOutput.sAFStatConfig.uTH[3], m_r3AOutput.bUpdateAFStatConfig);
            }
            m_r3AStatConfig.rAFStatConfig = m_r3AOutput.rAFOutput.sAFStatConfig;
            setIspAFWinConfig(m_r3AStatConfig);      // set AF window config parameter
            setIspAFThr(m_r3AStatConfig);            // set AF threshold
        #ifndef MTK_ZSD_AF_ENHANCE
        }
        #endif
    }
    else
    {
        AAA_LOG("[AFlog][Thread] AF irq timeout\n");
    }

}

/*******************************************************************************
*
********************************************************************************/
static int getMs2()
{
	int t;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	t = (ts.tv_sec*1000+ts.tv_nsec/1000000);
	return t;
}
static int getMs()
{
//MSG_LOG("#QQ# getMs() Line=%d",__LINE__);
	//	max:
	//	2147483648 digit
	//	2147483.648 second
	//	35791.39413 min
	//	596.5232356 hour
	//	24.85513481 day
	int t;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	t = (tv.tv_sec*1000 + (tv.tv_usec+500)/1000);

	int t2;
	t2 = getMs2();

	AAA_LOG("getMs line=%d time=%d %d",__LINE__, t,t2);

	return t2;
}


int UnsignedSub(MUINT32 a, MUINT32 b)
{
	double v[3];
	double ret;
	v[0] = (double)a-b-0xffffffff-1;
	v[1] = (double)a-b+0xffffffff+1;
	v[2] = (double)a-b;

	ret=v[0];
	if(fabs(v[1])<fabs(ret))
		ret=v[1];
	if(fabs(v[2])<fabs(ret))
		ret=v[2];
	return (int)ret;
}
Hal3ABase*
Hal3A::
getInstance()
{
    AAA_LOG("[Hal3A] getInstance \n");
    static Hal3A singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void
Hal3A::
destroyInstance()
{

}

/*******************************************************************************
*
********************************************************************************/
Hal3A::Hal3A()
{
    m_pNvramDrvObj = NULL;
    m_pSensorDrvObj = NULL;
	m_pIspDrvObj = NULL;
	m_pMcuDrvObj = NULL;
	m_pStrobeDrvObj = NULL;
    m_pmtk3A = NULL;
	m_e3AState = AAA_STATE_NONE;
	m_bFlareAuto = TRUE;
	m_u4PreviewFlareOffset = 0;
	m_eSensorType = DUAL_CAMERA_NONE_SENSOR;
    m_u4SensorID = 0;
	m_bLockExposureSetting = FALSE;
    m_bAEASDInfoReady = FALSE;
    m_bAFASDInfoReady = FALSE;
    m_bAWBASDInfoReady = FALSE;

    memset(&m_r3AOutput, 0, sizeof(AAA_OUTPUT_PARAM_T));
	memset(&m_r3AStatConfig, 0, sizeof(AAA_STAT_CONFIG_T));
    memset(&m_rEZoomInfo, 0, sizeof(EZOOM_WIN_T));
	memset(&m_rFDInfo, 0, sizeof(FD_INFO_T));
	memset(&m_r3ADebugInfo, 0, sizeof(AAA_DEBUG_INFO_T));
	memset(&m_rAFWinResult, 0, sizeof(AF_WIN_RESULT_T));
	memset(&m_r3ANVRAMData, 0, sizeof(NVRAM_CAMERA_3A_STRUCT));
	memset(&m_rAFNVRAMData, 0, sizeof(NVRAM_LENS_PARA_STRUCT));
	memset(&m_strHDROutputInfo, 0, sizeof(Hal3A_HDROutputParam_T));

    m_i4AFLampMode = HAL3A_AF_LAMP_MODE_OFF;
    m_bAFLampStatus = FALSE;
    m_bAFLampIsOn = FALSE;
    m_i4AFLampCnt = 0;
    m_i4AFLampOffCnt = 0;
    m_bCancelAF = FALSE;
    m_bFlashActive = FALSE;

    m_Users = 0;

    //cotta--added for strobe protection
    strobeHal3AObj = this;
    g_pHal3AObj = this;

}

/*******************************************************************************
*
********************************************************************************/
Hal3A::~Hal3A()
{
    g_pHal3AObj = NULL;

}

/*******************************************************************************
*  Author : cotta
*  Functionality : setting ISP callback to turn on /off strobe
********************************************************************************/
static void setStrobeISPCallback(MBOOL isFlash)
{
    if(isFlash)
    {
        strobeHal3AObj->strobeOnISPCallback(); //strobe turn on callback
    }
    else
    {
        strobeHal3AObj->strobeOffISPCallback(); //strobe turn off callback
    }
}


/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::init(
    MINT32 a_i4SensorType
)
{
    MINT32 err = MHAL_NO_ERROR;
    MUINT32 result = 0xff;
    AAA_INIT_INPUT_PARAM_T r3AInitInput;
     MINT32 i4CurrLensId = 0;

    AAA_LOG("[Hal3AInit]\n");

	Mutex::Autolock lock(m_Lock);

	if (m_Users > 0)
	{
		AAA_LOG("%d has created \n", m_Users);
		android_atomic_inc(&m_Users);
		return MHAL_NO_ERROR;
	}


    EepromDrvBase *m_pEepromDrvObj = EepromDrvBase::createInstance();
    GET_SENSOR_CALIBRATION_DATA_STRUCT GetSensorCalData;

    // initial default value
    g_PreFrameRate = 0;
    g_SensorFrameRate = 0;
    m_bFlickerState = -1;

//    AAA_LOG("[init] g_SensorFrameRate:%d %d\n", g_SensorFrameRate, g_SensorFrameRate);

    if (m_pmtk3A != NULL) {
        return MHAL_NO_ERROR;
    }

    m_pmtk3A = create3AInstance();
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    // NVRAM driver
    m_pNvramDrvObj = NvramDrvBase::createInstance();

    //sensor driver
    m_pSensorDrvObj = SensorDrv::createInstance(SENSOR_MAIN|SENSOR_SUB);

    // ISP driver
	m_pIspDrvObj = IspDrv::createInstance();

    // lens driver
    i4CurrLensId = MCUDrv::getCurrLensID();
    m_pMcuDrvObj = MCUDrv::createInstance(i4CurrLensId);

	mcuMotorInfo rMotorInfo;
    if (m_pMcuDrvObj->getMCUInfo(&rMotorInfo) < 0) { // kernel driver open fail (in isp_hal)
        m_pMcuDrvObj->destroyInstance();
		m_pMcuDrvObj = NULL;
	}

	// strobe driver
	m_pStrobeDrvObj = StrobeDrv::createInstance();

	if (m_pStrobeDrvObj->getFlashlightType() == StrobeDrv::FLASHLIGHT_NONE) {
        m_pStrobeDrvObj->destroyInstance();
		m_pStrobeDrvObj = NULL;
	}

	// Get sensor ID
    if (a_i4SensorType == HAL3A_SENSOR_DEV_MAIN) {
	    m_eSensorType = DUAL_CAMERA_MAIN_SENSOR;
        m_u4SensorID = m_pSensorDrvObj->getMainSensorID();
    }
    else if (a_i4SensorType == HAL3A_SENSOR_DEV_SUB) {
        m_eSensorType = DUAL_CAMERA_SUB_SENSOR;
		m_u4SensorID = m_pSensorDrvObj->getSubSensorID();
	}
	else {
        AAA_ERR("init() error: unknown sensor type");
		return MHAL_INVALID_PARA;
	}

	AAA_LOG("sensor ID = %d\n", m_u4SensorID);


    // Get NVRAM data
    MHAL_ASSERT(m_pNvramDrvObj != NULL, "m_pNvramDrvObj is NULL");

    err = m_pNvramDrvObj->readNvram(m_eSensorType, m_u4SensorID, CAMERA_NVRAM_DATA_3A, (void *)&r3AInitInput.r3ANVRAMData, sizeof(NVRAM_CAMERA_3A_STRUCT));
	if (err < 0) {
		AAA_ERR("readNvram() error: CAMERA_NVRAM_DATA_3A");
		return err;
    }

    err = m_pNvramDrvObj->readNvram(m_eSensorType, m_u4SensorID, CAMERA_NVRAM_DATA_LENS, (void *)&r3AInitInput.rAFNVRAMData, sizeof(NVRAM_LENS_PARA_STRUCT));
    if (err < 0) {
		AAA_ERR("readNvram() error: CAMERA_NVRAM_DATA_LENS");
		return err;
    }

    err = m_pNvramDrvObj->readNvram(m_eSensorType, m_u4SensorID, CAMERA_DATA_3A_PARA, (void *)&r3AInitInput.r3AParam, sizeof(AAA_PARAM_T));
    if (err < 0) {
		AAA_ERR("readNvram() error: CAMERA_DATA_3A_PARA");
		return err;
    }

    AAA_LOG("u4AWBGainOutputScaleUnit = %d; u4AWBGainOutputUpperLimit = %d\n", r3AInitInput.r3AParam.rAWBParam.rChipParam.u4AWBGainOutputScaleUnit,
                                                                               r3AInitInput.r3AParam.rAWBParam.rChipParam.u4AWBGainOutputUpperLimit);

    err = m_pNvramDrvObj->readNvram(m_eSensorType, m_u4SensorID, CAMERA_DATA_3A_STAT_CONFIG_PARA, (void *)&r3AInitInput.r3AStatConfigParam, sizeof(AAA_STAT_CONFIG_PARAM_T));
    if (err < 0) {
		AAA_ERR("readNvram() error: CAMERA_DATA_3A_STAT_CONFIG_PARA");
		return err;
    }

    // AWB calibration
    if ((r3AInitInput.r3ANVRAMData.rAWBNVRAM.rCalData.rCalGain.u4R != 0) &&
		(r3AInitInput.r3ANVRAMData.rAWBNVRAM.rCalData.rCalGain.u4G != 0) &&
		(r3AInitInput.r3ANVRAMData.rAWBNVRAM.rCalData.rCalGain.u4B != 0) &&
        (r3AInitInput.r3ANVRAMData.rAWBNVRAM.rCalData.rDefGain.u4R != 0) &&
		(r3AInitInput.r3ANVRAMData.rAWBNVRAM.rCalData.rDefGain.u4G != 0) &&
		(r3AInitInput.r3ANVRAMData.rAWBNVRAM.rCalData.rDefGain.u4B != 0)) {
        result= m_pEepromDrvObj->GetEepromCalData(m_eSensorType, m_u4SensorID, CAMERA_EEPROM_DATA_PREGAIN, (void *)&GetSensorCalData);

        if (result&EEPROM_ERR_NO_PREGAIN)
        {
            AAA_ERR("No Pregain data\n 0x%x\n",result);
        }
        else
        {
            r3AInitInput.r3ANVRAMData.rAWBNVRAM.rCalData.rCalGain.u4R    = GetSensorCalData.rCalGain.u4R;
            r3AInitInput.r3ANVRAMData.rAWBNVRAM.rCalData.rCalGain.u4G    = GetSensorCalData.rCalGain.u4G;
            r3AInitInput.r3ANVRAMData.rAWBNVRAM.rCalData.rCalGain.u4B    = GetSensorCalData.rCalGain.u4B;
            AAA_LOG("rCalGain.u4R = %d\n",GetSensorCalData.rCalGain.u4R);
            AAA_LOG("rCalGain.u4G = %d\n",GetSensorCalData.rCalGain.u4G);
            AAA_LOG("rCalGain.u4B = %d\n",GetSensorCalData.rCalGain.u4B);
        }
    }

	m_r3ANVRAMData = r3AInitInput.r3ANVRAMData;
	m_rAFNVRAMData = r3AInitInput.rAFNVRAMData;

    // Get sensor info
    memset (&g_rSensorInfo, 0 , sizeof(ACDK_SENSOR_INFO_STRUCT));
    memset(&g_rSensorCfgData, 0, sizeof(ACDK_SENSOR_CONFIG_STRUCT));
    m_pSensorDrvObj->getInfo(ACDK_SCENARIO_ID_CAMERA_PREVIEW, &g_rSensorInfo, &g_rSensorCfgData);

    MUINT32 u4PvW, u4PvH;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT rSensorResolution;
    memset(&rSensorResolution, 0, sizeof(ACDK_SENSOR_RESOLUTION_INFO_STRUCT));
    m_pSensorDrvObj->getResolution(&rSensorResolution);
    u4PvW = rSensorResolution.SensorPreviewWidth;
    u4PvH = rSensorResolution.SensorPreviewHeight;

	AAA_LOG("u4PvW = %d; u4PvH = %d\n", u4PvW, u4PvH);

    r3AInitInput.rAEIniPara.strPreviewAEInfo.u4SensorWidth = u4PvW;
    r3AInitInput.rAEIniPara.strPreviewAEInfo.u4SensorHeight = u4PvH;

    r3AInitInput.rAEIniPara.strMovieAEInfo.u4SensorWidth = u4PvW;
    r3AInitInput.rAEIniPara.strMovieAEInfo.u4SensorHeight = u4PvH;

    m_rEZoomInfo.u4XOffset = 0;
    m_rEZoomInfo.u4YOffset = 0;
    m_rEZoomInfo.u4XWidth = u4PvW;
    m_rEZoomInfo.u4YHeight = u4PvH;

    if (m_pIspDrvObj) {
        m_pIspDrvObj->init();
	m_pIspDrvObj->sendCommand(CMD_GET_ISP_ADDR, (MINT32)&m_pIspReg);
    }

    if (m_pMcuDrvObj) {
        // Update infinite & macro position to lens driver
        m_pMcuDrvObj->setMCUInfPos(r3AInitInput.rAFNVRAMData.rFocusRange.i4InfPos);
		m_pMcuDrvObj->setMCUMacroPos(r3AInitInput.rAFNVRAMData.rFocusRange.i4MacroPos);

		// Get focus Info
        getFocusInfo(r3AInitInput.sFocusInfo);
    }

    m_pmtk3A->init3A(r3AInitInput, m_r3AOutput, m_r3AStatConfig);

    m_pmtk3A->setAFCoef(get_AF_Coef());

	if (!m_pMcuDrvObj) {
	    disableAF();
	}

    // Strobe
    if (m_pStrobeDrvObj)
    {
        m_pStrobeDrvObj->setState(0); //preview state

        //cotta--set capture delay of sensor to the strobe kernel driver
        sensorDelayValue = g_rSensorInfo.CaptureDelayFrame;

        if(sensorDelayValue >= strobeDelayCount)
        {
            m_pStrobeDrvObj->sendCommand(CMD_STROBE_SET_CAP_DELAY, sensorDelayValue - strobeDelayCount, NULL, NULL);
        }
        else
        {
            m_pStrobeDrvObj->sendCommand(CMD_STROBE_SET_CAP_DELAY, 0, NULL, NULL);
        }

        //--cotta

        //cotta-- added for strobe WDT customize

        MUINT32 strobeWDTValue = 0;
        m_pStrobeDrvObj->sendCommand(CMD_STROBE_GET_WDT_VALUE, 0, &strobeWDTValue, NULL);
        m_pmtk3A->setStrobeWDTValue(strobeWDTValue);

        //--cotta
    }

    //cotta-- initialize

    strobeZSDMode = 0;
    strobeZSDMFOffset = 0;

    //--cotta

    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("debug.aaa_hal.option", value, "0");
    AAA_HAL_DBG_OPTION = atoi(value);

    android_atomic_inc(&m_Users);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::uninit()
{
    AAA_LOG("[Hal3AUninit]\n");

	AAA_LOG("[%s()] - E. m_Users: %d \n", __FUNCTION__, m_Users);

	Mutex::Autolock lock(m_Lock);

	// If no more users, return directly and do nothing.
	if (m_Users <= 0)
	{
		return MHAL_NO_ERROR;
	}

	// More than one user, so decrease one User.
	android_atomic_dec(&m_Users);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        if (m_pmtk3A) {
            m_pmtk3A->deinit3A();
            delete m_pmtk3A;
            m_pmtk3A = NULL;
        }

        // NVRAM driver
        if (m_pNvramDrvObj) {
            m_pNvramDrvObj->destroyInstance();
		    m_pNvramDrvObj = NULL;
        }

	    // ISP driver
        if (m_pIspDrvObj) {
            m_pIspDrvObj->uninit();
            m_pIspDrvObj->destroyInstance();
		    m_pIspDrvObj = NULL;
        }

        // lens driver
        if (m_pMcuDrvObj) {
            m_pMcuDrvObj->destroyInstance();
		    m_pMcuDrvObj = NULL;
        }

	    // strobe driver
        if (m_pStrobeDrvObj)
        {
            strobeZSDMode = 0;  // cotta-- initialize
            m_pStrobeDrvObj->destroyInstance();
		    m_pStrobeDrvObj = NULL;
        }
    }
	else	// There are still some users.
	{
		AAA_LOG("Still %d users \n", m_Users);
	}

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setState(
    MINT32 a_i4aaaState
)
{
    AE_MODE_CFG_T strAEOutput;

    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    if((m_e3AState == AAA_STATE_AF) && (a_i4aaaState == AAA_STATE_AUTO_FRAMERATE_PREVIEW)) {   // for cancel AF state
        m_bCancelAF = TRUE;
    }

    m_e3AState = (AAA_STATE_T)a_i4aaaState;
    m_pmtk3A->set3AState(m_e3AState);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::do3A(
    MINT32 a_i4aaaState,
    MUINT32 *a_pWaitVDNum
)
{
    MINT32 err = MHAL_NO_ERROR; // -1: error
                               //  0: no error
                               //  1: update ISO
                               //  2: update CCT
                                //  4: update ASD info
    AAA_FRAME_INPUT_PARAM_T r3AInput;
    static MUINT32 u4OldFlashLevel = 0;
    MUINT32 u4FlashLevel;
    MINT32 i4FlickerMode;
    MINT32 i;

    m_e3AState = (AAA_STATE_T)a_i4aaaState;

    // Get 3A statistics
    getIsp3AStat(r3AInput.r3AStat);

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AE_BRIGHTNESS) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        // AE statistic window test
        MUINT32 u4AvgW = 0;

        for (i=0; i<25; i++) {
            u4AvgW += r3AInput.r3AStat.rAEStat.u4AEWindowInfo[i];
        }

        AAA_LOG("AE Y:%d\n", 4*u4AvgW/25/r3AInput.r3AStat.rAEStat.u4AEBlockCnt);

        AAA_LOG("a_r3AStat.rAEStat.u4AEBlockCnt = %d \n", r3AInput.r3AStat.rAEStat.u4AEBlockCnt);
        AAA_LOG("a_r3AStat.u4RawWidth = %d \n", r3AInput.r3AStat.u4RawWidth);
        AAA_LOG("a_r3AStat.u4RawHeight = %d \n", r3AInput.r3AStat.u4RawHeight);

        AAA_LOG("a_r3AStat.rAEStat.rFDAEStat.u4FDAE_SUM = %d %d\n", r3AInput.r3AStat.rAEStat.rFDAEStat.u4FDAE_SUM[0], r3AInput.r3AStat.rAEStat.rFDAEStat.u4FDAE_SUM[1]);
        AAA_LOG("a_r3AStat.rAEStat.rFDAEStat.u4FDAE_COUNT = %d %d\n", r3AInput.r3AStat.rAEStat.rFDAEStat.u4FDAE_COUNT[0], r3AInput.r3AStat.rAEStat.rFDAEStat.u4FDAE_SUM[1]);
    }

    // Get focus info
    getFocusInfo(r3AInput.sFocusInfo);

    // Get zoom info
    getEZoomInfo(r3AInput.sEZoom);

    // Get FD info
    getFDInfo(r3AInput.sFDInfo);

    if ((m_e3AState == AAA_STATE_AF) && (!m_r3AOutput.bAFDone))
    {
        if ((m_pStrobeDrvObj) && (m_i4AFLampMode != HAL3A_AF_LAMP_MODE_OFF) && (m_bAFLampStatus == FALSE))
        {
            if (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AF_LOG)  {
            AAA_LOG("[AFLamp] Entry [m_e3AState]%d [m_i4AFLampMode]%d\n", m_e3AState, m_i4AFLampMode);
            }
            m_bAFLampStatus = TRUE;

            if ((m_i4AFLampMode == HAL3A_AF_LAMP_MODE_ON) || (m_pmtk3A->getAFLampIsAutoOn()))
            {
                m_bAFLampIsOn = TRUE;
                if (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AF_LOG)  {
                AAA_LOG("[AFLamp] set AFLampOn flag TRUE to App3A\n");
                }
                m_pmtk3A->setAFLampInfo(TRUE, (m_i4AFLampMode == HAL3A_AF_LAMP_MODE_AUTO));
            }
        }
    }

    if (m_i4AFLampCnt > 0)
    {
        // Wait AE stable when strobe on;
        if (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AF_LOG)  {
        AAA_LOG("[AFLamp] Wait Frame\n");
        }
        m_r3AOutput.bUpdateAE = 0;
        m_i4AFLampCnt--;
    }
    else
    {
        // Perform 3A algorithm
        m_pmtk3A->handle3A(r3AInput, m_r3AOutput);
        m_i4AFLampCnt = 0;
    }

    // Get wait VD number for next 3A statistics
    *a_pWaitVDNum = m_r3AOutput.u4WaitVDNum;

#if 0
    AAA_LOG("focus current pos:%d\n", r3AInput.sFocusInfo.i4CurrentPos);

    static MUINT32 u4Count = 0;

	if (u4Count == 1)
	{
	    setFocusPos(0);
	}
	else if (u4Count == 6)
	{
	    setFocusPos(1023);
	}
	else if (u4Count == 11)
	{
	    u4Count = 0;
	}

    u4Count++;
#endif

    if ((m_e3AState == AAA_STATE_AUTO_FRAMERATE_PREVIEW) || (m_e3AState == AAA_STATE_MANUAL_FRAMERATE_PREVIEW)) {

        if ((m_pStrobeDrvObj) && (m_i4AFLampMode != HAL3A_AF_LAMP_MODE_OFF) && (m_bAFLampStatus == TRUE))
        {
            m_bAFLampStatus = FALSE;
            m_i4AFLampOffCnt = 0;

            if (m_pStrobeDrvObj->setFire(0) != MHAL_NO_ERROR) {
                AAA_LOG("check setFire OFF\n");
            }
        }

        holdIspRegs(TRUE);

        if (m_r3AOutput.bUpdateAE) {
//            AAA_LOG("[do3A] Preview frame rate:%d Exp:%d Sensor gain:%d ISP gain:%d ISO:%d\n", m_r3AOutput.rAEOutput.rPreviewMode.u2FrameRate,
//            	m_r3AOutput.rAEOutput.rPreviewMode.u4Eposuretime, m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain, m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain,
//            	m_r3AOutput.rAEOutput.rPreviewMode.u4ISOSpeed);

            setExpParam(m_r3AOutput.rAEOutput.rPreviewMode.u4Eposuretime,
                        m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain,
                        m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain >> 3);

            g_u4PreviewShutterValue = m_r3AOutput.rAEOutput.rPreviewMode.u4Eposuretime;
            
            setIspFlare(m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG);

            m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_AUTO_MODE, &i4FlickerMode);
//            AAA_LOG("[do3A] m_e3AState:%d framerate:%d %d\n", m_e3AState, m_r3AOutput.rAEOutput.rPreviewMode.u2FrameRate, i4FlickerMode);
            if((i4FlickerMode == LIB3A_AE_FLICKER_AUTO_MODE_50HZ) || (i4FlickerMode == LIB3A_AE_FLICKER_AUTO_MODE_60HZ)) {
                // enable auto flicker
                setSensorFlickerFrameRate(TRUE);
            } else {
                // disable auto flicker
                setSensorFlickerFrameRate(FALSE);
            }

            if((m_e3AState == AAA_STATE_MANUAL_FRAMERATE_PREVIEW) ||
            	((m_e3AState == AAA_STATE_AUTO_FRAMERATE_PREVIEW) && (m_r3AOutput.rAEOutput.rPreviewMode.u2FrameRate == 0))){
                if(g_PreFrameRate == m_r3AOutput.rAEOutput.rPreviewMode.u2FrameRate) {
                    setSensorFrameRate((MUINT32)m_r3AOutput.rAEOutput.rPreviewMode.u2FrameRate);
                }
            }

            g_PreFrameRate = m_r3AOutput.rAEOutput.rPreviewMode.u2FrameRate;

            m_bAEASDInfoReady = TRUE;

            err |= HAL_3A_UPDATE_ISO;
        }

        if (m_r3AOutput.bUpdateAWB)  {
            setIspAWBGain(m_r3AOutput.rAWBOutput.rPreviewAWBGain, TRUE);

            m_bAWBASDInfoReady = TRUE;

            err |= HAL_3A_UPDATE_CCT;
        }

        holdIspRegs(FALSE);

        if (m_r3AOutput.bUpdateAF) {
            setFocusPos(m_r3AOutput.rAFOutput.i4FocusPos);

            m_bAFASDInfoReady = TRUE;
        }

        if (m_r3AOutput.bUpdateAEStatConfig) {
            m_r3AStatConfig.rAEStatConfig = m_r3AOutput.rAEStatConfig;
        }

        if (m_r3AOutput.bUpdateAFStatConfig) {
            m_r3AStatConfig.rAFStatConfig = m_r3AOutput.rAFOutput.sAFStatConfig;
        }

        if (m_r3AOutput.bUpdateAWBStatConfig) {
            m_r3AStatConfig.rAWBStatConfig = m_r3AOutput.rAWBStatConfig;
        }

        if (m_r3AOutput.bUpdateAEStatConfig || m_r3AOutput.bUpdateAWBStatConfig || m_r3AOutput.bUpdateAFStatConfig) {
           setIsp3AStatConfig(m_r3AStatConfig);
        }

        if (m_bAEASDInfoReady && m_bAWBASDInfoReady && m_bAFASDInfoReady) {
           err |= HAL_3A_UPDATE_ASD_INFO;
           m_bAEASDInfoReady = FALSE;
           m_bAWBASDInfoReady = FALSE;
           m_bAFASDInfoReady = FALSE;
        }

    }
    else if ((m_e3AState == AAA_STATE_AF) && (!m_r3AOutput.bAFDone)) {

        m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_AUTO_MODE, &i4FlickerMode);
//    AAA_LOG("[do3A] m_e3AState:%d framerate:%d %d\n", m_e3AState, m_r3AOutput.rAEOutput.rPreviewMode.u2FrameRate, i4FlickerMode);
        if((i4FlickerMode == LIB3A_AE_FLICKER_AUTO_MODE_50HZ) || (i4FlickerMode == LIB3A_AE_FLICKER_AUTO_MODE_60HZ)) {
            // disable auto flicker
            setSensorFlickerFrameRate(FALSE);
        }

        if (m_bAFLampIsOn == TRUE)
        {
            m_bAFLampIsOn = FALSE;

            if (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AF_LOG)  {
            AAA_LOG("[AFLamp] Turn on LED\n");
            }

            m_i4AFLampCnt = 2;
            m_pStrobeDrvObj->setState(0);

            if (m_pStrobeDrvObj->setLevel(20) != MHAL_NO_ERROR) {
                AAA_LOG("check setLevel ON\n");
            }

            if (m_pStrobeDrvObj->setFire(1) != MHAL_NO_ERROR) {
                AAA_LOG("check setFire ON\n");
            }
            m_bFlashActive = TRUE;
        }

		holdIspRegs(TRUE);

        if (m_r3AOutput.bUpdateAE) {
            AAA_LOG("[do3A] Preview frame rate:%d Exp:%d Sensor gain:%d ISP gain:%d ISO:%d\n", m_r3AOutput.rAEOutput.rPreviewMode.u2FrameRate,
            	m_r3AOutput.rAEOutput.rPreviewMode.u4Eposuretime, m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain, m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain,
            	m_r3AOutput.rAEOutput.rPreviewMode.u4ISOSpeed);
            AAA_LOG("[do3A] AF frame rate:%d Exp:%d Sensor gain:%d ISP gain:%d ISO:%d\n", m_r3AOutput.rAEOutput.rAFMode.u2FrameRate,
            	m_r3AOutput.rAEOutput.rAFMode.u4Eposuretime, m_r3AOutput.rAEOutput.rAFMode.u4AfeGain, m_r3AOutput.rAEOutput.rAFMode.u4IspGain,
            	m_r3AOutput.rAEOutput.rAFMode.u4ISOSpeed);
            AAA_LOG("[do3A] 1'st Capture frame rate:%d Exp:%d Sensor gain:%d ISP gain:%d ISO:%d\n", m_r3AOutput.rAEOutput.rCaptureMode[0].u2FrameRate,
            	m_r3AOutput.rAEOutput.rCaptureMode[0].u4Eposuretime, m_r3AOutput.rAEOutput.rCaptureMode[0].u4AfeGain, m_r3AOutput.rAEOutput.rCaptureMode[0].u4IspGain,
            	m_r3AOutput.rAEOutput.rCaptureMode[0].u4ISOSpeed);
            AAA_LOG("[do3A] 2'st Capture frame rate:%d Exp:%d Sensor gain:%d ISP gain:%d ISO:%d\n", m_r3AOutput.rAEOutput.rCaptureMode[1].u2FrameRate,
            	m_r3AOutput.rAEOutput.rCaptureMode[1].u4Eposuretime, m_r3AOutput.rAEOutput.rCaptureMode[1].u4AfeGain, m_r3AOutput.rAEOutput.rCaptureMode[1].u4IspGain,
            	m_r3AOutput.rAEOutput.rCaptureMode[1].u4ISOSpeed);
            AAA_LOG("[do3A] 3'st Capture frame rate:%d Exp:%d Sensor gain:%d ISP gain:%d ISO:%d\n", m_r3AOutput.rAEOutput.rCaptureMode[1].u2FrameRate,
            	m_r3AOutput.rAEOutput.rCaptureMode[2].u4Eposuretime, m_r3AOutput.rAEOutput.rCaptureMode[2].u4AfeGain, m_r3AOutput.rAEOutput.rCaptureMode[2].u4IspGain,
            	m_r3AOutput.rAEOutput.rCaptureMode[2].u4ISOSpeed);

            setExpParam(m_r3AOutput.rAEOutput.rAFMode.u4Eposuretime,
                        m_r3AOutput.rAEOutput.rAFMode.u4AfeGain,
                        m_r3AOutput.rAEOutput.rAFMode.u4IspGain >> 3);

            setIspFlare(m_r3AOutput.rAEOutput.rAFMode.FlareCFG);
        }

        if (m_r3AOutput.bUpdateAWB)  {
            if (m_r3AOutput.rAWBOutput.i4IsStrobeFired)
                setIspAWBGain(m_r3AOutput.rAWBOutput.rPreviewStrobeAWBGain, TRUE);
            else
                setIspAWBGain(m_r3AOutput.rAWBOutput.rPreviewAWBGain, TRUE);

            err |= HAL_3A_UPDATE_CCT;
        }

        holdIspRegs(FALSE);

        if (m_r3AOutput.bUpdateAEStatConfig) {
            m_r3AStatConfig.rAEStatConfig = m_r3AOutput.rAEStatConfig;
        }

        if (m_r3AOutput.bUpdateAF) {
            setFocusPos(m_r3AOutput.rAFOutput.i4FocusPos);
        }

        if (m_r3AOutput.bUpdateAEStatConfig ||m_r3AOutput.bUpdateAFStatConfig) {
            m_r3AStatConfig.rAFStatConfig = m_r3AOutput.rAFOutput.sAFStatConfig;

            setIsp3AStatConfig(m_r3AStatConfig);
        }

    }
    else if ((m_e3AState == AAA_STATE_AF) && (m_r3AOutput.bAFDone)) {
		//switch to preview mode for lock AE

        if ((m_pStrobeDrvObj) && (m_i4AFLampMode != HAL3A_AF_LAMP_MODE_OFF) && (m_bAFLampStatus == TRUE))
        {
            m_i4AFLampOffCnt++;

            if (m_i4AFLampOffCnt > 1)
            {
                m_bAFLampStatus = FALSE;

                if (m_pStrobeDrvObj->setFire(0) != MHAL_NO_ERROR) {
                    AAA_LOG("check setFire OFF\n");
                }
                m_i4AFLampOffCnt = 0;
            }
        }

		holdIspRegs(TRUE);

        if (m_r3AOutput.bUpdateAE) {
            setExpParam(m_r3AOutput.rAEOutput.rPreviewMode.u4Eposuretime,
                        m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain,
                        m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain >> 3);

            setIspFlare(m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG);

            err |= HAL_3A_UPDATE_ISO;
        }

        if (m_r3AOutput.bUpdateAWB) {
            if (m_r3AOutput.rAWBOutput.i4IsStrobeFired)
                setIspAWBGain(m_r3AOutput.rAWBOutput.rPreviewStrobeAWBGain, TRUE);
            else
                setIspAWBGain(m_r3AOutput.rAWBOutput.rPreviewAWBGain, TRUE);

            err |= HAL_3A_UPDATE_CCT;
        }

        holdIspRegs(FALSE);

        if (m_r3AOutput.bUpdateAF) {
            setFocusPos(m_r3AOutput.rAFOutput.i4FocusPos);
        }

    }
    else if (m_e3AState == AAA_STATE_PRE_CAPTURE)
    {
        holdIspRegs(TRUE);

        if (m_r3AOutput.bUpdateAWB)  {
            if (m_r3AOutput.rAWBOutput.i4IsStrobeFired)
                setIspAWBGain(m_r3AOutput.rAWBOutput.rPreviewStrobeAWBGain, TRUE);
            else
                setIspAWBGain(m_r3AOutput.rAWBOutput.rPreviewAWBGain, TRUE);

            err |= HAL_3A_UPDATE_CCT;
        }

        //for AE preflash process
        if (m_r3AOutput.bUpdateAE)
        {
            setExpParam(m_r3AOutput.rAEOutput.rPreviewMode.u4Eposuretime,
                        m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain,
                        m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain >> 3);

            setIspFlare(m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG);

            if (m_pStrobeDrvObj)
            {
                // for preflash
                u4FlashLevel = m_r3AOutput.rAEOutput.rPreviewMode.u4StrobeWidth;

                // Set LED flash on/off
                if (u4FlashLevel)
                {
                    if (u4OldFlashLevel && (u4OldFlashLevel != u4FlashLevel))
                    {
                        // Run time change level, disable 1st.
                        if (m_pStrobeDrvObj->setFire(0)  == MHAL_NO_ERROR)
                        {
                            AAA_LOG("check setFire OFF\n");
                        }
                    }

                    if (m_pStrobeDrvObj->setLevel(u4FlashLevel) == MHAL_NO_ERROR)
                    {
                        AAA_LOG("check setLevel:%d\n",u4FlashLevel);
                    }
                    else
                    {
                        err = -1;
                    }

                    // LED ON
                    if (m_pStrobeDrvObj->setFire(1) == MHAL_NO_ERROR)
                    {
                        AAA_LOG("check setFire ON\n");
                        m_bFlashActive = TRUE;
                    }
                    else
                    {
                        err = -1;
                    }

                    u4OldFlashLevel = u4FlashLevel;
                }
                else
                {
                    // Disable LED
                    if (m_pStrobeDrvObj->setFire(0) == MHAL_NO_ERROR)
                    {
                        AAA_LOG("check halStrobeSetFire OFF\n");
                    }
					else
                    {
                        err = -1;
                    }

                    u4OldFlashLevel = u4FlashLevel;
                }
            }
        }
        else if(g_bHDRUpdate == FALSE)
        {
            updateCapParamsByHDR();   // for HDR to update the capture information.
        }

        holdIspRegs(FALSE);
    }
    else if (m_e3AState == AAA_STATE_CAPTURE)
    {
        AAA_LOG("AAA_STATE_CAPTURE\n");
        if (m_r3AOutput.bUpdateAWB)  {
            AAA_LOG("m_r3AOutput.rAWBOutput.rCaptureAWBGain.u4R = %d\n", m_r3AOutput.rAWBOutput.rCaptureAWBGain.u4R);
            AAA_LOG("m_r3AOutput.rAWBOutput.rCaptureAWBGain.u4G = %d\n", m_r3AOutput.rAWBOutput.rCaptureAWBGain.u4G);
            AAA_LOG("m_r3AOutput.rAWBOutput.rCaptureAWBGain.u4B = %d\n", m_r3AOutput.rAWBOutput.rCaptureAWBGain.u4B);
            setIspAWBGain(m_r3AOutput.rAWBOutput.rCaptureAWBGain, FALSE);
        }
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setPreviewParam()
{
    m_pmtk3A->getAEModeSetting(m_r3AOutput.rAEOutput.rPreviewMode, AAA_STATE_AUTO_FRAMERATE_PREVIEW);
    g_SensorFrameRate = 0;   // reset to default
    g_u4CaptureCnt = 0;     //Reset Capture Cnt

    AAA_LOG("[setPreviewParam] Preview frame rate:%d Exp:%d Sensor gain:%d ISP gain:%d ISO:%d\n", m_r3AOutput.rAEOutput.rPreviewMode.u2FrameRate,
            	m_r3AOutput.rAEOutput.rPreviewMode.u4Eposuretime, m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain, m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain,
            	m_r3AOutput.rAEOutput.rPreviewMode.u4ISOSpeed);

    // Set sensor exposure time
    setSensorExpTime(m_r3AOutput.rAEOutput.rPreviewMode.u4Eposuretime);

    // Set sensor gain
    setSensorGain(m_r3AOutput.rAEOutput.rPreviewMode.u4AfeGain);

    // Set RAW gain
	setIspRAWGain((m_r3AOutput.rAEOutput.rPreviewMode.u4IspGain)>>3, FALSE);

    // Set 3A statistics config
    setIsp3AStatConfig(m_r3AStatConfig);

    // Set flare
    setIspFlare(m_r3AOutput.rAEOutput.rPreviewMode.FlareCFG);

    // Set AWB gain
    setIspAWBGain(m_r3AOutput.rAWBOutput.rPreviewAWBGain, FALSE);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
 Author : Cotta
 Functionality : ISP callback function to turn on strobe. sync with ISP start
********************************************************************************/

void Hal3A::strobeOnISPCallback()
{
    aaaTimer strobeTimer("strobeOnISPCallback");
    MUINT32 tempSensorDelay = 0;
    MUINT32 cleanISPVDSignal;

    //=== setting capture delay value ===

    // get ISP capture delay value
    m_pIspDrvObj->sendCommand(ISP_DRV_CMD_GET_CAPTURE_DELAY,(MINT32)&tempSensorDelay,0,0);

    if(tempSensorDelay != sensorDelayValue) //value of sensor capture delay has changed
    {
        AAA_LOG("[strobeOnISPCallback] sensorDelayValue = %u, tempSensorDelay = %u\n", sensorDelayValue,tempSensorDelay);
        sensorDelayValue = tempSensorDelay;

        if(sensorDelayValue >= strobeDelayCount)
        {
            //set capture delay of sensor to the strobe kernel driver
            m_pStrobeDrvObj->sendCommand(CMD_STROBE_SET_CAP_DELAY, sensorDelayValue - strobeDelayCount, NULL, NULL);
        }
        else
        {
            //set capture delay of sensor to the strobe kernel driver
            m_pStrobeDrvObj->sendCommand(CMD_STROBE_SET_CAP_DELAY, 0, NULL, NULL);
        }
    }

    // clean ISP register remaining VD signal
    //m_pIspDrvObj->sendCommand(ISP_DRV_CMD_CLEAR_INT_STATUS,0,0,0);

    // send strobe trigger command
    m_pStrobeDrvObj->setCaptureFlashlightConf(m_r3AOutput.rAEOutput.rCaptureMode[0].u4StrobeWidth);

    // === strobe and VD timing control ===

    if(sensorDelayValue < strobeDelayCount)   // need to wait VD signal between setFire and ISP start
    {
        AAA_LOG("[strobeOnISPCallback] waitVD : sensorDelayValue = %u, strobeDelayCount = %u\n", sensorDelayValue,strobeDelayCount);

        // force turn on VD interrupt workqueue
        MBOOL Enable = MTRUE;
        m_pIspDrvObj->sendCommand(ISP_DRV_CMD_SET_VD_PROC,(MINT32)&Enable,0,0);

        // there need to be one VD between strobe turn on and ISP start
        ISP_DRV_WAIT_IRQ_STRUCT WaitIrq;

        WaitIrq.Mode = ISP_DRV_IRQ_CLEAR_WAIT|ISP_DRV_IRQ_VSYNC;
        WaitIrq.Timeout = 1000;

        m_pIspDrvObj->waitIrq(&WaitIrq);    // wait once

        // wait extra VD when in high current mode of ZSD mode
        if(sensorDelayValue == 0 && (strobeZSDMode == 1 || m_r3AOutput.rAEOutput.rCaptureMode[0].u4StrobeWidth == 0xFF))
        {
            m_pIspDrvObj->waitIrq(&WaitIrq);    // wait twice
        }
    }
}


/*******************************************************************************
 Author : Cotta
 Functionality : ISP callback function to turn off strobe. sync with ISP stop
********************************************************************************/

void Hal3A::strobeOffISPCallback()
{
    aaaTimer strobeTimer("strobeOffISPCallback");
    MUINT32 strobeEndTime = 0;

    //turn off strobe
    AAA_LOG("[strobeISPCallback] turn off strobe\n");

    m_pStrobeDrvObj->setFire(0);

    if(m_r3AOutput.rAEOutput.rCaptureMode[0].u4StrobeWidth == 0xFF) // high current
    {
        //strobeEndTime = (MUINT32)strobeTimer.getUsTime(); // strobe end time(us)
        //strobeEndTime = (MUINT32)strobeTimer.getUsTime(); // strobe end time(us)
        strobeEndTime = getMs();

        AAA_LOG("[strobeISPCallback] Update : MFPrevEndTime = %u\n",strobeEndTime);

        m_pmtk3A->setPrevMFEndTime(strobeEndTime);    //set previous main flash end time
    }
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setCaptureHDRParam(
    MUINT32 a_u4CntIdx,
    MINT32 a_IsRawGain       // 0 :  sensor shutter and gain,
)
{
    MINT32 err = MHAL_NO_ERROR;

    if (a_u4CntIdx > 2) {
        a_u4CntIdx = 0;
    } else {
        g_u4CaptureCnt = a_u4CntIdx;
    }

    // disable auto flicker
    setSensorFlickerFrameRate(FALSE);

    holdIspRegs(1);    //Vent@20120904: Fix "There is a band in the top of HDR image" issue.

    if (a_IsRawGain == 2) {    //  Pass2 by pass isp gain
        AAA_LOG("[setCaptureHDRParam] Cnt:%d IspGain:%d IspRawGain:%d\n", a_u4CntIdx, m_r3AOutput.rAEOutput.rCaptureMode[a_u4CntIdx].u4IspGain, a_IsRawGain);
        // Set RAW gain
        setIspRAWGain(128, TRUE);

        // Set flare
        setIspFlare(m_r3AOutput.rAEOutput.rCaptureMode[a_u4CntIdx].FlareCFG);
    } else if (a_IsRawGain == 1) {     // 1 : Pass1 isp gain
        AAA_LOG("[setCaptureHDRParam] Cnt:%d IspGain:%d IspRawGain:%d\n", a_u4CntIdx, m_r3AOutput.rAEOutput.rCaptureMode[a_u4CntIdx].u4IspGain, a_IsRawGain);
        // Set AWB gain
        setIspAWBGain(m_r3AOutput.rAWBOutput.rCaptureAWBGain, TRUE);
        // Set RAW gain
        setCaptureIspRAWGain((m_r3AOutput.rAEOutput.rCaptureMode[a_u4CntIdx].u4IspGain)>>3);
    } else {
        AAA_LOG("[setCaptureHDRParam] Cnt:%d, Expos:%d, Sensor Gain:%d Flare R:%d G:%d B:%d\n", a_u4CntIdx, m_r3AOutput.rAEOutput.rCaptureMode[a_u4CntIdx].u4Eposuretime,
                                                          m_r3AOutput.rAEOutput.rCaptureMode[a_u4CntIdx].u4AfeGain, m_r3AOutput.rAEOutput.rCaptureMode[a_u4CntIdx].FlareCFG.uFlare_R,
                                                          m_r3AOutput.rAEOutput.rCaptureMode[a_u4CntIdx].FlareCFG.uFlare_G, m_r3AOutput.rAEOutput.rCaptureMode[a_u4CntIdx].FlareCFG.uFlare_B);
        // Set exposure time and sensor gain
        setEShutterParam(m_r3AOutput.rAEOutput.rCaptureMode[a_u4CntIdx].u4Eposuretime, m_r3AOutput.rAEOutput.rCaptureMode[a_u4CntIdx].u4AfeGain);

        // Set flare
        setIspFlare(m_r3AOutput.rAEOutput.rCaptureMode[a_u4CntIdx].FlareCFG);

        // Set AWB gain
        setIspAWBGain(m_r3AOutput.rAWBOutput.rCaptureAWBGain, TRUE);
    }

    holdIspRegs(0);    //Vent@20120904: Fix "There is a band in the top of HDR image" issue.

    g_bHDRUpdate = FALSE;  // reset HDR mode
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setCaptureParam(
    MUINT32 a_u4IsRawInDram
)
{
    MINT32 err = MHAL_NO_ERROR;

    if (g_u4CaptureCnt > 2) {
        g_u4CaptureCnt = 0;
    }

    AAA_LOG("[setCaptureParam] Cnt:%d, IsDRAM:%d Expos:%d, Sensor Gain:%d IspGain:%d\n", g_u4CaptureCnt, a_u4IsRawInDram, m_r3AOutput.rAEOutput.rCaptureMode[g_u4CaptureCnt].u4Eposuretime,
    	                                                      m_r3AOutput.rAEOutput.rCaptureMode[g_u4CaptureCnt].u4AfeGain, m_r3AOutput.rAEOutput.rCaptureMode[g_u4CaptureCnt].u4IspGain);

    // disable auto flicker
    setSensorFlickerFrameRate(FALSE);

    if((g_bBypassSensorSetting == FALSE) && (a_u4IsRawInDram == 0)){
        // Set exposure time
        setSensorExpTime(m_r3AOutput.rAEOutput.rCaptureMode[g_u4CaptureCnt].u4Eposuretime);

        // Set sensor gain
        setSensorGain(m_r3AOutput.rAEOutput.rCaptureMode[g_u4CaptureCnt].u4AfeGain);
    } else {
        AAA_LOG("[setCaptureParam] g_bBypassSensorSetting:%d\n", g_bBypassSensorSetting);
    }

    // Set flare
    setIspFlare(m_r3AOutput.rAEOutput.rCaptureMode[g_u4CaptureCnt].FlareCFG);

    // Set AWB gain
    setIspAWBGain(m_r3AOutput.rAWBOutput.rCaptureAWBGain, FALSE);

    if (a_u4IsRawInDram) { // second path, image input from DRAM
        // Set RAW gain
        setIspRAWGain(128, FALSE);
    }
    else    // first path or on-the-fly mode
    {
        // Set RAW gain
        setCaptureIspRAWGain((m_r3AOutput.rAEOutput.rCaptureMode[g_u4CaptureCnt].u4IspGain)>>3);

        // Config capture flash
        if (m_pStrobeDrvObj)
        {
            //cotta--modified for strobe protection
            if(m_r3AOutput.rAEOutput.rCaptureMode[0].u4StrobeWidth != 0)    //strobe is going to turn on
            {
                AAA_LOG("[setCaptureParam] register strobe ISP callback, width=%u\n",m_r3AOutput.rAEOutput.rCaptureMode[0].u4StrobeWidth);

                //set ISP callback here
                m_pIspDrvObj->regCallback(setStrobeISPCallback);

                if(strobeZSDMode == 1)  // ZSD mode on
                {
                    AAA_LOG("[setCaptureParam] ZSD strobe\n");

                    if(m_r3AOutput.rAEOutput.rCaptureMode[0].u4StrobeWidth != 0xFF) // low current
                    {
                        m_pStrobeDrvObj->setCaptureFlashlightConf(m_r3AOutput.rAEOutput.rCaptureMode[0].u4StrobeWidth);
                    }
                    else
                    {
                        strobeOnISPCallback();
                    }
                }
            }
            else
            {
                AAA_LOG("[setCaptureParam] strobe off\n");
                m_pStrobeDrvObj->setCaptureFlashlightConf(m_r3AOutput.rAEOutput.rCaptureMode[0].u4StrobeWidth);
            }

            //--cotta
        }

        g_u4CaptureCnt++;

        setBestShotConfig();
    }
    g_bHDRUpdate = FALSE;  // reset HDR mode
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::set3AParam(
    MINT32 a_i4Param0,
    MINT32 a_i4Param1,
    MINT32 a_i4Param2,
    MINT32 a_i4Param3
)
{
    MINT32 err = MHAL_NO_ERROR;
    MINT32 aeFlashlightType = 0;

	MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_PARAM) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON) ||
        (a_i4Param0 != HAL_3A_AE_FLICKER_AUTO_MODE)) {
        AAA_LOG("[set3AParam] %d, 0x%x 0x%x\n", a_i4Param0, a_i4Param1, a_i4Param2);
    }

    switch (a_i4Param0) {
        case HAL_3A_AE_MODE:
            err = setAEMode(a_i4Param1);
            break;
        case HAL_3A_AE_EVCOMP:
            err = setAEEVCompMode(a_i4Param1);
            break;
        case HAL_3A_AE_METERING_MODE:
            err = setAEMeteringMode(a_i4Param1);
            break;
        case HAL_3A_AE_ISOSPEED:
            err = setAEISOSpeed(a_i4Param1);
            break;
        case HAL_3A_AE_STROBE_MODE:
            if (m_pStrobeDrvObj)
            {
                aeFlashlightType = (MINT32)m_pStrobeDrvObj->getFlashlightType();
                setAEFlashlightType(aeFlashlightType);
                if (FLASHLIGHT_LED_PEAK == (FLASHLIGHT_TYPE_ENUM)aeFlashlightType ||
                    FLASHLIGHT_LED_CONSTANT == (FLASHLIGHT_TYPE_ENUM)aeFlashlightType)
                {
                    AAA_LOG("[HAL_3A_AE_STROBE_MODE]line=%d setAEStrobeMode=%d\n",__LINE__, a_i4Param1);
                    err = setAEStrobeMode(a_i4Param1);
                    if(a_i4Param1 == LIB3A_AE_STROBE_MODE_FORCE_OFF)
                    {
                        AAA_LOG("line=%d [HAL_3A_AE_STROBE_MODE]:strobe off\n",__LINE__, a_i4Param1);
                        if(m_pStrobeDrvObj!=0)
                            m_pStrobeDrvObj->setFire(0);
                    }
                }
            }
            break;
        //case HAL_3A_AE_REDEYE_MODE:
        //    err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_REDEYE_MODE, a_i4Param1);
        //    break;
        case HAL_3A_AE_FLICKER_MODE:
            err = setAEFlickerMode(a_i4Param1);
            break;
        case HAL_3A_AE_FRAMERATE_MODE:
            err = setAEFrameRateMode(a_i4Param1);
            break;
        case HAL_3A_AE_FPS_MIN_MAX:
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_AE_FPS_MIN, a_i4Param1);
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_AE_FPS_MAX, a_i4Param2);
            break;
        case HAL_3A_AE_FLICKER_AUTO_MODE:  // for flicker detection algorithm used
            err = setAEFlickerAutoMode(a_i4Param1);
            break;
        case HAL_3A_AE_PREVIEW_MODE:    // default "0" is preview auto.
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_AE_PREVIEW_MODE, a_i4Param1);
            break;
        case HAL_3A_AF_MODE:
            err = setAFMode(a_i4Param1);
            break;
        case HAL_3A_AF_METERING_MODE:
            err = setAFMeteringMode(a_i4Param1);
            break;
        case HAL_3A_AF_METERING_POS: {
            mhalCamMoveSpotInfo_t *pmsInfo = (mhalCamMoveSpotInfo_t *) a_i4Param1;
            m_pmtk3A->setAFMoveSpotPos(
                pmsInfo->u4OffsetX, pmsInfo->u4OffsetY,
                pmsInfo->u4DispW, pmsInfo->u4DispH,
                pmsInfo->u4DispX, pmsInfo->u4DispY, pmsInfo->u4Rotate);
            }
            break;
        case HAL_3A_AWB_MODE:
            err = setAWBMode(a_i4Param1);
            break;
        default:
            err = MHAL_INVALID_PARA;
            break;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::get3AParam(
    MINT32 a_i4Param0,
    MINT32 *a_pParam1
)
{
    MINT32 err = MHAL_NO_ERROR;
    MINT32 i4AFMode;

    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    switch (a_i4Param0) {
        case HAL_3A_AE_MODE:
            err = m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, a_pParam1);
            break;
        case HAL_3A_AE_EVCOMP:
            err = m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, a_pParam1);
            break;
        case HAL_3A_AE_METERING_MODE:
            err = m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_METERING_MODE, a_pParam1);
            break;
        case HAL_3A_AE_ISOSPEED:
            err = m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, a_pParam1);
            break;
        case HAL_3A_AE_STROBE_MODE:
            err = m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_STROBE_MODE, a_pParam1);
            break;
        case HAL_3A_AE_REDEYE_MODE:
            err = m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_REDEYE_MODE, a_pParam1);
            break;
        case HAL_3A_AE_FLICKER_MODE:
            err = m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_MODE, a_pParam1);
            break;
        case HAL_3A_AE_FRAMERATE_MODE:
            err = m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_FRAMERATE_MODE, a_pParam1);
            break;
        case HAL_3A_AE_FPS_MIN_MAX:
            err = m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_AE_FPS_MIN, a_pParam1);
            err |= m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_AE_FPS_MAX, a_pParam1+1);
            break;
        case HAL_3A_AE_SUPPORT_FPS_NUM:
            err = m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_AE_SUPPORT_FPS_NUM, a_pParam1);
            break;
        case HAL_3A_AE_SUPPORT_FPS_RANGE:
            err = m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_AE_SUPPORT_FPS_RANGE, a_pParam1);
            break;
        case HAL_3A_AE_FLICKER_AUTO_MODE:  // for flicker detection algorithm used
            err = m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_AUTO_MODE , a_pParam1);
            break;
        case HAL_3A_AE_PREVIEW_MODE:    // default "0" is preview auto.
            err = m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_AE_PREVIEW_MODE , a_pParam1);
            break;
   		case HAL_3A_AF_MODE:
			err = m_pmtk3A->get3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, a_pParam1);

            i4AFMode = *a_pParam1;

            switch (i4AFMode) {
                case LIB3A_AF_MODE_AFS: // AF-Single Shot Mode
                    *a_pParam1 = AF_MODE_AFS;
                    break;
                case LIB3A_AF_MODE_AFC: // AF-Continuous Mode
                    *a_pParam1 = AF_MODE_AFC;
                    break;
                case LIB3A_AF_MODE_AFC_VIDEO: // AF-Continuous Mode (Video)
                    *a_pParam1 = AF_MODE_AFC_VIDEO;
			break;
                case LIB3A_AF_MODE_MACRO: // AF Macro Mode
                    *a_pParam1 = AF_MODE_MACRO;
                    break;
                case LIB3A_AF_MODE_INFINITY: // Focus is set at infinity
                    *a_pParam1 = AF_MODE_INFINITY;
                    break;
                case LIB3A_AF_MODE_MF: // Manual Focus Mode
                    *a_pParam1 = AF_MODE_MF;
                    break;
                case LIB3A_AF_MODE_FULLSCAN: // AF Full Scan Mode
                    *a_pParam1 = AF_MODE_FULLSCAN;
                    break;
                default:
                    *a_pParam1 = AF_MODE_AFS;
                    break;
            }

            //AAA_LOG("[get3AParam]getAFMode = %d\n", *a_pParam1);

			break;
		case HAL_3A_AF_METERING_MODE:
			err = m_pmtk3A->get3ACmd(LIB3A_AF_CMD_ID_SET_AF_METER, a_pParam1);
			break;
        case HAL_3A_AWB_MODE:
            err = m_pmtk3A->get3ACmd(LIB3A_AWB_CMD_ID_SET_AWB_MODE, a_pParam1);
            break;
        default:
            err = MHAL_INVALID_PARA;
            break;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::enableAE()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->enableAE();

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::disableAE()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->disableAE();

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::enableAWB()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->enableAWB();

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::disableAWB()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->disableAWB();

    return MHAL_NO_ERROR;
}



/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::enableAF()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->enableAF();

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::disableAF()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->disableAF();

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getAFDone()
{
    return m_r3AOutput.bAFDone;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getAFBestPos()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

	return m_pmtk3A->getAFBestPos();
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::isAFFinish(
)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    return m_pmtk3A->isAFFinish();
}

/////////////////////////////////////////////////////////////////////////
//
// setDoFocusState () -
//! \brief
//
/////////////////////////////////////////////////////////////////////////
void
Hal3A::setDoFocusState()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->setDoFocusState();
}


/////////////////////////////////////////////////////////////////////////
//
// pause Focus () -
//! \brief
//
/////////////////////////////////////////////////////////////////////////
void
Hal3A::pauseFocus()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->pauseFocus();
}

/////////////////////////////////////////////////////////////////////////
//
// isFocused () -
//! \brief
//
/////////////////////////////////////////////////////////////////////////
MBOOL
Hal3A::isFocused()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    return m_pmtk3A->isFocused();
}

/////////////////////////////////////////////////////////////////////////
//
// resetFocus () -
//! \brief
//
/////////////////////////////////////////////////////////////////////////
void
Hal3A::resetFocus()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->resetFocus();
}

/////////////////////////////////////////////////////////////////////////
//
// setFocusAreas () -
//! \brief
//
/////////////////////////////////////////////////////////////////////////
void
Hal3A::setFocusAreas(MINT32 a_i4Cnt, AREA_T *a_psFocusArea)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->setFocusAreas(a_i4Cnt, a_psFocusArea);
}

/////////////////////////////////////////////////////////////////////////
//
// getFocusAreas () -
//! \brief
//
/////////////////////////////////////////////////////////////////////////
void
Hal3A::getFocusAreas(MINT32 &a_i4Cnt, AREA_T **a_psFocusArea)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->getFocusAreas(a_i4Cnt, a_psFocusArea);
}

/////////////////////////////////////////////////////////////////////////
//
// getMaxNumFocusAreas () -
//! \brief
//
/////////////////////////////////////////////////////////////////////////
MINT32
Hal3A::getMaxNumFocusAreas()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    return m_pmtk3A->getMaxNumFocusAreas();
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::lockHalfPushAEAWB(
    MBOOL a_bLockAEAWB
)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    if (a_bLockAEAWB) {
        m_pmtk3A->lockHalfPushAEAWB();
    }
    else {
        m_pmtk3A->unlockHalfPushAEAWB();
    }

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setEZoomInfo(
    MUINT32 a_u4XOffset,
    MUINT32 a_u4YOffset,
    MUINT32 a_u4Width,
    MUINT32 a_u4Height
)
{
    if ((a_u4Width != 0) && (a_u4Height != 0)) {
        m_rEZoomInfo.u4XOffset = a_u4XOffset;
        m_rEZoomInfo.u4YOffset = a_u4YOffset;
        m_rEZoomInfo.u4XWidth = a_u4Width;
        m_rEZoomInfo.u4YHeight = a_u4Height;

        return MHAL_NO_ERROR;
    }

    return MHAL_INVALID_PARA;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setFDInfo(
    MUINT32 a_u4IsFDOn,
    MUINT32 a_u4Addr
)
{
    m_rFDInfo.bFDon  = (MBOOL)a_u4IsFDOn;

    if (m_rFDInfo.bFDon == TRUE) {
        if (a_u4Addr == 0) {
            AAA_ERR("setFDInfo() error: NULL pointer");
            return MHAL_INVALID_PARA;
        }

        camera_face_metadata_m *presult = (camera_face_metadata_m *) a_u4Addr;

        for (int i = 0; i < MAX_FACE_NUM; i++) {
            if (presult->faces[i].score == 100) {
                m_rFDInfo.sWin[i].fgPrimary = 1;
            }
            else  {
                m_rFDInfo.sWin[i].fgPrimary = 0;
            }
            m_rFDInfo.sWin[i].i4Left    = presult->faces[i].rect[0] + 1000;
            m_rFDInfo.sWin[i].i4Up      = presult->faces[i].rect[1] + 1000;
            m_rFDInfo.sWin[i].i4Right   = presult->faces[i].rect[2] + 1000;
            m_rFDInfo.sWin[i].i4Bottom  = presult->faces[i].rect[3] + 1000;
        }

        m_rFDInfo.i4Width = 2000;
        m_rFDInfo.i4Height = 2000;
    }

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::drawFocusRect(
    UINT8 *a_pBuf,
    MUINT32 a_u4DispW,
    MUINT32 a_u4DispH,
    MUINT32 a_u4DispX,
    MUINT32 a_u4DispY,
    MUINT32 a_u4Rotate
)
{
	MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->AFDrawRect((MUINT32) a_pBuf, a_u4DispW, a_u4DispH, a_u4DispX, a_u4DispY, a_u4Rotate);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getDebugInfo(
    MVOID **a_p3ADebugInfo,
    MUINT32 *a_u4SizeInByte
)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->get3ADebugInfo(m_r3ADebugInfo);

    *a_p3ADebugInfo = (MVOID *)&m_r3ADebugInfo;
    *a_u4SizeInByte = sizeof(AAA_DEBUG_INFO_T);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
Hal3A::enableBestShot(
)
{
    g_bBestShotEnable = TRUE;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
Hal3A::disableBestShot(
)
{
    g_bBestShotEnable = FALSE;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
Hal3A::bestShotProcess(
)
{
    MINT32 err;
    AAA_STAT_T r3AStat;

    memset(&r3AStat,   0, sizeof(r3AStat));

    if (g_bBestShotEnable == TRUE) {
        // Get 3A statistics
        getIsp3AStat(r3AStat);

#ifdef AAA_BEST_SHOT_LOG
		AAA_LOG("[BestShot]%11u\n"	, r3AStat.rAFStat.sWin[0].u4FV[0]);
		AAA_LOG("[BestShot]%11u\n"	, r3AStat.rAFStat.sWin[1].u4FV[0]);
		AAA_LOG("[BestShot]%11u\n"	, r3AStat.rAFStat.sWin[2].u4FV[0]);
		AAA_LOG("[BestShot]%11u\n" , r3AStat.rAFStat.sWin[3].u4FV[0]);
		AAA_LOG("[BestShot]%11u\n"	, r3AStat.rAFStat.sWin[4].u4FV[0]);
		AAA_LOG("\n");
		AAA_LOG("\n");
#endif
    }

    g_u4BestShotValue = r3AStat.rAFStat.sWin[0].u4FV[0];
}

/*******************************************************************************
*
********************************************************************************/
MUINT32
Hal3A::getBestShotValue(
)
{
    return g_u4BestShotValue;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
Hal3A::enableAFLamp()
{
    m_i4AFLampMode = HAL3A_AF_LAMP_MODE_ON;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
Hal3A::autoAFLamp()
{
    m_i4AFLampMode = HAL3A_AF_LAMP_MODE_AUTO;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
Hal3A::disableAFLamp()
{
    m_i4AFLampMode = HAL3A_AF_LAMP_MODE_OFF;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getReadyForCapture(
)
{
    return m_r3AOutput.bReadyForCapture;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::get3AEXIFInfo(
    MVOID *a_p3AEXIFInfo
)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->get3AEXIFInfo((AAA_EXIF_INFO_T *)a_p3AEXIFInfo);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::resetHalfPushState(
)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->resetHalfPushState();

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getAFWinResult(
    MINT32 *a_pBuf,
    MINT32 *a_pWinW,
    MINT32 *a_pWinH
)
{
    AF_WIN_RESULT_T sAFWinResult;

    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->getAFWinResult(sAFWinResult);

	MHAL_ASSERT(sAFWinResult.i4Count <= 5, "Err");

    if (getAFDone()) {
        sAFWinResult.i4Count = m_rAFWinResult.i4Count;
        sAFWinResult.i4Width = m_rAFWinResult.i4Width;
        sAFWinResult.i4Height = m_rAFWinResult.i4Height;

        for (int i = 0; i < sAFWinResult.i4Count; i++) {
            sAFWinResult.sRect[i].i4Left   = m_rAFWinResult.sRect[i].i4Left;
            sAFWinResult.sRect[i].i4Up     = m_rAFWinResult.sRect[i].i4Up;
            sAFWinResult.sRect[i].i4Right  = m_rAFWinResult.sRect[i].i4Right;
            sAFWinResult.sRect[i].i4Bottom = m_rAFWinResult.sRect[i].i4Bottom;
        }
    }

    m_rAFWinResult = sAFWinResult;

    for (int i = 0; i < sAFWinResult.i4Count; i++) {
        *a_pBuf++ = sAFWinResult.sRect[i].i4Left;
        *a_pBuf++ = sAFWinResult.sRect[i].i4Up;
        *a_pBuf++ = sAFWinResult.sRect[i].i4Right;
        *a_pBuf++ = sAFWinResult.sRect[i].i4Bottom;
        *a_pBuf++ = sAFWinResult.sRect[i].eMarkStatus;
    }

    *a_pWinW = sAFWinResult.i4Width;
    *a_pWinH = sAFWinResult.i4Height;

    return sAFWinResult.i4Count;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setAFFullStep(
    MINT32 a_i4Step
)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->setAFFullStep(a_i4Step);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setMFPos(
    MINT32 a_i4MFPos
)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

	m_pmtk3A->setMFPos(a_i4MFPos);

	return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setFocusDistanceRange(MINT32 a_i4Distance_N, MINT32 a_i4Distance_M)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

	m_pmtk3A->setFocusDistanceRange(a_i4Distance_N, a_i4Distance_M);

	return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getFocusDistance(MINT32 &a_i4Near, MINT32 &a_i4Curr, MINT32 &a_i4Far)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

	m_pmtk3A->getFocusDistance(a_i4Near, a_i4Curr, a_i4Far);

	return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getISO(
    MINT32 a_i4Mode,
    MINT32 *a_pISO
)
{
    if (a_i4Mode == 0) { // preview mode
        *a_pISO = m_r3AOutput.rAEOutput.rPreviewMode.u4ISOSpeed;
    }
	else { // capture mode
	    *a_pISO = m_r3AOutput.rAEOutput.rCaptureMode[g_u4CaptureCnt].u4ISOSpeed;
	}

	return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getCCT(
    MINT32 a_i4Mode,
    MVOID *a_pCCT
)
{
    AWB_CCT_T *pAWBCCT = (AWB_CCT_T *)a_pCCT;

    if (a_i4Mode == 0) { // preview mode
        *pAWBCCT = m_r3AOutput.rAWBOutput.rCCT;
    }
	else { // capture mode
	    *pAWBCCT = m_r3AOutput.rAWBOutput.rCCT;
	}

	return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getLV(
    MINT32 *a_pLV
)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

	*a_pLV = m_pmtk3A->getSceneLV();

	return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getAEPlineEV(
    MINT32 *a_pLV
)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

	*a_pLV = m_pmtk3A->getAEPlineEV();

	return MHAL_NO_ERROR;
}

MINT32
Hal3A::getHDRCapInfo(Hal3A_HDROutputParam_T & strHDROutputInfo)
{
       strHDROutputInfo = m_strHDROutputInfo;
       AAA_LOG("[getHDRCapInfo] OutputFrameNum:%d FinalGainDiff[0]:%d FinalGainDiff[1]:%d TargetTone:%d\n", strHDROutputInfo.u4OutputFrameNum, strHDROutputInfo.u4FinalGainDiff[0], strHDROutputInfo.u4FinalGainDiff[1], m_strHDROutputInfo.u4TargetTone);
	return MHAL_NO_ERROR;
}

MINT32
Hal3A::updateCapParamsByHDR()
{
    MUINT8 i;
    MINT32 i4AEMode;
    AAA_STAT_T r3AStat;
    NSCamCustom::HDRExpSettingInputParam_T strHDRInputSetting;
    NSCamCustom::HDRExpSettingOutputParam_T strHDROutputSetting;
    AE_EXP_GAIN_MODIFY_T	rSensorInputData, rSensorOutputData;

    if(m_pmtk3A != NULL) {
        m_pmtk3A->get3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, &i4AEMode);
        if(i4AEMode == LIB3A_AE_MODE_HDR) {   // in HDR mode
            memset(&r3AStat,   0, sizeof(r3AStat));
            getIsp3AStat(r3AStat);
            strHDRInputSetting.u4MaxSensorAnalogGain = 3*m_r3ANVRAMData.rAENVRAM.rDevicesInfo.u4MaxGain;
            strHDRInputSetting.u4MaxAEExpTimeInUS = 500000; // 0.5sec
            strHDRInputSetting.u4MinAEExpTimeInUS = 500;  // 500us
            if(m_r3ANVRAMData.rAENVRAM.rDevicesInfo.u4CapExpUnit < 10000) {
                strHDRInputSetting.u4ShutterLineTime = 1000*m_r3ANVRAMData.rAENVRAM.rDevicesInfo.u4CapExpUnit;
            } else {
                strHDRInputSetting.u4ShutterLineTime = m_r3ANVRAMData.rAENVRAM.rDevicesInfo.u4CapExpUnit;
            }
            strHDRInputSetting.u4MaxAESensorGain = 3*m_r3ANVRAMData.rAENVRAM.rDevicesInfo.u4MaxGain;
            strHDRInputSetting.u4MinAESensorGain = m_r3ANVRAMData.rAENVRAM.rDevicesInfo.u4MinGain;
            strHDRInputSetting.u4ExpTimeInUS0EV = m_r3AOutput.rAEOutput.rCaptureMode[0].u4Eposuretime;
            strHDRInputSetting.u4SensorGain0EV = (m_r3AOutput.rAEOutput.rCaptureMode[0].u4AfeGain)*(m_r3AOutput.rAEOutput.rCaptureMode[0].u4IspGain) >>10;;
            strHDRInputSetting.u1FlareOffset0EV = m_r3AOutput.rAEOutput.rCaptureMode[0].FlareCFG.uFlare_B;
            for (i = 0; i < HAL_3A_HISTOGRAM_BIN_NUM; i++) {
                 strHDRInputSetting.u4Histogram[i] = r3AStat.rAEStat.u4AEHistogram[i];
            }
            AAA_LOG("[updateCapParamsByHDR] Input MaxSensorAnalogGain:%d MaxExpTime:%d MinExpTime:%d LineTime:%d MaxSensorGain:%d ExpTime:%d SensorGain:%d FlareOffset:%d\n",
            	           strHDRInputSetting.u4MaxSensorAnalogGain, strHDRInputSetting.u4MaxAEExpTimeInUS, strHDRInputSetting.u4MinAEExpTimeInUS, strHDRInputSetting.u4ShutterLineTime,
            	           strHDRInputSetting.u4MaxAESensorGain, strHDRInputSetting.u4ExpTimeInUS0EV, strHDRInputSetting.u4SensorGain0EV, strHDRInputSetting.u1FlareOffset0EV);
            for (i = 0; i < HAL_3A_HISTOGRAM_BIN_NUM; i+=8) {
                AAA_LOG("[updateCapParamsByHDR] Input Histogram%d~%d:%d %d %d %d %d %d %d %d\n", i, i+7, strHDRInputSetting.u4Histogram[i],
            	           strHDRInputSetting.u4Histogram[i+1], strHDRInputSetting.u4Histogram[i+2], strHDRInputSetting.u4Histogram[i+3], strHDRInputSetting.u4Histogram[i+4],
            	           strHDRInputSetting.u4Histogram[i+5], strHDRInputSetting.u4Histogram[i+6], strHDRInputSetting.u4Histogram[i+7]);
            }
            NSCamCustom::getHDRExpSetting(strHDRInputSetting, strHDROutputSetting);
            m_strHDROutputInfo.u4OutputFrameNum = strHDROutputSetting.u4OutputFrameNum;
            for(i=0; i<m_strHDROutputInfo.u4OutputFrameNum; i++) {
            	  rSensorInputData.u4SensorExpTime = strHDROutputSetting.u4ExpTimeInUS[i];
            	  rSensorInputData.u4SensorGain = strHDROutputSetting.u4SensorGain[i];
            	  rSensorInputData.u4IspGain = 1024;
            	  m_pmtk3A->switchSensorExposureGain(rSensorInputData, rSensorOutputData);   // send to 3A to calculate the exposure time and gain
                m_r3AOutput.rAEOutput.rCaptureMode[i].u4Eposuretime = rSensorOutputData.u4SensorExpTime;
                m_r3AOutput.rAEOutput.rCaptureMode[i].u4AfeGain = rSensorOutputData.u4SensorGain;
                m_r3AOutput.rAEOutput.rCaptureMode[i].u4IspGain = rSensorOutputData.u4IspGain;
                m_r3AOutput.rAEOutput.rCaptureMode[i].u4ISOSpeed = rSensorOutputData.u4ISOSpeed;
                m_r3AOutput.rAEOutput.rCaptureMode[i].FlareCFG.uFlare_B = strHDROutputSetting.u1FlareOffset[i];
                m_r3AOutput.rAEOutput.rCaptureMode[i].FlareCFG.uFlare_G = strHDROutputSetting.u1FlareOffset[i];
                m_r3AOutput.rAEOutput.rCaptureMode[i].FlareCFG.uFlare_R = strHDROutputSetting.u1FlareOffset[i];
            }
            m_strHDROutputInfo.u4FinalGainDiff[0] = strHDROutputSetting.u4FinalGainDiff[0];
            m_strHDROutputInfo.u4FinalGainDiff[1] = strHDROutputSetting.u4FinalGainDiff[1];
            m_strHDROutputInfo.u4TargetTone = strHDROutputSetting.u4TargetTone;
            g_bHDRUpdate = TRUE;

            AAA_LOG("[updateCapParamsByHDR] OutputFrameNum : %d FinalGainDiff[0]:%d FinalGainDiff[1]:%d TargetTone:%d\n", m_strHDROutputInfo.u4OutputFrameNum, m_strHDROutputInfo.u4FinalGainDiff[0], m_strHDROutputInfo.u4FinalGainDiff[1], m_strHDROutputInfo.u4TargetTone);
            AAA_LOG("[updateCapParamsByHDR] HDR Exposuretime[0] : %d Gain[0]:%d Flare[0]:%d\n", strHDROutputSetting.u4ExpTimeInUS[0], strHDROutputSetting.u4SensorGain[0], strHDROutputSetting.u1FlareOffset[0]);
            AAA_LOG("[updateCapParamsByHDR] HDR Exposuretime[1] : %d Gain[1]:%d Flare[1]:%d\n", strHDROutputSetting.u4ExpTimeInUS[1], strHDROutputSetting.u4SensorGain[1], strHDROutputSetting.u1FlareOffset[1]);
            AAA_LOG("[updateCapParamsByHDR] HDR Exposuretime[2] : %d Gain[2]:%d Flare[2]:%d\n", strHDROutputSetting.u4ExpTimeInUS[2], strHDROutputSetting.u4SensorGain[2], strHDROutputSetting.u1FlareOffset[2]);

            AAA_LOG("[updateCapParamsByHDR] Modify Exposuretime[0] : %d AfeGain[0]:%d IspGain[0]:%d ISO:%d\n", m_r3AOutput.rAEOutput.rCaptureMode[0].u4Eposuretime,
            	    m_r3AOutput.rAEOutput.rCaptureMode[0].u4AfeGain, m_r3AOutput.rAEOutput.rCaptureMode[0].u4IspGain, m_r3AOutput.rAEOutput.rCaptureMode[0].u4ISOSpeed);
            AAA_LOG("[updateCapParamsByHDR] Modify Exposuretime[1] : %d AfeGain[1]:%d IspGain[1]:%d ISO:%d\n", m_r3AOutput.rAEOutput.rCaptureMode[1].u4Eposuretime,
            	    m_r3AOutput.rAEOutput.rCaptureMode[1].u4AfeGain, m_r3AOutput.rAEOutput.rCaptureMode[1].u4IspGain, m_r3AOutput.rAEOutput.rCaptureMode[1].u4ISOSpeed);
            AAA_LOG("[updateCapParamsByHDR] Modify Exposuretime[2] : %d AfeGain[2]:%d IspGain[2]:%d ISO:%d\n", m_r3AOutput.rAEOutput.rCaptureMode[2].u4Eposuretime,
            	    m_r3AOutput.rAEOutput.rCaptureMode[2].u4AfeGain, m_r3AOutput.rAEOutput.rCaptureMode[2].u4IspGain, m_r3AOutput.rAEOutput.rCaptureMode[2].u4ISOSpeed);
        }
    }
    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3A::getASDInfo(
    AAA_ASD_INFO_T &a_ASDInfo
)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

	m_pmtk3A->getASDInfo(a_ASDInfo);
#if 0
    AAA_LOG("[i4AELv_x10] %d\n", a_ASDInfo.i4AELv_x10);
    AAA_LOG("[bAEBacklit] %d\n", a_ASDInfo.bAEBacklit);
    AAA_LOG("[bAEStable] %d\n", a_ASDInfo.bAEStable);
    AAA_LOG("[i4AWBRgain_X128] %d\n", a_ASDInfo.i4AWBRgain_X128);
    AAA_LOG("[i4AWBBgain_X128] %d\n", a_ASDInfo.i4AWBBgain_X128);
    AAA_LOG("[i4AWBRgain_D65_X128] %d\n", a_ASDInfo.i4AWBRgain_D65_X128);
    AAA_LOG("[i4AWBBgain_D65_X128] %d\n", a_ASDInfo.i4AWBBgain_D65_X128);
    AAA_LOG("[i4AWBRgain_CWF_X128] %d\n", a_ASDInfo.i4AWBRgain_CWF_X128);
    AAA_LOG("[i4AWBBgain_CWF_X128] %d\n", a_ASDInfo.i4AWBBgain_CWF_X128);
    AAA_LOG("[bAWBStable] %d\n", a_ASDInfo.bAWBStable);
    AAA_LOG("[pAFTable] %d\n", a_ASDInfo.pAFTable);
    AAA_LOG("[pAFTable[0]] %d\n", ((MINT32 *)a_ASDInfo.pAFTable)[0]);
    AAA_LOG("[pAFTable[1]] %d\n", ((MINT32 *)a_ASDInfo.pAFTable)[1]);
    AAA_LOG("[i4AFTableMacroIdx] %d\n", a_ASDInfo.i4AFTableMacroIdx);
    AAA_LOG("[i4AFTableIdxNum] %d\n", a_ASDInfo.i4AFTableIdxNum);
    AAA_LOG("[bAFStable] %d\n", a_ASDInfo.bAFStable);
    AAA_LOG("[i4AFTableOffset] %d\n", a_ASDInfo.i4AFTableOffset);
    AAA_LOG("[i4AFPos] %d\n", a_ASDInfo.i4AFPos);
#endif
    return MHAL_NO_ERROR;
}








/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getAFValue()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

	return m_pmtk3A->getAFValue();
}

/*******************************************************************************
*
********************************************************************************/
MVOID
Hal3A::getAFValueAll(AF_STAT_T &a_rAFStat)
{
    AAA_STAT_T r3AStat;

    getIsp3AStat(r3AStat);

    a_rAFStat = r3AStat.rAFStat;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
Hal3A::getEZoomInfo(
    EZOOM_WIN_T &a_eZoomInfo
)
{
    a_eZoomInfo.u4XOffset = m_rEZoomInfo.u4XOffset;
    a_eZoomInfo.u4YOffset = m_rEZoomInfo.u4YOffset;
    a_eZoomInfo.u4XWidth = m_rEZoomInfo.u4XWidth;
    a_eZoomInfo.u4YHeight = m_rEZoomInfo.u4YHeight;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getFDInfo(
    FD_INFO_T &a_sFDInfo
)
{
    a_sFDInfo = m_rFDInfo;

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setExpParam(
    MUINT32 a_u4ExpTime,
    MUINT32 a_u4SensorGain,
    MUINT32 a_u4RawGain
)
{
    MINT32 err;
    MUINT32 u4ExpData[5];

    if (m_bLockExposureSetting) {
        return MHAL_NO_ERROR;
    }

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_SENSOR) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("setExpParam(): a_u4ExpTime = %d; a_u4SensorGain = %d; a_u4RawGain = %d\n", a_u4ExpTime, a_u4SensorGain, a_u4RawGain);
	AAA_LOG("setExpParam(): g_rSensorInfo.AEShutDelayFrame = %d; g_rSensorInfo.AESensorGainDelayFrame = %d; g_rSensorInfo.AEISPGainDelayFrame = %d\n", g_rSensorInfo.AEShutDelayFrame, g_rSensorInfo.AESensorGainDelayFrame, g_rSensorInfo.AEISPGainDelayFrame);
    }

    if ((a_u4ExpTime == 0) || (a_u4SensorGain == 0) || (a_u4RawGain == 0)) {
        AAA_ERR("setExpParam() error: a_u4ExpTime = %d; a_u4SensorGain = %d; a_u4RawGain = %d\n", a_u4ExpTime, a_u4SensorGain, a_u4RawGain);
        return MHAL_INVALID_PARA;
    }

    MUINT32 u4OBLevel_R = ISP_BITS(m_pIspReg, CAM_RGBOFF, OFF11);
    MUINT32 u4OBLevel_Gr = ISP_BITS(m_pIspReg, CAM_RGBOFF, OFF10);
    MUINT32 u4OBLevel_Gb = ISP_BITS(m_pIspReg, CAM_RGBOFF, OFF01);
    MUINT32 u4OBLevel_B = ISP_BITS(m_pIspReg, CAM_RGBOFF, OFF00);

    MUINT32 u4DefaultISPRAWGain_R =  (128 * 1024 + ((1024 - u4OBLevel_R) >> 1)) / (1024 - u4OBLevel_R);
    MUINT32 u4DefaultISPRAWGain_Gr =  (128 * 1024 + ((1024 - u4OBLevel_Gr) >> 1)) / (1024 - u4OBLevel_Gr);
    MUINT32 u4DefaultISPRAWGain_Gb =  (128 * 1024 + ((1024 - u4OBLevel_Gb) >> 1)) / (1024 - u4OBLevel_Gb);
    MUINT32 u4DefaultISPRAWGain_B =  (128 * 1024 + ((1024 - u4OBLevel_B) >> 1)) / (1024 - u4OBLevel_B);

    // Set raw gain: a_u4RawGain is 10-bit
    MUINT32 u4RAWGain_R = (UINT16)((u4DefaultISPRAWGain_R * a_u4RawGain + (a_u4RawGain >> 1)) >> 7);
    MUINT32 u4RAWGain_Gr = (UINT16)((u4DefaultISPRAWGain_Gr * a_u4RawGain + (a_u4RawGain >> 1)) >> 7);
    MUINT32 u4RAWGain_Gb = (UINT16)((u4DefaultISPRAWGain_Gb * a_u4RawGain + (a_u4RawGain >> 1)) >> 7);
    MUINT32 u4RAWGain_B = (UINT16)((u4DefaultISPRAWGain_B * a_u4RawGain + (a_u4RawGain >> 1)) >> 7);

    if ((u4RAWGain_R < 128) ||
        (u4RAWGain_R > 511) ||
        (u4RAWGain_Gr < 128) ||
        (u4RAWGain_Gr > 511) ||
        (u4RAWGain_Gb < 128) ||
        (u4RAWGain_Gb > 511) ||
        (u4RAWGain_B < 128) ||
        (u4RAWGain_B > 511)) {
        AAA_ERR("setExpParam() error: u4RAWGain_R = %d, u4RAWGain_Gr = %d, u4RAWGain_Gb = %d, u4RAWGain_B = %d \n",
            u4RAWGain_R, u4RAWGain_Gr, u4RAWGain_Gb, u4RAWGain_B);
        return MHAL_INVALID_PARA;
    }

#if 1
    u4ExpData[0] = (u4RAWGain_R) | (u4RAWGain_Gr << 16);
	u4ExpData[1] = (u4RAWGain_Gb) | (u4RAWGain_B << 16);
	u4ExpData[2] = a_u4ExpTime;
    u4ExpData[3] = a_u4SensorGain;

//    if((m_e3AState == AAA_STATE_AF) || (m_bCancelAF == TRUE)){
    u4ExpData[4] = g_rSensorInfo.AEShutDelayFrame |
		          (g_rSensorInfo.AESensorGainDelayFrame << 8) |
		          ((g_rSensorInfo.AEISPGainDelayFrame) << 16);
//    m_bCancelAF = FALSE; // for cancel AF condition
//    } else {
//	u4ExpData[4] = g_rSensorInfo.AEShutDelayFrame |
//		          (g_rSensorInfo.AESensorGainDelayFrame << 8) |
//		          (g_rSensorInfo.AEISPGainDelayFrame << 16);
//    }
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_SENSOR_SYNC, &u4ExpData[0], NULL, NULL);
	MHAL_ASSERT(err == MHAL_NO_ERROR, "Err SENSOR_FEATURE_SET_SENSOR_SYNC");
#else
    setIspRAWGain(u4RAWGain, TRUE);
    setSensorExpTime(a_u4ExpTime);
    setSensorGain(a_u4SensorGain);
#endif

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setEShutterParam(
    MUINT32 a_u4ExpTime,
    MUINT32 a_u4SensorGain
)
{
    MINT32 err;
    MUINT32 u4ExpData[2];

    if (m_bLockExposureSetting) {
        return MHAL_NO_ERROR;
    }

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_SENSOR) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("setExpParam(): a_u4ExpTime = %d; a_u4SensorGain = %d; \n", a_u4ExpTime, a_u4SensorGain);
    }

    if ((a_u4ExpTime == 0) || (a_u4SensorGain == 0)) {
        AAA_ERR("setExpParam() error: a_u4ExpTime = %d; a_u4SensorGain = %d; \n", a_u4ExpTime, a_u4SensorGain);
        return MHAL_INVALID_PARA;
    }

    u4ExpData[0] = a_u4ExpTime;
    u4ExpData[1] = a_u4SensorGain;

    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_ESHUTTER_GAIN, &u4ExpData[0], NULL, NULL);
	MHAL_ASSERT(err == MHAL_NO_ERROR, "Err CMD_SENSOR_SET_ESHUTTER_GAIN");

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setBestShotConfig(
)
{
    MINT32 err = MHAL_NO_ERROR;

    if (g_bBestShotEnable == TRUE) {
        // Get sensor info
        MUINT32 capW, capH;
        ACDK_SENSOR_RESOLUTION_INFO_STRUCT rSensorResolution;
        memset(&rSensorResolution, 0, sizeof(ACDK_SENSOR_RESOLUTION_INFO_STRUCT));
        m_pSensorDrvObj->getResolution(&rSensorResolution);
        capW = rSensorResolution.SensorFullWidth;
        capH = rSensorResolution.SensorFullHeight;

        //halSensorGetFullRes(&capW, &capH);

        if (capW > 4080)	{capW = 4080;}
	    if (capH > 4080)	{capH = 4080;}

	    UINT8 x1 = (UINT8)(capW/160);
		UINT8 x2 = x1*9;

		UINT8 y1 = (UINT8)(capH/160);
		UINT8 y2 = y1*9;

		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uLeft   = x1;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uRight  = x2;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uUp	   = y1;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uBottom = y2;

		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[1].uLeft   = 0;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[1].uRight  = 0;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[1].uUp	   = 0;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[1].uBottom = 0;

		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[2].uLeft   = 0;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[2].uRight  = 0;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[2].uUp	   = 0;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[2].uBottom = 0;

		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[3].uLeft   = 0;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[3].uRight  = 0;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[3].uUp	   = 0;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[3].uBottom = 0;

		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[4].uLeft   = 0;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[4].uRight  = 0;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[4].uUp	   = 0;
		m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[4].uBottom = 0;

		m_r3AStatConfig.rAFStatConfig.bEn = TRUE;
		m_r3AStatConfig.rAFStatConfig.bSel = TRUE;
		m_r3AStatConfig.rAFStatConfig.uTH[0] = 2;
		m_r3AStatConfig.rAFStatConfig.uTH[1] = 0;
		m_r3AStatConfig.rAFStatConfig.uTH[2] = 0;
		m_r3AStatConfig.rAFStatConfig.uTH[3] = 0;
		m_r3AStatConfig.rAFStatConfig.uTH[4] = 0;

#ifdef AAA_BEST_SHOT_LOG
		AAA_LOG("[BestShot Config]%4u, %4u, %4u\n"
				, m_r3AStatConfig.rAFStatConfig.bEn
				, m_r3AStatConfig.rAFStatConfig.bSel
				, m_r3AStatConfig.rAFStatConfig.uTH[0]);

		AAA_LOG("[BestShot Config]%4u, %4u, %4u, %4u\n"
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uLeft
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uRight
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uUp
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uBottom);

		AAA_LOG("[BestShot Config]%4u, %4u, %4u, %4u\n"
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[1].uLeft
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[1].uRight
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[1].uUp
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[1].uBottom);

		AAA_LOG("[BestShot Config]%4u, %4u, %4u, %4u\n"
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[2].uLeft
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[2].uRight
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[2].uUp
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[2].uBottom);

		AAA_LOG("[BestShot Config]%4u, %4u, %4u, %4u\n"
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[3].uLeft
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[3].uRight
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[3].uUp
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[3].uBottom);

		AAA_LOG("[BestShot Config]%4u, %4u, %4u, %4u\n"
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[4].uLeft
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[4].uRight
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[4].uUp
				, m_r3AStatConfig.rAFStatConfig.sAFWin.sWin[4].uBottom);

		AAA_LOG("\n");
		AAA_LOG("\n");
#endif

	    // Set 3A statistics config
        setIsp3AStatConfig(m_r3AStatConfig);
    }

    return err;
}


/*******************************************************************************
*
********************************************************************************/
MTK3A*
Hal3A::create3AInstance()
{
    return openLib3A();
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::writeIspRegs(
    MUINT32 a_u4StartRegAddr,
    MUINT32 a_u4Count,
    MUINT32 *a_pRegData
)
{
    MINT32 err = MHAL_NO_ERROR;
    IspDrv::reg_t *pRegs = new IspDrv::reg_t[a_u4Count];

	MHAL_ASSERT(m_pIspDrvObj != NULL, "m_pIspDrvObj is NULL");

    for (MUINT32 i = 0; i < a_u4Count; i++) {
        pRegs[i].addr = a_u4StartRegAddr + i * 4;
		pRegs[i].val = *(a_pRegData + i);
    }

    err = m_pIspDrvObj->writeRegs(pRegs, (MINT32)a_u4Count);
	if (err < 0) {
		AAA_ERR("writeIspRegs() error");
    }

	delete [] pRegs;

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::readIspRegs(
    MUINT32 a_u4StartRegAddr,
    MUINT32 a_u4Count,
    MUINT32 *a_pRegData
)
{
    MINT32 err = MHAL_NO_ERROR;
    IspDrv::reg_t *pRegs = new IspDrv::reg_t[a_u4Count];

	MHAL_ASSERT(m_pIspDrvObj != NULL, "m_pIspDrvObj is NULL");

	for (MUINT32 i = 0; i < a_u4Count; i++) {
        pRegs[i].addr = a_u4StartRegAddr + i * 4;

    }

    err = m_pIspDrvObj->readRegs(pRegs, (MINT32)a_u4Count);
	if (err < 0) {
		AAA_ERR("readIspRegs() error");
    }

	for (MUINT32 i = 0; i < a_u4Count; i++) {
		*(a_pRegData + i) = pRegs[i].val;
    }

	delete [] pRegs;

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::holdIspRegs(
    MBOOL a_bHold
)
{
    MHAL_ASSERT(m_pIspDrvObj != NULL, "m_pIspDrvObj is NULL");

    m_pIspDrvObj->holdReg(a_bHold);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setIsp3AStatGain()
{
    MINT32 err = MHAL_NO_ERROR;
	cam_3again_t rIsp3AStatGainCtrl; // 0x003C

	memset((void *)&rIsp3AStatGainCtrl, 0, sizeof(cam_3again_t));

	rIsp3AStatGainCtrl.u.bits.AWBST_GAIN = 0;
	rIsp3AStatGainCtrl.u.bits.AWB_PGAIN = 0;
	rIsp3AStatGainCtrl.u.bits.AWB_PGAIN_EN = 0;
	rIsp3AStatGainCtrl.u.bits.AWBST_GAIN_EN = 0;

    ISP_REG(m_pIspReg, CAM_3AGAIN) = rIsp3AStatGainCtrl.u.reg;

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AAA_STAT_CONFIG) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("[setIsp3AStatGain]\n");
	AAA_LOG("[0x003C] 0x%08x\n", rIsp3AStatGainCtrl.u.reg);
	AAA_LOG("ISP_REG(m_pIspReg, CAM_3A_STAT_GAIN_CTRL) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_3AGAIN));
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setIspPreprocessingCtrl2(
    AAA_STAT_CONFIG_T &a_r3AStatConfig
)
{
    MINT32 err = MHAL_NO_ERROR;
	cam_ctrl2_t rIspPreprocessingCtrl2; // 0x0044
    MUINT32  u4SensorPixelClkFreq = 0;

    memset((void *)&rIspPreprocessingCtrl2, 0, sizeof(cam_ctrl2_t));

    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_PIXEL_CLOCK_FREQ, &u4SensorPixelClkFreq, NULL, NULL);
    MHAL_ASSERT(err == MHAL_NO_ERROR, "Err SENSOR_GET_PIXEL_CLOCK_FREQ");

	rIspPreprocessingCtrl2.u.bits.INTEN = 1;
    rIspPreprocessingCtrl2.u.bits.FHIS_SEL = 1;
    rIspPreprocessingCtrl2.u.bits.REV1 = (u4SensorPixelClkFreq > (g_BaseBandClk>>1))? 1:0;  // for AE histogram horizontal down scale.
    rIspPreprocessingCtrl2.u.bits.AF_SEL = (MUINT32)a_r3AStatConfig.rAFStatConfig.bSel;
    rIspPreprocessingCtrl2.u.bits.AF_EN = (MUINT32)a_r3AStatConfig.rAFStatConfig.bEn;
    rIspPreprocessingCtrl2.u.bits.FHIS_EN = (MUINT32)a_r3AStatConfig.rAEStatConfig.bFlareHisEnable;
    rIspPreprocessingCtrl2.u.bits.DFDB = 0;
    rIspPreprocessingCtrl2.u.bits.RAWACCM_MODE = 1;
    rIspPreprocessingCtrl2.u.bits.AEWIN_EN = (MUINT32)a_r3AStatConfig.rAEStatConfig.bAELumEnable;
    rIspPreprocessingCtrl2.u.bits.AECCM_EN = 1;
    rIspPreprocessingCtrl2.u.bits.AEAWG_EN = 1;
    rIspPreprocessingCtrl2.u.bits.AEHIS_EN = (MUINT32)a_r3AStatConfig.rAEStatConfig.bAEHisEnable;
    rIspPreprocessingCtrl2.u.bits.ISPMEM_R1T = 1;

    ISP_REG(m_pIspReg, CAM_CTRL2) = rIspPreprocessingCtrl2.u.reg;

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AAA_STAT_CONFIG) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("[setIspPreprocessingCtrl2]\n");
        AAA_LOG("rIspPreprocessingCtrl2.u.bits.AF_SEL = %d\n", rIspPreprocessingCtrl2.u.bits.AF_SEL);
	AAA_LOG("rIspPreprocessingCtrl2.u.bits.AF_EN = %d\n", rIspPreprocessingCtrl2.u.bits.AF_EN);
	AAA_LOG("rIspPreprocessingCtrl2.u.bits.FHIS_EN = %d\n", rIspPreprocessingCtrl2.u.bits.FHIS_EN);
	AAA_LOG("rIspPreprocessingCtrl2.u.bits.AEWIN_EN = %d\n", rIspPreprocessingCtrl2.u.bits.AEWIN_EN);
	AAA_LOG("rIspPreprocessingCtrl2.u.bits.AEHIS_EN = %d\n", rIspPreprocessingCtrl2.u.bits.AEHIS_EN);
        AAA_LOG("[0x0044] 0x%08x\n", rIspPreprocessingCtrl2.u.reg);
	AAA_LOG("ISP_REG(m_pIspReg, PREPROCESSING_CTRL2) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_CTRL2));
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setIspAEWinConfig(
    AAA_STAT_CONFIG_T &a_r3AStatConfig
)
{
    MINT32 err = MHAL_NO_ERROR;

	struct {
		cam_aewinh_t rAEWinH; // 0x0048
		cam_aehiswin_t rAEHisWin; // 0x004C
        cam_aegain_t rAEGain; // 0x0050
	} rIspAEWin;

    memset((void *)&rIspAEWin, 0, sizeof(rIspAEWin));

	rIspAEWin.rAEWinH.u.bits.AE_HEIGHT = (MUINT32)a_r3AStatConfig.rAEStatConfig.AE_WinHeight;
	rIspAEWin.rAEWinH.u.bits.AE_VOFFSET = (MUINT32)a_r3AStatConfig.rAEStatConfig.AE_WinVOffset & 0x0000007F;
    rIspAEWin.rAEWinH.u.bits.AE_WIDTH = (MUINT32)a_r3AStatConfig.rAEStatConfig.AE_WinWidth;
    rIspAEWin.rAEWinH.u.bits.AE_HOFFSET = (MUINT32)a_r3AStatConfig.rAEStatConfig.AE_WinHOffset & 0x0000007F;
    rIspAEWin.rAEHisWin.u.bits.AEHIS_WIND = (MUINT32)(a_r3AStatConfig.rAEStatConfig.HIST_WinVOffset + a_r3AStatConfig.rAEStatConfig.HIST_WinHeight);
    rIspAEWin.rAEHisWin.u.bits.AEHIS_WINU = (MUINT32)a_r3AStatConfig.rAEStatConfig.HIST_WinVOffset;
    rIspAEWin.rAEHisWin.u.bits.AEHIS_WINR = (MUINT32)(a_r3AStatConfig.rAEStatConfig.HIST_WinHOffset + a_r3AStatConfig.rAEStatConfig.HIST_WinWidth);
    rIspAEWin.rAEHisWin.u.bits.AEHIS_WINL = (MUINT32)a_r3AStatConfig.rAEStatConfig.HIST_WinHOffset;
    rIspAEWin.rAEGain.u.bits.AE_GAIN = (1 << 8);
	rIspAEWin.rAEGain.u.bits.AE_ACC_SEL = (MUINT32)a_r3AStatConfig.rAEStatConfig.AE_ACC_SEL;
//	rIspAEWin.rAEGain.u.bits.AE_ACC_SEL = (u4SensorPixelClkFreq > (g_BaseBandClk>>1))? 1:0;
	rIspAEWin.rAEGain.u.bits.AE_VOFFSET_H  = (MUINT32)((a_r3AStatConfig.rAEStatConfig.AE_WinVOffset & 0x00000F80) >> 7);
	rIspAEWin.rAEGain.u.bits.AE_HOFFSET_H  = (MUINT32)((a_r3AStatConfig.rAEStatConfig.AE_WinHOffset & 0x00000F80) >> 7);

    ISP_REG(m_pIspReg, CAM_AEWINH) = rIspAEWin.rAEWinH.u.reg;
    ISP_REG(m_pIspReg, CAM_AEHISWIN) = rIspAEWin.rAEHisWin.u.reg;
	ISP_REG(m_pIspReg, CAM_AEGAIN) = rIspAEWin.rAEGain.u.reg;

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AAA_STAT_CONFIG) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("[setIspAEStatConfig]\n");
        AAA_LOG("a_r3AStatConfig.rAEStatConfig.AE_WinVOffset = %d \n", a_r3AStatConfig.rAEStatConfig.AE_WinVOffset);
        AAA_LOG("a_r3AStatConfig.rAEStatConfig.AE_WinHOffset = %d \n", a_r3AStatConfig.rAEStatConfig.AE_WinHOffset);
        AAA_LOG("rIspAEWin.rAEWinH.u.bits.AE_HEIGHT = %d \n", rIspAEWin.rAEWinH.u.bits.AE_HEIGHT);
        AAA_LOG("rIspAEWin.rAEWinH.u.bits.AE_VOFFSET = %d \n", rIspAEWin.rAEWinH.u.bits.AE_VOFFSET);
	AAA_LOG("rIspAEWin.rAEWinH.u.bits.AE_WIDTH = %d \n", rIspAEWin.rAEWinH.u.bits.AE_WIDTH);
	AAA_LOG("rIspAEWin.rAEWinH.u.bits.AE_HOFFSET = %d \n", rIspAEWin.rAEWinH.u.bits.AE_HOFFSET);
	AAA_LOG("rIspAEWin.rAEHisWin.u.bits.AEHIS_WIND = %d \n", rIspAEWin.rAEHisWin.u.bits.AEHIS_WIND);
        AAA_LOG("rIspAEWin.rAEHisWin.u.bits.AEHIS_WINU = %d \n", rIspAEWin.rAEHisWin.u.bits.AEHIS_WINU);
	AAA_LOG("rIspAEWin.rAEHisWin.u.bits.AEHIS_WINR = %d \n", rIspAEWin.rAEHisWin.u.bits.AEHIS_WINR);
	AAA_LOG("rIspAEWin.rAEHisWin.u.bits.AEHIS_WINL = %d \n", rIspAEWin.rAEHisWin.u.bits.AEHIS_WINL);
	AAA_LOG("rIspAEWin.rAEGain.u.bits.AE_GAIN = %d \n", rIspAEWin.rAEGain.u.bits.AE_GAIN);
	AAA_LOG("rIspAEWin.rAEGain.u.bits.AE_ACC_SEL = %d \n", rIspAEWin.rAEGain.u.bits.AE_ACC_SEL);
	AAA_LOG("rIspAEWin.rAEGain.u.bits.AE_VOFFSET_H = %d \n", rIspAEWin.rAEGain.u.bits.AE_VOFFSET_H);
	AAA_LOG("rIspAEWin.rAEGain.u.bits.AE_HOFFSET_H = %d \n", rIspAEWin.rAEGain.u.bits.AE_HOFFSET_H);
        AAA_LOG("[0x0048] 0x%08x\n", rIspAEWin.rAEWinH.u.reg);
	AAA_LOG("[0x004C] 0x%08x\n", rIspAEWin.rAEHisWin.u.reg);
	AAA_LOG("[0x0050] 0x%08x\n", rIspAEWin.rAEGain.u.reg);
	AAA_LOG("ISP_REG(pIspReg, AE_WIN_HORI_WIDTH) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AEWINH));
	AAA_LOG("ISP_REG(pIspReg, AE_HIS_WIN_VERT) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AEHISWIN));
	AAA_LOG("ISP_REG(pIspReg, AE_GAIN) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AEGAIN));
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setIspFDAEWinConfig(
    AAA_STAT_CONFIG_T &a_r3AStatConfig
)
{
    MINT32 err = MHAL_NO_ERROR;
	MINT32 i;
	struct {
		cam_fd_aewin0lr_t rCfg1; // 0x04E0
		cam_fd_aewin0ud_t rCfg2; // 0x04E4
	} rIspFDAEWin[HAL_3A_FD_AE_WIN_NUM];

    memset((void *)rIspFDAEWin, 0, sizeof(rIspFDAEWin));

	for (i = 0; i < HAL_3A_FD_AE_WIN_NUM; i++) {
		rIspFDAEWin[i].rCfg1.u.bits.FD_AEWIN0R = a_r3AStatConfig.rAEStatConfig.rFDAEStatConfig[i].u4FDAEWINR;
		rIspFDAEWin[i].rCfg1.u.bits.FD_AEWIN0L = a_r3AStatConfig.rAEStatConfig.rFDAEStatConfig[i].u4FDAEWINL;
		rIspFDAEWin[i].rCfg1.u.bits.FD_AEWIN0_EN = a_r3AStatConfig.rAEStatConfig.rFDAEStatConfig[i].u4FDAEWIN_EN;
		rIspFDAEWin[i].rCfg2.u.bits.FD_AEWIN0D = a_r3AStatConfig.rAEStatConfig.rFDAEStatConfig[i].u4FDAEWIND;
		rIspFDAEWin[i].rCfg2.u.bits.FD_AEWIN0U = a_r3AStatConfig.rAEStatConfig.rFDAEStatConfig[i].u4FDAEWINU;
    }

    ISP_REG(m_pIspReg, CAM_FD_AEWIN0LR) = rIspFDAEWin[0].rCfg1.u.reg;
    ISP_REG(m_pIspReg, CAM_FD_AEWIN0UD) = rIspFDAEWin[0].rCfg2.u.reg;
    ISP_REG(m_pIspReg, CAM_FD_AEWIN1LR) = rIspFDAEWin[1].rCfg1.u.reg;
    ISP_REG(m_pIspReg, CAM_FD_AEWIN1UD) = rIspFDAEWin[1].rCfg2.u.reg;
    ISP_REG(m_pIspReg, CAM_FD_AEWIN2LR) = rIspFDAEWin[2].rCfg1.u.reg;
    ISP_REG(m_pIspReg, CAM_FD_AEWIN2UD) = rIspFDAEWin[2].rCfg2.u.reg;
    ISP_REG(m_pIspReg, CAM_FD_AEWIN3LR) = rIspFDAEWin[3].rCfg1.u.reg;
    ISP_REG(m_pIspReg, CAM_FD_AEWIN3UD) = rIspFDAEWin[3].rCfg2.u.reg;

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AAA_STAT_CONFIG) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("[setIspFDAEWinConfig]\n");

	for (i = 0; i < HAL_3A_FD_AE_WIN_NUM; i++) {
            AAA_LOG("rIspFDAEWin[%d].rCfg1.u.bits.FD_AEWIN0R = %d \n", i, rIspFDAEWin[i].rCfg1.u.bits.FD_AEWIN0R);
	    AAA_LOG("rIspFDAEWin[%d].rCfg1.u.bits.FD_AEWIN0L = %d \n", i, rIspFDAEWin[i].rCfg1.u.bits.FD_AEWIN0L);
	    AAA_LOG("rIspFDAEWin[%d].rCfg1.u.bits.FD_AEWIN0_EN = %d \n", i, rIspFDAEWin[i].rCfg1.u.bits.FD_AEWIN0_EN);
	    AAA_LOG("rIspFDAEWin[%d].rCfg2.u.bits.FD_AEWIN0D = %d \n", i, rIspFDAEWin[i].rCfg2.u.bits.FD_AEWIN0D);
	    AAA_LOG("rIspFDAEWin[%d].rCfg2.u.bits.FD_AEWIN0U = %d \n", i, rIspFDAEWin[i].rCfg2.u.bits.FD_AEWIN0U);
	    AAA_LOG("[0x%08x] 0x%08x\n", 0x04E0 + 8*i, rIspFDAEWin[i].rCfg1.u.reg);
	    AAA_LOG("[0x%08x] 0x%08x\n", 0x04E4 + 8*i, rIspFDAEWin[i].rCfg2.u.reg);
        }

        AAA_LOG("ISP_REG(m_pIspReg, CAM_FD_AEWIN0LR) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_FD_AEWIN0LR));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_FD_AEWIN0UD) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_FD_AEWIN0UD));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_FD_AEWIN1LR) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_FD_AEWIN1LR));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_FD_AEWIN1UD) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_FD_AEWIN1UD));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_FD_AEWIN2LR) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_FD_AEWIN2LR));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_FD_AEWIN2UD) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_FD_AEWIN2UD));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_FD_AEWIN3LR) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_FD_AEWIN3LR));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_FD_AEWIN3UD) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_FD_AEWIN3UD));
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setIspAFWinConfig(
    AAA_STAT_CONFIG_T &a_r3AStatConfig
)
{
    MINT32 err = MHAL_NO_ERROR;
	MINT32 i;
	cam_afwin0_t rIspAFWin[HAL_3A_AF_WIN_NUM]; // 0x03D8

    memset((void *)rIspAFWin, 0, sizeof(cam_afwin0_t) * HAL_3A_AF_WIN_NUM);

	for (i = 0; i < HAL_3A_AF_WIN_NUM; i++) {
		rIspAFWin[i].u.bits.BOTTOM = (MUINT32)a_r3AStatConfig.rAFStatConfig.sAFWin.sWin[i].uBottom;
		rIspAFWin[i].u.bits.TOP = (MUINT32)a_r3AStatConfig.rAFStatConfig.sAFWin.sWin[i].uUp;
		rIspAFWin[i].u.bits.RIGHT = (MUINT32)a_r3AStatConfig.rAFStatConfig.sAFWin.sWin[i].uRight;
		rIspAFWin[i].u.bits.LEFT = (MUINT32)a_r3AStatConfig.rAFStatConfig.sAFWin.sWin[i].uLeft;
    }


    ISP_REG(m_pIspReg, CAM_AFWIN0) = rIspAFWin[0].u.reg;
    ISP_REG(m_pIspReg, CAM_AFWIN1) = rIspAFWin[1].u.reg;
    ISP_REG(m_pIspReg, CAM_AFWIN2) = rIspAFWin[2].u.reg;
    ISP_REG(m_pIspReg, CAM_AFWIN3) = rIspAFWin[3].u.reg;
    ISP_REG(m_pIspReg, CAM_AFWIN4) = rIspAFWin[4].u.reg;
    ISP_REG(m_pIspReg, CAM_AFWIN5) = rIspAFWin[5].u.reg;
    ISP_REG(m_pIspReg, CAM_AFWIN6) = rIspAFWin[6].u.reg;
    ISP_REG(m_pIspReg, CAM_AFWIN7) = rIspAFWin[7].u.reg;
    ISP_REG(m_pIspReg, CAM_AFWIN8) = rIspAFWin[8].u.reg;

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AAA_STAT_CONFIG) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("[setIspAFWinConfig]\n");

	for (i = 0; i < HAL_3A_AF_WIN_NUM; i++) {
            AAA_LOG("rIspAFWin[%d].u.bits.BOTTOM = %d \n", i, rIspAFWin[i].u.bits.BOTTOM);
	    AAA_LOG("rIspAFWin[%d].u.bits.TOP = %d \n", i, rIspAFWin[i].u.bits.TOP);
	    AAA_LOG("rIspAFWin[%d].u.bits.RIGHT = %d \n", i, rIspAFWin[i].u.bits.RIGHT);
	    AAA_LOG("rIspAFWin[%d].u.bits.LEFT = %d \n", i, rIspAFWin[i].u.bits.LEFT);
	    AAA_LOG("[0x%08x] 0x%08x\n", 0x03D8 + 4*i, rIspAFWin[i].u.reg);
        }

        AAA_LOG("ISP_REG(m_pIspReg, CAM_AFWIN0) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AFWIN0));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AFWIN1) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AFWIN1));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AFWIN2) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AFWIN2));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AFWIN3) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AFWIN3));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AFWIN4) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AFWIN4));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AFWIN5) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AFWIN5));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AFWIN6) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AFWIN6));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AFWIN7) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AFWIN7));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AFWIN8) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AFWIN8));
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setIspAFThr(
    AAA_STAT_CONFIG_T &a_r3AStatConfig
)
{
    MINT32 err = MHAL_NO_ERROR;
	struct {
		cam_afth0_t rThr0; // 0x04D4
		cam_afth1_t rThr1; // 0x04D8
	} rIspAFThr;

    memset((void *)&rIspAFThr, 0, sizeof(rIspAFThr));

    rIspAFThr.rThr0.u.bits.AF_TH0 = (MUINT32)a_r3AStatConfig.rAFStatConfig.uTH[0];
	rIspAFThr.rThr0.u.bits.AF_TH1 = (MUINT32)a_r3AStatConfig.rAFStatConfig.uTH[1];
	rIspAFThr.rThr0.u.bits.AF_TH2 = (MUINT32)a_r3AStatConfig.rAFStatConfig.uTH[2];
	rIspAFThr.rThr0.u.bits.AF_TH3 = (MUINT32)a_r3AStatConfig.rAFStatConfig.uTH[3];
	rIspAFThr.rThr1.u.bits.AF_TH4 = (MUINT32)a_r3AStatConfig.rAFStatConfig.uTH[4];

    ISP_REG(m_pIspReg, CAM_AFTH0) = rIspAFThr.rThr0.u.reg;
    ISP_REG(m_pIspReg, CAM_AFTH1) = rIspAFThr.rThr1.u.reg;

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AAA_STAT_CONFIG) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("[setIspAFThr]\n");
        AAA_LOG("rIspAFThr.rThr0.u.bits.AF_TH0 = %d \n", rIspAFThr.rThr0.u.bits.AF_TH0);
        AAA_LOG("rIspAFThr.rThr0.u.bits.AF_TH1 = %d \n", rIspAFThr.rThr0.u.bits.AF_TH1);
	AAA_LOG("rIspAFThr.rThr0.u.bits.AF_TH2 = %d \n", rIspAFThr.rThr0.u.bits.AF_TH2);
	AAA_LOG("rIspAFThr.rThr0.u.bits.AF_TH3 = %d \n", rIspAFThr.rThr0.u.bits.AF_TH3);
	AAA_LOG("rIspAFThr.rThr1.u.bits.AF_TH4 = %d \n", rIspAFThr.rThr1.u.bits.AF_TH4);
        AAA_LOG("[0x04D4] 0x%08x\n", rIspAFThr.rThr0.u.reg);
	AAA_LOG("[0x04D8] 0x%08x\n", rIspAFThr.rThr1.u.reg);
	AAA_LOG("ISP_REG(pIspReg, CAM_AFTH0) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AFTH0));
	AAA_LOG("ISP_REG(pIspReg, CAM_AFTH1) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AFTH1));
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setIspAWBWinConfig(
    AAA_STAT_CONFIG_T &a_r3AStatConfig
)
{
    MINT32 err = MHAL_NO_ERROR;
	MINT32 i;
	struct {
		cam_awbsum_win_t rSumWinCfg; // 0x027C
		cam_awb_ctrl_t rCtrl; // 0x0280
		cam_awbth_t rThr; // 0x0284
		cam_awbxyh1_t rXYH1; // 0x0288
		cam_awbxyh2_t rXYH2; // 0x028C
		cam_awbce_winh_t rCEWinHori; // 0x0290
		cam_awbce_winv_t rCEWinVert; // 0x0294
		struct {
			cam_awbxywinh0_t rHori; // 0x0298
			cam_awbxywinv0_t rVert; // 0x029C
		} rXYWin[HAL_3A_AWB_WIN_NUM];
	} rIspAWBWin;

    memset((void *)&rIspAWBWin, 0, sizeof(rIspAWBWin));

    // AWB Sum Window Config Register
    rIspAWBWin.rSumWinCfg.u.bits.AWBSUM_WIND = a_r3AStatConfig.rAWBStatConfig.u4AWBSUM_WIND;
    rIspAWBWin.rSumWinCfg.u.bits.AWBSUM_WINU = a_r3AStatConfig.rAWBStatConfig.u4AWBSUM_WINU;
    rIspAWBWin.rSumWinCfg.u.bits.AWBSUM_WINR = a_r3AStatConfig.rAWBStatConfig.u4AWBSUM_WINR;
    rIspAWBWin.rSumWinCfg.u.bits.AWBSUM_WINL = a_r3AStatConfig.rAWBStatConfig.u4AWBSUM_WINL;

    // AWB Control Register
    rIspAWBWin.rCtrl.u.bits.PAXEL_YL = a_r3AStatConfig.rAWBStatConfig.u4PAXEL_YL;
    rIspAWBWin.rCtrl.u.bits.PAXEL_RGBH = a_r3AStatConfig.rAWBStatConfig.u4PAXEL_RGBH;
    rIspAWBWin.rCtrl.u.bits.AWBDM_DEBUG = a_r3AStatConfig.rAWBStatConfig.u4AWBDM_DEBUG;
    rIspAWBWin.rCtrl.u.bits.SMAREA_NO = a_r3AStatConfig.rAWBStatConfig.u4SMAREA_NO;
    rIspAWBWin.rCtrl.u.bits.SMAREA_EN = a_r3AStatConfig.rAWBStatConfig.u4SMAREA_EN;
    rIspAWBWin.rCtrl.u.bits.COLOREDGE_EN = a_r3AStatConfig.rAWBStatConfig.u4COLOREDGE_EN;
    rIspAWBWin.rCtrl.u.bits.NEUTRAL_EN = a_r3AStatConfig.rAWBStatConfig.u4NEUTRAL_EN;
    rIspAWBWin.rCtrl.u.bits.AWB_EN = a_r3AStatConfig.rAWBStatConfig.u4AWB_EN;

    // AWB Threshold Config
    rIspAWBWin.rThr.u.bits.NEUTRAL_TH = a_r3AStatConfig.rAWBStatConfig.u4NEUTRAL_TH;
    rIspAWBWin.rThr.u.bits.CEDGEY_TH = a_r3AStatConfig.rAWBStatConfig.u4CEDGEY_TH;
    rIspAWBWin.rThr.u.bits.CEDGEX_TH = a_r3AStatConfig.rAWBStatConfig.u4CEDGEX_TH;

    // AWB Color Space H1/H2 Config Register
    if (a_r3AStatConfig.rAWBStatConfig.i4AWBH11 >= 0) {
        rIspAWBWin.rXYH1.u.bits.AWBH11 = (MUINT32)a_r3AStatConfig.rAWBStatConfig.i4AWBH11;
    }
    else { // Sign bit = 1
        rIspAWBWin.rXYH1.u.bits.AWBH11 = (MUINT32)((1 << 8) - a_r3AStatConfig.rAWBStatConfig.i4AWBH11);
    }

    if (a_r3AStatConfig.rAWBStatConfig.i4AWBH12 >= 0) {
        rIspAWBWin.rXYH1.u.bits.AWBH12 = (MUINT32)a_r3AStatConfig.rAWBStatConfig.i4AWBH12;
    }
    else { // Sign bit = 1
        rIspAWBWin.rXYH1.u.bits.AWBH12 = (MUINT32)((1 << 8) - a_r3AStatConfig.rAWBStatConfig.i4AWBH12);
    }

    if (a_r3AStatConfig.rAWBStatConfig.i4AWBH21 >= 0) {
        rIspAWBWin.rXYH2.u.bits.AWBH21 = (MUINT32)a_r3AStatConfig.rAWBStatConfig.i4AWBH21;
    }
    else { // Sign bit = 1
        rIspAWBWin.rXYH2.u.bits.AWBH21 = (MUINT32)((1 << 8) - a_r3AStatConfig.rAWBStatConfig.i4AWBH21);
    }


    if (a_r3AStatConfig.rAWBStatConfig.i4AWBH22 >= 0) {
        rIspAWBWin.rXYH2.u.bits.AWBH22 = (MUINT32)a_r3AStatConfig.rAWBStatConfig.i4AWBH22;
    }
    else { // Sign bit = 1
        rIspAWBWin.rXYH2.u.bits.AWBH22 = (MUINT32)((1 << 8) - a_r3AStatConfig.rAWBStatConfig.i4AWBH22);
    }

    // AWB Color Edge Window Vertical/Horizontal Config Register
    rIspAWBWin.rCEWinHori.u.bits.AWBCE_WINR = (MUINT32)a_r3AStatConfig.rAWBStatConfig.i4AWBCE_WINR;
	rIspAWBWin.rCEWinHori.u.bits.AWBCE_WINL = (MUINT32)a_r3AStatConfig.rAWBStatConfig.i4AWBCE_WINL;
	rIspAWBWin.rCEWinVert.u.bits.AWBCE_WIND = (MUINT32)a_r3AStatConfig.rAWBStatConfig.i4AWBCE_WIND;
	rIspAWBWin.rCEWinVert.u.bits.AWBCE_WINU = (MUINT32)a_r3AStatConfig.rAWBStatConfig.i4AWBCE_WINU;

    // AWB XY Window Vertical/Horizontal Config Register
    for (i = 0; i < HAL_3A_AWB_WIN_NUM; i++) {
		rIspAWBWin.rXYWin[i].rHori.u.bits.AWBXY_WINR0 = (MUINT32)((1 << 15) + a_r3AStatConfig.rAWBStatConfig.i4AWBXY_WINR[i]);
		rIspAWBWin.rXYWin[i].rHori.u.bits.AWBXY_WINL0 = (MUINT32)((1 << 15) + a_r3AStatConfig.rAWBStatConfig.i4AWBXY_WINL[i]);
		rIspAWBWin.rXYWin[i].rVert.u.bits.AWBXY_WIND0 = (MUINT32)((1 << 15) + a_r3AStatConfig.rAWBStatConfig.i4AWBXY_WIND[i]);
		rIspAWBWin.rXYWin[i].rVert.u.bits.AWBXY_WINU0 = (MUINT32)((1 << 15) + a_r3AStatConfig.rAWBStatConfig.i4AWBXY_WINU[i]);
    }

    ISP_REG(m_pIspReg, CAM_AWBSUM_WIN) = rIspAWBWin.rSumWinCfg.u.reg;
    ISP_REG(m_pIspReg, CAM_AWB_CTRL) = rIspAWBWin.rCtrl.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBTH) = rIspAWBWin.rThr.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYH1) = rIspAWBWin.rXYH1.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYH2) = rIspAWBWin.rXYH2.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBCE_WINH) = rIspAWBWin.rCEWinHori.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBCE_WINV) = rIspAWBWin.rCEWinVert.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINH0) = rIspAWBWin.rXYWin[0].rHori.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINV0) = rIspAWBWin.rXYWin[0].rVert.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINH1) = rIspAWBWin.rXYWin[1].rHori.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINV1) = rIspAWBWin.rXYWin[1].rVert.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINH2) = rIspAWBWin.rXYWin[2].rHori.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINV2) = rIspAWBWin.rXYWin[2].rVert.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINH3) = rIspAWBWin.rXYWin[3].rHori.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINV3) = rIspAWBWin.rXYWin[3].rVert.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINH4) = rIspAWBWin.rXYWin[4].rHori.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINV4) = rIspAWBWin.rXYWin[4].rVert.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINH5) = rIspAWBWin.rXYWin[5].rHori.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINV5) = rIspAWBWin.rXYWin[5].rVert.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINH6) = rIspAWBWin.rXYWin[6].rHori.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINV6) = rIspAWBWin.rXYWin[6].rVert.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINH7) = rIspAWBWin.rXYWin[7].rHori.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINV7) = rIspAWBWin.rXYWin[7].rVert.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINH8) = rIspAWBWin.rXYWin[8].rHori.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINV8) = rIspAWBWin.rXYWin[8].rVert.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINH9) = rIspAWBWin.rXYWin[9].rHori.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINV9) = rIspAWBWin.rXYWin[9].rVert.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINH10) = rIspAWBWin.rXYWin[10].rHori.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINV10) = rIspAWBWin.rXYWin[10].rVert.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINH11) = rIspAWBWin.rXYWin[11].rHori.u.reg;
	ISP_REG(m_pIspReg, CAM_AWBXYWINV11) = rIspAWBWin.rXYWin[11].rVert.u.reg;


    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AAA_STAT_CONFIG) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("[setIspAWBWinConfig]\n");
        AAA_LOG("rIspAWBWin.rSumWinCfg.u.bits.AWBSUM_WIND = %d \n", rIspAWBWin.rSumWinCfg.u.bits.AWBSUM_WIND);
        AAA_LOG("rIspAWBWin.rSumWinCfg.u.bits.AWBSUM_WINU = %d \n", rIspAWBWin.rSumWinCfg.u.bits.AWBSUM_WINU);
	AAA_LOG("rIspAWBWin.rSumWinCfg.u.bits.AWBSUM_WINR = %d \n", rIspAWBWin.rSumWinCfg.u.bits.AWBSUM_WINR);
	AAA_LOG("rIspAWBWin.rSumWinCfg.u.bits.AWBSUM_WINL = %d \n", rIspAWBWin.rSumWinCfg.u.bits.AWBSUM_WINL);
        AAA_LOG("[0x027C] 0x%08x\n", rIspAWBWin.rSumWinCfg.u.reg);
	AAA_LOG("ISP_REG(pIspReg, CAM_AWBSUM_WIN) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBSUM_WIN));

	AAA_LOG("rIspAWBWin.rCtrl.u.bits.PAXEL_YL = %d \n", rIspAWBWin.rCtrl.u.bits.PAXEL_YL);
        AAA_LOG("rIspAWBWin.rCtrl.u.bits.PAXEL_RGBH = %d \n", rIspAWBWin.rCtrl.u.bits.PAXEL_RGBH);
	AAA_LOG("rIspAWBWin.rCtrl.u.bits.AWBDM_DEBUG = %d \n", rIspAWBWin.rCtrl.u.bits.AWBDM_DEBUG);
	AAA_LOG("rIspAWBWin.rCtrl.u.bits.SMAREA_NO = %d \n", rIspAWBWin.rCtrl.u.bits.SMAREA_NO);
	AAA_LOG("rIspAWBWin.rCtrl.u.bits.SMAREA_EN = %d \n", rIspAWBWin.rCtrl.u.bits.SMAREA_EN);
        AAA_LOG("rIspAWBWin.rCtrl.u.bits.COLOREDGE_EN = %d \n", rIspAWBWin.rCtrl.u.bits.COLOREDGE_EN);
	AAA_LOG("rIspAWBWin.rCtrl.u.bits.NEUTRAL_EN = %d \n", rIspAWBWin.rCtrl.u.bits.NEUTRAL_EN);
	AAA_LOG("rIspAWBWin.rCtrl.u.bits.AWB_EN = %d \n", rIspAWBWin.rCtrl.u.bits.AWB_EN);
	AAA_LOG("[0x0280] 0x%08x\n", rIspAWBWin.rCtrl.u.reg);
        AAA_LOG("ISP_REG(pIspReg, CAM_AWB_CTRL) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWB_CTRL));

	AAA_LOG("rIspAWBWin.rThr.u.bits.NEUTRAL_TH = %d \n", rIspAWBWin.rThr.u.bits.NEUTRAL_TH);
	AAA_LOG("rIspAWBWin.rThr.u.bits.CEDGEY_TH = %d \n", rIspAWBWin.rThr.u.bits.CEDGEY_TH);
	AAA_LOG("rIspAWBWin.rThr.u.bits.CEDGEX_TH = %d \n", rIspAWBWin.rThr.u.bits.CEDGEX_TH);
        AAA_LOG("[0x0284] 0x%08x\n", rIspAWBWin.rThr.u.reg);
        AAA_LOG("ISP_REG(pIspReg, CAM_AWBTH) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBTH));

	AAA_LOG("rIspAWBWin.rXYH1.u.bits.AWBH11 = %d \n", rIspAWBWin.rXYH1.u.bits.AWBH11);
	AAA_LOG("rIspAWBWin.rXYH1.u.bits.AWBH12 = %d \n", rIspAWBWin.rXYH1.u.bits.AWBH12);
        AAA_LOG("[0x0288] 0x%08x\n", rIspAWBWin.rXYH1.u.reg);
        AAA_LOG("ISP_REG(pIspReg, CAM_AWBXYH1) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYH1));

	AAA_LOG("rIspAWBWin.rXYH2.u.bits.AWBH21 = %d \n", rIspAWBWin.rXYH2.u.bits.AWBH21);
	AAA_LOG("rIspAWBWin.rXYH2.u.bits.AWBH22 = %d \n", rIspAWBWin.rXYH2.u.bits.AWBH22);
        AAA_LOG("[0x028C] 0x%08x\n", rIspAWBWin.rXYH2.u.reg);
        AAA_LOG("ISP_REG(pIspReg, CAM_AWBXYH2) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYH2));

	AAA_LOG("rIspAWBWin.rCEWinHori.u.bits.AWBCE_WINR = %d \n", rIspAWBWin.rCEWinHori.u.bits.AWBCE_WINR);
	AAA_LOG("rIspAWBWin.rCEWinHori.u.bits.AWBCE_WINL = %d \n", rIspAWBWin.rCEWinHori.u.bits.AWBCE_WINL);
        AAA_LOG("[0x0290] 0x%08x\n", rIspAWBWin.rCEWinHori.u.reg);
        AAA_LOG("ISP_REG(pIspReg, CAM_AWBCE_WINH) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBCE_WINH));

	AAA_LOG("rIspAWBWin.rCEWinVert.u.bits.AWBCE_WIND = %d \n", rIspAWBWin.rCEWinVert.u.bits.AWBCE_WIND);
	AAA_LOG("rIspAWBWin.rCEWinVert.u.bits.AWBCE_WINU = %d \n", rIspAWBWin.rCEWinVert.u.bits.AWBCE_WINU);
        AAA_LOG("[0x0294] 0x%08x\n", rIspAWBWin.rCEWinVert.u.reg);
	AAA_LOG("ISP_REG(pIspReg, CAM_AWBCE_WINV) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBCE_WINV));

        for (i = 0; i < HAL_3A_AWB_WIN_NUM; i++) {
	    AAA_LOG("rIspAWBWin.rXYWin[%d].rHori.u.bits.AWBXY_WINR0 = %d \n", i, rIspAWBWin.rXYWin[i].rHori.u.bits.AWBXY_WINR0);
	    AAA_LOG("rIspAWBWin.rXYWin[%d].rHori.u.bits.AWBXY_WINL0 = %d \n", i, rIspAWBWin.rXYWin[i].rHori.u.bits.AWBXY_WINL0);
	    AAA_LOG("rIspAWBWin.rXYWin[%d].rVert.u.bits.AWBXY_WIND0 = %d \n", i, rIspAWBWin.rXYWin[i].rVert.u.bits.AWBXY_WIND0);
	    AAA_LOG("rIspAWBWin.rXYWin[%d].rVert.u.bits.AWBXY_WINU0 = %d \n", i, rIspAWBWin.rXYWin[i].rVert.u.bits.AWBXY_WINU0);
	    AAA_LOG("[0x%08x] 0x%08x\n", 0x0298 + 8*i, rIspAWBWin.rXYWin[i].rHori.u.reg);
	    AAA_LOG("[0x%08x] 0x%08x\n", 0x029C + 8*i, rIspAWBWin.rXYWin[i].rVert.u.reg);
	}

        AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINH0) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINH0));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINV0) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINV0));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINH1) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINH1));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINV1) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINV1));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINH2) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINH2));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINV2) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINV2));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINH3) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINH3));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINV3) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINV3));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINH4) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINH4));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINV4) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINV4));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINH5) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINH5));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINV5) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINV5));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINH6) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINH6));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINV6) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINV6));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINH7) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINH7));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINV7) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINV7));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINH8) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINH8));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINV8) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINV8));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINH9) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINH9));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINV9) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINV9));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINV10) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINV10));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINH10) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINH10));
        AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINH11) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINH11));
	AAA_LOG("ISP_REG(m_pIspReg, CAM_AWBXYWINV11) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_AWBXYWINV11));
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setIsp3AStatConfig(
    AAA_STAT_CONFIG_T &a_r3AStatConfig
)
{
    MINT32 err = MHAL_NO_ERROR;

    // set 3A statistics gain
    err = setIsp3AStatGain();
	if (err < 0) {
		AAA_ERR("setIsp3AStatGain() error");
        return err;
    }

	// set preprocessing control 2
    err = setIspPreprocessingCtrl2(a_r3AStatConfig);
	if (err < 0) {
		AAA_ERR("setIspPreprocessingCtrl2() error");
        return err;
    }

	// set AE window config parameter
	err = setIspAEWinConfig(a_r3AStatConfig);
	if (err < 0) {
		AAA_ERR("setIspAEWinConfig() error");
        return err;
    }

	// set FD AE window config parameter
	err = setIspFDAEWinConfig(a_r3AStatConfig);
	if (err < 0) {
		AAA_ERR("setIspFDAEWinConfig() error");
        return err;
    }

    if (m_r3AOutput.bUpdateAFStatConfig == TRUE)
    {
	    // set AF window config parameter
        if (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AF_LOG)  {
        AAA_LOG("3 UpdateAFStatConfig %d %d %d %d\n", a_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uLeft, a_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uRight, a_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uUp, a_r3AStatConfig.rAFStatConfig.sAFWin.sWin[0].uBottom);
        }

        err = setIspAFWinConfig(a_r3AStatConfig);
	    if (err < 0) {
    		AAA_ERR("setIspAFWinConfig() error");
            return err;
        }

    	// set AF threshold
        err = setIspAFThr(a_r3AStatConfig);
    	if (err < 0) {
	    	AAA_ERR("setIspAFThr() error");
            return err;
        }
    }

	// set AWB window config parameter
    err = setIspAWBWinConfig(a_r3AStatConfig);
	if (err < 0) {
		AAA_ERR("setIspAWBWinConfig() error");
        return err;
    }

    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getIsp3AStat(
    AAA_STAT_T &a_r3AStat
)
{
    MINT32 err = MHAL_NO_ERROR;
	MINT32 i,j;
	struct {
		// AE window result: 0x1000 ~ 0x104C
		MUINT32 AEWinResult[HAL_3A_AE_WIN_RESULT_NUM];
		// 3A info: 0x1050 ~ 0x105C
		MUINT32 aaaInfo[HAL_3A_INFO_NUM];
		// Flare histogram: 0x1060 ~ 0x1084
		cam_flaremem_t rFlareHist[HAL_3A_FLARE_HISTOGRAM_BIN_NUM];
		// AF window result: 0x1088 ~ 0x1138
		cam_afn_t rAFWinResult[HAL_3A_AF_WIN_NUM][HAL_3A_AF_THRESHOLD_NUM];
		// AF mean: 0x113C ~ 0x115C
		cam_afgsum_t rAFMean[HAL_3A_AF_WIN_NUM];
		// AWB XY result: 0x1160 ~ 0x121C
		struct {
			MUINT32 PCNT;
			MUINT32 RSUM;
			MUINT32 GSUM;
			MUINT32 BSUM;
		} rAWBXYResult[HAL_3A_AWB_WIN_NUM];
		// AWB sum result: 0x1220 ~ 0x122C
		struct {
			MUINT32 PCNT;
			MUINT32 RSUM;
			MUINT32 GSUM;
			MUINT32 BSUM;
		} rAWBSumResult;
		// AWB color edge result: 0x1230 ~ 0x123C
		struct {
			MUINT32 PCNT;
			MUINT32 RSUM;
			MUINT32 GSUM;
			MUINT32 BSUM;
		} rAWBCEResult;
		// AE histogram: 0x1240 ~ 0x133C
		cam_aehis_t rAEHist[HAL_3A_HISTOGRAM_BIN_NUM];
		// FD AE sum: 0x1340 ~ 0x134C
		cam_fdaesum_t rFDAESum[HAL_3A_FD_AE_WIN_NUM];
		// FD AE count: 0x1350 ~ 0x135C
		cam_fdaecnt_t rFDAECount[HAL_3A_FD_AE_WIN_NUM];
		// FD AWB R sum: 0x1360 ~ 0x136C
		cam_fdawbrsum_t rFDAWBRSum[HAL_3A_FD_AE_WIN_NUM];
		// FD AWB G sum: 0x1370 ~ 0x137C
		cam_fdawbgsum_t rFDAWBGSum[HAL_3A_FD_AE_WIN_NUM];
		// FD AWB B sum: 0x1380 ~ 0x138C
		cam_fdawbbsum_t rFDAWBBSum[HAL_3A_FD_AE_WIN_NUM];
		// FD AWB count: 0x1390 ~ 0x139C
		cam_fdawbcnt_t rFDAWBCount[HAL_3A_FD_AE_WIN_NUM];
	} rIsp3AStat;

        struct {
            // AF window result: 0x3FC ~ only 8 win
            cam_afn_t rAFWinResult[HAL_3A_AF_WIN_NUM][HAL_3A_AF_THRESHOLD_NUM];
            // AF mean: 0x113C ~ 0x115C
            cam_afgsum_t rAFMean[HAL_3A_AF_WIN_NUM-1];
        } rIspAFStat;


	memset((void *)&rIsp3AStat, 0, sizeof(rIsp3AStat));
	memset((void *)&rIspAFStat, 0, sizeof(rIspAFStat));

    err = readIspRegs(0x1000, sizeof(rIsp3AStat)/sizeof(MUINT32), (MUINT32 *)&rIsp3AStat);
	if (err < 0) {
		AAA_ERR("readIspRegs() error");
        return err;
    }

    err = readIspRegs(0x3FC, sizeof(rIspAFStat)/sizeof(MUINT32), (MUINT32 *)&rIspAFStat);
	if (err < 0) {
		AAA_ERR("readIspRegs() error");
        return err;
    }

	//____AE Statistics____
    for (i = 0; i < 5; i++) {
        a_r3AStat.rAEStat.u4AEWindowInfo[0+5*i] = rIsp3AStat.AEWinResult[0+4*i] & 0x00FFFFFF;
        a_r3AStat.rAEStat.u4AEWindowInfo[1+5*i] = ((rIsp3AStat.AEWinResult[0+4*i] >> 24) & 0x000000FF) | ((rIsp3AStat.AEWinResult[1+4*i] & 0x0000FFFF) << 8);
        a_r3AStat.rAEStat.u4AEWindowInfo[2+5*i] = ((rIsp3AStat.AEWinResult[1+4*i] >> 16) & 0x0000FFFF) | ((rIsp3AStat.AEWinResult[2+4*i] & 0x000000FF) << 16);
        a_r3AStat.rAEStat.u4AEWindowInfo[3+5*i] = (rIsp3AStat.AEWinResult[2+4*i] >> 8) & 0x00FFFFFF;
        a_r3AStat.rAEStat.u4AEWindowInfo[4+5*i] = rIsp3AStat.AEWinResult[3+4*i] & 0x00FFFFFF;
    }

    a_r3AStat.rAEStat.u4AEBlockCnt = rIsp3AStat.aaaInfo[0];
    // workround for the ZSD and non-ZSD switch
    if((g_AEStatCnt != a_r3AStat.rAEStat.u4AEBlockCnt) && (g_AEStatCnt != 0) && (m_rEZoomInfo.u4XOffset == 0) && (m_rEZoomInfo.u4YOffset == 0) && (g_AEStatCnt > a_r3AStat.rAEStat.u4AEBlockCnt)) {
    	 a_r3AStat.rAEStat.u4AEBlockCnt = g_AEStatCnt;
        AAA_LOG("[getIsp3AStat] Restore the AE Statistic counter:%d %d\n", g_AEStatCnt, rIsp3AStat.aaaInfo[0]);    	
    }
    g_AEStatCnt = rIsp3AStat.aaaInfo[0];
    	
    a_r3AStat.u4RawWidth = rIsp3AStat.aaaInfo[1] >> 16;
    a_r3AStat.u4RawHeight = rIsp3AStat.aaaInfo[1] & 0xFFFF;

    for (i = 0; i < HAL_3A_FLARE_HISTOGRAM_BIN_NUM; i++) {
        a_r3AStat.rAEStat.u4FlareHistogram[i] = rIsp3AStat.rFlareHist[i].u.bits.FLAREB0_9;
    }

    for (i = 0; i < HAL_3A_HISTOGRAM_BIN_NUM; i++) {
        a_r3AStat.rAEStat.u4AEHistogram[i] = rIsp3AStat.rAEHist[i].u.bits.AEHIS;
    }

	for (i = 0; i < HAL_3A_FD_AE_WIN_NUM; i++) {
        a_r3AStat.rAEStat.rFDAEStat.u4FDAE_SUM[i] = rIsp3AStat.rFDAESum[i].u.bits.FD_AESUM0_3;
		a_r3AStat.rAEStat.rFDAEStat.u4FDAE_COUNT[i] = rIsp3AStat.rFDAECount[i].u.bits.FD_AECNT0_3;
    }
        
    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AE_STAT) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("____AE Statistics____\n");
        for (i = 0; i < 5; i++) {
		    AAA_LOG("%d %d %d %d %d\n", a_r3AStat.rAEStat.u4AEWindowInfo[0+5*i],
			                            a_r3AStat.rAEStat.u4AEWindowInfo[1+5*i],
			                            a_r3AStat.rAEStat.u4AEWindowInfo[2+5*i],
			                            a_r3AStat.rAEStat.u4AEWindowInfo[3+5*i],
			                            a_r3AStat.rAEStat.u4AEWindowInfo[4+5*i]);
        }

        // AE statistic window test
        MUINT32 u4AvgW = 0;

        for (i=0; i<25; i++)
        {
            u4AvgW += a_r3AStat.rAEStat.u4AEWindowInfo[i];
        }

        AAA_LOG("AE Y:%d\n", 4*u4AvgW/25/a_r3AStat.rAEStat.u4AEBlockCnt);

        AAA_LOG("a_r3AStat.rAEStat.u4AEBlockCnt = %d \n", a_r3AStat.rAEStat.u4AEBlockCnt);
        AAA_LOG("a_r3AStat.u4RawWidth = %d \n", a_r3AStat.u4RawWidth);
        AAA_LOG("a_r3AStat.u4RawHeight = %d \n", a_r3AStat.u4RawHeight);

        for (i = 0; i < HAL_3A_FLARE_HISTOGRAM_BIN_NUM; i++) {
            AAA_LOG("a_r3AStat.rAEStat.u4FlareHistogram[%d] = %d \n", i, a_r3AStat.rAEStat.u4FlareHistogram[i]);
        }

        for (i = 0; i < HAL_3A_HISTOGRAM_BIN_NUM; i++) {
            AAA_LOG("a_r3AStat.rAEStat.u4AEHistogram[%d] = %d \n", i, a_r3AStat.rAEStat.u4AEHistogram[i]);
        }

        for (i = 0; i < HAL_3A_FD_AE_WIN_NUM; i++) {
            AAA_LOG("a_r3AStat.rAEStat.rFDAEStat.u4FDAE_SUM[%d] = %d \n", i, a_r3AStat.rAEStat.rFDAEStat.u4FDAE_SUM[i]);
            AAA_LOG("a_r3AStat.rAEStat.rFDAEStat.u4FDAE_COUNT[%d] = %d \n", i, a_r3AStat.rAEStat.rFDAEStat.u4FDAE_COUNT[i]);
        }
    }

    //____AF Statistics____
    for (i = 0; i < HAL_3A_AF_WIN_NUM-1; i++) {
        for (j = 0; j < HAL_3A_AF_THRESHOLD_NUM; j++) {
            a_r3AStat.rAFStat.sWin[i].u4FV[j] = rIspAFStat.rAFWinResult[i][j].u.bits.AFN_SUM;
        }
        a_r3AStat.rAFStat.u4Mean[i] = rIspAFStat.rAFMean[i].u.bits.AFGSUMN;
    }

    for (j = 0; j < HAL_3A_AF_THRESHOLD_NUM; j++) {
        a_r3AStat.rAFStat.sWin[HAL_3A_AF_WIN_NUM-1].u4FV[j] = rIsp3AStat.rAFWinResult[HAL_3A_AF_WIN_NUM-1][j].u.bits.AFN_SUM;
    }
    a_r3AStat.rAFStat.u4Mean[HAL_3A_AF_WIN_NUM-1] = rIsp3AStat.rAFMean[HAL_3A_AF_WIN_NUM-1].u.bits.AFGSUMN;


    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AF_STAT) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("____AF Statistics____\n");
        for (i = 0; i < HAL_3A_AF_WIN_NUM; i++) {
            for (j = 0; j < HAL_3A_AF_THRESHOLD_NUM; j++) {
                AAA_LOG("a_r3AStat.rAFStat.sWin[%d].u4FV[%d] = %d \n", i, j, a_r3AStat.rAFStat.sWin[i].u4FV[j]);
            }
            AAA_LOG("a_r3AStat.rAFStat.u4Mean[%d] = %d \n", i, a_r3AStat.rAFStat.u4Mean[i]);
        }
    }

    //____AWB Statistics____

    // AWB XY Window Result
    for (i = 0; i < HAL_3A_AWB_WIN_NUM; i++) {
        a_r3AStat.rAWBStat.AWBXY_RESULT[i].u4PCNT = rIsp3AStat.rAWBXYResult[i].PCNT;
        a_r3AStat.rAWBStat.AWBXY_RESULT[i].u4RSUM = rIsp3AStat.rAWBXYResult[i].RSUM;
        a_r3AStat.rAWBStat.AWBXY_RESULT[i].u4GSUM = rIsp3AStat.rAWBXYResult[i].GSUM;
        a_r3AStat.rAWBStat.AWBXY_RESULT[i].u4BSUM = rIsp3AStat.rAWBXYResult[i].BSUM;
    }

    // AWB Sum Window Result
    a_r3AStat.rAWBStat.AWBSUM_RESULT.u4PCNT = rIsp3AStat.rAWBSumResult.PCNT;
    a_r3AStat.rAWBStat.AWBSUM_RESULT.u4RSUM = rIsp3AStat.rAWBSumResult.RSUM;
    a_r3AStat.rAWBStat.AWBSUM_RESULT.u4GSUM = rIsp3AStat.rAWBSumResult.GSUM;
    a_r3AStat.rAWBStat.AWBSUM_RESULT.u4BSUM = rIsp3AStat.rAWBSumResult.BSUM;

    // AWB Color Edge Window Result
    a_r3AStat.rAWBStat.AWBCE_RESULT.u4PCNT = rIsp3AStat.rAWBCEResult.PCNT;
    a_r3AStat.rAWBStat.AWBCE_RESULT.u4RSUM = rIsp3AStat.rAWBCEResult.RSUM;
    a_r3AStat.rAWBStat.AWBCE_RESULT.u4GSUM = rIsp3AStat.rAWBCEResult.GSUM;
    a_r3AStat.rAWBStat.AWBCE_RESULT.u4BSUM = rIsp3AStat.rAWBCEResult.BSUM;

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_AWB_STAT) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("____AWB Statistics____\n");
        for (i = 0; i < HAL_3A_AWB_WIN_NUM; i++) {
            AAA_LOG("a_r3AStat.rAWBStat.AWBXY_RESULT[%d].u4PCNT = %d \n", i, a_r3AStat.rAWBStat.AWBXY_RESULT[i].u4PCNT);
            AAA_LOG("a_r3AStat.rAWBStat.AWBXY_RESULT[%d].u4RSUM = %d \n", i, a_r3AStat.rAWBStat.AWBXY_RESULT[i].u4RSUM);
	    AAA_LOG("a_r3AStat.rAWBStat.AWBXY_RESULT[%d].u4GSUM = %d \n", i, a_r3AStat.rAWBStat.AWBXY_RESULT[i].u4GSUM);
	    AAA_LOG("a_r3AStat.rAWBStat.AWBXY_RESULT[%d].u4BSUM = %d \n", i, a_r3AStat.rAWBStat.AWBXY_RESULT[i].u4BSUM);
        }

	    AAA_LOG("a_r3AStat.rAWBStat.AWBSUM_RESULT.u4PCNT = %d \n", a_r3AStat.rAWBStat.AWBSUM_RESULT.u4PCNT);
	    AAA_LOG("a_r3AStat.rAWBStat.AWBSUM_RESULT.u4RSUM = %d \n", a_r3AStat.rAWBStat.AWBSUM_RESULT.u4RSUM);
	    AAA_LOG("a_r3AStat.rAWBStat.AWBSUM_RESULT.u4GSUM = %d \n", a_r3AStat.rAWBStat.AWBSUM_RESULT.u4GSUM);
	    AAA_LOG("a_r3AStat.rAWBStat.AWBSUM_RESULT.u4BSUM = %d \n", a_r3AStat.rAWBStat.AWBSUM_RESULT.u4BSUM);
	    AAA_LOG("a_r3AStat.rAWBStat.AWBCE_RESULT.u4PCNT = %d \n", a_r3AStat.rAWBStat.AWBCE_RESULT.u4PCNT);
	    AAA_LOG("a_r3AStat.rAWBStat.AWBCE_RESULT.u4RSUM = %d \n", a_r3AStat.rAWBStat.AWBCE_RESULT.u4RSUM);
	    AAA_LOG("a_r3AStat.rAWBStat.AWBCE_RESULT.u4GSUM = %d \n", a_r3AStat.rAWBStat.AWBCE_RESULT.u4GSUM);
	    AAA_LOG("a_r3AStat.rAWBStat.AWBCE_RESULT.u4BSUM = %d \n", a_r3AStat.rAWBStat.AWBCE_RESULT.u4BSUM);
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setIspFlare(
    strFlareCFG &a_rFlareConfig
)
{
    MINT32 err = MHAL_NO_ERROR;
	struct {
		cam_flregain_t rGain; // 0x00B0
		cam_flreoff_t rOffset; // 0x00B4
	} rIspFlare;

    if (m_bLockExposureSetting) {
        return MHAL_NO_ERROR;
    }

	if ((a_rFlareConfig.uFlareRGain == 0) ||
        (a_rFlareConfig.uFlareGGain == 0) ||
        (a_rFlareConfig.uFlareBGain == 0)) {
        AAA_ERR("setFlare() error, %d, %d, %d \n", a_rFlareConfig.uFlareRGain,
			                                       a_rFlareConfig.uFlareGGain,
			                                       a_rFlareConfig.uFlareBGain);
        return MHAL_INVALID_PARA;
    }

    memset((void *)&rIspFlare, 0, sizeof(rIspFlare));

    if (m_bFlareAuto == TRUE) {
        rIspFlare.rGain.u.bits.FLARE_BGAIN = (MUINT32)a_rFlareConfig.uFlareBGain;
		rIspFlare.rGain.u.bits.FLARE_GGAIN = (MUINT32)a_rFlareConfig.uFlareGGain;
        rIspFlare.rGain.u.bits.FLARE_RGAIN = (MUINT32)a_rFlareConfig.uFlareRGain;

        rIspFlare.rOffset.u.bits.FLARE_B = (MUINT32)a_rFlareConfig.uFlare_B;
		rIspFlare.rOffset.u.bits.SIGN_B = 1;
        rIspFlare.rOffset.u.bits.FLARE_G = (MUINT32)a_rFlareConfig.uFlare_G;
		rIspFlare.rOffset.u.bits.SIGN_G = 1;
        rIspFlare.rOffset.u.bits.FLARE_R = (MUINT32)a_rFlareConfig.uFlare_R;
		rIspFlare.rOffset.u.bits.SIGN_R = 1;

    }
    else { // manual
        MUINT32 u4flareGain;

        AAA_LOG("m_bFlareAuto = %d; m_u4PreviewFlareOffset = %d\n", m_bFlareAuto, m_u4PreviewFlareOffset);

		if(m_u4PreviewFlareOffset < 128) {
            u4flareGain = 128 * 128 /(128 - m_u4PreviewFlareOffset);

			rIspFlare.rGain.u.bits.FLARE_BGAIN = u4flareGain;
            rIspFlare.rGain.u.bits.FLARE_GGAIN = u4flareGain;
		    rIspFlare.rGain.u.bits.FLARE_RGAIN = u4flareGain;
			rIspFlare.rOffset.u.bits.FLARE_B = m_u4PreviewFlareOffset;
		    rIspFlare.rOffset.u.bits.SIGN_B = 1;
            rIspFlare.rOffset.u.bits.FLARE_G = m_u4PreviewFlareOffset;
		    rIspFlare.rOffset.u.bits.SIGN_G = 1;
            rIspFlare.rOffset.u.bits.FLARE_R = m_u4PreviewFlareOffset;
		    rIspFlare.rOffset.u.bits.SIGN_R = 1;
        }
        else {
            AAA_ERR("Flare over limit:%d \n", m_u4PreviewFlareOffset);
			return MHAL_INVALID_PARA;
        }
    }

#if 1 // FIXME: SW double buffer
	err = writeIspRegs(0x00B0, 2, (MUINT32 *)&rIspFlare);
	if (err < 0) {
		AAA_ERR("writeIspRegs() error");
		return err;
    }
#else
    ISP_REG(m_pIspReg, CAM_FLREGAIN) = rIspFlare.rGain.u.reg;
    ISP_REG(m_pIspReg, CAM_FLREOFF) = rIspFlare.rOffset.u.reg;
#endif

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ISP) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("____Flare____\n");
	AAA_LOG("rIspFlare.rGain.u.bits.FLARE_BGAIN = %d \n", rIspFlare.rGain.u.bits.FLARE_BGAIN);
	AAA_LOG("rIspFlare.rGain.u.bits.FLARE_GGAIN = %d \n", rIspFlare.rGain.u.bits.FLARE_GGAIN);
	AAA_LOG("rIspFlare.rGain.u.bits.FLARE_RGAIN = %d \n", rIspFlare.rGain.u.bits.FLARE_RGAIN);
	AAA_LOG("rIspFlare.rOffset.u.bits.FLARE_B = %d \n", rIspFlare.rOffset.u.bits.FLARE_B);
	AAA_LOG("rIspFlare.rOffset.u.bits.FLARE_G = %d \n", rIspFlare.rOffset.u.bits.FLARE_G);
	AAA_LOG("rIspFlare.rOffset.u.bits.FLARE_R = %d \n", rIspFlare.rOffset.u.bits.FLARE_R);
        AAA_LOG("[0x00B0] 0x%08x\n", rIspFlare.rGain.u.reg);
	AAA_LOG("ISP_REG(m_pIspReg, CAM_FLREGAIN) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_FLREGAIN));
        AAA_LOG("[0x00B4] 0x%08x\n", rIspFlare.rOffset.u.reg);
	AAA_LOG("ISP_REG(m_pIspReg, CAM_FLREGAIN) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_FLREOFF));
    }

    return err;

}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setIspAWBGain(
    AWB_GAIN_T &a_rAWBGain,
    MBOOL bSyncWithVD
)
{
    MINT32 err = MHAL_NO_ERROR;
	struct {
		cam_rgbgain1_t rCtrl1; // 0x0064
		cam_rgbgain2_t rCtrl2; // 0x0068
	} rIspAWBGain;

    memset((void *)&rIspAWBGain, 0, sizeof(rIspAWBGain));

	rIspAWBGain.rCtrl2.u.bits.R_GAIN = (a_rAWBGain.u4R >> 2);
    rIspAWBGain.rCtrl2.u.bits.R_GAIN_EXT = (a_rAWBGain.u4R & 0x03);
	rIspAWBGain.rCtrl2.u.bits.GR_GAIN = (a_rAWBGain.u4G >> 2);
    rIspAWBGain.rCtrl2.u.bits.GR_GAIN_EXT = (a_rAWBGain.u4G & 0x03);
	rIspAWBGain.rCtrl1.u.bits.GB_GAIN = (a_rAWBGain.u4G >> 2);
    rIspAWBGain.rCtrl1.u.bits.GB_GAIN_EXT = (a_rAWBGain.u4G & 0x03);
	rIspAWBGain.rCtrl1.u.bits.B_GAIN = (a_rAWBGain.u4B >> 2);
    rIspAWBGain.rCtrl1.u.bits.B_GAIN_EXT = (a_rAWBGain.u4B & 0x03);

    if (bSyncWithVD) {// SW double buffer
	    err = writeIspRegs(0x0064, 2, (MUINT32 *)&rIspAWBGain);
	    if (err < 0) {
		    AAA_ERR("writeIspRegs() error");
		    return err;
        }
    }
    else {
        ISP_REG(m_pIspReg, CAM_RGBGAIN1) = rIspAWBGain.rCtrl1.u.reg;
        ISP_REG(m_pIspReg, CAM_RGBGAIN2) = rIspAWBGain.rCtrl2.u.reg;
    }

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ISP) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("____AWB Gain____\n");
	AAA_LOG("rIspAWBGain.rCtrl2.u.bits.R_GAIN = %d \n", rIspAWBGain.rCtrl2.u.bits.R_GAIN);
        AAA_LOG("rIspAWBGain.rCtrl2.u.bits.R_GAIN_EXT = %d \n", rIspAWBGain.rCtrl2.u.bits.R_GAIN_EXT);
	AAA_LOG("rIspAWBGain.rCtrl2.u.bits.GR_GAIN = %d \n", rIspAWBGain.rCtrl2.u.bits.GR_GAIN);
        AAA_LOG("rIspAWBGain.rCtrl2.u.bits.GR_GAIN_EXT = %d \n", rIspAWBGain.rCtrl2.u.bits.GR_GAIN_EXT);
	AAA_LOG("rIspAWBGain.rCtrl1.u.bits.GB_GAIN = %d \n", rIspAWBGain.rCtrl1.u.bits.GB_GAIN);
        AAA_LOG("rIspAWBGain.rCtrl1.u.bits.GB_GAIN_EXT = %d \n", rIspAWBGain.rCtrl1.u.bits.GB_GAIN_EXT);
	AAA_LOG("rIspAWBGain.rCtrl1.u.bits.B_GAIN = %d \n", rIspAWBGain.rCtrl1.u.bits.B_GAIN);
        AAA_LOG("rIspAWBGain.rCtrl1.u.bits.B_GAIN_EXT = %d \n", rIspAWBGain.rCtrl1.u.bits.B_GAIN_EXT);
        AAA_LOG("[0x0064] 0x%08x\n", rIspAWBGain.rCtrl1.u.reg);
	AAA_LOG("ISP_REG(m_pIspReg, CAM_RGBGAIN1) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_RGBGAIN1));
        AAA_LOG("[0x0068] 0x%08x\n", rIspAWBGain.rCtrl2.u.reg);
	AAA_LOG("ISP_REG(m_pIspReg, CAM_RGBGAIN2) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_RGBGAIN2));
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setIspRAWGain(
    MUINT32 u4IncreaseGain,
    MBOOL bBypassOBGain
)
{
    MINT32 err = MHAL_NO_ERROR;
	MUINT32 u4RAWGain_R, u4RAWGain_Gr, u4RAWGain_Gb, u4RAWGain_B;
	struct {
		cam_rawgain0_t rCtrl0; // 0x016C
		cam_rawgain1_t rCtrl1; // 0x0170
	} rIspRAWGain;

    if (m_bLockExposureSetting) {
        return MHAL_NO_ERROR;
    }

    memset(&rIspRAWGain, 0, sizeof(rIspRAWGain));

    if (bBypassOBGain) {
        u4RAWGain_R = u4IncreaseGain;
        u4RAWGain_Gr = u4IncreaseGain;
        u4RAWGain_Gb = u4IncreaseGain;
        u4RAWGain_B = u4IncreaseGain;
    }
	else {
        MUINT32 u4OBLevel_R = ISP_BITS(m_pIspReg, CAM_RGBOFF, OFF11);
        MUINT32 u4OBLevel_Gr = ISP_BITS(m_pIspReg, CAM_RGBOFF, OFF10);
        MUINT32 u4OBLevel_Gb = ISP_BITS(m_pIspReg, CAM_RGBOFF, OFF01);
        MUINT32 u4OBLevel_B = ISP_BITS(m_pIspReg, CAM_RGBOFF, OFF00);

        MUINT32 u4DefaultISPRAWGain_R =  (128 * 1024 + ((1024 - u4OBLevel_R) >> 1)) / (1024 - u4OBLevel_R);
        MUINT32 u4DefaultISPRAWGain_Gr =  (128 * 1024 + ((1024 - u4OBLevel_Gr) >> 1)) / (1024 - u4OBLevel_Gr);
        MUINT32 u4DefaultISPRAWGain_Gb =  (128 * 1024 + ((1024 - u4OBLevel_Gb) >> 1)) / (1024 - u4OBLevel_Gb);
        MUINT32 u4DefaultISPRAWGain_B =  (128 * 1024 + ((1024 - u4OBLevel_B) >> 1)) / (1024 - u4OBLevel_B);

        // Set raw gain
        u4RAWGain_R = (u4DefaultISPRAWGain_R * u4IncreaseGain + 64) >> 7;
        u4RAWGain_Gr = (u4DefaultISPRAWGain_Gr * u4IncreaseGain + 64) >> 7;
        u4RAWGain_Gb = (u4DefaultISPRAWGain_Gb * u4IncreaseGain + 64) >> 7;
        u4RAWGain_B = (u4DefaultISPRAWGain_B * u4IncreaseGain + 64) >> 7;
	}

//	if ((u4RAWGain < 128) || (u4RAWGain > 511)) {
	if ((u4RAWGain_R < 32)   ||
        (u4RAWGain_R > 511)  ||
        (u4RAWGain_Gr < 32)  ||
        (u4RAWGain_Gr > 511) ||
        (u4RAWGain_Gb < 32)  ||
        (u4RAWGain_Gb > 511) ||
        (u4RAWGain_B < 32)   ||
        (u4RAWGain_B > 511)) {     // for HDR mode, the raw gain maybe 1x/4
        AAA_ERR("setRAWGain() error: u4RAWGain_R = %d, u4RAWGain_Gr = %d, u4RAWGain_Gb = %d, u4RAWGain_B = %d \n",
            u4RAWGain_R, u4RAWGain_Gr, u4RAWGain_Gb, u4RAWGain_B);
        return MHAL_INVALID_PARA;
    }

    rIspRAWGain.rCtrl0.u.bits.RAW_RGAIN = u4RAWGain_R;
	rIspRAWGain.rCtrl0.u.bits.RAW_GRGAIN = u4RAWGain_Gr;
	rIspRAWGain.rCtrl1.u.bits.RAW_GBGAIN = u4RAWGain_Gb;
	rIspRAWGain.rCtrl1.u.bits.RAW_BGAIN = u4RAWGain_B;

#if 1 // FIXME: SW double buffer
	err = writeIspRegs(0x016C, 2, (MUINT32 *)&rIspRAWGain);
	if (err < 0) {
		AAA_ERR("writeIspRegs() error");
		return err;
    }
#else
    ISP_REG(m_pIspReg, CAM_RAWGAIN0) = rIspRAWGain.rCtrl0.u.reg;
    ISP_REG(m_pIspReg, CAM_RAWGAIN1) = rIspRAWGain.rCtrl1.u.reg;
#endif


    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ISP) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("____RAW Gain____\n");
        AAA_LOG("u4IncreaseGain = %d \n", u4IncreaseGain);
	AAA_LOG("rIspRAWGain.rCtrl0.u.bits.RAW_RGAIN = %d \n", rIspRAWGain.rCtrl0.u.bits.RAW_RGAIN);
	AAA_LOG("rIspRAWGain.rCtrl0.u.bits.RAW_GRGAIN = %d \n", rIspRAWGain.rCtrl0.u.bits.RAW_GRGAIN);
	AAA_LOG("rIspRAWGain.rCtrl1.u.bits.RAW_GBGAIN = %d \n", rIspRAWGain.rCtrl1.u.bits.RAW_GBGAIN);
	AAA_LOG("rIspRAWGain.rCtrl1.u.bits.RAW_BGAIN = %d \n", rIspRAWGain.rCtrl1.u.bits.RAW_BGAIN);
        AAA_LOG("[0x0016C] 0x%08x\n", rIspRAWGain.rCtrl0.u.reg);
	AAA_LOG("ISP_REG(m_pIspReg, CAM_RAWGAIN0) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_RAWGAIN0));
        AAA_LOG("[0x00170] 0x%08x\n", rIspRAWGain.rCtrl1.u.reg);
	AAA_LOG("ISP_REG(m_pIspReg, CAM_RAWGAIN1) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_RAWGAIN1));
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setCaptureIspRAWGain(
    MUINT32 u4IncreaseGain
)
{
    MINT32 err = MHAL_NO_ERROR;
	MUINT32 u4RAWGain_R, u4RAWGain_Gr, u4RAWGain_Gb, u4RAWGain_B;
	struct {
		cam_rawgain0_t rCtrl0; // 0x016C
		cam_rawgain1_t rCtrl1; // 0x0170
	} rIspRAWGain;

    if (m_bLockExposureSetting) {
        return MHAL_NO_ERROR;
    }

    memset(&rIspRAWGain, 0, sizeof(rIspRAWGain));

    MUINT32 u4OBLevel_R = ISP_BITS(m_pIspReg, CAM_RGBOFF, OFF11);
    MUINT32 u4OBLevel_Gr = ISP_BITS(m_pIspReg, CAM_RGBOFF, OFF10);
    MUINT32 u4OBLevel_Gb = ISP_BITS(m_pIspReg, CAM_RGBOFF, OFF01);
    MUINT32 u4OBLevel_B = ISP_BITS(m_pIspReg, CAM_RGBOFF, OFF00);

    MUINT32 u4DefaultISPRAWGain_R =  (128 * 1024 + ((1024 - u4OBLevel_R) >> 1)) / (1024 - u4OBLevel_R);
    MUINT32 u4DefaultISPRAWGain_Gr =  (128 * 1024 + ((1024 - u4OBLevel_Gr) >> 1)) / (1024 - u4OBLevel_Gr);
    MUINT32 u4DefaultISPRAWGain_Gb =  (128 * 1024 + ((1024 - u4OBLevel_Gb) >> 1)) / (1024 - u4OBLevel_Gb);
    MUINT32 u4DefaultISPRAWGain_B =  (128 * 1024 + ((1024 - u4OBLevel_B) >> 1)) / (1024 - u4OBLevel_B);

    // Set raw gain
    u4RAWGain_R = (u4DefaultISPRAWGain_R * u4IncreaseGain + 64) >> 7;
    u4RAWGain_Gr = (u4DefaultISPRAWGain_Gr * u4IncreaseGain + 64) >> 7;
    u4RAWGain_Gb = (u4DefaultISPRAWGain_Gb * u4IncreaseGain + 64) >> 7;
    u4RAWGain_B = (u4DefaultISPRAWGain_B * u4IncreaseGain + 64) >> 7;

    NSCamCustom::refineCaptureISPRAWGain(m_r3AOutput.rAEOutput.rCaptureMode[g_u4CaptureCnt].u4AfeGain,
                                         u4RAWGain_R,
                                         u4RAWGain_Gr,
                                         u4RAWGain_Gb,
                                         u4RAWGain_B);

//	if ((u4RAWGain < 128) || (u4RAWGain > 511)) {
	if ((u4RAWGain_R < 32)   ||
        (u4RAWGain_R > 511)  ||
        (u4RAWGain_Gr < 32)  ||
        (u4RAWGain_Gr > 511) ||
        (u4RAWGain_Gb < 32)  ||
        (u4RAWGain_Gb > 511) ||
        (u4RAWGain_B < 32)   ||
        (u4RAWGain_B > 511)) {     // for HDR mode, the raw gain maybe 1x/4
        AAA_ERR("setRAWGain() error: u4RAWGain_R = %d, u4RAWGain_Gr = %d, u4RAWGain_Gb = %d, u4RAWGain_B = %d \n",
            u4RAWGain_R, u4RAWGain_Gr, u4RAWGain_Gb, u4RAWGain_B);
        return MHAL_INVALID_PARA;
    }

    rIspRAWGain.rCtrl0.u.bits.RAW_RGAIN = u4RAWGain_R;
	rIspRAWGain.rCtrl0.u.bits.RAW_GRGAIN = u4RAWGain_Gr;
	rIspRAWGain.rCtrl1.u.bits.RAW_GBGAIN = u4RAWGain_Gb;
	rIspRAWGain.rCtrl1.u.bits.RAW_BGAIN = u4RAWGain_B;

#if 1 // FIXME: SW double buffer
	err = writeIspRegs(0x016C, 2, (MUINT32 *)&rIspRAWGain);
	if (err < 0) {
		AAA_ERR("writeIspRegs() error");
		return err;
    }
#else
    ISP_REG(m_pIspReg, CAM_RAWGAIN0) = rIspRAWGain.rCtrl0.u.reg;
    ISP_REG(m_pIspReg, CAM_RAWGAIN1) = rIspRAWGain.rCtrl1.u.reg;
#endif


    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ISP) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("____Capture RAW Gain____\n");
        AAA_LOG("u4IncreaseGain = %d \n", u4IncreaseGain);
	    AAA_LOG("rIspRAWGain.rCtrl0.u.bits.RAW_RGAIN = %d \n", rIspRAWGain.rCtrl0.u.bits.RAW_RGAIN);
	    AAA_LOG("rIspRAWGain.rCtrl0.u.bits.RAW_GRGAIN = %d \n", rIspRAWGain.rCtrl0.u.bits.RAW_GRGAIN);
	    AAA_LOG("rIspRAWGain.rCtrl1.u.bits.RAW_GBGAIN = %d \n", rIspRAWGain.rCtrl1.u.bits.RAW_GBGAIN);
	    AAA_LOG("rIspRAWGain.rCtrl1.u.bits.RAW_BGAIN = %d \n", rIspRAWGain.rCtrl1.u.bits.RAW_BGAIN);
        AAA_LOG("[0x0016C] 0x%08x\n", rIspRAWGain.rCtrl0.u.reg);
	    AAA_LOG("ISP_REG(m_pIspReg, CAM_RAWGAIN0) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_RAWGAIN0));
        AAA_LOG("[0x00170] 0x%08x\n", rIspRAWGain.rCtrl1.u.reg);
	    AAA_LOG("ISP_REG(m_pIspReg, CAM_RAWGAIN1) = 0x%08x\n", ISP_REG(m_pIspReg, CAM_RAWGAIN1));
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setSensorExpTime(
    MUINT32 a_u4ExpTime
)
{
    MINT32 err;

    if (m_bLockExposureSetting) {
        return MHAL_NO_ERROR;
    }

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_SENSOR) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("[setSensorExpTime] a_u4ExpTime = %d \n", a_u4ExpTime);
    }

    if (a_u4ExpTime == 0) {
        AAA_ERR("setSensorExpTime() error: exposure time = 0\n");
        return MHAL_INVALID_PARA;
    }

    // Set exposure time
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_EXP_TIME, &a_u4ExpTime, NULL, NULL);
    MHAL_ASSERT(err == MHAL_NO_ERROR, "Err HAL_SENSOR_PARAM_SET_EXP_TIME");

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setSensorGain(
    MUINT32 a_u4SensorGain
)
{
    MINT32 err;

    if (m_bLockExposureSetting) {
        return MHAL_NO_ERROR;
    }

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_SENSOR) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("[setSensorGain] a_u4SensorGain:%d \n", a_u4SensorGain);
    }

    if (a_u4SensorGain == 0) {
        AAA_ERR("setSensorGain() error: sensor gain = 0 \n");
        return MHAL_INVALID_PARA;
    }

    // Set sensor gain
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_GAIN, &a_u4SensorGain, NULL, NULL);
	MHAL_ASSERT(err == MHAL_NO_ERROR, "Err HAL_SENSOR_PARAM_SET_GAIN");

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setSensorFrameRate(
    MUINT32 a_u4SensorFrameRate
)
{
    MINT32 err;
    MUINT32 u4SensorFrameRate;

    if (m_bLockExposureSetting)
    {
        return MHAL_NO_ERROR;
    }

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_SENSOR) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("[setSensorFrameRate] a_u4SensorFrameRate:%d %d\n", a_u4SensorFrameRate, g_SensorFrameRate);
    }
#if 0
    if (a_u4SensorFrameRate == 0) {
        AAA_ERR("setSensorFrameRate() error: sensor frame rate = 0 \n");
        return MHAL_INVALID_PARA;
    } else
#endif
    if(a_u4SensorFrameRate == g_SensorFrameRate){
        if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_SENSOR) ||
            (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
            AAA_LOG("[setSensorFrameRate] g_SensorFrameRate:%d \n", g_SensorFrameRate);
        }
        return MHAL_NO_ERROR;
    }

    // Set sensor gain
    u4SensorFrameRate = a_u4SensorFrameRate / 10;    // 10 base frame rate from AE
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_FRAME_RATE, &u4SensorFrameRate, NULL, NULL);
    MHAL_ASSERT(err == MHAL_NO_ERROR, "Err HAL_SENSOR_PARAM_SET_FRAME_RATE");
    g_SensorFrameRate = a_u4SensorFrameRate;
    m_bFlickerState = -1;
    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setSensorFlickerFrameRate(
    MBOOL a_flickerEnable
)
{
    MINT32 err;
    MUINT32 u4FlickerInfo;

    if (m_bLockExposureSetting)
    {
        return MHAL_NO_ERROR;
    }

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_SENSOR) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("[setSensorFlickerFrameRate] a_flickerEnable:%d\n", a_flickerEnable);
    }

    if ((MINT32) a_flickerEnable == m_bFlickerState) {
        return MHAL_NO_ERROR;
    }

    // Set sensor gain
    u4FlickerInfo = (MUINT32)a_flickerEnable;
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_FLICKER_FRAME_RATE, &u4FlickerInfo, NULL, NULL);
    MHAL_ASSERT(err == MHAL_NO_ERROR, "Err HAL_SENSOR_PARAM_SET_FLICKER_FRAME_RATE");
    m_bFlickerState = (MINT32) a_flickerEnable;
   return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::getFocusInfo(
    FOCUS_INFO_T &a_rFocusInfo
)
{
    MINT32 err = MHAL_NO_ERROR;
	mcuMotorInfo rMotorInfo;

	if (m_pMcuDrvObj) {
	    err = m_pMcuDrvObj->getMCUInfo(&rMotorInfo);

        a_rFocusInfo.bIsMotorMoving = rMotorInfo.bIsMotorMoving;
        a_rFocusInfo.bIsMotorOpen   = rMotorInfo.bIsMotorOpen;
        a_rFocusInfo.i4CurrentPos   = (MINT32)rMotorInfo.u4CurrentPosition;
        a_rFocusInfo.i4MacroPos     = (MINT32)rMotorInfo.u4MacroPosition;
        a_rFocusInfo.i4InfPos       = (MINT32)rMotorInfo.u4InfPosition;
        a_rFocusInfo.bIsSupportSR   = rMotorInfo.bIsSupportSR;

            if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_SENSOR) ||
                (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
                AAA_LOG("[getFocusInfo] bIsMotorMoving = %d\n", a_rFocusInfo.bIsMotorMoving);
                AAA_LOG("[getFocusInfo] bIsMotorOpen = %d\n", a_rFocusInfo.bIsMotorOpen);
                AAA_LOG("[getFocusInfo] i4CurrentPos = %d\n", a_rFocusInfo.i4CurrentPos);
                AAA_LOG("[getFocusInfo] i4MacroPos = %d\n", a_rFocusInfo.i4MacroPos);
                AAA_LOG("[getFocusInfo] i4InfPos = %d\n", a_rFocusInfo.i4InfPos);
                AAA_LOG("[getFocusInfo] bIsSupportSR = %d\n", a_rFocusInfo.bIsSupportSR);
            }
	}

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3A::setFocusPos(
    MINT32 a_i4FocusPos
)
{
    MINT32 err = MHAL_NO_ERROR;

    if ((AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_SENSOR) ||
        (AAA_HAL_DBG_OPTION == AAA_HAL_DBG_OPTION_ALL_ON)) {
        AAA_LOG("[setFocusPos] a_i4FocusPos = %d\n", a_i4FocusPos);
    }

    if (m_pMcuDrvObj) {
        err = m_pMcuDrvObj->moveMCU(a_i4FocusPos);
    }

    return err;
}

/////////////////////////////////////////////////////////////////////////
//
// setAEMode () -
//! \brief set AE mode
//
/////////////////////////////////////////////////////////////////////////
MINT32
Hal3A::setAEMode(
    MINT32 a_i4AEMode
)
{
    MINT32 err = MHAL_NO_ERROR;

    switch (a_i4AEMode) {
        case AE_MODE_OFF: // disable AE
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_OFF);
            break;
        case AE_MODE_AUTO: // auto mode   full auto ,EV ISO LCE .. is inactive
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_AUTO);
            break;
        case AE_MODE_PROGRAM: // AE program mode , allow set EV ISO LCE ....
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_PROGRAM);
            break;
        case AE_MODE_TV: // AE TV mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_TV);
            break;
        case AE_MODE_AV: // AE AV mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_AV);
            break;
        case AE_MODE_SV: // AE SV mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_SV);
            break;
        case AE_MODE_VIDEO: // Video mode AE
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_VIDEO);
            break;
        case AE_MODE_NIGHT: // Night Scene mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_NIGHT);
            break;
        case AE_MODE_ACTION: // AE Action mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_ACTION);
            break;
        case AE_MODE_BEACH: // AE beach mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_BEACH);
            break;
        case AE_MODE_CANDLELIGHT: // AE Candlelight mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_CANDLELIGHT);
            break;
        case AE_MODE_FIREWORKS: // AE firework mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_FIREWORKS);
            break;
        case AE_MODE_LANDSCAPE: // AE landscape mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_LANDSCAPE);
            break;
        case AE_MODE_PORTRAIT: // AE portrait mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_PORTRAIT);
            break;
        case AE_MODE_NIGHT_PORTRAIT: // AE night portrait mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_NIGHT_PORTRAIT);
            break;
        case AE_MODE_PARTY: // AE party mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_PARTY);
            break;
        case AE_MODE_SNOW: // AE snow mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_SNOW);
            break;
        case AE_MODE_SPORTS: // AE sport mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_SPORTS);
            break;
        case AE_MODE_STEADYPHOTO: // AE steadyphoto mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_STEADYPHOTO);
            break;
        case AE_MODE_SUNSET: // AE sunset mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_SUNSET);
            break;
        case AE_MODE_THEATRE: // AE theatre mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_THEATRE);
            break;
        case AE_MODE_ISO_ANTI_SHAKE: // AE ISO anti shake mode
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_ISO_ANTI_SHAKE);
            break;
        case AE_MODE_BRACKET_AE:
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_BRACKET_AE);
            break;
        case AE_MODE_HDR:
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_HDR);
            break;
        case AE_MODE_AUTO_PANORAMA:
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_MODE, LIB3A_AE_MODE_AUTO_PANORAMA);
            break;
        default:
            err = MHAL_INVALID_PARA;
            break;
    }

    return err;
}

/////////////////////////////////////////////////////////////////////////
//
// setAEEVCompMode () -
//! \brief set AE EV compensation mode
//
/////////////////////////////////////////////////////////////////////////
MINT32
Hal3A::setAEEVCompMode(
    MINT32 a_i4AEEVCompMode
)
{
    MINT32 err = MHAL_NO_ERROR;

    switch (a_i4AEEVCompMode) {
        case AE_EV_COMP_00: // Disable EV compenate
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_00);
            break;
        case AE_EV_COMP_03: // EV compensate 0.3
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_03);
            break;
        case AE_EV_COMP_05: // EV compensate 0.5
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_05);
            break;
        case AE_EV_COMP_07: // EV compensate 0.7
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_07);
            break;
        case AE_EV_COMP_10: // EV compensate 1.0
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_10);
            break;
        case AE_EV_COMP_13: // EV compensate 1.3
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_13);
            break;
        case AE_EV_COMP_15: // EV compensate 1.5
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_15);
            break;
        case AE_EV_COMP_17: // EV compensate 1.7
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_17);
            break;
        case AE_EV_COMP_20: // EV compensate 2.0
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_20);
            break;
        case AE_EV_COMP_30: // EV compensate 3.0
             err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_30);
            break;
        case AE_EV_COMP_n03: // EV compensate -0.3
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_n03);
            break;
        case AE_EV_COMP_n05: // EV compensate -0.5
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_n05);
            break;
        case AE_EV_COMP_n07: // EV compensate -0.7
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_n07);
            break;
        case AE_EV_COMP_n10: // EV compensate -1.0
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_n10);
            break;
        case AE_EV_COMP_n13: // EV compensate -1.3
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_n13);
            break;
        case AE_EV_COMP_n15: // EV compensate -1.5
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_n15);
            break;
        case AE_EV_COMP_n17: // EV compensate -1.7
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_n17);
            break;
        case AE_EV_COMP_n20: // EV compensate -2.0
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_n20);
            break;
        case AE_EV_COMP_n30: // EV compensate -2.0
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_EVCOMP, LIB3A_AE_EV_COMP_n30);
            break;
        default:
            err = MHAL_INVALID_PARA;
            break;
    }

    return err;
}

/////////////////////////////////////////////////////////////////////////
//
// setAEMeteringMode () -
//! \brief set AE metering mode
//
/////////////////////////////////////////////////////////////////////////
MINT32
Hal3A::setAEMeteringMode(
    MINT32 a_i4AEMeteringMode
)
{
    MINT32 err = MHAL_NO_ERROR;

    switch (a_i4AEMeteringMode) {
        case AE_METERING_MODE_CENTER_WEIGHT: // CENTER WEIGHTED MODE
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_METERING_MODE, LIB3A_AE_METERING_MODE_CENTER_WEIGHT);
            break;
        case AE_METERING_MODE_SOPT: // SPOT MODE
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_METERING_MODE, LIB3A_AE_METERING_MODE_SOPT);
            break;
        case AE_METERING_MODE_AVERAGE: // AVERAGE MODE
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_METERING_MODE, LIB3A_AE_METERING_MODE_AVERAGE);
            break;
        default:
            err = MHAL_INVALID_PARA;
            break;
    }

    return err;
}

/////////////////////////////////////////////////////////////////////////
//
// setAEISOSpeed () -
//! \brief set AE ISO speed
//
/////////////////////////////////////////////////////////////////////////
MINT32
Hal3A::setAEISOSpeed(
    MINT32 a_i4AEISOSpeed
)
{
    MINT32 err = MHAL_NO_ERROR;

    switch (a_i4AEISOSpeed) {
        case AE_ISO_AUTO: // Auto
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_ISO_SPEED_AUTO);
            break;
        case AE_ISO_100: // ISO 100
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_ISO_SPEED_100);
            break;
        case AE_ISO_200: // ISO 200
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_ISO_SPEED_200);
            break;
        case AE_ISO_400: // ISO 400
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_ISO_SPEED_400);
            break;
        case AE_ISO_800: // ISO 800
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_ISO_SPEED_800);
            break;
        case AE_ISO_1600: // ISO 1600
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_ISO_SPEED_1600);
            break;
        case 100:
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_REAL_ISO_SPEED_100);
            break;
        case 150:
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_REAL_ISO_SPEED_150);
            break;
        case 200:
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_REAL_ISO_SPEED_200);
            break;
        case 300:
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_REAL_ISO_SPEED_300);
            break;
        case 400:
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_REAL_ISO_SPEED_400);
            break;
        case 600:
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_REAL_ISO_SPEED_600);
            break;
        case 800:
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_REAL_ISO_SPEED_800);
            break;
        case 1600:
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_ISOSPEED, LIB3A_AE_REAL_ISO_SPEED_1600);
            break;
        default:
            AAA_LOG("[setAEISOSpeed]: Error ISO value%d \n", a_i4AEISOSpeed);
            err = MHAL_INVALID_PARA;
            break;
    }

    return err;
}

/////////////////////////////////////////////////////////////////////////
//
// setAEStrobeMode () -
//! \brief set AE strobe mode
//
/////////////////////////////////////////////////////////////////////////
MINT32
Hal3A::setAEStrobeMode(
    MINT32 a_i4AEStrobeMode
)
{
    MINT32 err = MHAL_NO_ERROR;

    AAA_LOG("[setAEStrobeMode] StrobeMode=%d\n",a_i4AEStrobeMode);
    switch (a_i4AEStrobeMode)
    {
        case FLASHLIGHT_AUTO: // Auto
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_STROBE_MODE, LIB3A_AE_STROBE_MODE_AUTO);
            break;
        case FLASHLIGHT_FORCE_ON: // Force On
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_STROBE_MODE, LIB3A_AE_STROBE_MODE_FORCE_ON);
            break;
        case FLASHLIGHT_FORCE_OFF: // Force Off
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_STROBE_MODE, LIB3A_AE_STROBE_MODE_FORCE_OFF);
            break;
        case FLASHLIGHT_REDEYE: // Red-eye
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_STROBE_MODE, LIB3A_AE_STROBE_MODE_REDEYE);
            break;
        case FLASHLIGHT_TORCH :
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_STROBE_MODE, LIB3A_AE_STROBE_MODE_FORCE_TORCH);
            break;
        default:
            err = MHAL_INVALID_PARA;
            break;
    }

    return err;
}

/////////////////////////////////////////////////////////////////////////
//
// setAEFlashlightType () -
//! \brief set AE flashlight type
//
/////////////////////////////////////////////////////////////////////////
MINT32
Hal3A::setAEFlashlightType(
    MINT32 a_iAEFlashlightType
)
{
    MINT32 err = MHAL_NO_ERROR;

    AAA_LOG("[setAEFlashlightType]:%d \n",a_iAEFlashlightType);
    err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLASHLIGHT_TYPE, a_iAEFlashlightType);
    return err;
}

/////////////////////////////////////////////////////////////////////////
//
// setAEFlickerMode () -
//! \brief set AE flicker mode
//
/////////////////////////////////////////////////////////////////////////
MINT32
Hal3A::setAEFlickerMode(
    MINT32 a_i4AEFlickerMode
)
{
    MINT32 err = MHAL_NO_ERROR;
#if 1
    switch (a_i4AEFlickerMode) {
        case AE_FLICKER_MODE_60HZ: // 60Hz
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_MODE, LIB3A_AE_FLICKER_MODE_60HZ);
            break;
        case AE_FLICKER_MODE_50HZ: // 50Hz
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_MODE, LIB3A_AE_FLICKER_MODE_50HZ);
            break;
        case AE_FLICKER_MODE_AUTO: // Auto
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_MODE, LIB3A_AE_FLICKER_MODE_AUTO);
            break;
        case AE_FLICKER_MODE_OFF: // Off
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_MODE, LIB3A_AE_FLICKER_MODE_OFF);
            break;
        default:
            err = MHAL_INVALID_PARA;
            break;
    }
#else   // for auto flicker detection test.
    err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_MODE, LIB3A_AE_FLICKER_MODE_AUTO);
#endif

    return err;
}

/////////////////////////////////////////////////////////////////////////
//
// setAEFlickerAutoMode () -
//! \brief set AE flicker auto mode
//
/////////////////////////////////////////////////////////////////////////
MINT32
Hal3A::setAEFlickerAutoMode(
    MINT32 a_i4AEFlickerAutoMode
)
{
    MINT32 err = MHAL_NO_ERROR;
#if 1
    switch (a_i4AEFlickerAutoMode) {
        case HAL_FLICKER_AUTO_60HZ: // 60Hz
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_AUTO_MODE, LIB3A_AE_FLICKER_AUTO_MODE_60HZ);
            break;
        case HAL_FLICKER_AUTO_50HZ: // 50Hz
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_AUTO_MODE, LIB3A_AE_FLICKER_AUTO_MODE_50HZ);
            break;
        case HAL_FLICKER_AUTO_OFF: // Off
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_FLICKER_AUTO_MODE, LIB3A_AE_FLICKER_AUTO_MODE_OFF);
            break;
        default:
            err = MHAL_INVALID_PARA;
            break;
    }
#endif
    return err;
}

/////////////////////////////////////////////////////////////////////////
//
// setAEFrameRateMode () -
//! \brief set AE frame-rate mode
//
/////////////////////////////////////////////////////////////////////////
MINT32
Hal3A::setAEFrameRateMode(
    MINT32 a_i4AEFrameRateMode
)
{
    MINT32 err = MHAL_NO_ERROR;
    AE_MODE_CFG_T strAEOutput;
#if 0
    switch (a_i4AEFrameRateMode) {
        case FRAME_RATE_150FPS: // 15fps
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_FRAMERATE_MODE, LIB3A_AE_FRAMERATE_MODE_15FPS);
            break;
        case FRAME_RATE_300FPS: // 30fps
            err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_FRAMERATE_MODE, LIB3A_AE_FRAMERATE_MODE_30FPS);
            break;
        default:
            err = MHAL_INVALID_PARA;
            break;
    }
#else
    // The value "a_i4AEFrameRateMode" is mean the real frame rate.
    err = m_pmtk3A->send3ACmd(LIB3A_AE_CMD_ID_SET_AE_FRAMERATE_MODE, 10*a_i4AEFrameRateMode);  // 300 mean 30fps
    g_SensorFrameRate = 0;  // reset the frame rate for sensor driver
#if 0
    m_pmtk3A->getAEModeSetting(strAEOutput, AAA_STATE_MANUAL_FRAMERATE_PREVIEW);
    if((m_r3AOutput.rAEOutput.rPreviewMode.u2FrameRate != 300) && (m_r3AOutput.rAEOutput.rPreviewMode.u2FrameRate != 150)) {
        setIspRAWGain(strAEOutput.u4IspGain >> 3, TRUE);
        setSensorExpTime(strAEOutput.u4Eposuretime);
        setSensorGain(strAEOutput.u4AfeGain);
        AAA_LOG("[setAEFrameRateMode] Modify the exposure time and gain:%d %d %d\n", strAEOutput.u4Eposuretime, strAEOutput.u4AfeGain, strAEOutput.u4IspGain);
    }
#endif
#endif

    return err;
}

/////////////////////////////////////////////////////////////////////////
//
// setAFMode () -
//! \brief set AF mode
//
/////////////////////////////////////////////////////////////////////////
MINT32
Hal3A::setAFMode(
    MINT32 a_i4AFMode
)
{
    MINT32 err = MHAL_NO_ERROR;

    switch (a_i4AFMode) {
        case AF_MODE_AFS: // AF-Single Shot Mode
            err = m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_AFS);
            break;
        case AF_MODE_AFC: // AF-Continuous Mode
            err = m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_AFC);
            break;
        case AF_MODE_AFC_VIDEO: // AF-Continuous Mode (Video)
            err = m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_AFC_VIDEO);
            break;
        case AF_MODE_MACRO: // AF Macro Mode
            err = m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_MACRO);
            break;
        case AF_MODE_INFINITY: // Focus is set at infinity
            err = m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_INFINITY);
            break;
        case AF_MODE_MF: // Manual Focus Mode
            err = m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_MF);
            break;
        case AF_MODE_FULLSCAN: // AF Full Scan Mode
            err = m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_MODE, LIB3A_AF_MODE_FULLSCAN);
            break;
        default:
            err = MHAL_INVALID_PARA;
            break;
    }

    return err;
}

/////////////////////////////////////////////////////////////////////////
//
// setAFMeteringMode () -
//! \brief set AF metering mode
//
/////////////////////////////////////////////////////////////////////////
MINT32
Hal3A::setAFMeteringMode(
    MINT32 a_i4AFMeteringMode
)
{
    MINT32 err = MHAL_NO_ERROR;

    switch (a_i4AFMeteringMode) {
        case AF_METER_SPOT: // Spot Window
            err = m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_METER, LIB3A_AF_METER_SPOT);
            break;
        case AF_METER_MATRIX: // Matrix Window
            err = m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_METER, LIB3A_AF_METER_MATRIX);
            break;
        case AF_METER_MOVESPOT: // MoveSpot Window
            err = m_pmtk3A->send3ACmd(LIB3A_AF_CMD_ID_SET_AF_METER, LIB3A_AF_METER_MOVESPOT);
            break;

        default:
            err = MHAL_INVALID_PARA;
            break;
    }

    return err;
}

/////////////////////////////////////////////////////////////////////////
//
// setAWBMode () -
//! \brief set AWB mode
//
/////////////////////////////////////////////////////////////////////////
MINT32
Hal3A::setAWBMode(
    MINT32 a_i4AWBMode
)
{
    MINT32 err = MHAL_NO_ERROR;

    switch (a_i4AWBMode) {
        case AWB_MODE_AUTO: // Auto
            err = m_pmtk3A->send3ACmd(LIB3A_AWB_CMD_ID_SET_AWB_MODE, LIB3A_AWB_MODE_AUTO);
            break;
        case AWB_MODE_DAYLIGHT: // Daylight
            err = m_pmtk3A->send3ACmd(LIB3A_AWB_CMD_ID_SET_AWB_MODE, LIB3A_AWB_MODE_DAYLIGHT);
            break;
        case AWB_MODE_CLOUDY_DAYLIGHT: // Cloudy daylight
            err = m_pmtk3A->send3ACmd(LIB3A_AWB_CMD_ID_SET_AWB_MODE, LIB3A_AWB_MODE_CLOUDY_DAYLIGHT);
            break;
        case AWB_MODE_SHADE: // Shade
            err = m_pmtk3A->send3ACmd(LIB3A_AWB_CMD_ID_SET_AWB_MODE, LIB3A_AWB_MODE_SHADE);
            break;
        case AWB_MODE_TWILIGHT: // Twilight
            err = m_pmtk3A->send3ACmd(LIB3A_AWB_CMD_ID_SET_AWB_MODE, LIB3A_AWB_MODE_TWILIGHT);
            break;
        case AWB_MODE_FLUORESCENT: // Fluorescent
            err = m_pmtk3A->send3ACmd(LIB3A_AWB_CMD_ID_SET_AWB_MODE, LIB3A_AWB_MODE_FLUORESCENT);
            break;
        case AWB_MODE_WARM_FLUORESCENT: // Warm fluorescent
            err = m_pmtk3A->send3ACmd(LIB3A_AWB_CMD_ID_SET_AWB_MODE, LIB3A_AWB_MODE_WARM_FLUORESCENT);
            break;
        case AWB_MODE_INCANDESCENT: // Incandescent
            err = m_pmtk3A->send3ACmd(LIB3A_AWB_CMD_ID_SET_AWB_MODE, LIB3A_AWB_MODE_INCANDESCENT);
            break;
        default:
            err = MHAL_INVALID_PARA;
            break;
    }

    return err;
}


void Hal3A::setFlashActive(MBOOL a_FlashActive)
{
    m_bFlashActive = a_FlashActive;
}

MBOOL Hal3A::getFlashActive(void)
{
    return m_bFlashActive;
}

/////////////////////////////////////////////////////////////////////////
//
// Author : Cotta
// Functionality : main-flash delay function, used for strobe protection
//
/////////////////////////////////////////////////////////////////////////
MVOID Hal3A::strobeDelay()
{
    MUINT32 prevEndTime, currectStartTime, strobeProtectionIntv;
	MUINT32 mainFlashTime = 0;
    aaaTimer strobeTimer("strobeTimer");

    if(m_r3AOutput.rAEOutput.rCaptureMode[0].u4StrobeWidth == 0xFF) //high current
	{
		prevEndTime = m_pmtk3A->getPrevMFEndTime();
		strobeProtectionIntv = m_pmtk3A->getStrobeProtectionIntv();

		if(prevEndTime == 0 && strobeProtectionIntv == 0) //first time capture
		{
            mainFlashTime = (strobeDelayCount + 1)* (1000 / (m_r3AOutput.rAEOutput.rCaptureMode[0].u2FrameRate / 10)); // main-flash time (ms)
                                                                                                                        // u2FrameRate is 10-based. eg : 100 -> 10fps


AAA_LOG("mainFlashTime=%d, strobeDelayCount=%d, frameRate=%d",
            	(int)mainFlashTime, (int)strobeDelayCount, (int)m_r3AOutput.rAEOutput.rCaptureMode[0].u2FrameRate);



			mainFlashTime -= strobeZSDMFOffset; // if in ZSD mode, decrease 100000 us
AAA_LOG("mainFlashTime=%d", mainFlashTime);

			prevEndTime = mainFlashTime + getMs(); // strobe end time(us)

			strobeProtectionIntv = 8 * mainFlashTime; // 8 times protection time

            AAA_LOG("[strobeDelay] First : uPrevEndTime = %u, Interval = %u\n",prevEndTime,strobeProtectionIntv);

			m_pmtk3A->setPrevMFEndTime(prevEndTime);    //set previous main flash end time
			m_pmtk3A->setStrobeProtectionIntv(strobeProtectionIntv); //set strobe protection end time
		}
		else //not the fiert time
		{
            strobeProtectionIntv = m_pmtk3A->getPrevMFEndTime() + m_pmtk3A->getStrobeProtectionIntv();  //claculate what time can main-flash work again
			currectStartTime = getMs(); // the time want to use strobe

			int sleepInterval;

			sleepInterval = UnsignedSub(strobeProtectionIntv, currectStartTime);

			AAA_LOG("[strobeDelay] Judge sleepInterval=%d end=%d protection_int=%d end+pro=%d cur=%d\n",
			    (int)sleepInterval, (int)m_pmtk3A->getPrevMFEndTime(), (int)m_pmtk3A->getStrobeProtectionIntv(), (int)strobeProtectionIntv, (int)currectStartTime);
			AAA_LOG("[strobeDelay]u Judge sleepInterval=%u end=%u protection_int=%uend+pro=%u cur=%u\n",
			    (unsigned int)sleepInterval, (unsigned int)m_pmtk3A->getPrevMFEndTime(), (unsigned int)m_pmtk3A->getStrobeProtectionIntv(), (unsigned int)strobeProtectionIntv, (unsigned int)currectStartTime);

			if(sleepInterval>0) //interval is too short
			{
               // sleepInterval = strobeProtectionIntv - currectStartTime;

				AAA_LOG("[strobeDelay] MF will sleep %u ms, currentTime = %u, protection end time = %u\n", sleepInterval,currectStartTime,strobeProtectionIntv);

				usleep(sleepInterval*1000);
			}
			else
			{
                AAA_LOG("[strobeDelay] Long enough\n");
			}

            //== update time info ==

			mainFlashTime = (strobeDelayCount + 1) * (1000 / (m_r3AOutput.rAEOutput.rCaptureMode[0].u2FrameRate / 10)); // main-flash time (us)
			                                                                                                            // u2FrameRate is 10-based. eg : 100 -> 10fps

            mainFlashTime -= strobeZSDMFOffset; // if in ZSD mode, decrease 100000 us

			prevEndTime = mainFlashTime + getMs(); // strobe end time(us)
			strobeProtectionIntv = 8 * mainFlashTime; //8 times protection time

            AAA_LOG("[strobeDelay] Update : uPrevEndTime = %u, Interval = %u\n",prevEndTime,strobeProtectionIntv);

			m_pmtk3A->setPrevMFEndTime(prevEndTime);    //set previous main flash end time
			m_pmtk3A->setStrobeProtectionIntv(strobeProtectionIntv); //set strobe protection end time
		}
    }
}


/////////////////////////////////////////////////////////////////////////
//
// Author : Cotta
// Functionality : set ZSD mode on / off
//
/////////////////////////////////////////////////////////////////////////
MVOID Hal3A::set3AZSDMode(MBOOL ZSDFlag)
{
    // === strobe ===

    strobeZSDMode = ZSDFlag;

    if(strobeZSDMode == 1)
    {
        strobeZSDMFOffset = 100;
    }
    else
    {
        strobeZSDMFOffset = 0;
    }

    AAA_LOG("[set3AZSDMode] strobeZSDMode=%d, strobeZSDMFOffset=%u\n",strobeZSDMode,strobeZSDMFOffset);
}


MVOID Hal3A::setZSDMode(MBOOL ZSDFlag)
{
    m_pmtk3A->setZSDMode(ZSDFlag);
}

MVOID Hal3A::setBypassSensorSetting(MBOOL bBypassSensorSetting)
{
    AAA_LOG("[setBypassSensorSetting] bBypassSensorSetting:%d\n", bBypassSensorSetting);
    g_bBypassSensorSetting = bBypassSensorSetting;
}


// For ICS4.0 provide some APIs for control.
MVOID Hal3A::setAutoExposureLock(MBOOL bLockAE)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    AAA_LOG("[setAutoExposureLock] bLockAE:%d\n", bLockAE);

    if(bLockAE == TRUE) {   // disable AE
     	m_pmtk3A->lockAE();
    } else {
     	m_pmtk3A->unlockAE();
    }
}

MBOOL Hal3A::getAutoExposureLock()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    return (m_pmtk3A->isAELocked());
}

MBOOL Hal3A::isAutoExposureLockSupported()
{
    return TRUE;
}

MINT32 Hal3A::getMaxNumMeteringAreas()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    AAA_LOG("[getMaxNumMeteringAreas] Max num:%d \n", m_pmtk3A->getAEMaxNumMeteringAreas());
    return m_pmtk3A->getAEMaxNumMeteringAreas();
}

MVOID Hal3A::getMeteringAreas(MINT32 &a_i4Cnt, AREA_T **a_psAEArea)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    m_pmtk3A->getAEMeteringAreas(a_i4Cnt, a_psAEArea);
}

MVOID Hal3A::setMeteringAreas(MINT32 a_i4Cnt, AREA_T const *a_psAEArea)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");
    m_pmtk3A->setAEMeteringAreas(a_i4Cnt, a_psAEArea);
}

// For ZSD used
MBOOL Hal3A::isStrobeOn()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");
    AAA_LOG("isStrobeOn:%d \n", m_pmtk3A->isStrobeOn());
    return m_pmtk3A->isStrobeOn();
}

/////////////////////////////////////////////////////////////////////////
//
//   setHal3AMeteringModeStatus() -
//! \brief set 3A metering mode to let AE understand the current mode.
//!
//! \param [in] a_i4MeteringModeStatus 0 : video preview, 1:video recording, 2:camera preview
//!
//!
//!
//! \return 0 if success, 1 if failed.
//
 /////////////////////////////////////////////////////////////////////////
MRESULT Hal3A::setHal3AMeteringModeStatus(MINT32 a_i4MeteringModeStatus)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");
    m_pmtk3A->set3AMeteringModeStatus(a_i4MeteringModeStatus);
    return 0;
}

/////////////////////////////////////////////////////////////////////////
//
// isAutoWhiteBalanceLockSupported () -
//! \brief [ICS] returns true if auto-white balance locking is supported
//
/////////////////////////////////////////////////////////////////////////
MBOOL Hal3A::isAutoWhiteBalanceLockSupported()
{
    return MTRUE;
}

/////////////////////////////////////////////////////////////////////////
//
// getAutoWhiteBalanceLock () -
//! \brief [ICS] Gets the state of the auto-white balance lock: Returns true if auto-white balance is currently locked, and false otherwise
//
/////////////////////////////////////////////////////////////////////////
MBOOL Hal3A::getAutoWhiteBalanceLock()
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    return (m_pmtk3A->isAWBLocked());
}

/////////////////////////////////////////////////////////////////////////
//
// setAutoWhiteBalanceLock () -
//! \brief [ICS] If set to true, the camera auto-white balance routine will immediately pause until the lock is set to false.
//               If auto-white balance is already locked, setting this to true again has no effect (the driver will not recalculate white balance values).
//
/////////////////////////////////////////////////////////////////////////
MVOID Hal3A::setAutoWhiteBalanceLock(MBOOL bToggle)
{
    MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    if (bToggle) { // AWB lock
        m_pmtk3A->lockAWB();
    }
    else { // AWB unlock
        m_pmtk3A->unlockAWB();
    }
}

/*******************************************************************************
// setPreviewToCaptureImage () -
//! \brief If set to zero, the camera will use the preview setting to capture image.
********************************************************************************/
MVOID Hal3A::setPreviewToCaptureImage(
    MUINT32 a_CaptureMode
)
{
    AAA_LOG("[setPreviewToCaptureImage]Capture mode = %d\n", a_CaptureMode);

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
    }
}


MVOID Hal3A::setIsBurtShootMode(MINT32 isBurst)
{
	  m_pmtk3A->setIsBurstMode(isBurst);
}

MVOID
Hal3A::setStrobeOn(MBOOL bEnable)
{

    if(bEnable)
        m_pStrobeDrvObj->setFire(1);
    else
        m_pStrobeDrvObj->setFire(0);

    return;
}

