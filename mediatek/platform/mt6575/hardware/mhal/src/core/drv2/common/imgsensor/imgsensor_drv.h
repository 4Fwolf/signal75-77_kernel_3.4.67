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
#ifndef _IMGSENSOR_DRV_H
#define _IMGSENSOR_DRV_H

#include "sensor_drv.h"
#include "camera_custom_sensor.h"
#include "kd_camera_feature.h"
#include <utils/threads.h>
using namespace android;

/*******************************************************************************
*
********************************************************************************/
typedef unsigned short  MUINT16;
typedef unsigned char   MUINT8;

#define YUV_TUNING_SUPPORT 


/*******************************************************************************
*
********************************************************************************/
class ImgSensorDrv : public SensorDrv {
public:
    static SensorDrv* getInstance();

private:
    ImgSensorDrv();
    virtual ~ImgSensorDrv();

public:
    virtual void destroyInstance();

public:

    virtual MINT32 init(MINT32 sensorIdx);
    virtual MINT32 uninit();
    
    virtual MINT32 open();
    virtual MINT32 close();
    
    virtual MINT32 setScenario(ACDK_SCENARIO_ID_ENUM sId);

    virtual MINT32 start();
    virtual MINT32 stop();
	virtual MINT32 waitSensorEventDone( MUINT32 EventType, MUINT32 Timeout);

    virtual MINT32 getInfo(ACDK_SCENARIO_ID_ENUM ScenarioId,ACDK_SENSOR_INFO_STRUCT *pSensorInfo,ACDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
    virtual MINT32 getResolution(ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);

    virtual MINT32 sendCommand(MUINT32 cmd, MUINT32 *parg1 = NULL, MUINT32 *parg2 = NULL, MUINT32 *parg3 = NULL);

    virtual MINT32 setFoundDrvsActive(MUINT32 socketIdxes);
    virtual MUINT32 getMainSensorID() const { return m_mainSensorId; }
    virtual MUINT32 getMain2SensorID() const { return m_main2SensorId; }
    virtual MUINT32 getSubSensorID() const { return m_subSensorId; }
    virtual IMAGE_SENSOR_TYPE getCurrentSensorType(); 
    virtual NSFeature::SensorInfoBase*  getMainSensorInfo() const { return  m_pMainSensorInfo; }
    virtual NSFeature::SensorInfoBase*  getSubSensorInfo()  const { return  m_pSubSensorInfo;  }

private:
    virtual MINT32 impSearchSensor(pfExIdChk pExIdChkCbf);
    MINT32 featureControl(ACDK_SENSOR_FEATURE_ENUM FeatureId,  UINT8 *pFeaturePara,MUINT32 *pFeatureParaLen);    
    MINT32 getSensorDelayFrameCnt(MUINT32 const mode); 
    //->
    IMGSENSOR_SOCKET_POSITION_ENUM getSocketPosition(CAMERA_DUAL_CAMERA_SENSOR_ENUM socket);
#if 0 //defined (ATVCHIP_MTK_ENABLE)
    MINT32 atvWorkaroundOn(); 
    MINT32 atvWorkaroundOff(); 
#endif 

#if defined (YUV_TUNING_SUPPORT)
    void customerInit(void);
    char* getHexToken(char *inStr, MUINT32 *outVal);
#endif 

private:
    int     m_fdSensor;
    //int     m_userCnt;     

    MUINT32  m_LineTimeInus;

    MUINT32     m_mainSensorId;
    MUINT32     m_main2SensorId;
    MUINT32     m_subSensorId;
    
    enum { BAD_SENSOR_INDEX = 0xFF };
    UINT8   m_mainSensorIdx; 
    UINT8   m_main2SensorIdx; 
    UINT8   m_subSensorIdx; 

    SENSOR_DRIVER_LIST_T m_mainSensorDrv;
    SENSOR_DRIVER_LIST_T m_main2SensorDrv;
    SENSOR_DRIVER_LIST_T m_subSensorDrv;

    NSFeature::SensorInfoBase*  m_pMainSensorInfo;
    NSFeature::SensorInfoBase*  m_pSubSensorInfo;

    MSDK_SENSOR_INIT_FUNCTION_STRUCT*   m_pstSensorInitFunc;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT  m_SenosrResInfo;
    //pthread_mutex_t                     m_sensorMutex;
    volatile int mUsers;
    mutable Mutex mLock;    

}; 

/*******************************************************************************
*
********************************************************************************/

#endif // _IMGSENSOR_DRV_H

