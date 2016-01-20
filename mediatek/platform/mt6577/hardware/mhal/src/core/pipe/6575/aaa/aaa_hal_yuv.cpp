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
#define LOG_TAG "aaa_hal_yuv"

#include <stdlib.h>
#include <stdio.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>


//
#include "MediaAssert.h"
#include "aaa_hal_yuv.h"

#include "MTK3A.h"

//
#include "kd_camera_feature.h"

//#include "AppFeature.h"
#include <cutils/properties.h>

#include "faces.h"


#include "math.h"
#include "camera_custom_if.h"
#include "cct_feature.h"
/*******************************************************************************
*
********************************************************************************/
//debug
static int AAA_YUV_LOG_DBG = 0;  

#define AAA_YUV_DEBUG
#ifdef AAA_YUV_DEBUG
#define AAA_YUV_LOG(fmt, arg...)        XLOGD(" "fmt, ##arg)
#define AAA_YUV_ERR(fmt, arg...)    XLOGE("Err: %5d: "fmt, __LINE__, ##arg)
#else
#define AAA_YUV_LOG(a,...)
#define AAA_YUV_ERR(a,...)
#endif

#define AAA_YUV_LOG_LEVEL        (AAA_YUV_LOG_DBG)

#define AAA_YUV_LOGI(fmt, arg...) \
    do { \
        if (AAA_YUV_LOG_LEVEL > 0) { \
             XLOGD(fmt, ##arg); \
        } \
    }while (0)


//for yuv+af+fd
#define GRAY  0x39E7
#define GRAYRED  0x79E7
#define GRAYGREEN  0x3BE7
#define RED   0xF800
#define GREEN 0x7E0
#define WHITE 0xFFFF
#define NOCOLOR 0x0

#define AF_SCALE 15

/*******************************************************************************
*
********************************************************************************/
//
static Hal3ABase *pHal3A = NULL;

// Sensor Info
static ACDK_SENSOR_INFO_STRUCT g_rSensorInfo;
static ACDK_SENSOR_CONFIG_STRUCT g_rSensorCfgData;




/*******************************************************************************
*
********************************************************************************/
Hal3ABase*
Hal3AYuv::
getInstance(MINT32 sensorDev)
{
    AAA_YUV_LOG("[Hal3AYuv] getInstance \n");
    static Hal3AYuv singleton(sensorDev);
    singleton.setConf(sensorDev);
    return &singleton;

}

/*******************************************************************************
*
********************************************************************************/
void
Hal3AYuv::
destroyInstance()
{
    AAA_YUV_LOG("[Hal3AYuv] destroyInstance  %p\n", this);

}

/*******************************************************************************
*
********************************************************************************/
Hal3AYuv::Hal3AYuv(MINT32 sensorDev)
{
    AAA_YUV_LOG("Hal3AYuv::Hal3AYuv\n");


}

/*******************************************************************************
*
********************************************************************************/
Hal3AYuv::~Hal3AYuv()
{
    AAA_YUV_LOG("~Hal3AYuv");

    // Sensor driver
    if (m_pSensorDrvObj) {
        m_pSensorDrvObj->destroyInstance();
        m_pSensorDrvObj = NULL;
    }

   	// strobe driver
    if (m_pStrobeDrvObj) {
        m_pStrobeDrvObj->destroyInstance();
		m_pStrobeDrvObj = NULL;
    }

}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::setConf(
    MINT32 sensorDev
)
{
    AAA_YUV_LOG("[Hal3AYuv] setConf,sensorDev = 0x%x \n", sensorDev);
    MINT32 i4CurrLensId = 0;
    //sensor driver
    m_pSensorDrvObj = SensorDrv::createInstance(sensorDev);

    // strobe driver
    m_pStrobeDrvObj = StrobeDrv::createInstance();

    if (m_pStrobeDrvObj) {
        if (m_pStrobeDrvObj->getFlashlightType() == StrobeDrv::FLASHLIGHT_NONE) {
            m_pStrobeDrvObj->destroyInstance();
            m_pStrobeDrvObj = NULL;
        }
    }
    // Strobe init state
    if (m_pStrobeDrvObj) {
        m_pStrobeDrvObj->setState(0); //preview state
    }
    AAA_YUV_LOG("[YUVAAA_hal][setcof]m_pStrobeDrvObj=0x%x\n", m_pStrobeDrvObj);

    // lens driver
    i4CurrLensId = MCUDrv::getCurrLensID();
    AAA_YUV_LOG("[YUVAAA_hal][currLensId] %d", i4CurrLensId);

    m_bIsdummylens = (i4CurrLensId == SENSOR_DRIVE_LENS_ID) ? FALSE : TRUE;

    AAA_YUV_LOG("m_bIsdummylens = %d", m_bIsdummylens);

    m_e3AState = AAA_STATE_NONE;
    m_aeFlashlightType = 0;
    m_aeStrobeMode = 0;
    m_bReadyForCapture = 0;
    m_strobeTrigerBV = 64;
    m_strobeWidth = 0;
    m_awbMode = AWB_MODE_AUTO;

//for yuv+af FD: 320*240
    m_imageXS = 320;
    m_imageYS = 240;

    m_AFzone[0] = m_imageXS / 2 - m_imageXS * AF_SCALE / 200;//x0
    m_AFzone[1] = m_imageYS / 2 - m_imageYS * AF_SCALE / 200;//y0
    m_AFzone[2] = m_imageXS / 2 + m_imageXS * AF_SCALE / 200;//x1
    m_AFzone[3] = m_imageYS / 2 + m_imageYS * AF_SCALE / 200;//y1
    m_AFzone[4] = m_imageXS;
    m_AFzone[5] = m_imageYS;

    m_bFDFaceFound = FALSE;
    //m_bisPreview = FALSE;
    //m_u4AFMode = AF_MODE_INFINITY;

    memset(m_touch_zone, 0, sizeof(m_touch_zone));
    m_touch_zone[2] = 350;//left
    m_touch_zone[3] = 446;//right
    m_touch_zone[4] = 110;//up
    m_touch_zone[5] = 181;//bottom

    m_bIsMoveSpotMeter = FALSE;

    m_u4AFErrorCount = 30;//30 * 30ms = 900ms

    m_bAEStable = FALSE;

    m_bFlashActive = FALSE;
    m_bHasBeenInRect = FALSE;
    m_bDecideAFSceneDetectingColorWhite = FALSE;

    memset(&m_rFDInfo, 0, sizeof(FD_INFO_T));
    memset(&m_rEZoomInfo, 0, sizeof(EZOOM_WIN_T));
    //memset(&m_ref, 0, sizeof(SENSOR_AE_AWB_REF_STRUCT));

// For ICS4.0 provide some APIs for control.
    bIsAELocked = FALSE;
    bIsAWBLocked = FALSE;

    return MHAL_NO_ERROR;

}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::init(
    MINT32 a_i4SensorType
)
{
    AAA_YUV_LOG("Hal3AYuv::init\n");
    MINT32 err = MHAL_NO_ERROR;
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

    AAA_YUV_LOG("u4PvW = %d; u4PvH = %d\n", u4PvW, u4PvH);

    m_rEZoomInfo.u4XOffset = 0;
    m_rEZoomInfo.u4YOffset = 0;
    m_rEZoomInfo.u4XWidth = u4PvW;
    m_rEZoomInfo.u4YHeight = u4PvH;

    m_u4AFMode = AF_MODE_INFINITY;

    SENSOR_AE_AWB_REF_STRUCT ref;
    memset(&ref, 0, sizeof(SENSOR_AE_AWB_REF_STRUCT));
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_EV_AWB_REF, (MUINT32*)&ref);
    if(err < 0){
        AAA_YUV_ERR("getASDInfo error: CMD_SENSOR_GET_EV_AWB_REF\n");
        return err;
    }

    memcpy(&m_ref, &ref, sizeof(SENSOR_AE_AWB_REF_STRUCT));

    m_ref.SensorLV05LV13EVRef = ASDLog2Func(ref.SensorAERef.AeRefLV05Shutter * ref.SensorAERef.AeRefLV05Gain,
                                                        ref.SensorAERef.AeRefLV13Shutter * ref.SensorAERef.AeRefLV13Gain);
    AAA_YUV_LOG("SensorLV05LV13EVRef = %d\n", m_ref.SensorLV05LV13EVRef);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.aaa.debug", value, "0");
    AAA_YUV_LOG_DBG = atoi(value);

    char value_string[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("yuv.flash.bv", value_string, "0");
    MINT32 flash_value = atoi(value_string);

    if(flash_value != 0)
    {
        BV_THRESHOLD = flash_value;
        AAA_YUV_LOG("Get yuv flash threshold through property_get() \n");
    }
    else
    {
        BV_THRESHOLD = NSCamCustom::custom_GetYuvFlashlightThreshold();
        AAA_YUV_LOG("Get yuv flash threshold from config.cpp,%f \n",BV_THRESHOLD);
    }

    return MHAL_NO_ERROR;

}

/*******************************************************************************
*
********************************************************************************/

MINT32
Hal3AYuv::isAEFlashOn()
{
    MINT32 rtn = 0;

    if (m_pStrobeDrvObj)
    {
        m_strobeTrigerBV = calcBV();

        AAA_YUV_LOG("m_aeFlashlightType=0x%x, m_aeStrobeMode=0x%x;BV_THRESHOLD=%f,m_strobeTrigerBV=%f\n",
            m_aeFlashlightType,
            m_aeStrobeMode,
            BV_THRESHOLD,
            m_strobeTrigerBV);

        if ( FLASHLIGHT_LED_PEAK == (FLASHLIGHT_TYPE_ENUM)m_aeFlashlightType || \
             FLASHLIGHT_LED_CONSTANT == (FLASHLIGHT_TYPE_ENUM)m_aeFlashlightType) {
            if ( (LIB3A_AE_STROBE_MODE_T)LIB3A_AE_STROBE_MODE_FORCE_ON == m_aeStrobeMode ) {
                rtn = 1;
            }
            else if ((LIB3A_AE_STROBE_MODE_T)LIB3A_AE_STROBE_MODE_AUTO == m_aeStrobeMode && \
                     (BV_THRESHOLD > m_strobeTrigerBV ) ) {
                rtn = 1;
            }
        }
    }
    return rtn;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::getAFDone()
{
    //AAA_YUV_LOG("Hal3AYuv::getAFDone\n");
    //return 1;

    if(m_bIsdummylens){
        //AAA_YUV_LOG("dummy getAFDone\n");
        return 1;
    }

    MINT32 err = 0;
    MINT32 focus_status = 0xffffffff;
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_AF_STATUS, (MUINT32 * )&focus_status);
    if(err < 0){
        AAA_YUV_ERR("sendCommand() error: CMD_SENSOR_GET_AF_STATUS\n");
        return 1;
    }
    AAA_YUV_LOG("focus_status=0x%x\n",focus_status);
    if(SENSOR_AF_FOCUSED == focus_status){
        //reset sensor af error timing count
        m_u4AFErrorCount = 30;

        return 1;
        /*
        if(AF_MODE_AFS == m_u4AFMode){
            //AFS
            return 1;
        }
        else{
            //AFC
            return m_bHasBeenInRect ? 1 : 0;
        }
        */
    }
    else if(SENSOR_AF_ERROR == focus_status){
        if(!m_u4AFErrorCount){
            m_u4AFErrorCount = 30;
            return 1;
        }

        m_u4AFErrorCount--;
        return 0;

    }
    else{
        return 0;

    }

}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::getAFWinResult(
    MINT32 *a_pBuf,
    MINT32 *a_pWinW,
    MINT32 *a_pWinH
)
{
    AF_WIN_RESULT_T sAFWinResult;

    sAFWinResult.i4Count = 1;

    //MHAL_ASSERT(m_pmtk3A != NULL, "m_pmtk3A is NULL");

    //m_pmtk3A->getAFWinResult(sAFWinResult);

	//MHAL_ASSERT(sAFWinResult.i4Count <= 5, "Err");

    if (getAFDone()) {
        sAFWinResult.i4Count = 1;//m_rAFWinResult.i4Count;
        /*
        sAFWinResult.i4Width = m_rAFWinResult.i4Width;
        sAFWinResult.i4Height = m_rAFWinResult.i4Height;

        for (int i = 0; i < sAFWinResult.i4Count; i++) {
            sAFWinResult.sRect[i].i4Left   = m_rAFWinResult.sRect[i].i4Left;
            sAFWinResult.sRect[i].i4Up     = m_rAFWinResult.sRect[i].i4Up;
            sAFWinResult.sRect[i].i4Right  = m_rAFWinResult.sRect[i].i4Right;
            sAFWinResult.sRect[i].i4Bottom = m_rAFWinResult.sRect[i].i4Bottom;
        }
        */
    }

    //m_rAFWinResult = sAFWinResult;


    MINT32 err = MHAL_NO_ERROR;
    MINT32 focus_status = 0xffffffff;
    MINT32 result;
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_AF_STATUS, (MUINT32 * )&focus_status);
    if(err < 0){
        AAA_YUV_ERR("sendCommand() error: CMD_SENSOR_GET_AF_STATUS\n");
        return TRUE;
    }
    AAA_YUV_LOG("getAFWinResult focus_status=0x%x\n",focus_status);

    switch (focus_status) {
        case SENSOR_AF_IDLE:
            result = AF_MARK_NONE;
            break;

        case SENSOR_AF_FOCUSING:
            result = AF_MARK_NORMAL;
            break;

        case SENSOR_AF_FOCUSED:
            result = AF_MARK_OK;
            break;

        case SENSOR_AF_ERROR:
            result = AF_MARK_FAIL;
            break;

        case SENSOR_AF_SCENE_DETECTING:
            result = AF_MARK_NONE;
            break;

        default:
            result = AF_MARK_NONE;
            break;

    }

    for (int i = 0; i < sAFWinResult.i4Count; i++) {
        *a_pBuf++ = -149;//sAFWinResult.sRect[i].i4Left;
        *a_pBuf++ = -150;//sAFWinResult.sRect[i].i4Up;
        *a_pBuf++ = 148;//sAFWinResult.sRect[i].i4Right;
        *a_pBuf++ = 149;//sAFWinResult.sRect[i].i4Bottom;
        *a_pBuf++ = result;//sAFWinResult.sRect[i].eMarkStatus;
    }

    //*a_pWinW = sAFWinResult.i4Width;
    //*a_pWinH = sAFWinResult.i4Height;
    *a_pWinW = 2000;
    *a_pWinH = 2000;

    return sAFWinResult.i4Count;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::getFDInfo(
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
Hal3AYuv::setFDInfo(
    MUINT32 a_u4IsFDOn,
    MUINT32 a_u4Addr
)
{
    m_rFDInfo.bFDon  = (MBOOL)a_u4IsFDOn;
    m_rFDInfo.i4Width = m_imageXS;
    m_rFDInfo.i4Height = m_imageYS;

    m_bFDFaceFound = FALSE;
    int face_num = MAX_FACE_NUM;
    int max_no = FD_MAX_NO;
    AAA_YUV_LOGI("m_rFDInfo.bFDon = %d, MAX_FACE_NUM = %d, FD_MAX_NO=%d, face_addr = %p\n",
        m_rFDInfo.bFDon, face_num, max_no, a_u4Addr);

    if (m_rFDInfo.bFDon == TRUE)
    {
        if (a_u4Addr == 0)// NULL)
        {
            AAA_YUV_ERR("setFDInfo() error: NULL pointer");
            return MHAL_INVALID_PARA;
    }

        camera_face_metadata_m *presult = (camera_face_metadata_m *) a_u4Addr;
        int left, top, right, bottom;
        for (int i = 0; i < MAX_FACE_NUM; i++) {

            m_rFDInfo.sWin[i].i4Left    = presult->faces[i].rect[0] + 1000;
            m_rFDInfo.sWin[i].i4Up      = presult->faces[i].rect[1] + 1000;
            m_rFDInfo.sWin[i].i4Right   = presult->faces[i].rect[2] + 1000;
            m_rFDInfo.sWin[i].i4Bottom  = presult->faces[i].rect[3] + 1000;
            if (presult->faces[i].score == 100) {
                m_rFDInfo.sWin[i].fgPrimary = 1;

                m_bFDFaceFound = TRUE;

                //update AF info
                updateAFAeraAndZoneInfo(m_rFDInfo.sWin[i].i4Left,
                    m_rFDInfo.sWin[i].i4Up,
                    m_rFDInfo.sWin[i].i4Right,
                    m_rFDInfo.sWin[i].i4Bottom,
                    2000,
                    2000);

                //update AE info
                updateAEAeraAndZoneInfo(m_rFDInfo.sWin[i].i4Left,
                    m_rFDInfo.sWin[i].i4Up,
                    m_rFDInfo.sWin[i].i4Right,
                    m_rFDInfo.sWin[i].i4Bottom,
                    2000,
                    2000);
                MINT32 err = MHAL_NO_ERROR;
                MUINT32* zone_addr = (MUINT32*)&m_AEzone[0];
                AAA_YUV_LOG("FD setMeteringAreas, zone_addr=0x%x\n", zone_addr);
                err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_AE_WINDOW, zone_addr);

                AAA_YUV_LOGI("win[%d] isPrimary = %d ,left = %d, up = %d, right = %d, bottom = %d\n",
                i,
                m_rFDInfo.sWin[i].fgPrimary,
                m_rFDInfo.sWin[i].i4Left ,
                m_rFDInfo.sWin[i].i4Up,
                m_rFDInfo.sWin[i].i4Right,
                m_rFDInfo.sWin[i].i4Bottom);
            }
            else  {
                m_rFDInfo.sWin[i].fgPrimary = 0;
            }

        }

        m_rFDInfo.i4Width = 2000;
        m_rFDInfo.i4Height = 2000;
    }
    else
    {
        m_AFzone[0] = m_imageXS / 2 - m_imageXS * AF_SCALE / 200;//x0
        m_AFzone[1] = m_imageYS / 2 - m_imageYS * AF_SCALE / 200;//y0
        m_AFzone[2] = m_imageXS / 2 + m_imageXS * AF_SCALE / 200;//x1
        m_AFzone[3] = m_imageYS / 2 + m_imageYS * AF_SCALE / 200;//y1

        m_AEzone[0] = m_AFzone[0];
        m_AEzone[1] = m_AFzone[1];
        m_AEzone[2] = m_AFzone[2];
        m_AEzone[3] = m_AFzone[3];

    }


    AAA_YUV_LOGI("[mHal3ASetFDInfo] zone: \n       x0 = %d, y0 = %d, x1 = %d, y1 = %d, Xwindow = %d, Ywindow = %d\n",
        m_AFzone[0],
        m_AFzone[1],
        m_AFzone[2],
        m_AFzone[3],
        m_AFzone[4],
        m_AFzone[5]);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::setEZoomInfo(
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
MVOID
Hal3AYuv::getEZoomInfo(
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
void
Hal3AYuv::setAFMoveSpotPos(
    MUINT32 a_u4Xoffset,//x
    MUINT32 a_u4Yoffset,//y
    MUINT32 a_u4Width,//655  camDispParam.dispW
    MUINT32 a_u4Height,//480  camDispParam.dispH
    MUINT32 a_u4OffsetW,//72  camDispParam.dispX
    MUINT32 a_u4OffsetH,// 0    camDispParam.dispY
    MUINT8 a_uOri//camDispParam.rotate
)
{
    AAA_YUV_LOG("setAFMoveSpotPos,x=%d,y=%d,dispW=%d,dispH=%d,dispX=%d,dispY=%d,ori=%d\n",
         a_u4Xoffset,//x
         a_u4Yoffset,//y
         a_u4Width,//655  camDispParam.dispW
         a_u4Height,//480  camDispParam.dispH
         a_u4OffsetW,//72  camDispParam.dispX
         a_u4OffsetH,// 0    camDispParam.dispY
         a_uOri//camDispParam.rotate
    );
    MUINT32 iAFW, iAFH, dispW, dispH, x, y, left, right, up, bottom;

    //a_u4Width	 = 655;
    //a_u4Height = 480;

    dispW = a_u4Width;
    dispH = a_u4Height;

    iAFW = dispW * AF_SCALE /100;
    iAFH = dispH * AF_SCALE / 100;

    //a_u4Xoffset = 120;
    //a_u4Yoffset = 44;
    //a_u4OffsetW = 72;
    //a_u4OffsetH = 0;

    if(    a_u4Xoffset < a_u4OffsetW
	|| a_u4Xoffset >= a_u4OffsetW + a_u4Width
	|| a_u4Yoffset < a_u4OffsetH
	|| a_u4Yoffset >= a_u4OffsetH + a_u4Height
	)
    //{    x = 400;    y = 240;}
    {    a_u4Xoffset = a_u4OffsetW + a_u4Width / 2;    a_u4Yoffset = a_u4OffsetH + a_u4Height / 2;}

    x = a_u4Xoffset - a_u4OffsetW;
    y = a_u4Yoffset - a_u4OffsetH;


    if(x <= iAFW / 2)
    {    left = 0;    right = iAFW - 1;}
    else
        if(x < dispW - iAFW / 2)
        {    left = x -iAFW / 2;    right = x +iAFW / 2;}
        else
        {    left = dispW - iAFW - 1;    right = dispW - 1; }

    if(y <= iAFH / 2)
    {    up = 0;    bottom = iAFH - 1;}
    else
        if(y < dispH - iAFH / 2)
        {    up = y - iAFH / 2;    bottom = y + iAFH / 2;}
        else
        {    up = dispH - iAFH - 1;    bottom = dispH - 1; }

    AAA_YUV_LOG("setAFMoveSpotPos,x=%d,y=%d,left=%d,right=%d,up=%d,bottom=%d,\n",
		x,
		y,
		left,
		right,
		up,
		bottom);
    m_touch_zone[0] = x;
    m_touch_zone[1] = y;
    m_touch_zone[2] = left;
    m_touch_zone[3] = right;
    m_touch_zone[4] = up;
    m_touch_zone[5] = bottom;

//    m_bIsMoveSpotMeter = TRUE;
    AAA_YUV_LOG("m_bIsMoveSpotMeter=%d\n", m_bIsMoveSpotMeter);
    if(m_bIsMoveSpotMeter /*&& !m_rFDInfo.bFDon*/)
        {setAFMoveSpotToSensor(left, right, up, bottom, dispW, dispH, a_uOri);}

}

/*******************************************************************************
*
********************************************************************************/
void
Hal3AYuv::setAFMoveSpotToSensor(
    MUINT32 a_u4Left,
    MUINT32 a_u4Right,
    MUINT32 a_u4Up,
    MUINT32 a_u4Bottom,
    MUINT32 a_u4DispW,
    MUINT32 a_u4DispH,
    MUINT8 a_uOri
)
{
    MUINT32 iFDW, iFDH, x0, y0, x1, y1;

    iFDW = m_rFDInfo.i4Width;
    iFDH = m_rFDInfo.i4Height;

    x0 = a_u4Left * iFDW / a_u4DispW;
    y0 = a_u4Up * iFDH / a_u4DispH;
    x1 = a_u4Right * iFDW / a_u4DispW;
    y1 = a_u4Bottom * iFDH / a_u4DispH;

    m_AFzone[0] = x0;
    m_AFzone[1] = y0;
    m_AFzone[2] = x1;
    m_AFzone[3] = y1;
    m_AFzone[4] = iFDW;
    m_AFzone[5] = iFDH;

    AAA_YUV_LOG("setAFMoveSpotToSensor,x0=%d,y0=%d,x1=%d,y1=%d,FDW=%d,FDH=%d,\n",
		m_AFzone[0],
		m_AFzone[1],
		m_AFzone[2],
		m_AFzone[3],
		m_AFzone[4],
		m_AFzone[5]);
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::drawFocusRect(
    MUINT8 *a_pBuf,
    MUINT32 a_u4DispW,
    MUINT32 a_u4DispH,
    MUINT32 a_u4DispX,
    MUINT32 a_u4DispY,
    MUINT32 a_u4Rotate
)
{
    if(m_bIsdummylens){
        AAA_YUV_LOGI("dummy drawFocusRect\n");
        return 0;
    }

    AAA_YUV_LOGI("drawFocusRect,m_bIsMoveSpotMeter=%d,m_rFDInfo.bFDon=%d,m_e3AState=%d\n",
		m_bIsMoveSpotMeter,
		m_rFDInfo.bFDon,
		m_e3AState,
		m_u4AFMode);

    MINT32 err = 0;
    MINT32 focus_status = 0xffffffff;

    MINT16 *display_buffer_ptr = (MINT16*)a_pBuf;

    MINT32 i4Left, i4Right, i4Up, i4Bottom;
    MINT32 i4Width, i4Height, i4LCDWidth;
    MBOOL  fg3Line = FALSE;
    MINT32 i4LineLen;

    MINT32 i4UpLCDWidthRight0;
    MINT32 i4UpLCDWidthLeft0, i4UpLCDWidthLeft1, i4UpLCDWidthLeft2, i4UpLCDWidthLeft3, i4UpLCDWidthLeft4;
    MINT32 i4BottomLCDWidthLeft0, i4BottomLCDWidthLeft1, i4BottomLCDWidthLeft2, i4BottomLCDWidthLeft3, i4BottomLCDWidthLeft4;
    MINT32 i4WidthLineLen, i4HeightLineLen;

    MINT16 color;
    MINT16 colorgray;

    AF_WIN_RESULT_T sAF_Win_Result;

    EZOOM_WIN_T m_sEZoom;

    getEZoomInfo(m_sEZoom);

    m_sEZoom.u4XWidth = m_rFDInfo.i4Width;
    m_sEZoom.u4YHeight = m_rFDInfo.i4Height;

    memset((void*)a_pBuf, 0, a_u4DispW*a_u4DispH*2);


    if(!m_bIsMoveSpotMeter)
    {//normal

    }
    else{
        if(m_u4AFMode == AF_MODE_AFS)
        {
            if(m_e3AState != AAA_STATE_AF){
        	  //movespot meter does not draw focus rectangle when capture botton pressed

                return 0;
            }
        }
        if(m_u4AFMode == AF_MODE_AFC)
        {//continuous focus

            if(m_e3AState == AAA_STATE_AUTO_FRAMERATE_PREVIEW
               ||  m_e3AState == AAA_STATE_MANUAL_FRAMERATE_PREVIEW)
            {//in scenen detecting state, return

                err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_AF_STATUS, (MUINT32 * )&m_u4SensorFocusStatus);
                if(err < 0){
                    AAA_YUV_ERR("drawFocusRect sendCommand() error: CMD_SENSOR_GET_AF_STATUS\n");
                    return err;
                }

                if(m_u4SensorFocusStatus == SENSOR_AF_SCENE_DETECTING)
                {
                    return 0;
                }
            }
        }

    }

// getAFWinResult(sAF_Win_Result);
// yuv+af+fd support only one focus window
    sAF_Win_Result.i4Count = 1;
    sAF_Win_Result.i4Height = m_rFDInfo.i4Height;
    sAF_Win_Result.i4Width =  m_rFDInfo.i4Width;
    if(m_rFDInfo.bFDon && m_bFDFaceFound)
    {

        sAF_Win_Result.sRect[0].i4Left = m_AFzone[0];
        sAF_Win_Result.sRect[0].i4Right = m_AFzone[2];
        sAF_Win_Result.sRect[0].i4Up = m_AFzone[1];
        sAF_Win_Result.sRect[0].i4Bottom = m_AFzone[3];

    }
    else
    {

        sAF_Win_Result.sRect[0].i4Left = m_imageXS / 2 - m_imageXS * AF_SCALE / 200;
        sAF_Win_Result.sRect[0].i4Right = m_imageXS / 2 + m_imageXS * AF_SCALE / 200;
        sAF_Win_Result.sRect[0].i4Up = m_imageYS / 2 - m_imageYS * AF_SCALE / 200;
        sAF_Win_Result.sRect[0].i4Bottom = m_imageYS / 2 + m_imageYS * AF_SCALE / 200;

    }


//query sensor af status
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_AF_STATUS, (MUINT32 * )&focus_status);
    if(err < 0){
        AAA_YUV_ERR("drawFocusRect sendCommand() error: CMD_SENSOR_GET_AF_STATUS\n");
        return err;
    }
    m_bHasBeenInRect = (focus_status == SENSOR_AF_FOCUSED) ? TRUE : FALSE;

    if(focus_status == SENSOR_AF_FOCUSING
        || focus_status == SENSOR_AF_IDLE
        || (focus_status == SENSOR_AF_SCENE_DETECTING && m_bDecideAFSceneDetectingColorWhite))
    {
        sAF_Win_Result.sRect[0].eMarkStatus = AF_MARK_NORMAL;
        m_bDecideAFSceneDetectingColorWhite = FALSE;
    }
    else if(focus_status == SENSOR_AF_FOCUSED
        || (focus_status == SENSOR_AF_SCENE_DETECTING && !m_bDecideAFSceneDetectingColorWhite))
    {
        sAF_Win_Result.sRect[0].eMarkStatus = AF_MARK_OK;
    }
    else
    {
        sAF_Win_Result.sRect[0].eMarkStatus = AF_MARK_FAIL;
    }



    //if (m_bisPreview == 1) {return MHAL_NO_ERROR;}

    if (sAF_Win_Result.i4Count == 0)    {return MHAL_NO_ERROR;}



    a_u4DispX = 0;
    a_u4DispY = 0;

    for (MINT32 i=0; i<sAF_Win_Result.i4Count; i++)
    {
        if (a_u4Rotate == 1 || a_u4Rotate == 3)
        {

            if(!m_bIsMoveSpotMeter)
            {
                sAF_Win_Result.sRect[i].i4Left   = sAF_Win_Result.sRect[i].i4Left   * a_u4DispW / m_sEZoom.u4XWidth;
                sAF_Win_Result.sRect[i].i4Right  = sAF_Win_Result.sRect[i].i4Right  * a_u4DispW / m_sEZoom.u4XWidth;
                sAF_Win_Result.sRect[i].i4Up     = sAF_Win_Result.sRect[i].i4Up     * a_u4DispH  / m_sEZoom.u4YHeight;
                sAF_Win_Result.sRect[i].i4Bottom = sAF_Win_Result.sRect[i].i4Bottom * a_u4DispH  / m_sEZoom.u4YHeight;

            }
            else
            {
                if(m_u4AFMode == AF_MODE_AFS)//single focus
                {

                    sAF_Win_Result.sRect[i].i4Left   = m_touch_zone[2];
                    sAF_Win_Result.sRect[i].i4Right  = m_touch_zone[3];
                    sAF_Win_Result.sRect[i].i4Up     = m_touch_zone[4];
                    sAF_Win_Result.sRect[i].i4Bottom = m_touch_zone[5];
                }
                else
                {  //AF_MODE_AFC

                    if(m_bInSingleFocus)
                    {
                        sAF_Win_Result.sRect[i].i4Left   = m_touch_zone[2];
                        sAF_Win_Result.sRect[i].i4Right  = m_touch_zone[3];
                        sAF_Win_Result.sRect[i].i4Up     = m_touch_zone[4];
                        sAF_Win_Result.sRect[i].i4Bottom = m_touch_zone[5];

                    }
                    else
                    {
                        sAF_Win_Result.sRect[i].i4Left   = a_u4DispW / 2 - a_u4DispW * AF_SCALE / 200;//x0;
                        sAF_Win_Result.sRect[i].i4Right  = a_u4DispW / 2 + a_u4DispW * AF_SCALE / 200;//x1;
                        sAF_Win_Result.sRect[i].i4Up     = a_u4DispH / 2 - a_u4DispH * AF_SCALE / 200;//y0;
                        sAF_Win_Result.sRect[i].i4Bottom = a_u4DispH / 2 + a_u4DispH * AF_SCALE / 200;//y1;
                    }

                }
            }


            i4Left   = a_u4DispH - 1 - sAF_Win_Result.sRect[i].i4Bottom + a_u4DispY;
            i4Right  = a_u4DispH - 1 - sAF_Win_Result.sRect[i].i4Up     + a_u4DispY;
            i4Up     = sAF_Win_Result.sRect[i].i4Left  + a_u4DispX;
            i4Bottom = sAF_Win_Result.sRect[i].i4Right + a_u4DispX;
            i4LCDWidth = a_u4DispH;

/*
            if (m_sAFNvram.i4TUNE_PARA1 == 1)
            {
                LOGD("[sAF_Win_Result]%6d %6d %6d %6d\n", sAF_Win_Result.sRect[i].i4Left, sAF_Win_Result.sRect[i].i4Right, sAF_Win_Result.sRect[i].i4Up, sAF_Win_Result.sRect[i].i4Bottom);
                LOGD("[]%6d %6d %6d %6d\n", i4Left, i4Right, i4Up, i4Bottom);
            }
*/
            if (i4Left   >= (INT32)a_u4DispH) {i4Left   = a_u4DispH-1;}
            if (i4Right  >= (INT32)a_u4DispH) {i4Right  = a_u4DispH-1;}
            if (i4Up     >= (INT32)a_u4DispW)  {i4Up     = a_u4DispW-1;}
            if (i4Bottom >= (INT32)a_u4DispW)  {i4Bottom = a_u4DispW-1;}
        }
        else
        {
            if(!m_bIsMoveSpotMeter)
            {
                sAF_Win_Result.sRect[i].i4Left   = sAF_Win_Result.sRect[i].i4Left   * a_u4DispW / m_sEZoom.u4XWidth;
                sAF_Win_Result.sRect[i].i4Right  = sAF_Win_Result.sRect[i].i4Right  * a_u4DispW / m_sEZoom.u4XWidth;
                sAF_Win_Result.sRect[i].i4Up     = sAF_Win_Result.sRect[i].i4Up     * a_u4DispH  / m_sEZoom.u4YHeight;
                sAF_Win_Result.sRect[i].i4Bottom = sAF_Win_Result.sRect[i].i4Bottom * a_u4DispH  / m_sEZoom.u4YHeight;

            }
            else
            {
                if(m_u4AFMode == AF_MODE_AFS)//single focus
                {
                    sAF_Win_Result.sRect[i].i4Left   = m_touch_zone[2];
                    sAF_Win_Result.sRect[i].i4Right  = m_touch_zone[3];
                    sAF_Win_Result.sRect[i].i4Up     = m_touch_zone[4];
                    sAF_Win_Result.sRect[i].i4Bottom = m_touch_zone[5];
                }
                else//AF_MODE_AFC continuous focus
                {
                    if(m_bInSingleFocus)
                    {
                        sAF_Win_Result.sRect[i].i4Left   = m_touch_zone[2];
                        sAF_Win_Result.sRect[i].i4Right  = m_touch_zone[3];
                        sAF_Win_Result.sRect[i].i4Up     = m_touch_zone[4];
                        sAF_Win_Result.sRect[i].i4Bottom = m_touch_zone[5];
                    }
                    else
                    {
                        sAF_Win_Result.sRect[i].i4Left   = a_u4DispW / 2 - a_u4DispW * AF_SCALE / 200;//x0;
                        sAF_Win_Result.sRect[i].i4Right  = a_u4DispW / 2 + a_u4DispW * AF_SCALE / 200;//x1;
                        sAF_Win_Result.sRect[i].i4Up     = a_u4DispH / 2 - a_u4DispH * AF_SCALE / 200;//y0;
                        sAF_Win_Result.sRect[i].i4Bottom = a_u4DispH / 2 + a_u4DispH * AF_SCALE / 200;//y1;
                    }

                }
            }


            i4Left   = sAF_Win_Result.sRect[i].i4Left   + a_u4DispX;
            i4Right  = sAF_Win_Result.sRect[i].i4Right  + a_u4DispX;
            i4Up     = sAF_Win_Result.sRect[i].i4Up     + a_u4DispY;
            i4Bottom = sAF_Win_Result.sRect[i].i4Bottom + a_u4DispY;
            i4LCDWidth = a_u4DispW;

            if (i4Left   >= (INT32)a_u4DispW)   {i4Left   = a_u4DispW-1;}
            if (i4Right  >= (INT32)a_u4DispW)   {i4Right  = a_u4DispW-1;}
            if (i4Up     >= (INT32)a_u4DispH)  {i4Up     = a_u4DispH-1;}
            if (i4Bottom >= (INT32)a_u4DispH)  {i4Bottom = a_u4DispH-1;}
        }

        //AAA_YUV_LOG("KY, a_u4DispW = %d, a_u4DispH = %d, i4Left = %d, i4Right = %d, i4Up = %d, i4Bottom = %d,win[0]:left = %d, i4Right = %d, i4Up = %d, bottom = %d\n" ,
	//		a_u4DispW, a_u4DispH, i4Left, i4Right, i4Up, i4Bottom,
	//		sAF_Win_Result.sRect[i].i4Left,
	//		sAF_Win_Result.sRect[i].i4Right,
	//		sAF_Win_Result.sRect[i].i4Up,
	//		sAF_Win_Result.sRect[i].i4Bottom);


        i4Width  = i4Right - i4Left + 1;
        i4Height = i4Bottom - i4Up + 1;

        if (i4Left < 0)   {i4Left = 0;}
        if (i4Right < 0)  {i4Right = 0;}
        if (i4Up < 0)     {i4Up = 0;}
        if (i4Bottom < 0) {i4Bottom = 0;}

        if      (sAF_Win_Result.sRect[i].eMarkStatus == AF_MARK_OK)
        {
            color = GREEN;
            colorgray = GRAYGREEN;
        }
        else if (sAF_Win_Result.sRect[i].eMarkStatus == AF_MARK_FAIL)
        {
            color = RED;
            colorgray = GRAYRED;
        }
        else
        {
            color = WHITE;
            colorgray = GRAY;
        }

        if ((a_u4DispW + a_u4DispH) > 1000)
        {
            fg3Line = TRUE;
            i4LineLen = 12;
        }
        else
        {
            fg3Line = FALSE;
            i4LineLen = 8;
        }

        i4UpLCDWidthLeft0 = i4Up * i4LCDWidth + i4Left;
        i4UpLCDWidthLeft1 = i4UpLCDWidthLeft0 + i4LCDWidth;
        i4UpLCDWidthLeft2 = i4UpLCDWidthLeft1 + i4LCDWidth;
        i4UpLCDWidthLeft3 = i4UpLCDWidthLeft2 + i4LCDWidth;
        i4UpLCDWidthLeft4 = i4UpLCDWidthLeft3 + i4LCDWidth;

        i4BottomLCDWidthLeft0 = i4Bottom * i4LCDWidth + i4Left;
        i4BottomLCDWidthLeft1 = i4BottomLCDWidthLeft0 - i4LCDWidth;
        i4BottomLCDWidthLeft2 = i4BottomLCDWidthLeft1 - i4LCDWidth;
        i4BottomLCDWidthLeft3 = i4BottomLCDWidthLeft2 - i4LCDWidth;
        i4BottomLCDWidthLeft4 = i4BottomLCDWidthLeft3 - i4LCDWidth;

        i4UpLCDWidthRight0 = i4Up * i4LCDWidth + i4Right;
        i4WidthLineLen = i4Width-i4LineLen;
        i4HeightLineLen = i4Height-i4LineLen;


        for(INT32 k=0; k<i4LineLen; k++)
        {
            *(display_buffer_ptr + i4UpLCDWidthLeft1 + k) = color;
            *(display_buffer_ptr + i4UpLCDWidthLeft2 + k) = color;
            if (fg3Line)
            {*(display_buffer_ptr + i4UpLCDWidthLeft3 + k) = color;}

            *(display_buffer_ptr + i4BottomLCDWidthLeft1 + k) = color;
            *(display_buffer_ptr + i4BottomLCDWidthLeft2 + k) = color;
            if (fg3Line)
            {*(display_buffer_ptr + i4BottomLCDWidthLeft3 + k) = color;}
        }

        for(INT32 k=i4WidthLineLen; k<i4Width; k++)
        {
            *(display_buffer_ptr + i4UpLCDWidthLeft1 + k) = color;
            *(display_buffer_ptr + i4UpLCDWidthLeft2 + k) = color;
            if (fg3Line)
            {*(display_buffer_ptr + i4UpLCDWidthLeft3 + k) = color;}

            *(display_buffer_ptr + i4BottomLCDWidthLeft1 + k) = color;
            *(display_buffer_ptr + i4BottomLCDWidthLeft2 + k) = color;
            if (fg3Line)
            {*(display_buffer_ptr + i4BottomLCDWidthLeft3 + k) = color;}
        }

        for(INT32 k=0; k<i4LineLen; k++)
        {
            *(display_buffer_ptr + i4UpLCDWidthLeft0 + k * i4LCDWidth + 1) = color;
            *(display_buffer_ptr + i4UpLCDWidthLeft0 + k * i4LCDWidth + 2) = color;
            if (fg3Line)
            {*(display_buffer_ptr + i4UpLCDWidthLeft0 + k * i4LCDWidth + 3) = color;}

            *(display_buffer_ptr + i4UpLCDWidthRight0 + k * i4LCDWidth - 1) = color;
            *(display_buffer_ptr + i4UpLCDWidthRight0 + k * i4LCDWidth - 2) = color;
            if (fg3Line)
            {*(display_buffer_ptr + i4UpLCDWidthRight0 + k * i4LCDWidth - 3) = color;}
        }

        for(INT32 k=i4HeightLineLen; k<i4Height; k++)
        {
            *(display_buffer_ptr + i4UpLCDWidthLeft0 + k * i4LCDWidth + 1) = color;
            *(display_buffer_ptr + i4UpLCDWidthLeft0 + k * i4LCDWidth + 2) = color;
            if (fg3Line)
            {*(display_buffer_ptr + i4UpLCDWidthLeft0 + k * i4LCDWidth + 3) = color;}

            *(display_buffer_ptr + i4UpLCDWidthRight0 + k * i4LCDWidth - 1) = color;
            *(display_buffer_ptr + i4UpLCDWidthRight0 + k * i4LCDWidth - 2) = color;
            if (fg3Line)
            {*(display_buffer_ptr + i4UpLCDWidthRight0 + k * i4LCDWidth - 3) = color;}
        }

        // --- plot gray line ---
        for(INT32 k=0; k<i4LineLen; k++)
        {
            *(display_buffer_ptr + i4UpLCDWidthLeft0 + k) = colorgray;
            if (fg3Line && k >= 4)
            {*(display_buffer_ptr + i4UpLCDWidthLeft4 + k) = colorgray;}
            else if (!fg3Line && k >= 3)
            {*(display_buffer_ptr + i4UpLCDWidthLeft3 + k) = colorgray;}

            *(display_buffer_ptr + i4BottomLCDWidthLeft0 + k) = colorgray;
            if (fg3Line && k >= 4)
            {*(display_buffer_ptr + i4BottomLCDWidthLeft4 + k) = colorgray;}
            else if (!fg3Line && k >= 3)
            {*(display_buffer_ptr + i4BottomLCDWidthLeft3 + k) = colorgray;}
        }

        *(display_buffer_ptr + i4UpLCDWidthLeft1 + i4LineLen) = colorgray;
        *(display_buffer_ptr + i4UpLCDWidthLeft2 + i4LineLen) = colorgray;
        if (fg3Line)
        {*(display_buffer_ptr + i4UpLCDWidthLeft3 + i4LineLen) = colorgray;}

        *(display_buffer_ptr + i4BottomLCDWidthLeft1 + i4LineLen) = colorgray;
        *(display_buffer_ptr + i4BottomLCDWidthLeft2 + i4LineLen) = colorgray;
        if (fg3Line)
        {*(display_buffer_ptr + i4BottomLCDWidthLeft3 + i4LineLen) = colorgray;}

        for(INT32 k=i4WidthLineLen; k<i4Width; k++)
        {
            *(display_buffer_ptr + i4UpLCDWidthLeft0 + k) = colorgray;
            if (fg3Line && k < (i4Width - 4))
            {*(display_buffer_ptr + i4UpLCDWidthLeft4 + k) = colorgray;}
            else if (!fg3Line && k < (i4Width - 3))
            {*(display_buffer_ptr + i4UpLCDWidthLeft3 + k) = colorgray;}

            *(display_buffer_ptr + i4BottomLCDWidthLeft0 + k) = colorgray;
            if (fg3Line && k < (i4Width - 4))
            {*(display_buffer_ptr + i4BottomLCDWidthLeft4 + k) = colorgray;}
            else if (!fg3Line && k < (i4Width - 3))
            {*(display_buffer_ptr + i4BottomLCDWidthLeft3 + k) = colorgray;}
        }

        *(display_buffer_ptr + i4UpLCDWidthLeft1 + (i4WidthLineLen-1)) = colorgray;
        *(display_buffer_ptr + i4UpLCDWidthLeft2 + (i4WidthLineLen-1)) = colorgray;
        if (fg3Line)
        {*(display_buffer_ptr + i4UpLCDWidthLeft3 + (i4WidthLineLen-1)) = colorgray;}

        *(display_buffer_ptr + i4BottomLCDWidthLeft1 + (i4WidthLineLen-1)) = colorgray;
        *(display_buffer_ptr + i4BottomLCDWidthLeft2 + (i4WidthLineLen-1)) = colorgray;
        if (fg3Line)
        {*(display_buffer_ptr + i4BottomLCDWidthLeft3 + (i4WidthLineLen-1)) = colorgray;}

        for(INT32 k=0; k<i4LineLen; k++)
        {
            *(display_buffer_ptr + i4UpLCDWidthLeft0 + k * i4LCDWidth) = colorgray;
            if (fg3Line && k >= 4)
            {*(display_buffer_ptr + i4UpLCDWidthLeft0 + k * i4LCDWidth + 4) = colorgray;}
            else if (!fg3Line && k >= 3)
            {*(display_buffer_ptr + i4UpLCDWidthLeft0 + k * i4LCDWidth + 3) = colorgray;}

            *(display_buffer_ptr + i4UpLCDWidthRight0 + k * i4LCDWidth) = colorgray;
            if (fg3Line && k >= 4)
            {*(display_buffer_ptr + i4UpLCDWidthRight0 + k * i4LCDWidth - 4) = colorgray;}
            else if (!fg3Line && k >= 3)
            {*(display_buffer_ptr + i4UpLCDWidthRight0 + k * i4LCDWidth - 3) = colorgray;}
        }

        *(display_buffer_ptr + i4UpLCDWidthLeft0 + i4LineLen * i4LCDWidth + 1) = colorgray;
        *(display_buffer_ptr + i4UpLCDWidthLeft0 + i4LineLen * i4LCDWidth + 2) = colorgray;
        if (fg3Line)
        {*(display_buffer_ptr + i4UpLCDWidthLeft0 + i4LineLen * i4LCDWidth + 3) = colorgray;}

        *(display_buffer_ptr + i4UpLCDWidthRight0 + i4LineLen * i4LCDWidth - 1) = colorgray;
        *(display_buffer_ptr + i4UpLCDWidthRight0 + i4LineLen * i4LCDWidth - 2) = colorgray;
        if (fg3Line)
        {*(display_buffer_ptr + i4UpLCDWidthRight0 + i4LineLen * i4LCDWidth - 3) = colorgray;}

        for(INT32 k=i4HeightLineLen; k<i4Height; k++)
        {
            *(display_buffer_ptr + i4UpLCDWidthLeft0 + k * i4LCDWidth) = colorgray;
            if (fg3Line && k < (i4Height - 4))
            {*(display_buffer_ptr + i4UpLCDWidthLeft0 + k * i4LCDWidth + 4) = colorgray;}
            else if (!fg3Line && k < (i4Height - 3))
            {*(display_buffer_ptr + i4UpLCDWidthLeft0 + k * i4LCDWidth + 3) = colorgray;}

            *(display_buffer_ptr + i4UpLCDWidthRight0 + k * i4LCDWidth) = colorgray;
            if (fg3Line && k < (i4Height - 4))
            {*(display_buffer_ptr + i4UpLCDWidthRight0 + k * i4LCDWidth - 4) = colorgray;}
            else if (!fg3Line && k < (i4Height - 3))
            {*(display_buffer_ptr + i4UpLCDWidthRight0 + k * i4LCDWidth - 3) = colorgray;}
        }

        *(display_buffer_ptr + i4UpLCDWidthLeft0 + (i4HeightLineLen-1) * i4LCDWidth + 1) = colorgray;
        *(display_buffer_ptr + i4UpLCDWidthLeft0 + (i4HeightLineLen-1) * i4LCDWidth + 2) = colorgray;
        if (fg3Line)
        {*(display_buffer_ptr + i4UpLCDWidthLeft0 + (i4HeightLineLen-1) * i4LCDWidth + 3) = colorgray;}

        *(display_buffer_ptr + i4UpLCDWidthRight0 + (i4HeightLineLen-1) * i4LCDWidth - 1) = colorgray;
        *(display_buffer_ptr + i4UpLCDWidthRight0 + (i4HeightLineLen-1) * i4LCDWidth - 2) = colorgray;
        if (fg3Line)
        {*(display_buffer_ptr + i4UpLCDWidthRight0 + (i4HeightLineLen-1) * i4LCDWidth - 3) = colorgray;}


    }



    return err;

}

MINT32
Hal3AYuv::trigger3AAutoFrameRatePreviewStateAF()
{
    AAA_YUV_LOG("trigger3AAutoFrameRatePreviewStateAF aaa_state = 0x%x, AF_MODE=0x%x \n",
        m_e3AState,
        m_u4AFMode);

    MINT32 err = MHAL_NO_ERROR;

    switch (m_u4AFMode) {
        case AF_MODE_AFS:
            if(m_bIsMoveSpotMeter/* && !m_rFDInfo.bFDon*/){
            // is movespot meter mode && !FD
            //1. lock AF when transfer to preview state

            }
            else{//not movespot meter mode
                err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CANCEL_AF);
                if(err < 0){
                    AAA_YUV_ERR("trigger3AAutoFrameRatePreviewStateAF error: CMD_SENSOR_CANCEL_AF\n");
                    return err;
                }
            }
            break;
        case AF_MODE_AFC:
            if(m_bIsMoveSpotMeter/* && !m_rFDInfo.bFDon*/){
            // is movespot meter mode && !FD
            //1. lock AF when transfer to preview state
                err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CONSTANT_AF);
                if(err < 0){
                    AAA_YUV_ERR("trigger3AAutoFrameRatePreviewStateAF error: CMD_SENSOR_CONSTANT_AF\n");
                    return err;
                }
            }
            else{//not movespot meter mode
                err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CONSTANT_AF);
                if(err < 0){
                    AAA_YUV_ERR("trigger3AAutoFrameRatePreviewStateAF error: CMD_SENSOR_CONSTANT_AF\n");
                    return err;
                }
            }
            break;
        case AF_MODE_INFINITY:
            break;
        default:
            break;
    }

    return err;
}

