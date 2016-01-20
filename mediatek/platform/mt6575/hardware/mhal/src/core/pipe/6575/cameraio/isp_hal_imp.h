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
#ifndef ISP_HAL_IMP_H
#define ISP_HAL_IMP_H
//----------------------------------------------------------------------------
#include "isp_hal.h"
#include <cutils/xlog.h>
#include "isp_drv.h"
#include "sensor_drv.h"
#include "mcu_drv.h"
//----------------------------------------------------------------------------
using namespace android;
//----------------------------------------------------------------------------
#define LOG_MSG(fmt, arg...)    XLOGD("[%s]"fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    XLOGW("[%s]Warning(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    XLOGE("[%s]Err(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
//----------------------------------------------------------------------------
#define ISP_HAL_PROFILE             (0)
#define ISP_HAL_TUNING_SUPPORT      (1)
//----------------------------------------------------------------------------
#if defined(MTK_M4U_SUPPORT)
#include "m4u_lib.h"
#define ISP_HAL_M4U_SUPPORT         (1)
#else
#define ISP_HAL_M4U_SUPPORT         (0)
#endif
//
#if ISP_HAL_TUNING_SUPPORT
#include "isp_tuning_hal.h"
using namespace NSIspTuning;
#endif 
//----------------------------------------------------------------------------
#define ISP_HAL_RAW_WIDTH_PADD      (4)
#define ISP_HAL_RAW_HEIGHT_PADD     (6)
//----------------------------------------------------------------------------
#define ISP_HAL_MCLK_DRIVING_CURRENT_SETTING_FILE   "/data/data/com.mediatek.internalem/sharefile/mclkdriving"
#define ISP_HAL_STEREO_3D_SUPPORT       "Camera.Stereo.3D.Support"
#define ISP_HAL_STEREO_3D_SUPPORT_N     "0"
#define ISP_HAL_STEREO_3D_SUPPORT_Y     "1"
#define ISP_HAL_STEREO_3D_ENABLE        "Camera.Stereo.3D.Enable"
#define ISP_HAL_STEREO_3D_ENABLE_N      "0"
#define ISP_HAL_STEREO_3D_ENABLE_Y      "1"
//----------------------------------------------------------------------------
#if ISP_HAL_M4U_SUPPORT
typedef struct
{
    MUINT32 virtAddr; 
    MUINT32 useM4U; 
    MUINT32 M4UVa; 
    MUINT32 size; 
}ISP_HAL_M4U_STRUCT;
#endif

typedef enum
{
    ISP_HAL_3D_TYPE_NONE,
    ISP_HAL_3D_TYPE_B3D,
    ISP_HAL_3D_TYPE_N3D
}ISP_HAL_3D_TYPE_ENUM;

//----------------------------------------------------------------------------
class IspHalImp : public IspHal
{
    private:
        IspHalImp();
        virtual ~IspHalImp();
    //
    public:
        static IspHal* getInstance(void);
        virtual void destroyInstance(void);
        virtual MINT32 searchSensor(void);
        virtual MINT32 init(void);
        virtual MINT32 uninit(void);
        virtual MINT32 start(void);
        virtual MINT32 stop(void);
        virtual MINT32 setConf(ISP_HAL_CONFIG_STRUCT *pConfig);
        virtual MINT32 setConfPQ(ISP_HAL_CONFIG_PQ_STRUCT* pConfig);
        virtual MINT32 waitDone(MINT32 mode,MINT32 Timeout = ISP_DRV_IRQ_TIMEOUT_DEFAULT);
        virtual MINT32 waitIrq(ISP_HAL_WAIT_IRQ_STRUCT* pWaitIrq);
        virtual MINT32 sendCommand(
            MINT32      cmd,
            MINT32      arg1 = 0,
            MINT32      arg2 = 0,
            MINT32      arg3 = 0);
        virtual MINT32 dumpReg(void);
    //
    private:
        MINT32 initSensor(void);
        MINT32 openSensor(void);
        MINT32 getSensorInfo(MINT32 mode);
        MINT32 setTgPhase(void);
        MINT32 setTg2Phase(void);   
        MINT32 initCSI2Peripheral(MINT32 initCSI2);
        MINT32 setCSI2Config(MINT32 enableCSI2);
        MINT32 setSensorIODrivingCurrent(void);
        MINT32 getRawInfo(
            ISP_HAL_RAW_INFO_STRUCT*    pRawInfo,
            MINT32                      mode = 0);
        MINT32 setCamMode(MINT32 mode);
        //
        IspDrv* mpDrv;
        SensorDrv*  mpSensorDrv;
        MCUDrv* mpMcuDrv;
        MINT32 mSensorDev;
        MINT32 mImageSensorType;
        ISP_HAL_SENSOR_TYPE_ENUM mIspSensorType;
        ACDK_SCENARIO_ID_ENUM mSensorScenarioId;
        ACDK_SENSOR_INFO_STRUCT mSensorInfo;
        ACDK_SENSOR_CONFIG_STRUCT mSensorCfg;
        ACDK_SENSOR_RESOLUTION_INFO_STRUCT mSensorResolution;
        mutable Mutex mLock;
        volatile int mUsers;
        ISP_HAL_3D_TYPE_ENUM m3dType; 
        //
        #if ISP_HAL_TUNING_SUPPORT
        IspTuningHal*   mpIspTuningObj[6];
        IspTuningHal*   mpIspTuningCurrent;
        #endif
        //
        #if ISP_HAL_M4U_SUPPORT
        MTKM4UDrv*  mpM4UDrv;
        ISP_HAL_M4U_STRUCT mIspReadPort;
        ISP_HAL_M4U_STRUCT mIspWritePort; 
		ISP_HAL_M4U_STRUCT mIspM4UPort;
		MBOOL mbFreeM4U;
        MINT32 allocM4UMemory(
            MUINT32     virtAddr,
            MUINT32     size,
            MUINT32*    m4uVa); 
        MINT32 freeM4UMemory(
            MUINT32     virtAddr,
            MUINT32     m4uVa,
            MUINT32     size);
		
        void freeM4UPort();
		
        #endif
};
//----------------------------------------------------------------------------
#endif

