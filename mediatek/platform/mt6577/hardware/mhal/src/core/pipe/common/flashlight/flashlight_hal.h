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
/*
** $Log: flashlight_hal.h $
 *
*/

#ifndef _FLASHLIGHT_HAL_H_
#define _FLASHLIGHT_HAL_H_

#include "flashlight_hal_base.h"
#include "strobe_drv.h"

class StrobeDrv;
/*******************************************************************************
*
********************************************************************************/
class halFLASHLIGHT: public halFLASHLIGHTBase 
{
public:
    //
    static halFLASHLIGHTBase* getInstance();
    virtual void destroyInstance();
    //
    /////////////////////////////////////////////////////////////////////////
    //
    // halFDBase () -
    //! \brief FD Hal constructor
    //
    /////////////////////////////////////////////////////////////////////////                       
    halFLASHLIGHT(); 

    /////////////////////////////////////////////////////////////////////////
    //
    // ~mhalCamBase () -
    //! \brief mhal cam base descontrustor 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual ~halFLASHLIGHT();

        /////////////////////////////////////////////////////////////////////////
    //
    // mHalFLASHLIGHTInit () -
    //! \brief init flashlightrama 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFlashlightInit(MINT32 sensorDev);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFLASHLIGHTUninit () -
    //! \brief flashlight uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFlashlightUninit();
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFlashlightSetParam () -
    //! \brief process face detection 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFlashlightSetParam(MINT32 param0, MINT32 param1, MINT32 param2, MINT32 param3);
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFlashlightGetParam () -
    //! \brief draw fd face detection result rectangle 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFlashlightGetParam(MINT32 param0, MINT32 *param1, MINT32 *param2, MINT32 *param3);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFlashlightSetFire () -
    //! \brief draw fd face detection result rectangle 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFlashlightSetFire(MINT32 fire);

protected:

private:
    //
    StrobeDrv* m_pStrobeDrvObj;
};

#endif

