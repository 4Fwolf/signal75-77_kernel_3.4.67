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
#define LOG_TAG "IspHal"
//
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <cutils/properties.h>
//#include "CamUtilsProperty.h"
//
#include "strobe_drv.h"
#include "isp_hal_imp.h"
#include "CameraProfile.h"
#include "camera_custom_n3d.h"

//using namespace android::MHalCamUtils;

//-----------------------------------------------------------------------------
IspHal* IspHal::createInstance(void)
{
    return IspHalImp::getInstance();
}
//-----------------------------------------------------------------------------
IspHal* IspHalImp::getInstance(void)
{
    MINT32 ret;
    LOG_MSG("");
    static IspHalImp singleton;
    return &singleton;
}
//-----------------------------------------------------------------------------
void IspHalImp::destroyInstance(void) 
{  
}
//-----------------------------------------------------------------------------
IspHalImp::IspHalImp() :IspHal()
{
    MUINT32 i;
    //
    LOG_MSG("");
    //
    m3dType = ISP_HAL_3D_TYPE_NONE;   
    mUsers = 0;
    mpDrv = NULL;
    mpSensorDrv = NULL;
    mpMcuDrv = NULL;
    mSensorDev = ISP_HAL_SENSOR_DEV_MAIN;
    mIspSensorType = ISP_HAL_SENSOR_TYPE_RAW;
    mImageSensorType = IMAGE_SENSOR_TYPE_RAW;
    //
    #if ISP_HAL_TUNING_SUPPORT
    for(i=0; i<6; i++)
    {
        mpIspTuningObj[i] = NULL;
    }
    mpIspTuningCurrent = NULL;
    #endif
    //
    #if ISP_HAL_M4U_SUPPORT
    mpM4UDrv = NULL;
    #endif

    mbFreeM4U = MTRUE;
    memset(&mIspM4UPort,0,sizeof(ISP_HAL_M4U_STRUCT));

}
//-----------------------------------------------------------------------------
IspHalImp::~IspHalImp()
{
    LOG_MSG("");
}
//-----------------------------------------------------------------------------
MINT32 IspHalImp::dumpReg(void)
{
    MINT32 ret = 0;
    //
    LOG_MSG("");
    //
    ret = mpDrv->dumpReg();
    //
    return ret;   
}
//-----------------------------------------------------------------------------
MINT32 IspHalImp::searchSensor(void)
{
    MINT32 sensorDevs = 0;
    MINT32 ret;
    MINT32 clkCnt = 5;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MBOOL n3dFlag = get_N3DFeatureFlag();
    //
    LOG_MSG("");
    //
    if(!mpDrv)
    {
        mpDrv = IspDrv::createInstance();
        if(!mpDrv)
        {
            LOG_ERR("IspDrv::createInstance fail");
            ret = -1;
            return ret;
        }
    }
    //
    ret = mpDrv->init();
    if(ret < 0)
    {
        LOG_ERR("mpDrv->init() fail");
        return ret;
    }


    
    if(n3dFlag == true)
    {
        ret = mpDrv->initTG2(0,0);
        if(ret < 0)
        {
            LOG_ERR("mpDrv->initTG2() fail");
            return ret;
        }
        else
        {
            LOG_MSG("mpDrv->initTG2() pass\n");
        }
    }



    memset(&mSensorInfo, 0, sizeof(ACDK_SENSOR_INFO_STRUCT));
    memset(&mSensorCfg, 0, sizeof(ACDK_SENSOR_CONFIG_STRUCT));
    // Before searching sensor, need to turn on clock of TG
    // Config TG, always use Camera PLL, 1: 120.3MHz, clkCnt is 5, 120.3 / 5 = 24.6MHz
    // TODO FIX_ME
    #if 1
    ret = mpDrv->setTgPhaseCounter(
            1,
            ISP_DRV_CAM_PLL_48_GROUP, /*mSensorInfo.SensorMasterClockSwitch,*/
            clkCnt,
            mSensorInfo.SensorClockPolarity ? 0 : 1,
            mSensorInfo.SensorClockFallingCount,
            mSensorInfo.SensorClockRisingCount);

    if(ret < 0)
    {
        LOG_ERR("setTgPhaseCounter fail");
        return ret;       
    }



    if(n3dFlag == true)
    {
        mpDrv->setIOConfig(1);
        
        ret = mpDrv->setTg2PhaseCounter(1, 
                                        ISP_DRV_CAM_PLL_48_GROUP,
                                        clkCnt, 
                                        mSensorInfo.SensorClockPolarity ? 0 : 1,
                                        mSensorInfo.SensorClockFallingCount,
                                        mSensorInfo.SensorClockRisingCount);

        if(ret < 0)
        {
            LOG_ERR("setTg2PhaseCounter fail");
            return ret;
        }
        else
        {
            LOG_MSG("setTg2PhaseCounter pass\n");
        }
    }



    #else
    // for FPGA
    // clkCnt = 0;
    ret = mpDrv->setTgPhaseCounter(
            1,
            0, /*mSensorInfo.SensorMasterClockSwitch,*/
            clkCnt,
            mSensorInfo.SensorClockPolarity ? 0 : 1,
            mSensorInfo.SensorClockFallingCount,
            mSensorInfo.SensorClockRisingCount);
    #endif        
    
    // Search sensor
    sensorDevs = SensorDrv::searchSensor(NULL);
    //
    mpSensorDrv = SensorDrv::createInstance(mSensorDev);
    ret = mpSensorDrv->init(mSensorDev);
    if(ret < 0)
    {
        LOG_ERR("halSensorInit fail");
    }
    //
    ret = mpSensorDrv->getInfo(ACDK_SCENARIO_ID_CAMERA_PREVIEW, &mSensorInfo, &mSensorCfg);
    if(ret < 0)
    {
        LOG_ERR("getSensorInfo fail");      
    }
    


    LOG_MSG("sensorDevs(%d), SensorDriver3D(%d)",sensorDevs, mSensorInfo.SensorDriver3D);
    
    if(sensorDevs > ISP_SENSOR_DEV_MAIN_2)  
    {
        property_set(ISP_HAL_STEREO_3D_SUPPORT, ISP_HAL_STEREO_3D_SUPPORT_Y);
    }
    else    
    {
        if(mSensorInfo.SensorDriver3D == SENSOR_3D_NOT_SUPPORT)
        {
            property_set(ISP_HAL_STEREO_3D_SUPPORT, ISP_HAL_STEREO_3D_SUPPORT_N);
        }
        else
        {
            property_set(ISP_HAL_STEREO_3D_SUPPORT, ISP_HAL_STEREO_3D_SUPPORT_Y);
        }
    }
    

    
    property_get(ISP_HAL_STEREO_3D_SUPPORT,value,ISP_HAL_STEREO_3D_SUPPORT_N);
    LOG_MSG("ISP_HAL_STEREO_3D_SUPPORT(%d)",atoi(value));
    //
    mpSensorDrv->uninit();
    mpSensorDrv->destroyInstance();
    mpSensorDrv = NULL;
    //
    if(mpDrv)
    {
        if(n3dFlag == true)
        {
            mpDrv->uninitTG2();
        }        
        mpDrv->uninit();
        mpDrv->destroyInstance();
        mpDrv = NULL;
    }
    //    
    if(ret < 0)
    {
        LOG_ERR("mpDrv->uninit() fail");
        return ret;
    }
    //
    return sensorDevs;
}
//-----------------------------------------------------------------------------
MINT32 IspHalImp::init(void)
{
    AutoCPTLog cptlog(Event_IspHal_init);
    MINT32 ret = 0;
    MINT32 i4CurrSensorDev = 0, i4CurrSensorId = 0, i4CurrLensId = 0;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    //
    LOG_MSG("mUsers(%d)", mUsers);
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUsers > 0)
    {
        LOG_MSG("Has inited");
        android_atomic_inc(&mUsers);
        return 0;
    }
    //
    CPTLogStr(Event_IspHal_init, CPTFlagSeparator, "create/init ispDrv");
    if(!mpDrv)
    {
        mpDrv = IspDrv::createInstance();
        if(!mpDrv)
        {
            LOG_ERR("IspDrv::createInstance fail");
            ret = -1;
            return ret;
        }
    }
    //
    if(mSensorDev == ISP_HAL_SENSOR_DEV_NONE)
    {
        LOG_MSG("mSensorDev is NONE");
        return ret;
    }
    //
    ret = mpDrv->init();
    if(ret < 0)
    {
        LOG_ERR("mpDrv->init() fail");
        return ret;
    }
    //
    CPTLogStr(Event_IspHal_init, CPTFlagSeparator, "create sensorDrv");



    if(m3dType == ISP_HAL_3D_TYPE_N3D)
    {
        ret = mpDrv->initTG2(0,0);  
        if(ret < 0)
        {
            LOG_ERR("mpDrv->initTG2() fail");
            return ret;
        }
        else
        {
            LOG_MSG("mpDrv->initTG2() pass\n");
            mpSensorDrv = SensorDrv::createInstance(ISP_HAL_SENSOR_DEV_MAIN | ISP_HAL_SENSOR_DEV_MAIN_2);
        }
    }
    else
    {
        mpSensorDrv = SensorDrv::createInstance(mSensorDev); 
    }        
    

    
    // Before searching sensor, need to turn on TG
    CPTLogStr(Event_IspHal_init, CPTFlagSeparator, "initSensor");
    ret = initSensor();
    if(ret < 0)
    {
        LOG_ERR("initSensor fail");
        return ret;
    }
    // Get sensor info before setting TG phase counter
    CPTLogStr(Event_IspHal_init, CPTFlagSeparator, "getSensorInfo");
    ret = getSensorInfo(1);
    if(ret < 0)
    {
        LOG_ERR("getSensorInfo fail");
        return ret;
    }
    //
    CPTLogStr(Event_IspHal_init, CPTFlagSeparator, "setTgPhase");
    ret = setTgPhase();
    if(ret < 0)
    {
        LOG_ERR("setTgPhase fail");
        return ret;
    }


    if(m3dType == ISP_HAL_3D_TYPE_N3D)
    {
        ret = setTg2Phase();
        if(ret < 0)
        {
            LOG_ERR("setTg2Phase() fail");
            return ret;
        }

         LOG_MSG("setTg2Phase() pass\n");
    }

    
    CPTLogStr(Event_IspHal_init, CPTFlagSeparator, "setSensorIODrivingCurrent");
    ret = setSensorIODrivingCurrent();
    if(ret < 0)
    {
        LOG_ERR("initial IO driving current fail");
        return ret;
    }
    //
    CPTLogStr(Event_IspHal_init, CPTFlagSeparator, "initCSI2Peripheral");
    ret = initCSI2Peripheral(1);  // if the interface is mipi, enable the csi2
    if(ret < 0)
    {
        LOG_ERR("initial CSI2 peripheral fail");
        return ret;
    }
    //
    CPTLogStr(Event_IspHal_init, CPTFlagSeparator, "setCSI2Config(1)");
    ret = setCSI2Config(1);     // enable and config CSI2.
    if(ret < 0)
    {
        LOG_ERR("set CSI2 config fail");
        return ret;
    }    
    // Open sensor, try to open 3 time
    CPTLogStr(Event_IspHal_init, CPTFlagSeparator, "open Sensor");
    for(MINT32 i =0; i < 3; i++)
    {
        ret = mpSensorDrv->open();
        if(ret < 0)
        {
            LOG_ERR("mpSensorDrv->open fail, retry(%d)", i);
        }
        else
        {
            break; 
        }
    }
    //
    if(ret < 0)
    {
        LOG_ERR("mpSensorDrv->open fail");
        return ret;
    }
    // Init Mcu
    CPTLogStr(Event_IspHal_init, CPTFlagSeparator, "get sensor id");
    if(mSensorDev == ISP_HAL_SENSOR_DEV_MAIN)
    {
        i4CurrSensorDev  = 0;
        i4CurrSensorId = mpSensorDrv->getMainSensorID();
    }
    else
    if(mSensorDev == ISP_HAL_SENSOR_DEV_SUB)
    {

        i4CurrSensorDev  = 1;
        i4CurrSensorId = mpSensorDrv->getSubSensorID();        
    }
    //    
    CPTLogStr(Event_IspHal_init, CPTFlagSeparator, "create/init MCUDrv");
    MCUDrv::lensSearch(i4CurrSensorDev, i4CurrSensorId);
    i4CurrLensId = MCUDrv::getCurrLensID();
    //LOG_MSG("[Isp_hal][currSensorId] %d [currLensId] %d", i4CurrSensorId, i4CurrLensId);
    mpMcuDrv = MCUDrv::createInstance(i4CurrLensId);   
    //
    if(!mpMcuDrv)
    {
        LOG_ERR("McuDrv::createInstance fail");
        ret = -3;
        return ret;
    }
    //
    if(mpMcuDrv->init() < 0)
    {
        LOG_ERR("mpMcuDrv->init() fail");
    }
    // Init tuning
    #if ISP_HAL_TUNING_SUPPORT
    CPTLogStr(Event_IspHal_init, CPTFlagSeparator, "create/init IspTuning");
    ESensorType_T type = ESensorType_RAW;
    ESensorRole_T role = ESensorRole_Main;
    //
    if(mIspSensorType == ISP_HAL_SENSOR_TYPE_YUV)
    {
        type = ESensorType_YUV;
    }
    //
    switch (mSensorDev)
    {
        case ISP_HAL_SENSOR_DEV_MAIN:
        {
            role = ESensorRole_Main;
            break;
        }
        case ISP_HAL_SENSOR_DEV_SUB:
        {
            role = ESensorRole_Sub;
            break;
        }
        case ISP_HAL_SENSOR_DEV_ATV:
        {
            role = ESensorRole_ATV;
            break;
        }
        default:
        {
            LOG_ERR("mSensorDev error");
            return -1;
            break;
        }
    }
    //
    if(mpIspTuningObj[mSensorDev] == NULL)
    {
        mpIspTuningObj[mSensorDev] = IspTuningHal::createInstance(type, role);
        if(mpIspTuningObj[mSensorDev] == NULL)
        {
            ret = -1;
            LOG_ERR("mpIspTuningObj[mSensorDev] is NULL");
            return ret;
        }
    }
    mpIspTuningCurrent = mpIspTuningObj[mSensorDev]; 
    if(mpIspTuningCurrent->init() != NSIspTuning::MERR_OK)
    {
        ret = -2;
        LOG_ERR("mpIspTuningCurrent->init() fail, type(%d), mSensorDev(%d)",type,mSensorDev);
        return ret;
    }
    #endif
    //
    CPTLogStr(Event_IspHal_init, CPTFlagSeparator, "new MTKM4UDrv");
    //
    LOG_MSG("SensorDriver3D(%d)",mSensorInfo.SensorDriver3D);
    if(mSensorInfo.SensorDriver3D != SENSOR_3D_NOT_SUPPORT)
    {
        //Property::get(ISP_HAL_STEREO_3D_ENABLE,value,ISP_HAL_STEREO_3D_ENABLE_N);
        //if((MBOOL)(atoi(value)))
        //{
        //    m3dType = ISP_HAL_3D_TYPE_B3D;
        //}
        LOG_MSG("Not support B3D");
    }
    LOG_MSG("m3dType(%d)",(int)m3dType);
    //
    #if ISP_HAL_M4U_SUPPORT
    mpM4UDrv = new MTKM4UDrv();
    memset (&mIspReadPort, 0,  sizeof(ISP_HAL_M4U_STRUCT)); 
    memset (&mIspWritePort, 0,  sizeof(ISP_HAL_M4U_STRUCT)); 
    #endif
    //
    android_atomic_inc(&mUsers);
    //
    return ret;   
}
//-----------------------------------------------------------------------------
MINT32 IspHalImp::uninit(void)
{
    AutoCPTLog cptlog(Event_IspHal_uninit);
    MINT32 ret = 0;
    //
    LOG_MSG("mUsers(%d)", mUsers);
    //
    Mutex::Autolock lock(mLock);
    //
    if(mUsers <= 0)
    {
        // No more users
        return 0;
    }
    // More than one user
    android_atomic_dec(&mUsers);
    //
    if(mUsers == 0)
    {
        CPTLogStr(Event_IspHal_uninit,CPTFlagSeparator, "initCSI2Peripheral(0)");
        ret = initCSI2Peripheral(0);  // if the interface is mipi, disable the csi2
        if(ret < 0)
        {
            LOG_ERR("initial CSI2 peripheral fail");
            return ret;
        }
        //
        CPTLogStr(Event_IspHal_uninit,CPTFlagSeparator, "uninit/destroy McuDrv");
        if(mpMcuDrv)
        {
            CPTLog(Event_uninitMCUDrv, CPTFlagStart);
            mpMcuDrv->uninit();
            CPTLog(Event_uninitMCUDrv, CPTFlagEnd);
            mpMcuDrv->destroyInstance();
            mpMcuDrv = NULL;
        }
        //
        CPTLogStr(Event_IspHal_uninit,CPTFlagSeparator, "uninit/destroy SensorDrv");
        if(mpSensorDrv)
        {
            mpSensorDrv->close();
            mpSensorDrv->uninit();
            mpSensorDrv->destroyInstance();
            mpSensorDrv = NULL;
        }
        //
        CPTLogStr(Event_IspHal_uninit,CPTFlagSeparator, "destroy M4UDrv");
        #if ISP_HAL_M4U_SUPPORT
        if(mpM4UDrv)
        {
            delete mpM4UDrv;
            mpM4UDrv = NULL; 
            memset(&mIspReadPort,0,sizeof(ISP_HAL_M4U_STRUCT)); 
            memset(&mIspWritePort,0,sizeof(ISP_HAL_M4U_STRUCT)); 
        }
        #endif
        //
        CPTLogStr(Event_IspHal_uninit,CPTFlagSeparator, "destroy IspTuning");
        #if ISP_HAL_TUNING_SUPPORT
        for(MINT32 i = 0; i <= ISP_HAL_SENSOR_DEV_ATV; i++)
        {
            if(mpIspTuningObj[i])
            {
                mpIspTuningObj[i]->destroyInstance();
                mpIspTuningObj[i] = NULL;
            }
        }
        //
        mpIspTuningCurrent = NULL;
        #endif
        //
        CPTLogStr(Event_IspHal_uninit,CPTFlagSeparator, "uninit/destroy IspDrv");
        if(mpDrv)
        {
            if(m3dType == ISP_HAL_3D_TYPE_N3D)
            {
                mpDrv->uninitTG2(); 
            }
            mpDrv->uninit();
            mpDrv->destroyInstance();
            mpDrv = NULL;
        }
        //
        mSensorDev = ISP_HAL_SENSOR_DEV_MAIN;
        mIspSensorType = ISP_HAL_SENSOR_TYPE_RAW;
        mImageSensorType = IMAGE_SENSOR_TYPE_RAW;
        m3dType = ISP_HAL_3D_TYPE_NONE; 
    }
    else
    {
        LOG_MSG("Still users(%d)",mUsers);
    }
    //
    return ret;   
}
//-----------------------------------------------------------------------------
MINT32 IspHalImp::start(void)
{
    MINT32 ret = 0;
    //
    LOG_MSG("");
    //
    ret = mpDrv->control(1);
    if(ret < 0)
    {
        LOG_ERR("control fail");
        return ret;
    }



    if(m3dType == ISP_HAL_3D_TYPE_N3D)
    {
        ret = mpDrv->controlTG2(1);
        if(ret < 0)
        {
            LOG_ERR("control start TG2 fail");
            return ret;
        }
        LOG_MSG("control start TG2 pass\n");
    }
    
    
    if(mSensorDev == ISP_HAL_SENSOR_DEV_ATV)
    {
        ret = mpDrv->setMCLKEn(0);
        if(ret < 0)
        {
            LOG_ERR("CloseMclk fail");
            return ret;
        }
    }
    //
    return ret;   
}
//-----------------------------------------------------------------------------
MINT32 IspHalImp::stop(void)
{
    MINT32 ret = 0;
    //
    LOG_MSG("");
    //
    mpDrv->setSubsampleWidth(
        MFALSE,
        0,
        0);
    //
    mpDrv->setSubsampleHeight(
        MFALSE,
        0,
        0);
    //
    ret = mpDrv->control(0);
    if(ret < 0)
    {
        LOG_ERR("control fail");
        return ret;
    }
    
  

    if(m3dType == ISP_HAL_3D_TYPE_N3D)
    {
        ret = mpDrv->controlTG2(0);
        if(ret < 0)
        {
            LOG_ERR("control stop TG2 fail");
            return ret;
        }
        LOG_MSG("control off TG2 pass\n");
    }
    

    
    #if ISP_HAL_M4U_SUPPORT
    freeM4UPort();
    #endif 
    //
    return ret;
}
//-----------------------------------------------------------------------------
MINT32 IspHalImp::setConf(ISP_HAL_CONFIG_STRUCT *pConfig)
{
    AutoCPTLog cptlog(Event_IspHal_setConf);
    MINT32 ret = 0;
    MINT32 pixelX0, pixelY0, pixelX1, pixelY1;
    MINT32 inPathSel, swapY, swapCbCr, inTypeSel;
    MINT32 sensorScenarioId;
    MUINT32 inDataFmt = 0;
    MUINT32 TG2grabW, TG2grabH; 
    SENSOR_DEV_ENUM eSensorDev = SENSOR_NONE;
    //
    LOG_MSG("src:W(%d),H(%d)", pConfig->u4SrcW, pConfig->u4SrcH);
    LOG_MSG("Continuous(%d),srcDram(%d),dstDram(%d),Scen(%d)",pConfig->u4IsContinous, pConfig->u4IsSrcDram, pConfig->u4IsDestDram,pConfig->Scen);
    LOG_MSG("SensorScenario(%d),SensorDelay(%d)",pConfig->u4IsBypassSensorScenario, pConfig->u4IsBypassSensorDelay);
    // Reset isp
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "reset IspDrv");
    ret = mpDrv->reset();
    if(ret < 0)
    {
        LOG_ERR("reset fail");
        goto EXIT;
    }
    // Reset isp sw buffer
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "reset Isp SW buffer");
    ret = mpDrv->sendCommand(ISP_DRV_CMD_RESET_BUF);
    if(ret < 0)
    {
        LOG_ERR("reset isp buffer fail");
    }
    //
    CPTLog(Event_IspHal_setConf, CPTFlagSeparator);
    if(pConfig->u4IsZsd == 1)
    {
        LOG_MSG("ACDK_SCENARIO_ID_CAMERA_ZSD"); 
        sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_ZSD;
    }
    else
    {
        
        if(m3dType == ISP_HAL_3D_TYPE_B3D)
        {
            if( (pConfig->u4SrcW < mSensorResolution.Sensor3DFullWidth) &&
                 (pConfig->u4SrcH < mSensorResolution.Sensor3DFullHeight))
            {
                LOG_MSG("ACDK_SCENARIO_ID_CAMERA_3D_PREVIEW"); 
                sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_3D_PREVIEW;
            }
            else
            {
                LOG_MSG("ACDK_SCENARIO_ID_CAMERA_3D_CAPTURE"); 
                sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_3D_CAPTURE;
            }
        }
        else if( mSensorResolution.SensorPreviewWidth == mSensorResolution.SensorFullWidth &&
                 mSensorResolution.SensorPreviewHeight == mSensorResolution.SensorFullHeight)
        {
            LOG_MSG("Preview and capture size are the same,Scen(%d)",pConfig->Scen);
            switch(pConfig->Scen)
            {
                case ISP_HAL_SCEN_CAM_PRV:
                {
                    LOG_MSG("ACDK_SCENARIO_ID_CAMERA_PREVIEW");
                    sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
                    break;
                }
                case ISP_HAL_SCEN_CAM_CAP:
                {
                    LOG_MSG("ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG");
                    sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
                    break;
                }
                default:
                {
                    LOG_ERR("Unknown Scen(%d)",pConfig->Scen);
                    break;
                }
            }
        }
        else
        {
            if( (pConfig->u4SrcW < mSensorResolution.SensorFullWidth) &&
                 (pConfig->u4SrcH < mSensorResolution.SensorFullHeight))
            {
                LOG_MSG("ACDK_SCENARIO_ID_CAMERA_PREVIEW"); 
                sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_PREVIEW;
            }
            else
            {
                LOG_MSG("ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG");
                sensorScenarioId = ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG;
            }
        }
    }
    //
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "getSensorInfo");
    if(mSensorDev != ISP_HAL_SENSOR_DEV_NONE)
    {
        ret = getSensorInfo(sensorScenarioId);
        if(ret < 0)
        {
            LOG_ERR("getSensorInfo fail");
            goto EXIT;
        }
    }
    //
    CPTLog(Event_IspHal_setConf, CPTFlagSeparator);
    if(pConfig->u4IsSrcDram == 0)
    {
        #if 0
        //no need to set tg again 
        ret = setTgPhase();
        if(ret < 0)
        {
            LOG_ERR("setTgPhase fail");
            goto EXIT;
        }
        #endif 
        //
        if( 0 == pConfig->u4IsBypassSensorScenario)
        {
            CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "setScenario");
            ret = mpSensorDrv->setScenario((ACDK_SCENARIO_ID_ENUM) sensorScenarioId);
            if(ret < 0)
            {
                LOG_ERR("halSensorSetScenario fail");
                goto EXIT;
            }
            CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "reset VD count");
            mpDrv->sendCommand(ISP_DRV_CMD_RESET_VD_COUNT,0,0,0);
            CPTLog(Event_IspHal_setConf, CPTFlagSeparator);
        }
        // Source is from sensor
        if(mImageSensorType == IMAGE_SENSOR_TYPE_RAW)
        {
            // RAW
            pixelX0 = mSensorInfo.SensorGrabStartX;
            pixelY0 = mSensorInfo.SensorGrabStartY;
            pixelX1 = pixelX0 + pConfig->u4SrcW - 1;
            pixelY1 = pixelY0 + pConfig->u4SrcH - 1;
            inTypeSel = ISP_DRV_INPUT_FORMAT_BAYER;
            swapY = 0;
            swapCbCr = 0;
        }
        else
        {
            // Yuv422 or YCbCr
            pixelX0 = mSensorInfo.SensorGrabStartX;
            pixelY0 = mSensorInfo.SensorGrabStartY;
            pixelX1 = pixelX0 + pConfig->u4SrcW * 2 - 1;
            pixelY1 = pixelY0 + pConfig->u4SrcH - 1;
            swapY = 0;
            swapCbCr = 0;
            //
            if(mImageSensorType == IMAGE_SENSOR_TYPE_YUV)
            {
                inTypeSel = ISP_DRV_INPUT_FORMAT_YUV422;
                if( (mSensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_UYVY) ||
                    (mSensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_VYUY))
                {
                    swapY = 1;
                }
                //
                if( (mSensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_YVYU)||
                    (mSensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_VYUY))
                {
                    swapCbCr = 1;
                }
            }
            else
            {
                inTypeSel = ISP_DRV_INPUT_FORMAT_YCBCR;
                if( (mSensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_CbYCrY) ||
                    (mSensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_CrYCbY))
                {
                    swapY = 1;
                }
                //
                if( mSensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_YCrYCb ||
                    mSensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_CrYCbY)
                {
                    swapCbCr = 1;
                }
            }
        }
        inPathSel = 0;  // From sensor
    }
    else
    {
        // Source is from dram
        pixelX0 = 0;
        pixelY0 = 0;
        swapY = 0;
        swapCbCr = 0;
        inPathSel = pConfig->u4IsSrcDram == 1 ? 1 : 2;   // From CAM or MDP RDMA
        if(pConfig->u4IsDestBayer == 1)
        {
            inTypeSel = ISP_DRV_INPUT_FORMAT_BAYER;  
            pixelX1 = pixelX0 + pConfig->u4SrcW - 1;
            pixelY1 = pixelY0 + pConfig->u4SrcH - 1;
        }
        else
        {
            inTypeSel = ISP_DRV_INPUT_FORMAT_YUV422; 
            pixelX1 = pixelX0 + pConfig->u4SrcW * 2 - 1;
            pixelY1 = pixelY0 + pConfig->u4SrcH -1;
        }        
    }
    //
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "setTgGrabRange");
    ret = mpDrv->setTgGrabRange(pixelX0, pixelX1, pixelY0, pixelY1);
    if(ret < 0)
    {
        LOG_ERR("setTgGrabRange fail");
        goto EXIT;
    }
    //
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "setSensorModeCfg");


    if(m3dType == ISP_HAL_3D_TYPE_N3D)
    {
        if(sensorScenarioId == ACDK_SCENARIO_ID_CAMERA_PREVIEW)
        {
            TG2grabW = mSensorResolution.SensorPreviewWidth & (~0x1); //must be multiple of 2
            TG2grabH = mSensorResolution.SensorPreviewHeight;
        }
        else if(sensorScenarioId == ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG)
        {
            TG2grabW = mSensorResolution.SensorFullWidth & (~0x1);  //must be multiple of 2
            TG2grabH = mSensorResolution.SensorFullHeight;
        }
       
        ret = mpDrv->setTg2GrabRange(0, TG2grabW << 1, 0, TG2grabH);
        if(ret < 0)
        {
            LOG_ERR("setTg2GrabRange fail");
            goto EXIT;
        }
        LOG_MSG("setTg2FrabRange pass\n");
        
        ret = mpDrv->setTG2SensorModeCfg(
                      mSensorInfo.SensorHsyncPolarity ? 0 : 1, 
                      mSensorInfo.SensorVsyncPolarity ? 0 : 1);

        if(ret < 0)
        {
            LOG_ERR("setTG2SensorModeCfg fail");
            goto EXIT;
        }
    }

    
    ret = mpDrv->setSensorModeCfg(
                    mSensorInfo.SensorHsyncPolarity ? 0 : 1, 
                    mSensorInfo.SensorVsyncPolarity ? 0 : 1);
    if(ret < 0)
    {
        LOG_ERR("setSensorModeCfg fail");
        goto EXIT;
    }
    //
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "set pixel type");
    ret = mpDrv->sendCommand(ISP_DRV_CMD_SET_PIXEL_TYPE, (MINT32)mSensorInfo.SensorOutputDataFormat);
    if(ret < 0)
    {
        LOG_ERR("ISP_DRV_CMD_SET_PIXEL_TYPE fail");
        goto EXIT;
    }
    //
    #if 0
    // Set result win
    ret = mpDrv->setResultWin(0, pConfig->u4SrcW - 1, 0, pConfig->u4SrcH - 1, 1);
    if(ret < 0)
    {
        LOG_ERR("setResultWin fail");
        goto EXIT;
    }
    #endif
    // Set IO addr;
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "set IO addr");
    #if ISP_HAL_M4U_SUPPORT
    if(pConfig->u4IsSrcDram &&
        pConfig->u4InPhyAddr != 0)
    {       
        mIspReadPort.useM4U = 1; 
        mIspReadPort.virtAddr = pConfig->u4InPhyAddr; 
        mIspReadPort.size = pConfig->u4MemSize; 

        if(mIspM4UPort.useM4U)
        {
            if(mIspReadPort.virtAddr == mIspM4UPort.virtAddr && mIspReadPort.size == mIspM4UPort.size)
            {
                LOG_MSG("use reserved M4U ISP port memory");
                mIspReadPort.M4UVa = mIspM4UPort.M4UVa;
            }
            else
            {
                freeM4UMemory(mIspM4UPort.virtAddr, mIspM4UPort.M4UVa, mIspM4UPort.size); 
                mIspM4UPort.useM4U = 0; 
                
                ret = allocM4UMemory(pConfig->u4InPhyAddr, mIspReadPort.size, &mIspReadPort.M4UVa); 
                if(ret != 0)
                {
                    LOG_ERR("allocM4UMemory fail"); 
                    goto EXIT; 
                }
            }
        }
        else
        {
            ret = allocM4UMemory(pConfig->u4InPhyAddr, mIspReadPort.size, &mIspReadPort.M4UVa); 
            if(ret != 0)
            {
                LOG_ERR("allocM4UMemory fail"); 
                goto EXIT; 
            }
        }
    }
    //
    if( pConfig->u4IsDestDram &&
        pConfig->u4OutPhyAddr != 0)
    {
        mIspWritePort.useM4U = 1; 
        mIspWritePort.virtAddr = pConfig->u4OutPhyAddr; 
        mIspWritePort.size = pConfig->u4MemSize;

        if(mIspM4UPort.useM4U)
        {
            if(mIspWritePort.virtAddr == mIspM4UPort.virtAddr && mIspWritePort.size == mIspM4UPort.size)
            {
                LOG_MSG("use reserved M4U ISP port memory");
                mIspWritePort.M4UVa = mIspM4UPort.M4UVa;
            }
            else
            {
                freeM4UMemory(mIspM4UPort.virtAddr, mIspM4UPort.M4UVa, mIspM4UPort.size); 
                mIspM4UPort.useM4U = 0;
                
                ret = allocM4UMemory(mIspWritePort.virtAddr , mIspWritePort.size , &mIspWritePort.M4UVa); 
                if(ret != 0)
                {
                    LOG_ERR("allocM4UMemory fail"); 
                    goto EXIT; 
                }
            }
        }
        else
        {
            ret = allocM4UMemory(mIspWritePort.virtAddr , mIspWritePort.size , &mIspWritePort.M4UVa); 
            if(ret != 0)
            {
                LOG_ERR("allocM4UMemory fail"); 
                goto EXIT; 
            }
        }
    }
    //
    ret = mpDrv->setMemIOAddr(mIspReadPort.M4UVa, mIspWritePort.M4UVa);
    if(ret < 0)
    {
        LOG_ERR("setMemIOAddr fail");
        goto EXIT;
    }
    #else 
    ret = mpDrv->setMemIOAddr(pConfig->u4InPhyAddr, pConfig->u4OutPhyAddr);
    if(ret < 0)
    {
        LOG_ERR("setMemIOAddr fail");
        goto EXIT;
    }
    #endif 
    // Set camera module path
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "set camera module path");
    switch (mSensorDev)
    {
        case ISP_HAL_SENSOR_DEV_MAIN:
        {
            eSensorDev = SENSOR_MAIN;
            break;
        }
        case ISP_HAL_SENSOR_DEV_SUB:
        {
            eSensorDev = SENSOR_SUB;
            break;
        }
        default:
        {
            break;
        }
    }
    //
    LOG_MSG("eSensorDev(%d),SensroInterfaceType(%d),SensorOutputDataFormat(%d),SensorRawType(%d)",eSensorDev,mSensorInfo.SensroInterfaceType,mSensorInfo.SensorOutputDataFormat,mSensorInfo.SensorRawType);
    if(mSensorInfo.SensroInterfaceType == SENSOR_INTERFACE_TYPE_PARALLEL)
    {
        ret = mpSensorDrv->sendCommand(CMD_SENSOR_GET_INDATA_FORMAT, &inDataFmt, (MUINT32*)eSensorDev);
        if(ret < 0)
        {
            LOG_ERR("CMD_SENSOR_GET_INDATA_FORMAT fail");
            goto EXIT;
        }
    }
    else
    if(mSensorInfo.SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI)
    {
        if( mSensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_RAW_B ||
            mSensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_RAW_Gb||
            mSensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_RAW_Gr||
            mSensorInfo.SensorOutputDataFormat == SENSOR_OUTPUT_FORMAT_RAW_R)
        {
            if(mSensorInfo.SensorRawType == RAW_TYPE_8BIT)
            {
                inDataFmt = 1;
            }
        }
        else //YUV format
        {
            inDataFmt = 1;
        }
    }
    else
    {
        LOG_ERR("Unsupported SensroInterfaceType(%d)",mSensorInfo.SensroInterfaceType);
        goto EXIT;
    }
    //
    ret = mpDrv->setCamModulePath(
            inPathSel,
            inTypeSel,
            swapY,
            swapCbCr, 
            pConfig->u4IsDestBayer ? ISP_DRV_OUTPUT_FORMAT_BAYER : ISP_DRV_OUTPUT_FORMAT_YUV422, 
            pConfig->u4IsDestDram,
            inDataFmt);
    //
    if(ret < 0)
    {
        LOG_ERR("setCamModulePath fail");
        goto EXIT;
    }
    // Set view finder mode
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "set view finder mode");
    ret = mpDrv->setViewFinderMode(
            pConfig->u4IsContinous ? 0 : 1,
            pConfig->u4IsBypassSensorDelay ? 0 : mSensorInfo.CaptureDelayFrame);
    //
    if(ret < 0)
    {
        LOG_ERR("setViewFinderMode fail");
        goto EXIT;
    }


    if(m3dType == ISP_HAL_3D_TYPE_N3D)
    {
        // Set TG2 view finder mode, same as TG1
        ret = mpDrv->setTG2ViewFinderMode(
                pConfig->u4IsContinous ? 0 : 1,
                pConfig->u4IsBypassSensorDelay ? 0 : mSensorInfo.CaptureDelayFrame);
        
        if(ret < 0)
        {
            LOG_ERR("setTG2ViewFinderMode fail");
            goto EXIT;
        }
    }
    //
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "setCSI2Config(0)");
    ret = setCSI2Config(0);    // disable csi2
    if(ret < 0)
    {
        LOG_ERR("disable csi2 fail");
        goto EXIT;
    }
    //
    #if ISP_HAL_TUNING_SUPPORT
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "setSensorMode");
    ret = mpIspTuningCurrent->setSensorMode(
            sensorScenarioId == ACDK_SCENARIO_ID_CAMERA_PREVIEW ? 
            NSIspTuning::ESensorMode_Preview : NSIspTuning::ESensorMode_Capture);
    if(ret < 0)
    {
        LOG_ERR("setSensorMode fail");
        goto EXIT;
    }
    #endif
    //
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "setCSI2Config(1)");
    ret = setCSI2Config(1);    // enable csi2
    if(ret < 0)
    {
        LOG_ERR("enable csi2 fail");
        goto EXIT;
    }
    //
    #if ISP_HAL_TUNING_SUPPORT
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "validateFrameless");
    ret = mpIspTuningCurrent->validateFrameless();
    if(ret < 0)
    {
        if((MUINT32) ret == NSIspTuning::MERR_UNSUPPORT)
        {
            if(mIspSensorType == ISP_HAL_SENSOR_TYPE_YUV)
            {
                // FIXME
                ret = 0;
            }
        }
        else
        {
            LOG_ERR("validateFrameless fail");
            goto EXIT;
        }
    }
    //
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "validatePerFrame(MFALSE)");
    ret = mpIspTuningCurrent->validatePerFrame(MFALSE);
    if(ret < 0)
    {
        if((MUINT32) ret == NSIspTuning::MERR_UNSUPPORT)
        {
            if(mIspSensorType == ISP_HAL_SENSOR_TYPE_YUV)
            {
                // FIXME
                ret = 0;
            }
        }
        else
        {
            LOG_ERR("validatePerFrame fail");
            goto EXIT;
        }
    }
    #endif
    //
    CPTLogStr(Event_IspHal_setConf, CPTFlagSeparator, "setSubsampleWidth");
    mpDrv->setSubsampleWidth(
        pConfig->SubsampleWidth.Enable,
        pConfig->SubsampleWidth.Input,
        pConfig->SubsampleWidth.Output);
    //
    mpDrv->setSubsampleHeight(
        pConfig->SubsampleHeight.Enable,
        pConfig->SubsampleHeight.Input,
        pConfig->SubsampleHeight.Output);
    //
    EXIT:
    return ret;   
}
//-----------------------------------------------------------------------------
MINT32 IspHalImp::setConfPQ(ISP_HAL_CONFIG_PQ_STRUCT* pConfig)
{
    MINT32 ret = 0;
    //
    if(mpDrv->reset() < 0)
    {
        LOG_ERR("mpDrv->reset() fail");
        ret = -1;
        goto EXIT;
    }
    //
    if(mpDrv->sendCommand(ISP_DRV_CMD_RESET_BUF) < 0)
    {
        LOG_ERR("mpDrv reset buffer fail");
        ret = -1;
        goto EXIT;
    }
    //
    if(mpDrv->setTgGrabRange(0, pConfig->InputSize.Width-1, 1, pConfig->InputSize.Height) < 0)
    {
        LOG_ERR("mpDrv->setTgGrabRange() fail");
        ret = -1;
        goto EXIT;
    }
    //

    if(mpDrv->setCamModulePath(2, 1, 0, 0, ISP_DRV_OUTPUT_FORMAT_YUV422, 0, 1) < 0)
    {
        LOG_ERR("mpDrv->setCamModulePath() fail");
        ret = -1;
        goto EXIT;
    }
    //
    if(mpDrv->setViewFinderMode(1 , 0) < 0)
    {
        LOG_ERR("mpDrv->setViewFinderMode() fail");
        ret = -1;
        goto EXIT;
    }


    if(m3dType == ISP_HAL_3D_TYPE_N3D)
    {
        // Set TG2 view finder mode, same as TG1
        if(mpDrv->setTG2ViewFinderMode(1,0) < 0)        
        {
            LOG_ERR("mpDrv->setTG2ViewFinderMode fail");
            ret = -1;
            goto EXIT;
        }
    }
    //
    if(mpDrv->setSensorModeCfg(0 , 0) < 0)
   {
       LOG_ERR("mpDrv->setSensorModeCfg() fail");
       ret = -1;
       goto EXIT;
   }
    //
    mpDrv->setSubsampleWidth(MFALSE, 0, 0);
    mpDrv->setSubsampleHeight(MFALSE, 0, 0);
    //
    EXIT:
    //
    return ret;
}