MINT32
Hal3AYuv::trigger3AAFStateAF()
{
    AAA_YUV_LOG("trigger3AAFStateAF aaa_state = 0x%x, AF_MODE=0x%x \n",
        m_e3AState,
        m_u4AFMode);

    MINT32 err = MHAL_NO_ERROR;
    MUINT32 zone_addr = (MUINT32)m_AFzone;
    switch (m_u4AFMode) {
        case AF_MODE_AFS:
        case AF_MODE_AFC:
            //1. cancel focus
            //2. single focus and lock AF
            err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CANCEL_AF);
            if(err < 0){
                AAA_YUV_ERR("trigger3AAFStateAF error: CMD_SENSOR_CANCEL_AF\n");
                return err;
            }
            err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_AF_WINDOW, (MUINT32*)zone_addr);

            if(err < 0){
                AAA_YUV_ERR("trigger3AAFStateAF error: CMD_SENSOR_SET_AF_WINDOW\n");
                return err;
            }

            err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SINGLE_FOCUS_MODE);
            if(err < 0){
                AAA_YUV_ERR("trigger3AAFStateAF error: CMD_SENSOR_SINGLE_FOCUS_MODE\n");
                return err;
            }

            break;
        case AF_MODE_INFINITY:
            break;
        default:
            break;
    }

    return err;


}


