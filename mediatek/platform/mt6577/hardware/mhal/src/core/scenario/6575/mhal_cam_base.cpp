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

#define LOG_TAG "scenario/mHalCam_base"
//
#include <utils/threads.h>	// For "mhal_cam.h" Mutex.
//
#include <mhal/inc/camera.h>
#include <scenario_types.h> // For "mhal_cam_base.h" MINT*/MUINT*/M* type names.
//
#include "mhal_cam_base.h"
#include "mhal_cam.h"

#ifdef  MTK_NATIVE_3D_SUPPORT
    #include <utils/String8.h>
    #include "mhal_cam_n3d.h"
    #include "property/CamUtilsProperty.h"
    using namespace android::MHalCamUtils;
#endif  //MTK_NATIVE_3D_SUPPORT
//
#include <mcam_profile.h>   // For CamTimer
#include "camera_custom_if.h" // For NSCamCustom

#define MHAL_CAM_STEREO_3D_ENABLE       "Camera.Stereo.3D.Enable"
#define MHAL_CAM_STEREO_3D_ENABLE_N     "0"
#define MHAL_CAM_STEREO_3D_ENABLE_Y     "1"
//
#define MHAL_CAM_NATIVE_3D_SUPPORT      "Camera.Native.3D.Support"
#define MHAL_CAM_NATIVE_3D_SUPPORT_N    "0"
#define MHAL_CAM_NATIVE_3D_SUPPORT_Y    "1"

mhalCamSensorInfo_t mHalCamBase::mHalCamSensorInfo[4];
MINT32 mHalCamBase::mRetFakeSubOrientation;

MUINT32 mHalCamBase::mu4Camera3dId;


/******************************************************************************
*
*******************************************************************************/
MINT32
mHalCamBase::
mHalCamQuerySensorInfo(MINT32 cameraId, mhalCamSensorInfo_t* pInfo)
{
    MCAM_DBG("[%s] cameraId:%d pInfo:%p\n", __FUNCTION__, cameraId, pInfo);
    if  ( ! pInfo )
    {
        MCAM_ERR("[%s] pInfo = NULL\n", __FUNCTION__);
        return  -1;
    }
    *pInfo = mHalCamSensorInfo[cameraId];
    return  0;
}


/******************************************************************************
*
*******************************************************************************/

