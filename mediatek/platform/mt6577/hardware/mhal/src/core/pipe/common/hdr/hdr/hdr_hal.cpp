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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "HdrHal"


#include <stdlib.h>
#include <stdio.h>

#include <utils/threads.h>	// For Mutex.
#include <cutils/atomic.h>	// For android_atomic_*().
#include <cutils/xlog.h>	// For XLOG?() macros.


//#include "m4u_lib.h"		// For MT6573M4UDrv.
//#include <cutils/pmem.h>	// For pmem_alloc_sync()/pmem_get_phys()/pmem_free().

#include "hdr_hal.h"
#include "MTKHdr.h"
#include "MTKMav.h"	// For MavInitInfo struct
#include "MTKMavCommon.h"	// For mav_rec_par_struct/MavMatchImagePairStruct struct.

//#include "mdp_drv.h"	// For MdpDrv*().	// For mdp_path.h.
//#include "mdp_path.h"	// For 75. For MdpPathStnrParameter struct.


#include "camera_custom_hdr.h"	// For HDR Customer Parameters in Customer Folder.




/**************************************************************************
 *						D E F I N E S / M A C R O S 					  *
 **************************************************************************/
#define MULTI_CORE_MAV	1

// Log level below ERROR will be disabled at user-mp production load. 
// Do not put qualified "FATAL/ERROR  level" log at level WARNING or below.
#define HDR_HAL_TAG			"{HdrHal} "

#define LOG_LEVEL_SILENT	8
#define LOG_LEVEL_ASSERT	7
#define LOG_LEVEL_ERROR		6
#define LOG_LEVEL_WARN		5
#define LOG_LEVEL_INFO		4
#define LOG_LEVEL_DEBUG		3
#define LOG_LEVEL_VERBOSE	2
#define HDR_HAL_LOG_LEVEL	LOG_LEVEL_DEBUG

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_SILENT)		// 8
	#define HDR_ASSERT(cond, fmt, arg...)
	#define HDR_LOGE(fmt, arg...)
	#define HDR_LOGW(fmt, arg...)
	#define HDR_LOGI(fmt, arg...)
	#define HDR_LOGD(fmt, arg...)
	#define HDR_LOGV(fmt, arg...)
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_SILENT)

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_ASSERT)		// 7
	#undef  HDR_ASSERT
    #define HDR_ASSERT(expr, fmt, arg...)                                   \
        do {                                                                \
            if (!(expr))                                                    \
                XLOGE("%s, Line%s: ASSERTION FAILED!: " fmt, __FUNCTION__, __LINE__, ##arg);   \
        } while (0)
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_ASSERT)

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_ERROR)		// 6
	#undef  HDR_LOGE
	#define HDR_LOGE(fmt, arg...)	XLOGE(HDR_HAL_TAG "[%s, line%04d] " fmt, __FILE__, __LINE__, ##arg)	// When MP, will only show log of this level. // <Fatal>: Serious error that cause program can not execute. <Error>: Some error that causes some part of the functionality can not operate normally.
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_ERROR)

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_WARN)		// 5
	#undef  HDR_LOGW
	#define HDR_LOGW(fmt, arg...)	XLOGW(HDR_HAL_TAG "[%s] " fmt, __FUNCTION__, ##arg)	// <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_WARN)

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_INFO)		// 4
	#undef  HDR_LOGI
	#define HDR_LOGI(fmt, arg...)	XLOGI(HDR_HAL_TAG "[%s] " fmt, __FUNCTION__, ##arg)	// <Info>: Show general system information. Like OS version, start/end of Service...
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_INFO)

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_DEBUG)		// 3
	#undef  HDR_LOGD
	#define HDR_LOGD(fmt, arg...)	XLOGD(HDR_HAL_TAG "[%s] " fmt, __FUNCTION__, ##arg)	// <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_DEBUG)

#if (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_VERBOSE)	// 2
	#undef  HDR_LOGV
	#define HDR_LOGV(fmt, arg...)	XLOGV(HDR_HAL_TAG "[%s] " fmt, __FUNCTION__, ##arg)	// <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
#endif	// (HDR_HAL_LOG_LEVEL <= LOG_LEVEL_VERBOSE)


#define HDR_HAL_PROFILE	 0	// Profile HDR execution time.

#define EARLY_MAV_INIT	0	// FIXME: Move MAV init from HDR::init() to Do_FeatureExtraction(). Should move back when MAV init issue is fixed.