MINT32
Hal3AYuv::setAFMode(
    MINT32 AFMode
)
{
    MINT32 err = MHAL_NO_ERROR;

    m_u4AFMode = AFMode;

    AAA_YUV_LOG("[setAFMode] af mode = 0x%x \n", m_u4AFMode);

    switch (m_u4AFMode) {
        case AF_MODE_AFS:
        case AF_MODE_INFINITY:

            err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CANCEL_AF);
            if(err < 0){
                AAA_YUV_ERR("setState sendCommand() error: CMD_SENSOR_CANCEL_AF\n");
                return err;
            }
            break;
        case AF_MODE_AFC:
        case AF_MODE_AFC_VIDEO:
            err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CONSTANT_AF);
            if(err < 0){
                AAA_YUV_ERR("setState sendCommand() error: CMD_SENSOR_CONSTANT_AF\n");
                return err;
            }
            break;
        default:
            break;
    }

    return err;

}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::isAFFinish()
{
    if(m_bIsdummylens){
        AAA_YUV_LOGI("isAFFinish\n");
        return 0;
    }

    MINT32 err = 0;
    MINT32 focus_status = 0xffffffff;
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_AF_STATUS, (MUINT32 * )&focus_status);
    if(err < 0){
        AAA_YUV_ERR("sendCommand() error: CMD_SENSOR_GET_AF_STATUS\n");
        return 1;
    }
    AAA_YUV_LOGI("isAFFinish query focus_status=0x%x\n",focus_status);
    if(SENSOR_AF_SCENE_DETECTING == focus_status
        ||SENSOR_AF_IDLE == focus_status)
        return 1;
    else
        return 0;


}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::getReadyForCapture(
)
{
    return (MINT32)m_bReadyForCapture;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::getDebugInfo(MVOID **a_p3ADebugInfo, MUINT32 *a_u4SizeInByte)
{
    memset(&m_r3ADebugInfo, 0, sizeof(AAA_DEBUG_INFO_T));

    *a_p3ADebugInfo = (MVOID *)&m_r3ADebugInfo;
    *a_u4SizeInByte = 0;
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::set3AParam(
    MINT32 a_i4Param0,
    MINT32 a_i4Param1,
    MINT32 a_i4Param2,
    MINT32 a_i4Param3
)
{
    MINT32 err = 0;
    MUINT32  yuvCmd = 0;
    MUINT32  yuvParam = 0;
    MUINT32  cmdLen = 2 * sizeof(MUINT32);

    MHAL_ASSERT(m_pSensorDrvObj != NULL, "m_pSensorDrvObj is NULL\n");

    AAA_YUV_LOG("[set3AParam] Cmd = 0x%x, Param= 0x%x \n", a_i4Param0, a_i4Param1);

    switch (a_i4Param0) {
        case HAL_3A_AE_MODE:
            yuvCmd = FID_AE_SCENE_MODE;
            break;
        case HAL_3A_AE_EVCOMP:
            yuvCmd  = FID_AE_EV;
            break;
        case HAL_3A_AE_METERING_MODE:
            yuvCmd = FID_AE_METERING;
            break;
        case HAL_3A_AE_ISOSPEED:
            yuvCmd = FID_AE_ISO;
            break;
        case HAL_3A_AE_STROBE_MODE:
            yuvCmd = FID_AE_STROBE;
            if (m_pStrobeDrvObj) {
                m_aeFlashlightType = (MINT32)m_pStrobeDrvObj->getFlashlightType();
                if (FLASHLIGHT_LED_PEAK == (FLASHLIGHT_TYPE_ENUM)m_aeFlashlightType ||
                    FLASHLIGHT_LED_CONSTANT == (FLASHLIGHT_TYPE_ENUM)m_aeFlashlightType) {
                    AAA_YUV_LOG("[HAL_3A_AE_STROBE_MODE]:setAEStrobeMode \n");
                    m_aeStrobeMode = a_i4Param1;
                }
            }
            break;
        case HAL_3A_AE_REDEYE_MODE:
            break;
        case HAL_3A_AE_FLICKER_MODE:
            yuvCmd = FID_AE_FLICKER;
            break;
        case HAL_3A_AE_FRAMERATE_MODE:
            err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_FRAME_RATE,  (MUINT32 * )&a_i4Param1);
            return err;
        case HAL_3A_AE_FPS_MIN_MAX:
            //TODO : implement later
            // need to send to sensor driver
            break;
        case HAL_3A_AF_MODE:
            yuvCmd = FID_AF_MODE;
            setAFMode(a_i4Param1);
            break;
        case HAL_3A_AF_METERING_MODE:
            yuvCmd = FID_AF_METERING;
            m_bIsMoveSpotMeter = FALSE;
            break;
        case HAL_3A_AF_METERING_POS: {
            m_bIsMoveSpotMeter = TRUE;
            mhalCamMoveSpotInfo_t *pmsInfo = (mhalCamMoveSpotInfo_t *) a_i4Param1;
            setAFMoveSpotPos(
                pmsInfo->u4OffsetX, pmsInfo->u4OffsetY,
                pmsInfo->u4DispW, pmsInfo->u4DispH,
                pmsInfo->u4DispX, pmsInfo->u4DispY, pmsInfo->u4Rotate);
            }

            break;
        case HAL_3A_AWB_MODE:
            yuvCmd = FID_AWB_MODE;
            m_awbMode = a_i4Param1;
            break;
        default:
            AAA_YUV_LOG("No support this command \n");
            return 0;
    }

    yuvParam = a_i4Param1;
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_YUV_CMD, (MUINT32 * )&yuvCmd, (MUINT32 * )&yuvParam);

    MHAL_ASSERT(err == SENSOR_NO_ERROR, "Err HAL_SET_3A_PARAM to YUV sensor");

    return err;
}
/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::get3AParam(
    MINT32 a_i4Param0,
    MINT32 *a_pParam1
)
{
    MINT32 err = MHAL_NO_ERROR;

    MHAL_ASSERT(m_pSensorDrvObj != NULL, "m_pSensorDrvObj is NULL\n");

    switch (a_i4Param0) {
        case HAL_3A_AE_FPS_MIN_MAX:
            //TODO : implement later
            // need to get from sensor driver
            *a_pParam1 = 30000;   //max  30fps
            *(a_pParam1+1) = 10000;   //min 10fps
            break;
        case HAL_3A_AE_SUPPORT_FPS_NUM:
            //TODO : implement later
            // need to get from sensor driver
            *a_pParam1 = 3;
            break;
        case HAL_3A_AE_SUPPORT_FPS_RANGE:
            //TODO : implement later
            // need to get from sensor driver
            *a_pParam1 = 30000;   //max 30fps
            *(a_pParam1+1) = 20000;   //20fps
            *(a_pParam1+2) = 10000;   //min 10fps
            break;
    }

    MHAL_ASSERT(err == MHAL_NO_ERROR, "Err HAL_GET_3A_PARAM");

    return err;
}
/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::setState(
    MINT32 a_i4aaaState
)
{

    AAA_YUV_LOG("Hal3AYuv::setState, aaa_state=%d, m_bIsdummylens=%d\n",a_i4aaaState, m_bIsdummylens);
    m_e3AState = (AAA_STATE_T)a_i4aaaState;

    MINT32 err = MHAL_NO_ERROR;

    if (m_e3AState == AAA_STATE_PRE_CAPTURE) {
        m_bReadyForCapture = 0;
    }


    if(m_bIsdummylens){
        //AAA_YUV_LOG("dummy setState\n");
        return err;
    }


    if(m_e3AState == AAA_STATE_AF){
        m_bInSingleFocus = TRUE;
        err = trigger3AAFStateAF();
    }

    if(m_e3AState == AAA_STATE_AUTO_FRAMERATE_PREVIEW){
        m_bInSingleFocus = FALSE;
        err = trigger3AAutoFrameRatePreviewStateAF();
    }

    if(m_e3AState == AAA_STATE_MANUAL_FRAMERATE_PREVIEW){
        m_bInSingleFocus = FALSE;
        err = trigger3AAutoFrameRatePreviewStateAF();
    }

    return err;
}
static MUINT32 preflashFrmCnt = NSCamCustom::custom_GetYuvFlashlightFrameCnt();

