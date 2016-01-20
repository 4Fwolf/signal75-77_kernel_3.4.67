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

#define LOG_TAG "ImgSensorDrv"

//! This should depend on MATV complier option 
//! Just for workaround 
#if 0 // def ATVCHIP_MTK_ENABLE 
extern "C" {
#include "kal_release.h"
#include "matvctrl.h"
}

#include "ATVCtrl.h"
#include "ATVCtrlService.h"
#include "RefBase.h"
#include "threads.h"
#include "ATVCtrlService.h"
#include <binder/IServiceManager.h>
using namespace android;
#endif 
//! ===End MATV work around 

#include <utils/Errors.h>
#include <cutils/xlog.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
//
#include <camera_custom_if.h>
#include "imgsensor_drv.h"
#include "kd_imgsensor.h"
#include "CameraProfile.h"


/*******************************************************************************
*
********************************************************************************/
#define DEBUG_SENSOR_DRV
#ifdef DEBUG_SENSOR_DRV
#define SENSOR_DRV_DBG(fmt, arg...) XLOGD(fmt, ##arg)
#define SENSOR_DRV_ERR(fmt, arg...)  XLOGE("Err: %5d:, "fmt, __LINE__, ##arg)
#else
#define SENSOR_DRV_DBG(a,...)
#define SENSOR_DRV_ERR(a,...)
#endif

#if 0 //def ATVCHIP_MTK_ENABLE  
BOOL g_bATVWorkaroundDoShutDown = true;
int psFlag = 0;

int is_factory_boot(void)
{
    int fd;
    size_t s;
    char boot_mode;
    fd = open("/sys/class/BOOT/BOOT/boot/boot_mode", O_RDONLY);
    if (fd < 0) {        
        SENSOR_DRV_DBG("fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_mode");        
        return 0;    
    }    
    s = read(fd, (void *)&boot_mode, sizeof(boot_mode)); 
    close(fd);    
    if (s <= 0 || (boot_mode != '4' && boot_mode != '1')) // '1' for meta mode
        return 0;    
    SENSOR_DRV_DBG("Factory Mode Booting.....\n");    
    return 1;
}
extern int matvdrv_exist();
#endif

/*******************************************************************************
*
********************************************************************************/
SensorDrv*
ImgSensorDrv::
getInstance()
{
    SENSOR_DRV_DBG("[ImgSensorDrv] getInstance \n");
    static ImgSensorDrv singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void   
ImgSensorDrv::
destroyInstance() 
{
}

/*******************************************************************************
*
********************************************************************************/
ImgSensorDrv::
ImgSensorDrv()
    : SensorDrv()
    , m_fdSensor(-1)
    , m_LineTimeInus(31)
    , m_mainSensorId(SENSOR_DOES_NOT_EXIST)
    , m_main2SensorId(SENSOR_DOES_NOT_EXIST)
    , m_subSensorId(SENSOR_DOES_NOT_EXIST)
    , m_mainSensorIdx(BAD_SENSOR_INDEX)
    , m_main2SensorIdx(BAD_SENSOR_INDEX)
    , m_subSensorIdx(BAD_SENSOR_INDEX)
    , m_pMainSensorInfo(NULL)
    , m_pSubSensorInfo(NULL)
    , m_pstSensorInitFunc(NULL)
    , m_SenosrResInfo()
    , mUsers(0)

{
    memset(&m_SenosrResInfo, 0, sizeof(m_SenosrResInfo));
    //    
    memset((void*)&m_mainSensorDrv, 0xFF, sizeof(SENSOR_DRIVER_LIST_T));
    memset((void*)&m_main2SensorDrv, 0xFF, sizeof(SENSOR_DRIVER_LIST_T));
    memset((void*)&m_subSensorDrv, 0xFF, sizeof(SENSOR_DRIVER_LIST_T));
    m_mainSensorDrv.number = 0;
    m_main2SensorDrv.number = 0;
    m_subSensorDrv.number = 0;
}

/*******************************************************************************
*
********************************************************************************/
ImgSensorDrv::
~ImgSensorDrv()
{
    m_mainSensorIdx = BAD_SENSOR_INDEX; 
    m_main2SensorIdx = BAD_SENSOR_INDEX; 
    m_subSensorIdx = BAD_SENSOR_INDEX;
    SENSOR_DRV_DBG ("[~ImgSensorDrv]\n");
}

/*******************************************************************************
*
********************************************************************************/
#if !defined(MTK_NATIVE_3D_SUPPORT)
MINT32 
ImgSensorDrv::impSearchSensor(pfExIdChk pExIdChkCbf)
{
    // CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorEnum = DUAL_CAMERA_MAIN_SENSOR;
    MUINT32 SensorEnum = (MUINT32) DUAL_CAMERA_MAIN_SENSOR;
    MUINT32 i,id;
    BOOL SensorConnect=TRUE;
    UCHAR cBuf[64];
    MINT32 err = SENSOR_NO_ERROR;
    MINT32 err2 = SENSOR_NO_ERROR;
    ACDK_SENSOR_INFO_STRUCT SensorInfo;
    ACDK_SENSOR_CONFIG_STRUCT SensorConfigData;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT SensorResolution;
    MINT32 sensorDevs = SENSOR_NONE;

    //! If imp sensor search process already done before, 
    //! only need to return the sensorDevs, not need to 
    //! search again. 
    if (SENSOR_DOES_NOT_EXIST != m_mainSensorId) {
        //been processed.
        SENSOR_DRV_DBG("[impSearchSensor] Already processed \n"); 
        if (BAD_SENSOR_INDEX != m_mainSensorIdx) {
            sensorDevs |= SENSOR_MAIN;
        }
        if (BAD_SENSOR_INDEX != m_subSensorIdx) {
            sensorDevs |= SENSOR_SUB;
        }
        //
        #ifdef  ATVCHIP_MTK_ENABLE  
        //if (matvdrv_exist() != 0) {
        //<2014/07/02-kylechang, remove HAWK35 ATV
           // sensorDevs |= SENSOR_ATV;
        //>2014/07/02-kylechang
        //}
        #endif
        return sensorDevs; 
    }
    
    GetSensorInitFuncList(&m_pstSensorInitFunc);

    SENSOR_DRV_DBG("SENSOR search start \n");

    if (-1 != m_fdSensor) {
        ::close(m_fdSensor);
        m_fdSensor = -1;
    }
    sprintf(cBuf,"/dev/%s",CAMERA_HW_DEVNAME);
    m_fdSensor = ::open(cBuf, O_RDWR);
    if (m_fdSensor < 0) {
        SENSOR_DRV_ERR("Can't open sensor driver \n");    
        return sensorDevs;
    }

    // search MAIN/SUB sensor
    for (SensorEnum = DUAL_CAMERA_MAIN_SENSOR; SensorEnum <= DUAL_CAMERA_SUB_SENSOR; SensorEnum++)  {
        for (i = 0; i < MAX_NUM_OF_SUPPORT_SENSOR; i++) {
            if (m_pstSensorInitFunc[i].getCameraDefault == NULL) {
                SENSOR_DRV_DBG("m_pstSensorInitFunc[i].getCameraDefault is NULL: %d \n", i);                
                break;
            }
            //
            if (i != m_mainSensorIdx) {
                //set sensor driver
                id = (SensorEnum << KDIMGSENSOR_DUAL_SHIFT) | i;
                SENSOR_DRV_DBG("set sensor driver id =%x\n", id); 
                err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_DRIVER,&id );
                if (err < 0) {
                    SENSOR_DRV_ERR("ERROR:KDCAMERAHWIOC_X_SET_DRIVER\n");
                }

                //check sensor ID
                //power on sensor
                //include matv workaround
                //DONOT shutdown in sensor searching state,
#if 0 //def ATVCHIP_MTK_ENABLE  
                g_bATVWorkaroundDoShutDown = false;
                atvWorkaroundOn(); 
#endif
                //err = open();
                err = ioctl(m_fdSensor, KDIMGSENSORIOC_T_CHECK_IS_ALIVE);
                if (err < 0) {
                    SENSOR_DRV_DBG("[impSearchSensor] Err-ctrlCode (%s) \n", strerror(errno));
                }
    
                //check extra ID , from EEPROM maybe
                //may need to keep power here
                if (NULL != pExIdChkCbf) {
                    err2 = pExIdChkCbf();
                    if (err2 < 0) {
                        SENSOR_DRV_ERR("Error:pExIdChkCbf() \n");
                    }
                }

                //power off sensor
                //include matv workaround
                close();
#if 0 //def  ATVCHIP_MTK_ENABLE  
                atvWorkaroundOff(); 
                g_bATVWorkaroundDoShutDown = true;
#endif
                if (err < 0 || err2 < 0) {
                    SENSOR_DRV_DBG("sensor ID mismatch\n");
                }
                else {
                    if (SensorEnum == DUAL_CAMERA_MAIN_SENSOR) {
                        m_mainSensorIdx = i;
                        m_mainSensorId = m_pstSensorInitFunc[m_mainSensorIdx].SensorId;
                        //  pointer to the function which returns the instance of sensor info (class SensorInfoBase).
                        NSFeature::SensorInfoBase* (*pfGetSensorInfoInstance)() = m_pstSensorInitFunc[i].pfGetSensorInfoInstance;
                        if  ( pfGetSensorInfoInstance )
                        {
                            m_pMainSensorInfo = pfGetSensorInfoInstance();
                        }
                        if  ( ! m_pMainSensorInfo )
                        {
                            SENSOR_DRV_DBG("m_pMainSensorInfo==NULL\n");
                        }
                        SENSOR_DRV_DBG("MAIN sensor found:%d,0x%x \n",i,id);
                        break;
                    }
                    else if (SensorEnum == DUAL_CAMERA_SUB_SENSOR) {
                        m_subSensorIdx = i;
                        m_subSensorId = m_pstSensorInitFunc[m_subSensorIdx].SensorId;
                        //  pointer to the function which returns the instance of sensor info (class SensorInfoBase).
                        NSFeature::SensorInfoBase* (*pfGetSensorInfoInstance)() = m_pstSensorInitFunc[i].pfGetSensorInfoInstance;
                        if  ( pfGetSensorInfoInstance )
                        {
                            m_pSubSensorInfo = pfGetSensorInfoInstance();
                        }
                        if  ( ! m_pSubSensorInfo )
                        {
                            SENSOR_DRV_DBG("m_pSubSensorInfo==NULL\n");
                        }
                        SENSOR_DRV_DBG("SUB sensor found:%d,0x%x \n",i,id);
                        break;
                    }
                }
            }
        }
    }
    //close system call may be off sensor power. check first!!!
    ::close(m_fdSensor);
    m_fdSensor = -1;
    //
    if (BAD_SENSOR_INDEX != m_mainSensorIdx) {
        sensorDevs |= SENSOR_MAIN;
    }
    if (BAD_SENSOR_INDEX != m_subSensorIdx) {
        sensorDevs |= SENSOR_SUB;
    }
    //
    #ifdef  ATVCHIP_MTK_ENABLE  
    //if (matvdrv_exist() !=0) {
    //<2014/07/02-kylechang, remove HAWK35  ATV
       // sensorDevs |= SENSOR_ATV;
    //>2014/07/02-kylechang
    //}
    #endif
    //    
    if (sensorDevs == SENSOR_NONE) {
        SENSOR_DRV_ERR( "Error No sensor found!! \n");
    }
    //
    SENSOR_DRV_DBG("SENSOR search end: 0x%x \n", sensorDevs);
    
    return sensorDevs;
}//

