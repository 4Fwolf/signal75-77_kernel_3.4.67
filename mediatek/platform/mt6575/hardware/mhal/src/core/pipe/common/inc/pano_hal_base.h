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

#ifndef _PANO_HAL_BASE_H_
#define _PANO_HAL_BASE_H_

typedef signed int              kal_int32;

/*******************************************************************************
*
********************************************************************************/
typedef enum HalPANOObject_s {
    HAL_PANO_OBJ_NONE = 0,
    HAL_PANO_OBJ_MANU,
    HAL_PANO_OBJ_UNKNOWN = 0xFF,
} HalPANOObject_e;
//
#define HAL_PANO_RESET_PARAM    0x00
#define HAL_PANO_SET_DST_DIR    0x10   // Direction (direction)
#define HAL_PANO_SET_DST_BUF    0x11   // Buffer (buffer address, size)
#define HAL_PANO_SET_SRC_DIM    0x20   // Dimension (width, height)
#define HAL_PANO_SET_SRC_BUF    0x21   // Buffer (index, buffer address)
#define HAL_PANO_GET_RESULT     0x40   // result (buffer address, width, height)

/*******************************************************************************
*
********************************************************************************/
class halPANOBase {
public:
    //
    static halPANOBase* createInstance(HalPANOObject_e eobject);
    virtual void      destroyInstance() = 0;
    virtual ~halPANOBase() {};
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalPANOInit () -
    //! \brief init panorama 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual kal_int32 mHalPanoInit() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalPANOUninit () -
    //! \brief pano uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual kal_int32 mHalPanoUninit() {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalPanoSetParam () -
    //! \brief process face detection 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual kal_int32 mHalPanoSetParam(kal_int32 param0, kal_int32 param1, kal_int32 param2, kal_int32 param3) {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalPanoGetParam () -
    //! \brief draw fd face detection result rectangle 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual kal_int32 mHalPanoGetParam(kal_int32 param0, kal_int32 *param1, kal_int32 *param2, kal_int32 *param3) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalPanoCalcStitch () -
    //! \brief get face detection result 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual kal_int32 mHalPanoCalcStitch() {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalPanoDoStitch () -
    //! \brief get face detection result 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual kal_int32 mHalPanoDoStitch() {return 0;}

};

class halPANOTmp : public halPANOBase {
public:
    //
    static halPANOBase* getInstance();
    virtual void destroyInstance();
    //
    halPANOTmp() {}; 
    virtual ~halPANOTmp() {};
};

#endif

