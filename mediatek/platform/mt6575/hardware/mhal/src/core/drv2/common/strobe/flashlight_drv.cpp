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
 
#define LOG_TAG "FlashlightDrv"
#include <utils/Errors.h>
#include <cutils/xlog.h>
#include <fcntl.h>

//
#include "ae_feature.h"
#include "../inc/strobe_drv.h"
#include "flashlight_drv.h"
#include "kd_flashlight.h"


/*******************************************************************************
*
********************************************************************************/
#define STROBE_DEV_NAME    "/dev/kd_camera_flashlight"

#define DEBUG_STROBE_DRV
#ifdef  DEBUG_STROBE_DRV
#define DRV_DBG(fmt, arg...)  XLOGD( fmt, ##arg)
#define DRV_ERR(fmt, arg...)   XLOGE("Err: %5d:, "fmt, __LINE__, ##arg)
#else
#define DRV_DBG(a,...)
#define DRV_ERR(a,...)
#endif


/*******************************************************************************
*
********************************************************************************/
StrobeDrv* FlashlightDrv::getInstance()
{
    static FlashlightDrv singleton;
    return &singleton;
}


/*******************************************************************************
*
********************************************************************************/
void FlashlightDrv::destroyInstance()
{
}


/*******************************************************************************
*
********************************************************************************/
StrobeDrv::FLASHLIGHT_TYPE_ENUM FlashlightDrv::getFlashlightType() const
{
    int err = 0;
    
    if (m_fdSTROBE < 0)
    {
        DRV_DBG(" [getFlashlightType] m_fdSTROBE < 0\n");
        return StrobeDrv::FLASHLIGHT_NONE;
    }

    DRV_DBG("[getFlashlightType] m_flashType=%d\n",m_flashType);
    err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_G_FLASHTYPE,&m_flashType);
    if (err < 0)
    {
        DRV_ERR("FLASHLIGHTIOC_G_FLASHTYPE error:%d\n",m_flashType);
        return StrobeDrv::FLASHLIGHT_NONE;;
    }
    return (StrobeDrv::FLASHLIGHT_TYPE_ENUM)m_flashType;
}


/*******************************************************************************
*
********************************************************************************/   
FlashlightDrv::FlashlightDrv()
	  : StrobeDrv()
    , m_fdSTROBE(-1)
    , m_flashType((int)StrobeDrv::FLASHLIGHT_NONE)
    , m_strobeMode(LIB3A_AE_STROBE_MODE_UNSUPPORTED)
    , mUsers(0)
{
    DRV_DBG("FlashlightDrv()\n");  
}


/*******************************************************************************
*
********************************************************************************/  
FlashlightDrv::~FlashlightDrv()
{   
	
}


/*******************************************************************************
*
********************************************************************************/
int FlashlightDrv::init(unsigned long sensorDev)
{
    int err = 0;
    unsigned long flashlightIdx = 0; //default dummy driver

    //MHAL_LOG("[halSTROBEInit]: %s \n\n", __TIME__);
    DRV_DBG("[init] mUsers = %d\n", mUsers); 
    Mutex::Autolock lock(mLock); 
    if (mUsers == 0)
    {       
        if (m_fdSTROBE == -1)
        {
            m_fdSTROBE = open(STROBE_DEV_NAME, O_RDWR);
            if (m_fdSTROBE < 0)
            {
                DRV_ERR("error opening %s: %s", STROBE_DEV_NAME, strerror(errno));
                 return StrobeDrv::STROBE_UNKNOWN_ERROR;
            }

            //set flashlight driver
            DRV_DBG("[init] sensorDev = %ld\n", sensorDev); 
            //TODO: support sub flashlight
            if (1 == sensorDev) //main sensor
            { 
                flashlightIdx = 1; //custom driver
            }
            err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_X_SET_DRIVER,flashlightIdx);
            if (err < 0)
            {
                DRV_ERR("FLASHLIGHTIOC_X_SET_DRIVER error\n");
                return err ;
            }
        }
    }
    android_atomic_inc(&mUsers); 
    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/   
