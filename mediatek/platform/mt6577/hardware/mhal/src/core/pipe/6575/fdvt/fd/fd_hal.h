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

#ifndef _FD_HAL_H_
#define _FD_HAL_H_

#include "fd_hal_base.h"

class MTKDetection; 
/*******************************************************************************
*
********************************************************************************/
class halFD: public halFDBase
{
public:
    //
    static halFDBase* getInstance();
    virtual void destroyInstance();
    //
    /////////////////////////////////////////////////////////////////////////
    //
    // halFDBase () -
    //! \brief FD Hal constructor
    //
    /////////////////////////////////////////////////////////////////////////                       
    halFD(); 

    /////////////////////////////////////////////////////////////////////////
    //
    // ~mhalCamBase () -
    //! \brief mhal cam base descontrustor 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual ~halFD();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDInit () -
    //! \brief init face detection 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDInit(MUINT32 fdW, MUINT32 fdH);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDDo () -
    //! \brief process face detection 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDDo(MUINT32 imgBufAddr); 
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDUninit () -
    //! \brief fd uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDUninit();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDDrawFaceRect () -
    //! \brief draw fd face detection result rectangle 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDDrawFaceRect(MUINT8 *pbuf);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDGetFaceResult () -
    //! \brief get face detection result 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDGetFaceResult(camera_face_metadata_m * fd_result, MUINT32 bufSize); 

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDSetDispInfo () -
    //! \brief set display info
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDSetDispInfo(MUINT32 x, MUINT32 y, MUINT32 w, MUINT32 h, MUINT32 rotate, MUINT32 sensor_rotate, MINT32 CameraId) ; 

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSetDetectPara () -
    //! \brief set detection parameter 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halSetDetectPara(MUINT8 Para) ;
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSDGetSmileResult () -
    //! \brief get smile detection result 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halSDGetSmileResult() ; 

    /////////////////////////////////////////////////////////////////////////
    //
    // halFDM4URegister () -
    //! \brief register buffer 
    //
    /////////////////////////////////////////////////////////////////////////   
    virtual MINT32 halFDM4URegister(MUINT8 *pbuf,MUINT32 BufSize,MUINT8 BufCunt) {return 0;}
    
protected:


private:
    MTKDetection* m_pMTKFDObj;
    
    MUINT32 m_FDW; 
    MUINT32 m_FDH; 
    MUINT32 m_DispW; 
    MUINT32 m_DispH; 
    MUINT32 m_DispX; 
    MUINT32 m_DispY;
    MUINT32 m_DispRoate;
    MUINT8 m_DetectPara;
};

#endif