#else //MTK_NATIVE_3D_SUPPORT

MINT32 
ImgSensorDrv::impSearchSensor(pfExIdChk pExIdChkCbf)
{
    // CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorEnum = DUAL_CAMERA_MAIN_SENSOR;
    MUINT32 SensorEnum = (MUINT32) DUAL_CAMERA_MAIN_SENSOR;
    MUINT32 i,id[KDIMGSENSOR_MAX_INVOKE_DRIVERS] = {0,0};
    BOOL SensorConnect=TRUE;
    UCHAR cBuf[64];
    MINT32 err = SENSOR_NO_ERROR;
    MINT32 err2 = SENSOR_NO_ERROR;
    ACDK_SENSOR_INFO_STRUCT SensorInfo;
    ACDK_SENSOR_CONFIG_STRUCT SensorConfigData;
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT SensorResolution;
    MINT32 sensorDevs = SENSOR_NONE;
    IMAGE_SENSOR_TYPE sensorType = IMAGE_SENSOR_TYPE_UNKNOWN;
    IMGSENSOR_SOCKET_POSITION_ENUM socketPos = IMGSENSOR_SOCKET_POS_NONE;

    //! If imp sensor search process already done before, 
    //! only need to return the sensorDevs, not need to 
    //! search again. 
    if (SENSOR_DOES_NOT_EXIST != m_mainSensorId) {
        //been processed.
        SENSOR_DRV_DBG("[impSearchSensor] Already processed \n"); 
        if (BAD_SENSOR_INDEX != m_mainSensorIdx) {
            sensorDevs |= SENSOR_MAIN;
        }
        if (BAD_SENSOR_INDEX != m_main2SensorIdx) {
            sensorDevs |= SENSOR_MAIN_2;
        }
        if (BAD_SENSOR_INDEX != m_subSensorIdx) {
            sensorDevs |= SENSOR_SUB;
        }
        //
        #ifdef  ATVCHIP_MTK_ENABLE  
        //if (matvdrv_exist() != 0) {
            sensorDevs |= SENSOR_ATV;
        //}
        #endif
        return sensorDevs; 
    }
    
    GetSensorInitFuncList(&m_pstSensorInitFunc);

    SENSOR_DRV_DBG("SENSOR search start \n");

    if (-1 != m_fdSensor) {
        ::close(m_fdSensor);
        m_fdSensor = -1;
    }
    sprintf(cBuf,"/dev/%s",CAMERA_HW_DEVNAME);
    m_fdSensor = ::open(cBuf, O_RDWR);
    if (m_fdSensor < 0) {
        SENSOR_DRV_ERR("[impSearchSensor]: error opening %s: %s \n", cBuf, strerror(errno));    
        return sensorDevs;
    }

    // search main/main_2/sub 3 sockets
    for (SensorEnum = DUAL_CAMERA_MAIN_SENSOR; SensorEnum <= DUAL_CAMERA_MAIN_2_SENSOR; SensorEnum <<= 1)  {
        //skip atv case
        if ( 0x04 == SensorEnum ) continue;
        //
        for (i = 0; i < MAX_NUM_OF_SUPPORT_SENSOR; i++) {
            //end of driver list
            if (m_pstSensorInitFunc[i].getCameraDefault == NULL) {
                SENSOR_DRV_DBG("m_pstSensorInitFunc[i].getCameraDefault is NULL: %d \n", i);                
                break;
            }
                //set sensor driver
            id[KDIMGSENSOR_INVOKE_DRIVER_0] = (SensorEnum << KDIMGSENSOR_DUAL_SHIFT) | i;
            SENSOR_DRV_DBG("set sensor driver id =%x\n", id[KDIMGSENSOR_INVOKE_DRIVER_0]); 
            err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_DRIVER,&id[KDIMGSENSOR_INVOKE_DRIVER_0] );
                if (err < 0) {
                    SENSOR_DRV_ERR("ERROR:KDCAMERAHWIOC_X_SET_DRIVER\n");
                }
                //check sensor ID
                //power on sensor
                //include matv workaround
                //DONOT shutdown in sensor searching state,
#if 0 //def ATVCHIP_MTK_ENABLE  
                g_bATVWorkaroundDoShutDown = false;
                atvWorkaroundOn(); 
#endif
                //err = open();
                err = ioctl(m_fdSensor, KDIMGSENSORIOC_T_CHECK_IS_ALIVE);
                if (err < 0) {
                    SENSOR_DRV_DBG("[impSearchSensor] Err-ctrlCode (%s) \n", strerror(errno));
                }
            //
            sensorType = this->getCurrentSensorType();
            //
            socketPos = this->getSocketPosition((CAMERA_DUAL_CAMERA_SENSOR_ENUM)SensorEnum);
                //check extra ID , from EEPROM maybe
                //may need to keep power here
                if (NULL != pExIdChkCbf) {
                    err2 = pExIdChkCbf();
                    if (err2 < 0) {
                        SENSOR_DRV_ERR("Error:pExIdChkCbf() \n");
                    }
                }

                //power off sensor
                //include matv workaround
                close();
#if 0 //def  ATVCHIP_MTK_ENABLE  
                atvWorkaroundOff(); 
                g_bATVWorkaroundDoShutDown = true;
#endif
                if (err < 0 || err2 < 0) {
                    SENSOR_DRV_DBG("sensor ID mismatch\n");
                }
                else {
                    if (SensorEnum == DUAL_CAMERA_MAIN_SENSOR) {
                //m_mainSensorIdx = i;
                //m_mainSensorId = m_pstSensorInitFunc[m_mainSensorIdx].SensorId;
                m_mainSensorDrv.index[m_mainSensorDrv.number] = i;
                m_mainSensorDrv.type[m_mainSensorDrv.number] = sensorType;
                if ( IMAGE_SENSOR_TYPE_RAW == sensorType && BAD_SENSOR_INDEX == m_mainSensorDrv.firstRawIndex ) {
                    m_mainSensorDrv.firstRawIndex = i;
                }
                else if ( IMAGE_SENSOR_TYPE_YUV == sensorType && BAD_SENSOR_INDEX == m_mainSensorDrv.firstYuvIndex ) {
                    m_mainSensorDrv.firstYuvIndex = i;
                }
                m_mainSensorDrv.position = socketPos;
                m_mainSensorDrv.sensorID = m_pstSensorInitFunc[m_mainSensorDrv.index[m_mainSensorDrv.number]].SensorId;
                m_mainSensorDrv.number++;
                        //  pointer to the function which returns the instance of sensor info (class SensorInfoBase).
                        NSFeature::SensorInfoBase* (*pfGetSensorInfoInstance)() = m_pstSensorInitFunc[i].pfGetSensorInfoInstance;
                        if  ( pfGetSensorInfoInstance )
                        {
                            m_pMainSensorInfo = pfGetSensorInfoInstance();
                        }
                        if  ( ! m_pMainSensorInfo )
                        {
                            SENSOR_DRV_DBG("m_pMainSensorInfo==NULL\n");
                        }
                SENSOR_DRV_DBG("MAIN sensor found:[%d]/[0x%x]/[%d]/[%d] \n",i,id[KDIMGSENSOR_INVOKE_DRIVER_0],sensorType,socketPos);
                //break;
            }
            else if (SensorEnum == DUAL_CAMERA_MAIN_2_SENSOR) {
                //
                if ( IMAGE_SENSOR_TYPE_RAW == sensorType ) {
                    SENSOR_DRV_DBG("[WARNING]tg2 path not support RAW type \n");
                    continue;
                }
                m_main2SensorDrv.index[m_main2SensorDrv.number] = i;
                m_main2SensorDrv.type[m_main2SensorDrv.number] = sensorType;
                if ( IMAGE_SENSOR_TYPE_RAW == sensorType && BAD_SENSOR_INDEX == m_main2SensorDrv.firstRawIndex ) {
                    m_main2SensorDrv.firstRawIndex = i;
                }
                else if ( IMAGE_SENSOR_TYPE_YUV == sensorType && BAD_SENSOR_INDEX == m_main2SensorDrv.firstYuvIndex ) {
                    m_main2SensorDrv.firstYuvIndex = i;
                }
                m_main2SensorDrv.position = socketPos;
                m_main2SensorDrv.sensorID = m_pstSensorInitFunc[m_main2SensorDrv.index[m_main2SensorDrv.number]].SensorId;
                m_main2SensorDrv.number++;
                SENSOR_DRV_DBG("MAIN_2 sensor found:[%d]/[0x%x]/[%d]/[%d] \n",i,id[KDIMGSENSOR_INVOKE_DRIVER_0],sensorType,socketPos);
                    }
                    else if (SensorEnum == DUAL_CAMERA_SUB_SENSOR) {
                //m_subSensorIdx = i;
                //m_subSensorId = m_pstSensorInitFunc[m_subSensorIdx].SensorId;
                m_subSensorDrv.index[m_subSensorDrv.number] = i;
                m_subSensorDrv.type[m_subSensorDrv.number] = sensorType;
                if ( IMAGE_SENSOR_TYPE_RAW == sensorType && BAD_SENSOR_INDEX == m_subSensorDrv.firstRawIndex ) {
                    m_subSensorDrv.firstRawIndex = i;
                }
                else if ( IMAGE_SENSOR_TYPE_YUV == sensorType && BAD_SENSOR_INDEX == m_subSensorDrv.firstYuvIndex ) {
                    m_subSensorDrv.firstYuvIndex = i;
                }
                m_subSensorDrv.position = socketPos;
                m_subSensorDrv.sensorID = m_pstSensorInitFunc[m_subSensorDrv.index[m_subSensorDrv.number]].SensorId;
                m_subSensorDrv.number++;
                        //  pointer to the function which returns the instance of sensor info (class SensorInfoBase).
                        NSFeature::SensorInfoBase* (*pfGetSensorInfoInstance)() = m_pstSensorInitFunc[i].pfGetSensorInfoInstance;
                        if  ( pfGetSensorInfoInstance )
                        {
                            m_pSubSensorInfo = pfGetSensorInfoInstance();
                        }
                        if  ( ! m_pSubSensorInfo )
                        {
                            SENSOR_DRV_DBG("m_pSubSensorInfo==NULL\n");
                        }
                SENSOR_DRV_DBG("SUB sensor found:[%d]/[0x%x]/[%d]/[%d] \n",i,id[KDIMGSENSOR_INVOKE_DRIVER_0],sensorType,socketPos);
                //break;
            }
        }//
        
        }
    }
    //close system call may be off sensor power. check first!!!
    ::close(m_fdSensor);
    m_fdSensor = -1;
    //
    if (BAD_SENSOR_INDEX != m_mainSensorDrv.index[0]) {
        m_mainSensorId = m_mainSensorDrv.sensorID;
        //init to choose first
        m_mainSensorIdx = m_mainSensorDrv.index[0];
        sensorDevs |= SENSOR_MAIN;
    }
    if (BAD_SENSOR_INDEX != m_main2SensorDrv.index[0]) {
        m_main2SensorId = m_main2SensorDrv.sensorID;
        //init to choose first
        m_main2SensorIdx = m_main2SensorDrv.index[0];
        sensorDevs |= SENSOR_MAIN_2;
    }
    if (BAD_SENSOR_INDEX != m_subSensorDrv.index[0]) {
        m_subSensorId = m_subSensorDrv.sensorID;
        //init to choose first
        m_subSensorIdx = m_subSensorDrv.index[0];
        sensorDevs |= SENSOR_SUB;
    }
    //
    #ifdef  ATVCHIP_MTK_ENABLE  
    //if (matvdrv_exist() !=0) {
        sensorDevs |= SENSOR_ATV;
    //}
    #endif
    //    
    if (sensorDevs == SENSOR_NONE) {
        SENSOR_DRV_ERR( "Error No sensor found!! \n");
    }
    //
    SENSOR_DRV_DBG("SENSOR search end: 0x%x /[0x%x][%d]/[0x%x][%d]/[0x%x][%d]\n", sensorDevs,
    m_mainSensorId,m_mainSensorIdx,m_main2SensorId,m_main2SensorIdx,m_subSensorId,m_subSensorIdx);
    
    return sensorDevs;
}//

