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
#define LOG_TAG "mHalCamN3D"
//
#include <utils/Errors.h>
#include <utils/threads.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>
//
#include <mhal/inc/camera.h>
//
#include "CamExifN3D.h"
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
CamExifN3D::
CamExifN3D(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
    : meSensorType(eSensorType)
    , meDeviceId(eDeviceId)
    , mCamExifParam()
    , mpHal3A(NULL)
{
}


/*******************************************************************************
*
********************************************************************************/
CamExifN3D::
~CamExifN3D()
{
    uninit();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExifN3D::
init(CamExifParam const& rCamExifParam, Hal3ABase*const pHal3A)
{
    if  ( ! pHal3A )
    {
        MY_LOGE("[CamExifN3D::init] NULL pHal3A");
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
CamExifN3D::
uninit()
{
    mpHal3A = NULL;
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExifN3D::
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
            "[CamExifN3D::makeExifApp1] invalid parameters:(puApp1Buf, u4ImgWidth, u4ImgHeight)=(%p, %d, %d)"
            , puApp1Buf, u4ImgWidth, u4ImgHeight
        );
        goto lbExit;
    }

    //  (1) Fill exifApp1Info
    ::memset(&exifApp1Info, 0, sizeof(exifAPP1Info_t));
    if  ( ! queryExifApp1Info(&exifApp1Info) )
    {
        MY_LOGE("[CamExifN3D::makeExifApp1] queryExifApp1Info");
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
        MY_LOGE("[CamExifN3D::makeExifApp1] exifApp1Make - err(%x)", err);
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
CamExifN3D::
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
CamExifN3D::
mapExpProgramIdx(MUINT32 const u4ExpProgram)
{
    using namespace NSFeature;
    MUINT32 eExpProgramId = eExpProgramId_Manual;
    switch  (u4ExpProgram)
    {
    case SCENE_MODE_OFF:
    case SCENE_MODE_NORMAL:
    case SCENE_MODE_NIGHTPORTRAIT:
    case SCENE_MODE_NIGHTSCENE:
    case SCENE_MODE_THEATRE:
    case SCENE_MODE_BEACH:
    case SCENE_MODE_SNOW:
    case SCENE_MODE_SUNSET:
    case SCENE_MODE_STEADYPHOTO:
    case SCENE_MODE_FIREWORKS:
    case SCENE_MODE_SPORTS:
    case SCENE_MODE_PARTY:
    case SCENE_MODE_CANDLELIGHT:
        eExpProgramId = eExpProgramId_Manual;
        break;
    case SCENE_MODE_PORTRAIT:
        eExpProgramId = eExpProgramId_Portrait;
        break;
    case SCENE_MODE_LANDSCAPE:
        eExpProgramId = eExpProgramId_Landscape;
        break;
    default:
        eExpProgramId = eExpProgramId_Manual;
        break;
    }
    return  eExpProgramId;

}

#ifdef MTK_NATIVE_3D_SUPPORT
/*******************************************************************************
* new interface, use n3d compiler option for MP patch back
********************************************************************************/
MBOOL
CamExifN3D::
makeExifApp3(
    MBOOL   const bIsN3dEnable,     //  [I] Native3D(AC) Enable
    MUINT8* const puApp3Buf,        //  [O] Pointer to APP3 Buffer
    MUINT32*const pu4App3Size       //  [O] Pointer to APP3 Size
)
{
    MINT32  err = 0;
    MUINT32 u4App3Size = 0;

    MUINT8* pDst = puApp3Buf;
    
    MUINT8* pJpsInfo = NULL;
    MUINT32 jpsSize = 0;
    MUINT32 app3ReturnSize = 0;

    exifAPP3Info_t exifAPP3Info;
    ::memset(&exifAPP3Info, 0, sizeof(exifAPP3Info_t));

    /*
     *  0xFF + Marker Number(1 byte) + Data size(2 bytes) + Data(n bytes)
     *  Data size(jpsSize) contains 'Data size' descriptor (plus 2 bytes).
     */
    jpsSize = sizeof(exifAPP3Info_t) + 2;
    MY_LOGD("[CamExif::makeExifApp3] jpsSize  = %d", jpsSize);
    //
    const char jpsIdentifier[] = { 0x5F, 0x4A, 0x50, 0x53, 0x4A, 0x50, 0x53, 0x5F }; // _JPSJPS_
    ::memcpy(exifAPP3Info.identifier, jpsIdentifier, sizeof(jpsIdentifier));

#define swap16(x) (((x & 0xff00) >> 8) | ((x & 0x00ff) << 8))
    /*
     *  length(2 bytes): the length of stereoscopic descriptor
     *                   it contains 'Length' descriptor (plus 2 bytes).
     *  this value should be written Most Significant Byte first.
     */
    MUINT16 length      = swap16(sizeof(exifAPP3Info.stereoDesc) + 2);
    ::memcpy(exifAPP3Info.length, &length, sizeof(length));
    //
    exifAPP3Info.stereoDesc[0].type[0]          = (unsigned char)SD_MTYPE_STEREOSCOPIC_IMAGE;
    exifAPP3Info.stereoDesc[0].layout[0]        = (unsigned char)SD_LAYOUT_SIDEBYSIDE;
    exifAPP3Info.stereoDesc[0].flags[0]         = (unsigned char)(SD_FULL_HEIGHT | SD_FULL_WIDTH | SD_LEFT_FIELD_FIRST);
    exifAPP3Info.stereoDesc[0].separation[0]    = (unsigned char)0x00;
    //
    MUINT16 cmtSize     = swap16(sizeof(exifAPP3Info.cmt));
    ::memcpy(exifAPP3Info.cmt.size, &cmtSize, sizeof(cmtSize));
    //
    if ( bIsN3dEnable )
    {
        const char jpsComments[] = { 0x5F, 0x4D, 0x54, 0x4B, 0x5F, 0x41, 0x43, 0x5F }; // _MTK_AC_
        ::memcpy(exifAPP3Info.cmt.comment, jpsComments, sizeof(jpsComments));
    }
    //
    err = ::exifAppnMake(3, pDst, (unsigned char*) &exifAPP3Info, jpsSize, &app3ReturnSize);
    pDst += app3ReturnSize;
    u4App3Size += app3ReturnSize;
    //
    if  (pu4App3Size)
    {
        *pu4App3Size = u4App3Size;
    }
    //
    return (0==err);
}
#else
/*******************************************************************************
* old interface
********************************************************************************/
MBOOL
CamExifN3D::
makeExifApp3(MUINT8* const puApp3Buf, MUINT32*const pu4App3Size)
{

    MINT32  err = 0;
    MUINT32 u4App3Size = 0;

    MUINT8* pDst = puApp3Buf;
    
    MUINT8* pJpsInfo = NULL;
    MUINT32 jpsSize = 0;
    MUINT32 app3ReturnSize = 0;

    exifAPP3Info_t exifAPP3Info;
    ::memset(&exifAPP3Info, 0, sizeof(exifAPP3Info_t));

    /*
     *  0xFF + Marker Number(1 byte) + Data size(2 bytes) + Data(n bytes)
     *  Data size(jpsSize) contains 'Data size' descriptor.
     */
    jpsSize = sizeof(exifAPP3Info_t) + 2;
    MY_LOGD("[CamExif::makeExifApp3] jpsSize  = %d", jpsSize);

    const char jpsIdentifier[] = { 0x5F, 0x4A, 0x50, 0x53, 0x4A, 0x50, 0x53, 0x5F }; // _JPSJPS_
    ::memcpy(exifAPP3Info.identifier, jpsIdentifier, sizeof(jpsIdentifier));

    /*
     *  length(2 bytes): the length of stereoscopic descriptor
     *                   it contains 'Length' descriptor (plus 2 bytes).
     *  this value should be written Most Significant Byte first.
     */
    MUINT16 length      = sizeof(exifAPP3Info.stereoDesc) + 2;
    length = (((length & 0xff00) >> 8) | ((length & 0x00ff) << 8)); // swap16 
    exifAPP3Info.length[0] = (unsigned char )((length     ) & 0x00ff);
    exifAPP3Info.length[1] = (unsigned char )((length >> 8) & 0x00ff);
    //
    exifAPP3Info.stereoDesc[0].type[0]          = (MUINT8)SD_MTYPE_STEREOSCOPIC_IMAGE;
    exifAPP3Info.stereoDesc[0].layout[0]        = (MUINT8)SD_LAYOUT_SIDEBYSIDE;
    exifAPP3Info.stereoDesc[0].flags[0]         = (MUINT8)(SD_FULL_HEIGHT | SD_FULL_WIDTH | SD_LEFT_FIELD_FIRST);
    exifAPP3Info.stereoDesc[0].separation[0]    = (MUINT8)0x00;

    err = ::exifAppnMake(3, pDst, (unsigned char*) &exifAPP3Info, jpsSize, &app3ReturnSize);
    pDst += app3ReturnSize;
    u4App3Size += app3ReturnSize;

    if  (pu4App3Size)
    {
        *pu4App3Size = u4App3Size;
    }

    return (0==err);

}
#endif

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamExifN3D::
queryExifApp1Info(exifAPP1Info_s*const pexifApp1Info)
{
    MY_LOGI("[CamExifN3D::queryExifApp1Info] E");

    if  ( ! pexifApp1Info )
    {
        MY_LOGE("[CamExifN3D::queryExifApp1Info] NULL pexifApp1Info");
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
        MY_LOGE("[CamExifN3D::queryExifApp1Info] mrHal3A.get3AEXIFInfo()");
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
    MY_LOGI("[CamExifN3D::queryExifApp1Info] X");

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
CamExifN3D::
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
        "[CamExifN3D::determineExifOrientation] - "
        "(u4DeviceOrientation, bIsFacing, bIsFacingFlip, result)=(%d, %d, %d, %d)"
        , u4DeviceOrientation, bIsFacing, bIsFacingFlip, result
    );

    return  result;
}