/*******************************************************************************
*
********************************************************************************/

MINT32
Hal3AYuv::do3A(
    MINT32 a_i4aaaState,
    MUINT32 *a_pWaitVDNum
)
{
    MINT32 err = MHAL_NO_ERROR; // -1: error
                               //  0: no error
                               //  1: update ISO
                               //  2: update CCT
                               //  3: update ISO and CCT
    SENSOR_FLASHLIGHT_AE_INFO_STRUCT mflashInfo;
    int mflashcnt = NSCamCustom::custom_GetYuvFlashlightFrameCnt();;

//for yuv+af+fd
    AAA_FRAME_INPUT_PARAM_T r3AInput;
    // Get zoom info
    getEZoomInfo(r3AInput.sEZoom);
    // Get FD info
    getFDInfo(r3AInput.sFDInfo);

    if (m_e3AState == AAA_STATE_PRE_CAPTURE) {
        //for AE preflash process
        if (m_pStrobeDrvObj) {
            if (isAEFlashOn() || preflashFrmCnt !=mflashcnt){
                if (0 == (--preflashFrmCnt)) {
                    memset(&mflashInfo, 0, sizeof(SENSOR_FLASHLIGHT_AE_INFO_STRUCT));                    
                    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_AE_FLASHLIGHT_INFO, (MUINT32*)&mflashInfo);  
                    if(err < 0){
                        AAA_YUV_ERR("queryAEFlashlightInfoFromSensor fail\n");
                        return err;
                    }
                    pre_shutter = mflashInfo.Exposuretime;
                    pre_gain = mflashInfo.Gain;
                    //OFF flashlight after preflash done.
                    if (m_bFlashActive == TRUE){
                    if (m_pStrobeDrvObj->setFire(0)  == MHAL_NO_ERROR) {
                        AAA_YUV_LOG("setFire OFF\n");
                    }
                        m_bFlashActive = FALSE;
                    }
                    preflashFrmCnt = mflashcnt;
                    m_bReadyForCapture = 1;
                    m_strobeWidth = 12;
                    AAA_YUV_LOG("custom flash cnt:%d\n",mflashcnt);
                }
                else {
                	   if (m_strobecurrent_BV == 0.0){
                        m_strobecurrent_BV=m_strobeTrigerBV;
                	   }
                    m_strobePreflashBV = m_strobeTrigerBV;                    
                    //ON flashlight
                    if (m_pStrobeDrvObj->setLevel(1) == MHAL_NO_ERROR) {
                        AAA_YUV_LOG("setLevel:%d\n",1);
                    }
                    if (m_pStrobeDrvObj->setFire(1) == MHAL_NO_ERROR) {
                        AAA_YUV_LOG("setFire ON\n");
                        m_bFlashActive = TRUE;
                    }
                }
            }
            else {
                m_bReadyForCapture = 1;
                m_strobeWidth = 0;
            }
        }
        else {
            m_bReadyForCapture = 1;
        }
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3AYuv::setCaptureParam(
    MUINT32 a_IsRawInDram
)
{
	   int mshutter=0,mcfg_gain=0,mgain=0;
	   int mflashEng=0;

    AAA_YUV_LOG("Hal3AYuv::setCaptureParam: %d\n",m_strobeWidth);

#if 1	   
    if ( (m_pStrobeDrvObj) && (m_strobeWidth >0) && (a_IsRawInDram==0) ){    
    	   mflashEng = NSCamCustom::custom_GetFlashlightGain10X();
        AAA_YUV_LOG("flashEng:%d,current_BV:%f,PreflashBV:%f,pre_shutter:%d,pre_gain:%d\n",mflashEng,m_strobecurrent_BV,m_strobePreflashBV,pre_shutter,pre_gain);
        MTK3A::convertFlashExpPara(mflashEng,2048,m_strobecurrent_BV,m_strobePreflashBV,pre_shutter,2048,pre_gain,&mshutter,&mcfg_gain,&mgain);
        m_strobecurrent_BV = 0.0;
        if (mflashEng > 10){
            m_strobeWidth = 0xff;
            AAA_YUV_LOG("open high current mode\n");
        }
        AAA_YUV_LOG("mshutter%d\,mgain%d \n",mshutter,mgain);
        // set to sensor
        setEShutterParam(mshutter,mgain);
		
    if (m_pStrobeDrvObj) {
		   m_AEFlashlightInfo.GAIN_BASE = 0xAABBCCDD;
		   m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_AE_FLASHLIGHT_INFO, (MUINT32*)&m_AEFlashlightInfo);
		}
    }    	
    // Config capture flash
    if ( (m_pStrobeDrvObj) && (a_IsRawInDram==0)) {
       m_pStrobeDrvObj->setCaptureFlashlightConf(m_strobeWidth);
    }
#else
    //call flashlight by isp callback, not implement
#endif
 
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::enableAE()
{
    AAA_YUV_LOG("enableAE\n");

    MINT32 err = 0;

    MUINT32 yuvCmd = FID_AE_SCENE_MODE;
    MUINT32 yuvParam = AE_MODE_AUTO;
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_YUV_CMD, (MUINT32 * )&yuvCmd, (MUINT32 * )&yuvParam);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::disableAE()
{
    AAA_YUV_LOG("disableAE\n");

    MINT32 err = 0;

    MUINT32 yuvCmd = FID_AE_SCENE_MODE;
    MUINT32 yuvParam = AE_MODE_OFF;
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_YUV_CMD, (MUINT32 * )&yuvCmd, (MUINT32 * )&yuvParam);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::enableAWB()
{
    AAA_YUV_LOG("enableAWB\n");

    MINT32 err = 0;

    MUINT32 yuvCmd = FID_AWB_MODE;
    MUINT32 yuvParam = m_awbMode;
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_YUV_CMD, (MUINT32 * )&yuvCmd, (MUINT32 * )&yuvParam);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::disableAWB()
{
    AAA_YUV_LOG("disableAWB\n");

    MINT32 err = 0;

    MUINT32 yuvCmd = FID_AWB_MODE;
    MUINT32 yuvParam = AWB_MODE_OFF;
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_YUV_CMD, (MUINT32 * )&yuvCmd, (MUINT32 * )&yuvParam);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::enableAF()
{
    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::disableAF()
{
    return MHAL_NO_ERROR;
}

MBOOL
Hal3AYuv::isAutoWhiteBalanceLockSupported()
{
    AAA_YUV_LOG("isAutoWhiteBalanceLockSupported\n");

    return TRUE;
}

MBOOL
Hal3AYuv::isAutoExposureLockSupported()
{
    AAA_YUV_LOG("isAutoExposureLockSupported\n");

    return TRUE;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::resetHalfPushState(
)
{

    m_bDecideAFSceneDetectingColorWhite = TRUE;

    return MHAL_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
/* LUT for gain & dEv */
#define ASD_LOG2_LUT_RATIO_BASE 256
#define ASD_LOG2_LUT_NO 101
#define ASD_LOG2_LUT_CENTER 0
#define YUV_EVDELTA_THRESHOLD  10


const MINT32 ASD_LOG2_LUT_RATIO[ASD_LOG2_LUT_NO]={
256,/* 0 */
274, 294, 315, 338, 362, 388, 416, 446, 478, 512,/* 0.1~1.0 */
549, 588, 630, 676, 724, 776, 832, 891, 955, 1024,/* 1.1~2.0 */
1097, 1176, 1261, 1351, 1448, 1552, 1663, 1783, 1911, 2048,/* 2.1~3.0 */
2195, 2353, 2521, 2702, 2896, 3104, 3327, 3566, 3822, 4096,/* 3.1~4.0 */
4390, 4705, 5043, 5405, 5793, 6208, 6654, 7132, 7643, 8192,/* 4.1~5.0 */
8780, 9410, 10086, 10809, 11585, 12417, 13308, 14263, 15287, 16384,/* 5.1~6.0 */
17560, 18820, 20171, 21619, 23170, 24834, 26616, 28526, 30574, 32768,/* 6.1~7.0 */
35120, 37640, 40342, 43238, 46341, 49667, 53232, 57052, 61147, 65536,/* 7.1~8.0 */
70240, 75281, 80864, 86475, 92682, 99334, 106464, 114105, 122295, 131072,/* 8.1~9.0 */
140479, 150562, 161369, 172951, 185364, 198668, 212927, 228210, 244589, 262144/* 9.1~10.0 */
};

MINT32
Hal3AYuv::ASDLog2Func(
    MUINT32 numerator,
    MUINT32 denominator)
{
    MUINT32 temp_p;
    MINT32 x;
    MUINT32 *p_LOG2_LUT_RATIO = (MUINT32*)(&ASD_LOG2_LUT_RATIO[0]);

    temp_p = numerator*p_LOG2_LUT_RATIO[ASD_LOG2_LUT_CENTER];

    if (temp_p>denominator*ASD_LOG2_LUT_RATIO_BASE)
    {
        for (x=ASD_LOG2_LUT_CENTER; x<ASD_LOG2_LUT_NO; x++)
        {
            temp_p = denominator*p_LOG2_LUT_RATIO[x];

            if (temp_p>=numerator*ASD_LOG2_LUT_RATIO_BASE)
            {
                if ((temp_p -numerator*ASD_LOG2_LUT_RATIO_BASE)
                    > (numerator*ASD_LOG2_LUT_RATIO_BASE-denominator*p_LOG2_LUT_RATIO[x-1]))
                {
                    return x-1;
                }
                else
                {
                    return x;
                }
            }
            else if (x==ASD_LOG2_LUT_NO-1)
            {
                return (ASD_LOG2_LUT_NO-1);
            }
        }
    }
    return ASD_LOG2_LUT_CENTER;
}


#define ASD_ABS(val) (((val) < 0) ? -(val) : (val))

void
Hal3AYuv::calcASDEv(
    SENSOR_AE_AWB_CUR_STRUCT cur
)
{
    MINT32 AeEv;
    AAA_YUV_LOG("[calcASDEv]shutter=%d,gain=%d,", cur.SensorAECur.AeCurShutter,cur.SensorAECur.AeCurGain);

    //m_i4AELv_x10
    if ((m_ref.SensorAERef.AeRefLV05Shutter * m_ref.SensorAERef.AeRefLV05Gain)
        <= (cur.SensorAECur.AeCurShutter * cur.SensorAECur.AeCurGain))
    {
        AeEv =  50;//0*80/IspSensorAeAwbRef.SensorLV05LV13EVRef+50;
    }
    else
    {
        AeEv = ASDLog2Func(m_ref.SensorAERef.AeRefLV05Shutter * m_ref.SensorAERef.AeRefLV05Gain,
                                      cur.SensorAECur.AeCurShutter * cur.SensorAECur.AeCurGain)
                   *80 / m_ref.SensorLV05LV13EVRef + 50;
    }

    if (AeEv > 150) // EV range from 50 ~150
    {
        AeEv = 150;
    }

    if (ASD_ABS(m_i4AELv_x10 -AeEv) <= YUV_EVDELTA_THRESHOLD)
    {
        m_bAEStable = TRUE;
    }
    else
    {
        m_bAEStable = FALSE;
    }

    m_i4AELv_x10 = AeEv;

    AAA_YUV_LOG("m_i4AELv_x10=%d", m_i4AELv_x10);
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::getASDInfo(
    AAA_ASD_INFO_T &a_ASDInfo
)
{
    MINT32 err = MHAL_NO_ERROR;

    memset(&a_ASDInfo, 0, sizeof(a_ASDInfo));

    SENSOR_AE_AWB_CUR_STRUCT cur;

    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_SHUTTER_GAIN_AWB_GAIN, (MUINT32*)&cur);
    if(err < 0){
        AAA_YUV_ERR("getASDInfo error: CMD_SENSOR_GET_SHUTTER_GAIN_AWB_GAIN\n");
        return err;
    }

    calcASDEv(cur);

    a_ASDInfo.i4AELv_x10 = m_i4AELv_x10;
    a_ASDInfo.bAEStable = m_bAEStable;
    a_ASDInfo.i4AWBRgain_X128 = cur.SensorAwbGainCur.AwbCurRgain;
    a_ASDInfo.i4AWBBgain_X128 = cur.SensorAwbGainCur.AwbCurBgain;
    a_ASDInfo.i4AWBRgain_D65_X128 = m_ref.SensorAwbGainRef.AwbRefD65Rgain;
    a_ASDInfo.i4AWBBgain_D65_X128 = m_ref.SensorAwbGainRef.AwbRefD65Bgain;
    a_ASDInfo.i4AWBRgain_CWF_X128 = m_ref.SensorAwbGainRef.AwbRefCWFRgain;
    a_ASDInfo.i4AWBBgain_CWF_X128 = m_ref.SensorAwbGainRef.AwbRefCWFBgain;



#if 1
    AAA_YUV_LOGI("[i4AELv_x10] %d\n", a_ASDInfo.i4AELv_x10);
    //AAA_YUV_LOGI("[bAEBacklit] %d\n", a_ASDInfo.bAEBacklit);
    AAA_YUV_LOGI("[bAEStable] %d\n", a_ASDInfo.bAEStable);
    AAA_YUV_LOGI("[i4AWBRgain_X128] %d\n", a_ASDInfo.i4AWBRgain_X128);
    AAA_YUV_LOGI("[i4AWBBgain_X128] %d\n", a_ASDInfo.i4AWBBgain_X128);
    AAA_YUV_LOGI("[i4AWBRgain_D65_X128] %d\n", a_ASDInfo.i4AWBRgain_D65_X128);
    AAA_YUV_LOGI("[i4AWBBgain_D65_X128] %d\n", a_ASDInfo.i4AWBBgain_D65_X128);
    AAA_YUV_LOGI("[i4AWBRgain_CWF_X128] %d\n", a_ASDInfo.i4AWBRgain_CWF_X128);
    AAA_YUV_LOGI("[i4AWBBgain_CWF_X128] %d\n", a_ASDInfo.i4AWBBgain_CWF_X128);
    //AAA_YUV_LOGI("[bAWBStable] %d\n", a_ASDInfo.bAWBStable);
    //AAA_YUV_LOGI("[pAFTable] %d\n", a_ASDInfo.pAFTable);
    //AAA_YUV_LOGI("[pAFTable[0]] %d\n", ((MINT32 *)a_ASDInfo.pAFTable)[0]);
    //AAA_YUV_LOGI("[pAFTable[1]] %d\n", ((MINT32 *)a_ASDInfo.pAFTable)[1]);
    //AAA_YUV_LOGI("[i4AFTableMacroIdx] %d\n", a_ASDInfo.i4AFTableMacroIdx);
    //AAA_YUV_LOGI("[i4AFTableIdxNum] %d\n", a_ASDInfo.i4AFTableIdxNum);
    //AAA_YUV_LOGI("[bAFStable] %d\n", a_ASDInfo.bAFStable);
    //AAA_YUV_LOGI("[i4AFTableOffset] %d\n", a_ASDInfo.i4AFTableOffset);
    //AAA_YUV_LOGI("[i4AFPos] %d\n", a_ASDInfo.i4AFPos);
#endif
    return MHAL_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
void Hal3AYuv::setFlashActive(MBOOL a_FlashActive)
{
    m_bFlashActive = a_FlashActive;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL Hal3AYuv::getFlashActive(void)
{
    return m_bFlashActive;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::CCTOPFlashControl(MVOID *a_pflashCtrl)
{
    AAA_YUV_LOG("[ACDK_CCT_OP_FLASH_CONTROL]\n");
    ACDK_FLASH_CONTROL *pflashCtrl = (ACDK_FLASH_CONTROL *)a_pflashCtrl;

    if (m_pStrobeDrvObj) {
        m_pStrobeDrvObj->setLevel(20);
        m_pStrobeDrvObj->setFire(1);
        usleep(pflashCtrl->duration);
        m_pStrobeDrvObj->setFire(0);
        usleep(pflashCtrl->duration);
    }
    else {
        AAA_YUV_LOG("[m_pStrobeDrvObj == NULL]\n");
        return MHAL_UNKNOWN_ERROR;
    }

    return MHAL_NO_ERROR;
}


// For ICS4.0 provide some APIs for control.
MVOID
Hal3AYuv::setAutoExposureLock(MBOOL bLockAE)
{
    AAA_YUV_LOG("[setAutoExposureLock] bLockAE:%d\n", bLockAE);

    if(bLockAE == TRUE){   // enable AE
        lockAE();
    }
    else {
        unlockAE();
    }
}

MBOOL
Hal3AYuv::getAutoExposureLock()
{
    return isAELocked();
}

MBOOL
Hal3AYuv::isAELocked()
{
    AAA_YUV_LOG("[isAELocked] bIsAELocked:%d\n", bIsAELocked);
    return bIsAELocked;
}

MVOID
Hal3AYuv::lockAE()
{
    // disable AE
    disableAE();
    bIsAELocked = TRUE;
}

MVOID
Hal3AYuv::unlockAE()
{
    // enable AE
    enableAE();
    bIsAELocked = FALSE;
}

MVOID
Hal3AYuv::setAutoWhiteBalanceLock(MBOOL bToggle)
{
    AAA_YUV_LOG("[setAutoWhiteBalanceLock] blockAWB:%d\n", bToggle);

    if (bToggle) { // AWB lock
        lockAWB();
    }
    else { // AWB unlock
        unlockAWB();
    }
}

MBOOL
Hal3AYuv::getAutoWhiteBalanceLock()
{
    return isAWBLocked();
}

MBOOL
Hal3AYuv::isAWBLocked()
{
    AAA_YUV_LOG("[isAWBLocked] bIsAWBLocked:%d\n", bIsAWBLocked);
    return bIsAWBLocked;
}

MVOID
Hal3AYuv::lockAWB()
{
    // disable AWB
    disableAWB();
    bIsAWBLocked = TRUE;
}

MVOID
Hal3AYuv::unlockAWB()
{
    // enable AWB
    enableAWB();
    bIsAWBLocked = FALSE;
}

MBOOL
Hal3AYuv::isFocused()
{
    AAA_YUV_LOG("isFocused aaa_state = 0x%x, AF_MODE=0x%x \n",
        m_e3AState,
        m_u4AFMode);

    if(m_bIsdummylens){
        return TRUE;
    }

    MINT32 err = MHAL_NO_ERROR;
    MINT32 focus_status = 0xffffffff;

    switch (m_u4AFMode) {
        case AF_MODE_AFS:
            err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_AF_STATUS, (MUINT32 * )&focus_status);
            if(err < 0){
                AAA_YUV_ERR("sendCommand() error: CMD_SENSOR_GET_AF_STATUS\n");
                return TRUE;
            }
            AAA_YUV_LOG("isFocused focus_status=0x%x\n",focus_status);
            if(SENSOR_AF_FOCUSED == focus_status) {return TRUE;}
            else {return FALSE;}

            break;
        case AF_MODE_AFC:
            //YUV sensor can not pause
            err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_AF_STATUS, (MUINT32 * )&focus_status);
            if(err < 0){
                AAA_YUV_ERR("sendCommand() error: CMD_SENSOR_GET_AF_STATUS\n");
                return TRUE;
            }
            //AAA_YUV_LOG("AF_MODE_AFC isFocused focus_status=0x%x\n",focus_status);
            if(SENSOR_AF_FOCUSED == focus_status) {return TRUE;}
            else {return FALSE;}

            break;
        case AF_MODE_INFINITY:

            break;
        default:
            break;
    }

    return TRUE;


}

void
Hal3AYuv::resetFocus()
{
    AAA_YUV_LOG("resetFocus aaa_state = 0x%x, AF_MODE=0x%x \n",
        m_e3AState,
        m_u4AFMode);
    if(m_bIsdummylens){
        return ;
    }

    MINT32 err = MHAL_NO_ERROR;

    switch (m_u4AFMode) {
        case AF_MODE_AFS:
            err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CANCEL_AF);
            if(err < 0){
                AAA_YUV_ERR("trigger3AAFStateAF error: CMD_SENSOR_CANCEL_AF\n");
                return ;
            }


            break;
        case AF_MODE_AFC:

            err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CANCEL_AF);
            if(err < 0){
                AAA_YUV_ERR("setState sendCommand() error: CMD_SENSOR_CANCEL_AF\n");
                return ;
            }

            err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_CONSTANT_AF);
            if(err < 0){
                AAA_YUV_ERR("setState sendCommand() error: CMD_SENSOR_CONSTANT_AF\n");
                return ;
            }
            break;
        case AF_MODE_INFINITY:

            break;
        default:
            break;
    }


}

MINT32
Hal3AYuv::clamp(MINT32 x, MINT32 min, MINT32 max)
{
    if (x > max) return max;
    if (x < min) return min;
    return x;
}


void
Hal3AYuv::setFocusAreas(MINT32 a_i4Cnt, AREA_T *a_psFocusArea)
{
    if ((a_i4Cnt == 0) || (a_i4Cnt > m_max_focus_areas))
    {
        return ;

    }
    else  // spot or matrix meter
    {
        m_sAFAREA[0] = *a_psFocusArea;

        m_sAFAREA[0].i4Left      = clamp(m_sAFAREA[0].i4Left + 1000, 0, 1999);
        m_sAFAREA[0].i4Right    = clamp(m_sAFAREA[0].i4Right + 1000, 0, 1999);
        m_sAFAREA[0].i4Top      = clamp(m_sAFAREA[0].i4Top + 1000, 0, 1999);
        m_sAFAREA[0].i4Bottom = clamp(m_sAFAREA[0].i4Bottom + 1000, 0, 1999);
        AAA_YUV_LOG("touch auto focus setFocusAreas\n");
        mapAeraToZone(&m_sAFAREA[0], 2000, 2000, &m_AFzone[0], m_imageXS, m_imageYS);
    }
}

void
Hal3AYuv::getFocusAreas(MINT32 &a_i4Cnt, AREA_T **a_psFocusArea)
{
    AAA_YUV_LOG(" getFocusAreas, cnt=%d\n", m_max_focus_areas);
    //how to fill in m_sAFAREA[0] default??
    a_i4Cnt = m_max_focus_areas;
    *a_psFocusArea = &m_sAFAREA[0];
}

MINT32
Hal3AYuv::clampMaxNumAreas(MINT32 x, MINT32 min, MINT32 max)
{
    if (x > max) return min;
    if (x < min) return min;
    return x;
}

MINT32
Hal3AYuv::getMaxNumFocusAreas()
{
    MINT32 err = 0;
    MINT32 max_focus_areas = 0;
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_AF_MAX_NUM_FOCUS_AREAS, (MUINT32 * )&max_focus_areas);
    if(err < 0){
        AAA_YUV_ERR("sendCommand() error: CMD_SENSOR_GET_AF_MAX_NUM_FOCUS_AREAS\n");
        return 0;
    }
    m_max_focus_areas = clampMaxNumAreas(max_focus_areas, 0, 1);
    AAA_YUV_LOG("clamp sensor max focus area num 0x%x to 0x%x\n", max_focus_areas, m_max_focus_areas);
    return m_max_focus_areas;

}

void
Hal3AYuv::mapAeraToZone(
    AREA_T *p_area,
    MINT32 areaW,
    MINT32 areaH,
    MINT32* p_zone,
    MINT32 zoneW,
    MINT32 zoneH)
{

    MINT32 left, top, right, bottom;

    p_area->i4Left = clamp(p_area->i4Left, 0, areaW-1);
    p_area->i4Right = clamp(p_area->i4Right, 0, areaW-1);
    p_area->i4Top = clamp(p_area->i4Top, 0, areaH-1);
    p_area->i4Bottom = clamp(p_area->i4Bottom, 0, areaH-1);

    left        = p_area->i4Left   * zoneW  / areaW;
    right      = p_area->i4Right  * zoneW  / areaW;
    top        = p_area->i4Top   * zoneH / areaH;
    bottom   = p_area->i4Bottom * zoneH / areaH;

    *p_zone = clamp(left, 0, zoneW-1);
    *(p_zone+1) = clamp(top, 0, zoneH-1);
    *(p_zone+2) = clamp(right, 0, zoneW-1);
    *(p_zone+3) = clamp(bottom, 0, zoneH-1);
    *(p_zone+4) = zoneW;
    *(p_zone+5) = zoneH;

    AAA_YUV_LOG("\tmaping area [L]%d,[U]%d,[R]%d,[B]%d [width]%d [height]%d\n to               [L]%d,[U]%d,[R]%d,[B]%d [width]%d [height]%d\n",
        p_area->i4Left,
        p_area->i4Top,
        p_area->i4Right,
        p_area->i4Bottom,
        areaW,
        areaH,
        *p_zone,
        *(p_zone+1),
        *(p_zone+2),
        *(p_zone+3),
        *(p_zone+4),
        *(p_zone+5)
    );
}

void
Hal3AYuv::updateAFAeraAndZoneInfo(
    MINT32 x0,
    MINT32 y0,
    MINT32 x1,
    MINT32 y1,
    MINT32 areaW,
    MINT32 areaH)
{
    AAA_YUV_LOG("updateAFAeraAndZoneInfo\n");

    //update AF info
    //AF info (1) AREA_T m_sAFAREA[AF_WIN_NUM_MATRIX]
    m_sAFAREA[0].i4Left = x0;
    m_sAFAREA[0].i4Top = y0;
    m_sAFAREA[0].i4Right = x1;
    m_sAFAREA[0].i4Bottom = y1;

    //AF info (2) MINT32 m_AFzone[6]
    mapAeraToZone(&m_sAFAREA[0], areaW, areaH, &m_AFzone[0], m_imageXS, m_imageYS);
}

void
Hal3AYuv::updateAEAeraAndZoneInfo(
    MINT32 x0,
    MINT32 y0,
    MINT32 x1,
    MINT32 y1,
    MINT32 areaW,
    MINT32 areaH)
{
    AAA_YUV_LOG("updateAEAeraAndZoneInfo\n");

    //update AE info
    //AE info (1) AREA_T m_sAEAREA[1]; //touch AE has only one window
    m_sAEAREA[0].i4Left = x0;
    m_sAEAREA[0].i4Top = y0;
    m_sAEAREA[0].i4Right = x1;
    m_sAEAREA[0].i4Bottom = y1;

    //AE info (2) MINT32 m_AEzone[6]
    mapAeraToZone(&m_sAEAREA[0], areaW, areaH, &m_AEzone[0], m_imageXS, m_imageYS);
}


MINT32
Hal3AYuv::getMaxNumMeteringAreas()
{
    MINT32 err = 0;
    MINT32 max_metering_areas = 0;
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_AE_MAX_NUM_METERING_AREAS, (MUINT32 * )&max_metering_areas);
    if(err < 0){
        AAA_YUV_ERR("sendCommand() error: CMD_SENSOR_GET_AE_MAX_NUM_METERING_AREAS\n");
        return 0;
    }
    m_max_metering_areas = clampMaxNumAreas(max_metering_areas, 0, 1);
    AAA_YUV_LOG("clamp sensor max metering area num 0x%x to 0x%x\n", max_metering_areas, m_max_metering_areas);
    return m_max_metering_areas;
}