#endif
/*******************************************************************************
*
********************************************************************************/
#define N2D_PRIORITY_DRIVER Yuv
#define N3D_PRIORITY_DRIVER Yuv
#define _SELECT_PRIORITY_DRIVER_(a,b)    do { if ( m_##a##SensorDrv.number > 1 && BAD_SENSOR_INDEX != m_##a##SensorDrv.first##b##Index ) { \
                                        m_##a##SensorIdx = m_##a##SensorDrv.first##b##Index; }}while(0)
#define SELECT_PRIORITY_DRIVER(a,b) _SELECT_PRIORITY_DRIVER_(a,b)

#define IMGSNESOR_FILL_SET_DRIVER_INFO(a) do{ \
        if ( DUAL_CAMERA_MAIN_SENSOR & a ) {    \
            sensorDrvInfo[KDIMGSENSOR_INVOKE_DRIVER_0] = \
                    (DUAL_CAMERA_MAIN_SENSOR << KDIMGSENSOR_DUAL_SHIFT) | m_mainSensorIdx; \
        }   \
        if ( DUAL_CAMERA_MAIN_2_SENSOR & a ) {  \
            sensorDrvInfo[KDIMGSENSOR_INVOKE_DRIVER_1] = \
                    (DUAL_CAMERA_MAIN_2_SENSOR << KDIMGSENSOR_DUAL_SHIFT) | m_main2SensorIdx;   \
        }   \
        if ( DUAL_CAMERA_SUB_SENSOR & a ) { \
            sensorDrvInfo[KDIMGSENSOR_INVOKE_DRIVER_0] = \
                    (DUAL_CAMERA_SUB_SENSOR << KDIMGSENSOR_DUAL_SHIFT) | m_subSensorIdx;    \
        }\
    }while(0);

#if !defined(MTK_NATIVE_3D_SUPPORT)
MINT32
ImgSensorDrv::init(
    MINT32 sensorIdx
)
{
    UCHAR cBuf[64];
    MINT32 ret = SENSOR_NO_ERROR;

    MUINT16 pFeaturePara16[2];
    MUINT32 FeaturePara32;
    MUINT32 FeatureParaLen;

    //g_currSensor = sensorIdx;

    if (DUAL_CAMERA_MAIN_SENSOR == sensorIdx) {
        sensorIdx = (sensorIdx << KDIMGSENSOR_DUAL_SHIFT) | m_mainSensorIdx;
    }
    else if (DUAL_CAMERA_SUB_SENSOR == sensorIdx) {
        sensorIdx = (sensorIdx << KDIMGSENSOR_DUAL_SHIFT) | m_subSensorIdx;
    }
    else {
        SENSOR_DRV_ERR("[init]: error sensorIdx \n");
        return SENSOR_INVALID_SENSOR;
    }
    SENSOR_DRV_DBG("[init] mUsers = %d\n", mUsers); 

    //pthread_mutex_lock(&m_sensorMutex);        
    Mutex::Autolock lock(mLock);
    if (mUsers == 0) {
        if (m_fdSensor == -1) {
    //        sprintf(cBuf,"/dev/%s",pstSensorInitFunc[sensorIdx].drvname);
            sprintf(cBuf,"/dev/%s",CAMERA_HW_DEVNAME);
            m_fdSensor = ::open(cBuf, O_RDWR);
            if (m_fdSensor < 0) {
                SENSOR_DRV_ERR("[init]: error opening %s: %s \n", cBuf, strerror(errno));                
                return SENSOR_INVALID_DRIVER;
            }
            //
        }
    }

    //set sensor driver
    SENSOR_DRV_DBG("[init]: %x\n",sensorIdx);
    if (0xFF == (sensorIdx&KDIMGSENSOR_DUAL_MASK_LSB)) {
        SENSOR_DRV_ERR("[init]: Error sensor Idx:0xFF,check MAIN/SUB sensor config!! \n");        
        return SENSOR_INVALID_SENSOR; 
    }
    ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_DRIVER,&sensorIdx);
    if (ret < 0) {
       SENSOR_DRV_ERR("[init]: ERROR:KDCAMERAHWIOC_X_SET_DRIVER\n");
    }

    android_atomic_inc(&mUsers); 

    //init. resolution
