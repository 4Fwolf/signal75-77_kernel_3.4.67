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
#ifndef _BEST_SHOT_H_
#define _BEST_SHOT_H_


/*******************************************************************************
*
********************************************************************************/
#include <vector>


/*******************************************************************************
*
********************************************************************************/
class BestShot : public NormalShot
{
    friend  class   IBestShot;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Constructor/Destructor.
    BestShot(char const*const szShotName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId);

public:     ////    Interfaces.
    //
    virtual MBOOL   init(ShotParam const& rShotParam, Hal3ABase*const pHal3A, ICameraIO*const pCameraIO);
    virtual MBOOL   uninit();

protected:  ////
    virtual MBOOL   handleImageReady();
    virtual MBOOL   handleCaptureDone(MINT32 const i4ErrCode /*= 0*/);

protected:  ////
    MBOOL           handleCaptureDone_Intermediate();
    MBOOL           handleCaptureDone_LastOne();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Shutter Callback.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    virtual MBOOL   invokeShutterCB();
    virtual MBOOL   waitShutterCBDone();

protected:  ////    Data Members.
    MUINT32         mu4CurrentCount;        //  current shot count
    MUINT32         mu4TotalCount;          //  total shot count.
    MUINT32         mu4BestShotValue;       //  
    MUINT32         mu4BestShotIndex;       //
    //
protected:  ////    Temp Buffers.
    typedef std::vector<MUINT8> TmpBuf_t;
    MBOOL           saveToTmpBuf(TmpBuf_t& rvTmpBuf, MUINT8 const*const pData, MUINT32 const u4Size);
    MBOOL           loadFromTmpBuf(TmpBuf_t const& rvTmpBuf, MUINT8*const pData, MUINT32& ru4Size);
    //
    TmpBuf_t        mvTmpBuf_postview;      //
    TmpBuf_t        mvTmpBuf_jpeg;          //

};


#endif  //  _BEST_SHOT_H_

