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
#define LOG_TAG "mHalCam"
//
#include <utils/Errors.h>
#include <utils/threads.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>
//
#include <mhal/inc/camera.h>
//
#include "CamExif.h"
//
#include <camera_custom_if.h>
#include <camera_feature.h>
#include <exif_api.h>
#include "exif_type.h"
#include <aaa_hal_base.h>


/*******************************************************************************
*
********************************************************************************/
#define MY_LOGD(fmt, arg...)        XLOGD(fmt, ##arg)
#define MY_LOGI(fmt, arg...)        XLOGI(fmt, ##arg)
#define MY_LOGW(fmt, arg...)        XLOGW(fmt, ##arg)
#define MY_LOGE(fmt, arg...)        XLOGE(fmt"<%s:#%d>", ##arg, __FILE__, __LINE__)
//
#if (SHOT_ASSERT_ON)
    #define MY_LOGA(x, fmt, arg...) \
        if ( ! (x) ) { \
            printf(fmt"<Assert %s:#%d>", ##arg, __FILE__, __LINE__); \
            LOGE  (fmt"<Assert %s:#%d>", ##arg, __FILE__, __LINE__); \
            while (1) { ::usleep(500 * 1000); } \
        }
#else
    #define MY_LOGA(x, fmt, arg...)
#endif


/*******************************************************************************
*
********************************************************************************/
CamExif::
CamExif(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
    : meSensorType(eSensorType)
    , meDeviceId(eDeviceId)
    , mCamExifParam()
    , mpHal3A(NULL)
{
}


/*******************************************************************************
*
********************************************************************************/
CamExif::
~CamExif()
{
    uninit();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
init(CamExifParam const& rCamExifParam, Hal3ABase*const pHal3A)
{
    if  ( ! pHal3A )
    {
        MY_LOGE("[CamExif::init] NULL pHal3A");
        return  MFALSE;
    }

    mCamExifParam = rCamExifParam;
    mpHal3A = pHal3A;
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
uninit()
{
    mpHal3A = NULL;
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
makeExifApp1(
    MUINT32 const u4ImgWidth,           //  [I] Image Width
    MUINT32 const u4ImgHeight,          //  [I] Image Height
    MUINT32 const u4ThumbSize,          //  [I] Thumb Size
    MUINT8* const puApp1Buf,            //  [O] Pointer to APP1 Buffer
    MUINT32*const pu4App1HeaderSize     //  [O] Pointer to APP1 Header Size
)
{
    MINT32  err = -1;
    //
    exifAPP1Info_t exifApp1Info;
    exifImageInfo_t exifImgInfo;
    //
    MUINT32 u4App1HeaderSize = 0;

    //  (0) Check arguments.
    if  ( ! puApp1Buf || 0 == u4ImgWidth || 0 == u4ImgHeight )
    {
        MY_LOGE(
            "[CamExif::makeExifApp1] invalid parameters:(puApp1Buf, u4ImgWidth, u4ImgHeight)=(%p, %d, %d)"
            , puApp1Buf, u4ImgWidth, u4ImgHeight
        );
        goto lbExit;
    }

    //  (1) Fill exifApp1Info
    ::memset(&exifApp1Info, 0, sizeof(exifAPP1Info_t));
    if  ( ! queryExifApp1Info(&exifApp1Info) )
    {
        MY_LOGE("[CamExif::makeExifApp1] queryExifApp1Info");
        goto lbExit;
    }

    //  (2) Fill exifImgInfo
    ::memset(&exifImgInfo, 0, sizeof(exifImageInfo_t));
    exifImgInfo.bufAddr     = reinterpret_cast<MUINT32>(puApp1Buf);
    exifImgInfo.mainWidth   = u4ImgWidth;
    exifImgInfo.mainHeight  = u4ImgHeight;
    exifImgInfo.thumbSize   = u4ThumbSize;
    err = ::exifApp1Make(&exifImgInfo, &exifApp1Info, &u4App1HeaderSize);
    if  ( err )
    {
        MY_LOGE("[CamExif::makeExifApp1] exifApp1Make - err(%x)", err);
        goto lbExit;
    }

    if  (pu4App1HeaderSize)
    {
        *pu4App1HeaderSize = u4App1HeaderSize;
    }

lbExit:
    return  (0==err);
}

/*******************************************************************************
*
********************************************************************************/
MUINT32
CamExif::
mapCapTypeIdx(MUINT32 const u4CapType)
{
    using namespace NSFeature;
    MUINT32 eCapTypeId = eCapTypeId_Standard;
    switch  (u4CapType)
    {
    case SCENE_MODE_OFF:
    case SCENE_MODE_NORMAL:
    case SCENE_MODE_NIGHTPORTRAIT:
    case SCENE_MODE_THEATRE:
    case SCENE_MODE_BEACH:
    case SCENE_MODE_SNOW:
    case SCENE_MODE_SUNSET:
    case SCENE_MODE_STEADYPHOTO:
    case SCENE_MODE_FIREWORKS:
    case SCENE_MODE_SPORTS:
    case SCENE_MODE_PARTY:
    case SCENE_MODE_CANDLELIGHT:
        eCapTypeId = eCapTypeId_Standard;
        break;
    case SCENE_MODE_PORTRAIT:
        eCapTypeId = eCapTypeId_Portrait;
        break;
    case SCENE_MODE_LANDSCAPE:
        eCapTypeId = eCapTypeId_Landscape;
        break;
    case SCENE_MODE_NIGHTSCENE:
        eCapTypeId = eCapTypeId_Night;
        break;
    default:
        eCapTypeId = eCapTypeId_Standard;
        break;
    }
    return  eCapTypeId;

}

/*******************************************************************************
*
********************************************************************************/
MUINT32
CamExif::
mapExpProgramIdx(MUINT32 const u4SceneMode)
{
    using namespace NSFeature;
    using namespace NSCamCustom;
    MUINT32 eExpProgramId   = eExpProgramId_NotDefined;
    customExif_t customExif;
    switch  (u4SceneMode)
    {
        case SCENE_MODE_PORTRAIT:
            eExpProgramId = eExpProgramId_Portrait;
            break;
        case SCENE_MODE_LANDSCAPE:
            eExpProgramId = eExpProgramId_Landscape;
            break;
        default:
        {
            customExif = getCustomExif();
            if ( customExif.bEnCustom )
            {
                MY_LOGD("[mapExpProgramIdx] bEnCustom(%d), u4ExpProgram(%d)", customExif.bEnCustom, customExif.u4ExpProgram);
                switch (customExif.u4ExpProgram)
                {
                case eExpProgramId_Manual:
                    eExpProgramId = eExpProgramId_Manual;
                    break;
                case eExpProgramId_Normal:
                    eExpProgramId = eExpProgramId_Normal;
                    break;
                case eExpProgramId_NotDefined:
                default:
                    eExpProgramId = eExpProgramId_NotDefined;
                    break;
                }
            }
            else
            {
                eExpProgramId = eExpProgramId_NotDefined;
            }
        }
        break;
    }
    return  eExpProgramId;

}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExif::
queryExifApp1Info(exifAPP1Info_s*const pexifApp1Info)
{
    MY_LOGI("[CamExif::queryExifApp1Info] E");

    if  ( ! pexifApp1Info )
    {
        MY_LOGE("[CamExif::queryExifApp1Info] NULL pexifApp1Info");
        return  MFALSE;
    }

    const int evBias[25] = { 0,  3,  5,  7, 10,
                            13, 15, 17, 20, -3,
                            -5, -7, 30,-13,-15,
                           -17,-10,  0,  0,  0,
                           -20,  0,  0,  0,-30};
    const unsigned short aeMeterMode[4] = {2,3,1,4};
    const unsigned short lightSource[10] = {0,255,1,10,11,255,2,255,255,3};
    time_t t;
    struct tm tm;

    if  (mCamExifParam.gpsIsOn == 1)
    {
        float latitude = atof(mCamExifParam.gpsLatitude);
        float longitude = atof(mCamExifParam.gpsLongitude);
        long long timestamp = atol(mCamExifParam.gpsTimeStamp);
        char const*pgpsProcessingMethod = mCamExifParam.gpsProcessingMethod;
        //
        // Set GPS Info
        if (latitude >= 0) {
            strcpy((char *) pexifApp1Info->gpsLatitudeRef, "N");
        }
        else {
            strcpy((char *) pexifApp1Info->gpsLatitudeRef, "S");
            latitude *= -1;     // make it positive
        }
        if (longitude >= 0) {
            strcpy((char *) pexifApp1Info->gpsLongitudeRef, "E");
        }
        else {
            strcpy((char *) pexifApp1Info->gpsLongitudeRef, "W");
            longitude *= -1;    // make it positive
        }
        pexifApp1Info->gpsIsOn = 1;
        // Altitude
        pexifApp1Info->gpsAltitude[0] = mCamExifParam.gpsAltitude;
        pexifApp1Info->gpsAltitude[1] = 1;
        // Latitude
        pexifApp1Info->gpsLatitude[0] = (int) latitude;
        pexifApp1Info->gpsLatitude[1] = 1;
        latitude -= pexifApp1Info->gpsLatitude[0];
        latitude *= 60;
        pexifApp1Info->gpsLatitude[2] = (int) latitude;
        pexifApp1Info->gpsLatitude[3] = 1;
        latitude -= pexifApp1Info->gpsLatitude[2];
        latitude *= 60;
        latitude *= 10000;
        pexifApp1Info->gpsLatitude[4] = (int) latitude;
        pexifApp1Info->gpsLatitude[5] = 10000;
        // Longtitude
        pexifApp1Info->gpsLongitude[0] = (int) longitude;
        pexifApp1Info->gpsLongitude[1] = 1;
        longitude -= pexifApp1Info->gpsLongitude[0];
        longitude *= 60;
        pexifApp1Info->gpsLongitude[2] = (int) longitude;
        pexifApp1Info->gpsLongitude[3] = 1;
        longitude -= pexifApp1Info->gpsLongitude[2];
        longitude *= 60;
        longitude *= 10000;
        pexifApp1Info->gpsLongitude[4] = (int) longitude;
        pexifApp1Info->gpsLongitude[5] = 10000;

        // Timestamp
        time_t tim = (time_t) timestamp;
        struct tm *ptime = gmtime(&tim);
        pexifApp1Info->gpsTimeStamp[0] = ptime->tm_hour;
        pexifApp1Info->gpsTimeStamp[1] = 1;
        pexifApp1Info->gpsTimeStamp[2] = ptime->tm_min;
        pexifApp1Info->gpsTimeStamp[3] = 1;
        pexifApp1Info->gpsTimeStamp[4] = ptime->tm_sec;
        pexifApp1Info->gpsTimeStamp[5] = 1;
        sprintf((char *) pexifApp1Info->gpsDateStamp, "%04d:%02d:%02d", ptime->tm_year + 1900, ptime->tm_mon + 1, ptime->tm_mday);
        // ProcessingMethod
        const char exifAsciiPrefix[] = { 0x41, 0x53, 0x43, 0x49, 0x49, 0x0, 0x0, 0x0 };
        int len1, len2, maxLen;
        len1 = sizeof(exifAsciiPrefix);
        memcpy(pexifApp1Info->gpsProcessingMethod, exifAsciiPrefix, len1);
        maxLen = sizeof(pexifApp1Info->gpsProcessingMethod) - len1;
        len2 = strlen(pgpsProcessingMethod);
        if (len2 > maxLen) {
            len2 = maxLen;
        }
        memcpy(&pexifApp1Info->gpsProcessingMethod[len1], pgpsProcessingMethod, len2);
    }

    /*********************************************************************************
                                           common
    **********************************************************************************/
    //software information
    memset(pexifApp1Info->strSoftware,0,32);
    strcpy((char *)pexifApp1Info->strSoftware, "MediaTek Camera Application");
    //get datetime
    tzset();
    time(&t);
    localtime_r(&t, &tm);
    strftime((char *)pexifApp1Info->strDateTime, 20, "%Y:%m:%d %H:%M:%S", &tm);

    /*********************************************************************************
                                       from CamExifParam
    **********************************************************************************/
    //digital zoom ratio
    pexifApp1Info->digitalZoomRatio[0] = mCamExifParam.u4ZoomRatio;
    pexifApp1Info->digitalZoomRatio[1] = 100;
    //metering mode
    if (mCamExifParam.aeMeterMode < 0 || mCamExifParam.aeMeterMode > 3) {
        pexifApp1Info->meteringMode = aeMeterMode[0];
    }
    else {
        pexifApp1Info->meteringMode = aeMeterMode[mCamExifParam.aeMeterMode];
    }
    //focal length
    using namespace NSCamCustom;
    SensorExifInfo_T sensorExif;
    sensorExif = getParamSensorExif();
    pexifApp1Info->focalLength[0] = sensorExif.uFLengthNum;
    pexifApp1Info->focalLength[1] = (sensorExif.uFLengthDenom == 0) ? 10 : sensorExif.uFLengthDenom;
    MY_LOGI("[CamExif::queryExifApp1Info] focalLength(%d/%d)", sensorExif.uFLengthNum, sensorExif.uFLengthDenom);
    //exposure program , scene mode
    using namespace NSFeature;
    MUINT32 u4SceneMode = mCamExifParam.sceneMode;
    MUINT32 u4CapTypeIdx = eCapTypeId_Standard;
    MUINT32 u4ExpProgramIdx = eExpProgramId_Manual;
    if  ( SCENE_MODE_NUM <= u4SceneMode )
    {
        u4SceneMode = SCENE_MODE_OFF;
    }
    u4ExpProgramIdx = mapExpProgramIdx(u4SceneMode);
    pexifApp1Info->exposureProgram = u4ExpProgramIdx;

    u4CapTypeIdx = mapCapTypeIdx(u4SceneMode);
    pexifApp1Info->sceneCaptureType = u4CapTypeIdx;

    //Ev offset
    //pexifApp1Info->exposureBiasValue[0] = evBias[r3AEXIFInfo.eAEComp];
    pexifApp1Info->exposureBiasValue[0] = evBias[mCamExifParam.aeExpMode];
    pexifApp1Info->exposureBiasValue[1] = 10;
    //light source
    pexifApp1Info->lightSource = lightSource[mCamExifParam.awbMode];
    //flashPixVer
    memcpy(pexifApp1Info->strFlashPixVer,"0100 ",5);
    //exposure mode
    pexifApp1Info->exposureMode = 0;
    //

    /*********************************************************************************
                                           from 3A
    **********************************************************************************/
    //sensor type dependency    
    AAA_EXIF_INFO_T r3AEXIFInfo;
    //get 3A exif info.
    if  ( 0 != mpHal3A->get3AEXIFInfo(&r3AEXIFInfo) )
    {
        MY_LOGE("[CamExif::queryExifApp1Info] mrHal3A.get3AEXIFInfo()");
    }

    //
    //f number
    pexifApp1Info->fnumber[0] = r3AEXIFInfo.u4FNumber;
    pexifApp1Info->fnumber[1] = 10;

    //iso speed
    //pexifApp1Info->isoSpeedRatings = (unsigned short)100;
    if (r3AEXIFInfo.eAEISOSpeed <= LIB3A_AE_ISO_SPEED_AUTO) {
        //r3AEXIFInfo.eAEISOSpeed = AE_ISO_SPEED_100;
        pexifApp1Info->isoSpeedRatings = (unsigned short)r3AEXIFInfo.u4RealISOValue;
    }
    else {
        if (r3AEXIFInfo.eAEISOSpeed > LIB3A_AE_ISO_SPEED_3200) {
            r3AEXIFInfo.eAEISOSpeed = LIB3A_AE_ISO_SPEED_3200;
        }
        pexifApp1Info->isoSpeedRatings = (unsigned short)r3AEXIFInfo.eAEISOSpeed;
    }

    //esposure time
    if(r3AEXIFInfo.u4CapExposureTime == 0){
        //YUV sensor
        pexifApp1Info->exposureTime[0] = 0;
        pexifApp1Info->exposureTime[1] = 0;
    }
     else{
        //RAW sensor
        if (r3AEXIFInfo.u4CapExposureTime > 1000000) { //1 sec
            pexifApp1Info->exposureTime[0] = r3AEXIFInfo.u4CapExposureTime/100000;
            pexifApp1Info->exposureTime[1] = 10;
        }
        else{ // us
            pexifApp1Info->exposureTime[0] = r3AEXIFInfo.u4CapExposureTime;
            pexifApp1Info->exposureTime[1] = 1000000;
        }
     }

    //flashlight 
    pexifApp1Info->flash = (0 != r3AEXIFInfo.u4FlashLightTimeus)?1:0;

    // white balance mode
    pexifApp1Info->whiteBalanceMode = (0 != r3AEXIFInfo.eAWBMode)?1:0;
    
 

    pexifApp1Info->orientation = determineExifOrientation(
        mCamExifParam.orientation, 
        (NSCamera::eDevId_ImgSensor1 == meDeviceId)     //  front device
    );

    //  update customized exif
    {
        using namespace NSCamCustom;
        customExifInfo_t* pCustomExifInfo = NULL;
        if  ( 0 == custom_SetExif((void **)&pCustomExifInfo) )
        {
            if  ( pCustomExifInfo )
            {
                //Make
                if ( 32 >= ::strlen((const char*)pCustomExifInfo->strMake) )
                {
                    ::strcpy((char*)pexifApp1Info->strMake, (const char*)pCustomExifInfo->strMake);
                }

                //Model
                if ( 32 >= ::strlen((const char*)pCustomExifInfo->strModel) )
                {
                    ::strcpy((char*)pexifApp1Info->strModel, (const char*)pCustomExifInfo->strModel);
                }

                //software
                ::memset(pexifApp1Info->strSoftware,0,32);
                if ( 32 >= ::strlen((const char*)pCustomExifInfo->strSoftware) )
                {
                    ::strcpy((char*)pexifApp1Info->strSoftware, (const char*)pCustomExifInfo->strSoftware);
                }
            }
        }
        else
        {
            MY_LOGW("[queryExifApp1Info] do not update customized exif");
        }
    }
    MY_LOGI("[CamExif::queryExifApp1Info] X");

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
CamExif::
determineExifOrientation(
    MUINT32 const   u4DeviceOrientation, 
    MBOOL const     bIsFacing, 
    MBOOL const     bIsFacingFlip /*= MFALSE*/
)
{
    MINT32  result = -1;

    if  ( bIsFacing && bIsFacingFlip )
    {
        //  Front Camera with Flip
        switch  (u4DeviceOrientation)
        {
        case 0:
            result = 1;
            break;
        case 90:
            result = 8;
            break;
        case 180:
            result = 3;
            break;
        case 270:
            result = 6;
            break;
        default:
            result = 1;
            break;
        }
    }
    else
    {   //  Rear Camera or Front Camera without Flip
        switch  (u4DeviceOrientation)
        {
        case 0:
            result = 1;
            break;
        case 90:
            result = 6;
            break;
        case 180:
            result = 3;
            break;
        case 270:
            result = 8;
            break;
        default:
            result = 1;
            break;
        }
    }

    MY_LOGD(
        "[CamExif::determineExifOrientation] - "
        "(u4DeviceOrientation, bIsFacing, bIsFacingFlip, result)=(%d, %d, %d, %d)"
        , u4DeviceOrientation, bIsFacing, bIsFacingFlip, result
    );

    return  result;
}

