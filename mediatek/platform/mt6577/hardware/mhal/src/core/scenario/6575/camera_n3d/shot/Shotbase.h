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
#ifndef _SHOT_BASE_N3D_H_
#define _SHOT_BASE_N3D_H_


/*******************************************************************************
*
********************************************************************************/
class IShotN3D;
class Hal3ABase;
class IspHal;
class MdpHal;
using NSCamera::BufInfo;


/*******************************************************************************
*
********************************************************************************/
class ShotBaseN3D : public IShotN3D
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    ShotBaseN3D(char const*const szShotName, ESensorType_t const eSensorType, EDeviceId_t const eDeviceId);

public:     ////    Attributes.
    //
    virtual char const*     getShotName() const     { return mszShotName; }
    virtual ESensorType_t   getSensorType() const   { return meSensorType; }
    virtual EDeviceId_t     getDeviceId() const     { return meDeviceId; }

public:     ////    Interfaces.
    //
    virtual MBOOL   init(ShotParam const& rShotParam, Hal3ABase*const pHal3A);
    virtual MBOOL   uninit();
    virtual MBOOL   setParam(ShotParam const& rShotParam);

protected:  ////    Handle Done.
    virtual MBOOL   handleCaptureDone(MINT32 const i4ErrCode = 0);

protected:  ////    Info.
    virtual MBOOL   queryIspBayerResolution(MUINT32& ru4Width, MUINT32& ru4Height) const;
    virtual MBOOL   queryIspYuvResolution(MUINT32& ru4Width, MUINT32& ru4Height) const;

protected:  ////    Attributes.
    char const*const mszShotName;
    ESensorType_t   meSensorType;
    EDeviceId_t     meDeviceId;
    MUINT32         mu4ShotDumpOPT; //  Dump if 1.

protected:  ////    Parameters.
    ShotParam       mShotParam;

protected:  ////    Scenario/Pipes.
    Hal3ABase*      mpHal3A;
    IspHal*         mpIspHal;
    MdpHal*         mpMdpHal;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  JPEG-Related
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    //
    virtual MBOOL   waitJpgCaptureDone();

protected:  ////    JPG Encoder
    virtual MBOOL   encodeJpg(mhalCamFrame_t*const psrc, mhalCamFrame_t*const pdst, MBOOL const isSOI);

protected:  ////    The JPG Size after encoding done.
    MUINT32         mu4JpgEncDoneSize;
    virtual MUINT32 getJpgEncDoneSize() const               { return mu4JpgEncDoneSize; }
    virtual MVOID   setJpgEncDoneSize(MUINT32 const u4Size) { mu4JpgEncDoneSize = u4Size; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Callback Info.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    MBOOL           invokeCallback(MUINT32 const u4Type, MVOID*const pData, MUINT32 const u4Size = 0);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Shutter Callback.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    struct  ShutterCBInfo
    {
    public: ////    Fields.
        typedef mHalCamObserver::CallbackFunc_t CallbackFunc_t;
        CallbackFunc_t      mpfCallback;
        mHalCamCBInfo       mInfo;
        //
    public: ////    Operations.
        //
        ShutterCBInfo()
            : mpfCallback(NULL), mInfo(NULL, 0)
        {}
        //
        inline void set(CallbackFunc_t const pfCallback, mHalCamCBInfo const& rInfo)
        {
            mpfCallback = pfCallback;
            mInfo = rInfo;
        }
    };

protected:  ////    
    ShutterCBInfo   mShutterCBInfo;
    pthread_t       mShutterCBThread;

protected:  ////    
    static MVOID*   shutterCBThread(MVOID* arg);
    virtual MBOOL   invokeShutterCB();
    virtual MBOOL   waitShutterCBDone();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Utilities
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    
    static MUINT32  getAlignedSize(MUINT32 const u4Size);

    virtual BufInfo getBufInfo_JpgEnc() const;
    virtual BufInfo getBufInfo_Raw() const;

    //  Quick View.
    virtual MBOOL   makeQuickViewThumbnail(MUINT32& ru4ThumbSize);

    virtual MBOOL   flattenJpgPicBuf(MUINT32 const u4JpgImgSize, MUINT32 const u4ThumbSize, MUINT32& ru4JpgPictureSize);

};


#endif  //  _SHOT_BASE_N3D_H_

