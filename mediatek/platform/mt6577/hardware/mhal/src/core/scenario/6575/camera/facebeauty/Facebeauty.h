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
#ifndef _MAHL_FB_H_
#define _MAHL_FB_H_

#include "facebeautify_hal_base.h"
#include <mcam_mem.h>

//#define DebugMode
//#define BanchMark 
/*******************************************************************************
*
*******************************************************************************/
class Mhal_facebeauty : public ShotBase
{
protected:  
    halFACEBEAUTIFYBase*     mpFb;
    mHalCamMemPool* mpWorkingBuferr;
    MUINT32 FBWorkingBufferSize;
    
protected:  ////    Resolutions.
    MUINT32         mu4W_raw;       //  RAW Width
    MUINT32         mu4H_raw;       //  RAW Height
    MUINT32         mu4W_yuv;       //  YUV Width
    MUINT32         mu4H_yuv;       //  YUV Height

protected:  ////    Buffers.
    //
    //  Source.
    mHalCamMemPool* mpSource;
    MUINT32         mu4SourceSize;
    //  Resize
    mHalCamMemPool* mpSreResize;   
    //  alpha map.
    mHalCamMemPool* mpAlphamap;
    //  Blurred image
    mHalCamMemPool* mpBlurImg;
        
    MUINT32         mu4Step;
    
    MTKPipeFaceBeautyResultInfo msFaceBeautyResultInfo;

protected:  ////    Info.    
    ESensorType_t meSensorType;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

protected:  ////    Control
	  virtual MUINT32 getControlStep() const { return mu4Step; }
	  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.
    virtual MVOID   destroyInstance();
    Mhal_facebeauty(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId);
    virtual ~Mhal_facebeauty()  {}
    virtual MBOOL   init(ShotParam const& rShotParam, Hal3ABase*const pHal3A);
    virtual MBOOL   uninit();
    virtual MBOOL   capture();
    virtual MBOOL   doCapture(); 
    virtual MBOOL   SaveOrgFile();
protected:  ////    Invoked by capture().
    virtual MBOOL   createSource(MUINT32 const u4Mode);
    virtual MBOOL   createJpgImage(MUINT32 srcadr,MBOOL dumpOrg);
    virtual MBOOL   reConfigISP();
    virtual MBOOL   MDPJPGENC(MUINT32 srcadr,BufInfo bufInfo,MHAL_BITBLT_FORMAT_ENUM srcformat,int u4Orientation);
protected:  ////    Lightmap
    virtual MBOOL   createFullFrame();
    virtual MBOOL   createFullFrame2Pass();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Utilities.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Buffers.
    virtual MBOOL   requestBufs(MUINT32 u4Mode);
    virtual MBOOL   releaseBufs();

protected:  ////    ISP.
    virtual MBOOL   setIspMode(MUINT32 const u4Mode);
    virtual MBOOL   do_Isp(MBOOL const fgDoEis = MFALSE, MUINT32 const u4Idx = 0);

protected:  ////    Misc.
    virtual MBOOL   InitialAlgorithm(MUINT32 srcWidth, MUINT32 srcHeight, MINT32 gBlurLevel, MINT32 FBTargetColor);
    virtual MBOOL   configISPParam(MVOID*const pParam, PortInfo const& rInPort, PortInfo const& rOutPort) const;
    virtual MBOOL   configMDPParam(MVOID*const pParam, PortInfo const& rInPort, PortInfo const& rOutPort) const;
    virtual MBOOL   MDPResize(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MUINT32 desWidth, MUINT32 desHeight) const;
    virtual MBOOL   MDPZoomCrop(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MUINT32 desWidth, MUINT32 desHeight) const;
    virtual MBOOL   MDPRotate(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MUINT32 desWidth, MUINT32 desHeight) const;
    virtual MBOOL   MDPPCA(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MUINT32 desWidth, MUINT32 desHeight, void* FaceBeautyResultInfo);
    virtual MBOOL   MDPImgTrasn(void* srcAdr,MUINT32 srcWidth, MUINT32 srcHeight,void* desAdr,MHAL_BITBLT_FORMAT_ENUM srcformat, MHAL_BITBLT_FORMAT_ENUM desformat) const;
    virtual MBOOL   createBlurImage(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MUINT32 desWidth, MUINT32 desHeight, MINT8 NRTimes);
    virtual MBOOL   createAlphaMap(void* ImgSrcAddr, void* ImgDsAddr,void* ImgBlurAddr,void* FDResult,void* FDInfo,void* FaceBeautyResultInfo) const;
    virtual MBOOL   createBlendedImage(void* tmpDsAddr,void* BlenddesAdr,void* BlendYSdesAdr,void* TextAlphaMapAddr,void* ColorAlphaMapAddr, MUINT32 AlphaWidth, MUINT32 AlphaHeight,void* AlphaMapAddr,void* FaceBeautyResultInfo) const;
    virtual MBOOL   createFinalImage(void* FinaldesAddr,void* PCAImgAddr,void* ColorAlphaMapAddr,void* FaceBeautyResultInfo) const;
    
protected:  ////    MDP.
            int     MdpPathImageProcess(void* parameter);         
};


#endif  //  _MAHL_FB_H_

