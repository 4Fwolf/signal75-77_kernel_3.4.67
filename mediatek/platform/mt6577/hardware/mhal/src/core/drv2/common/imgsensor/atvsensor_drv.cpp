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

#define LOG_TAG "AtvSensorDrv"

#if 0 // def ATVCHIP_MTK_ENABLE 
extern "C" {
#include "kal_release.h"
#include "matvctrl.h"
}

#include "ATVCtrl.h"
#include "ATVCtrlService.h"
#include "RefBase.h"
#include "threads.h"

#include <binder/IServiceManager.h>
using namespace android;
#endif 

#ifdef ATVCHIP_MTK_ENABLE 
extern "C" {
#include "kal_release.h"
#include "matvctrl.h"
}

#endif 

#include <stdlib.h>
#include <utils/Errors.h>
#include <cutils/xlog.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cutils/properties.h>

/*******************************************************************************
*
********************************************************************************/
#define DEBUG_SENSOR_DRV
#ifdef DEBUG_SENSOR_DRV
#define SENSOR_DRV_DBG(fmt, arg...) SXLOGD(fmt, ##arg)
#define SENSOR_DRV_WARN(fmt, arg...) SXLOGW(fmt, ##arg)
#define SENSOR_DRV_ERR(fmt, arg...)  SXLOGE("Err: %5d:, "fmt, __LINE__, ##arg)
#else
#define SENSOR_DRV_DBG(a,...)
#define SENSOR_DRV_WARN(a,...)
#define SENSOR_DRV_ERR(a,...)
#endif

/*******************************************************************************
*
********************************************************************************/
#include "atvsensor_drv.h"
#include "camera_custom_if.h"

#define ATV_MODE_NTSC 30000
#define ATV_MODE_PAL  25000

/*******************************************************************************
*
********************************************************************************/
#if 0 // def ATVCHIP_MTK_ENABLE  
extern int is_factory_boot(void);
//extern int matvdrv_exist();

