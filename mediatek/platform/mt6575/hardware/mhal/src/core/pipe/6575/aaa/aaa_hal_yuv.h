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

#ifndef _AAA_HAL_YUV_H_
#define _AAA_HAL_YUV_H_

#include "aaa_hal_base.h"
#include "strobe_drv.h"
#include "camera_custom_flashlight.h"
#include "mcu_drv.h"
#include "MTKDetection.h"

#include "sensor_drv.h"



class StrobeDrv;
class SensorDrv;
class MCUDrv;

/*******************************************************************************
*
********************************************************************************/
class Hal3AYuv : public Hal3ABase {
public:
    //static Hal3ABase* getInstance();
    static Hal3ABase* getInstance(MINT32 sensorDev);

    MVOID setIsBurtShootMode(MINT32 isBurst);

private:
    Hal3AYuv(MINT32 sensorDev);
    virtual ~Hal3AYuv();

    MINT32 isAEFlashOn();

    StrobeDrv *m_pStrobeDrvObj;
    SensorDrv *m_pSensorDrvObj;

    AAA_DEBUG_INFO_T m_r3ADebugInfo;

    AAA_STATE_T m_e3AState;
    MINT32 m_aeFlashlightType;
    MINT32 m_aeStrobeMode;
    MBOOL m_bReadyForCapture;
		
    MUINT32 m_strobeWidth;
    MUINT32 m_awbMode;

//for yuv+af+fd
    MINT32 m_imageXS;
    MINT32 m_imageYS;
    MINT32 m_AFzone[6];
    MBOOL m_bFDFaceFound;
    MINT32 m_u4AFMode;
    //MBOOL m_bisPreview;
    FD_INFO_T m_rFDInfo;
    EZOOM_WIN_T m_rEZoomInfo;

    MBOOL m_bIsdummylens;
    MBOOL m_bDecideAFSceneDetectingColorWhite;
    //TRUE=AF_MARK_NORMAL, FALSE=AF_MARK_OK

//for yuv movespot
    MBOOL m_bIsMoveSpotMeter;
    MINT32 m_touch_zone[6];//x, y, left, right, up, bottom,
    MBOOL m_bInSingleFocus;
    MINT32 m_u4SensorFocusStatus;
    MBOOL m_bHasBeenInRect;

//for sensor af error state timing 900ms
    MINT32 m_u4AFErrorCount;

//for ASD
    MINT32 m_i4AELv_x10;
    MBOOL m_bAEStable;
    SENSOR_AE_AWB_REF_STRUCT m_ref;

    MBOOL m_bFlashActive;

// For ICS4.0 provide some APIs for control.
    AREA_T m_sAFAREA[AF_WIN_NUM_MATRIX];
    MINT32 m_max_focus_areas;
    AREA_T m_sAEAREA[1]; //touch AE has only one window
    MINT32 m_AEzone[6];
    MINT32 m_max_metering_areas;

    MBOOL bIsAELocked;
    MBOOL bIsAWBLocked;

//3A EXIF info
    SENSOR_EXIF_INFO_STRUCT m_exifInfo;
    SENSOR_FLASHLIGHT_AE_INFO_STRUCT m_AEFlashlightInfo;
    SENSOR_FLASHLIGHT_AE_INFO_STRUCT m_AEHighCurrModeInfo;
    double BV_THRESHOLD;	
    double m_strobeTrigerBV;	
    double m_strobePreflashBV;
	double m_strobecurrent_BV;
	int pre_shutter;
	int pre_gain;
    /////////////////////////////////////////////////////////////////////////
    //
    // setConf () -
    //! \brief set imgsensor/ATV
    //
    /////////////////////////////////////////////////////////////////////////
    MINT32 setConf(MINT32 sensorDev);

    /////////////////////////////////////////////////////////////////////////
    //
    // getFDInfo () -
    //! \brief get FD info
    //
    /////////////////////////////////////////////////////////////////////////
    MINT32 getFDInfo(FD_INFO_T &a_sFDInfo);