#if 1 
    ret = getResolution(&m_SenosrResInfo);
    if (ret < 0) {
        SENSOR_DRV_ERR("[init]: Get Resolution error\n");
        return SENSOR_UNKNOWN_ERROR;
    }
#endif 

    //calculater g_LineTimeInus for exposure time convert.
    FeatureParaLen = sizeof(MUINT32);
    ret = featureControl(SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
    if (ret < 0) {
       SENSOR_DRV_ERR("[init]:  SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error\n");
       return SENSOR_UNKNOWN_ERROR;
    }    

    FeatureParaLen = sizeof(pFeaturePara16);
    ret = featureControl(SENSOR_FEATURE_GET_PERIOD,  (MUINT8*)pFeaturePara16,(MUINT32*)&FeatureParaLen);
    if (ret < 0) {
        SENSOR_DRV_ERR("[init]: SENSOR_FEATURE_GET_PERIOD error\n");
        return SENSOR_UNKNOWN_ERROR;
    }

    if (FeaturePara32) {
        //in setting domain, use preview line time only
        //sensor drv will convert to capture line time when setting to capture mode.
        //m_LineTimeInus = (MUINT32)((pFeaturePara16[0]*1000000+(FeaturePara32>>1))/FeaturePara32);
        // To keep the clock and pixels value to improve the calculate precision.
        if(FeaturePara32 >= 1000) {
            m_LineTimeInus = (MUINT32)((pFeaturePara16[0]*1000000 + ((FeaturePara32/1000)-1))/(FeaturePara32/1000));   // 1000 base , 33657 mean 33.657 us
        } else {
            SENSOR_DRV_ERR("[init]: Sensor clock too slow = %d %d\n", FeaturePara32, pFeaturePara16[0]);        
        }
        SENSOR_DRV_DBG("[init] m_LineTimeInus = %d PixelClk = %d PixelInLine = %d\n", m_LineTimeInus, FeaturePara32, pFeaturePara16[0]); 
    }
    return SENSOR_NO_ERROR;
}
#else

#define N2D_PRIORITY_DRIVER Yuv
#define N3D_PRIORITY_DRIVER Yuv
#define _SELECT_PRIORITY_DRIVER_(a,b)    do { if ( m_##a##SensorDrv.number > 1 && BAD_SENSOR_INDEX != m_##a##SensorDrv.first##b##Index ) { \
                                        m_##a##SensorIdx = m_##a##SensorDrv.first##b##Index; }}while(0)
#define SELECT_PRIORITY_DRIVER(a,b) _SELECT_PRIORITY_DRIVER_(a,b)

#define IMGSNESOR_FILL_SET_DRIVER_INFO(a) do{ \
        if ( DUAL_CAMERA_MAIN_SENSOR & a ) {    \
            sensorDrvInfo[KDIMGSENSOR_INVOKE_DRIVER_0] = \
                    (DUAL_CAMERA_MAIN_SENSOR << KDIMGSENSOR_DUAL_SHIFT) | m_mainSensorIdx; \
        }   \
        if ( DUAL_CAMERA_MAIN_2_SENSOR & a ) {  \
            sensorDrvInfo[KDIMGSENSOR_INVOKE_DRIVER_1] = \
                    (DUAL_CAMERA_MAIN_2_SENSOR << KDIMGSENSOR_DUAL_SHIFT) | m_main2SensorIdx;   \
        }   \
        if ( DUAL_CAMERA_SUB_SENSOR & a ) { \
            sensorDrvInfo[KDIMGSENSOR_INVOKE_DRIVER_0] = \
                    (DUAL_CAMERA_SUB_SENSOR << KDIMGSENSOR_DUAL_SHIFT) | m_subSensorIdx;    \
        }\
    }while(0);

MINT32
ImgSensorDrv::init(
    MINT32 sensorIdx
)
{
    UCHAR cBuf[64];
    MINT32 ret = SENSOR_NO_ERROR;
    MUINT16 pFeaturePara16[2];
    MUINT32 FeaturePara32;
    MUINT32 FeatureParaLen;
    MUINT32 sensorDrvInfo[KDIMGSENSOR_MAX_INVOKE_DRIVERS] = {0,0};
    IMAGE_SENSOR_TYPE sensorType_prioriy = IMAGE_SENSOR_TYPE_UNKNOWN;

    //ONLY "main/main_2" can be ON simultaneously
    if ( ( 0 == sensorIdx ) ||
         ( 0x04 & sensorIdx ) ||
         ( ( DUAL_CAMERA_SUB_SENSOR & sensorIdx ) && ( DUAL_CAMERA_SUB_SENSOR ^ sensorIdx ) ) ) {
        SENSOR_DRV_ERR("invalid sensorIdx[0x%08x] \n",sensorIdx);
        return SENSOR_INVALID_PARA;
    }
    //select driver
    if ( ( DUAL_CAMERA_MAIN_SENSOR|DUAL_CAMERA_MAIN_2_SENSOR ) == sensorIdx ) {
        //N3D mode
        SELECT_PRIORITY_DRIVER(main,N3D_PRIORITY_DRIVER);
    }
    else {
        //2D mode
        SELECT_PRIORITY_DRIVER(main,N2D_PRIORITY_DRIVER);
        //SELECT_PRIORITY_DRIVER(main2,N2D_PRIORITY_DRIVER);
        SELECT_PRIORITY_DRIVER(sub,N2D_PRIORITY_DRIVER);
    }
    //
    IMGSNESOR_FILL_SET_DRIVER_INFO(sensorIdx);
    //
    SENSOR_DRV_DBG("[init] mUsers = %d\n", mUsers); 
    Mutex::Autolock lock(mLock);

    if (mUsers == 0) {
        if (m_fdSensor == -1) {
            sprintf(cBuf,"/dev/%s",CAMERA_HW_DEVNAME);
            m_fdSensor = ::open(cBuf, O_RDWR);
            if (m_fdSensor < 0) {
                SENSOR_DRV_ERR("[init]: error opening %s: %s \n", cBuf, strerror(errno));                
                return SENSOR_INVALID_DRIVER;
            }
        }
    }

    //set sensor driver
    ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_DRIVER,sensorDrvInfo);
    if (ret < 0) {
       SENSOR_DRV_ERR("[init]: ERROR:KDCAMERAHWIOC_X_SET_DRIVER\n");
    }

    android_atomic_inc(&mUsers); 

    //init. resolution
#if 1 
    ret = getResolution(&m_SenosrResInfo);
    if (ret < 0) {
        SENSOR_DRV_ERR("[init]: Get Resolution error\n");
        return SENSOR_UNKNOWN_ERROR;
    }
#endif 

    //calculater g_LineTimeInus for exposure time convert.
    FeatureParaLen = sizeof(MUINT32);
    ret = featureControl(SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
    if (ret < 0) {
       SENSOR_DRV_ERR("[init]:  SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error\n");
       return SENSOR_UNKNOWN_ERROR;
    }    

    FeatureParaLen = sizeof(pFeaturePara16);
    ret = featureControl(SENSOR_FEATURE_GET_PERIOD,  (MUINT8*)pFeaturePara16,(MUINT32*)&FeatureParaLen);
    if (ret < 0) {
        SENSOR_DRV_ERR("[init]: SENSOR_FEATURE_GET_PERIOD error\n");
        return SENSOR_UNKNOWN_ERROR;
    }

    if (FeaturePara32) {
        //in setting domain, use preview line time only
        //sensor drv will convert to capture line time when setting to capture mode.
        m_LineTimeInus = (MUINT32)((pFeaturePara16[0]*1000000+(FeaturePara32>>1))/FeaturePara32);
    }

    return SENSOR_NO_ERROR;
}