/**************************************************************************
 *	   E N U M / S T R U C T / T Y P E D E F	D E C L A R A T I O N	  *
 **************************************************************************/

/**************************************************************************
 *				   E X T E R N A L	  R E F E R E N C E S				  *
 **************************************************************************/

/**************************************************************************
 *						   G L O B A L	  D A T A						  *
 **************************************************************************/
static MINT32 G_i4GMV2[10];
static HdrHalBase *pHdrHal = NULL;
static MUINT32 GS_u4OutputFrameNum = 0;	// Output Frame Num passed by HDR scenario, used in Pipe layer.

//#include "Hdr_Core.h"
//extern CropInfo crop_info[HDR_MAX_IMAGE_NUM];

/*******************************************************************************
*
********************************************************************************/
HdrHalBase*
HdrHal::
getInstance()
{
	HDR_LOGD("getInstance.\n");

    if (pHdrHal == NULL)
	{
        pHdrHal = new HdrHal();
		if (pHdrHal == NULL)
		{
			HDR_LOGE("[getInstance] HdrHal getInstance fail.\n");
		}
    }

    return pHdrHal;

}


/*******************************************************************************
*
********************************************************************************/
MVOID
HdrHal::
destroyInstance()
{
    if (pHdrHal) {
        delete pHdrHal;
    }
    pHdrHal = NULL;
}


/*******************************************************************************
*
********************************************************************************/
HdrHal::HdrHal():HdrHalBase()
{
	mUsers = 0;
	m_pHdrDrv = NULL;
	m_pMavDrv = NULL;
}


/*******************************************************************************
*
********************************************************************************/
HdrHal::~HdrHal()
{

}


