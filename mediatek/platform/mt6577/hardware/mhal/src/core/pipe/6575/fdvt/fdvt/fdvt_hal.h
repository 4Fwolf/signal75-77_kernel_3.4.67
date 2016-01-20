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

#ifndef _FDVT_HAL_H_
#define _FDVT_HAL_H_

#include "fd_hal_base.h"

//Please fix me Sava
#if defined(MTK_M4U_SUPPORT)
//#undef MTK_M4U_SUPPORT
#endif

#if defined(MTK_M4U_SUPPORT)    
typedef struct fdvtM4UInfo_t {
    MUINT32 virtAddr; 
    MUINT32 useM4U; 
    MUINT32 M4UVa; 
    MUINT32 size; 
}FDVTM4UInfo; 
#endif 

class MTKDetection; 
/*******************************************************************************
*
********************************************************************************/
class halFDVT: public halFDBase 
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
    halFDVT(); 

    /////////////////////////////////////////////////////////////////////////
    //
    // ~mhalCamBase () -
    //! \brief mhal cam base descontrustor 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual ~halFDVT();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDInit () -
    //! \brief init face detection 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDInit(MUINT32 fdW, MUINT32 fdH);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDVTDo () -
    //! \brief process face detection 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDDo(MUINT32 imgBufAddr); 
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDVTUninit () -
    //! \brief FDVT uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDUninit();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDVTDrawFaceRect () -
    //! \brief draw FDVT face detection result rectangle 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDDrawFaceRect(MUINT8 *pbuf);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSDVTDrawFaceRect () -
    //! \brief draw smile detection result rectangle 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halSDDrawFaceRect(MUINT8 *pbuf);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDVTGetFaceInfo () -
    //! \brief get face detection result 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDGetFaceInfo(mtk_face_info_m *fd_info_result); 

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDVTGetFaceResult () -
    //! \brief get face detection result 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halFDGetFaceResult(camera_face_metadata_m * fd_result, MUINT32 bufSize); 

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDVTSetDispInfo () -
    //! \brief set display info
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalFDSetDispInfo(MUINT32 x, MUINT32 y, MUINT32 w, MUINT32 h, MUINT32 rotate, MUINT32 sensor_rotate, MINT32 CameraId) ;

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
    virtual MINT32 halSDGetSmileResult( ) ; 

    #if defined (MTK_M4U_SUPPORT)
    /////////////////////////////////////////////////////////////////////////
    //
    // halFDM4URegister () -
    //! \brief register buffer 
    //
    /////////////////////////////////////////////////////////////////////////   
    virtual MINT32 halFDM4URegister(MUINT8 *pbuf,MUINT32 BufSize,MUINT8 BufCunt) ;
    #endif        
    
protected:


private:
#if defined(MTK_M4U_SUPPORT)    
    FDVTM4UInfo mIspReadPort;
    //ISPM4UInfo mIspWritePort; 
    MINT32 allocM4UMemory(MUINT32 virtAddr, MUINT32 size, MUINT32 *m4uVa); 
    MINT32 freeM4UMemory(MUINT32 m4uVa, MUINT32 size);   
    MUINT32 vir2m4u(MUINT32 imgBufAddr);     
#endif 

    MTKDetection* m_pMTKFDVTObj;
    
    MUINT32 m_FDW; 
    MUINT32 m_FDH; 
    MUINT32 m_DispW; 
    MUINT32 m_DispH; 
    MUINT32 m_DispX; 
    MUINT32 m_DispY;
    MUINT32 m_DispRoate;
    MUINT32 m_RegisterBuff;
    MUINT32 m_BuffCount;
    MUINT8 m_DetectPara;
};

#endif