    /////////////////////////////////////////////////////////////////////////
    //
    // getEZoomInfo () -
    //! \brief get zoom info
    //
    /////////////////////////////////////////////////////////////////////////
    MVOID getEZoomInfo(EZOOM_WIN_T &a_eZoomInfo);

    /////////////////////////////////////////////////////////////////////////
    //
    // setAFMoveSpotToSensor () -
    //! \brief set movespot info to m_zone
    //
    /////////////////////////////////////////////////////////////////////////
    void setAFMoveSpotToSensor(MUINT32 a_u4Left,MUINT32 a_u4Right,MUINT32 a_u4Up,MUINT32 a_u4Bottom,MUINT32 a_u4DispW,MUINT32 a_u4DispH,MUINT8 a_uOri);

    /////////////////////////////////////////////////////////////////////////
    //
    // trigger3AAutoFrameRatePreviewStateAF() -
    //! \brief triggerAF
    //
    /////////////////////////////////////////////////////////////////////////
    MINT32 trigger3AAutoFrameRatePreviewStateAF();
    /////////////////////////////////////////////////////////////////////////
    //
    // trigger3AAFStateAF() -
    //! \brief triggerAF
    //
    /////////////////////////////////////////////////////////////////////////
    MINT32 trigger3AAFStateAF();
    /////////////////////////////////////////////////////////////////////////
    //
    // setAFMode () -
    //! \brief set AF mode
    //        AF_MODE_AFS     =   AF_MODE_BEGIN, = 0 // (AF-Single Shot Mode)
    //    AF_MODE_AFC,                        // AF-Continuous Mode
    //    AF_MODE_MACRO,                      // AF Macro Mode
    //    AF_MODE_INFINITY,
    //
    /////////////////////////////////////////////////////////////////////////
    MINT32 setAFMode(MINT32 AFMode);
    /////////////////////////////////////////////////////////////////////////
    //
    // calcASDEv () -
    //! \caculate Ev
    //
    /////////////////////////////////////////////////////////////////////////
    void calcASDEv(SENSOR_AE_AWB_CUR_STRUCT cur);

    MINT32 ASDLog2Func(MUINT32 numerator, MUINT32 denominator);
    MINT32 clamp(MINT32 x, MINT32 min, MINT32 max);
    MINT32 clampMaxNumAreas(MINT32 x, MINT32 min, MINT32 max);
    void mapAeraToZone(AREA_T *p_area, MINT32 areaW, MINT32 areaH, MINT32* p_zone, MINT32 zoneW, MINT32 zoneH);
    void updateAFAeraAndZoneInfo(MINT32 x0, MINT32 y0, MINT32 x1, MINT32 y1, MINT32 areaW, MINT32 areaH);
    void updateAEAeraAndZoneInfo(MINT32 x0, MINT32 y0, MINT32 x1, MINT32 y1, MINT32 areaW, MINT32 areaH);

    MBOOL isAELocked();
    MVOID lockAE();
    MVOID unlockAE();
    MBOOL isAWBLocked();
    MVOID lockAWB();
    MVOID unlockAWB();

    MINT32 queryExifInfoFromSensor();
    LIB3A_AE_ISO_SPEED_T mapAEISOSpeed(MUINT32 sensorISO);
    MUINT32 mapAERealISOSpeed(MUINT32 sensorRealISO);
    LIB3A_AWB_MODE_T mapAWBMode(MUINT32 sensorAWB);
    MUINT32 mapFNumber(MUINT32 sensorFNumber);
    MUINT32 mapCapExposureTime(MUINT32 sensorExposureTime);
    MUINT32 mapFlashLightTimeus(MUINT32 sensorFlashLightTime);
    MINT32 queryAEFlashlightInfoFromSensor();
    double AEFlashlightLog2(double x);
    double calcBV();    //BV---(40~50)
public:
    virtual void destroyInstance();