/*******************************************************************************
*
********************************************************************************/
MBOOL
HdrHal::init(void *pInitInData)
{
	HDR_LOGD("- E. mUsers: %d.", mUsers);
	HDR_PIPE_INIT_INFO* prHdrPipeInitInfo = (HDR_PIPE_INIT_INFO*)pInitInData;
	HDR_LOGV("ImgW/H: (%d, %d). FinalGainDiff[0/1]: (%d, %d). OutputFrameNum: %d. TargetTone: %d.", prHdrPipeInitInfo->u4ImgW, prHdrPipeInitInfo->u4ImgH, prHdrPipeInitInfo->u4FinalGainDiff0, prHdrPipeInitInfo->u4FinalGainDiff1, prHdrPipeInitInfo->u4OutputFrameNum, prHdrPipeInitInfo->u4TargetTone);
	HDR_LOGV("pSourceImgBufAddr[0/1/2]: (0x%08X, 0x%08X, 0x%08X).", prHdrPipeInitInfo->pSourceImgBufAddr[0], prHdrPipeInitInfo->pSourceImgBufAddr[1], prHdrPipeInitInfo->pSourceImgBufAddr[2]);

    MBOOL   ret = MFALSE;
	MINT32	err = 0;	// 0: No error. other value: error.

	Mutex::Autolock lock(mLock);

	if (mUsers > 0)
	{
		HDR_LOGD("%d has created.", mUsers);
		android_atomic_inc(&mUsers);

		HDR_LOGD("- X. ret: %d.", MTRUE);
		return MTRUE;
	}

	// Create HdrDrv instance.
    m_pHdrDrv = MTKHdr::createInstance();
	if (!m_pHdrDrv)
	{
		HDR_LOGD("MTKHdr::createInstance() fail.");
		goto create_fail_exit;
	}

	// Allocate working buffer needed by Drv (HdrDrv and MavDrv).

	// Init HdrDrv object.
    //     Fill init data.
    HDR_SET_ENV_INFO_STRUCT rHdrInitInfo;

	rHdrInitInfo.image_num		= prHdrPipeInitInfo->u4OutputFrameNum;
	GS_u4OutputFrameNum			= prHdrPipeInitInfo->u4OutputFrameNum;	// Record u4OutputFrameNum for HDR Pipe to use.
	rHdrInitInfo.ev_gain1		= (MUINT16)prHdrPipeInitInfo->u4FinalGainDiff0;
	rHdrInitInfo.ev_gain2		= 1024; 	// Fix at 1024.
	rHdrInitInfo.ev_gain3		= (MUINT16)prHdrPipeInitInfo->u4FinalGainDiff1;
	rHdrInitInfo.target_tone	= prHdrPipeInitInfo->u4TargetTone;
	rHdrInitInfo.image_width	= prHdrPipeInitInfo->u4ImgW;
    rHdrInitInfo.image_height	= prHdrPipeInitInfo->u4ImgH;
	rHdrInitInfo.image_addr[0]	= prHdrPipeInitInfo->pSourceImgBufAddr[0];
	rHdrInitInfo.image_addr[1]	= prHdrPipeInitInfo->pSourceImgBufAddr[1];
	rHdrInitInfo.image_addr[2]	= prHdrPipeInitInfo->pSourceImgBufAddr[2];

    rHdrInitInfo.hdr_tuning_data.BlurRatio	= CustomHdrBlurRatioGet();

    for (MUINT32 i = 0; i < 11; i++)
    {
		rHdrInitInfo.hdr_tuning_data.Gain[i]	= CustomHdrGainArrayGet(i);
    }
 
    rHdrInitInfo.hdr_tuning_data.BottomFlareRatio	= CustomHdrBottomFlareRatioGet();
    rHdrInitInfo.hdr_tuning_data.TopFlareRatio		= CustomHdrTopFlareRatioGet();
    rHdrInitInfo.hdr_tuning_data.BottomFlareBound	= CustomHdrBottomFlareBoundGet();
    rHdrInitInfo.hdr_tuning_data.TopFlareBound		= CustomHdrTopFlareBoundGet();

    rHdrInitInfo.hdr_tuning_data.ThHigh				= CustomHdrThHighGet();
    rHdrInitInfo.hdr_tuning_data.ThLow				= CustomHdrThLowGet();
    
    rHdrInitInfo.hdr_tuning_data.TargetLevelSub		= CustomHdrTargetLevelSubGet();
    rHdrInitInfo.hdr_tuning_data.CoreNumber		    = CustomHdrCoreNumberGet();

	HDR_LOGV("rHdrInitInfo:: ImgW/H: (%d, %d). FinalGainDiff[0/1]: (%d, %d). OutputFrameNum: %d. TargetTone: %d.", rHdrInitInfo.image_width, rHdrInitInfo.image_height, rHdrInitInfo.ev_gain1, rHdrInitInfo.ev_gain3, rHdrInitInfo.image_num, rHdrInitInfo.target_tone);
	HDR_LOGV("rHdrInitInfo:: pSourceImgBufAddr[0/1/2]: (0x%08X, 0x%08X, 0x%08X).", rHdrInitInfo.image_addr[0], rHdrInitInfo.image_addr[1], rHdrInitInfo.image_addr[2]);
	HDR_LOGV("rHdrInitInfo:: BlurRatio: %d. BottomFlareRatio: %f. TopFlareRatio: %f. BottomFlareBound:	%d. TopFlareBound: %d.",
		rHdrInitInfo.hdr_tuning_data.BlurRatio,
		rHdrInitInfo.hdr_tuning_data.BottomFlareRatio,
		rHdrInitInfo.hdr_tuning_data.TopFlareRatio,
		rHdrInitInfo.hdr_tuning_data.BottomFlareBound,
		rHdrInitInfo.hdr_tuning_data.TopFlareBound
	);
	HDR_LOGD("rHdrInitInfo:: ThHigh: %d. ThLow: %d. TargetLevelSub: %d. CoreNumber: %d.",
		rHdrInitInfo.hdr_tuning_data.ThHigh,
		rHdrInitInfo.hdr_tuning_data.ThLow,
		rHdrInitInfo.hdr_tuning_data.TargetLevelSub,
		rHdrInitInfo.hdr_tuning_data.CoreNumber
	);
    for (MUINT32 i = 0; i < 11; i++)
		HDR_LOGV("rHdrInitInfo:: u4Gain[%d]: %d.", i, rHdrInitInfo.hdr_tuning_data.Gain[i]);

    //     Call HdrDrv init.
    err = m_pHdrDrv->HdrInit(&rHdrInitInfo, 0);
	if (err)	// if ret != 0 means error happened.
	{
		HDR_LOGD("m_pHdrDrv->HdrInit() fail.");
		goto create_fail_exit;
	}

#if EARLY_MAV_INIT
	// Create MavDrv instance.
	m_pMavDrv = MTKMav::createInstance(DRV_MAV_OBJ_SWHDR);
	if (!m_pMavDrv)
	{
		HDR_LOGD("MTKMav::createInstance() fail.");
		goto create_fail_exit;
	}

	// Init MavDrv object.
    //     Fill init data.
	MavInitInfo rMavInitInfo;
	rMavInitInfo.WorkingBuffAddr = (MUINT32)global_buffer_rectify;	// FIXME: Allocate working buffer for MAV.
    rMavInitInfo.pTuningInfo = NULL;
    //     Call MavDrv init.
	err = m_pMavDrv->MavInit(&rMavInitInfo, 0);
	if (err)	// if ret != 0 means error happened.
	{
		HDR_LOGD("m_pMavDrv->MavInit() fail. err: %d.", err);
		goto create_fail_exit;
	}
#endif	// EARLY_MAV_INIT

	android_atomic_inc(&mUsers);

	ret = MTRUE;
	HDR_LOGD("- X. ret: %d.", ret);
	return ret;

create_fail_exit:

	// HdrDrv Init failed, destroy HdrDrv instance.
	if (m_pHdrDrv)
	{
		m_pHdrDrv->destroyInstance();
		m_pHdrDrv = NULL;
	}

#if EARLY_MAV_INIT
	// MavDrv Init failed, destroy MavDrv instance.
	if (m_pMavDrv)
	{
		m_pMavDrv->destroyInstance();
		m_pMavDrv = NULL;
	}
#endif	// EARLY_MAV_INIT

	HDR_LOGD("- X. ret: %d.", ret);
	return ret;	// 0: No error. other value: error.

}