MVOID
Hal3AYuv::getMeteringAreas(MINT32 &a_i4Cnt, AREA_T **a_psAEArea)
{
    AAA_YUV_LOG(" getMeteringAreas cnt=%d\n", m_max_metering_areas);
    //how to fill in m_sAFAREA[0] default??
    a_i4Cnt = m_max_metering_areas;
    *a_psAEArea = &m_sAEAREA[0];
}

MVOID
Hal3AYuv::setMeteringAreas(MINT32 a_i4Cnt, AREA_T const *a_psAEArea)
{
    MINT32 err = MHAL_NO_ERROR;
    MUINT32* zone_addr = (MUINT32*)&m_AEzone[0];

    if ((a_i4Cnt == 0) || (a_i4Cnt > m_max_metering_areas))
    {
        return ;
    }
    else  // spot or matrix meter
    {
        m_sAEAREA[0] = *a_psAEArea;

        m_sAEAREA[0].i4Left      = clamp(m_sAEAREA[0].i4Left + 1000, 0, 1999);
        m_sAEAREA[0].i4Right    = clamp(m_sAEAREA[0].i4Right + 1000, 0, 1999);
        m_sAEAREA[0].i4Top      = clamp(m_sAEAREA[0].i4Top + 1000, 0, 1999);
        m_sAEAREA[0].i4Bottom = clamp(m_sAEAREA[0].i4Bottom + 1000, 0, 1999);
        AAA_YUV_LOG("touch auto exposure setMeteringAreas\n");
        mapAeraToZone(&m_sAEAREA[0], 2000, 2000, &m_AEzone[0], m_imageXS, m_imageYS);
        AAA_YUV_LOG("touch auto exposure setMeteringAreas, zone_addr=0x%x\n", zone_addr);
        err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_AE_WINDOW, zone_addr);
    }
}