#endif
/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::uninit(
)
{
    //MHAL_LOG("[halSensorUninit] \n");
    MINT32 ret = SENSOR_NO_ERROR;

#if 0 
    ret = close();
    if (ret < 0)
    {
        SENSOR_DRV_ERR("[uninit] SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error\n");
        return MHAL_UNKNOWN_ERROR;
    }
#endif 

    //pthread_mutex_lock(&m_sensorMutex);        
    SENSOR_DRV_DBG("[uninit] mUsers = %d\n", mUsers);     

    Mutex::Autolock lock(mLock);
    //
    if (mUsers <= 0) {
        // No more users
        return SENSOR_NO_ERROR;
    }

    if (mUsers == 1) {
        if (m_fdSensor > 0) {
            ::close(m_fdSensor);
        }
        m_fdSensor = -1;            
    }
    android_atomic_dec(&mUsers); 
    //m_userCnt --; 
    //pthread_mutex_unlock(&m_sensorMutex);        


    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::setScenario(ACDK_SCENARIO_ID_ENUM sId)
{
    AutoCPTLog cptlog(Event_Sensor_setScenario);
    MINT32 ret = SENSOR_NO_ERROR;
    ACDK_SENSOR_EXPOSURE_WINDOW_STRUCT ImageWindow;
    ACDK_SENSOR_CONFIG_STRUCT SensorConfigData;
    SensorConfigData.SensorImageMirror = ACDK_SENSOR_IMAGE_NORMAL;

    MUINT16 pFeaturePara16[2];
    MUINT32 FeaturePara32;
    MUINT32 FeatureParaLen;
    
    switch(sId)
    {        
        case ACDK_SCENARIO_ID_CAMERA_PREVIEW:
        case ACDK_SCENARIO_ID_VIDEO_PREVIEW:
        case ACDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
        case ACDK_SCENARIO_ID_CAMERA_3D_PREVIEW:
            SensorConfigData.SensorOperationMode = ACDK_SENSOR_OPERATION_MODE_CAMERA_PREVIEW;
            ImageWindow.ImageTargetWidth = m_SenosrResInfo.SensorPreviewWidth;
            ImageWindow.ImageTargetHeight = m_SenosrResInfo.SensorPreviewHeight;
            break;
        case ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        case ACDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
        case ACDK_SCENARIO_ID_CAMERA_ZSD:
        case ACDK_SCENARIO_ID_CAMERA_3D_CAPTURE:
            SensorConfigData.EnableShutterTansfer = FALSE;
            ImageWindow.ImageTargetWidth = m_SenosrResInfo.SensorFullWidth;
            ImageWindow.ImageTargetHeight = m_SenosrResInfo.SensorFullHeight;
            break;
        default:
            SENSOR_DRV_ERR("[setScenario] error scenario id\n");
            return SENSOR_UNKNOWN_ERROR;
    }

    //set sensor preview/capture mode
    ACDK_SENSOR_CONTROL_STRUCT sensorCtrl;
    sensorCtrl.ScenarioId = sId;
    sensorCtrl.pImageWindow = &ImageWindow;
    sensorCtrl.pSensorConfigData = &SensorConfigData;

    ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_CONTROL , &sensorCtrl);
    if (ret < 0) {
        SENSOR_DRV_ERR("[setScenario]Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }

    if(sId != ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG) {
        //calculater g_LineTimeInus for exposure time convert.
        FeatureParaLen = sizeof(MUINT32);
        ret = featureControl(SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ,  (MUINT8*)&FeaturePara32,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
           SENSOR_DRV_ERR("[init]:  SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ error\n");
           return SENSOR_UNKNOWN_ERROR;
        }    
    
        FeatureParaLen = sizeof(pFeaturePara16);
        ret = featureControl(SENSOR_FEATURE_GET_PERIOD,  (MUINT8*)pFeaturePara16,(MUINT32*)&FeatureParaLen);
        if (ret < 0) {
            SENSOR_DRV_ERR("[setScenario]: SENSOR_FEATURE_GET_PERIOD error\n");
            return SENSOR_UNKNOWN_ERROR;
        }

        if (FeaturePara32) {
            //in setting domain, use preview line time only
            //sensor drv will convert to capture line time when setting to capture mode.
 //           m_LineTimeInus = (MUINT32)((pFeaturePara16[0]*1000000+(FeaturePara32>>1))/FeaturePara32);

            // To keep the clock and pixels value to improve the calculate precision.
            if(FeaturePara32 >= 1000) {
                m_LineTimeInus = (MUINT32)((pFeaturePara16[0]*1000000 + ((FeaturePara32/1000)-1))/(FeaturePara32/1000));   // 1000 base , 33657 mean 33.657 us
            } else {
                SENSOR_DRV_ERR("[setScenario]: Sensor clock too slow = %d %d\n", FeaturePara32, pFeaturePara16[0]);        
            }
            SENSOR_DRV_DBG("[setScenario] m_LineTimeInus = %d id = %d PixelClk = %d PixelInLine = %d\n", m_LineTimeInus, sId, FeaturePara32, pFeaturePara16[0]); 
        }
    }
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::start(
)
{
    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::stop(
)
{
    return SENSOR_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
ImgSensorDrv::getSensorDelayFrameCnt(
    MUINT32 const mode
)
{
    ACDK_SENSOR_INFO_STRUCT SensorInfo;
    ACDK_SENSOR_CONFIG_STRUCT SensorConfigData;
    ::memset(&SensorInfo, 0, sizeof(ACDK_SENSOR_INFO_STRUCT)); 
    ::memset(&SensorConfigData, 0, sizeof(ACDK_SENSOR_CONFIG_STRUCT));   
    SENSOR_DRV_DBG("[getSensorDelayFrameCnt] mode = %d\n", mode); 
    if (SENSOR_NO_ERROR != getInfo(ACDK_SCENARIO_ID_CAMERA_PREVIEW, &SensorInfo, &SensorConfigData)) {
       SENSOR_DRV_ERR("[searchSensor] Error:getInfo() \n");
       return 0;
    }    

    if ( 0 == mode) {
        return SensorInfo.PreviewDelayFrame; 
    }
    else if (1 == mode) {
        return SensorInfo.VideoDelayFrame; 
    }
    else if (2 == mode) {
        return SensorInfo.CaptureDelayFrame;
    }    
    else {
        return 0;
    }
}

/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::waitSensorEventDone(
    MUINT32 EventType,    
    MUINT32 Timeout
)
{
    MINT32 ret = 0;
    SENSOR_DRV_DBG("[ImgSensorDrv]waitSensorEventDone: EventType = %d, Timeout=%d\n",EventType,Timeout);
    switch (EventType) {
        case WAIT_SENSOR_SET_SHUTTER_GAIN_DONE:
            ret = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_SHUTTER_GAIN_WAIT_DONE, &Timeout);
            break;
        default :
            break;           
    }
    if(ret < 0)
    {
        SENSOR_DRV_ERR("waitSensorEventDone err, Event = %d", EventType);
    }
    return ret;    
}


/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::sendCommand(
        MUINT32 cmd, 
        MUINT32 *parg1, 
        MUINT32 *parg2,
        MUINT32 *parg3
)
{ 
    MINT32 err = SENSOR_NO_ERROR;

    ACDK_SENSOR_FEATURE_ENUM FeatureId = SENSOR_FEATURE_BEGIN;
    MUINT16 u2FeaturePara=0;
    MUINT8 *pFeaturePara = NULL; 
    MUINT32 u4FeaturePara[4];
    MUINT32 FeatureParaLen = 0;
    MUINT16 *pu2FeaturePara = NULL; 
    MUINT32 *pu4Feature = NULL; 
    MUINT32 *pu4FeaturePara = NULL;

    switch (cmd) {
    case CMD_SENSOR_SET_EXP_TIME:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER;
//        u2FeaturePara = (MUINT16)(((*parg1+(m_LineTimeInus>>1))/m_LineTimeInus)|0x01);
        u2FeaturePara = (MUINT16)((1000*(*parg1))/m_LineTimeInus);
        if(u2FeaturePara ==0) {   // avoid the line number to zero
              SENSOR_DRV_DBG("[CMD_SENSOR_SET_EXP_TIME] m_LineTime = %d %d\n", u2FeaturePara, m_LineTimeInus);  
       	u2FeaturePara = 1;        	
        }
        FeatureParaLen = sizeof(MUINT16);
        pFeaturePara = (MUINT8*)&u2FeaturePara;
        break;
        
    case CMD_SENSOR_SET_EXP_LINE:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER;
        u2FeaturePara = (MUINT16)(*parg1);
        FeatureParaLen = sizeof(MUINT16);
        pFeaturePara = (MUINT8*)&u2FeaturePara;
        break;        
        
    case CMD_SENSOR_SET_GAIN:
        //MHAL_LOG("HAL_SENSOR_PARAM_SET_GAIN \n");
        FeatureId = SENSOR_FEATURE_SET_GAIN;
        u2FeaturePara=(MUINT16)(*parg1 >>= 4); //from 10b to 6b base
        FeatureParaLen = sizeof(MUINT16);
        pFeaturePara =  (MUINT8*)&u2FeaturePara;            
        break;
    case CMD_SENSOR_SET_ESHUTTER_GAIN:
        FeatureId = SENSOR_FEATURE_SET_ESHUTTER_GAIN;
        u4FeaturePara[0] = *parg1; // exposure time (us)
        u4FeaturePara[0] = ((1000 * u4FeaturePara[0] )/m_LineTimeInus);
        if(u4FeaturePara[0]  ==0) {   // avoid the line number to zero
              SENSOR_DRV_DBG("[CMD_SENSOR_SET_ESHUTTER_GAIN] m_LineTime = %d %d\n", u4FeaturePara[0] , m_LineTimeInus);  
       	u4FeaturePara[0]  = 1;        	
        }
        u4FeaturePara[2] = (u4FeaturePara[0] ) | (((*(parg1+1))>>4) << 16); // Sensor gain from 10b to 6b base
        u4FeaturePara[0] = 0; // RAW Gain R, Gr
        u4FeaturePara[1] = 0;  // RAW Gain Gb, B
        u4FeaturePara[3] = 0;  // Delay frame cnt
        SENSOR_DRV_DBG("CMD_SENSOR_SET_ESHUTTER_GAIN: Exp=%d, SensorGain=%d\n", u4FeaturePara[2]&0x0000FFFF, u4FeaturePara[2]>>16);
        FeatureParaLen = sizeof(MUINT32) * 4;
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];    	
    	 break;
    
    case CMD_SENSOR_SET_FRAME_RATE:
       FeatureId = SENSOR_FEATURE_SET_VIDEO_MODE;
       u2FeaturePara = (MUINT16)*parg1; 
       FeatureParaLen = sizeof(MUINT16);
       pFeaturePara =  (MUINT8*)&u2FeaturePara;            
       break;
       
    case CMD_SENSOR_SET_YUV_CMD:        
        FeatureId = SENSOR_FEATURE_SET_YUV_CMD; 
        u4FeaturePara[0] = *parg1; 
        u4FeaturePara[1] = *parg2;         
        FeatureParaLen = sizeof(MUINT32) * 2; 
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];        
        break;

    case CMD_SENSOR_SET_VIDEO_MODE:
        FeatureId = SENSOR_FEATURE_SET_VIDEO_MODE;
        u2FeaturePara = (MUINT16)*parg1; 
        FeatureParaLen = sizeof(MUINT16);
        pFeaturePara =  (MUINT8*)&u2FeaturePara;            
        break; 

    case CMD_SENSOR_SET_SENSOR_SYNC:
        FeatureId = SENSOR_FEATURE_SET_SENSOR_SYNC;
        u4FeaturePara[0] = *parg1; // RAW Gain R, Gr
        u4FeaturePara[1] = *(parg1+1);  // RAW Gain Gb, B
        u4FeaturePara[2] = *(parg1+2);  // Exposure time
//        u4FeaturePara[2] = (((u4FeaturePara[2] +(m_LineTimeInus>>1))/m_LineTimeInus)|0x01) | (((*(parg1+3))>>4) << 16); // Sensor gain from 10b to 6b base
        u4FeaturePara[2] = ((1000 * u4FeaturePara[2] )/m_LineTimeInus);
        if(u4FeaturePara[2]  ==0) {   // avoid the line number to zero
              SENSOR_DRV_DBG("[CMD_SENSOR_SET_SENSOR_SYNC] m_LineTime = %d %d\n", u4FeaturePara[2] , m_LineTimeInus);  
       	u4FeaturePara[2]  = 1;        	
        }
        u4FeaturePara[2] = (u4FeaturePara[2] ) | (((*(parg1+3))>>4) << 16); // Sensor gain from 10b to 6b base
        u4FeaturePara[3] = *(parg1+4);  // Delay frame cnt
        //SENSOR_DRV_DBG("HAL_SENSOR_PARAM_SET_SENSOR_SYNC:RAWGain=%d %d %d %d Exp=%d, SensorGain=%d Delay frame=0x%08x\n", u4FeaturePara[0]>>16, u4FeaturePara[0]&0x0000FFFF,
        //    u4FeaturePara[1]>>16, u4FeaturePara[1]&0x0000FFFF, u4FeaturePara[2]&0x0000FFFF, u4FeaturePara[2]>>16, u4FeaturePara[3]);
        FeatureParaLen = sizeof(MUINT32) * 4;
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        break;

    case CMD_SENSOR_GET_PIXEL_CLOCK_FREQ:
        FeatureId = SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ;
        pu4FeaturePara = (MUINT32*)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)pu4FeaturePara;    	
    	 break;
    case CMD_SENSOR_GET_PIXEL_PERIOD:
        FeatureId = SENSOR_FEATURE_GET_PERIOD;
        pu4FeaturePara = (MUINT32 *)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)pu4FeaturePara;    	    	
    	 break;
    case CMD_SENSOR_SET_FLICKER_FRAME_RATE:
        FeatureId = SENSOR_FEATURE_SET_AUTO_FLICKER_MODE;
        u4FeaturePara[0] = *parg1; 
        FeatureParaLen = sizeof(MUINT16)*2;
        pFeaturePara =  (MUINT8*)&u4FeaturePara[0];
        SENSOR_DRV_DBG("Flicker frame rate :0x%04x 0x%04x 0x%0x 0x%x\n", *parg1, u4FeaturePara[0], *pFeaturePara, *(pFeaturePara+1));
        break;

    case CMD_SENSOR_CCT_FEATURE_CONTROL:
        FeatureId = (ACDK_SENSOR_FEATURE_ENUM)*parg1;
        pFeaturePara = (MUINT8*)parg2;
        FeatureParaLen = (MUINT32)*parg3;
        break;

    case CMD_SENSOR_GET_INDATA_FORMAT:
        // Bit 0~7 as data input
        switch  ((int)parg2)
        {
        using namespace NSCamCustom;
        case SENSOR_MAIN:
            *parg1 = getSensorInputDataBitOrder(eDevId_ImgSensor0);
            err = 0;
            break;
        case SENSOR_SUB:
            *parg1 = getSensorInputDataBitOrder(eDevId_ImgSensor1);
            err = 0;
            break;
        default:
            SENSOR_DRV_ERR("[sendCommand]<CMD_SENSOR_GET_INDATA_FORMAT> - bad sensor id(%x)", (int)parg2);
            *parg1 = 0;
            err = -1;
            break;
        }
        return  err;
        break;

    case CMD_SENSOR_GET_PAD_PCLK_INV:
        switch  ((int)parg2)
        {
        using namespace NSCamCustom;
        case SENSOR_MAIN:
            *parg1 = getSensorPadPclkInv(eDevId_ImgSensor0);
            err = 0;
            break;
        case SENSOR_SUB:
            *parg1 = getSensorPadPclkInv(eDevId_ImgSensor1);
            err = 0;
            break;
        default:
            SENSOR_DRV_ERR("[sendCommand]<CMD_SENSOR_GET_PAD_PCLK_INV> - bad sensor id(%x)", (int)parg2);
            *parg1 = 0;
            err = -1;
            break;
        }
        return  err;
        break;

    case CMD_SENSOR_GET_SENSOR_VIEWANGLE:

       switch  ((int)parg3)
        {
            using namespace NSCamCustom;
            case SENSOR_MAIN:
                *parg1 = (getSensorViewAngle(eDevId_ImgSensor0) & 0xFFFF0000)>>16;
                *parg2 = getSensorViewAngle(eDevId_ImgSensor0) & 0xFFFF;
                err = 0;
                break;
            case SENSOR_SUB:
                *parg1 = (getSensorViewAngle(eDevId_ImgSensor1) & 0xFFFF0000)>>16;
                *parg2 = getSensorViewAngle(eDevId_ImgSensor1) & 0xFFFF;
                err = 0;
                break;
            default:
                SENSOR_DRV_ERR("[sendCommand]<CMD_SENSOR_GET_SENSOR_VIEWANGLE> - bad sensor id(%x)", (int)parg3);
                *parg1 = 0;
                err = -1;
                break;
        }      
        return err;
       break;         

    case CMD_SENSOR_CONSTANT_AF:
        FeatureId = SENSOR_FEATURE_CONSTANT_AF;
        break;

    case CMD_SENSOR_GET_AF_STATUS:
        FeatureId = SENSOR_FEATURE_GET_AF_STATUS;
        pu4FeaturePara = (MUINT32*)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)pu4FeaturePara;
        //SENSOR_DRV_DBG("CMD_SENSOR_GET_AF_STATUS,parg1=0x%x,FeatureParaLen=0x%x,pFeaturePara=0x%x\n",
        //parg1, FeatureParaLen, pFeaturePara);	
        break;

    case CMD_SENSOR_SINGLE_FOCUS_MODE:
        FeatureId = SENSOR_FEATURE_SINGLE_FOCUS_MODE;
        //SENSOR_DRV_DBG("CMD_SENSOR_SINGLE_FOCUS_MODE\n");
        break; 
  
    case CMD_SENSOR_CANCEL_AF:
        FeatureId = SENSOR_FEATURE_CANCEL_AF; 
        //SENSOR_DRV_DBG("CMD_SENSOR_CANCEL_AF\n");
        break;

    case CMD_SENSOR_SET_AF_WINDOW:
        FeatureId = SENSOR_FEATURE_SET_AF_WINDOW;
        u4FeaturePara[0] = (MUINT32)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        //SENSOR_DRV_DBG("zone_addr=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_SET_CALIBRATION_DATA:
        FeatureId = SENSOR_FEATURE_SET_CALIBRATION_DATA;
        pFeaturePara = (UINT8*)parg1;
        FeatureParaLen = sizeof(SET_SENSOR_CALIBRATION_DATA_STRUCT);
        break;

    case CMD_SENSOR_GET_DELAY_FRAME_CNT:
        {
            *parg2 = getSensorDelayFrameCnt(*parg1); 
            return err; 
        }
        break; 

    case CMD_SENSOR_GET_EV_AWB_REF:
        FeatureId = SENSOR_FEATURE_GET_EV_AWB_REF;
        u4FeaturePara[0] = (MUINT32)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        SENSOR_DRV_DBG("p_ref=0x%x\n", u4FeaturePara[0]);
        break;

    case CMD_SENSOR_GET_SHUTTER_GAIN_AWB_GAIN:
        FeatureId = SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN;
        u4FeaturePara[0] = (MUINT32)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        SENSOR_DRV_DBG("p_cur=0x%x\n", u4FeaturePara[0]);
        break;
        
    case CMD_SENSOR_GET_AF_MAX_NUM_FOCUS_AREAS:
        FeatureId = SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS;
        pu4FeaturePara = (MUINT32*)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)pu4FeaturePara;
        //SENSOR_DRV_DBG("CMD_SENSOR_GET_AF_MAX_NUM_FOCUS_AREAS,p_cur=0x%x\n", u4FeaturePara[0]);
        break;       

    case CMD_SENSOR_GET_AE_MAX_NUM_METERING_AREAS:
        FeatureId = SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS;
        pu4FeaturePara = (MUINT32*)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)pu4FeaturePara;
        //SENSOR_DRV_DBG("CMD_SENSOR_GET_AE_MAX_NUM_METERING_AREAS,p_cur=0x%x\n", u4FeaturePara[0]);
        break;   

    case CMD_SENSOR_SET_AE_WINDOW:
        FeatureId = SENSOR_FEATURE_SET_AE_WINDOW;
        u4FeaturePara[0] = (MUINT32)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        //SENSOR_DRV_DBG("AEzone_addr=0x%x\n", u4FeaturePara[0]);
        break;
        
    case CMD_SENSOR_GET_EXIF_INFO:
        FeatureId = SENSOR_FEATURE_GET_EXIF_INFO;
        u4FeaturePara[0] = (MUINT32)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        SENSOR_DRV_DBG("EXIF_addr=0x%x\n", u4FeaturePara[0]);
        break;