/*******************************************************************************
*
********************************************************************************/
MBOOL
HdrHal::uninit()
{
	HDR_LOGD("- E. mUsers: %d.", mUsers);
    MBOOL   ret = MTRUE;

	Mutex::Autolock lock(mLock);

	// If no more users, return directly and do nothing.
	if (mUsers <= 0)
	{
		HDR_LOGD("- X. ret: %d.", ret);
		return ret;
	}

	// More than one user, so decrease one User.
	android_atomic_dec(&mUsers);

	if (mUsers == 0) // There is no more User after decrease one User, then destroy IspDrv instance.
	{
		// Destroy HdrDrv instance.
		if (m_pHdrDrv)
		{
			m_pHdrDrv->destroyInstance();
			m_pHdrDrv = NULL;
		}

		#if EARLY_MAV_INIT
		// Destroy MavDrv instance.
		if (m_pMavDrv)
		{
			m_pMavDrv->destroyInstance();
			m_pMavDrv = NULL;
		}
		#endif	// EARLY_MAV_INIT
	}
	else	// There are still some users.
	{
		HDR_LOGD("Still %d users.", mUsers);
	}

	HDR_LOGD("- X. ret: %d.", ret);
	return ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Do Y normalization for small images.
///
/// Normalize small images to 0EV. Input small images (e.g. 2M, Y800
/// format) and output normalized small images (e.g. 2M, Y800. Normalized
/// result images are generated internally, won't pass outsize.).
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::Do_Normalization(void)
{
	HDR_LOGD("- E.");
    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.

    ret = ( 0 == (err = m_pHdrDrv->HdrMain(HDR_STATE_NORMALIZE)) );  // Do Y normalization
	if (err)	// if ret != 0 means error happened.
	{
		HDR_LOGD("m_pHdrDrv->HdrMain(HDR_STATE_NORMALIZE) fail. err: %d.", err);
		uninit();
	}

	HDR_LOGD("- X. ret: %d.", ret);
	return ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Do SW EIS.
///
/// @param [IN]  u4SwEisImgBuffAddr		SW EIS image buffer (contains 3
///										images for SW EIS).
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::Do_SwEis(HDR_PIPE_SW_EIS_INPUT_INFO& rHdrPipeSwEisInputInfo)
{
	HDR_LOGD("- E.");

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.
	MUINT32 u4Width = 0, u4Height = 0;	// Width/Height of SW EIS image.
	EIS_INPUT_IMG_INFO EISImgInfo[HDR_MAX_IMAGE_NUM];

	QuerySwEisImgResolution(u4Width, u4Height);

    for(MUINT32 i = 0; i < GS_u4OutputFrameNum; i++)
	{
	    EISImgInfo[i].eis_image_width	= rHdrPipeSwEisInputInfo.u2SwEisImgWidth;
	    EISImgInfo[i].eis_image_height	= rHdrPipeSwEisInputInfo.u2SwEisImgHeight;
	    EISImgInfo[i].eis_image_addr	= rHdrPipeSwEisInputInfo.pSwEisImgBufAddr[i];	// /4: u4SwEisImgBuffAddr is UINT32, not UINT8, so + 1 jumps 4 bytes, not 1 byte.
    }

	ret =	( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SET_EIS_INPUT_IMG, EISImgInfo, NULL)) )	// Set SW EIS info.
	    &&	( 0 == (err = m_pHdrDrv->HdrMain(HDR_STATE_SWEIS)) )										// Do SW EIS.
	    &&	( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_GET_GMV, NULL, G_i4GMV2)) )				// Get GMV.
		;


    for (MUINT32 i = 0; i < 10; i++)
		HDR_LOGD("G_i4GMV2[%d]: %d.", i, G_i4GMV2[i]);

	HDR_LOGD("- X. ret: %d.", ret);
	return ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Do Feature Extraction.