    virtual MINT32 getAFDone();
    virtual MINT32 getReadyForCapture() ;
    virtual MINT32 set3AParam(MINT32 a_i4Param0, MINT32 a_i4Param1, MINT32 a_i4Param2, MINT32 a_i4Param3) ;
    virtual MINT32 get3AParam(MINT32 a_i4Param0, MINT32 *a_pParam1);

    virtual MINT32 getDebugInfo(MVOID **a_p3ADebugInfo, MUINT32 *a_u4SizeInByte);
    virtual MINT32 setState(MINT32 a_i4aaaState);
    virtual MINT32 do3A(MINT32 a_i4aaaState, MUINT32 *a_pWaitVDNum);
    virtual MINT32 setCaptureParam(MUINT32 a_IsRawInDram);
    virtual MINT32 enableAE();
    virtual MINT32 disableAE();
    virtual MINT32 enableAWB();
    virtual MINT32 disableAWB();
    virtual MINT32 enableAF();
    virtual MINT32 disableAF();

    /////////////////////////////////////////////////////////////////////////
    //
    // isAutoWhiteBalanceLockSupported () -
    //! \brief [ICS] returns true if auto-white balance locking is supported
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MBOOL isAutoWhiteBalanceLockSupported() ;//{return TRUE;}

    /////////////////////////////////////////////////////////////////////////
    //
    // getAutoWhiteBalanceLock () -
    //! \brief [ICS] Gets the state of the auto-white balance lock: Returns true if auto-white balance is currently locked, and false otherwise
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MBOOL getAutoWhiteBalanceLock();

    /////////////////////////////////////////////////////////////////////////
    //
    // setAutoWhiteBalanceLock () -
    //! \brief [ICS] If set to true, the camera auto-white balance routine will immediately pause until the lock is set to false.
    //               If auto-white balance is already locked, setting this to true again has no effect (the driver will not recalculate white balance values).
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MVOID setAutoWhiteBalanceLock(MBOOL bToggle);

    /////////////////////////////////////////////////////////////////////////
    //
    // setFDInfo () -
    //! \brief set FD Info
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 setFDInfo(MUINT32 a_u4IsFDOn, MUINT32 a_u4Addr);

    /////////////////////////////////////////////////////////////////////////
    //
    // drawFocusRect () -
    //! \brief to get the best shot value
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 drawFocusRect(MUINT8 *a_pBuf, MUINT32 a_u4DispW, MUINT32 a_u4DispH, MUINT32 a_u4DispX, MUINT32 a_u4DispY, MUINT32 a_u4Rotate);

    /////////////////////////////////////////////////////////////////////////
    //
    // setEZoomInfo () -
    //! \brief set zoom info
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 setEZoomInfo(MUINT32 a_u4XOffset, MUINT32 a_u4YOffset, MUINT32 a_u4Width, MUINT32 a_u4Height);
    /////////////////////////////////////////////////////////////////////////
    //
    // init () -
    //! \brief init 3A hal
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 init(MINT32 a_i4SensorType);

    /////////////////////////////////////////////////////////////////////////
    //
    // setAFMoveSpotPos () -
    //! \brief set movespot info to m_touch_zone and m_zone
    //
    /////////////////////////////////////////////////////////////////////////
    virtual void setAFMoveSpotPos(MUINT32 a_u4Xoffset,MUINT32 a_u4Yoffset,MUINT32 a_u4Width,MUINT32 a_u4Height,MUINT32 a_u4OffsetW,MUINT32 a_u4OffsetH,MUINT8 a_uOri);

    /////////////////////////////////////////////////////////////////////////
    //
    // isAFFinish () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 isAFFinish();

    /////////////////////////////////////////////////////////////////////////
    //
    // resetHalfPushState () -
    //! \brief reset half button push state
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 resetHalfPushState() ;