MINT32
Hal3AYuv::queryExifInfoFromSensor()
{
    MINT32 err = MHAL_NO_ERROR;
    memset(&m_exifInfo, 0, sizeof(SENSOR_EXIF_INFO_STRUCT));

    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_EXIF_INFO, (MUINT32*)&m_exifInfo);
    if(err < 0){
        AAA_YUV_ERR("getASDInfo error: CMD_SENSOR_GET_SHUTTER_GAIN_AWB_GAIN\n");
        return err;
    }

    AAA_YUV_LOG("FNumber=%d, AEISOSpeed=%d, AWBMode=%d, CapExposureTime=%d, FlashLightTimeus=%d, RealISOValue=%d\n",
        m_exifInfo.FNumber,
        m_exifInfo.AEISOSpeed,
        m_exifInfo.AWBMode,
        m_exifInfo.CapExposureTime,
        m_exifInfo.FlashLightTimeus,
        m_exifInfo.RealISOValue);

    return err;
}

LIB3A_AE_ISO_SPEED_T
Hal3AYuv::mapAEISOSpeed(MUINT32 sensorISO)
{
    LIB3A_AE_ISO_SPEED_T lib3A_iso_speed;

    switch(sensorISO)
    {
    case AE_ISO_AUTO:
        lib3A_iso_speed = LIB3A_AE_ISO_SPEED_AUTO;
        break;
    case AE_ISO_100:
        lib3A_iso_speed = LIB3A_AE_ISO_SPEED_100;
        break;
    case AE_ISO_200:
        lib3A_iso_speed = LIB3A_AE_ISO_SPEED_200;
        break;
    case AE_ISO_400:
        lib3A_iso_speed = LIB3A_AE_ISO_SPEED_400;
        break;
    case AE_ISO_800:
        lib3A_iso_speed = LIB3A_AE_ISO_SPEED_800;
        break;
    case AE_ISO_1600:
        lib3A_iso_speed = LIB3A_AE_ISO_SPEED_1600;
        break;
    default:
        lib3A_iso_speed = LIB3A_AE_ISO_SPEED_100;
        break;
    }

    AAA_YUV_LOG("map sensor AEISOSpeed( %d) to LIB3A AEISOSpeed( %d)\n",
        sensorISO,
        lib3A_iso_speed);

    return lib3A_iso_speed;
}