///
/// @param [IN]  u4SmallImgBuffAddr		Resized image buffer (contains 3
///										resized images for Feature Extraction).
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::Do_FeatureExtraction(HDR_PIPE_FEATURE_EXTRACT_INPUT_INFO& rHdrPipeFeatureExtractInputInfo)
{
	HDR_LOGD("- E.");
	HDR_LOGV("rHdrPipeFeatureExtractInputInfo: W/H: (%d, %d). Addr[0/1/2]: (0x%08X, 0x%08X, 0x%08X) .", rHdrPipeFeatureExtractInputInfo.u2SmallImgW, rHdrPipeFeatureExtractInputInfo.u2SmallImgH, rHdrPipeFeatureExtractInputInfo.pSmallImgBufAddr[0], rHdrPipeFeatureExtractInputInfo.pSmallImgBufAddr[1], rHdrPipeFeatureExtractInputInfo.pSmallImgBufAddr[2]);

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.
	MavImageInfo rMavInfo;

    // overall process starts from 2nd image
    for (MUINT32 i = 0; i < GS_u4OutputFrameNum; i++)
    {
		// Add i image
		rMavInfo.Width	 = rHdrPipeFeatureExtractInputInfo.u2SmallImgW;
		rMavInfo.Height	 = rHdrPipeFeatureExtractInputInfo.u2SmallImgH;
		rMavInfo.ImgAddr = rHdrPipeFeatureExtractInputInfo.pSmallImgBufAddr[i];

		if ( i == 0)	// 1st image.
		{
			rMavInfo.MotionValue[0] = 0;
			rMavInfo.MotionValue[1] = 0;
		}
		else	// 2nd and 3rd image.
		{
			rMavInfo.MotionValue[0] = G_i4GMV2[(i-1)*2];
			rMavInfo.MotionValue[1] = G_i4GMV2[(i-1)*2+1];
		}

		HDR_LOGV("rMavInfo[%d]:: W/H: (%d, %d). ImgAddr: 0x%08X. MotionValue[0/1]: (%d, %d).", i, rMavInfo.Width, rMavInfo.Height, rMavInfo.ImgAddr, rMavInfo.MotionValue[0], rMavInfo.MotionValue[1]);

		#if MULTI_CORE_MAV
		ret =	( 0 == (err = m_pMavDrv->MavFeatureCtrl(MAV_FEATURE_ADD_IMAGE, &rMavInfo, 0)) );
		#else	// Not multi-core MAV.
		ret =	( 0 == (err = m_pMavDrv->MavFeatureCtrl(MAV_FEATURE_ADD_IMAGE, &rMavInfo, 0)) )
			&&	( 0 == (err = m_pMavDrv->MavMain()) )	// Feature Extraction
			;
		#endif	//  MULTI_CORE_MAV

		// Leave For_loop if error happened.
		if (! ret)	// if ret != MTRUE means error happened.
	    {
	        goto lbExit;
	    }

    }

	#if MULTI_CORE_MAV
	ret =	( 0 == (err = m_pMavDrv->MavMain()) );	// Feature Extraction
	if (! ret)	// if ret != MTRUE means error happened.
	{
	    goto lbExit;
	}
	#endif	//  MULTI_CORE_MAV

	// Get MAV result, and set for HDR.
	mav_rec_par_struct gRecPar;
	MavMatchImagePairStruct gMyMavMatchImagePair;
    gRecPar.m_Match = &gMyMavMatchImagePair;

	ret =	( 0 == (err = m_pMavDrv->MavFeatureCtrl(MAV_FEATURE_GET_MATCH_PAIR_INFO,NULL,&gRecPar)) )
    	&&	( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SET_REC_PAIR_INFO, &gRecPar, NULL)) )
			;

