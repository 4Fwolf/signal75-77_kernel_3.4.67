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
** $Log: mav_hal.h $
 *
*/

#ifndef _MAV_HAL_H_
#define _MAV_HAL_H_

#include "3DF_hal_base.h"
#include "MTKMav.h"
#include "MTKMotion.h"
#include "MTKWarp.h"


class MTKMav;
class MTKMotion;
class MTKWarp;

/*******************************************************************************
*
********************************************************************************/
class halMAV: public hal3DFBase 
{
public:
    //
    static hal3DFBase* getInstance();
    virtual void destroyInstance();
    //
    /////////////////////////////////////////////////////////////////////////
    //
    // halFDBase () -
    //! \brief FD Hal constructor
    //
    /////////////////////////////////////////////////////////////////////////                       
    halMAV(); 

    /////////////////////////////////////////////////////////////////////////
    //
    // ~mhalCamBase () -
    //! \brief mhal cam base descontrustor 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual ~halMAV();

        /////////////////////////////////////////////////////////////////////////
    //
    // mHalMAVInit () -
    //! \brief init mav
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHal3dfInit(void* MavInitInData,void* MotionInitInData,void* WarpInitInData,void* Pano3DInitInData);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMAVUninit () -
    //! \brief mav uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHal3dfUninit();
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMavMain () -
    //! \brief mav main function 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 mHalMavMain();
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMavAddImg () -
    //! \brief mav add image  
    //
    /////////////////////////////////////////////////////////////////////////      
    virtual MINT32 mHal3dfAddImg(MavPipeImageInfo* pParaIn);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHal3dfGetMavResult () -
    //! \brief mav result info  
    //
    /////////////////////////////////////////////////////////////////////////  
    virtual MINT32 mHal3dfGetMavResult(void* pParaOut);  

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMavMerge () -
    //! \brief mav merge image and return result
    //
    /////////////////////////////////////////////////////////////////////////     
    virtual MINT32 mHal3dfMerge(MUINT32 *MavResult);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMavMerge () -
    //! \brief mav merge image and return result
    //
    /////////////////////////////////////////////////////////////////////////   
    virtual MINT32 mHal3dfDoMotion(void* InputData,MUINT32* MotionResult);
    	    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMavWarp () -
    //! \brief do warp image 
    //
    /////////////////////////////////////////////////////////////////////////   
    virtual MINT32 mHal3dfWarp(MavPipeImageInfo* pParaIn,MUINT32 *MavResult,MUINT8 ImgNum);
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMavWarp () -
    //! \brief do warp image 
    //
    /////////////////////////////////////////////////////////////////////////      
    virtual MINT32 mHal3dfCrop(MUINT32 *MavResult,MUINT8 ImgNum);
 
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalMavGetResult () -
    //! \brief check warp image success or not
    //
    /////////////////////////////////////////////////////////////////////////   
    virtual MINT32 mHal3dfGetResult(MUINT32& MavResult);  
    	
protected:


private:
    MTKMav* m_pMTKMavObj;
    MTKMotion* m_pMTKMotionObj;
    MTKMotion* m_pMTKPanoMotionObj;
    MTKWarp* m_pMTKWarpObj;
    MUINT32* SrcImg;
    MAVMotionResultInfo MAVPreMotionResult;
    MUINT8 FrameCunt;
};

#endif