#ifdef MTK_CAMERA_YUV_AUTOFLASHLIGHT
    case CMD_SENSOR_GET_AE_FLASHLIGHT_INFO:
        FeatureId = SENSOR_FEATURE_GET_AE_FLASHLIGHT_INFO;
        u4FeaturePara[0] = (MUINT32)parg1;
        FeatureParaLen = sizeof(MUINT32);
        pFeaturePara = (MUINT8*)&u4FeaturePara[0];
        SENSOR_DRV_DBG("AE_FLASHLIGHT_INFO_addr=0x%x\n", u4FeaturePara[0]);
        break;        
#endif
    default:

#ifdef MTK_CAMERA_YUV_AUTOFLASHLIGHT		
		SENSOR_DRV_ERR("...YUV_AUTOFL... defined");
#else
		SENSOR_DRV_ERR("...YUV_AUTOFL... NOT define.");
#endif
	
		SENSOR_DRV_ERR("CMD_SENSOR_GET_AE_FLASHLIGHT_INFO=%d", CMD_SENSOR_GET_AE_FLASHLIGHT_INFO);
        SENSOR_DRV_ERR("[sendCommand] cmd=%d Undefined", cmd);
        return SENSOR_UNKNOWN_ERROR; 
    }

    if (m_fdSensor == -1) {
        SENSOR_DRV_ERR("[sendCommand]m_fdSensor fail");
        return SENSOR_UNKNOWN_ERROR;
    }

    err= featureControl(FeatureId,  (MUINT8*)pFeaturePara,(MUINT32*)&FeatureParaLen);
    if (err < 0) {
        SENSOR_DRV_ERR("[sendCommand] Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
#if defined(YUV_TUNING_SUPPORT)
/////////////////////////////////////////////////////////////////////////
//
//  getHexToken () - 
//! @brief skip the space of the input string 
//! @param ppInStr: The point of the input string 
/////////////////////////////////////////////////////////////////////////
char* 
ImgSensorDrv::getHexToken(char *inStr, MUINT32 *outVal)
{
    MUINT32 thisVal, tVal;
    char x;
    char *thisStr = inStr;

    thisVal = 0;

    // If first character is ';', we have a comment, so
    // get out of here.

    if (*thisStr == ';')
    {
        return (thisStr);
    }
        // Process hex characters.

    while (*thisStr)
    {
        // Do uppercase conversion if necessary.

        x = *thisStr;
        if ((x >= 'a') && (x <= 'f'))
        {
            x &= ~0x20;
        }
        // Check for valid digits.

        if ( !(((x >= '0') && (x <= '9')) || ((x >= 'A') && (x <= 'F'))))
        {
            break;
        }
        // Hex ASCII to binary conversion.

        tVal = (MUINT32)(x - '0');
        if (tVal > 9)
        {
            tVal -= 7;
        }

        thisVal = (thisVal * 16) + tVal;

        thisStr++;
    }

        // Return updated pointer and decoded value.

    *outVal = thisVal;
    return (thisStr);
}

/*******************************************************************************
*
********************************************************************************/
void 
ImgSensorDrv::customerInit(void)
{
    FILE *fp = NULL; 

    fp = fopen("/data/sensor_init.txt", "r"); 
    if (fp == NULL)
    {
        printf("No Customer Sensor Init File Exist \n"); 
        return; 
    }

    ACDK_SENSOR_REG_INFO_STRUCT sensorReg; 
    memset (&sensorReg, 0, sizeof(sensorReg)); 
    UINT32 featureLen = sizeof(ACDK_SENSOR_REG_INFO_STRUCT); 
    
    char addrStr[20]; 
    char dataStr[20]; 
    SENSOR_DRV_DBG("[Write Customer Sensor Init Reg]:\n"); 
    fgets(dataStr, 20, fp); 
    if (strncmp(dataStr, "mt65xx_yuv_tuning", 17) != 0)
    {
        SENSOR_DRV_ERR("Error Password \n"); 
        fclose(fp); 
        return; 
    }

    while (!feof(fp))
    {
        fscanf(fp, "%s %s\n", addrStr, dataStr); 
        if (strlen(addrStr) != 0 && strlen(dataStr) != 0)
        {
            u32 addrVal = 0; 
            u32 dataVal = 0; 

            getHexToken(addrStr, &addrVal); 
            getHexToken(dataStr, &dataVal); 

            SENSOR_DRV_DBG("Addr:0x%x, data:0x%x\n", addrVal, dataVal); 
            sensorReg.RegAddr = addrVal; 
            sensorReg.RegData = dataVal;         

            featureControl(SENSOR_FEATURE_SET_REGISTER, (MUINT8 *)&sensorReg, (MUINT32 *)&featureLen); 
        }
    }
    
    fclose(fp); 
}
#endif 


/*******************************************************************************
*
********************************************************************************/
MINT32
ImgSensorDrv::open()
{
    AutoCPTLog cptlog(Event_Sensor_open);
    SENSOR_DRV_DBG("ImgSensorDrv::open() + \r\n");

    MINT32 err = SENSOR_NO_ERROR;
//! MT5192 chip workaround, due to IO leakage 
//! This will cause the init time to increase about 450ms 


#if 0 //def ATVCHIP_MTK_ENABLE  
#if 1
    atvWorkaroundOn(); 
#else 
    SENSOR_DRV_DBG("MATV WORKAROUND - power up init\n");
    //android::ATVCtrl::ATVC_matv_ps_init(1);
    //using service directly instead of using ATVCtrl
    //ATVCtrl will causes the client of ATVCtrlService to change
    //This break the rule of using ATVCtrlService by only 1 client
    //The callbacks will change and may cause problem
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
                    
             LOGW("ATVCtrlService not published, waiting...");
             usleep(500000); // 0.5 s
          } while(true);
          spATVCtrlService = interface_cast<IATVCtrlService>(binder);

          //DONOT shutdown in sensor searching state,
          if (true == g_bATVWorkaroundDoShutDown) {
            spATVCtrlService->ATVCS_matv_shutdown();
          }
          spATVCtrlService->ATVCS_matv_ps_init(1);        
       }
        else
        {
            matv_shutdown();
            matv_ps_init(1);
        }
    }
   psFlag = 1;
   SENSOR_DRV_DBG("[psFlag]:1\n");