lbExit:

	HDR_LOGD("- X. ret: %d.", ret);
	return ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Do Alignment (includeing "Feature Matching" and "Weighting Map Generation").
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::Do_Alignment(void)
{
	HDR_LOGD("- E.");

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.

	ret =	( 0 == (err = m_pHdrDrv->HdrMain(HDR_STATE_ALIGNMENT)) )	// Do FM and Weighting table gen.
			;

	HDR_LOGD("- X. ret: %d.", ret);
	return ret;

}

///////////////////////////////////////////////////////////////////////////
/// @brief Do Laplacian pyramid and Fusion.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::Do_Fusion(HDR_PIPE_WEIGHT_TBL_INFO** pprBlurredWeightMapInfo)
{
	HDR_LOGD("- E.");

//    for (MUINT32 i = 0; i < 3; i++)
//    {
//		HDR_LOGD("crop_info[%d]: StartX/Y: (%d, %d). EndX/Y: (%d, %d).",
//			i,
//			crop_info[i].startX,
//			crop_info[i].startY,
//			crop_info[i].endX,
//			crop_info[i].endY
//		);
//    }

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.

    ret =	( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SET_BLUR_WEIGHTING_TBL, pprBlurredWeightMapInfo, NULL)) )
	    &&	( 0 == (err = m_pHdrDrv->HdrMain(HDR_STATE_FUSION)) )	//Do Laplacian pyramid and Fusion
		;

	HDR_LOGD("- X. ret: %d.", ret);
	return ret;

}



///////////////////////////////////////////////////////////////////////////
/// @brief Get Weighting Map info.
///
/// @param [OUT]  pprWeightMapInfo		Weighting Map info include width, height, addresses.
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::WeightingMapInfoGet(HDR_PIPE_WEIGHT_TBL_INFO** pprWeightMapInfo)
{
	HDR_LOGD("- E.");

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.

	ret = ( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_GET_WEIGHTING_TBL, NULL, pprWeightMapInfo)) );

//	pprWeightMapInfo = (HDR_PIPE_WEIGHT_TBL_INFO**)OriWeight;

	HDR_LOGD("- X. ret: %d.", ret);
	return ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Get HDR final result.
///
/// @param [OUT] prHdrResult		A pointer pointing to HDR result (including width, height, address).
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::HdrCroppedResultGet(HDR_PIPE_HDR_RESULT_STRUCT& rHdrCroppedResult)
{
	HDR_LOGD("- E.");

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.

    ret = ( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_GET_RESULT, 0, &rHdrCroppedResult)) );


	HDR_LOGV("rHdrResult:: W/H: (%d, %d). Addr: 0x%08X. Size: %d.", rHdrCroppedResult.output_image_width, rHdrCroppedResult.output_image_height, rHdrCroppedResult.output_image_addr, rHdrCroppedResult.output_image_width * rHdrCroppedResult.output_image_height * 3 / 2);


	HDR_LOGD("- X. ret: %d.", ret);
	return ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Clear SW HDR setting.