MINT32
mHalCamBase::searchCamera(MVOID *a_pOutBuffer, MUINT32 *pBytesReturned)
{
    MINT32 err = MHAL_NO_ERROR;
    IspHal *pIspHalObj = NULL;
    MUINT32 supportedSensorDevs;
    MINT32 count = 0;
    ResMgrHal* pResMgrHalObj = NULL;
    
    MCAM_DBG("[searchCamera] \n");
    CamTimer camTmr("[searchCamera]", MHAL_CAM_GENERIC_PROFILE_OPT);    
    //
    mhalCamSensorInfo_t *pinfo = &mHalCamSensorInfo[0];
    memset(pinfo, 0, sizeof(mHalCamSensorInfo));
    //
    pResMgrHalObj = ResMgrHal::CreateInstance();
    //
    if(!(pResMgrHalObj->Init()))
    {
        err  = MHAL_INVALID_RESOURCE;
        return err;
    }
    //
    pIspHalObj = IspHal::createInstance();
    supportedSensorDevs = pIspHalObj->searchSensor();
    MCAM_DBG("  supportedSensorDevs: 0x%x \n", supportedSensorDevs);

    MINT32 count_sub = 0;   
    mRetFakeSubOrientation = MFALSE;
    //
    if (supportedSensorDevs & ISP_SENSOR_DEV_MAIN) {

        int viewangles = NSCamCustom::getSensorViewAngle(NSCamCustom::eDevId_ImgSensor0);
        pinfo[count].viewAngles[0] = 0xFFFF & (viewangles>>16);
        pinfo[count].viewAngles[1] = 0xFFFF & (viewangles);

    #if (MHAL_CAM_ISP_TUNING)
        NSCamCustom::SensorOrientation_T orientation;
        orientation = NSCamCustom::getSensorOrientation();
        pinfo[count].orientation = orientation.u4Degree_0;
    #else 
        pinfo[count].orientation = 90; 
    #endif 
        pinfo[count].facing = 0;    // back
        pinfo[count].devType = MHAL_CAM_SENSOR_DEV_MAIN;
        count++;
    }
    if (supportedSensorDevs & ISP_SENSOR_DEV_SUB) {

        int viewangles = NSCamCustom::getSensorViewAngle(NSCamCustom::eDevId_ImgSensor1);
        pinfo[count].viewAngles[0] = 0xFFFF & (viewangles>>16);
        pinfo[count].viewAngles[1] = 0xFFFF & (viewangles);

    #if (MHAL_CAM_ISP_TUNING)
        NSCamCustom::SensorOrientation_T orientation;
        orientation = NSCamCustom::getSensorOrientation();
        pinfo[count].orientation = orientation.u4Degree_1;
        mRetFakeSubOrientation = NSCamCustom::isRetFakeSubOrientation();
    #else 
        pinfo[count].orientation = 0; 
    #endif 

        count_sub = count;
        pinfo[count].facing = 1;
        // FIXME: Should query from sensor        
        pinfo[count].devType = MHAL_CAM_SENSOR_DEV_SUB;
        count++;
    }
    if (supportedSensorDevs & ISP_SENSOR_DEV_ATV) {
        pinfo[count].facing = 0;
        // FIXME: Should query from sensor
        pinfo[count].orientation = 0;
        pinfo[count].devType = MHAL_CAM_SENSOR_DEV_ATV;
        count++;
    }

    *pBytesReturned = count * sizeof(mhalCamSensorInfo_t);
    memcpy((MUINT8 *) a_pOutBuffer, (MUINT8 *) pinfo, *pBytesReturned);

    #ifdef  MTK_NATIVE_3D_SUPPORT
    if (supportedSensorDevs & (ISP_SENSOR_DEV_MAIN | ISP_SENSOR_DEV_MAIN_2)) {
        MCAM_DBG("sensorDevs: support main2");
        property_set(MHAL_CAM_NATIVE_3D_SUPPORT, MHAL_CAM_NATIVE_3D_SUPPORT_Y);
        mu4Camera3dId = count;
        MCAM_DBG("mu4Camera3dId = %d", mu4Camera3dId);

        int viewangles = NSCamCustom::getSensorViewAngle(NSCamCustom::eDevId_ImgSensor0);
        pinfo[count].viewAngles[0] = 0xFFFF & (viewangles>>16);
        pinfo[count].viewAngles[1] = 0xFFFF & (viewangles);

    #if (MHAL_CAM_ISP_TUNING)
        NSCamCustom::SensorOrientation_T orientation;
        orientation = NSCamCustom::getSensorOrientation();
        pinfo[count].orientation = orientation.u4Degree_0;
    #else 
        pinfo[count].orientation = 90; 
    #endif 
        pinfo[count].facing = 0;    // back
        pinfo[count].devType = ISP_SENSOR_DEV_MAIN | ISP_SENSOR_DEV_MAIN_2;
        count++;
    }
    else {
        property_set(MHAL_CAM_NATIVE_3D_SUPPORT, MHAL_CAM_NATIVE_3D_SUPPORT_N);
    }
    #endif  //MTK_NATIVE_3D_SUPPORT
    //
    pIspHalObj->destroyInstance();
    //
    pResMgrHalObj->Uninit();
    pResMgrHalObj->DestroyInstance();
    pResMgrHalObj = NULL;
    //
    mhalCamSensorInfo_t *pOutInfo = (mhalCamSensorInfo_t*)a_pOutBuffer;
    if(MTRUE == mRetFakeSubOrientation)
    { 
	    if(pOutInfo[count_sub].orientation == 0)
    	{
            MCAM_DBG("return fake orientation(90) for front sensor \n");
            pOutInfo[count_sub].orientation = 90;
    	}
	    else if(pOutInfo[count_sub].orientation == 180)
        {
            MCAM_DBG("return fake orientation(270) for front sensor \n");
            pOutInfo[count_sub].orientation = 270;
	    }
	    else if(pOutInfo[count_sub].orientation == 90)
        {
            MCAM_DBG("return fake orientation(270) for front sensor \n");
            pOutInfo[count_sub].orientation = 270;
	    }	    
    }
    
    camTmr.printTime(); 
    return err;
}


/******************************************************************************
*
*******************************************************************************/
mHalCamBase*
mHalCamBase::
createInstance()
{
    MCAM_DBG("[createInstance] \n");
    //
    MBOOL isEnable3D = MFALSE;
    MBOOL isNative3D = MFALSE;
    char value_3d[PROPERTY_VALUE_MAX] = {'\0'};

    #ifdef  MTK_NATIVE_3D_SUPPORT

    Property::get(MHAL_CAM_STEREO_3D_ENABLE, value_3d, MHAL_CAM_STEREO_3D_ENABLE_N);
    isEnable3D = (MBOOL)(atoi(value_3d));
    LOGD("MHAL_CAM_STEREO_3D_ENABLE(%d)", isEnable3D);
    //
    property_get(MHAL_CAM_NATIVE_3D_SUPPORT, value_3d, MHAL_CAM_NATIVE_3D_SUPPORT_N);
    isNative3D = (MBOOL)(atoi(value_3d));
    LOGD("MHAL_CAM_NATIVE_3D_SUPPORT(%d)", isNative3D);
    //
    if (isEnable3D) {
        if (isNative3D) {
            LOGD("[createInstance] N3D mode");
            return mHalCamN3D::getInstance();
        }
        else {
            LOGD("[createInstance] B3D mode");
            return mHalCam::getInstance();
        }
    }
    else {
        LOGD("[createInstance] 2D mode");
        return mHalCam::getInstance();
    }
    #else

        LOGD("[createInstance] 2D mode (not define N3D)");
        return mHalCam::getInstance();
    
    #endif  //MTK_NATIVE_3D_SUPPORT

}

mHalCamBase::
~mHalCamBase()
{
#ifdef MTK_NATIVE_3D_SUPPORT
    Property::clear();
#endif //MTK_NATIVE_3D_SUPPORT
}

