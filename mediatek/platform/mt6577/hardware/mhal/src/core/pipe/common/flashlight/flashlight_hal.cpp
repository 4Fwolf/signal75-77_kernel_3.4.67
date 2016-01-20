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
#define LOG_TAG "mHalFlashlight"

#include "../inc/MediaLog.h"
#include "../inc/MediaAssert.h"

#include "flashlight_hal.h"
#include "flashlight_hal_base.h"
//#include "camera_custom_flashlight.h"

/*******************************************************************************
*
********************************************************************************/
/*
static CAMERA_FLASHLIGHT_PROCESS_STRUCT stFlashlightParam;
static CAMERA_FLASHLIGHT_PROCESS_STRUCT *pstFlashlightParam = &stFlashlightParam;
static CAMERA_FLASHLIGHT_RESULT_STRUCT stFlashlightResult;
static CAMERA_FLASHLIGHT_RESULT_STRUCT *pstFlashlightResult = &stFlashlightResult;


static halFLASHLIGHTBase *pHalFLASHLIGHT = NULL;
*/

/*******************************************************************************
*
********************************************************************************/
halFLASHLIGHTBase*
halFLASHLIGHT::
getInstance()
{

    MHAL_LOG("[halFLASHLIGHT] getInstance \n");
    static halFLASHLIGHT singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void   
halFLASHLIGHT::
destroyInstance() 
{
}

/*******************************************************************************
*                                            
********************************************************************************/
halFLASHLIGHT::halFLASHLIGHT()
{
    m_pStrobeDrvObj = NULL;
    MHAL_LOG("halFLASHLIGHT()\n");
}
halFLASHLIGHT::~halFLASHLIGHT()
{
}
/*******************************************************************************
*
********************************************************************************/
MINT32 halFLASHLIGHT::mHalFlashlightSetParam(
    MINT32 param0,
    MINT32 param1,
    MINT32 param2,
    MINT32 param3
)
{
    MINT32 err = 0;
    MINT32 strobeMode = -1;//AE_STROBE_MODE_UNSUPPORTED;
    
    if (param0) {
        strobeMode = *(MINT32*)param0;
        //AE_STROBE_MODE_FORCE_TORCH
    }
    MHAL_LOG("[mHalFlashlightSetParam]:strobeMode %d \n",strobeMode);

    if (m_pStrobeDrvObj) {
        err = m_pStrobeDrvObj->setFlashlightModeConf(strobeMode);
    }
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 halFLASHLIGHT::mHalFlashlightGetParam(
    MINT32 param0,
    MINT32 *param1,
    MINT32 *param2,
    MINT32 *param3
)
{
    MINT32 err = 0;
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFLASHLIGHT::mHalFlashlightInit(
    MINT32 sensorDev
)
{
    MINT32 err = 0;

    MHAL_LOG("[mHalFlashlightInit] \n");

    if (m_pStrobeDrvObj) {
        MHAL_LOG("[mHalFlashlightInit] Err, Init has been called \n");
        return err;
    }
	// strobe driver
	m_pStrobeDrvObj = StrobeDrv::createInstance();
    if (NULL == m_pStrobeDrvObj) {
        MHAL_LOG("[mHalFlashlightInit] ERROR:create m_pStrobeDrvObj fail \n");
        return -1;
    }
    //strobe drv init
    m_pStrobeDrvObj->init((unsigned long)sensorDev);
    
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
halFLASHLIGHT::mHalFlashlightUninit(
)
{
    MHAL_LOG("[mHalFlashlightUninit] \n");

	// strobe driver
    if (m_pStrobeDrvObj) {
        m_pStrobeDrvObj->uninit();
        m_pStrobeDrvObj->destroyInstance();
		m_pStrobeDrvObj = NULL;
    }
    
    m_pStrobeDrvObj = NULL;

    return 0;
}
/*******************************************************************************
*
********************************************************************************/

MINT32
halFLASHLIGHT::mHalFlashlightSetFire(
    MINT32 fire
)
{
MINT32 err = 0;
    MHAL_LOG("[mHalFlashlightSetFire]:%d \n",fire);

    if (m_pStrobeDrvObj) {
        err = m_pStrobeDrvObj->setFire((unsigned long)fire);
        if (err <0) {
            MHAL_LOG("[mHalFlashlightSetFire] setFire fail \n");
        }
    }

    return err;
}
/*******************************************************************************
*
********************************************************************************/