///
/// Reset HDR Drv, and uninit HDR hal.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::HdrSettingClear(void)
{
	HDR_LOGD("- E.");

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.

    ret =	( 0 == (err = m_pHdrDrv->HdrReset()) )
		&&	uninit()
		;
	
	HDR_LOGD("- X. ret: %d.", ret);
	return ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Set 3 SmallImg Buffer addresses to HDR Drv.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::HdrSmallImgBufSet(HDR_PIPE_CONFIG_PARAM& rHdrPipeConfigParam)
{
	HDR_LOGD("- E.");
	HDR_LOGV("rHdrPipeConfigParam.pSmallImgBufAddr[0/1/2]: (0x%08X, 0x%08X, 0x%08X).", rHdrPipeConfigParam.pSmallImgBufAddr[0], rHdrPipeConfigParam.pSmallImgBufAddr[1], rHdrPipeConfigParam.pSmallImgBufAddr[2]);

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.
    HDR_SET_PROC_INFO_STRUCT HDR_SET_PROC_INFO;

    // Set process info (small image addr and working buffer)
	HDR_SET_PROC_INFO.small_image_addr[0] = rHdrPipeConfigParam.pSmallImgBufAddr[0];
	HDR_SET_PROC_INFO.small_image_addr[1] = rHdrPipeConfigParam.pSmallImgBufAddr[1];
	HDR_SET_PROC_INFO.small_image_addr[2] = rHdrPipeConfigParam.pSmallImgBufAddr[2];

    ret = ( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SET_PROC_INFO, &HDR_SET_PROC_INFO, NULL)) );

	HDR_LOGD("- X. ret: %d.", ret);
	return ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Config MAV parameters.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::ConfigMavParam(MUINT32 u4WorkingBuffAddr)
{
	HDR_LOGD("- E. u4WorkingBuffAddr: 0x%08X.", u4WorkingBuffAddr);
    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.
#if (!EARLY_MAV_INIT)
	// Create MavDrv instance.
	m_pMavDrv = MTKMav::createInstance(DRV_MAV_OBJ_SWHDR);
	if (!m_pMavDrv)
	{
		HDR_LOGD("MTKMav::createInstance() fail.");
		goto create_fail_exit;
	}

	// Init MavDrv object.
    //     Fill init data.
	MavInitInfo rMavInitInfo;
	rMavInitInfo.WorkingBuffAddr = u4WorkingBuffAddr;	// FIXME: Allocate working buffer for MAV.
    rMavInitInfo.pTuningInfo = NULL;
    //     Call MavDrv init.
    ret = ( 0 == (err = m_pMavDrv->MavInit(&rMavInitInfo, 0)) );

	if (err)	// if ret != 0 means error happened.
	{
		HDR_LOGD("m_pMavDrv->MavInit() fail. err: %d.", err);
		goto create_fail_exit;
	}
#endif	// EARLY_MAV_INIT
	HDR_LOGD("- X. ret: %d.", ret);
	return ret;

create_fail_exit:

#if (!EARLY_MAV_INIT)
	// MavDrv Init failed, destroy MavDrv instance.
	if (m_pMavDrv)
	{
		m_pMavDrv->destroyInstance();
		m_pMavDrv = NULL;
	}
#endif	// EARLY_MAV_INIT

	HDR_LOGD("- X. ret: %d.", ret);
	return ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Get HDR Working Buffer size.
///
/// Get HDR Working Buffer size. Working Buffer size is obtained from HDR Drv.
/// Important Note: This function can only be used after HDR Drv knows the
///     width and Height of Image, i.e. after HDR Drv init.
///
/// @return HDR Working Buffer Size.
///////////////////////////////////////////////////////////////////////////
MUINT32
HdrHal::HdrWorkingBuffSizeGet(void)
{
    MUINT32 u4HdrWorkingBuffSize = 0;
    HDR_GET_PROC_INFO_STRUCT rHdrGetProcInfo;
    m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_GET_PROC_INFO, NULL, &rHdrGetProcInfo); //Get small image width/height and ask working buf size
	u4HdrWorkingBuffSize = rHdrGetProcInfo.ext_mem_size;

	HDR_LOGV("HdrWorkingBuffSize: %d.", u4HdrWorkingBuffSize);
	return u4HdrWorkingBuffSize;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Get MAV Working Buffer size.
///
/// Get MAV Working Buffer size. Working Buffer size is obtained from MAV DRv.
///
/// @param [IN]  ru4SmallImgWidth		Small Image width.
/// @param [IN]  ru4SmallImgHeight		Small Image height.
/// @return MAV Working Buffer Size.
///////////////////////////////////////////////////////////////////////////
MUINT32
HdrHal::MavWorkingBuffSizeGet(MUINT32 ru4SmallImgWidth, MUINT32 ru4SmallImgHeight)
{
	HDR_LOGV("- E. ru4SmallImgWidth/Height: (%d, %d).", ru4SmallImgWidth, ru4SmallImgHeight);
    MUINT32 u4MavWorkingBuffSize = 0;

    MavImageInfo ImageInfo;
    ImageInfo.Width = ru4SmallImgWidth;
    ImageInfo.Height= ru4SmallImgHeight;
//    m_pMavDrv->MavFeatureCtrl(MAV_FEATURE_GET_WORKBUF_SIZE, &ImageInfo, &u4MavWorkingBuffSize); //Get small image width/height and ask working buf size

	u4MavWorkingBuffSize = ru4SmallImgWidth * ru4SmallImgHeight * 10;	// FIXME: This knowledge (MAV working buffer size) should not be known by HDR Hal.

	HDR_LOGV("- X. MavWorkingBuffSize: %d.", u4MavWorkingBuffSize);

	return u4MavWorkingBuffSize;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Set HDR Working Buffer address and size to HDR Drv.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
HdrHal::HdrWorkingBufSet(MUINT32 u4BufAddr, MUINT32 u4BufSize)
{
	HDR_LOGD("- E. u4BufAddr: 0x%08X. u4BufSize: %d.", u4BufAddr, u4BufSize);

    MBOOL   ret = MTRUE;
	MINT32	err = 0;	// 0: No error.
    HDR_SET_WORK_BUF_INFO_STRUCT HDR_SET_WORK_BUF_INFO;

	// Allocate workin buffer
	HDR_SET_WORK_BUF_INFO.ext_mem_size = u4BufSize;
	HDR_SET_WORK_BUF_INFO.ext_mem_start_addr = u4BufAddr; 
    ret = ( 0 == (err = m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SET_WORK_BUF_INFO, &HDR_SET_WORK_BUF_INFO, NULL)) );

	HDR_LOGD("- X. ret: %d.", ret);
	return ret;
}


///////////////////////////////////////////////////////////////////////////
/// @brief Query small image width/height.
/// 
/// Important Note: This function can only be used after HDR Drv knows the
///     width and Height of Image, i.e. after HDR Drv init.
////
/// @param [OUT]  ru4Width		SW EIS Image width.
/// @param [OUT]  ru4Height		SW EIS Image height.
///////////////////////////////////////////////////////////////////////////
void
HdrHal::QuerySmallImgResolution(MUINT32& ru4Width, MUINT32& ru4Height)
{
    HDR_GET_PROC_INFO_STRUCT rHdrGetProcInfo;

    m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_GET_PROC_INFO, NULL, &rHdrGetProcInfo); //Get small image width/height and ask working buf size

    ru4Width    = rHdrGetProcInfo.small_image_width;
    ru4Height   = rHdrGetProcInfo.small_image_height;

	HDR_LOGV("SmallImg W/H: (%d, %d).", ru4Width, ru4Height);

}


