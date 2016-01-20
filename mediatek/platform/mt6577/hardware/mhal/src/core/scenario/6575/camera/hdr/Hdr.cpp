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
#include "MyHdr.h"
#include <utils/threads.h>
#include <sys/prctl.h>  // For prctl()/PR_SET_NAME.
#include <cutils/properties.h>  //Vent@20120427: For [ALPS00275297] Lenovo eService: UI freeze for 1 sec if press Power Key to sleep and resume in the middle of HDR capture.




/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
#if HDR_USE_THREAD
static pthread_t threadHdrNormal;
static sem_t semHdrThread, semHdrThreadBack, semHdrThreadEnd;
static Hdr* pHdrObj = NULL;
#endif  // HDR_USE_THREAD

/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/
#if HDR_USE_THREAD
MVOID *mHalCamHdrThread(MVOID *arg);
#endif  // HDR_USE_THREAD





/*******************************************************************************
*
*******************************************************************************/
IHdr*
IHdr::
createInstance(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
{
	using namespace NSCamera;
	switch	(eSensorType)
	{
	case eSensorType_RAW:
		{
		static Hdr	instHdr(eSensorType, eDeviceId);
		static IHdr instIHdr(&instHdr);
		return &instIHdr;
		}
		break;
	case eSensorType_YUV:
		{
		static Hdr	instHdr(eSensorType, eDeviceId);
		static IHdr instIHdr(&instHdr);
		return &instIHdr;
		}
		break;
	default:
		MY_WARN("[createInstance] YUV is unsupported!");
		break;
	}

	return	NULL;

}


/*******************************************************************************
*
*******************************************************************************/
MVOID
IHdr::
destroyInstance()
{
}


/*******************************************************************************
*
*******************************************************************************/
IHdr::
IHdr(ShotBase*const pShot)
	: IShot()
	, mrShotBase(*pShot)
{
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
IHdr::
init(ShotParam const& rShotParam, Hal3ABase*const pHal3A)
{
	return	mrShotBase.init(rShotParam, pHal3A);
}


/*******************************************************************************
*
*******************************************************************************/
MVOID
Hdr::
destroyInstance()
{
}


/*******************************************************************************
*
*******************************************************************************/
Hdr::
Hdr(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
	: ShotBase("HdrShot", eSensorType, eDeviceId)
	//
	, mu4W_raw(0)
	, mu4H_raw(0)
	, mu4W_yuv(0)
	, mu4H_yuv(0)
	, mu4W_small(0)
	, mu4H_small(0)
	, mu4W_sweis(0)
	, mu4H_sweis(0)
	, mu4W_dsmap(0)
	, mu4H_dsmap(0)
	//
	, mpHdrHal(NULL)
	//
	, mpRawImgBuf()
	, mu4RawSize(0)
	, mpSourceImgBuf()
	, mu4SourceSize(0)
	, mpSmallImgBuf()
	, mu4SmallImgSize(0)
	, mpSwEisImgBuf()
	, mu4SwEisImgSize(0)
	, mpDownSizedWeightMapBuf()
	, mu4DownSizedWeightMapSize(0)
	, mpResultImgBuf(NULL)
	, mu4ResultImgSize(0)
	, mpTempBuf(NULL)
	, mu4TempSize(0)
	, mpTemp2Buf(NULL)
	, mu4Temp2Size(0)
	, mpHdrWorkingBuf(NULL)
	, mu4HdrWorkingBufSize(0)
	, mpMavWorkingBuf(NULL)
	, mu4MavWorkingBufSize(0)
	, OriWeight(NULL)
	, BlurredWeight(NULL)
	//
	, mu4OutputFrameNum(0)
	, mu4TargetTone(0)
	, mu4RunningNumber(0)
	, mfgIsForceBreak(FALSE)
#if HDR_USE_THREAD
	, mHdrState(HDR_STATE_INIT)
#endif  //HDR_USE_THREAD

{
	::memset(mpRawImgBuf, 0, sizeof(mpRawImgBuf));
	::memset(mpSourceImgBuf, 0, sizeof(mpSourceImgBuf));
	::memset(mpSmallImgBuf, 0, sizeof(mpSmallImgBuf));
	::memset(mpSwEisImgBuf, 0, sizeof(mpSwEisImgBuf));
	::memset(mpDownSizedWeightMapBuf, 0, sizeof(mpDownSizedWeightMapBuf));

#if HDR_USE_THREAD
    pHdrObj = this; // Used in mHalCamHdrThread().
#endif  // HDR_USE_THREAD

	mu4FinalGainDiff[0] = 0;
	mu4FinalGainDiff[1] = 0;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
init(ShotParam const& rShotParam, Hal3ABase*const pHal3A)
{
	MY_DBG("[init] - E. v.01.");

	MBOOL	ret = MTRUE;
	MINT32	err = 0;	// Error code. 0: no error. others: error.

	ret =
			//	()	ShotBase init.
			ShotBase::init(rShotParam, pHal3A)
			//	()	Update Information.
		&&	updateInfo()
			//	()	Request Buffers.
		&&	requestRawImgBuf()
			//	()	Request Buffers.
		&&	requestSourceImgBuf()
			;
	if	( ! ret )
	{
		goto lbExit;
	}

	// HDR Pipe Init.
	//	   Config HDR Pipe init info structure.
	HDR_PIPE_INIT_INFO rHdrPipeInitInfo;
	rHdrPipeInitInfo.u4ImgW				= mu4W_yuv;
	rHdrPipeInitInfo.u4ImgH				= mu4H_yuv;
	rHdrPipeInitInfo.u4OutputFrameNum	= OutputFrameNumGet();
	rHdrPipeInitInfo.u4FinalGainDiff0	= mu4FinalGainDiff[0];
	rHdrPipeInitInfo.u4FinalGainDiff1	= mu4FinalGainDiff[1];
	rHdrPipeInitInfo.u4TargetTone		= mu4TargetTone;
	for (MUINT32 i = 0; i < OutputFrameNumGet(); i++)
	{
		rHdrPipeInitInfo.pSourceImgBufAddr[i] = (MUINT32)(mpSourceImgBuf[i]->getVirtAddr());
	}
	//	   Create HDR hal object.
	mpHdrHal = HdrHalBase::createInstance();
	if	( ! mpHdrHal )
	{
		MY_ERR("HdrHalBase::createInstance fail.");
		goto lbExit;
	}
	//	   Init HDR hal object.
	ret = mpHdrHal->init((void*)(&rHdrPipeInitInfo));
	if	( ! ret )
	{
		MY_ERR("mpHdrHal->init fail.");
		goto lbExit;
	}

#if HDR_USE_THREAD
    // Create HDR thread.
    SetHdrState(HDR_STATE_INIT);
    ::sem_init(&semHdrThread, 0, 0);
    ::sem_init(&semHdrThreadBack, 0, 0);
    ::sem_init(&semHdrThreadEnd, 0, 0);
    pthread_create(&threadHdrNormal, NULL, mHalCamHdrThread, NULL);
	MY_DBG("[init] threadHdrNormal: %d.", threadHdrNormal);
#endif  // HDR_USE_THREAD


lbExit:
	if	( ! ret )
	{
		uninit();
	}

	MY_DBG("[init] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
uninit()
{
	MY_DBG("[uninit] - E.");

	MBOOL ret = MTRUE;

	if (mpHdrHal)
	{
		mpHdrHal->uninit();
		mpHdrHal->destroyInstance();
		mpHdrHal = NULL;
	}

	mu4W_raw = mu4H_raw = mu4W_yuv = mu4H_yuv = 0;

#if HDR_USE_THREAD
    SetHdrState(HDR_STATE_UNINIT);
    ::sem_post(&semHdrThread);
    ::sem_wait(&semHdrThreadEnd);
    MY_DBG("semHdrThreadEnd received.");
#endif  // HDR_USE_THREAD

	ret = ShotBase::uninit();


	MY_DBG("[uninit] - X. ret: %d.", ret);

	return	ret;

}

/*******************************************************************************
*
*******************************************************************************/
MVOID
Hdr::
cancel()
{
	MY_DBG("[cancel] - E.");

    mfgIsForceBreak = TRUE;

	MY_DBG("[cancel] - X. mfgIsForceBreak: %d.", mfgIsForceBreak);
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
capture()
{
	MY_DBG("[capture] - E.");
	MBOOL	ret = MTRUE;

#if (HDR_PROFILE_CAPTURE2)
	MyDbgTimer DbgTmr("capture");
#endif


	// Increase 4-digit running number (range: 0 ~ 9999).
	if (mu4RunningNumber >= 9999)
	{
		mu4RunningNumber = 0;
	}
	else
	{
		mu4RunningNumber++;
	}



		    //	()	Request SmallImg Buffers. Move requestSmallImgBuf() before createFullFrame() because when HDR_MDP_TWO_OUT, createFullFrame() will use SmallImg buffer.
            requestSmallImgBuf();
                                            #if (HDR_PROFILE_CAPTURE2)
                                            DbgTmr.print("HdrProfiling2:: requestSmallImgBuf Time");
                                            #endif
		// Create source image with full frame size. when HDR_MDP_TWO_OUT, it will also create small images.
        if (meSensorType == eSensorType_RAW)
        {
            createFullFrame2Pass();
        }
        else
        {
			createFullFrame();
        }
                                            #if (HDR_PROFILE_CAPTURE2)
                                            DbgTmr.print("HdrProfiling2:: 3-FullFrame Time");
	                                        #endif
    ret =	
			//	()	Shutter Callback
			invokeShutterCB()
                                            #if (HDR_PROFILE_CAPTURE2)
                                    		&&	DbgTmr.print("HdrProfiling2:: invokeShutterCB Time")
                                            #endif
		&&	releaseRawImgBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: releaseRawImgBuf Time")
                                            #endif
#if (!HDR_MDP_TWO_OUT)
			// Create small image from resizing source image.
		&&	createSmallImg()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: 3-SmallImg Time")
                                            #endif
#endif  // !HDR_MDP_TWO_OUT
            // Save small images for debug use. (Only enabled when HDR_DEBUG_SAVE_SMALL_IMAGE is on.)
        &&  saveSmallImgForDebug();
			;

	if	( ! ret )
	{
		goto lbExit;
	}


/*Vent@20120427: For [ALPS00275297] Lenovo eService: UI freeze for 1 sec if press Power Key to sleep and resume in the middle of HDR capture.*/
if (mfgIsForceBreak)
{
	MY_DBG("[capture] Force break before NORMALIZATION.");
    goto lbExit;
}
/*Vent@20120427: END*/

    #if HDR_USE_THREAD
	//	()	Normalize small images, and put them back to SmallImg[].
    SetHdrState(HDR_STATE_NORMALIZATION);
    ::sem_post(&semHdrThread);
	MY_DBG("[capture] semHdrThread (HDR_STATE_NORMALIZATION) posted.");
    ::sem_wait(&semHdrThreadBack);
	MY_DBG("[capture] semHdrThreadBack (HDR_STATE_NORMALIZATION) received.");
    #else
	ret =
			//	()	Normalize small images, and put them back to SmallImg[].
			do_Normalization()
			;
    #endif  // HDR_USE_THREAD
                                            #if (HDR_PROFILE_CAPTURE2)
                                            DbgTmr.print("HdrProfiling2:: do_Normalization Time");
                                            #endif

/*Vent@20120427: For [ALPS00275297] Lenovo eService: UI freeze for 1 sec if press Power Key to sleep and resume in the middle of HDR capture.*/
if (mfgIsForceBreak)
{
	MY_DBG("[capture] Force break after NORMALIZATION.");
    goto lbExit;
}
/*Vent@20120427: END*/

	ret =
			//	()	Request SwEisImg Buffers.
			requestSwEisImgBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: requestSwEisImgBuf Time")
                                            #endif
			//	()	Create SwEis Img (resize 3 Small Img to 3 SwEis Img).
		&&	createSwEisImg()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: createSwEisImg Time")
                                            #endif
			//	()	Do SW EIS to get GMV.
		&&	do_SwEis()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: do_SwEis Time")
                                            #endif
			//	()	Release SwEisImg Buffers.
		&&	releaseSwEisImgBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: releaseSwEisImgBuf Time")
                                            #endif
			//	()	Request MAV Working Buffer.
		&&	requestMavWorkingBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: requestMavWorkingBuf Time")
                                            #endif
			;

	if	( ! ret )
	{
		goto lbExit;
	}

/*Vent@20120427: For [ALPS00275297] Lenovo eService: UI freeze for 1 sec if press Power Key to sleep and resume in the middle of HDR capture.*/
if (mfgIsForceBreak)
{
	MY_DBG("[capture] Force break before FEATURE_EXTRACITON.");
    goto lbExit;
}
/*Vent@20120427: END*/

    #if HDR_USE_THREAD
	//	()	Do Feature Extraciton.
    SetHdrState(HDR_STATE_FEATURE_EXTRACITON);
    ::sem_post(&semHdrThread);
	MY_DBG("[capture] semHdrThread (HDR_STATE_FEATURE_EXTRACITON) posted.");
    ::sem_wait(&semHdrThreadBack);
	MY_DBG("[capture] semHdrThreadBack (HDR_STATE_FEATURE_EXTRACITON) received.");
    #else
	ret =
			//	()	Do Feature Extraciton.
			do_FeatureExtraction()
			;
    #endif  // HDR_USE_THREAD
                                            #if (HDR_PROFILE_CAPTURE2)
                                            DbgTmr.print("HdrProfiling2:: do_FeatureExtraction Time");
                                            #endif

/*Vent@20120427: For [ALPS00275297] Lenovo eService: UI freeze for 1 sec if press Power Key to sleep and resume in the middle of HDR capture.*/
if (mfgIsForceBreak)
{
	MY_DBG("[capture] Force break after FEATURE_EXTRACITON.");
    goto lbExit;
}
/*Vent@20120427: END*/

	ret =
			//	()	Release MAV working buffer, because it's not needed anymore.
			releaseMavWorkingBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: releaseMavWorkingBuf Time")
                                            #endif
			//	()	Release MAV working buffer, because it's not needed anymore.
		&&	releaseSmallImgBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: releaseSmallImgBuf Time")
                                            #endif
			//	()	Request HDR Working Buffer.
		&&	requestHdrWorkingBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: requestHdrWorkingBuf Time")
                                            #endif
			;

	if	( ! ret )
	{
		goto lbExit;
	}

/*Vent@20120427: For [ALPS00275297] Lenovo eService: UI freeze for 1 sec if press Power Key to sleep and resume in the middle of HDR capture.*/
if (mfgIsForceBreak)
{
	MY_DBG("[capture] Force break before ALIGNMENT.");
    goto lbExit;
}
/*Vent@20120427: END*/

    #if HDR_USE_THREAD
    SetHdrState(HDR_STATE_ALIGNMENT);
    ::sem_post(&semHdrThread);
	MY_DBG("[capture] semHdrThread (HDR_STATE_ALIGNMENT) posted.");
    ::sem_wait(&semHdrThreadBack);
	MY_DBG("[capture] semHdrThreadBack (HDR_STATE_ALIGNMENT) received.");
    #else
	ret =
			//	()	Do Alignment (includeing "Feature Matching" and "Weighting Map Generation").
			do_Alignment()
			;
    #endif  // HDR_USE_THREAD
                                            #if (HDR_PROFILE_CAPTURE2)
                                            DbgTmr.print("HdrProfiling2:: do_Alignment Time");
                                            #endif

/*Vent@20120427: For [ALPS00275297] Lenovo eService: UI freeze for 1 sec if press Power Key to sleep and resume in the middle of HDR capture.*/
if (mfgIsForceBreak)
{
	MY_DBG("[capture] Force break after ALIGNMENT.");
    goto lbExit;
}
/*Vent@20120427: END*/

	ret =
			//	()	Request Original Weighting Table Buffer.
			requestOriWeightMapBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: requestOriWeightingTblBuf Time")
                                            #endif
			//	()	Get original Weighting Map.
		&&	do_OriWeightMapGet()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: do_OriWeightMapGet Time")
                                            #endif
			// Blur Weighting Map by downsize-then-upscale it.
		&&	requestDownSizedWeightMapBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: requestDownSizedWeightMapBuf Time")
                                            #endif
			//	()	Down-scale original weighting map, and put into DownSizedWeightMapBuf.
		&&	do_DownScaleWeightMap()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: do_DownScaleWeightMap Time")
                                            #endif
			//	()	Request Blurred weighting map buffer.
		&&	requestBlurredWeightMapBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: requestBlurredWeightMapBuf Time")
                                            #endif
			//	()	Up-scale DownSized WeightMap Buf, and put into blurred weighting map.
		&&	do_UpScaleWeightMap()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: do_UpScaleWeightMap Time")
                                            #endif
			//	()	Release OriWeightMapBuf, because it's not needed anymore. Note: Some info of OriWeightMap are needed when requestBlurredWeightMapBuf(), so must release it after requestBlurredWeightMapBuf().
		&&	releaseOriWeightMapBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: releaseOriWeightMapBuf Time")
                                            #endif
			// Release DownSizedWeightMapBuf[i] because it's no longer needed.
		&&	releaseDownSizedWeightMapBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: releaseDownSizedWeightMapBuf Time")
                                            #endif
			;

	if	( ! ret )
	{
		goto lbExit;
	}

	// Allocate temp buffer for rotation. This must be done before any createJpgImage().
    if (mShotParam.u4JPEGOrientation)   // If rotation, must allocate buffer for rotation use.
    {
        ret = requestTempBuf();
     	if	( ! ret )
    	{
	        MY_ERR("[capture] TempBuf for rotation allocate fail.");
    		goto lbExit;
    	}       

        ret = requestTemp2Buf();
     	if	( ! ret )
    	{
	        MY_ERR("[capture] Temp2Buf for rotation allocate fail.");
    		goto lbExit;
    	}       
    }

/*Vent@20120427: For [ALPS00275297] Lenovo eService: UI freeze for 1 sec if press Power Key to sleep and resume in the middle of HDR capture.*/
if (mfgIsForceBreak)
{
	MY_DBG("[capture] Force break before FUSION.");
    goto lbExit;
}
/*Vent@20120427: END*/

    #if HDR_USE_THREAD
    SetHdrState(HDR_STATE_FUSION);
    ::sem_post(&semHdrThread);
	MY_DBG("[capture] semHdrThread (HDR_STATE_FUSION) posted.");

#if JPG_SAVING_OPTIMIZE
ret =
		//{ B } Jpeg
		createJpgImage(mpSourceImgBuf[1]->getVirtAddr(), MHAL_FORMAT_YUV_420/*YUV420_Planar*/, MTRUE)
                                        #if (HDR_PROFILE_CAPTURE2)
                                        &&	DbgTmr.print("HdrProfiling2:: createJpgImage Time (thread)")
                                        #endif
		//	()	Save source image jpg.
	&&	saveSourceJpg()
                                        #if (HDR_PROFILE_CAPTURE2)
                                        &&	DbgTmr.print("HdrProfiling2:: saveSourceJpg Time (thread)")
                                        #endif
        ;
#endif  // JPG_SAVING_OPTIMIZE

    ::sem_wait(&semHdrThreadBack);
	MY_DBG("[capture] semHdrThreadBack (HDR_STATE_FUSION) received.");
    #else
	ret =
			//	()	Do Fusion.
			do_Fusion()
			;
    #endif  // HDR_USE_THREAD
                                            #if (HDR_PROFILE_CAPTURE2)
                                            DbgTmr.print("HdrProfiling2:: do_Fusion Time");
                                            #endif

/*Vent@20120427: For [ALPS00275297] Lenovo eService: UI freeze for 1 sec if press Power Key to sleep and resume in the middle of HDR capture.*/
if (mfgIsForceBreak)
{
	MY_DBG("[capture] Force break after FUSION.");
    goto lbExit;
}
/*Vent@20120427: END*/

#if (!JPG_SAVING_OPTIMIZE)
ret =
		//{ B } Jpeg
		createJpgImage(mpSourceImgBuf[1]->getVirtAddr(), MHAL_FORMAT_YUV_420/*YUV420_Planar*/, MTRUE)
                                        #if (HDR_PROFILE_CAPTURE2)
                                        &&	DbgTmr.print("HdrProfiling2:: createJpgImage Time (non-thread)")
                                        #endif
		//	()	Save source image jpg.
	&&	saveSourceJpg()
                                        #if (HDR_PROFILE_CAPTURE2)
                                        &&	DbgTmr.print("HdrProfiling2:: saveSourceJpg Time (non-thread)")
                                        #endif
        ;
#endif  // JPG_SAVING_OPTIMIZE

	ret =
			//	()	Release SourceImgBuf[i] because it's no longer needed.
			releaseSourceImgBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: releaseSourceImgBuf Time")
                                            #endif
			//	()	Release Blurred weighting map because it's no longer needed.
		&&	releaseBlurredWeightMapBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: releaseBlurredWeightMapBuf Time")
                                            #endif
			//	()	Get HDR Cropped Result image.
		&&	do_HdrCroppedResultGet()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: do_HdrResultGet Time")
                                            #endif
			//	()	Request Result Image Buffer to put image (which is resized to original size) for JPG encode.
		&&	requestResultImgBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: requestResultImgBuf Time")
                                            #endif
			//	()	Resized cropped result image to original size for JPG encode, and put it into ResultImgBuf.
		&&	do_CroppedResultResize()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling:: do_CroppedResultResize Time")
                                            #endif
			//	()	Release HDR working buffer because it's no longer needed.
		&&	releaseHdrWorkingBuf()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: releaseHdrWorkingBuf Time")
                                            #endif
			//	()	HDR finished, clear HDR setting.
		&&	do_HdrSettingClear()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: do_HdrSettingClear Time")
                                            #endif
			//{ B } Jpeg
		&&	createJpgImage(mpResultImgBuf->getVirtAddr(), MHAL_FORMAT_YUY2/*YUY2_Pack*/, MTRUE)
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: createJpgImage Time")
                                            #endif
			//	()	Wait Shutter Callback Done
		&&	waitShutterCBDone()
                                            #if (HDR_PROFILE_CAPTURE2)
                                            &&	DbgTmr.print("HdrProfiling2:: waitShutterCBDone Time")
                                            #endif
			;

	// Restore to the last setting which Preview AE used before HDR start.
	// Note: There is a short period between Preview unfreeze and Preview AE start.
	//       So if not set this, user will see a bright screen when QuickView
	//       finish (i.e. Preview unfreeze) because PV AE is not start yet.
	mpHal3A->setPreviewParam();

	//	Force to handle done even if there is any error before.
	handleCaptureDone();
                                            #if (HDR_PROFILE_CAPTURE2)
                                            DbgTmr.print("HdrProfiling2:: handleCaptureDone Time");
                                            #endif

	if	( ! ret )
	{
		goto lbExit;
	}

//	  ret = MTRUE;
lbExit:
    // Don't know exact time of lbExit in HDR flow, so release all again (there is protection in each release function).
    releaseRawImgBuf();
    releaseSourceImgBuf();
    releaseSmallImgBuf();
    releaseSwEisImgBuf();
    releaseHdrWorkingBuf();
    releaseMavWorkingBuf();
    releaseOriWeightMapBuf();
    releaseDownSizedWeightMapBuf();
    releaseBlurredWeightMapBuf();
    releaseResultImgBuf();
    if (mShotParam.u4JPEGOrientation)   // If rotation.
    {
        releaseTempBuf();
        releaseTemp2Buf();
    }

    #if (HDR_PROFILE_CAPTURE2)
	DbgTmr.print("HdrProfiling2:: HDRFinish Time");
    #endif

	MY_DBG("[capture] - X. ret: %d. mfgIsForceBreak: %d.", ret, mfgIsForceBreak);

    // Reset mfgIsForceBreak.
	mfgIsForceBreak = FALSE;

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
createJpgImage(MUINT32 u4SrcAddr, MHAL_BITBLT_FORMAT_ENUM emInputFormat, MBOOL fgShowQuickView)
{
    // Get orientation.
    MUINT32 u4Orientation = mShotParam.u4JPEGOrientation;

	MY_DBG("[createJpgImage] - E. u4SrcAddr: 0x%08X. emInputFormat: %d. u4Orientation: %d. fgShowQuickView: %d.", u4SrcAddr, emInputFormat, u4Orientation, fgShowQuickView);

	using namespace NSCamera;

#if (HDR_PROFILE_CAPTURE)
	MyDbgTimer DbgTmr("createJpgImage");
#endif


	MBOOL	ret = MFALSE;
	MINT32	err = -1;

#if 1   // For HDR Rotate.
	BufInfo bufInfo;

    if (u4Orientation)
    {
        MY_DBG("[createJpgImage] Enter HDR rotation process.."); 
        mrRotatedQVBuf.u4BufSize = mShotParam.frmQv.w*mShotParam.frmQv.h * 3 / 2; //for ARGB888
        allocateRotatedQuickViewBuf(mrRotatedQVBuf);
    }
    bufInfo.u4BufVA = mShotParam.frmQv.virtAddr;   
    bufInfo.u4BufPA = mShotParam.frmQv.phyAddr; 
    MY_DBG("[MDPJPGENC] bufInfo VA 0x%x bufInfo PA 0x%x\n",bufInfo.u4BufVA,bufInfo.u4BufPA); 

    if (!u4Orientation)    // No rotation.
    {
        // Encode both thumbnail and normal image.
        MDPJPGENC(u4SrcAddr, bufInfo, emInputFormat, 0);
                                                #if (HDR_PROFILE_CAPTURE)
                                                DbgTmr.print("HdrProfiling:: Source encode");
                                                #endif
    }
    else  // With rotation.
    {
        // Encode QuickView with no rotation (because rotate is done by LCD).
        MDPJPGENC_forQuickView(u4SrcAddr, bufInfo, emInputFormat, 0);
        // Transform image format to YUY2 for MDPRotate() use.
        MDPImgTrasn((void*)u4SrcAddr, mu4W_yuv, mu4H_yuv, (void*)mpTempBuf->getVirtAddr(), emInputFormat, MHAL_FORMAT_YUY2); // using mpSreResize for temp format transfer
                                                #if (HDR_PROFILE_CAPTURE)
                                                DbgTmr.print("HdrProfiling:: Source format chane");
                                                #endif
        // Rotate image.
        if ( (u4Orientation == 0) || (u4Orientation == 2) ) // 0 or 180 degree.
        {
            MDPRotate((void*)mpTempBuf->getVirtAddr(), mu4W_yuv, mu4H_yuv, (void*)mpTemp2Buf->getVirtAddr(), mu4W_yuv, mu4H_yuv); // using mpBlurImg for temp source rotate
        }
        else    // (u4Orientation == 1) || (u4Orientation == 3) // 90 or 270 degree.
        {
            MDPRotate((void*)mpTempBuf->getVirtAddr(), mu4W_yuv, mu4H_yuv, (void*)mpTemp2Buf->getVirtAddr(), mu4H_yuv, mu4W_yuv); // using mpBlurImg for temp source rotate
        }
                                                #if (HDR_PROFILE_CAPTURE)
                                                DbgTmr.print("HdrProfiling:: rotate");
                                                #endif

        // Input rotated image and encode JPG.
        bufInfo = getBufInfo_quickViewEnc();    
        MDPJPGENC(mpTemp2Buf->getVirtAddr(), bufInfo, MHAL_FORMAT_YUY2, u4Orientation);
                                                #if (HDR_PROFILE_CAPTURE)
                                                DbgTmr.print("HdrProfiling:: rotate encode");
                                                #endif

        // Release RotatedQuickViewBuf.
        freeRotatedQuickViewBuf(mrRotatedQVBuf);

    }

#else   // original.
	//	(1) Config ISPParam / MDPParam
	BufInfo const bufInfo = getBufInfo_JpgEnc();
	MY_VERB("[createJpgImage] u4BufVA 0x%x w %d h %d.",bufInfo.u4BufPA, mShotParam.frmJpg.w, mShotParam.frmJpg.h);
	halIDPParam_t	pMDPParam;
	MdpYuvAddr	jpgbuff;

	// For capture mode, SrcW/H is input, YuvW/H is jpeg, RgbW/H is for internal YUV
	// JPEG Config 
	::memset((void*)&pMDPParam, 0, sizeof(halIDPParam_t));
	::memset((void*)&jpgbuff, 0, sizeof(MdpYuvAddr));
	
	rect_t rcSrc, rcDst, rcCrop;
	::memset(&rcSrc, 0, sizeof(rect_t));
	::memset(&rcDst, 0, sizeof(rect_t));
	::memset(&rcCrop,0, sizeof(rect_t));
	rcSrc.w = mu4W_yuv;
	rcSrc.h = mu4H_yuv;
	rcDst.w = mShotParam.frmJpg.w;
	rcDst.h = mShotParam.frmJpg.h;
	mpMdpHal->calCropRect(rcSrc, rcDst, &rcCrop, mShotParam.u4ZoomRatio);
	MY_VERB("[createJpgImage] src:(w,h)=(%d,%d), dst:(w,h)=(%d,%d)", rcSrc.w, rcSrc.h, rcDst.w, rcDst.h);
	MY_VERB("[createJpgImage] crop:(x,y,w,h)=(%d,%d,%d,%d)", rcCrop.x, rcCrop.y, rcCrop.w, rcCrop.h);
		
	pMDPParam.mode = MDP_MODE_CAP_JPG;			// For capture mode
	jpgbuff.y=u4SrcAddr;
//	  jpgbuff.y=mpResultImgBuf->getVirtAddr();	// ===> HDR final result buff.
//	  jpgbuff.y=mpSourceImgBuf[1]->getVirtAddr();

	pMDPParam.Capture.pseudo_src_enable = 1;
	pMDPParam.Capture.pseudo_src_yuv_img_addr = jpgbuff;
	pMDPParam.Capture.pseudo_src_color_format = emInputFormat;	// ===> HDR final result format (YUV422).
//	  pMDPParam.Capture.pseudo_src_color_format = YUV420_Planar;	// YUV420.
	pMDPParam.Capture.src_img_size.w = mu4W_yuv;
	pMDPParam.Capture.src_img_size.h = mu4H_yuv;
	pMDPParam.Capture.src_img_roi.x = rcCrop.x;
	pMDPParam.Capture.src_img_roi.y = rcCrop.y;
	pMDPParam.Capture.src_img_roi.w = rcCrop.w;
	pMDPParam.Capture.src_img_roi.h = rcCrop.h;   
	
	pMDPParam.Capture.b_jpg_path_disen = 0;
	if (pMDPParam.Capture.b_jpg_path_disen == 0)
	{
		pMDPParam.Capture.jpg_img_size.w = mShotParam.frmJpg.w;
		pMDPParam.Capture.jpg_img_size.h = mShotParam.frmJpg.h;
		pMDPParam.Capture.jpg_yuv_color_format = 422;	//integer : 411, 422, 444, 400, 410 etc...
		pMDPParam.Capture.jpg_buffer_addr = bufInfo.u4BufPA;
		pMDPParam.Capture.jpg_buffer_size = mShotParam.frmJpg.w * mShotParam.frmJpg.h * 2;
		pMDPParam.Capture.jpg_quality = mShotParam.u4JpgQValue;    // 39~90
		pMDPParam.Capture.jpg_b_add_soi = 0;					   // 1:Add EXIF 0:none
	}

	pMDPParam.Capture.b_qv_path_en = fgShowQuickView;	// 0: disable QuickView. 1: enable QuickView.
//	  pMDPParam.Capture.b_qv_path_en = 1;	// 0: disable QuickView. 1: enable QuickView.
	if (pMDPParam.Capture.b_qv_path_en == 1)
	{
		pMDPParam.Capture.qv_path_sel =  0;  //0:auto select quick view path 1:QV_path_1 2:QV_path_2
		pMDPParam.Capture.qv_yuv_img_addr.y = mShotParam.frmQv.virtAddr;
		pMDPParam.Capture.qv_img_size.w = mShotParam.frmQv.w;
		pMDPParam.Capture.qv_img_size.h = mShotParam.frmQv.h;
		pMDPParam.Capture.qv_color_format = ABGR8888;
		pMDPParam.Capture.qv_flip = 0;
		pMDPParam.Capture.qv_rotate = 0;
	}

	pMDPParam.Capture.b_ff_path_en =  0;
	
	//	(2) Apply settings.
	ret = 0==(err = mpMdpHal->setConf(&pMDPParam));
	if	(!ret)
	{
		MY_ERR("[createJpgImage] mpMdpHal->setConf fail. ec(%x).", err);
		goto lbExit;
	}

	//	(3) Start
	ret = 0==(err = mpMdpHal->start());
	if	(!ret)
	{
		MY_ERR("[createJpgImage] mpMdpHal->Start - ec(%x).", err);
		goto lbExit;
	}

	//	(4) Wait Done
	ret =	waitJpgCaptureDone();
	if	(!ret)
	{
		MY_ERR("[createJpgImage] waitJpgCaptureDone fail.");
		goto lbExit;
	}

#endif  // For HDR Rotate.

lbExit:

	//	(5) Stop & Reset.
	mpMdpHal->stop();
	ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
	DbgTmr.print();
#endif

	MY_DBG("[createJpgImage] - X. ret: %d.", ret);

	return	ret;

}

#if HDR_USE_THREAD

/*******************************************************************************
*
********************************************************************************/
HdrState_e
Hdr::GetHdrState(void)
{
    return mHdrState;
}


/*******************************************************************************
*
********************************************************************************/
void
Hdr::SetHdrState(HdrState_e eHdrState)
{
    mHdrState = eHdrState;
}

/*******************************************************************************
*
********************************************************************************/
MVOID *mHalCamHdrThread(MVOID *arg)
{
    ::prctl(PR_SET_NAME,"mHalCamHdrThread", 0, 0, 0);   // Give this thread a name.
    ::pthread_detach(::pthread_self()); // Make this thread releases all its resources when it's finish.

    //
    MINT32  err = 0;     // 0: No error.
	MBOOL   ret = MTRUE;
    HdrState_e eHdrState;

    //
    MY_DBG("[mHalCamHdrThread] tid: %d.", gettid());
    eHdrState = pHdrObj->GetHdrState();
    while (eHdrState != HDR_STATE_UNINIT)
    {
        ::sem_wait(&semHdrThread);  
        eHdrState = pHdrObj->GetHdrState();
        MY_DBG("[mHalCamHdrThread] Got semHdrThread. eHdrState: %d.", eHdrState);

        pHdrObj->mHalCamHdrProc(eHdrState);
        ::sem_post(&semHdrThreadBack);

    }

    ::sem_post(&semHdrThreadEnd);
 
	MY_DBG("[mHalCamHdrThread] - X. err: %d.", err);

    return NULL;

}


#if 1
/*******************************************************************************
*
********************************************************************************/
MINT32
Hdr::mHalCamHdrProc(HdrState_e eHdrState)
{
    MY_DBG("[mHalCamHdrProc] - E. eHdrState: %d.", eHdrState);

    #if (HDR_PROFILE_CAPTURE2)
	MyDbgTimer DbgTmr("mHalCamHdrProc");
    #endif

	MBOOL	ret = MTRUE;

    //
//    if (!mfdObj)
//    {
//        break; 
//    }
//    ///
//    mfdObj->lock();


    switch (eHdrState)
    {
        case HDR_STATE_INIT:
        MY_DBG("[mHalCamHdrProc] HDR_STATE_INIT.");
        break;

        case HDR_STATE_NORMALIZATION:
        {
            MY_DBG("[mHalCamHdrProc] HDR_STATE_NORMALIZATION.");
        	ret =
        			//	()	Normalize small images, and put them back to SmallImg[].
        			do_Normalization()
                                                    #if (HDR_PROFILE_CAPTURE2)
                                                    &&	DbgTmr.print("HdrProfiling:: do_Normalization Time")
                                                    #endif
        			;
        }
        break;

        case HDR_STATE_FEATURE_EXTRACITON:
        {
            MY_DBG("[mHalCamHdrProc] HDR_STATE_FEATURE_EXTRACITON.");
        	ret =
        			//	()	Do Feature Extraciton.
        			do_FeatureExtraction()
                                                    #if (HDR_PROFILE_CAPTURE2)
                                                    &&	DbgTmr.print("HdrProfiling:: do_FeatureExtraction Time")
                                                    #endif
        			;
        }
        break;

        case HDR_STATE_ALIGNMENT:
        {
            MY_DBG("[mHalCamHdrProc] HDR_STATE_ALIGNMENT.");
            ret =
            		//	()	Do Alignment (includeing "Feature Matching" and "Weighting Map Generation").
            		do_Alignment()
                                                    #if (HDR_PROFILE_CAPTURE2)
                                                    &&	DbgTmr.print("HdrProfiling:: do_Alignment Time")
                                                    #endif
            		;
        }
        break;

        case HDR_STATE_FUSION:
        {
            MY_DBG("[mHalCamHdrProc] HDR_STATE_FUSION.");
        	ret =
        			//	()	Do Fusion.
        			do_Fusion()
                                                    #if (HDR_PROFILE_CAPTURE2)
                                                    &&	DbgTmr.print("HdrProfiling:: do_Fusion Time")
                                                    #endif
        			;
        }
        break;

        case HDR_STATE_UNINIT:
        MY_DBG("[mHalCamHdrProc] HDR_STATE_UNINIT.");
        // Do nothing. Later will leave while() and post semHdrThreadEnd to indicate that mHalCamHdrThread is end and is safe to uninit.
        break;

        default:
            MY_DBG("[mHalCamHdrProc] undefined HDR_STATE, do nothing.");
            
    }


    //
//    if (mfdObj)
//    {
//        mfdObj->unlock();
//    }

    //

    #if (HDR_PROFILE_CAPTURE2)
	DbgTmr.print("HdrProfiling:: mHalCamHdrProc Finish.");
    #endif

    MY_DBG("[mHalCamHdrProc] - X. ret: %d.", ret);

    return ret;

}
#endif

#endif  // HDR_USE_THREAD

