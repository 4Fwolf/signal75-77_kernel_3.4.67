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
#ifndef _HDR_H_
#define _HDR_H_

#include <hdr_hal_base.h>
#include "mcam_mem.h"	// For mHalCamMemPool struct.


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define HDR_USE_THREAD          1
#define JPG_SAVING_OPTIMIZE     1   // Save JPEG while HDR thread are doing things.
#define HDR_MDP_TWO_OUT         1   // When Capture Full-frame image, MDP outputs small image at the same time.

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/
#if HDR_USE_THREAD
typedef enum {
    HDR_STATE_INIT					= 0x0000,
    HDR_STATE_NORMALIZATION			= 0x0001,
    HDR_STATE_FEATURE_EXTRACITON	= 0x0002,
    HDR_STATE_ALIGNMENT				= 0x0003,
    HDR_STATE_FUSION				= 0x0004,
    HDR_STATE_UNINIT				= 0x0800,
} HdrState_e;
#endif	//HDR_USE_THREAD

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class Hdr : public ShotBase
{
protected:  ////    Multi-Frame
    enum    { eMaxOutputFrameNum = 3 };

protected:  ////    Resolutions.
    MUINT32         mu4W_raw;		//  RAW Width	// Obtained in updateInfo()\queryIspBayerResolution().
    MUINT32         mu4H_raw;		//  RAW Height	// Obtained in updateInfo()\queryIspBayerResolution().
    MUINT32         mu4W_yuv;		//  YUV Width	// Obtained in updateInfo()\queryIspYuvResolution().
    MUINT32         mu4H_yuv;		//  YUV Height	// Obtained in updateInfo()\queryIspYuvResolution().
    MUINT32         mu4W_small;		//  Small Image Width	// Obtained in requestOtherBufs()\QuerySmallImgResolution().
    MUINT32         mu4H_small;		//  Small Image Height	// Obtained in requestOtherBufs()\QuerySmallImgResolution().
    MUINT32         mu4W_sweis;		//  SW EIS Image Width	// Obtained in requestOtherBufs()\QuerySwEisImgResolution().
    MUINT32         mu4H_sweis;		//  SW EIS Image Height	// Obtained in requestOtherBufs()\QuerySwEisImgResolution().
    MUINT32         mu4W_dsmap;		//  Down-sized Weighting Map Width	// Obtained in requestDownSizedWeightMapBuf(). This should be after obtaining OriWeight[0]->weight_table_width.
    MUINT32         mu4H_dsmap;		//  Down-sized Weighting Map Height	// Obtained in requestDownSizedWeightMapBuf(). This should be after obtaining OriWeight[0]->weight_table_height.

protected:  ////    Pipes.
    HdrHalBase*    mpHdrHal;

protected:  ////    Buffers.
	mHalCamMemPool* mpRawImgBuf[eMaxOutputFrameNum];
    MUINT32         mu4RawSize;	// Raw Image Size.

	mHalCamMemPool* mpSourceImgBuf[eMaxOutputFrameNum];
    MUINT32         mu4SourceSize;	// Source Image Size.

	mHalCamMemPool* mpSmallImgBuf[eMaxOutputFrameNum];
    MUINT32         mu4SmallImgSize;	// Small Image Size.

	mHalCamMemPool* mpSwEisImgBuf[eMaxOutputFrameNum];
    MUINT32         mu4SwEisImgSize;	// SW EIS Image Size.

	mHalCamMemPool* mpDownSizedWeightMapBuf[eMaxOutputFrameNum];
    MUINT32         mu4DownSizedWeightMapSize;	// Down-sized Weighting Map Size.

	mHalCamMemPool* mpResultImgBuf;
    MUINT32         mu4ResultImgSize;	// HDR Result Image Size.

	mHalCamMemPool* mpTempBuf;
    MUINT32         mu4TempSize;	// HDR Result Image Size.

	mHalCamMemPool* mpTemp2Buf;
    MUINT32         mu4Temp2Size;	// HDR Result Image Size.

	mHalCamMemPool* mpHdrWorkingBuf;
    MUINT32         mu4HdrWorkingBufSize;	// HDR Working Buf Size.

	mHalCamMemPool* mpMavWorkingBuf;
    MUINT32         mu4MavWorkingBufSize;	// MAV Working Buf Size.

	HDR_PIPE_WEIGHT_TBL_INFO** OriWeight;
	HDR_PIPE_WEIGHT_TBL_INFO** BlurredWeight;


protected:  ////    Parameters.

	MUINT32			mu4OutputFrameNum;		// Output frame number (2 or 3).	// Do not use mu4OutputFrameNum in code directly, use OutputFrameNumGet() instead.

	MUINT32			mu4FinalGainDiff[2];
	MUINT32			mu4TargetTone;

	MUINT32			mu4RunningNumber;		// A serial number for file saving. For debug.

	HDR_PIPE_HDR_RESULT_STRUCT mrHdrCroppedResult;

	MUINT32			mfgIsForceBreak;		// A flag to indicate whether a cancel capture signal is sent.

#if HDR_USE_THREAD
	HdrState_e		mHdrState;
#endif	// HDR_USE_THREAD

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//protected:  ////    JPEG
//    virtual MUINT32 getJpgEncInAddr() const { return mu4RawDecAddr; }
//    virtual MUINT32 getJpgEncInSize() const { return mu4RawDecSize; }

public:     ////    Attributes.
    inline MUINT32	OutputFrameNumGet() const { return /*eMaxOutputFrameNum*/ mu4OutputFrameNum ;}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Interfaces.
    virtual MVOID   destroyInstance();
    Hdr(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId);
    virtual ~Hdr()  {}
    virtual MBOOL   init(ShotParam const& rShotParam, Hal3ABase*const pHal3A);
    virtual MBOOL   uninit();
    virtual MVOID   cancel();
    virtual MBOOL   capture();

protected:  ////    Invoked by capture().
    virtual MBOOL   createJpgImage(MUINT32 u4SrcAddr, MHAL_BITBLT_FORMAT_ENUM emInputFormat, MBOOL fgShowQuickView);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Utilities.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Buffers.
    virtual MBOOL   requestRawImgBuf(void);
    virtual MBOOL   releaseRawImgBuf(void);
    virtual MBOOL   requestSourceImgBuf(void);
    virtual MBOOL   releaseSourceImgBuf(void);
    virtual MBOOL   requestSmallImgBuf(void);
    virtual MBOOL   releaseSmallImgBuf(void);
    virtual MBOOL   requestSwEisImgBuf(void);
    virtual MBOOL   releaseSwEisImgBuf(void);
    virtual MBOOL	requestMavWorkingBuf(void);
    virtual MBOOL	releaseMavWorkingBuf(void);
    virtual MBOOL	requestHdrWorkingBuf(void);
    virtual MBOOL	releaseHdrWorkingBuf(void);
    virtual MBOOL	requestOriWeightMapBuf(void);
    virtual MBOOL	releaseOriWeightMapBuf(void);
    virtual MBOOL	requestBlurredWeightMapBuf(void);
    virtual MBOOL	releaseBlurredWeightMapBuf(void);
    virtual MBOOL	requestDownSizedWeightMapBuf(void);
    virtual MBOOL	releaseDownSizedWeightMapBuf(void);
    virtual MBOOL   requestResultImgBuf(void);
    virtual MBOOL   releaseResultImgBuf(void);
    virtual MBOOL   requestTempBuf(void);
    virtual MBOOL   releaseTempBuf(void);
    virtual MBOOL   requestTemp2Buf(void);
    virtual MBOOL   releaseTemp2Buf(void);


//protected:  ////    register/unregister Raw Codec Buf
//    virtual MBOOL   registerRawCodecBuf(MUINT32 const u4Idx);
//    virtual MBOOL   unregisterRawCodecBuf(MUINT32 const u4Idx);
//    virtual MBOOL   registerRawCodecBufs_StnrRaw();
//    virtual MBOOL   unregisterRawCodecBufs_StnrRaw();
//    virtual MBOOL   registerRawCodecBufs_Lightmap();
//    virtual MBOOL   unregisterRawCodecBufs_Lightmap();

protected:  ////    ISP.
    virtual MBOOL   setIspMode(MUINT32 const u4Mode);
    virtual MBOOL   do_Isp(MBOOL const fgDoEis = MFALSE, MUINT32 const u4Idx = 0);
    virtual MBOOL   camera_start(void);

protected:  ////    MDP.
    virtual MBOOL	MDPResize(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, MUINT32 srcFormat, void* desAdr, MUINT32 desWidth, MUINT32 desHeight, MUINT32 dstFormat) const;
    virtual MBOOL   MDPRotate(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MUINT32 desWidth, MUINT32 desHeight) const;
    virtual MBOOL   MDPJPGENC(MUINT32 srcadr,BufInfo bufInfo,MHAL_BITBLT_FORMAT_ENUM srcformat,int u4Orientation);
    virtual MBOOL   MDPJPGENC_forQuickView(MUINT32 srcadr,BufInfo bufInfo,MHAL_BITBLT_FORMAT_ENUM srcformat,int u4Orientation);
    virtual MBOOL   MDPImgTrasn(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MHAL_BITBLT_FORMAT_ENUM srcformat, MHAL_BITBLT_FORMAT_ENUM desformat) const;

protected:  ////    Save.
    MBOOL           saveRaw(MUINT8*const pRawBuf, MUINT32 const u4RawSize, char const*const szPrefix, char const*const szPostfix);

protected:  ////    Misc.
    virtual MBOOL   configISPParam(MVOID*const pParam, PortInfo const& rInPort, PortInfo const& rOutPort) const;
    virtual MBOOL   configMDPParam(MVOID*const pParam, PortInfo const& rInPort, PortInfo const& rOutPort) const;
    virtual MBOOL   configMDPParamTwoOut(MVOID*const pParam, PortInfo const& rInPort, PortInfo const& rOutPort, PortInfo const& rOutPortSmall) const;
    //
    virtual MBOOL   updateInfo();
    virtual MBOOL   decideCaptureMode();

    virtual MBOOL	createFullFrame(void);
    virtual MBOOL   createFullFrame2Pass(void);
    virtual MBOOL   createFullFrame2PassExample(void);
    virtual MBOOL	createSmallImg(void);
    virtual MBOOL	saveSmallImgForDebug(void);
    virtual MBOOL	createSwEisImg(void);

	virtual MBOOL	do_Normalization(void);
	virtual MBOOL	do_SwEis(void);
	virtual MBOOL	do_FeatureExtraction(void);
	virtual MBOOL	do_Alignment(void);
	virtual MBOOL	do_OriWeightMapGet(void);
	virtual MBOOL	do_DownScaleWeightMap(void);
	virtual MBOOL	do_UpScaleWeightMap(void);
	virtual MBOOL	do_Fusion(void);
	virtual MBOOL	do_HdrCroppedResultGet(void);
	virtual MBOOL	do_CroppedResultResize(void);
	virtual MBOOL	do_HdrSettingClear(void);
    virtual MBOOL	saveSourceJpg(void);

#if HDR_USE_THREAD
public:		////    Thread.
    virtual HdrState_e	GetHdrState(void);
    virtual void	SetHdrState(HdrState_e eHdrState);
    virtual MINT32	mHalCamHdrProc(HdrState_e eHdrState);
#endif	// HDR_USE_THREAD

};


#endif  //  _HDR_H_