MUINT32
Hal3AYuv::mapAERealISOSpeed(MUINT32 sensorRealISO)
{
    MUINT32 lib3A_real_iso_speed;

    switch(sensorRealISO)
    {
    case AE_ISO_AUTO:
        lib3A_real_iso_speed = LIB3A_AE_ISO_SPEED_100;
        break;
    case AE_ISO_100:
        lib3A_real_iso_speed = LIB3A_AE_ISO_SPEED_100;
        break;
    case AE_ISO_200:
        lib3A_real_iso_speed = LIB3A_AE_ISO_SPEED_200;
        break;
    case AE_ISO_400:
        lib3A_real_iso_speed = LIB3A_AE_ISO_SPEED_400;
        break;
    case AE_ISO_800:
        lib3A_real_iso_speed = LIB3A_AE_ISO_SPEED_800;
        break;
    case AE_ISO_1600:
        lib3A_real_iso_speed = LIB3A_AE_ISO_SPEED_1600;
        break;
    default:
        lib3A_real_iso_speed = LIB3A_AE_ISO_SPEED_100;
        break;
    }

    AAA_YUV_LOG("map sensor real AEISOSpeed( %d) to LIB3A real AEISOSpeed( %d)\n",
        sensorRealISO,
        lib3A_real_iso_speed);

    return lib3A_real_iso_speed;
}