int FlashlightDrv::uninit()
{
    //MHAL_LOG("[halSTROBEUninit] \n");
    DRV_DBG("[uninit] mUsers = %d\n", mUsers);

    Mutex::Autolock lock(mLock); 
    
    if (mUsers == 0)
    {
        DRV_DBG("[uninit] mUsers = %d\n", mUsers); 
    }
    
     if (mUsers == 1)
     {
        if (m_fdSTROBE > 0)
        {          
            close(m_fdSTROBE);            
        }
        m_fdSTROBE = -1;
    }
    
    android_atomic_dec(&mUsers);    
    
    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/  
int FlashlightDrv::setFire(unsigned long a_fire)
{
    int err = 0;
    unsigned long  fire = 0;
    
    if (m_fdSTROBE < 0)
    {
        DRV_DBG(" [setFire] m_fdSTROBE < 0\n");
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }

    DRV_DBG("[FlashlightDrv] setFire = %lu\n",a_fire);
    fire = (0 == a_fire) ? 0 : 1;
    err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_T_ENABLE,fire);
    
    if (err < 0)
    {
        DRV_ERR("FLASHLIGHTIOC_T_ENABLE error\n");
        return err ;
    }
    
    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/  
int FlashlightDrv::setLevel(unsigned long  a_level)
{
    int err = 0;
    
    if (m_fdSTROBE < 0)
    {
        DRV_DBG(" [setLevel] m_fdSTROBE < 0\n");
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }

    DRV_DBG("[FlashlightDrv] setLevel = %lu\n",a_level);
    err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_T_LEVEL,a_level);
    if (err < 0) 
    {
        DRV_ERR("FLASHLIGHTIOC_T_ENABLE error;%ld\n",a_level);
        return err;
    }

    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
int FlashlightDrv::setTimeus(unsigned long a_timeus)
{
    int err = 0;
    if (m_fdSTROBE < 0)
    {
        DRV_DBG(" [setTimeus] m_fdSTROBE < 0\n");   
        return StrobeDrv::STROBE_UNKNOWN_ERROR;
    }

    err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_T_FLASHTIME,a_timeus);
    if (err < 0)
    {
        DRV_ERR("FLASHLIGHTIOC_T_ENABLE error\n");
        return err; 
    }

    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/  
int FlashlightDrv::setStartTimeus(unsigned long a_timeus)
{
    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/  
int FlashlightDrv::setState(unsigned long  a_state)
{
    int err = 0;
    unsigned long strobeState = 0;
    
    if (m_fdSTROBE < 0)
    {
        DRV_DBG(" [setState] m_fdSTROBE < 0\n");
        return 0;
    }

    strobeState = (0 == a_state) ? (unsigned long)FLASHLIGHTDRV_STATE_PREVIEW : (unsigned long)FLASHLIGHTDRV_STATE_STILL;
    err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_T_STATE,strobeState);
    if (err < 0)
    {
        DRV_ERR("FLASHLIGHTIOC_T_STATE error\n");
        return err;
    }
    
    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/  
int FlashlightDrv::setFlashlightModeConf(unsigned long a_strobeMode)
{
    int err = StrobeDrv::STROBE_NO_ERROR;
    static bool torchOn = false;    // cotta : added for 555 (torch) mode

    //cotta : modified for 555 (torch) mode
    if (StrobeDrv::FLASHLIGHT_NONE == m_flashType)
    {
        if(a_strobeMode != 555)
        {
            if(torchOn == true)
            {
                DRV_DBG("[setFlashlightModeConf] Torch mode off\n"); 
                torchOn = false;
                err = FlashlightDrv::setFire(0);
                return err;
            }
            else
            {
                DRV_DBG("[setFlashlightModeConf] FLASHLIGHT_NONE\n"); 
                return StrobeDrv::STROBE_NO_ERROR;
            }
        }
    }

    m_strobeMode = a_strobeMode;
    
    DRV_DBG("[setFlashlightModeConf] a_strobeMode: %d \n",m_strobeMode); 

    switch(m_strobeMode)
    {
        case LIB3A_AE_STROBE_MODE_AUTO:
        //case LIB3A_AE_STROBE_MODE_SLOWSYNC:
            break;
        case LIB3A_AE_STROBE_MODE_FORCE_ON:
                //err = FlashlightDrv::setState(1); //still capture state
                //err = FlashlightDrv::setLevel(31); //max level
            if (StrobeDrv::FLASHLIGHT_LED_TORCH == m_flashType)
            {
                err = FlashlightDrv::setFire(1);
            }
            break;

        //FIXME: create an official control path for torch light
        case 555://LIB3A_AE_STROBE_MODE_FORCE_TORCH:
            err = FlashlightDrv::setLevel(1);
            err = FlashlightDrv::setFire(1);
            torchOn = true; //cotta : added for 555 (torch) mode
            break;

        case LIB3A_AE_STROBE_MODE_REDEYE:
            break;
        case LIB3A_AE_STROBE_MODE_FORCE_OFF:
        default:
            err = FlashlightDrv::setFire(0);

            if(torchOn == true) //cotta : added for 555 (torch) mode
            {
                torchOn = false;
            }
            
            break;
    }

    return err;    
}


/*******************************************************************************
*
********************************************************************************/
int FlashlightDrv::setCaptureFlashlightConf(unsigned long a_strobeWidth)
{
    int err = StrobeDrv::STROBE_NO_ERROR;

    DRV_DBG("[setCaptureFlashlightConf] m_flashType = %d, width=%lu\n",m_flashType,a_strobeWidth);

    if (StrobeDrv::FLASHLIGHT_LED_PEAK == m_flashType)
    {
        DRV_DBG("[setCaptureFlashlightConf] FLASHLIGHT_LED_PEAK\n"); 
        err = FlashlightDrv::setState(1); //still capture state
        if (err < 0)
        {
            DRV_DBG("[setCaptureFlashlightConf] setState fail\n"); 
        }
        err = FlashlightDrv::setLevel(a_strobeWidth);
        if (err < 0)
        {
            DRV_DBG("[setCaptureFlashlightConf] setLevel fail\n"); 
        }
    }
    else if (StrobeDrv::FLASHLIGHT_LED_CONSTANT == m_flashType && 0 != a_strobeWidth )
             /*LIB3A_AE_STROBE_MODE_FORCE_ON == m_strobeMode) */
    {       
        err = FlashlightDrv::setState(1); //still capture state
        if (err < 0)
        {
            DRV_DBG("[setCaptureFlashlightConf] setState fail\n"); 
        }
        
        err = FlashlightDrv::setLevel(a_strobeWidth); //set level
        if (err < 0)
        {
            DRV_DBG("[setCaptureFlashlightConf] setLevel fail\n"); 
        }
        
        err = FlashlightDrv::setFire(1);
        if (err < 0) 
        {
            DRV_DBG("[setCaptureFlashlightConf] setFire fail\n"); 
        }
    }
    else 
    {
        DRV_DBG("[setCaptureFlashlightConf] No config\n"); 
    }

    return err;
}


/*******************************************************************************
 Author : Cotta
 Functionality : set value of sensor capture delay
********************************************************************************/  
int FlashlightDrv::setCaptureDelay(unsigned int value)
{
    int err = 0;
    
    if (m_fdSTROBE < 0)
    {
        DRV_DBG(" [setCaptureDelay] m_fdSTROBE < 0\n");
        return 0;
    }

    DRV_DBG(" [setCaptureDelay] set capture delay : %u\n",value);

    err = ioctl(m_fdSTROBE,FLASHLIGHTIOC_T_DELAY,value);
    if (err < 0)
    {
        DRV_ERR("FLASHLIGHTIOC_T_DELAY error : %u\n",value);
        return err;
    }
    
    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
 Author : Cotta
 Functionality : get value of strobe watch dog timer
********************************************************************************/
int FlashlightDrv::getStrobeWDTValue(unsigned int *pValue)
{
    *pValue = FLASH_LIGHT_WDT_TIMEOUT_MS;    

    DRV_DBG("[getStrobeWDTValue] WDT value = %u\n",*pValue);
    return StrobeDrv::STROBE_NO_ERROR;
}


/*******************************************************************************
 Author : Cotta
 Functionality : command control
********************************************************************************/
int FlashlightDrv::sendCommand(unsigned int cmd, unsigned int pArg1, unsigned int *pArg2, unsigned int *pArg3)
{
    int err = 0;

    switch(cmd)
    {
        case CMD_STROBE_SET_CAP_DELAY : 
            err = setCaptureDelay(pArg1);   // set capture delay to strobe kernel driver
            break;
        case CMD_STROBE_GET_WDT_VALUE :
            err = getStrobeWDTValue(pArg2); // get WDT value
            break;       
        default :
            DRV_ERR("[strobe sendCommand] no commadn support\n");
            return StrobeDrv::STROBE_UNKNOWN_ERROR;                            
    }

    if (err < 0)
    {
        DRV_ERR("[strobe sendCommand] Err-ctrlCode(%x)\n",err);
        return err;
    }
    
    return err;
}



