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
** $Log: fd_hal_base.h $
 *
*/

#ifndef _FLASHLIGHT_HAL_BASE_H_
#define _FLASHLIGHT_HAL_BASE_H_

typedef unsigned int MUINT32;
typedef int MINT32;

/*******************************************************************************
*
********************************************************************************/
/*
typedef enum HalFLASHLIGHTObject_s {
    HAL_FLASHLIGHT_OBJ_NONE = 0,
    HAL_FLASHLIGHT_OBJ_MANU,
    HAL_FLASHLIGHT_OBJ_UNKNOWN = 0xFF,
} HalFLASHLIGHTObject_e;
//
#define HAL_FLASHLIGHT_RESET_PARAM    0x00
#define HAL_FLASHLIGHT_SET_DST_DIR    0x10   // Direction (direction)
#define HAL_FLASHLIGHT_SET_DST_BUF    0x11   // Buffer (buffer address, size)
#define HAL_FLASHLIGHT_SET_SRC_DIM    0x20   // Dimension (width, height)
#define HAL_FLASHLIGHT_SET_SRC_BUF    0x21   // Buffer (index, buffer address)
#define HAL_FLASHLIGHT_GET_RESULT     0x40   // result (buffer address, width, height)
*/
/*******************************************************************************
*
********************************************************************************/
class halFLASHLIGHTBase {
public:
    //
    static halFLASHLIGHTBase* createInstance();
    virtual void      destroyInstance() = 0;
    virtual ~halFLASHLIGHTBase() {};
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFLASHLIGHTInit () -
    //! \brief init flashlightrama 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFlashlightInit(MINT32 sensorDev) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFLASHLIGHTUninit () -
    //! \brief flashlight uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFlashlightUninit() {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFlashlightSetParam () -
    //! \brief process face detection 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFlashlightSetParam(MINT32 param0, MINT32 param1, MINT32 param2, MINT32 param3) {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFlashlightGetParam () -
    //! \brief draw fd face detection result rectangle 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFlashlightGetParam(MINT32 param0, MINT32 *param1, MINT32 *param2, MINT32 *param3) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFlashlightSetFire () -
    //! \brief draw fd face detection result rectangle 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFlashlightSetFire(MINT32 fire) {return 0;}

};

#endif

