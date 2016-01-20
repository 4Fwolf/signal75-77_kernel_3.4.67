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
#ifndef _MHAL_INC_CAMERA_PROFILE_H_
#define _MHAL_INC_CAMERA_PROFILE_H_

/******************************************************************************
*
*******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	// Define the root event of camera
	Event_Camera,

		// Define the event used in Cameraservice
		Event_CameraService,
			Event_CS_connect,
			Event_CS_newMediaPlayer,
			Event_CS_newCamHwIF,
			Event_CS_newClient,
			Event_CS_getParameters,
			Event_CS_setParameters,
			Event_CS_setPreviewDisplay,
			Event_CS_sendCommand,
			Event_CS_startPreview,
			Event_CS_startRecording,
			Event_CS_takePicture,
			Event_CS_stopPreview,
			Event_CS_stopRecording,
			Event_CS_playSound,
			Event_CS_disconnect,
			Event_CS_disconnectWindow,
			Event_CS_releaseRecordingFrame,
			Event_CS_dataCallbackTimestamp,

		// Define the event used in CamHwIF
		Event_CamHwIF,

			// --Define the event used in CamDevice
			Event_CamDevice,
				Event_CamDev_setPreviewWindow,
				Event_CamDev_initDisplayAdapter,

			// --Define the event used in DisplayAdapter
			Event_DisplayAdapter,
				Event_DispAdpt_enableDisplay,
				Event_DispAdpt_prepareQueue,
				Event_DispAdpt_postFrame,
				Event_DispAdpt_dequeueBuffer,
				Event_DispAdpt_copyFrame,
				Event_DispAdpt_queueWindow,
				Event_PW_enqueue_buffer,
				Event_DispAdpt_setWindow,
				Event_DispAdpt_dequeWindow,
				Event_PW_dequeue_buffer,

		// Define the event used in Scenario
		Event_Scenario,

			// --Define the event used in mHalCam
			Event_mHalCam,
				Event_mHalCamCreate,
				Event_mHalCamInit,
				Event_ASDInit,
				Event_mHalCamUninit,
				Event_mHalCamPreviewStart,
				Event_mHalCamPreviewStop,
				Event_mHalCamPreviewProc,
				Event_mHalCamWaitPreviewStable,
				Event_mHalCamPreviewProc_detail,
				Event_getmAtvVSync,
				Event_getPreviewFrame,
				Event_mHalCamCaptureInit,
				Event_mHalCamCaptureUninit,
				Event_mHalCamCaptureStart,
				Event_mHalCamCaptureProc,

			// --Define the event used in shot
			Event_NormalShot,
				Event_NShot_capture,
				Event_NShot_invokeShutterCB,
				Event_NShot_createRawImage,
				Event_NShot_createJpgImage,
				Event_NShot_handleCaptureDone,
				Event_NShot_callbackPostView,

		// Define the event used in Pipe
		Event_Pipe,

			// --Define the event used in CameraIO
			Event_CameraIO,
				Event_CamIO_start,
				Event_CamIO_dropPreviewFrame,
				Event_CamIO_waitCaptureDone,

			// --Define the event used in IspHal
			Event_IspHal,
				Event_IspHal_init,
				Event_IspHal_uninit,
				Event_uninitMCUDrv,
				Event_IspHal_setConf,
				Event_IspHal_allocM4UMemory,
				Event_IspHal_freeM4UMemory,

		// Define the event used in Driver
		Event_Driver,

			// --Define the event used in sensor driver
			Event_Sensor,
				Event_Sensor_search,
				Event_Sensor_open,
				Event_Sensor_close,
				Event_Sensor_setScenario,

	Event_Max_Num
}CPT_Event;

typedef enum
{
	CPTFlagStart,
	CPTFlagEnd,
	CPTFlagPulse,
	CPTFlagSeparator
}CPT_LogType;

typedef struct
{
	CPT_Event event;
	CPT_Event parent;
	const char* name;
}CPT_Event_Info;

void initCameraProfile();
bool CPTEnableEvent(CPT_Event event, int enable);
bool CPTLog(CPT_Event event, CPT_LogType type);
bool CPTLogEx(CPT_Event event, CPT_LogType type, unsigned int data1, unsigned int data2);
bool CPTLogStr(CPT_Event event, CPT_LogType type, const char* str);
bool CPTLogStrEx(CPT_Event event, CPT_LogType type, unsigned int data1, unsigned int data2, const char* str);

class AutoCPTLog
{
protected:
    CPT_Event mEvent;
	unsigned int mData1;
	unsigned int mData2;

public:
    AutoCPTLog(CPT_Event event, unsigned int data1 = 0, unsigned int data2 = 0);
    virtual ~AutoCPTLog();
};

#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////////////
#endif  //  _MHAL_INC_CAMERA_PROFILE_H_