///////////////////////////////////////////////////////////////////////////
/// @brief Query SW EIS image width/height.
///
/// @param [OUT]  ru4Width		SW EIS Image width.
/// @param [OUT]  ru4Height		SW EIS Image height.
///////////////////////////////////////////////////////////////////////////
void
HdrHal::QuerySwEisImgResolution(MUINT32& ru4Width, MUINT32& ru4Height)
{
    ru4Width    = EIS_WIDTH2;	// FIXME: Should be replaced by customer parameter.
    ru4Height   = EIS_HEIGHT2;	// FIXME: Should be replaced by customer parameter.

	HDR_LOGV("SW EIS W/H: (%d, %d).", ru4Width, ru4Height);

}


///////////////////////////////////////////////////////////////////////////
/// @brief Get SW EIS Image Buffer size.
///
/// @return SW EIS Image Buffer Size.
///////////////////////////////////////////////////////////////////////////
MUINT32
HdrHal::SwEisImgBuffSizeGet(void)
{
	HDR_LOGD("- E.");
	MUINT32 u4Width = 0, u4Height = 0;	// Width/Height of SW EIS image.
	MUINT32 SwEisImgBuffSize = 0;

	QuerySwEisImgResolution(u4Width, u4Height);

	SwEisImgBuffSize = u4Width * u4Height * HDR_MAX_IMAGE_NUM;

	HDR_LOGD("- X. SwEisImgBuffSize: %d.", SwEisImgBuffSize);
	return SwEisImgBuffSize;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Get SW EIS Image Buffer size.
///
/// @return SW EIS Image Buffer Size.
///////////////////////////////////////////////////////////////////////////
void
HdrHal::SaveHdrLog(MUINT32 u4RunningNumber)
{
	HDR_LOGD("- E.");
    m_pHdrDrv->HdrFeatureCtrl(HDR_FEATURE_SAVE_LOG, (void*)u4RunningNumber ,NULL);
	HDR_LOGD("- X.");
}