    /////////////////////////////////////////////////////////////////////////
    //
    // isFocused () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MBOOL   isFocused();

    /////////////////////////////////////////////////////////////////////////
    //
    // resetFocus () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual void    resetFocus();

    /////////////////////////////////////////////////////////////////////////
    //
    // setFocusAreas () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual void    setFocusAreas(MINT32 a_i4Cnt, AREA_T *a_psFocusArea);

    /////////////////////////////////////////////////////////////////////////
    //
    // getFocusAreas () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual void    getFocusAreas(MINT32 &a_i4Cnt, AREA_T **a_psFocusArea);

    /////////////////////////////////////////////////////////////////////////
    //
    // getMaxNumFocusAreas () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32  getMaxNumFocusAreas();

   /////////////////////////////////////////////////////////////////////////
   //
   // getAFWinResult () -
   //! \brief get AF window result
   //
   /////////////////////////////////////////////////////////////////////////
   virtual MINT32 getAFWinResult(MINT32 *a_pBuf, MINT32 *a_pWinW, MINT32 *a_pWinH);

   //====================AE===begin=========================//
   /////////////////////////////////////////////////////////////////////////
   //
   // setAutoExposureLock() -
   //! \brief enable/disable AE function
   //
   /////////////////////////////////////////////////////////////////////////
   virtual MVOID setAutoExposureLock(MBOOL bLockAE);

   /////////////////////////////////////////////////////////////////////////
   //
   // getAutoExposureLock() -
   //! \brief get AE lock state
   //
   /////////////////////////////////////////////////////////////////////////
   virtual MBOOL getAutoExposureLock();

   /////////////////////////////////////////////////////////////////////////
   //
   // isAutoExposureLockSupported() -
   //! \brief AE support lock or not
   //
   /////////////////////////////////////////////////////////////////////////
   virtual MBOOL isAutoExposureLockSupported() ;//{return TRUE;}

   /////////////////////////////////////////////////////////////////////////
   //
   // getMaxNumMeteringAreas() -
   //! \brief get AE support max number of metering area
   //
   /////////////////////////////////////////////////////////////////////////
   virtual MINT32 getMaxNumMeteringAreas();

   /////////////////////////////////////////////////////////////////////////
   //
   // getMeteringAreas() -
   //! \brief get AE metering area information
   //
   /////////////////////////////////////////////////////////////////////////
   virtual MVOID getMeteringAreas(MINT32 &a_i4Cnt, AREA_T **a_psAEArea);

   /////////////////////////////////////////////////////////////////////////
   //
   // setMeteringAreas() -
   //! \brief set AE metering area information
   //
   /////////////////////////////////////////////////////////////////////////
   virtual MVOID setMeteringAreas(MINT32 a_i4Cnt, AREA_T const *a_psAEArea);

   //====================AE=====end=========================//

    /////////////////////////////////////////////////////////////////////////
    //
    // getASDInfo () -
    //! \brief get ASD info
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 getASDInfo(AAA_ASD_INFO_T &a_ASDInfo);

    /////////////////////////////////////////////////////////////////////////
    //
    // CCTOPFlashControl () -
    //! \brief CCTOP: flash control
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 CCTOPFlashControl(MVOID *a_pflashCtrl);

    virtual void setFlashActive(MBOOL a_FlashActive);
    virtual MBOOL getFlashActive(void);

    /////////////////////////////////////////////////////////////////////////
    //
    // get3AEXIFInfo () -
    //! \brief get exif info
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 get3AEXIFInfo(MVOID *a_p3AEXIFInfo);

    virtual MVOID setZSDMode(MBOOL ZSDFlag);

    virtual void FocusThread(MINT32 enable);
    virtual MVOID setStrobeOn(MBOOL bEnable);
    virtual MBOOL isStrobeOn();
    MINT32 setEShutterParam(MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain);

};

#endif

