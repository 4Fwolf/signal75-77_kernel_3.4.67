/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <utils/Log.h> 
#include "CameraProfile.h"

#ifdef MTK_MMPROFILE_SUPPORT
#include "linux/mmprofile.h"

#define LOGD(fmt, arg...) ALOGD(fmt, ##arg)
#define LOGW(fmt, arg...) ALOGW(fmt, ##arg)

static CPT_Event_Info gCPTEventInfo[Event_Max_Num] =
				{
                {Event_Camera, Event_Camera, "Camera"}

    			    // Define the event info used in Cameraservice
    				,{Event_CameraService, Event_Camera, "CameraService"}
        	            ,{Event_CS_connect, Event_CameraService, "CS_connect"}
            	        ,{Event_CS_newMediaPlayer, Event_CS_connect, "newMediaPlayer"}
        	            ,{Event_CS_newCamHwIF, Event_CS_connect, "newCamHwIF"}
        	            ,{Event_CS_newClient, Event_CS_connect, "newClient"}
        	            ,{Event_CS_getParameters, Event_CameraService, "getParameters"}
            	        ,{Event_CS_setParameters, Event_CameraService, "setParameters"}
        	            ,{Event_CS_setPreviewDisplay, Event_CameraService, "setPreviewDisplay"}
                        ,{Event_CS_sendCommand, Event_CameraService, "sendCommand"}
        	            ,{Event_CS_startPreview, Event_CameraService, "startPreview"}
            	        ,{Event_CS_takePicture, Event_CameraService, "takePicture"}
        	            ,{Event_CS_stopPreview, Event_CameraService, "stopPreview"}
        	            ,{Event_CS_startRecording, Event_CameraService, "startRecording"}
                        ,{Event_CS_releaseRecordingFrame, Event_CameraService, "releaseRecordingFrame"}
                        ,{Event_CS_dataCallbackTimestamp, Event_CameraService, "dataCallbackTimestamp"}
        	            ,{Event_CS_stopRecording, Event_CameraService, "stopRecording"}
        	            ,{Event_CS_playSound, Event_CameraService, "playSound"}
            	        ,{Event_CS_disconnect, Event_CameraService, "disconnect"}
        	            ,{Event_CS_disconnectWindow, Event_CameraService, "disconnectWindow"}

    	            // Define the event info used in CamHwIF
    	            ,{Event_CamHwIF, Event_Camera, "CamHwIF"}

                        // --Define the event info used in CamDevice
                        ,{Event_CamDevice, Event_CamHwIF, "CamDevice"}
                            ,{Event_CamDev_setPreviewWindow, Event_CamDevice, "setPreviewWindow"}
                            ,{Event_CamDev_initDisplayAdapter, Event_CamDev_setPreviewWindow, "initDisplayAdapter"}

                        // --Define the event info used in DisplayAdapter
                        ,{Event_DisplayAdapter, Event_CamHwIF, "DisplayAdapter"}
                            ,{Event_DispAdpt_setWindow, Event_DisplayAdapter, "setWindow"}
                            ,{Event_DispAdpt_enableDisplay, Event_DisplayAdapter, "enableDisplay"}
                            ,{Event_DispAdpt_prepareQueue, Event_DispAdpt_enableDisplay, "prepareQueue"}
                            ,{Event_DispAdpt_postFrame, Event_DisplayAdapter, "postFrame"}
                            ,{Event_DispAdpt_dequeueBuffer, Event_DispAdpt_postFrame, "dequeueBuffer"}
                            ,{Event_DispAdpt_copyFrame, Event_DispAdpt_postFrame, "copyFrame"}
                            ,{Event_DispAdpt_queueWindow, Event_DispAdpt_postFrame, "queueWindow"}
                            ,{Event_PW_enqueue_buffer, Event_DispAdpt_queueWindow, "PW_enqueue_buffer"}
                            ,{Event_DispAdpt_dequeWindow, Event_DisplayAdapter, "dequeWindow"}
                            ,{Event_PW_dequeue_buffer, Event_DispAdpt_dequeWindow, "PW_dequeue_buffer"}


                    // Define the event info used in Scenario
    	            ,{Event_Scenario, Event_Camera, "Scenario"}

        				// --Define the event info used in mHalCam
        				,{Event_mHalCam, Event_Scenario, "mHalCam"}
            				,{Event_mHalCamCreate, Event_mHalCam, "mHalCamCreate"}
            				,{Event_mHalCamInit, Event_mHalCam, "mHalCamInit"}
                            ,{Event_ASDInit, Event_mHalCamInit, "ASDInit"}
            				,{Event_mHalCamPreviewStart, Event_mHalCam, "mHalCamPreviewStart"}
            				,{Event_mHalCamPreviewProc, Event_mHalCam, "mHalCamPreviewProc"}
                            ,{Event_mHalCamWaitPreviewStable, Event_mHalCam, "mHalCamWaitPreviewStable"}
            				,{Event_getPreviewFrame, Event_mHalCam, "getPreviewFrame"}
                            ,{Event_getmAtvVSync, Event_mHalCam, "getmAtvVSync"}
            				,{Event_mHalCamPreviewProc_detail,Event_mHalCamPreviewProc, "PreviewProc_Detail"}
            				,{Event_mHalCamPreviewStop, Event_mHalCam, "mHalCamPreviewStop"}
            		        ,{Event_mHalCamCaptureInit, Event_mHalCam, "mHalCamCaptureInit"}
            	            ,{Event_mHalCamCaptureStart, Event_mHalCam, "mHalCamCaptureStart"}
            	            ,{Event_mHalCamCaptureProc, Event_mHalCam, "mHalCamCaptureProc"}
            	            ,{Event_mHalCamCaptureUninit, Event_mHalCam, "mHalCamCaptureUninit"}
                            ,{Event_mHalCamUninit, Event_mHalCam, "mHalCamUninit"}

                        // --Define the event info used in NbomalShot
        	            ,{Event_NormalShot, Event_Scenario, "NormalShot"}
            	            ,{Event_NShot_capture, Event_NormalShot, "capture"}
                            ,{Event_NShot_invokeShutterCB, Event_NormalShot, "invokeShutterCB"}
            		        ,{Event_NShot_createRawImage, Event_NormalShot, "createRawImage"}
            		        ,{Event_NShot_createJpgImage, Event_NormalShot, "createJpgImage"}
            		        ,{Event_NShot_handleCaptureDone, Event_NormalShot, "handleCaptureDone"}
                            ,{Event_NShot_callbackPostView, Event_NormalShot, "callbackPostView"}

                    // Define the event info used in Pipe
    	            ,{Event_Pipe, Event_Camera, "Pipe"}

        				// --Define the event info used in CameraIO
        				,{Event_CameraIO, Event_Pipe, "CameraIO"}
                            ,{Event_CamIO_dropPreviewFrame, Event_CameraIO, "dropPreviewFrame"}
                            ,{Event_CamIO_start, Event_CameraIO, "start"}
                            ,{Event_CamIO_waitCaptureDone, Event_CameraIO, "waitCaptureDone"}

                        // --Define the event info used in IspHal
                        ,{Event_IspHal, Event_Pipe, "IspHal"}
                            ,{Event_IspHal_init, Event_IspHal, "init"}
                            ,{Event_IspHal_setConf, Event_IspHal, "setConf"}
                            ,{Event_IspHal_uninit, Event_IspHal, "uninit"}
                            ,{Event_uninitMCUDrv, Event_IspHal_uninit, "uninitMCUDrv"}
                            ,{Event_IspHal_allocM4UMemory, Event_IspHal, "allocM4UMemory"}
            		        ,{Event_IspHal_freeM4UMemory, Event_IspHal, "freeM4UMemory"}

                    // Define the event info used in Driver
    	            ,{Event_Driver, Event_Camera, "Driver"}

                        // --Define the event info in sensor driver
                        ,{Event_Sensor, Event_Driver, "Sensor"}
                            ,{Event_Sensor_search, Event_Sensor, "searchSensor"}
            		        ,{Event_Sensor_open, Event_Sensor, "open"}
            		        ,{Event_Sensor_close, Event_Sensor, "close"}
            		        ,{Event_Sensor_setScenario, Event_Sensor, "setScenario"}
				};