//-----------------------------------------------------------------------------
MINT32 IspHalImp::waitDone(MINT32 mode,MINT32 Timeout)
{
    MINT32 ret = 0;
    ISP_DRV_WAIT_IRQ_STRUCT IspDrvWaitIrq;
    //
    IspDrvWaitIrq.Mode = mode;
    IspDrvWaitIrq.Timeout = Timeout;//ISP_DRV_IRQ_TIMEOUT_DEFAULT;
    //LOG_MSG("0x%x", mode);
    // FIXME, mode isp done
    ret = mpDrv->waitIrq(&IspDrvWaitIrq);
    if(ret < 0)
    {
        LOG_ERR("waitDone fail");
        return ret;       
    }
    //LOG_MSG("OK");
    return ret;    
}
//-----------------------------------------------------------------------------
MINT32 IspHalImp::waitIrq(ISP_HAL_WAIT_IRQ_STRUCT* pWaitIrq)
{
    MINT32 ret = 0;
    ISP_DRV_WAIT_IRQ_STRUCT IspDrvWaitIrq;
    //
    IspDrvWaitIrq.Mode = pWaitIrq->Mode;
    IspDrvWaitIrq.Timeout = pWaitIrq->Timeout;
    //LOG_MSG("0x%x", mode);
    // FIXME, mode isp done
    ret = mpDrv->waitIrq(&IspDrvWaitIrq);
    if(ret < 0)
    {
        LOG_ERR("waitDone fail");
        return ret;       
    }
    //LOG_MSG("OK");
    return ret;    
}
//-----------------------------------------------------------------------------
MINT32 IspHalImp::initSensor(void)
{
    MINT32 ret = 0;
    //
    LOG_MSG("");


    if(m3dType == ISP_HAL_3D_TYPE_N3D)
    {        
        ret = mpSensorDrv->init(ISP_HAL_SENSOR_DEV_MAIN | ISP_HAL_SENSOR_DEV_MAIN_2);
        LOG_MSG("initSnsor to N3D\n");
    }
    else
    {
        ret = mpSensorDrv->init(mSensorDev);
    }

    if(ret < 0)
    {
        LOG_ERR("halSensorInit fail");
        return ret;
    }

    
    // Get Sensor Resolution
    ret = mpSensorDrv->getResolution(&mSensorResolution);
    if(ret < 0)
    {
        LOG_ERR("halSensorGetResolution fail");
        return ret;
    }
    LOG_MSG("Sensor resolution, Preview: W(%d), H(%d), Full: W(%d), H(%d)", 
        mSensorResolution.SensorPreviewWidth,
        mSensorResolution.SensorPreviewHeight,
        mSensorResolution.SensorFullWidth,
        mSensorResolution.SensorFullHeight);

    LOG_MSG("Sensor 3D resolution, Preview: W(%d), H(%d), Full: W(%d), H(%d)", 
        mSensorResolution.Sensor3DPreviewWidth,
        mSensorResolution.Sensor3DPreviewHeight,
        mSensorResolution.Sensor3DFullWidth,
        mSensorResolution.Sensor3DFullHeight);
    //
    /*
        According to "MT6575 ISP Hardware Limitation.xls"
        when CAM_PATH.OUTPATH_EN = 1:
        bayer8:   (width*height)%8=0
        bayer10:  (width*height)%6=0
        yuv444:   (width*height)%2=0
        yuv422:   (width*height)%4=0
        */
    if(IMAGE_SENSOR_TYPE_RAW == mpSensorDrv->getCurrentSensorType())
    {
        MUINT32 u4PaddedWidth, u4PaddedHeight;
        //  Full Resolution
        u4PaddedWidth = mSensorResolution.SensorFullWidth + ISP_HAL_RAW_WIDTH_PADD;
        u4PaddedHeight = mSensorResolution.SensorFullHeight + ISP_HAL_RAW_HEIGHT_PADD;
        if(((u4PaddedWidth * u4PaddedHeight) % 6) != 0)
        {
            mSensorResolution.SensorFullHeight -= (u4PaddedHeight % 6);
            LOG_MSG("Sensor resolution after fixing:Full:W(%d), H(%d)",
                mSensorResolution.SensorFullWidth,
                mSensorResolution.SensorFullHeight);
        }
        //
        u4PaddedWidth = mSensorResolution.Sensor3DFullWidth + ISP_HAL_RAW_WIDTH_PADD;
        u4PaddedHeight= mSensorResolution.Sensor3DFullHeight + ISP_HAL_RAW_HEIGHT_PADD;
        if(((u4PaddedWidth * u4PaddedHeight) % 6) != 0)
        {
            mSensorResolution.Sensor3DFullHeight -= (u4PaddedHeight % 6);
            LOG_MSG("Sensor 3D resolution after fixing:Full:W(%d), H(%d)",
                mSensorResolution.Sensor3DFullWidth,
                mSensorResolution.Sensor3DFullHeight);
        }
    }
    //
    return ret;
}
//-----------------------------------------------------------------------------
MINT32 IspHalImp::getSensorInfo(MINT32 mode)
{
    MINT32 ret = 0;
    //
    LOG_MSG("mode(%d)",mode);
    //
    mSensorScenarioId = (ACDK_SCENARIO_ID_ENUM) mode;
    memset(&mSensorInfo, 0, sizeof(ACDK_SENSOR_INFO_STRUCT));
    memset(&mSensorCfg, 0, sizeof(ACDK_SENSOR_CONFIG_STRUCT));
    //
    LOG_MSG("getInfo +");
    ret = mpSensorDrv->getInfo(mSensorScenarioId, &mSensorInfo, &mSensorCfg);
	//
    if(ret < 0)
    {
        LOG_ERR("getSensorInfo fail");
        return ret;       
    }
    //
    LOG_MSG("getCurrentSensorType +");
    mImageSensorType = mpSensorDrv->getCurrentSensorType();
    switch (mImageSensorType)
    {
        case IMAGE_SENSOR_TYPE_RAW:
        {
            mIspSensorType = ISP_HAL_SENSOR_TYPE_RAW;    
            break;
        }
        case IMAGE_SENSOR_TYPE_YUV:
        case IMAGE_SENSOR_TYPE_YCBCR:
        {
            mIspSensorType = ISP_HAL_SENSOR_TYPE_YUV;
            break;
        }
        default:
        {
            mIspSensorType = ISP_HAL_SENSOR_TYPE_UNKNOWN;
            ret = -EINVAL;
            LOG_ERR("Unsupport Sensor Type");
            break;
        }
    }
    //
    LOG_MSG("getSensorInfo -");
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspHalImp::setTgPhase(void)
{
    MINT32 ret = 0;
    MUINT32 u4PadPclkInv = 0;
    MINT32 clkInKHz, clkCnt, mclk, mclkSel;
    //
    LOG_MSG("clk(%d)", mSensorInfo.SensorClockFreq);
    //
    clkInKHz = mSensorInfo.SensorClockFreq * 1000;     
    if((clkInKHz < 3250) || (clkInKHz >= 104000))
    {
        LOG_ERR("Err-Input clock rate error, clkInKHz(%d)", clkInKHz);
        return -EINVAL;
    }
    //
    if((clkInKHz % 48) == 0)
    {
        // Clock is in 48MHz group, original source is 120.3MHz
        mclk = 120250;
        mclkSel = ISP_DRV_CAM_PLL_48_GROUP;
    }
    else
    {
        // Clock is in 52MHz group
        mclk = 104000;//52000;
        mclkSel = ISP_DRV_CAM_PLL_52_GROUP;
    }
    //
    #if 0   // FIXME, isp driver control for camera pll not ready
    mclk = 122880;
    mclkSel = 0;
    #endif
    //
    clkCnt = (mclk + (clkInKHz >> 1)) / clkInKHz;
    // Maximum CLKCNT is 15
    clkCnt = clkCnt > 15 ? 15 : clkCnt;
    LOG_MSG("mclk(%d), clkCnt(%d)", mclk, clkCnt);
    //  Get the setting of Pixel Clock Inverse in PAD side.
    switch (mSensorDev)
    {
        case ISP_HAL_SENSOR_DEV_MAIN:
        {
            ret = mpSensorDrv->sendCommand(CMD_SENSOR_GET_PAD_PCLK_INV, &u4PadPclkInv, (MUINT32*)SENSOR_MAIN);
            break;
        }
        case ISP_HAL_SENSOR_DEV_SUB:
        {
            ret = mpSensorDrv->sendCommand(CMD_SENSOR_GET_PAD_PCLK_INV, &u4PadPclkInv, (MUINT32*)SENSOR_SUB);
            break;
        }
        default:
        {
            u4PadPclkInv = 0;
            ret = 0;
            break;
        }
    }
    if(ret < 0)
    {
        LOG_ERR("CMD_SENSOR_GET_PAD_PCLK_INV fail - err(%x)", ret);
    }
    LOG_MSG("u4PadPclkInv(%d)", u4PadPclkInv);
    // Config TG, always use Camera PLL, 1: 120.3MHz, 2: 52MHz
    #if 1
    ret = mpDrv->setTgPhaseCounter(
            1,
            mclkSel /*mSensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
            clkCnt,
            mSensorInfo.SensorClockPolarity ? 0 : 1,
            mSensorInfo.SensorClockFallingCount,
            mSensorInfo.SensorClockRisingCount,
            u4PadPclkInv);
    #else
    // For FPGA
    clkCnt = 0;
    ret = mpDrv->setTgPhaseCounter(
            1,
            0, /*mSensorInfo.SensorMasterClockSwitch,*/
            clkCnt,
            mSensorInfo.SensorClockPolarity ? 0 : 1,
            mSensorInfo.SensorClockFallingCount,
            mSensorInfo.SensorClockRisingCount,
            u4PadPclkInv);    
    #endif
    //
    if(ret < 0)
    {
        LOG_ERR("setTgPhaseCounter fail");
        return ret;
    }
    //
    return ret;
}


/*******************************************************
* Functionality : set TG2 phase
*******************************************************/
MINT32 IspHalImp::setTg2Phase(void)
{
    MINT32 ret = 0;
    MUINT32 u4PadPclkInv = 0;
    MINT32 clkInKHz, clkCnt, mclk, mclkSel;
 
    //=== output mclk for sensor open only ===

    LOG_MSG("clk(%d)", mSensorInfo.SensorClockFreq);

    clkInKHz = mSensorInfo.SensorClockFreq * 1000;
    
    if((clkInKHz < 3250) || (clkInKHz >= 104000))
    {
        LOG_ERR("Err-Input clock rate error, clkInKHz(%d)", clkInKHz);
        return -EINVAL;
    }
    
    if((clkInKHz % 48) == 0)
    {
        // Clock is in 48MHz group, original source is 120.3MHz
        mclk = 120250;
        mclkSel = ISP_DRV_CAM_PLL_48_GROUP;
    }
    else
    {
        // Clock is in 52MHz group
        mclk = 104000;  //52000;
        mclkSel = ISP_DRV_CAM_PLL_52_GROUP;
    }
    
    #if 0   // FIXME, isp driver control for camera pll not ready
    mclk = 122880;
    mclkSel = 0;
    #endif
    
    clkCnt = (mclk + (clkInKHz >> 1)) / clkInKHz;
    
    // Maximum CLKCNT is 15
    clkCnt = clkCnt > 15 ? 15 : clkCnt;
    LOG_MSG("mclk(%d), clkCnt(%d)", mclk, clkCnt);

    // === Config TG, always use Camera PLL, 1: 120.3MHz, 2: 52MHz ===
    
    //set  pinmux for TG2
    mpDrv->setIOConfig(1);
    
    ret = mpDrv->setTg2PhaseCounter(1, 
                                    mclkSel, /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/
                                    clkCnt, 
                                    mSensorInfo.SensorClockPolarity ? 0 : 1,
                                    mSensorInfo.SensorClockFallingCount,
                                    mSensorInfo.SensorClockRisingCount);
    
    return ret;
}


//----------------------------------------------------------------------------
MINT32 IspHalImp::getRawInfo(
    ISP_HAL_RAW_INFO_STRUCT*    pRawInfo,
    MINT32                      mode)
{
    MINT32 ret = 0;
    const char *porder[4] = {"B", "Gb", "Gr", "R"};
    //
    MINT32 W = 0, H = 0;
    //
    if(m3dType == ISP_HAL_3D_TYPE_B3D)
    {
        if(mode == 1)
        {
            // preview
            W = mSensorResolution.Sensor3DPreviewWidth;
            H = mSensorResolution.Sensor3DPreviewHeight;
        }
        else
        {
            // default or capture
            W = mSensorResolution.Sensor3DFullWidth;
            H = mSensorResolution.Sensor3DFullHeight;
        }
    }
    else
    {
        if(mode == 1)
        {
            // preview
            W = mSensorResolution.SensorPreviewWidth;
            H = mSensorResolution.SensorPreviewHeight;
        }
        else
        {
            // default or capture
            W = mSensorResolution.SensorFullWidth;
            H = mSensorResolution.SensorFullHeight;
        }
    }
    //
    memset(pRawInfo, 0, sizeof(ISP_HAL_RAW_INFO_STRUCT));
    switch (mIspSensorType)
    {
        case ISP_HAL_SENSOR_TYPE_RAW:
        {
            pRawInfo->u4BitDepth = 10; 
            pRawInfo->u4IsPacked = 1; 
            pRawInfo->u4Width = W + ISP_HAL_RAW_WIDTH_PADD; 
            pRawInfo->u4Height = H + ISP_HAL_RAW_HEIGHT_PADD; 
            pRawInfo->u4Size = (pRawInfo->u4Width * pRawInfo->u4Height * 4 + 2) / 3;  // 2 is for round up
            pRawInfo->u1Order = mSensorInfo.SensorOutputDataFormat;
            //if(mSensorInfo.SensorOutputDataFormat <= SENSOR_OUTPUT_FORMAT_RAW_R)
            //{
            //    strcpy(pRawInfo->u1Order, porder[mSensorInfo.SensorOutputDataFormat]);
            //}
            break;
        }
        case ISP_HAL_SENSOR_TYPE_YUV:
        {
            pRawInfo->u4BitDepth = 8; 
            pRawInfo->u4IsPacked = 0; 
            pRawInfo->u4Width = W;
            pRawInfo->u4Height = H; 
            pRawInfo->u4Size = pRawInfo->u4Width * pRawInfo->u4Height * 2;
            break;
        }
        default:
        {
            LOG_ERR("Unknow mIspSensorType(%s)",mIspSensorType);
            break;        
        }
    }
    // align size to 16x
    pRawInfo->u4Size = (pRawInfo->u4Size + 0xf) & (~0xf); 
    //
    return ret;   
}
//----------------------------------------------------------------------------
MINT32 IspHalImp::setCamMode(MINT32 mode)
{
    MINT32 ret = 0;
    #if ISP_HAL_TUNING_SUPPORT
    NSIspTuning::ECamMode_T eCamMode = NSIspTuning::ECamMode_Online_Preview;
    //
    LOG_MSG("mode(%d)", mode);
    //
    switch(mode)
    {
        case ISP_HAL_CAM_MODE_PREVIEW:
        {
            eCamMode = NSIspTuning::ECamMode_Online_Preview;
            break;
        }
        case ISP_HAL_CAM_MODE_VIDEO:
        {
            eCamMode = NSIspTuning::ECamMode_Video;
            break;
        }
        case ISP_HAL_CAM_MODE_CAPTURE_FLY:
        {
            eCamMode = NSIspTuning::ECamMode_Online_Capture;
            break;
        }
        case ISP_HAL_CAM_MODE_CAPTURE_FLY_ZSD:
        {
            eCamMode = NSIspTuning::ECamMode_Online_Capture_ZSD;
            break;
        }
#ifdef MTK_ZSD_AF_ENHANCE
        case ISP_HAL_CAM_MODE_PREVIEW_FLY_ZSD:
        {
            eCamMode = NSIspTuning::ECamMode_Online_Preview_ZSD;
            break;
        }
#endif
        case ISP_HAL_CAM_MODE_CAPTURE_PASS1:
        {
            eCamMode = NSIspTuning::ECamMode_Offline_Capture_Pass1;
            break;
        }
        case ISP_HAL_CAM_MODE_CAPTURE_PASS2:
        {
            eCamMode = NSIspTuning::ECamMode_Offline_Capture_Pass2;
            break;
        }
        case ISP_HAL_CAM_MODE_CAPTURE_HDR_PASS1_SF:
        {
            eCamMode = NSIspTuning::ECamMode_HDR_Cap_Pass1_SF;
            break;
        }
        case ISP_HAL_CAM_MODE_CAPTURE_HDR_PASS1_MF1:
        {
            eCamMode = NSIspTuning::ECamMode_HDR_Cap_Pass1_MF1;
            break;
        }
        case ISP_HAL_CAM_MODE_CAPTURE_HDR_PASS1_MF2:
        {
            eCamMode = NSIspTuning::ECamMode_HDR_Cap_Pass1_MF2;
            break;
        }
        case ISP_HAL_CAM_MODE_CAPTURE_HDR_PASS2:
        {
            eCamMode = NSIspTuning::ECamMode_HDR_Cap_Pass2;
            break;
        }
        case ISP_HAL_CAM_MODE_YUV2JPG_SCALADO:
        {
            eCamMode = NSIspTuning::ECamMode_YUV2JPG_Scalado;
            break;
        }
        case ISP_HAL_CAM_MODE_YUV2JPG_ZSD:
        {
            eCamMode = NSIspTuning::ECamMode_YUV2JPG_ZSD;
            break;
        }
        case ISP_HAL_CAM_MODE_FB_POSTPROCESS_NR2_ONLY:
        {
            eCamMode = NSIspTuning::ECamMode_FB_PostProcess_NR2_Only;
            break;
        }
        case ISP_HAL_CAM_MODE_FB_POSTPROCESS_PCA_ONLY:
        {
            eCamMode = NSIspTuning::ECamMode_FB_PostProcess_PCA_Only;
            break;
        }
        default:
        {
            eCamMode = NSIspTuning::ECamMode_Online_Preview;
            ret = -1;
            LOG_ERR("Unknow mode(%d)", mode);
            break;
        }
    }
    ret = mpIspTuningCurrent->setCamMode(eCamMode);
    if(ret < 0)
    {
        goto EXIT;
    }
    ret = mpIspTuningCurrent->validate();
    if( 0 > ret && NSIspTuning::MERR_UNSUPPORT != (MUINT32)ret)
    {
        LOG_ERR("validate fail - ec(%x)", ret);
        goto EXIT;
    }
    //
    ret = 0;
    EXIT:
    #endif
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspHalImp::sendCommand(
    MINT32      cmd,
    MINT32      arg1,
    MINT32      arg2,
    MINT32      arg3)
{
    MINT32 ret = 0;
    //LOG_MSG("cmd(0x%x)", cmd);
    //
    switch (cmd)
    {
        case ISP_HAL_CMD_RESET:
        {
            ret = mpDrv->reset();
            if(ret < 0)
            {
                LOG_ERR("mpDrv->reset() fail");
            }
            break;
        }
        case ISP_HAL_CMD_SET_SENSOR_DEV:
        {
            LOG_MSG("Sensor Dev(%d)", arg1);
            mSensorDev = arg1;

            if(mSensorDev == (ISP_HAL_SENSOR_DEV_MAIN | ISP_HAL_SENSOR_DEV_MAIN_2)) 
            {
                m3dType = ISP_HAL_3D_TYPE_N3D;
                mSensorDev = ISP_HAL_SENSOR_DEV_MAIN;
            }
       
            break;
        }
        case ISP_HAL_CMD_SET_SENSOR_GAIN:
        {
            mpSensorDrv->sendCommand(CMD_SENSOR_SET_GAIN, (MUINT32 *) arg1);
            break;
        }
        case ISP_HAL_CMD_SET_SENSOR_EXP:
        {
            mpSensorDrv->sendCommand(CMD_SENSOR_SET_EXP_TIME, (MUINT32 *) arg1);
            break;
        }
        case ISP_HAL_CMD_SET_CAM_MODE:
        {
            LOG_MSG("ISP_HAL_CMD_SET_CAM_MODE(%d)", arg1);    
            ret = setCamMode(arg1);
            break;
        }
        //
        #if ISP_HAL_TUNING_SUPPORT
        case ISP_HAL_CMD_SET_SCENE_MODE:
        {
            //LOG_MSG("ISP_HAL_CMD_SET_SCENE_MODE(%d)", *parg1);
            ret = mpIspTuningCurrent->setSceneMode(arg1);
            break;
        }
        case ISP_HAL_CMD_SET_ISO:
        {
            //LOG_MSG("ISP_HAL_CMD_SET_ISO(%d)", *parg1);
            ret = mpIspTuningCurrent->updateIso(arg1);
            break;
        }
        case ISP_HAL_CMD_SET_FLUORESCENT_CCT:
        {
            //LOG_MSG("ISP_HAL_CMD_SET_FLUORESCENT_CCT(%d),(%d)", arg1, arg2);
            ret = mpIspTuningCurrent->updateFluorescentAndCCT((MVOID *)arg1);
            break;
        }
        case ISP_HAL_CMD_SET_SCENE_LIGHT_VALUE:
        {
            //LOG_MSG("ISP_HAL_CMD_SET_SCENE_LIGHT_VALUE(%d)", arg1);
            ret = mpIspTuningCurrent->updateSceneLightValue(arg1);
            break;
        }
        case ISP_HAL_CMD_SET_VALIDATE_FRAME:
        {
            //LOG_MSG("ISP_HAL_CMD_SET_VALIDATE_FRAME");
            ret = mpIspTuningCurrent->validatePerFrame(arg1);
            break;
        }
        case ISP_HAL_CMD_SET_OPERATION_MODE:
        {
            LOG_MSG("ISP_HAL_CMD_SET_OPERATION_MODE(%d)", arg1);
            /*
                    ISP_HAL_OPER_MODE_NORMAL = 0,
                    ISP_HAL_OPER_MODE_PURE_RAW,
                    ISP_HAL_OPER_MODE_META,
                    ISP_HAL_OPER_MODE_CUSTOM,
                    */
            // Must be 1-1 mapping
            ret = mpIspTuningCurrent->setOperMode((EOperMode_T) arg1);
            break;
        }
        case ISP_HAL_CMD_SET_EFFECT:
        {
            //LOG_MSG("ISP_HAL_CMD_SET_EFFECT(%d)", arg1);
            ret = mpIspTuningCurrent->setEffect((MUINT32) arg1);
            break;
        }
        case ISP_HAL_CMD_SET_ZOOM_RATIO:
        {
            //LOG_MSG("ISP_HAL_CMD_SET_ZOOM_RATIO(%d)", arg1);
            ret = mpIspTuningCurrent->updateZoomRatio((MUINT32) arg1);
            break;
        }
        case ISP_HAL_CMD_SET_BRIGHTNESS:
        {
            //LOG_MSG("ISP_HAL_CMD_SET_BRIGHTNESS(%d)", arg1);
            ret = mpIspTuningCurrent->setIspUserIdx_Bright((MUINT32) arg1);
            break;
        }
        case ISP_HAL_CMD_SET_CONTRAST:
        {
            //LOG_MSG("ISP_HAL_CMD_SET_CONTRAST(%d)", arg1);
            ret = mpIspTuningCurrent->setIspUserIdx_Contrast((MUINT32) arg1);
            break;
        }
        case ISP_HAL_CMD_SET_EDGE:
        {
            //LOG_MSG("ISP_HAL_CMD_SET_EDGE(%d)", arg1);
            ret = mpIspTuningCurrent->setIspUserIdx_Edge((MUINT32) arg1);
            break;
        }
        case ISP_HAL_CMD_SET_HUE:
        {
            //LOG_MSG("ISP_HAL_CMD_SET_HUE(%d)", arg1);
            ret = mpIspTuningCurrent->setIspUserIdx_Hue((MUINT32) arg1);
            break;
        }
        case ISP_HAL_CMD_SET_SATURATION:
        {
            //LOG_MSG("ISP_HAL_CMD_SET_SATURATION(%d)", arg1);
            ret = mpIspTuningCurrent->setIspUserIdx_Sat((MUINT32) arg1);
            break;
        }
        case ISP_HAL_CMD_SET_TUNING_CMD:
        {
            ret = mpIspTuningCurrent->sendCommand((MVOID *) arg1);
            break;
        }
        case ISP_HAL_CMD_SET_OFFLINE_CAPTURE:
        {
            CmdArg_T cmd;
            cmd.eCmd = ECmd_DecideOfflineCapture;
            cmd.pOutBuf = reinterpret_cast<MVOID*>(arg1);
            cmd.u4OutBufSize = sizeof(MINT32);
            ret = mpIspTuningCurrent->sendCommand((MVOID *) &cmd);
            break;
        }
        case ISP_HAL_CMD_SET_LOCK_REG:
        {
            ret = (NULL != mpDrv) ? mpDrv->holdReg(arg1) : -1;
            break;
        }
        case ISP_HAL_CMD_SET_SHADING_IDX:
        {
            ret = mpIspTuningCurrent->setShadingIndex(arg1);
            break;        
        }
        #endif
        case ISP_HAL_CMD_SET_CAP_DELAY_FRAME:
        {
            ret = mpDrv->sendCommand(ISP_DRV_CMD_SET_CAPTURE_DELAY,arg1,0,0);
            if(ret < 0)
            {
                LOG_ERR("mpDrv->setCapDelayFrame() fail");
            }
            break;        
        }
        //
        case ISP_HAL_CMD_SET_SENSOR_WAIT:
        {
            LOG_MSG("ISP_HAL_CMD_SET_SENSOR_WAIT:(%d,%d)",arg1,arg2);
            if(arg1<0 || arg2<0)
            {
                LOG_ERR("arg1(%d) and arg2(%d) should be larger than 0.",arg1,arg2);
                break;
            }
            if(arg1 < ISP_HAL_SENSOR_WAIT_AMOUNT)
            {
                ret = mpSensorDrv->waitSensorEventDone(arg1,arg2);
                if(ret < 0)
                {
                    LOG_ERR("mpSensorDrv->waitSensorEventDone() fail,SENSOR_WAIT(%d),Timeout(%d)",(ISP_HAL_SENSOR_WAIT_ENUM)arg1,arg2);
                }
            }
            else
            {
                LOG_ERR("Unknow SENSOR_WAIT(%d)",arg1);
                ret = -1;
            }
            break;        
        }
        //
        case ISP_HAL_CMD_GET_SENSOR_PRV_RANGE:
        {
            if(m3dType == ISP_HAL_3D_TYPE_B3D)
            {
                *(MINT32 *) arg1 = mSensorResolution.Sensor3DPreviewWidth;
                *(MINT32 *) arg2 = mSensorResolution.Sensor3DPreviewHeight;
            }
            else
            {
                *(MINT32 *) arg1 = mSensorResolution.SensorPreviewWidth;
                *(MINT32 *) arg2 = mSensorResolution.SensorPreviewHeight;
            }
            LOG_MSG("ISP_HAL_CMD_GET_SENSOR_PRV_RANGE:m3dType(%d),W(%d),H(%d)",(int)m3dType,*(MINT32*)arg1,*(MINT32*)arg2);
            break;
        }
        case ISP_HAL_CMD_GET_SENSOR_FULL_RANGE:
        {
            if(m3dType == ISP_HAL_3D_TYPE_B3D)
            {
                *(MINT32 *) arg1 = mSensorResolution.Sensor3DFullWidth;
                *(MINT32 *) arg2 = mSensorResolution.Sensor3DFullHeight;
            }
            else
            {
                *(MINT32 *) arg1 = mSensorResolution.SensorFullWidth;
                *(MINT32 *) arg2 = mSensorResolution.SensorFullHeight;
            }
            LOG_MSG("ISP_HAL_CMD_GET_SENSOR_FULL_RANGE:m3dType(%d),W(%d),H(%d)",(int)m3dType,*(MINT32*)arg1,*(MINT32*)arg2);
            break;
        }
        case ISP_HAL_CMD_GET_RAW_DUMMY_RANGE:
        {
            LOG_MSG("ISP_HAL_CMD_GET_RAW_DUMMY_RANGE");
            *(MINT32 *) arg1 = ISP_HAL_RAW_WIDTH_PADD;
            *(MINT32 *) arg2 = ISP_HAL_RAW_HEIGHT_PADD;
            break;
        }
        case ISP_HAL_CMD_GET_SENSOR_TYPE:
        {
            *(MINT32 *) arg1 = mIspSensorType;
            break;
        }
        case ISP_HAL_CMD_GET_RAW_INFO:
        {
            ret = getRawInfo((ISP_HAL_RAW_INFO_STRUCT *) arg1, (MINT32)arg2);
            break;
        }
        //
        #if ISP_HAL_TUNING_SUPPORT
        case ISP_HAL_CMD_GET_EXIF_DEBUG_INFO:
        {
            ret = mpIspTuningCurrent->queryExifDebugInfo(*(NSIspExifDebug::IspExifDebugInfo_T *) arg1);
            *(MINT32 *) arg2 = sizeof(NSIspExifDebug::IspExifDebugInfo_T);
            break;
        }
        case ISP_HAL_CMD_GET_SHADING_IDX:
        {
            ret = mpIspTuningCurrent->getShadingIndex((MVOID *)arg1);
            break;
        }
        case ISP_HAL_CMD_GET_ATV_DISP_DELAY:
        {
            mpSensorDrv->sendCommand(CMD_SENSOR_GET_ATV_DISP_DELAY, (MUINT32 *) arg1);
            break;
        }
        #endif
        //
        case ISP_HAL_CMD_GET_SENSOR_DELAY_FRAME_CNT:
        {
            MINT32 mode = arg1; 
            mpSensorDrv->sendCommand(CMD_SENSOR_GET_DELAY_FRAME_CNT, (MUINT32 *) &mode, (MUINT32 *) arg2); 
            break;
        }
        case ISP_HAL_CMD_GET_SENSOR_3D_CAP_TYPE:
        {
            *(MUINT32*)arg1 = (ISP_HAL_CAP_3D_TYPE_ENUM)(mSensorInfo.SensorDriver3D);
            break;
        }
        case ISP_HAL_CMD_GET_VD_COUNT:
        {
            ret = mpDrv->sendCommand(ISP_DRV_CMD_GET_VD_COUNT,arg1,0,0);
            if(ret < 0)
            {
                LOG_ERR("mpDrv->getVdCount() fail");
            }
            LOG_MSG("VdCount(%d)",*(MUINT32*)arg1);
            break;
        }
        case ISP_HAL_CMD_SET_FREE_M4U_FLAG:
        {
            LOG_MSG("ISP_HAL_CMD_SET_FREE_M4U_FLAG(%d)", arg1);
            mbFreeM4U = (MBOOL)arg1;
            
#if ISP_HAL_M4U_SUPPORT
            if(mbFreeM4U)
            {
                freeM4UPort();
            }
#endif
            break;
        }
        case ISP_HAL_CMD_SET_SENSOR_RESTART:
        {
            LOG_MSG("Restart sensor!");

            ret = initCSI2Peripheral(0);  // if the interface is mipi, disable the csi2
            if(ret < 0)
    	      {
             LOG_ERR("initial[0] CSI2 peripheral fail");
    	      }
            mpDrv->writeReg(0x0010,mpDrv->readReg(0x0010)^0x0030);
			      mpSensorDrv->close();
            mpSensorDrv->uninit();
      
            //mpDrv->writeReg(0x0010,reg);
            //mpDrv->setMCLKEn(1);
			      ret = initSensor();
    	      ret = initCSI2Peripheral(1);  // if the interface is mipi, enable the csi2
    	      if(ret < 0)
    	      {
        	      LOG_ERR("initial CSI2 peripheral fail");
    	      }
    	  
    	      ret = setCSI2Config(1);     // enable and config CSI2.
			      for (int i =0; i < 3; i++) 
			      {
	        	    ret = mpSensorDrv->open();
	        	    if (ret < 0) {
            		    LOG_ERR("pSensorDrv->open fail, retry = %d", i);
        		    }
        		    else 
				        {
            		    break; 
        		    }
    	      }
    		    break;
        }
        default:
        {
            ret = -1;
            LOG_MSG("Unknow cmd(0x%X)",cmd);
            break;
        }
    }
    //
    #if ISP_HAL_TUNING_SUPPORT
    if((MUINT32) ret == NSIspTuning::MERR_UNSUPPORT)
    {
        if(mIspSensorType == ISP_HAL_SENSOR_TYPE_YUV)
        {
            // FIXME: Should be defined by cmd
            ret = 0;
        }
    }
    #endif
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspHalImp::initCSI2Peripheral(MINT32 initCSI2)
{
    MINT32 ret = 0;
    //
    if(mSensorInfo.SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI)
    {
        // enable peripheral
        if(initCSI2)
        {
            ret = mpDrv->initCSI2(1);
            if(ret < 0)
            {
                LOG_ERR("init CSI2 peripheral fail");
            }
        }
        else
        {
            ret = mpDrv->initCSI2(0);
            if(ret < 0)
            {
                LOG_ERR("uninit CSI2 peripheral fail");
            }
        }
    }
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspHalImp::setCSI2Config(MINT32 enableCSI2)
{
    MINT32 ret = 0;
    //
    if((enableCSI2 ==  1) && (mSensorInfo.SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI))
    {
        // enable csi2
        // Set mipi csi2         
        LOG_MSG("Enable");
        ret = mpDrv->setCSI2(
                mSensorInfo.MIPIDataLowPwr2HighSpeedTermDelayCount,
                mSensorInfo.MIPIDataLowPwr2HighSpeedSettleDelayCount, 
                mSensorInfo.MIPICLKLowPwr2HighSpeedTermDelayCount,
                mSensorInfo.SensorVsyncPolarity,
                mSensorInfo.SensorMIPILaneNumber,
                enableCSI2,
                mSensorInfo.SensorPacketECCOrder,
                0);
        if(ret < 0)
        {
            LOG_ERR("Enable fail");
        }
    }
    else
    {
        LOG_MSG("Disable");
        ret = mpDrv->setCSI2(0, 0, 0, 0, 0, 0, 0, 0);
        if(ret < 0)
        {
            LOG_ERR("Disable fail");
        }
    }
    //
    return ret;
}
//----------------------------------------------------------------------------
MINT32 IspHalImp::setSensorIODrivingCurrent(void)
{
    MINT32 ret = 0;
    MINT32 increaseDivingCurrent = 0x08; // set to default 2mA and slew raw control
    MINT32 drivingCurrentFlag;
    MINT32 fp,size;                  
    MINT32 hqaDrivingCurrent = -1;   
    char   readBuffer[2] = {0, '\0'};   // Fix klocwork warning "Possible attempt to access element 1 of array readBuffer". The input of atoi() is string, so size needs to be 2.
    

    fp = open(ISP_HAL_MCLK_DRIVING_CURRENT_SETTING_FILE, O_RDONLY, 0);
    
    if(fp < 0)
    {
        LOG_MSG("mclkdriving cannot be opend. errno(%d):%s",errno,strerror(errno));
    }
    else
    {
        size = read(fp, readBuffer, 1);
        hqaDrivingCurrent = atoi(readBuffer);
        close(fp);
        LOG_MSG("EM has set new driving current value");
    }

    LOG_MSG("hqaDrivingCurrent=%d,mSensorInfo.SensorDrivingCurrent=%d",hqaDrivingCurrent,mSensorInfo.SensorDrivingCurrent);
    
    if(hqaDrivingCurrent == mSensorInfo.SensorDrivingCurrent || hqaDrivingCurrent == -1)   
    {
        drivingCurrentFlag = mSensorInfo.SensorDrivingCurrent;
    }
    else
    {
        drivingCurrentFlag = hqaDrivingCurrent;
    }
    
    switch(drivingCurrentFlag)
    {
        case ISP_DRIVING_2MA:
        {
            increaseDivingCurrent = 0x08;
            break;
        }
        case ISP_DRIVING_4MA:
        {
            increaseDivingCurrent = 0x0A;
            break;
        }
        case ISP_DRIVING_6MA:
        {
            increaseDivingCurrent = 0x09;
            break;
        }
        case ISP_DRIVING_8MA:
        {
            increaseDivingCurrent = 0x0B;
            break;
        }
        default:
        {
            LOG_MSG("The driving current value is wrong");
            break;
        }
    }

    LOG_MSG("increaseDivingCurrent=%d",increaseDivingCurrent);
    
    
    ret = mpDrv->setIODrivingCurrent(increaseDivingCurrent);
    if(ret < 0)
    {
        LOG_ERR("The driving current setting is wrong");
    }
    //
    if(m3dType == ISP_HAL_3D_TYPE_N3D)
    {
        ret = mpDrv->setTG2IODrivingCurrent(increaseDivingCurrent);
        if(ret < 0)
        {
            LOG_ERR("The TG2 driving current setting is wrong");
        }
    }
    //
    return ret;
}
//----------------------------------------------------------------------------
#if ISP_HAL_M4U_SUPPORT
MINT32 IspHalImp::allocM4UMemory(
    MUINT32     virtAddr,
    MUINT32     size,
    MUINT32*    m4uVa)
{
    AutoCPTLog cptlog(Event_IspHal_allocM4UMemory);
    MINT32 ret = 0;
    //
    if(mpM4UDrv == NULL)
    {
        LOG_MSG("Null M4U driver"); 
        return -1;
    }
    //
    LOG_MSG("virtAddr(0x%x), size(%d)", virtAddr, size); 
    //
    #if(ISP_HAL_PROFILE) 
    MINT32 startTime = 0, endTime = 0; 
    struct timeval tv;
    gettimeofday(&tv, NULL);
    startTime = tv.tv_sec * 1000000 + tv.tv_usec;
    #endif 
    //
    M4U_MODULE_ID_ENUM eM4U_ID = M4U_CLNTMOD_DEFECT;
    M4U_PORT_STRUCT port;
    ret = mpM4UDrv->m4u_power_on(eM4U_ID); 
    if(ret != M4U_STATUS_OK)
    { 
        LOG_ERR("m4u_power_on fail"); 
        goto EXIT;         
    }
    //
    ret = mpM4UDrv->m4u_alloc_mva(
            eM4U_ID,
            virtAddr,
            size,
            m4uVa); 
    if(ret != M4U_STATUS_OK)
    {
        LOG_ERR("m4u_alloc_mva fail"); 
        goto EXIT; 
    }
    //set lock to fals, due to camera only use memory once 
    ret = mpM4UDrv->m4u_manual_insert_entry(
            eM4U_ID,
            *m4uVa,
            MFALSE);
    if(ret != M4U_STATUS_OK)
    {
        LOG_ERR("m4u_manual_insert_entry fail"); 
        goto EXIT;         
    }
    //
    ret = mpM4UDrv->m4u_insert_tlb_range(
            eM4U_ID,
            *m4uVa,
            *m4uVa + size -1,
            RT_RANGE_HIGH_PRIORITY,
            1);
    if(ret != M4U_STATUS_OK)
    {
        LOG_ERR("m4u_insert_tlb_range fail"); 
        goto EXIT;         
    }
    //
    LOG_MSG("m4uVa(0x%x)", *m4uVa); 
    //
    port.ePortID = M4U_PORT_DEFECT; 
    port.Virtuality = 1;
    port.Security = 0;
    port.Distance = 1;
    port.Direction = 0;
    ret = mpM4UDrv->m4u_config_port(&port);
    
    LOG_MSG("M4U Flush all"); 
    mpM4UDrv->m4u_cache_flush_all(eM4U_ID);
    
    //
    #if(ISP_HAL_PROFILE) 
    gettimeofday(&tv, NULL);
    endTime = tv.tv_sec * 1000000 + tv.tv_usec;
    LOG_MSG("profile time(%d) ms", (endTime - startTime) / 1000); 
    #endif 
    //
    EXIT:
    return ret; 
}
//----------------------------------------------------------------------------
MINT32 IspHalImp::freeM4UMemory(
    MUINT32     virtAddr,
    MUINT32     m4uVa,
    MUINT32     size)
{
    AutoCPTLog cptlog(Event_IspHal_freeM4UMemory);
    MINT32 ret = 0;
    //
    if(mpM4UDrv == NULL)
    {
        LOG_MSG("Null M4U driver"); 
        return -1;
    } 
    //
    LOG_MSG("va(0x%x), m4uVa(0x%x), size(%d)", virtAddr, m4uVa, size); 
    //
    #if(ISP_HAL_PROFILE) 
    MINT32 startTime = 0, endTime = 0; 
    struct timeval tv;
    gettimeofday(&tv, NULL);
    startTime = tv.tv_sec * 1000000 + tv.tv_usec;
    #endif
    //
    M4U_PORT_STRUCT port;
    ret = mpM4UDrv->m4u_invalid_tlb_range(
            M4U_CLNTMOD_DEFECT,
            m4uVa,
            m4uVa + size-1);
    ret = mpM4UDrv->m4u_dealloc_mva(
            M4U_CLNTMOD_DEFECT,
            virtAddr,
            size,
            m4uVa);
    port.ePortID = M4U_PORT_DEFECT;
    port.Virtuality = 0;
    ret = mpM4UDrv->m4u_config_port(&port);
    //
    #if(ISP_HAL_PROFILE) 
    gettimeofday(&tv, NULL);
    endTime = tv.tv_sec * 1000000 + tv.tv_usec;
    LOG_MSG("profile time(%d) ms", (endTime - startTime) / 1000); 
    #endif
    //
    ret = mpM4UDrv->m4u_power_off(M4U_CLNTMOD_DEFECT); 
    if(ret != M4U_STATUS_OK)
    {
        LOG_ERR("m4u_power_on fail"); 
    }
    //
    return ret; 
}

void IspHalImp::freeM4UPort()
{
    if(mIspReadPort.useM4U)
    {
        if(mbFreeM4U)
        {   
            LOG_MSG("free M4U ISP read port memory"); 
            freeM4UMemory(mIspReadPort.virtAddr, mIspReadPort.M4UVa, mIspReadPort.size);
        }
        else
        {
            LOG_MSG("Do not free M4U ISP read port memory");
            mIspM4UPort.virtAddr = mIspReadPort.virtAddr; 
            mIspM4UPort.useM4U = mIspReadPort.useM4U; 
            mIspM4UPort.M4UVa = mIspReadPort.M4UVa; 
            mIspM4UPort.size = mIspReadPort.size; 
        }
        
        mIspReadPort.useM4U = 0; 
    }
    //
    if(mIspWritePort.useM4U)
    {
        if(mbFreeM4U)
        {
            LOG_MSG("free M4U ISP write port memory");         
            freeM4UMemory(mIspWritePort.virtAddr, mIspWritePort.M4UVa, mIspWritePort.size); 
        }
        else
        {
            LOG_MSG("Do not free M4U ISP write port memory");         
            mIspM4UPort.virtAddr = mIspWritePort.virtAddr; 
            mIspM4UPort.useM4U = mIspWritePort.useM4U; 
            mIspM4UPort.M4UVa = mIspWritePort.M4UVa; 
            mIspM4UPort.size = mIspWritePort.size; 
        }
        mIspWritePort.useM4U = 0; 
    }

    if(mIspM4UPort.useM4U)
    {
        if(mbFreeM4U)
        {
            LOG_MSG("free reserved M4U ISP port memory");
            freeM4UMemory(mIspM4UPort.virtAddr, mIspM4UPort.M4UVa, mIspM4UPort.size); 
            memset(&mIspM4UPort,0,sizeof(ISP_HAL_M4U_STRUCT));
        }
    }

}

//----------------------------------------------------------------------------
#endif

