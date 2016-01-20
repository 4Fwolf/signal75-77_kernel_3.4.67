/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc. */


/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.c
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 *   Source code of Sensor driver
 *
 *
 * Author:
 * -------
 *   Anyuan Huang (MTK70663)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "newport_tv.h"
#include "newport_tv_para.h"
#include "newport_tvCustomized.h"

#define NEWPORT_DEBUG

#ifdef NEWPORT_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif


UINT32 NewportTVOpen(void)
{
	SENSORDB("[NewportTV]NewportTVOpen\n");
		return ERROR_NONE;

}
/* NewportTVOpen() */

/*************************************************************************
* FUNCTION
* NewportTVClose
*
* DESCRIPTION
* This function is to turn off sensor module power.
*
* PARAMETERS
* None
*
* RETURNS
* None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 NewportTVClose(void)
{
  return ERROR_NONE;
} /* NewportTVClose() */

/*************************************************************************
* FUNCTION
* NewportTVPreview
*
* DESCRIPTION
* This function start the sensor preview.
*
* PARAMETERS
* *image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
* None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 NewportTVPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

  SENSORDB("\n\n\n\n\n\n");
  SENSORDB("[NewportTV]NewportTVPreview\n");
  
	   image_window->GrabStartX =0 ;
	   image_window->GrabStartY =0 ;
	   image_window->ExposureWindowWidth =NewportTV_FULL_WIDTH;
	   image_window->ExposureWindowHeight =NewportTV_FULL_HEIGHT;


  return ERROR_NONE;
}/* NewportTVPreview() */

UINT32 NewportTVCapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                      MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

  
  SENSORDB("\n\n\n\n\n\n");
  SENSORDB("[NewportTV]NewportTVCapture!!!!!!!!!!!!!\n");

  return ERROR_NONE;
} /* NewportTVCapture() */

UINT32 NewportTVGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	pSensorResolution->SensorFullWidth = NewportTV_FULL_WIDTH;
	pSensorResolution->SensorFullHeight = NewportTV_FULL_HEIGHT;

	pSensorResolution->SensorPreviewWidth=NewportTV_FULL_WIDTH ;
	pSensorResolution->SensorPreviewHeight=NewportTV_FULL_HEIGHT ;


  return ERROR_NONE;
} /* NewportTVGetResolution() */

UINT32 NewportTVGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
                    MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
                    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{

	pSensorInfo->SensorPreviewResolutionX=NewportTV_FULL_WIDTH;
	pSensorInfo->SensorPreviewResolutionY=NewportTV_FULL_HEIGHT;

	pSensorInfo->SensorFullResolutionX=NewportTV_FULL_WIDTH;
	pSensorInfo->SensorFullResolutionY=NewportTV_FULL_HEIGHT;

	pSensorInfo->SensorCameraPreviewFrameRate=30;
	pSensorInfo->SensorVideoFrameRate=30;
	pSensorInfo->SensorStillCaptureFrameRate=10;
	pSensorInfo->SensorWebCamCaptureFrameRate=15;
	pSensorInfo->SensorResetActiveHigh=FALSE;
	pSensorInfo->SensorResetDelayCount=1;

	pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;//nmi600 used	uniscope UYUV
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;	/*??? *///nmi600 used	
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;//nmi600 used	
	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;      //HSYNC HIGH valid//nmi600 used	
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;      //VSYNC LOW valid//nmi600 used uniscope LOW	


	pSensorInfo->SensorInterruptDelayLines = 1;
	pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_PARALLEL;

	pSensorInfo->CaptureDelayFrame = 1; 
	pSensorInfo->PreviewDelayFrame = 0; 
	pSensorInfo->VideoDelayFrame = 4;	

	pSensorInfo->SensorMasterClockSwitch = 0; 

  	pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_2MA; 


  switch (ScenarioId)
  {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
    default:
      pSensorInfo->SensorClockFreq= 26 ;
      pSensorInfo->SensorClockDividCount=3;
      pSensorInfo->SensorClockRisingCount=0;
      pSensorInfo->SensorClockFallingCount=2;
      pSensorInfo->SensorPixelClockCount=3;
      pSensorInfo->SensorDataLatchCount=2;
      pSensorInfo->SensorGrabStartX = NewportTV_GRAB_START_X; 
      pSensorInfo->SensorGrabStartY = NewportTV_GRAB_START_Y;
      break;
  }
  return ERROR_NONE;
} /* NewportTVGetInfo() */


UINT32 NewportTVControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
                    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
  switch (ScenarioId)
  {
  case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
  case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
  case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
    NewportTVPreview(pImageWindow, pSensorConfigData);
    break;

  default:
    break; 
  }
  return TRUE;
} /* NewportTVControl() */

UINT32 NewportTVFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
                        UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
  UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
  UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
  UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
  UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
  UINT32 NewportTVSensorRegNumber;
  UINT32 i;
  PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
  MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
  MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
  MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
  MSDK_SENSOR_ENG_INFO_STRUCT *pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;

  //SENSORDB("NewportTVFeatureControl£¬FeatureId:%d\n",FeatureId);
  switch (FeatureId)
  {
    case SENSOR_FEATURE_GET_RESOLUTION:
      *pFeatureReturnPara16++=NewportTV_FULL_WIDTH;
      *pFeatureReturnPara16=NewportTV_FULL_HEIGHT;
      *pFeatureParaLen=4;
      break;
    
    default:
      break;
  }
  return ERROR_NONE;
} /* NewportTVFeatureControl() */



UINT32 NewportTVSensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
  static SENSOR_FUNCTION_STRUCT SensorFuncNewportTV=
  {
    NewportTVOpen,
    NewportTVGetInfo,
    NewportTVGetResolution,
    NewportTVFeatureControl,
    NewportTVControl,
    NewportTVClose
  };

  /* To Do : Check Sensor status here */
  if (pfFunc!=NULL)
    *pfFunc=&SensorFuncNewportTV;

  return ERROR_NONE;
} /* SensorInit() */