static MMP_Event gMMPEvent[Event_Max_Num];
static bool gbInit = false;

void initCameraProfile()
{
	if(!gbInit)
	{
		gMMPEvent[Event_Camera] = MMProfileRegisterEvent(MMP_RootEvent, "Camera");

        if(0 == gMMPEvent[Event_Camera])
        {
            LOGW("register failed, maybe not enable MMProfile");
            return;
        }

		for(int i=1; i<Event_Max_Num; i++ )
		{
		    if(NULL == gCPTEventInfo[i].name)
                continue;

			gMMPEvent[gCPTEventInfo[i].event] = MMProfileRegisterEvent(gMMPEvent[gCPTEventInfo[i].parent], gCPTEventInfo[i].name);
			LOGD("Event: %s is registered as id %d", gCPTEventInfo[i].name, gMMPEvent[gCPTEventInfo[i].event]);
		}

		gbInit = true;
	}
	return;
}

bool CPTEnableEvent(CPT_Event event, int enable)
{
    MMProfileEnableEvent(gMMPEvent[event], enable);
    return true;
}

bool CPTLog(CPT_Event event, CPT_LogType type)
{
    if(!gbInit)
    {
        return true;
    }
    switch(type){
    	case 	CPTFlagStart:
    		MMProfileLog(gMMPEvent[event], MMProfileFlagStart);
    		break;
    	case 	CPTFlagEnd:
    		MMProfileLog(gMMPEvent[event], MMProfileFlagEnd);
    		break;
    	case 	CPTFlagPulse:
    		MMProfileLog(gMMPEvent[event], MMProfileFlagPulse);
    		break;
        case CPTFlagSeparator:
    		MMProfileLog(gMMPEvent[event], MMProfileFlagEventSeparator);
    		break;
    	default:
    		break;
    }

    return true;
}