#endif 
#endif 
    err = ioctl(m_fdSensor, KDIMGSENSORIOC_T_OPEN);
    if (err < 0) {
        SENSOR_DRV_DBG("[open] Err-ctrlCode (%s) \n", strerror(errno));
#if 0 // def ATVCHIP_MTK_ENABLE  
        atvWorkaroundOff(); 
#endif 
        return -errno;
    }

    SENSOR_DRV_DBG("ImgSensorDrv::open() - \r\n");

#if  defined(YUV_TUNING_SUPPORT )
    customerInit(); 
#endif 

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
ImgSensorDrv::close()
{
    AutoCPTLog cptlog(Event_Sensor_close);
    SENSOR_DRV_DBG("[ImgSensorDrv::close] + \n");

    MINT32 err = SENSOR_NO_ERROR;

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_T_CLOSE);
    if (err < 0) {
        SENSOR_DRV_ERR("[close] Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }

//! MT5192 chip workaround, due to IO leakage 
#if 0 //def ATVCHIP_MTK_ENABLE

#if 1
    atvWorkaroundOff(); 
#else  
    // calles only when psFlag is 1,
    // if it is 0, it is called by mATV, not camera, we should not execute
    if(psFlag == 1)
    {
    SENSOR_DRV_DBG("MATV WORKAROUND - power off\n");
      //android::ATVCtrl::ATVC_matv_ps_init(0);
      //using service directly instead of using ATVCtrl
      //ATVCtrl will causes the client of ATVCtrlService to change
      //This break the rule of using ATVCtrlService by only 1 client
      //The callbacks will change and may cause problem
      if (matvdrv_exist()!=0) {
          if (!is_factory_boot())
          {
          sp<IATVCtrlService> spATVCtrlService;
          sp<IServiceManager> sm = defaultServiceManager();
          sp<IBinder> binder;
          do{
             binder = sm->getService(String16("media.ATVCtrlService"));
             if (binder != 0)
                break;
                    
             LOGW("ATVCtrlService not published, waiting...");
             usleep(500000); // 0.5 s
          } while(true);
          spATVCtrlService = interface_cast<IATVCtrlService>(binder);
          spATVCtrlService->ATVCS_matv_ps_init(0);
          }
          else
          {
              matv_ps_init(0);
          }
      }
      psFlag = 0;
      SENSOR_DRV_DBG("[psFlag]:0\n");
    }
#endif 
#endif 

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
ImgSensorDrv::getInfo(
    ACDK_SCENARIO_ID_ENUM ScenarioId,
    ACDK_SENSOR_INFO_STRUCT *pSensorInfo,
    ACDK_SENSOR_CONFIG_STRUCT *pSensorConfigData
)
{
    ACDK_SENSOR_GETINFO_STRUCT getInfo;   
    MINT32 err = SENSOR_NO_ERROR;
    if (NULL == pSensorInfo|| NULL == pSensorConfigData) {
        SENSOR_DRV_ERR("[getInfo] NULL pointer\n");
        return SENSOR_UNKNOWN_ERROR;
    }
    getInfo.ScenarioId = ScenarioId;
    getInfo.pInfo = pSensorInfo;
    getInfo.pConfig = pSensorConfigData;

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GETINFO , &getInfo);
    if (err < 0) {
        SENSOR_DRV_ERR("[getInfo]Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
ImgSensorDrv::getResolution(
    ACDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution
)
{
    MINT32 err = SENSOR_NO_ERROR;
    if (NULL == pSensorResolution) {
        SENSOR_DRV_ERR("[getResolution] NULL pointer\n");
        return SENSOR_UNKNOWN_ERROR;
    }

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GETRESOLUTION , pSensorResolution);
    if (err < 0) {
        SENSOR_DRV_ERR("[getResolution] Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }
    
    return err;
    
}//halSensorGetResolution

/*******************************************************************************
*
********************************************************************************/
MINT32 
ImgSensorDrv::featureControl(
    ACDK_SENSOR_FEATURE_ENUM FeatureId,
    MUINT8 *pFeaturePara,
    MUINT32 *pFeatureParaLen
)
{
    ACDK_SENSOR_FEATURECONTROL_STRUCT featureCtrl;
    MINT32 err = SENSOR_NO_ERROR;

    if(SENSOR_FEATURE_SINGLE_FOCUS_MODE == FeatureId || SENSOR_FEATURE_CANCEL_AF == FeatureId 
		|| SENSOR_FEATURE_CONSTANT_AF == FeatureId){
    //AF INIT || AF constant has no parameters
    }
    else{
		
        if (NULL == pFeaturePara || NULL == pFeatureParaLen) {
            return SENSOR_INVALID_PARA;
        }
    }		

    featureCtrl.FeatureId = FeatureId;
    featureCtrl.pFeaturePara = pFeaturePara;
    featureCtrl.pFeatureParaLen = pFeatureParaLen;

    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_FEATURECONCTROL , &featureCtrl);
    if (err < 0) {
        SENSOR_DRV_ERR("[featureControl] Err-ctrlCode (%s) \n", strerror(errno));
        return -errno;
    }

    return err;
}//halSensorFeatureControl

/*******************************************************************************
*
********************************************************************************/
MINT32 
ImgSensorDrv::setFoundDrvsActive(
MUINT32 socketIdxes
)
{
    MINT32 err = SENSOR_NO_ERROR;
    MUINT32 sensorDrvInfo[KDIMGSENSOR_MAX_INVOKE_DRIVERS] = {0,0};

    IMGSNESOR_FILL_SET_DRIVER_INFO(socketIdxes);

    SENSOR_DRV_DBG("[%s][0x%x] \n",__FUNCTION__,socketIdxes);
    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_SET_DRIVER,sensorDrvInfo);
    if (err < 0) {
        SENSOR_DRV_ERR("ERROR:setFoundDrvsActive\n");
    }
    return err;
}
/*******************************************************************************
*
********************************************************************************/
IMAGE_SENSOR_TYPE 
ImgSensorDrv::getCurrentSensorType(
) 
{
     ACDK_SENSOR_INFO_STRUCT SensorInfo;
     ACDK_SENSOR_CONFIG_STRUCT SensorConfigData;
    if (SENSOR_NO_ERROR != getInfo(ACDK_SCENARIO_ID_CAMERA_PREVIEW, &SensorInfo, &SensorConfigData)) {
       SENSOR_DRV_ERR("[searchSensor] Error:getInfo() \n");
       return IMAGE_SENSOR_TYPE_UNKNOWN;
    }    

    if (SensorInfo.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_RAW_B && 
         SensorInfo.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_R) {
        return IMAGE_SENSOR_TYPE_RAW;
    }
    else if (SensorInfo.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_UYVY && 
                SensorInfo.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YVYU) {
        return IMAGE_SENSOR_TYPE_YUV; 
    }
    else if (SensorInfo.SensorOutputDataFormat >= SENSOR_OUTPUT_FORMAT_CbYCrY &&
                SensorInfo.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_YCrYCb) {
        return IMAGE_SENSOR_TYPE_YCBCR; 
    }
    else {
        return IMAGE_SENSOR_TYPE_UNKNOWN; 
    }

    return IMAGE_SENSOR_TYPE_UNKNOWN; 
}
/*******************************************************************************
*
********************************************************************************/
IMGSENSOR_SOCKET_POSITION_ENUM
ImgSensorDrv::getSocketPosition(CAMERA_DUAL_CAMERA_SENSOR_ENUM socket)
{
MUINT32 socketPos = socket;
MINT32 err = SENSOR_NO_ERROR;
    err = ioctl(m_fdSensor, KDIMGSENSORIOC_X_GET_SOCKET_POS , &socketPos);
    if (err < 0) {
        SENSOR_DRV_ERR("[getInfo]Err-ctrlCode (%s) \n", strerror(errno));
        return IMGSENSOR_SOCKET_POS_NONE;
    }

    SENSOR_DRV_DBG("[%s]:[%d][%d] \n",__FUNCTION__,socket,socketPos);

    return (IMGSENSOR_SOCKET_POSITION_ENUM)socketPos;
}

/*******************************************************************************
*
********************************************************************************/
#if 0 //defined (ATVCHIP_MTK_ENABLE)
MINT32 
ImgSensorDrv::atvWorkaroundOn()
{
    SENSOR_DRV_DBG("MATV WORKAROUND - power up init\n");
    //android::ATVCtrl::ATVC_matv_ps_init(1);
    //using service directly instead of using ATVCtrl
    //ATVCtrl will causes the client of ATVCtrlService to change
    //This break the rule of using ATVCtrlService by only 1 client
    //The callbacks will change and may cause problem
    if (matvdrv_exist() == 1) {
        if (!::is_factory_boot())
        {
          sp<IATVCtrlService> spATVCtrlService;
          sp<IServiceManager> sm = defaultServiceManager();
          sp<IBinder> binder;
          do{
             binder = sm->getService(String16("media.ATVCtrlService"));
             if (binder != 0)
                break;
                    
             LOGW("ATVCtrlService not published, waiting...");
             usleep(500000); // 0.5 s
          } while(true);
          spATVCtrlService = interface_cast<IATVCtrlService>(binder);

          //DONOT shutdown in sensor searching state,
          if (true == g_bATVWorkaroundDoShutDown) {
            spATVCtrlService->ATVCS_matv_shutdown();
          }
          spATVCtrlService->ATVCS_matv_ps_init(1);        
       }
        else
        {
            matv_shutdown();
            matv_ps_init(1);
        }
    }
   psFlag = 1;
   SENSOR_DRV_DBG("[psFlag]:1\n");
   return SENSOR_NO_ERROR; 
}

/*******************************************************************************
*
********************************************************************************/
MINT32 
ImgSensorDrv::atvWorkaroundOff()
{
    // calles only when psFlag is 1,
    // if it is 0, it is called by mATV, not camera, we should not execute
    if(psFlag == 1)
    {
        SENSOR_DRV_DBG("MATV WORKAROUND - power off\n");
        //android::ATVCtrl::ATVC_matv_ps_init(0);
        //using service directly instead of using ATVCtrl
        //ATVCtrl will causes the client of ATVCtrlService to change
        //This break the rule of using ATVCtrlService by only 1 client
        //The callbacks will change and may cause problem
        if (matvdrv_exist() == 1) {
            if (!is_factory_boot()) {
                sp<IATVCtrlService> spATVCtrlService;
                sp<IServiceManager> sm = defaultServiceManager();
                sp<IBinder> binder;
                do{
                    binder = sm->getService(String16("media.ATVCtrlService"));
                    if (binder != 0)
                       break;
                           
                    LOGW("ATVCtrlService not published, waiting...");
                    usleep(500000); // 0.5 s
                } while(true);
                spATVCtrlService = interface_cast<IATVCtrlService>(binder);
                spATVCtrlService->ATVCS_matv_ps_init(0);
          }
          else {
              matv_ps_init(0);
          }
       }
         psFlag = 0;
         SENSOR_DRV_DBG("[psFlag]:0\n");
    }
    return SENSOR_NO_ERROR; 
}
#endif 