/*
#define ATV_MODE_NTSC 30000
#define ATV_MODE_PAL  25000

******************************************************************
customized delay
******************************************************************
//unit: us
#define ATV_MODE_NTSC_DELAY 5000
#define ATV_MODE_PAL_DELAY  10000

unsigned int matv_cust_get_disp_delay(int mode) {
    return ((ATV_MODE_NTSC == mode)?ATV_MODE_NTSC_DELAY:((ATV_MODE_PAL == mode)?ATV_MODE_PAL_DELAY:0));
}
*/
#endif
/*******************************************************************************
*
********************************************************************************/
SensorDrv*
AtvSensorDrv::
getInstance()
{
    SENSOR_DRV_DBG("[AtvSensorDrv] getInstance \n");
    static AtvSensorDrv singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void   
AtvSensorDrv::
destroyInstance() 
{
}

/*******************************************************************************
*
********************************************************************************/
AtvSensorDrv::
AtvSensorDrv()
    : SensorDrv()
    , m_SenosrResInfo()
{
    memset(&m_SenosrResInfo, 0, sizeof(m_SenosrResInfo));
}

/*******************************************************************************
*
********************************************************************************/
AtvSensorDrv::
~AtvSensorDrv()
{
}


/*******************************************************************************
*
********************************************************************************/
MINT32
AtvSensorDrv::init(MINT32 sensorIdx)
{
    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AtvSensorDrv::uninit()
{
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AtvSensorDrv::setScenario(ACDK_SCENARIO_ID_ENUM sId)
{
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AtvSensorDrv::start()
{
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AtvSensorDrv::stop()
{
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
AtvSensorDrv::sendCommand(
    MUINT32 cmd, 
    MUINT32 *parg1, 
    MUINT32 *parg2,
    MUINT32 *parg3 
)
{ 
    MINT32 err = 0;

//add by tengfei mtk80343
    MINT32 AtvInputdata = NSCamCustom::get_atv_input_data();
    //MINT32 AtvInputdata = 1;
    
    switch (cmd) {
    case CMD_SENSOR_GET_INDATA_FORMAT:
        {
            *parg1 = 1;        
            if( 0 == AtvInputdata )
            {
                *parg1 = 0;
            }
            // Bit 0~7 as data input
            else
            {
                *parg1 = 1;
            }
        }  
        break;

#ifdef  ATVCHIP_MTK_ENABLE      
    case CMD_SENSOR_GET_ATV_DISP_DELAY:
        {
            char value[PROPERTY_VALUE_MAX] = {'\0'};     
            property_get("atv.disp.delay", value, "-1");
            MINT32 atvDelayTime = atoi(value); 

            if(atvDelayTime != -1)
            {
                if( atvDelayTime < 0)
                {
                    atvDelayTime = 0;
                }

                *parg1 = atvDelayTime;
                SENSOR_DRV_DBG("get atv display delay time through property_get() \n");
            }
            else
            {
                *parg1 = atvGetDispDelay();
                SENSOR_DRV_DBG("get atv display delay time from custom file definition \n");
            }

            SENSOR_DRV_DBG("atv display real delay time is %dus \n", *parg1);
        }
        break;
#endif

    case CMD_SENSOR_GET_DELAY_FRAME_CNT:   //unstable frame
        *(MUINT32*)parg2 = 0; 
        break; 

    default:
//Return OK for compatible with aaa_hal_yuv usage
//        err = -1;
        break;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
AtvSensorDrv::open()
{
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
AtvSensorDrv::close()
{
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
AtvSensorDrv::getInfo(ACDK_SCENARIO_ID_ENUM ScenarioId,ACDK_SENSOR_INFO_STRUCT *pSensorInfo,ACDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    if (NULL != pSensorInfo && NULL != pSensorConfigData) {

        //decide swapY swapCbCr
        pSensorInfo->SensorOutputDataFormat = SENSOR_OUTPUT_FORMAT_UYVY;    
        
        pSensorInfo->SensorClockPolarity = SENSOR_CLOCK_POLARITY_LOW;
        pSensorInfo->SensorClockFallingPolarity = SENSOR_CLOCK_POLARITY_LOW;
        pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
        pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_HIGH;
        pSensorInfo->SensorMasterClockSwitch = 0;
        pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_2MA;
        pSensorInfo->SensorClockFreq = 26;        
        pSensorInfo->SensorClockDividCount=	3;
        pSensorInfo->SensorClockRisingCount= 0;
        pSensorInfo->SensorClockFallingCount= 2;
        pSensorInfo->SensorPixelClockCount= 3;
        pSensorInfo->SensorDataLatchCount= 2;
        pSensorInfo->SensorGrabStartX = 1; 
        pSensorInfo->SensorGrabStartY = 3;			   
        //to skip beginning unstable frame.
        pSensorInfo->CaptureDelayFrame = 22; //11.11 by HP's comment
        pSensorInfo->PreviewDelayFrame = 22;
        pSensorInfo->VideoDelayFrame = 22;

        pSensorConfigData->SensorImageMirror = ACDK_SENSOR_IMAGE_NORMAL;
        pSensorConfigData->SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_CAMERA_PREVIEW;
    }
    else {
        SENSOR_DRV_ERR("getInfo: NULL parameter \n");
        return SENSOR_UNKNOWN_ERROR;
    }
    
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
AtvSensorDrv::getResolution(ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    if (pSensorResolution) {
        pSensorResolution->SensorPreviewWidth = ISP_HAL_ATV_PREVIEW_WIDTH;
        pSensorResolution->SensorPreviewHeight = ISP_HAL_ATV_PREVIEW_HEIGHT;
        pSensorResolution->SensorFullWidth = ISP_HAL_ATV_FULL_WIDTH;
        pSensorResolution->SensorFullHeight = ISP_HAL_ATV_FULLW_HEIGHT;
    }
    else {
        SENSOR_DRV_ERR("getResolution: NULL parameter \n");
        return SENSOR_UNKNOWN_ERROR;
    }
    
    return 0;
}
/*******************************************************************************
*
********************************************************************************/
#ifdef ATVCHIP_MTK_ENABLE 
MINT32 
AtvSensorDrv::atvGetDispDelay() {
    MINT32 atvMode =0;
    MINT32 atvDelay = 0;

    atvMode = matv_get_mode();

    if ( atvMode!=ATV_MODE_NTSC && atvMode!=ATV_MODE_PAL)
    {
        SENSOR_DRV_ERR("get wrong atv mode %d", atvMode);
    }
    else
    {
        atvDelay = NSCamCustom::get_atv_disp_delay(atvMode);
    
        SENSOR_DRV_DBG("atvMode: %d \n",atvMode);
        SENSOR_DRV_DBG("atvDelay = %dus", atvDelay);
    }
    
    return atvDelay;
}
#endif

#if 0 //defined (ATVCHIP_MTK_ENABLE)
MINT32 
AtvSensorDrv::atvGetDispDelay() {
MINT32 atvMode =0;
MINT32 atvDelay = 0;
    if (matvdrv_exist()!=0) {
        if (!::is_factory_boot())
        {
          sp<IATVCtrlService> spATVCtrlService;
          sp<IServiceManager> sm = defaultServiceManager();
          sp<IBinder> binder;
          do{
             binder = sm->getService(String16("media.ATVCtrlService"));
             if (binder != 0)
                break;
                    
             SENSOR_DRV_WARN("ATVCtrlService not published, waiting...");
             usleep(500000); // 0.5 s
          } while(true);
          spATVCtrlService = interface_cast<IATVCtrlService>(binder);

          atvMode = spATVCtrlService->ATVCS_matv_get_chipdep(187);
          //atvDelay = matv_cust_get_disp_delay((int)atvMode);
          atvDelay = NSCamCustom::get_atv_disp_delay(atvMode);

          SENSOR_DRV_DBG("atvMode: %d \n",atvMode);
          SENSOR_DRV_DBG("atvDelay = %dus", atvDelay);
       }
       else
       {
          atvMode = matv_get_chipdep(187);
          //atvDelay = matv_cust_get_disp_delay((int)atvMode);
          atvDelay = NSCamCustom::get_atv_disp_delay(atvMode);
  
          SENSOR_DRV_DBG("atvMode: %d \n",atvMode);
          SENSOR_DRV_DBG("atvDelay = %dus", atvDelay);
       }
    }

    return atvDelay;
}//
#endif