bool CPTLogEx(CPT_Event event, CPT_LogType type, unsigned int data1, unsigned int data2)
{
    if(!gbInit)
    {
        return true;
    }
    switch(type){
        case    CPTFlagStart:
            MMProfileLogEx(gMMPEvent[event], MMProfileFlagStart, data1, data2);
            break;
        case    CPTFlagEnd:
            MMProfileLogEx(gMMPEvent[event], MMProfileFlagEnd, data1, data2);
            break;
        case    CPTFlagPulse:
            MMProfileLogEx(gMMPEvent[event], MMProfileFlagPulse, data1, data2);
            break;
        case    CPTFlagSeparator:
            MMProfileLogEx(gMMPEvent[event], MMProfileFlagEventSeparator, data1, data2);
            break;
        default:
            break;
    }

    return true;

}

bool CPTLogStr(CPT_Event event, CPT_LogType type, const char* str)
{
    if(!gbInit)
    {
        return true;
    }
    switch(type){
    	case 	CPTFlagStart:
    		MMProfileLogMetaString(gMMPEvent[event], MMProfileFlagStart, str);
    		break;
    	case 	CPTFlagEnd:
    		MMProfileLogMetaString(gMMPEvent[event], MMProfileFlagEnd, str);
    		break;
    	case 	CPTFlagPulse:
    		MMProfileLogMetaString(gMMPEvent[event], MMProfileFlagPulse, str);
    		break;
        case    CPTFlagSeparator:
            MMProfileLogMetaString(gMMPEvent[event], MMProfileFlagEventSeparator, str);
            break;
    	default:
    		break;
    }
    return true;
}

bool CPTLogStrEx(CPT_Event event, CPT_LogType type, unsigned int data1, unsigned int data2, const char* str)
{
	if(!gbInit)
	{
        return true;
	}
	switch(type){
		case 	CPTFlagStart:
			MMProfileLogMetaStringEx(gMMPEvent[event], MMProfileFlagStart, data1, data2, str);
			break;
		case 	CPTFlagEnd:
			MMProfileLogMetaStringEx(gMMPEvent[event], MMProfileFlagEnd, data1, data2, str);
			break;
		case 	CPTFlagPulse:
			MMProfileLogMetaStringEx(gMMPEvent[event], MMProfileFlagPulse, data1, data2, str);
			break;
        case    CPTFlagSeparator:
            MMProfileLogMetaStringEx(gMMPEvent[event], MMProfileFlagEventSeparator, data1, data2, str);
            break;
		default:
			break;
	}
    return true;
}

AutoCPTLog::AutoCPTLog(CPT_Event event, unsigned int data1, unsigned int data2)
{
    mEvent = event;
    mData1 = data1;
	mData2 = data2;

	if(!gbInit)
	{
		return;
	}

    MMProfileLogEx(gMMPEvent[mEvent], MMProfileFlagStart, mData1, mData2);
}
AutoCPTLog:: ~AutoCPTLog()
{

    MMProfileLogEx(gMMPEvent[mEvent], MMProfileFlagEnd, mData1, mData2);
}


#else

void initCameraProfile()
{
	return;
}

bool CPTEnableEvent(CPT_Event event, int enable)
{
	return true;
}

bool CPTLog(CPT_Event event, CPT_LogType type)
{
	return true;
}
bool CPTLogEx(CPT_Event event, CPT_LogType type, unsigned int data1, unsigned int data2)
{
	return true;
}
bool CPTLogStr(CPT_Event event, CPT_LogType type, const char* str)
{
	return true;
}

bool CPTLogStrEx(CPT_Event event, CPT_LogType type, unsigned int data1, unsigned int data2, const char* str)
{
	return true;
}

AutoCPTLog::AutoCPTLog(CPT_Event event, unsigned int data1, unsigned int data2)
{
	return;
}
AutoCPTLog:: ~AutoCPTLog()
{

    return;
}

#endif


////////////////////////////////////////////////////////////////////////////////