LIB3A_AWB_MODE_T
Hal3AYuv::mapAWBMode(MUINT32 sensorAWB)
{
    LIB3A_AWB_MODE_T lib3A_awb_mode;

    switch(sensorAWB)
    {
    case AWB_MODE_AUTO:
        lib3A_awb_mode = LIB3A_AWB_MODE_AUTO;
        break;
    case AWB_MODE_DAYLIGHT:
        lib3A_awb_mode = LIB3A_AWB_MODE_DAYLIGHT;
        break;
    case AWB_MODE_CLOUDY_DAYLIGHT:
        lib3A_awb_mode = LIB3A_AWB_MODE_CLOUDY_DAYLIGHT;
        break;
    case AWB_MODE_SHADE:
        lib3A_awb_mode = LIB3A_AWB_MODE_SHADE;
        break;
    case AWB_MODE_TWILIGHT:
        lib3A_awb_mode = LIB3A_AWB_MODE_TWILIGHT;
        break;
    case AWB_MODE_FLUORESCENT:
        lib3A_awb_mode = LIB3A_AWB_MODE_FLUORESCENT;
        break;
    case AWB_MODE_WARM_FLUORESCENT:
        lib3A_awb_mode = LIB3A_AWB_MODE_WARM_FLUORESCENT;
        break;
    case AWB_MODE_INCANDESCENT:
        lib3A_awb_mode = LIB3A_AWB_MODE_INCANDESCENT;
        break;
    case AWB_MODE_TUNGSTEN:
        lib3A_awb_mode = LIB3A_AWB_MODE_MAX;
        break;
    default:
        lib3A_awb_mode = LIB3A_AWB_MODE_AUTO;
        break;
    }

    AAA_YUV_LOG("map sensor AWBMode( %d) to LIB3A AWBMode( %d)\n",
        sensorAWB,
        lib3A_awb_mode);

    return lib3A_awb_mode;
}

MUINT32
Hal3AYuv::mapFNumber(MUINT32 sensorFNumber)
{
    return (sensorFNumber > 0)? sensorFNumber : 28;
}

MUINT32
Hal3AYuv::mapCapExposureTime(MUINT32 sensorExposureTime)
{
    return (sensorExposureTime > 0)? sensorExposureTime : 0;
}

MUINT32
Hal3AYuv::mapFlashLightTimeus(MUINT32 sensorFlashLightTime)
{
    return (sensorFlashLightTime > 0)? sensorFlashLightTime : 0;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::get3AEXIFInfo(
    MVOID *a_p3AEXIFInfo
)
{
    MINT32 err = MHAL_NO_ERROR;
    AAA_EXIF_INFO_T* p_exif_info = (AAA_EXIF_INFO_T *)a_p3AEXIFInfo;

    queryExifInfoFromSensor();

    p_exif_info->u4FNumber = mapFNumber(m_exifInfo.FNumber);
    p_exif_info->eAEISOSpeed = mapAEISOSpeed(m_exifInfo.AEISOSpeed);
    p_exif_info->eAWBMode = mapAWBMode(m_exifInfo.AWBMode);
    p_exif_info->u4CapExposureTime = mapCapExposureTime(m_exifInfo.CapExposureTime);
    p_exif_info->u4FlashLightTimeus = mapFlashLightTimeus(m_exifInfo.FlashLightTimeus);
    //p_exif_info->u4RealISOValue = mapAERealISOSpeed(m_exifInfo.RealISOValue);
	p_exif_info->u4RealISOValue = m_exifInfo.RealISOValue;

    AAA_YUV_LOG("FNumber=%d, AEISOSpeed=%d, AWBMode=%d, CapExposureTime=%d, FlashLightTimeus=%d, RealISOValue=%d\n",
        p_exif_info->u4FNumber ,
        p_exif_info->eAEISOSpeed,
        p_exif_info->eAWBMode,
        p_exif_info->u4CapExposureTime,
        p_exif_info->u4FlashLightTimeus,
        p_exif_info->u4RealISOValue);

    return err;
}

MVOID Hal3AYuv::setZSDMode(MBOOL ZSDFlag)
{

}

void Hal3AYuv::FocusThread(MINT32 enable)
{

}

MVOID Hal3AYuv::setIsBurtShootMode(MINT32 isBurst)
{

}
MVOID
Hal3AYuv::setStrobeOn(MBOOL bEnable)
{

    if(bEnable)
        m_pStrobeDrvObj->setFire(1);
    else
        m_pStrobeDrvObj->setFire(0);

    return;
}
MINT32
Hal3AYuv::queryAEFlashlightInfoFromSensor()
{
    MINT32 err = MHAL_NO_ERROR;
    memset(&m_AEFlashlightInfo, 0, sizeof(SENSOR_FLASHLIGHT_AE_INFO_STRUCT));

    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_GET_AE_FLASHLIGHT_INFO, (MUINT32*)&m_AEFlashlightInfo);
    if(err < 0){
        AAA_YUV_ERR("queryAEFlashlightInfoFromSensor fail\n");
        return err;
    }
    m_AEFlashlightInfo.u4Fno = 28;
    m_AEFlashlightInfo.GAIN_BASE = 50;
    if (m_AEFlashlightInfo.Exposuretime > 20000||m_AEFlashlightInfo.Exposuretime == 0){
        AAA_YUV_ERR("query exp fail,val:%d,set to 1000\n",m_AEFlashlightInfo.Exposuretime);
        m_AEFlashlightInfo.Exposuretime=1000;
    }
    if (m_AEFlashlightInfo.Gain > 2000 || m_AEFlashlightInfo.Gain == 0){
        AAA_YUV_ERR("query gain fail,val:%d,set to 50\n",m_AEFlashlightInfo.Exposuretime);
        m_AEFlashlightInfo.Gain=50;
    }

    AAA_YUV_LOG("u4Fno=%d, Exposuretime=%d, Gain=%d, GAIN_BASE=%d\n",
        m_AEFlashlightInfo.u4Fno,
        m_AEFlashlightInfo.Exposuretime,
        m_AEFlashlightInfo.Gain,
        m_AEFlashlightInfo.GAIN_BASE);

    return err;
}

double
Hal3AYuv::AEFlashlightLog2(double x)
{
    //AAA_YUV_LOG("double x= %f, ln(x)=%f\n", x, log(x), log(x)/log((double)2));
    return log(x)/log((double)2);
}

double 
Hal3AYuv::calcBV()
{
    double  AV=0, TV=0, SV=0, BV=0;
    MINT32 ISO =0;
    MINT32 u4MiniISOGain = 50;
    queryAEFlashlightInfoFromSensor();

    AV=AEFlashlightLog2((double)m_AEFlashlightInfo.u4Fno/10)*2;
    TV=AEFlashlightLog2(1000000/((double)m_AEFlashlightInfo.Exposuretime));
    ISO=m_AEFlashlightInfo.Gain * u4MiniISOGain/ m_AEFlashlightInfo.GAIN_BASE;
    SV=AEFlashlightLog2(((double)ISO)/3.125);

    BV = AV + TV - SV ;

    AAA_YUV_LOG("AV=%f, TV=%f,ISO=%f,SV=%f,BV=%f\n",
        AV, TV, ISO, SV, BV);

    return (BV);

}

// For ZSD used
MBOOL Hal3AYuv::isStrobeOn()
{
    AAA_YUV_LOG("isStrobeOn\n");

    return isAEFlashOn();
}
/*******************************************************************************
*
********************************************************************************/
MINT32
Hal3AYuv::setEShutterParam(
    MUINT32 a_u4ExpTime,
    MUINT32 a_u4SensorGain
)
{
    MINT32 err;
  //  MUINT32 u4ExpData[2];

    if ((a_u4ExpTime == 0) || (a_u4SensorGain == 0)) {
        AAA_YUV_LOG("setExpParam() error: a_u4ExpTime = %d; a_u4SensorGain = %d; \n", a_u4ExpTime, a_u4SensorGain);
        return MHAL_INVALID_PARA;
    }

//    u4ExpData[0] = a_u4ExpTime;
//    u4ExpData[1] = a_u4SensorGain;
    a_u4SensorGain = a_u4SensorGain << 4;

//    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_ESHUTTER_GAIN, &u4ExpData[0], NULL, NULL);
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_EXP_LINE, &a_u4ExpTime, NULL, NULL);
	   MHAL_ASSERT(err == MHAL_NO_ERROR, "Err CMD_SENSOR_SET_EXP_LINE");
    err = m_pSensorDrvObj->sendCommand(CMD_SENSOR_SET_GAIN, &a_u4SensorGain, NULL, NULL);
	   MHAL_ASSERT(err == MHAL_NO_ERROR, "Err CMD_SENSOR_SET_GAIN");

    return MHAL_NO_ERROR;
}

