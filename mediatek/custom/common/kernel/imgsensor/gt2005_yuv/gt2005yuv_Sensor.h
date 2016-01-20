
#ifndef __SENSOR_H
#define __SENSOR_H

#define GT2005_WRITE_ID		(0x78)
#define GT2005_READ_ID			(0x79)



#define GT2005_PV_PERIOD_PIXEL_NUMS			(GT2005_FULL_PERIOD_PIXEL_NUMS / 2)
#define GT2005_PV_PERIOD_LINE_NUMS			(600)
#define GT2005_FULL_PERIOD_PIXEL_NUMS		(1600)	
#define GT2005_FULL_PERIOD_LINE_NUMS 		(1200)

#define GT2005_PV_EXPOSURE_LIMITATION		(800)
#define GT2005_FULL_EXPOSURE_LIMITATION		(1200)

#define GT2005_IMAGE_SENSOR_PV_WIDTH   		(800-16) 
#define GT2005_IMAGE_SENSOR_PV_HEIGHT  		(600-12) 

#define GT2005_IMAGE_SENSOR_FULL_WIDTH     	(1600-16)
#define GT2005_IMAGE_SENSOR_FULL_HEIGHT    	(1200-12)



/*ergate-004*/
#define GT2005_PV_GRAB_START_X 				(0) //(1) 
#define GT2005_PV_GRAB_START_Y  				(1) //(2) 
#define GT2005_PV_GRAB_WIDTH					(GT2005_IMAGE_SENSOR_PV_WIDTH  - 8) 
#define GT2005_PV_GRAB_HEIGHT				(GT2005_IMAGE_SENSOR_PV_HEIGHT - 6 ) 

#define GT2005_FULL_GRAB_START_X   			(16)
#define GT2005_FULL_GRAB_START_Y	  			(12)
#define GT2005_FULL_GRAB_WIDTH				(GT2005_IMAGE_SENSOR_FULL_WIDTH- 32 )
#define GT2005_FULL_GRAB_HEIGHT				(GT2005_IMAGE_SENSOR_FULL_HEIGHT  - 24)


typedef struct
{
kal_uint16	video_target_width;
kal_uint16	video_target_height;

kal_bool	MJPEG_encode_mode;			/* Motion JPEG */
kal_bool	MPEG4_encode_mode;			/* MJPEG4 JPEG */
kal_bool	FULLVIDEO_encode_mode;		/* 3G Video Call */

kal_bool	sensor_cap_state;			/* Preview or Capture mode */
kal_bool	is_PV_mode; 				/* PV size or Full size */
kal_bool	is_panorama_capturing;		/* 3G Video Call */

kal_uint32	curr_banding;				/* 50Hz/60Hz */
kal_bool	night_mode;
} GT2005_OPERATION_STATE_ST;
	
typedef struct
{
kal_uint8	sccb_write_id;
kal_uint8	sccb_read_id;

kal_uint32	pv_shutter;
kal_uint32	pv_extra_shutter;
kal_uint32	pv_sensor_gain;

kal_uint32	pv_dummy_pixels;
kal_uint32	pv_dummy_lines;
kal_uint32	cap_dummy_pixels;
kal_uint32	cap_dummy_lines;

/* Preview & Capture Pixel Clock, 360 means 36.0MHz. Unit Multiple 10. */
kal_uint32	preview_pclk;
kal_uint32	capture_pclk;

/* Video frame rate 300 means 30.0fps. Unit Multiple 10. */
kal_uint32	video_frame_rate;	
}GT2005_SENSOR_INFO_ST;

UINT32 GT2005Open(void);
UINT32 GT2005GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 GT2005GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 GT2005Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 GT2005FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 GT2005Close(void);

#endif 
