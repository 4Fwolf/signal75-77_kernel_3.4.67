/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *	   LEGAL DISCLAIMER
 *
 *	   (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *	   BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *	   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *	   FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *	   ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *	   INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *	   A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *	   WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *	   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *	   ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *	   NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *	   OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *	   BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *	   RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *	   FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *	   THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *	   OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include "MyHdr.h"
#include <linux/cache.h>
#include <mhal_misc.h>
#include <cutils/properties.h>  // For property_get().
#include <camera_custom_if.h>
#include "camera_custom_hdr.h"	// For CustomHdrProlongedVdGet().

#include "mhal_interface.h" // For Mt6575_mHalBitblt().



/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
updateInfo()
{
	MY_DBG("[updateInfo] - E.");

	MBOOL	ret = MTRUE;
	MINT32	err = 0;


	ret =
			//	()	Determine the capture mode.
			decideCaptureMode()
			//	()	Get the ISP Bayer/YUV Resolutions.
		&&	queryIspBayerResolution(mu4W_raw, mu4H_raw)
		&&	queryIspYuvResolution(mu4W_yuv, mu4H_yuv)
			;
	if	( ! ret )
	{
		goto lbExit;
	}

lbExit:

	MY_DBG("[updateInfo] - X ret: %d.", ret);

	return	ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
decideCaptureMode()
{
	MY_DBG("[decideCaptureMode] - E.");

	MINT32	ret = MTRUE;
	MINT32	err = 0;

	//	decide capture mode by AE info.
	Hal3A_HDROutputParam_T rHdrOutputParam;

	ret = ( 0 == (err = mpHal3A->getHDRCapInfo(rHdrOutputParam)) );
	if	(err)
	{
		MY_ERR("[decideCaptureMode] mpHal3A->getHDRCapInfo() fail. ec(%x)", err);
		goto lbExit;
	}

	MY_DBG("[decideCaptureMode] OutputFrameNum: %d. FinalGainDiff[0/1]: (%d, %d). TargetTone: %d.",
		rHdrOutputParam.u4OutputFrameNum,
		rHdrOutputParam.u4FinalGainDiff[0],
		rHdrOutputParam.u4FinalGainDiff[1],
		rHdrOutputParam.u4TargetTone
	);

	// Record value for later use.
	mu4OutputFrameNum	= rHdrOutputParam.u4OutputFrameNum;
	mu4FinalGainDiff[0]	= rHdrOutputParam.u4FinalGainDiff[0];
	mu4FinalGainDiff[1]	= rHdrOutputParam.u4FinalGainDiff[1];
	mu4TargetTone		= rHdrOutputParam.u4TargetTone;

lbExit:

	MY_DBG("[decideCaptureMode] - X. ret: %d.", ret);

	return	ret;

}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
requestRawImgBuf(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[requestRawImgBuf] - E. u4OutputFrameNum: %d", u4OutputFrameNum);
	MBOOL	ret = MTRUE;

	//Vent@20120723: For original HDR source image size, "mu4W_yuv*mu4H_yuv*3/2
	//    is enough (because it's YUV420 format). But when HDR image needs to be rotated,
	//    SourceBuf will be used to contain the image after transform and rotation,
	//    which format is YUY2 (YUV422 format). So we have to allocate a larger size.
//	mu4SourceSize = mu4W_yuv * mu4H_yuv * 3 / 2;	// *3/2: YUV420 size.
	mu4RawSize = mu4W_raw * mu4H_raw * 2;	// *3/2: YUV420 size.
	//mu4SourceSize = getAlignedSize(mu4W_yuv * mu4H_yuv * 3 / 2);	// *3/2: YUV420 size.	// Size will be already aligned in new mHalCamMemPool().

#if 0	 // szPoolName[] is not const, somehow the file names will be messed up.
	MY_VERB("[requestBufs] mu4SourceSize: %d.", mu4SourceSize);
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		char szPoolName[100];
		szPoolName[0] = '\0';
		::sprintf(szPoolName, "%s%d", "HdrRawImgBuf", i);

		mpSourceImgBuf[i]	= new mHalCamMemPool(szPoolName, mu4SourceSize , 0);
		if (! mpRawImgBuf[i])	// mpRawImgBuf[i] is NULL, allocation fail.
		{
			MY_ERR("[requestBufs] mpRawImgBuf[%d] fails to request %d bytes.", i, mu4RawSize);
			ret = MFALSE;
			goto lbExit;
		}

		MY_VERB("[requestRawImgBuf] mpRawImgBuf[%d]->getPoolName(): %s.", i,     mpRawImgBuf[i]->getPoolName());
		MY_VERB("[requestRawImgBuf] mpRawImgBuf[%d]->getVirtAddr(): 0x%08X.", i, mpRawImgBuf[i]->getVirtAddr());
		MY_VERB("[requestRawImgBuf] mpRawImgBuf[%d]->getPhyAddr() : 0x%08X.", i, mpRawImgBuf[i]->getPhyAddr());
		MY_VERB("[requestRawImgBuf] mpRawImgBuf[%d]->getPoolSize(): %d.", i,     mpRawImgBuf[i]->getPoolSize());
	}

#else

	switch (u4OutputFrameNum)	// Allocate buffers according to u4OutputFrameNum, note that there are no "break;" in case 3/case 2.
	{
		case 3:
		mpRawImgBuf[2]	= new mHalCamMemPool("HdrRawImgBuf2", mu4RawSize, 0);
		case 2:
		mpRawImgBuf[1]	= new mHalCamMemPool("HdrRawImgBuf1", mu4RawSize, 0);
		case 1:
		mpRawImgBuf[0]	= new mHalCamMemPool("HdrRawImgBuf0", mu4RawSize, 0);
		break;
	}

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		MY_VERB("[requestRawImgBuf] mu4RawSize: %d.", mu4RawSize);
		MY_VERB("[requestRawImgBuf] mpRawImgBuf[%d]->getPoolName(): %s.",		i, mpRawImgBuf[i]->getPoolName());
		MY_VERB("[requestRawImgBuf] mpRawImgBuf[%d]->getVirtAddr(): 0x%08X.",	i, mpRawImgBuf[i]->getVirtAddr());
		MY_VERB("[requestRawImgBuf] mpRawImgBuf[%d]->getPhyAddr() : 0x%08X.",	i, mpRawImgBuf[i]->getPhyAddr());
		MY_VERB("[requestRawImgBuf] mpRawImgBuf[%d]->getPoolSize(): %d.",		i, mpRawImgBuf[i]->getPoolSize());
	}

#endif

lbExit:
	if	( ! ret )
	{
		releaseRawImgBuf();
	}

	MY_DBG("[requestRawImgBuf] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
releaseRawImgBuf(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[releaseRawImgBuf] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL	ret = MTRUE;

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		if	(mpRawImgBuf[i])
		{
			delete mpRawImgBuf[i];
			mpRawImgBuf[i] = NULL;
		}
	}

	MY_DBG("[releaseRawImgBuf] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
requestSourceImgBuf(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[requestSourceImgBuf] - E. u4OutputFrameNum: %d", u4OutputFrameNum);
	MBOOL	ret = MTRUE;

	//Vent@20120723: For original HDR source image size, "mu4W_yuv*mu4H_yuv*3/2
	//    is enough (because it's YUV420 format). But when HDR image needs to be rotated,
	//    SourceBuf will be used to contain the image after transform and rotation,
	//    which format is YUY2 (YUV422 format). So we have to allocate a larger size.
//	mu4SourceSize = mu4W_yuv * mu4H_yuv * 3 / 2;	// *3/2: YUV420 size.
	mu4SourceSize = mu4W_yuv * mu4H_yuv * 2;	// *3/2: YUV420 size.
	//mu4SourceSize = getAlignedSize(mu4W_yuv * mu4H_yuv * 3 / 2);	// *3/2: YUV420 size.	// Size will be already aligned in new mHalCamMemPool().

#if 0	 // szPoolName[] is not const, somehow the file names will be messed up.
	MY_VERB("[requestBufs] mu4SourceSize: %d.", mu4SourceSize);
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		char szPoolName[100];
		szPoolName[0] = '\0';
		::sprintf(szPoolName, "%s%d", "HdrSrcImgBuf", i);

		mpSourceImgBuf[i]	= new mHalCamMemPool(szPoolName, mu4SourceSize , 0);
		if (! mpSourceImgBuf[i])	// mpSourceImgBuf[i] is NULL, allocation fail.
		{
			MY_ERR("[requestBufs] mpSourceImgBuf[%d] fails to request %d bytes.", i, mu4SourceSize);
			ret = MFALSE;
			goto lbExit;
		}

		MY_VERB("[requestBufs] mpSourceImgBuf[%d]->getPoolName(): %s.", i, mpSourceImgBuf[i]->getPoolName());
		MY_VERB("[requestBufs] mpSourceImgBuf[%d]->getVirtAddr(): 0x%08X.", i, mpSourceImgBuf[i]->getVirtAddr());
		MY_VERB("[requestBufs] mpSourceImgBuf[%d]->getPhyAddr() : 0x%08X.", i, mpSourceImgBuf[i]->getPhyAddr());
		MY_VERB("[requestBufs] mpSourceImgBuf[%d]->getPoolSize(): %d.", i, mpSourceImgBuf[i]->getPoolSize());
	}

#else

	switch (u4OutputFrameNum)	// Allocate buffers according to u4OutputFrameNum, note that there are no "break;" in case 3/case 2.
	{
		case 3:
		mpSourceImgBuf[2]	= new mHalCamMemPool("HdrSrcImgBuf2", mu4SourceSize, 0);
		case 2:
		mpSourceImgBuf[1]	= new mHalCamMemPool("HdrSrcImgBuf1", mu4SourceSize, 0);
		case 1:
		mpSourceImgBuf[0]	= new mHalCamMemPool("HdrSrcImgBuf0", mu4SourceSize, 0);
		break;
	}

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		MY_VERB("[requestSourceImgBuf] mu4SourceSize: %d.", mu4SourceSize);
		MY_VERB("[requestSourceImgBuf] mpSourceImgBuf[%d]->getPoolName(): %s.",		i, mpSourceImgBuf[i]->getPoolName());
		MY_VERB("[requestSourceImgBuf] mpSourceImgBuf[%d]->getVirtAddr(): 0x%08X.",	i, mpSourceImgBuf[i]->getVirtAddr());
		MY_VERB("[requestSourceImgBuf] mpSourceImgBuf[%d]->getPhyAddr() : 0x%08X.",	i, mpSourceImgBuf[i]->getPhyAddr());
		MY_VERB("[requestSourceImgBuf] mpSourceImgBuf[%d]->getPoolSize(): %d.",		i, mpSourceImgBuf[i]->getPoolSize());
	}

#endif

lbExit:
	if	( ! ret )
	{
		releaseSourceImgBuf();
	}

	MY_DBG("[requestSourceImgBuf] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
releaseSourceImgBuf(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[releaseSourceImgBuf] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL	ret = MTRUE;

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		if	(mpSourceImgBuf[i])
		{
			delete mpSourceImgBuf[i];
			mpSourceImgBuf[i] = NULL;
		}
	}

	MY_DBG("[releaseSourceImgBuf] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Request Small Image buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
requestSmallImgBuf(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[requestSmallImgBuf] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL	ret = MTRUE;

	// For SmallImg Buffer.
	mpHdrHal->QuerySmallImgResolution(mu4W_small, mu4H_small);

	mu4SmallImgSize = mu4W_small * mu4H_small;	// Y800 size.
	//mu4SmallImgSize = getAlignedSize(mu4W_small * mu4H_small);	// Size will be already aligned in new mHalCamMemPool().

	switch (u4OutputFrameNum)	// Allocate buffers according to u4OutputFrameNum, note that there are no "break;" in case 3/case 2.
	{
		case 3:
		mpSmallImgBuf[2]	= new mHalCamMemPool("HdrSmallImgBuf2", mu4SmallImgSize, 0);
		case 2:
		mpSmallImgBuf[1]	= new mHalCamMemPool("HdrSmallImgBuf1", mu4SmallImgSize, 0);
		case 1:
		mpSmallImgBuf[0]	= new mHalCamMemPool("HdrSmallImgBuf0", mu4SmallImgSize, 0);
		break;
	}

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		MY_VERB("[requestSmallImgBuf] mu4SmallImgSize: %d.", mu4SmallImgSize);
		MY_VERB("[requestSmallImgBuf] mpSmallImgBuf[%d]->getPoolName(): %s.",		i, mpSmallImgBuf[i]->getPoolName());
		MY_VERB("[requestSmallImgBuf] mpSmallImgBuf[%d]->getVirtAddr(): 0x%08X.",	i, mpSmallImgBuf[i]->getVirtAddr());
		MY_VERB("[requestSmallImgBuf] mpSmallImgBuf[%d]->getPhyAddr() : 0x%08X.",	i, mpSmallImgBuf[i]->getPhyAddr());
		MY_VERB("[requestSmallImgBuf] mpSmallImgBuf[%d]->getPoolSize(): %d.",		i, mpSmallImgBuf[i]->getPoolSize());
	}

lbExit:
	if	( ! ret )
	{
		releaseSmallImgBuf();
	}

	MY_DBG("[requestSmallImgBuf] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
releaseSmallImgBuf(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[releaseSmallImgBuf] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL	ret = MTRUE;

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		// For SmallImg Buffer.
		if	(mpSmallImgBuf[i])
		{
			delete mpSmallImgBuf[i];
			mpSmallImgBuf[i] = NULL;
		}
	}

	MY_DBG("[releaseSmallImgBuf] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Request SW EIS Image buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
requestSwEisImgBuf(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[requestSwEisImgBuf] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL	ret = MTRUE;


	// For SW EIS Image Buffer.
	mpHdrHal->QuerySwEisImgResolution(mu4W_sweis, mu4H_sweis);

	mu4SwEisImgSize = getAlignedSize(mu4W_sweis * mu4H_sweis);	// Y800 size.

	switch (u4OutputFrameNum)	// Allocate buffers according to u4OutputFrameNum, note that there are no "break;" in case 3/case 2.
	{
		case 3:
		mpSwEisImgBuf[2]	= new mHalCamMemPool("HdrSwEisImgBuf2", mu4SwEisImgSize, 0);
		case 2:
		mpSwEisImgBuf[1]	= new mHalCamMemPool("HdrSwEisImgBuf1", mu4SwEisImgSize, 0);
		case 1:
		mpSwEisImgBuf[0]	= new mHalCamMemPool("HdrSwEisImgBuf0", mu4SwEisImgSize, 0);
		break;
	}

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		MY_VERB("[requestSwEisImgBuf] mu4SwEisImgSize: %d.", mu4SwEisImgSize);
		MY_VERB("[requestSwEisImgBuf] mpSwEisImgBuf[%d]->getPoolName(): %s.",		i, mpSwEisImgBuf[i]->getPoolName());
		MY_VERB("[requestSwEisImgBuf] mpSwEisImgBuf[%d]->getVirtAddr(): 0x%08X.",	i, mpSwEisImgBuf[i]->getVirtAddr());
		MY_VERB("[requestSwEisImgBuf] mpSwEisImgBuf[%d]->getPhyAddr() : 0x%08X.",	i, mpSwEisImgBuf[i]->getPhyAddr());
		MY_VERB("[requestSwEisImgBuf] mpSwEisImgBuf[%d]->getPoolSize(): %d.",		i, mpSwEisImgBuf[i]->getPoolSize());
	}

lbExit:
	if	( ! ret )
	{
		releaseSwEisImgBuf();
	}

	MY_DBG("[requestSwEisImgBuf] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
releaseSwEisImgBuf(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[releaseSwEisImgBuf] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL	ret = MTRUE;

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		// For SW EIS Image Buffer.
		if	(mpSwEisImgBuf[i])
		{
			delete mpSwEisImgBuf[i];
			mpSwEisImgBuf[i] = NULL;
		}
	}

	MY_DBG("[releaseSwEisImgBuf] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Request MAV working buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
requestMavWorkingBuf(void)
{
	MY_DBG("[requestMavWorkingBuf] - E.");
	MBOOL	ret = MTRUE;

	// For MAV Working Buffer.
//	mu4MavWorkingBufSize = mpHdrHal->MavWorkingBuffSizeGet(mu4W_small, mu4H_small);
	mu4MavWorkingBufSize = mu4W_small * mu4H_small * 25;	// 25: For new Multi-core MAV. 10: For old Single-core MAV.
	//mu4MavWorkingBufSize = getAlignedSize(mu4MavWorkingBufSize);	// Size will be already aligned in new mHalCamMemPool().
	mpMavWorkingBuf = new mHalCamMemPool("MavWorkingBuf", mu4MavWorkingBufSize, 0);

	MY_VERB("[requestMavWorkingBuf] mu4MavWorkingBufSize: %d.", mu4MavWorkingBufSize);
	MY_VERB("[requestMavWorkingBuf] mpHdrWorkingBuf->getPoolName(): %s.",		mpMavWorkingBuf->getPoolName());
	MY_VERB("[requestMavWorkingBuf] mpHdrWorkingBuf->getVirtAddr(): 0x%08X.",	mpMavWorkingBuf->getVirtAddr());
	MY_VERB("[requestMavWorkingBuf] mpHdrWorkingBuf->getPhyAddr() : 0x%08X.",	mpMavWorkingBuf->getPhyAddr());
	MY_VERB("[requestMavWorkingBuf] mpHdrWorkingBuf->getPoolSize(): %d.",		mpMavWorkingBuf->getPoolSize());

lbExit:
	if	( ! ret )
	{
		releaseMavWorkingBuf();
	}

	MY_DBG("[requestMavWorkingBuf] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
releaseMavWorkingBuf(void)
{
	MY_DBG("[releaseMavWorkingBuf] - E.");
	MBOOL	ret = MTRUE;

   	// For MAV Working Buffer.
	if	(mpMavWorkingBuf)
	{
    	delete mpMavWorkingBuf;
    	mpMavWorkingBuf= NULL;
	}

	MY_DBG("[releaseMavWorkingBuf] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Request HDR working buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
requestHdrWorkingBuf(void)
{
	MY_DBG("[requestHdrWorkingBuf] - E.");
	MBOOL	ret = MTRUE;

	// For HDR Working Buffer.
	mu4HdrWorkingBufSize = mpHdrHal->HdrWorkingBuffSizeGet();
	//mu4HdrWorkingBufSize = getAlignedSize(mu4HdrWorkingBufSize);	// Size will be already aligned in new mHalCamMemPool().
	mpHdrWorkingBuf = new mHalCamMemPool("HdrWorkingBuf", mu4HdrWorkingBufSize, 0);

	MY_VERB("[requestHdrWorkingBuf] mu4HdrWorkingBufSize: %d.", mu4HdrWorkingBufSize);
	MY_VERB("[requestHdrWorkingBuf] mpHdrWorkingBuf->getPoolName(): %s.",		mpHdrWorkingBuf->getPoolName());
	MY_VERB("[requestHdrWorkingBuf] mpHdrWorkingBuf->getVirtAddr(): 0x%08X.",	mpHdrWorkingBuf->getVirtAddr());
	MY_VERB("[requestHdrWorkingBuf] mpHdrWorkingBuf->getPhyAddr() : 0x%08X.",	mpHdrWorkingBuf->getPhyAddr());
	MY_VERB("[requestHdrWorkingBuf] mpHdrWorkingBuf->getPoolSize(): %d.",		mpHdrWorkingBuf->getPoolSize());

lbExit:
	if	( ! ret )
	{
		releaseHdrWorkingBuf();
	}

	MY_DBG("[requestHdrWorkingBuf] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
releaseHdrWorkingBuf(void)
{
	MY_DBG("[releaseHdrWorkingBuf] - E.");
	MBOOL	ret = MTRUE;

	// For HDR Working Buffer.
    if (mpHdrWorkingBuf)
    {
    	delete mpHdrWorkingBuf;
    	mpHdrWorkingBuf= NULL;
    }

	MY_DBG("[releaseHdrWorkingBuf] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Request Original WeightingTable buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
requestOriWeightMapBuf(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[requestOriWeightMapBuf] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL	ret = MTRUE;

	//	   Allocate memory for original and blurred Weighting Map.
	MUINT32 u4Size = sizeof(HDR_PIPE_WEIGHT_TBL_INFO*) * u4OutputFrameNum;
	MUINT32 u4AlignedSize = getAlignedSize(u4Size);
	MUINT32 u4TableSize = sizeof(HDR_PIPE_WEIGHT_TBL_INFO);
	MUINT32 u4AlignedTableSize = getAlignedSize(u4TableSize);
	MY_VERB("[requestOriWeightMapBuf] u4Size: %d. u4AlignedSize: %d. u4TableSize: %d. u4AlignedTableSize: %d.", u4Size, u4AlignedSize, u4TableSize, u4AlignedTableSize);

	OriWeight = (HDR_PIPE_WEIGHT_TBL_INFO**)memalign(L1_CACHE_BYTES, u4AlignedSize);
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
		OriWeight[i] = (HDR_PIPE_WEIGHT_TBL_INFO*)memalign(L1_CACHE_BYTES, u4AlignedTableSize);

lbExit:
	if	( ! ret )
	{
		releaseOriWeightMapBuf();
	}

	MY_DBG("[requestOriWeightMapBuf] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Release Original WeightingTable buffers.
///
///	Note: Some info of OriWeightMap are needed when requestBlurredWeightMapBuf(),
///		  so must release it after requestBlurredWeightMapBuf().
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
releaseOriWeightMapBuf(void)
{
	MY_DBG("[releaseOriWeightMapBuf] - E.");
	MBOOL	ret = MTRUE;

    if (OriWeight)
    {
    	delete [] OriWeight;
    	OriWeight = NULL;
    }

	MY_DBG("[releaseOriWeightMapBuf] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Request DownSizedWeightMap buffers.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
requestDownSizedWeightMapBuf(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[requestDownSizedWeightMapBuf] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL	ret = MTRUE;

	mu4W_dsmap = OriWeight[0]->weight_table_width  / 40;
	mu4H_dsmap = OriWeight[0]->weight_table_height / 40;

	//	   Calculate width/height of Down-sized Weighting Map.
	mu4DownSizedWeightMapSize = mu4W_dsmap * mu4H_dsmap;	// Y800 size.
	//mu4DownSizedWeightMapSize = getAlignedSize(mu4W_dsmap * mu4H_dsmap);	// Y800 size.	// Size will be already aligned in new mHalCamMemPool().
	//	   Allocate memory for Down-sized Weighting Map.
	switch (u4OutputFrameNum)	// Allocate buffers according to u4OutputFrameNum, note that there are no "break;" in case 3/case 2.
	{
		case 3:
		mpDownSizedWeightMapBuf[2] = new mHalCamMemPool("DownSizedWeightMap2", mu4DownSizedWeightMapSize, 0);
		case 2:
		mpDownSizedWeightMapBuf[1] = new mHalCamMemPool("DownSizedWeightMap1", mu4DownSizedWeightMapSize, 0);
		case 1:
		mpDownSizedWeightMapBuf[0] = new mHalCamMemPool("DownSizedWeightMap0", mu4DownSizedWeightMapSize, 0);
		break;
	}

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		MY_VERB("[requestDownSizedWeightMapBuf] mu4DownSizedWeightMapSize: %d.", mu4DownSizedWeightMapSize);
		MY_VERB("[requestDownSizedWeightMapBuf] mpDownSizedWeightMapBuf[%d]->getPoolName(): %s.",		i, mpDownSizedWeightMapBuf[i]->getPoolName());
		MY_VERB("[requestDownSizedWeightMapBuf] mpDownSizedWeightMapBuf[%d]->getVirtAddr(): 0x%08X.",	i, mpDownSizedWeightMapBuf[i]->getVirtAddr());
		MY_VERB("[requestDownSizedWeightMapBuf] mpDownSizedWeightMapBuf[%d]->getPhyAddr() : 0x%08X.",	i, mpDownSizedWeightMapBuf[i]->getPhyAddr());
		MY_VERB("[requestDownSizedWeightMapBuf] mpDownSizedWeightMapBuf[%d]->getPoolSize(): %d.",		i, mpDownSizedWeightMapBuf[i]->getPoolSize());
	}

lbExit:
	if	( ! ret )
	{
		releaseDownSizedWeightMapBuf();
	}

	MY_DBG("[requestDownSizedWeightMapBuf] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
releaseDownSizedWeightMapBuf(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[releaseDownSizedWeightMapBuf] - E. u4OutputFrameNum %d.", u4OutputFrameNum);
	MBOOL	ret = MTRUE;

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		// For DownSized Weight Map Buffer.
		if	(mpDownSizedWeightMapBuf[i])
		{
			delete mpDownSizedWeightMapBuf[i];
			mpDownSizedWeightMapBuf[i] = NULL;
		}
	}

	MY_DBG("[releaseDownSizedWeightMapBuf] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Request Blurred WeightingTable buffers. Must execute after OriWeightTbl is gottn.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
requestBlurredWeightMapBuf(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[requestBlurredWeightMapBuf] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL	ret = MTRUE;

	//	   Allocate memory for original and blurred Weighting Map.
	MUINT32 u4Size = sizeof(HDR_PIPE_WEIGHT_TBL_INFO*) * u4OutputFrameNum;
	MUINT32 u4AlignedSize = getAlignedSize(u4Size);
	MUINT32 u4TableSize = sizeof(HDR_PIPE_WEIGHT_TBL_INFO);
	MUINT32 u4AlignedTableSize = getAlignedSize(u4TableSize);
	MY_VERB("[requestBlurredWeightMapBuf] u4Size: %d. u4AlignedSize: %d. u4TableSize: %d. u4AlignedTableSize: %d.", u4Size, u4AlignedSize, u4TableSize, u4AlignedTableSize);

	BlurredWeight = (HDR_PIPE_WEIGHT_TBL_INFO**)memalign(L1_CACHE_BYTES, u4AlignedSize);

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		BlurredWeight[i] = (HDR_PIPE_WEIGHT_TBL_INFO*)memalign(L1_CACHE_BYTES, u4AlignedTableSize);

		// Init BlurredWeight[i], and allocate memory for Blurred Weighting Map.
		BlurredWeight[i]->weight_table_width  = OriWeight[i]->weight_table_width;
		BlurredWeight[i]->weight_table_height = OriWeight[i]->weight_table_height;
		BlurredWeight[i]->weight_table_data = (MUINT8*)malloc(BlurredWeight[i]->weight_table_width * BlurredWeight[i]->weight_table_height);
	}

lbExit:
	if	( ! ret )
	{
		releaseBlurredWeightMapBuf();
	}

	MY_DBG("[requestBlurredWeightMapBuf] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
releaseBlurredWeightMapBuf(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[releaseBlurredWeightMapBuf] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL	ret = MTRUE;

	// Free allocated memory
    if (BlurredWeight)
    {
    	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
    	{
    		delete BlurredWeight[i]->weight_table_data;
    	}
    	delete [] BlurredWeight;
    	BlurredWeight = NULL;
    }

	MY_DBG("[releaseBlurredWeightMapBuf] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Request ResultImg buffer.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
requestResultImgBuf(void)
{
	MY_DBG("[requestResultImgBuf] - E.");
	MBOOL	ret = MTRUE;

	mu4ResultImgSize = mu4W_yuv * mu4H_yuv * 2;	// *2: YUV422 size.
	//mu4ResultImgSize = getAlignedSize(mu4W_yuv * mu4H_yuv * 2);	// *2: YUV422 size.	// Size will be already aligned in new mHalCamMemPool().
	mpResultImgBuf = new mHalCamMemPool("HdrResultImgBuf", mu4ResultImgSize, 0);

	MY_VERB("[requestResultImgBuf] mu4ResultImgSize: %d.", mu4ResultImgSize);
	MY_VERB("[requestResultImgBuf] mpResultImgBuf->getPoolName(): %s.",		mpResultImgBuf->getPoolName());
	MY_VERB("[requestResultImgBuf] mpResultImgBuf->getVirtAddr(): 0x%08X.",	mpResultImgBuf->getVirtAddr());
	MY_VERB("[requestResultImgBuf] mpResultImgBuf->getPhyAddr() : 0x%08X.",	mpResultImgBuf->getPhyAddr());
	MY_VERB("[requestResultImgBuf] mpResultImgBuf->getPoolSize(): %d.",		mpResultImgBuf->getPoolSize());

lbExit:
	if	( ! ret )
	{
		releaseResultImgBuf();
	}

	MY_DBG("[requestResultImgBuf] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
releaseResultImgBuf()
{
	MY_DBG("[releaseResultImgBuf] - E.");
	MBOOL	ret = MTRUE;

    if (mpResultImgBuf)
    {
    	delete mpResultImgBuf;
    	mpResultImgBuf = NULL;
    }

	MY_DBG("[releaseResultImgBuf] - X. ret: %d.", ret);

	return	ret;

}



///////////////////////////////////////////////////////////////////////////
/// @brief Request temp buffer for format change.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
requestTempBuf(void)
{
	MY_DBG("[requestTempBuf] - E.");
	MBOOL	ret = MTRUE;

	mu4TempSize = mu4W_yuv * mu4H_yuv * 2;	// *2: YUV422 size.
	//mu4ResultImgSize = getAlignedSize(mu4W_yuv * mu4H_yuv * 2);	// *2: YUV422 size.	// Size will be already aligned in new mHalCamMemPool().
	mpTempBuf = new mHalCamMemPool("HdrTempBuf", mu4TempSize, 0);

	MY_VERB("[requestTempBuf] mu4TempSize: %d.", mu4TempSize);
	MY_VERB("[requestTempBuf] mpTempBuf->getPoolName(): %s.",		mpTempBuf->getPoolName());
	MY_VERB("[requestTempBuf] mpTempBuf->getVirtAddr(): 0x%08X.",	mpTempBuf->getVirtAddr());
	MY_VERB("[requestTempBuf] mpTempBuf->getPhyAddr() : 0x%08X.",	mpTempBuf->getPhyAddr());
	MY_VERB("[requestTempBuf] mpTempBuf->getPoolSize(): %d.",		mpTempBuf->getPoolSize());

lbExit:
	if	( ! ret )
	{
		releaseTempBuf();
	}

	MY_DBG("[requestTempBuf] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
releaseTempBuf()
{
	MY_DBG("[releaseTempBuf] - E.");
	MBOOL	ret = MTRUE;

    if (mpTempBuf)
    {
    	delete mpTempBuf;
    	mpTempBuf = NULL;
    }

	MY_DBG("[releaseTempBuf] - X. ret: %d.", ret);

	return	ret;

}



///////////////////////////////////////////////////////////////////////////
/// @brief Request temp buffer for rotation.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
requestTemp2Buf(void)
{
	MY_DBG("[requestTemp2Buf] - E.");
	MBOOL	ret = MTRUE;

	mu4Temp2Size = mu4W_yuv * mu4H_yuv * 2;	// *2: YUV422 size.
	//mu4ResultImgSize = getAlignedSize(mu4W_yuv * mu4H_yuv * 2);	// *2: YUV422 size.	// Size will be already aligned in new mHalCamMemPool().
	mpTemp2Buf = new mHalCamMemPool("HdrTemp2Buf", mu4Temp2Size, 0);

	MY_VERB("[requestTemp2Buf] mu4Temp2Size: %d.", mu4Temp2Size);
	MY_VERB("[requestTemp2Buf] mpTemp2Buf->getPoolName(): %s.",		mpTemp2Buf->getPoolName());
	MY_VERB("[requestTemp2Buf] mpTemp2Buf->getVirtAddr(): 0x%08X.",	mpTemp2Buf->getVirtAddr());
	MY_VERB("[requestTemp2Buf] mpTemp2Buf->getPhyAddr() : 0x%08X.",	mpTemp2Buf->getPhyAddr());
	MY_VERB("[requestTemp2Buf] mpTemp2Buf->getPoolSize(): %d.",		mpTemp2Buf->getPoolSize());

lbExit:
	if	( ! ret )
	{
		releaseTemp2Buf();
	}

	MY_DBG("[requestTemp2Buf] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
releaseTemp2Buf()
{
	MY_DBG("[releaseTemp2Buf] - E.");
	MBOOL	ret = MTRUE;

    if (mpTemp2Buf)
    {
    	delete mpTemp2Buf;
    	mpTemp2Buf = NULL;
    }

	MY_DBG("[releaseTemp2Buf] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
setIspMode(MUINT32 const u4Mode)
{
	MINT32 err = 0;

	err = mpIspHal->sendCommand(ISP_CMD_SET_CAM_MODE, u4Mode);
	if	(err)
	{
		MY_ERR("[setIspMode] mpIspHal->sendCommand(ISP_CMD_SET_CAM_MODE, %d) fail. ec(%d)", u4Mode, err);
	}

	return	(0<=err);
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
camera_start(void)
{
	MY_DBG("[camera_start] - E.");

#if (HDR_PROFILE_CAPTURE3)
    MyDbgTimer DbgTmr("camera_start");
#endif

	MINT32 err = 0;

	if	( (err = mpMdpHal->start()) )
	{
		MY_ERR("[camera_start] mpMdpHal->start() fail. ec(%x)", err);
		goto lbExit;
	}
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling:: mpMdpHal->start() Time");
                                        #endif

	if	( (err = mpIspHal->start()) )
	{
		MY_ERR("[camera_start] mpIspHal->start() fail. ec(%x)", err);
		goto lbExit;
	}
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling:: mpIspHal->start() Time");
                                        #endif


	MY_DBG("[camera_start] - X. err: %d.", err);

	return	(0<=err);

lbExit:
	mpIspHal->stop();
	MY_DBG("[camera_start] Stop ISP.");
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling:: mpIspHal->stop() Time");
                                        #endif

	mpMdpHal->stop();
	MY_DBG("[camera_start] Stop MDP.");
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling:: mpMdpHal->stop() Time");
                                        #endif

	MY_DBG("[camera_start] - X. err: %d.", err);

	return	(0<=err);

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
do_Isp(MBOOL const fgDoEis /*= MFALSE*/, MUINT32 const u4Idx /*= 0*/)
{
	MY_DBG("[do_Isp] - E.");

#if (HDR_PROFILE_CAPTURE3)
    MyDbgTimer DbgTmr("do_Isp");
#endif

	MINT32 err = 0;

	if	( (err = mpMdpHal->start()) )
	{
		MY_ERR("[do_Isp] mpMdpHal->start() fail. ec(%x)", err);
		goto lbExit;
	}
                                        #if (HDR_PROFILE_CAPTURE3)
                                        	DbgTmr.print("HdrProfiling:: mpMdpHal->start() Time");
                                        #endif

	if	( (err = mpIspHal->start()) )
	{
		MY_ERR("[do_Isp] mpIspHal->start() fail. ec(%x)", err);
		goto lbExit;
	}
                                        #if (HDR_PROFILE_CAPTURE3)
                                        	DbgTmr.print("HdrProfiling:: mpIspHal->start() Time");
                                        #endif

	// If MDP and ISP start together, wait MDP done is enough.
	if	( (err = mpMdpHal->waitDone(0x01)) )
	{
		MY_ERR("[do_Isp] mpMdpHal->waitDone() fail. ec(%x)", err);
		goto lbExit;
	}
	MY_DBG("[do_Isp] mpMdpHal->waitDone() done.");
                                        #if (HDR_PROFILE_CAPTURE3)
                                        	DbgTmr.print("HdrProfiling:: mpMdpHal->waitDone() Time");
                                        #endif

/*
	if	( (err = mpIspHal->waitDone(ISP_HAL_IRQ_ISPDONE)) )
	{
		MY_ERR("mpIspHal->waitDone() fail. ec(%x)", err);
		goto lbExit;
	}
*/


lbExit:
	mpIspHal->stop();
	MY_DBG("[do_Isp] Stop ISP.");
                                        #if (HDR_PROFILE_CAPTURE3)
                                        	DbgTmr.print("HdrProfiling:: mpIspHal->stop() Time");
                                        #endif

	mpMdpHal->stop();
	MY_DBG("[do_Isp] Stop MDP.");
                                        #if (HDR_PROFILE_CAPTURE3)
                                        	DbgTmr.print("HdrProfiling:: mpMdpHal->stop() Time");
                                        #endif

	MY_DBG("[do_Isp] - X. err: %d.", err);

	return	(0<=err);

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
configISPParam(MVOID*const pParam, PortInfo const& rInPort, PortInfo const& rOutPort) const
{
	MY_DBG("[configISPParam] - E.");
	MY_VERB("[configISPParam] rInPort:: eImgFmt: %d. u4ImgWidth: %d. u4ImgHeight: %d. ePortID: %d.", rInPort.eImgFmt, rInPort.u4ImgWidth, rInPort.u4ImgHeight, rInPort.ePortID);
	MY_VERB("[configISPParam] rInPort:: u4BufVA: 0x%08X. u4BufPA: 0x%08X. u4BufSize: %d. u4BufCnt: %d.", rInPort.u4BufVA, rInPort.u4BufPA, rInPort.u4BufSize, rInPort.u4BufCnt);
	MY_VERB("[configISPParam] rOutPort:: eImgFmt: %d. u4ImgWidth: %d. u4ImgHeight: %d. ePortID: %d.", rOutPort.eImgFmt, rOutPort.u4ImgWidth, rOutPort.u4ImgHeight, rOutPort.ePortID);
	MY_VERB("[configISPParam] rOutPort:: u4BufVA: 0x%08X. u4BufPA: 0x%08X. u4BufSize: %d. u4BufCnt: %d.", rOutPort.u4BufVA, rOutPort.u4BufPA, rOutPort.u4BufSize, rOutPort.u4BufCnt);

	halISPIFParam_t*const pISPParam = reinterpret_cast<halISPIFParam_t*>(pParam);

	MBOOL	ret = MFALSE;
	//
	if	( ! pISPParam )
	{
		MY_ERR("[configISPParam] NULL pISPParam.");
		goto lbExit;
	}

	//
	::memset(pISPParam, 0, sizeof(halISPIFParam_t));

	//	(1) Input Port.
	if	( 0 != rInPort.u4BufSize )
	{	//	Input Image from Memory.
		pISPParam->u4IsSrcDram = 1;
		pISPParam->u4InPhyAddr = rInPort.u4BufPA;
	}
	else
	{	//	Input Image from Sensor
		pISPParam->u4IsSrcDram = 0;
		pISPParam->u4InPhyAddr = 0;
	}
	pISPParam->u4SrcW = rInPort.u4ImgWidth;
	pISPParam->u4SrcH = rInPort.u4ImgHeight;


	//	(2) Output Port.
	if	( 0 != rOutPort.u4BufSize )
	{	//	Output to Memory.
		pISPParam->u4IsDestDram = 1;
		pISPParam->u4OutPhyAddr = rOutPort.u4BufPA;
	}
	else
	{	//	Output to the next module.
		pISPParam->u4IsDestDram = 0;
		pISPParam->u4OutPhyAddr = 0;
	}

	pISPParam->u4IsDestBayer = 0;

	//	(3) Others.
	//	TODO: divide into in/out mem sizes
	pISPParam->u4MemSize =
		( rInPort.u4BufSize*rInPort.u4BufCnt > rOutPort.u4BufSize*rOutPort.u4BufCnt )
		? rInPort.u4BufSize*rInPort.u4BufCnt : rOutPort.u4BufSize*rOutPort.u4BufCnt;

	MY_VERB("[configISPParam] pISPParam:: u4IsDestBayer: %d. u4MemSize: %d. u4IsSrcDram: %d. u4InPhyAddr: 0x%08X.", pISPParam->u4IsDestBayer, pISPParam->u4MemSize, pISPParam->u4IsSrcDram, pISPParam->u4InPhyAddr);
	MY_VERB("[configISPParam] pISPParam:: u4IsDestDram: %d. u4OutPhyAddr: 0x%08X.", pISPParam->u4IsDestDram, pISPParam->u4OutPhyAddr);
	MY_VERB("[configISPParam] pISPParam:: u4SrcW: %d. u4SrcH: %d.", pISPParam->u4SrcW , pISPParam->u4SrcH);

	ret = MTRUE;
lbExit:
	MY_DBG("[configISPParam] - X ret: %d.", ret);
	return	ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
configMDPParam(MVOID*const pParam, PortInfo const& rInPort, PortInfo const& rOutPort) const
{
	MY_DBG("[configMDPParam] - E.");
	MY_VERB("[configMDPParam] rInPort:: eImgFmt: %d. u4ImgWidth: %d. u4ImgHeight: %d. ePortID: %d.", rInPort.eImgFmt, rInPort.u4ImgWidth, rInPort.u4ImgHeight, rInPort.ePortID);
	MY_VERB("[configMDPParam] rInPort:: u4BufVA: 0x%08X. u4BufPA: 0x%08X. u4BufSize: %d. u4BufCnt: %d.", rInPort.u4BufVA, rInPort.u4BufPA, rInPort.u4BufSize, rInPort.u4BufCnt);
	MY_VERB("[configMDPParam] rOutPort:: eImgFmt: %d. u4ImgWidth: %d. u4ImgHeight: %d. ePortID: %d.", rOutPort.eImgFmt, rOutPort.u4ImgWidth, rOutPort.u4ImgHeight, rOutPort.ePortID);
	MY_VERB("[configMDPParam] rOutPort:: u4BufVA: 0x%08X. u4BufPA: 0x%08X. u4BufSize: %d. u4BufCnt: %d.", rOutPort.u4BufVA, rOutPort.u4BufPA, rOutPort.u4BufSize, rOutPort.u4BufCnt);

	halIDPParam_t*const pMDPParam = reinterpret_cast<halIDPParam_t*>(pParam);

	if	( ! pMDPParam )
	{
		MY_ERR("[configMDPParam] NULL pMDPParam.");
		return	MFALSE;
	}

	rect_t rcSrc, rcDst, rcCrop;
	::memset(&rcSrc, 0, sizeof(rect_t));
	::memset(&rcDst, 0, sizeof(rect_t));
	::memset(&rcCrop,0, sizeof(rect_t));
	rcSrc.w = rInPort.u4ImgWidth;
	rcSrc.h = rInPort.u4ImgHeight;
	rcDst.w = rOutPort.u4ImgWidth;
	rcDst.h = rOutPort.u4ImgHeight;
	mpMdpHal->calCropRect(rcSrc, rcDst, &rcCrop, mShotParam.u4ZoomRatio);
	MY_VERB("[configMDPParam] src:(w,h)=(%d,%d), dst:(w,h)=(%d,%d)", rcSrc.w, rcSrc.h, rcDst.w, rcDst.h);
	MY_VERB("[configMDPParam] crop:(x,y,w,h)=(%d,%d,%d,%d)", rcCrop.x, rcCrop.y, rcCrop.w, rcCrop.h);

	// For capture mode, SrcW/H is input, YuvW/H is jpeg, RgbW/H is for internal YUV
	// JPEG Config
	::memset(pMDPParam, 0, sizeof(halIDPParam_t));
	pMDPParam->mode = MDP_MODE_CAP_JPG; 		 // For capture mode
	pMDPParam->Capture.b_jpg_path_disen = 1;
	pMDPParam->Capture.src_img_size.w = rInPort.u4ImgWidth;
	pMDPParam->Capture.src_img_size.h = rInPort.u4ImgHeight;
	pMDPParam->Capture.src_img_roi.x = rcCrop.x;
	pMDPParam->Capture.src_img_roi.y = rcCrop.y;
	pMDPParam->Capture.src_img_roi.w = rcCrop.w;
	pMDPParam->Capture.src_img_roi.h = rcCrop.h;
	/*
	pMDPParam->Capture.jpg_img_size.w = rOutPort.u4ImgWidth;
	pMDPParam->Capture.jpg_img_size.h = rOutPort.u4ImgHeight;
	pMDPParam->Capture.jpg_yuv_color_format = 422;	 //integer : 411, 422, 444, 400, 410 etc...
	pMDPParam->Capture.jpg_buffer_addr = rOutPort.u4BufPA;
	pMDPParam->Capture.jpg_buffer_size = rOutPort.u4BufSize * rOutPort.u4BufCnt;
	pMDPParam->Capture.jpg_quality = mShotParam.u4JpgQValue;	// 39~90
	pMDPParam->Capture.jpg_b_add_soi = 0;						// 1:Add EXIF 0:none
	*/
	// QV Config
	pMDPParam->Capture.b_qv_path_en = 0;
/*
	pMDPParam->Capture.qv_path_sel =  0;		   //0:auto select quick view path 1:QV_path_1 2:QV_path_2
#if defined(MTK_M4U_SUPPORT)
	pMDPParam->Capture.qv_yuv_img_addr.y = mShotParam.frmQv.virtAddr;
#else
	pMDPParam->Capture.qv_yuv_img_addr.y = mShotParam.frmQv.phyAddr;
#endif
	pMDPParam->Capture.qv_img_size.w = mShotParam.frmQv.w;
	pMDPParam->Capture.qv_img_size.h = mShotParam.frmQv.h;
	pMDPParam->Capture.qv_color_format = RGB888;
	pMDPParam->Capture.qv_flip = 0;
	pMDPParam->Capture.qv_rotate = 0;
	*/

	pMDPParam->Capture.b_ff_path_en =  1;			//0:auto select quick view path 1:QV_path_1 2:QV_path_2
#if defined(MTK_M4U_SUPPORT)
	pMDPParam->Capture.ff_yuv_img_addr.y = rOutPort.u4BufVA;
#else
	pMDPParam->Capture.ff_yuv_img_addr.y = rOutPort.u4BufVA;
#endif
	pMDPParam->Capture.ff_img_size.w = rOutPort.u4ImgWidth;
	pMDPParam->Capture.ff_img_size.h = rOutPort.u4ImgHeight;

//	if (rOutPort.eImgFmt == eDATA_FORMAT_YV12)
//	{
//		MY_VERB("[configMDPParam] Enter YUV420_Planar.");
		pMDPParam->Capture.ff_color_format = YUV420_Planar;
//	}

	pMDPParam->Capture.ff_flip = 0;
	pMDPParam->Capture.ff_rotate = 0;

	//
	/*
	using namespace NSCamCustom;
	TuningParam_CRZ_T const& crz_param = getParam_CRZ_Capture();
	pMDPParam->Capture.resz_coeff.crz_up_scale_coeff = crz_param.uUpScaleCoeff;
	pMDPParam->Capture.resz_coeff.crz_dn_scale_coeff = crz_param.uDnScaleCoeff;
	//
	TuningParam_PRZ_T const& prz_param = getParam_PRZ_QuickView();
	pMDPParam->Capture.resz_coeff.prz0_up_scale_coeff= prz_param.uUpScaleCoeff;
	pMDPParam->Capture.resz_coeff.prz0_dn_scale_coeff= prz_param.uDnScaleCoeff;
	pMDPParam->Capture.resz_coeff.prz0_ee_h_str 	 = prz_param.uEEHCoeff;
	pMDPParam->Capture.resz_coeff.prz0_ee_v_str 	 = prz_param.uEEVCoeff;
	*/


	MY_VERB("[configMDPParam] pMDPParam Cap.srcimg:: mode: %d. jpg_path_disen: %d. size_w/h: (%d, %d). roi_x/y: (%d, %d). roi_w/h: (%d, %d).",
		pMDPParam->mode,
		pMDPParam->Capture.b_jpg_path_disen,
		pMDPParam->Capture.src_img_size.w,
		pMDPParam->Capture.src_img_size.h,
		pMDPParam->Capture.src_img_roi.x,
		pMDPParam->Capture.src_img_roi.y,
		pMDPParam->Capture.src_img_roi.w,
		pMDPParam->Capture.src_img_roi.h
	);

	MY_VERB("[configMDPParam] pMDPParam Cap.qv:: qv_path_en: %d. path_sel: %d. yuv_img_addr_y: 0x%08X. img_w/h: (%d, %d).",
		pMDPParam->Capture.b_qv_path_en,
		pMDPParam->Capture.qv_path_sel,
		pMDPParam->Capture.qv_yuv_img_addr.y,
		pMDPParam->Capture.qv_img_size.w,
		pMDPParam->Capture.qv_img_size.h
	);

	MY_VERB("[configMDPParam] pMDPParam Cap.qv:: color_format: %d. flip: %d. rotate: %d.",
		pMDPParam->Capture.qv_color_format,
		pMDPParam->Capture.qv_flip,
		pMDPParam->Capture.qv_rotate
	);

	MY_VERB("[configMDPParam] pMDPParam Cap.ff:: path_en: %d. yuv_img_addr_y: 0x%08X. img_w/h: (%d, %d).",
		pMDPParam->Capture.b_ff_path_en,
		pMDPParam->Capture.ff_yuv_img_addr.y,
		pMDPParam->Capture.ff_img_size.w,
		pMDPParam->Capture.ff_img_size.h
	);

	MY_VERB("[configMDPParam] pMDPParam Cap.ff:: color_format: %d. flip: %d. rotate: %d.",
		pMDPParam->Capture.ff_color_format,
		pMDPParam->Capture.ff_flip,
		pMDPParam->Capture.ff_rotate
	);

	MY_DBG("[configMDPParam] - X.");

	return	MTRUE;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
configMDPParamTwoOut(MVOID*const pParam, PortInfo const& rInPort, PortInfo const& rOutPort, PortInfo const& rOutPortSmall) const
{
	MY_DBG("[configMDPParamTwoOut] - E.");
	MY_VERB("[configMDPParamTwoOut] rInPort:: eImgFmt: %d. u4ImgWidth: %d. u4ImgHeight: %d. ePortID: %d.", rInPort.eImgFmt, rInPort.u4ImgWidth, rInPort.u4ImgHeight, rInPort.ePortID);
	MY_VERB("[configMDPParamTwoOut] rInPort:: u4BufVA: 0x%08X. u4BufPA: 0x%08X. u4BufSize: %d. u4BufCnt: %d.", rInPort.u4BufVA, rInPort.u4BufPA, rInPort.u4BufSize, rInPort.u4BufCnt);
	MY_VERB("[configMDPParamTwoOut] rOutPort:: eImgFmt: %d. u4ImgWidth: %d. u4ImgHeight: %d. ePortID: %d.", rOutPort.eImgFmt, rOutPort.u4ImgWidth, rOutPort.u4ImgHeight, rOutPort.ePortID);
	MY_VERB("[configMDPParamTwoOut] rOutPort:: u4BufVA: 0x%08X. u4BufPA: 0x%08X. u4BufSize: %d. u4BufCnt: %d.", rOutPort.u4BufVA, rOutPort.u4BufPA, rOutPort.u4BufSize, rOutPort.u4BufCnt);
	MY_VERB("[configMDPParamTwoOut] rOutPortSmall:: eImgFmt: %d. u4ImgWidth: %d. u4ImgHeight: %d. ePortID: %d.", rOutPortSmall.eImgFmt, rOutPortSmall.u4ImgWidth, rOutPortSmall.u4ImgHeight, rOutPortSmall.ePortID);
	MY_VERB("[configMDPParamTwoOut] rOutPortSmall:: u4BufVA: 0x%08X. u4BufPA: 0x%08X. u4BufSize: %d. u4BufCnt: %d.", rOutPortSmall.u4BufVA, rOutPortSmall.u4BufPA, rOutPortSmall.u4BufSize, rOutPortSmall.u4BufCnt);

	halIDPParam_t*const pMDPParam = reinterpret_cast<halIDPParam_t*>(pParam);

	if	( ! pMDPParam )
	{
		MY_ERR("[configMDPParamTwoOut] NULL pMDPParam.");
		return	MFALSE;
	}

	rect_t rcSrc, rcDst, rcCrop;
	::memset(&rcSrc, 0, sizeof(rect_t));
	::memset(&rcDst, 0, sizeof(rect_t));
	::memset(&rcCrop,0, sizeof(rect_t));
	rcSrc.w = rInPort.u4ImgWidth;
	rcSrc.h = rInPort.u4ImgHeight;
	rcDst.w = rOutPort.u4ImgWidth;
	rcDst.h = rOutPort.u4ImgHeight;
	mpMdpHal->calCropRect(rcSrc, rcDst, &rcCrop, mShotParam.u4ZoomRatio);
	MY_VERB("[configMDPParamTwoOut] src:(w,h)=(%d,%d), dst:(w,h)=(%d,%d)", rcSrc.w, rcSrc.h, rcDst.w, rcDst.h);
	MY_VERB("[configMDPParamTwoOut] crop:(x,y,w,h)=(%d,%d,%d,%d)", rcCrop.x, rcCrop.y, rcCrop.w, rcCrop.h);

	// For capture mode, SrcW/H is input, YuvW/H is jpeg, RgbW/H is for internal YUV
	// JPEG Config
	::memset(pMDPParam, 0, sizeof(halIDPParam_t));
	pMDPParam->mode = MDP_MODE_CAP_JPG; 		 // For capture mode
	pMDPParam->Capture.b_jpg_path_disen = 1;
	pMDPParam->Capture.src_img_size.w = rInPort.u4ImgWidth;
	pMDPParam->Capture.src_img_size.h = rInPort.u4ImgHeight;
	pMDPParam->Capture.src_img_roi.x = rcCrop.x;
	pMDPParam->Capture.src_img_roi.y = rcCrop.y;
	pMDPParam->Capture.src_img_roi.w = rcCrop.w;
	pMDPParam->Capture.src_img_roi.h = rcCrop.h;
	/*
	pMDPParam->Capture.jpg_img_size.w = rOutPort.u4ImgWidth;
	pMDPParam->Capture.jpg_img_size.h = rOutPort.u4ImgHeight;
	pMDPParam->Capture.jpg_yuv_color_format = 422;	 //integer : 411, 422, 444, 400, 410 etc...
	pMDPParam->Capture.jpg_buffer_addr = rOutPort.u4BufPA;
	pMDPParam->Capture.jpg_buffer_size = rOutPort.u4BufSize * rOutPort.u4BufCnt;
	pMDPParam->Capture.jpg_quality = mShotParam.u4JpgQValue;	// 39~90
	pMDPParam->Capture.jpg_b_add_soi = 0;						// 1:Add EXIF 0:none
	*/
	// QV Config
	pMDPParam->Capture.b_qv_path_en         = 1;
	pMDPParam->Capture.qv_path_sel          = 0;		   //0:auto select quick view path 1:QV_path_1 2:QV_path_2
#if defined(MTK_M4U_SUPPORT)
	pMDPParam->Capture.qv_yuv_img_addr.y    = rOutPort.u4BufVA;
#else
	pMDPParam->Capture.qv_yuv_img_addr.y    = rOutPort.u4BufVA;
#endif
	pMDPParam->Capture.qv_img_size.w        = rOutPort.u4ImgWidth;
	pMDPParam->Capture.qv_img_size.h        = rOutPort.u4ImgHeight;
	pMDPParam->Capture.qv_color_format      = YUV420_Planar;
	pMDPParam->Capture.qv_flip              = 0;
	pMDPParam->Capture.qv_rotate            = 0;

	pMDPParam->Capture.b_ff_path_en         =  1;			// 0:auto select quick view path 1:QV_path_1 2:QV_path_2
#if defined(MTK_M4U_SUPPORT)
	pMDPParam->Capture.ff_yuv_img_addr.y    = rOutPortSmall.u4BufVA;
#else
	pMDPParam->Capture.ff_yuv_img_addr.y    = rOutPortSmall.u4BufVA;
#endif
	pMDPParam->Capture.ff_img_size.w        = rOutPortSmall.u4ImgWidth;
	pMDPParam->Capture.ff_img_size.h        = rOutPortSmall.u4ImgHeight;
	pMDPParam->Capture.ff_color_format      = Y800;
    pMDPParam->Capture.ff_flip              = 0;
	pMDPParam->Capture.ff_rotate            = 0;

	//
	/*
	using namespace NSCamCustom;
	TuningParam_CRZ_T const& crz_param = getParam_CRZ_Capture();
	pMDPParam->Capture.resz_coeff.crz_up_scale_coeff = crz_param.uUpScaleCoeff;
	pMDPParam->Capture.resz_coeff.crz_dn_scale_coeff = crz_param.uDnScaleCoeff;
	//
	TuningParam_PRZ_T const& prz_param = getParam_PRZ_QuickView();
	pMDPParam->Capture.resz_coeff.prz0_up_scale_coeff= prz_param.uUpScaleCoeff;
	pMDPParam->Capture.resz_coeff.prz0_dn_scale_coeff= prz_param.uDnScaleCoeff;
	pMDPParam->Capture.resz_coeff.prz0_ee_h_str 	 = prz_param.uEEHCoeff;
	pMDPParam->Capture.resz_coeff.prz0_ee_v_str 	 = prz_param.uEEVCoeff;
	*/


	MY_VERB("[configMDPParamTwoOut] pMDPParam Cap.srcimg:: mode: %d. jpg_path_disen: %d. size_w/h: (%d, %d). roi_x/y: (%d, %d). roi_w/h: (%d, %d).",
		pMDPParam->mode,
		pMDPParam->Capture.b_jpg_path_disen,
		pMDPParam->Capture.src_img_size.w,
		pMDPParam->Capture.src_img_size.h,
		pMDPParam->Capture.src_img_roi.x,
		pMDPParam->Capture.src_img_roi.y,
		pMDPParam->Capture.src_img_roi.w,
		pMDPParam->Capture.src_img_roi.h
	);

	MY_VERB("[configMDPParamTwoOut] pMDPParam Cap.qv:: qv_path_en: %d. path_sel: %d. yuv_img_addr_y: 0x%08X. img_w/h: (%d, %d).",
		pMDPParam->Capture.b_qv_path_en,
		pMDPParam->Capture.qv_path_sel,
		pMDPParam->Capture.qv_yuv_img_addr.y,
		pMDPParam->Capture.qv_img_size.w,
		pMDPParam->Capture.qv_img_size.h
	);

	MY_VERB("[configMDPParamTwoOut] pMDPParam Cap.qv:: color_format: %d. flip: %d. rotate: %d.",
		pMDPParam->Capture.qv_color_format,
		pMDPParam->Capture.qv_flip,
		pMDPParam->Capture.qv_rotate
	);

	MY_VERB("[configMDPParamTwoOut] pMDPParam Cap.ff:: path_en: %d. yuv_img_addr_y: 0x%08X. img_w/h: (%d, %d).",
		pMDPParam->Capture.b_ff_path_en,
		pMDPParam->Capture.ff_yuv_img_addr.y,
		pMDPParam->Capture.ff_img_size.w,
		pMDPParam->Capture.ff_img_size.h
	);

	MY_VERB("[configMDPParamTwoOut] pMDPParam Cap.ff:: color_format: %d. flip: %d. rotate: %d.",
		pMDPParam->Capture.ff_color_format,
		pMDPParam->Capture.ff_flip,
		pMDPParam->Capture.ff_rotate
	);

	MY_DBG("[configMDPParamTwoOut] - X.");

	return	MTRUE;
}


/*******************************************************************************
* JPG encode
*******************************************************************************/
MBOOL
Hdr::
MDPJPGENC_forQuickView(MUINT32 srcadr, BufInfo bufInfo, MHAL_BITBLT_FORMAT_ENUM srcformat,int u4Orientation)
{
    //  (1) Config ISPParam / MDPParam
    MBOOL   ret = MFALSE;
    MINT32  err = -1;

    BufInfo jpgbufInfo;
    jpgbufInfo = getBufInfo_JpgEnc();

    MY_DBG("[MDPJPGENC_forQuickView] jpgbufInfo:0x%08x. (w, h): (%d, %d)\n",jpgbufInfo.u4BufVA, mShotParam.frmJpg.w, mShotParam.frmJpg.h);
    halIDPParam_t   pMDPParam;
    MdpYuvAddr  jpgbuff;

    // For capture mode, SrcW/H is input, YuvW/H is jpeg, RgbW/H is for internal YUV
    // JPEG Config
    ::memset((void*)&pMDPParam, 0, sizeof(halIDPParam_t));
    ::memset((void*)&jpgbuff, 0, sizeof(MdpYuvAddr));

    rect_t rcSrc, rcDst, rcCrop;
    ::memset(&rcSrc, 0, sizeof(rect_t));
    ::memset(&rcDst, 0, sizeof(rect_t));
    ::memset(&rcCrop,0, sizeof(rect_t));
    if (u4Orientation==0 || u4Orientation==2)
    {
        rcSrc.w = mu4W_yuv;
        rcSrc.h = mu4H_yuv;
        rcDst.w = mShotParam.frmJpg.w;
        rcDst.h = mShotParam.frmJpg.h;
    }
    else
    {
        rcSrc.w = mu4H_yuv;
        rcSrc.h = mu4W_yuv;
        rcDst.w = mShotParam.frmJpg.h;
        rcDst.h = mShotParam.frmJpg.w;
    }
    //mpMdpHal->calCropRect(rcSrc, rcDst, &rcCrop, mShotParam.u4ZoomRatio);
    mpMdpHal->calCropRect(rcSrc, rcDst, &rcCrop, 100);

    MY_DBG("[MDPJPGENC_forQuickView] src:(w,h)=(%d,%d), dst:(w,h)=(%d,%d)", rcSrc.w, rcSrc.h, rcDst.w, rcDst.h);
    MY_DBG("[MDPJPGENC_forQuickView] crop:(x,y,w,h)=(%d,%d,%d,%d)", rcCrop.x, rcCrop.y, rcCrop.w, rcCrop.h);

    pMDPParam.mode = MDP_MODE_CAP_JPG;          // For capture mode
    jpgbuff.y = srcadr;
//    pMDPParam.Capture.b_jpg_path_disen = 0;
    pMDPParam.Capture.b_jpg_path_disen = 1;     // Don't do JPG encode.
    pMDPParam.Capture.pseudo_src_enable = 1;
    pMDPParam.Capture.pseudo_src_yuv_img_addr = jpgbuff;
    pMDPParam.Capture.pseudo_src_color_format = MhalColorFormatToMdp(srcformat);
    pMDPParam.Capture.src_img_roi.x = rcCrop.x;
    pMDPParam.Capture.src_img_roi.y = rcCrop.y;
    pMDPParam.Capture.src_img_roi.w = rcCrop.w;
    pMDPParam.Capture.src_img_roi.h = rcCrop.h;
    if (u4Orientation==0 || u4Orientation==2)
    {
        pMDPParam.Capture.src_img_size.w = mu4W_yuv;
        pMDPParam.Capture.src_img_size.h = mu4H_yuv;
    }
    else
    {
        pMDPParam.Capture.src_img_size.w = mu4H_yuv;
        pMDPParam.Capture.src_img_size.h = mu4W_yuv;
    }

    pMDPParam.Capture.jpg_yuv_color_format = 422;   //integer : 411, 422, 444, 400, 410 etc...
    pMDPParam.Capture.jpg_buffer_addr = jpgbufInfo.u4BufVA;//VA
    pMDPParam.Capture.jpg_buffer_size = jpgbufInfo.u4BufSize * jpgbufInfo.u4BufCnt;
    pMDPParam.Capture.jpg_quality = mShotParam.u4JpgQValue;    // 39~90
    pMDPParam.Capture.jpg_b_add_soi = 0;                       // 1:Add EXIF 0:none
    if (u4Orientation==0 || u4Orientation==2)
    {
        pMDPParam.Capture.jpg_img_size.w = mShotParam.frmJpg.w;
        pMDPParam.Capture.jpg_img_size.h = mShotParam.frmJpg.h;
    }
    else
    {
        pMDPParam.Capture.jpg_img_size.w = mShotParam.frmJpg.h;
        pMDPParam.Capture.jpg_img_size.h = mShotParam.frmJpg.w;
    }

    pMDPParam.Capture.b_qv_path_en = 1;
    pMDPParam.Capture.qv_path_sel =  0;  //0:auto select quick view path 1:QV_path_1 2:QV_path_2
    pMDPParam.Capture.qv_yuv_img_addr.y = bufInfo.u4BufVA; //VA
    pMDPParam.Capture.qv_color_format = YUV420_Planar;
    pMDPParam.Capture.qv_flip = 0;
    pMDPParam.Capture.qv_rotate = 0;

    pMDPParam.Capture.b_ff_path_en =  0;
    if (u4Orientation==0 || u4Orientation==2)
    {
        pMDPParam.Capture.qv_img_size.w = mShotParam.frmQv.w;
        pMDPParam.Capture.qv_img_size.h = mShotParam.frmQv.h;
    }
    else
    {
        pMDPParam.Capture.qv_img_size.w = mShotParam.frmQv.h;
        pMDPParam.Capture.qv_img_size.h = mShotParam.frmQv.w;
    }

    //  (2) Apply settings.
    ret = 0==(err = mpMdpHal->setConf(&pMDPParam));
    if  (!ret)
    {
        MY_ERR("[MDPJPGENC_forQuickView] Config LCE - ec(%x) \n", err);
        goto failExit;
    }

    //  (3) Start
    ret = 0==(err = mpMdpHal->start());
    if  (!ret)
    {
        MY_ERR("[MDPJPGENC_forQuickView] Start - ec(%x) \n", err);
        goto failExit;
    }

    //  (4) Wait Done
    ret =   waitJpgCaptureDone();
    if  (!ret)
    {
        MY_ERR("[MDPJPGENC_forQuickView] waitJpgCaptureDone fail \n");
        goto failExit;
    }

    ret = 0==(err = mpMdpHal->stop());
    if  (!ret)
    {
        MY_ERR("[MDPJPGENC_forQuickView] stop - ec(%x) \n", err);
        goto failExit;
    }
    return  ret;
failExit:
    ret = MTRUE;
    return  ret;
}


/*******************************************************************************
* JPG encode
*******************************************************************************/
MBOOL
Hdr::
MDPJPGENC(MUINT32 srcadr, BufInfo bufInfo, MHAL_BITBLT_FORMAT_ENUM srcformat,int u4Orientation)
{
    //  (1) Config ISPParam / MDPParam
    MBOOL   ret = MFALSE;
    MINT32  err = -1;

    BufInfo jpgbufInfo;
    jpgbufInfo = getBufInfo_JpgEnc();

    MY_DBG("[MDPJPGENC] jpgbufInfo:0x%08x. (w, h): (%d, %d)\n",jpgbufInfo.u4BufVA, mShotParam.frmJpg.w, mShotParam.frmJpg.h);
    halIDPParam_t   pMDPParam;
    MdpYuvAddr  jpgbuff;

    // For capture mode, SrcW/H is input, YuvW/H is jpeg, RgbW/H is for internal YUV
    // JPEG Config
    ::memset((void*)&pMDPParam, 0, sizeof(halIDPParam_t));
    ::memset((void*)&jpgbuff, 0, sizeof(MdpYuvAddr));

    rect_t rcSrc, rcDst, rcCrop;
    ::memset(&rcSrc, 0, sizeof(rect_t));
    ::memset(&rcDst, 0, sizeof(rect_t));
    ::memset(&rcCrop,0, sizeof(rect_t));
    if (u4Orientation==0 || u4Orientation==2)
    {
        rcSrc.w = mu4W_yuv;
        rcSrc.h = mu4H_yuv;
        rcDst.w = mShotParam.frmJpg.w;
        rcDst.h = mShotParam.frmJpg.h;
    }
    else
    {
        rcSrc.w = mu4H_yuv;
        rcSrc.h = mu4W_yuv;
        rcDst.w = mShotParam.frmJpg.h;
        rcDst.h = mShotParam.frmJpg.w;
    }
    //mpMdpHal->calCropRect(rcSrc, rcDst, &rcCrop, mShotParam.u4ZoomRatio);
    mpMdpHal->calCropRect(rcSrc, rcDst, &rcCrop, 100);

    MY_DBG("[MDPJPGENC] src:(w,h)=(%d,%d), dst:(w,h)=(%d,%d)", rcSrc.w, rcSrc.h, rcDst.w, rcDst.h);
    MY_DBG("[MDPJPGENC] crop:(x,y,w,h)=(%d,%d,%d,%d)", rcCrop.x, rcCrop.y, rcCrop.w, rcCrop.h);

    pMDPParam.mode = MDP_MODE_CAP_JPG;          // For capture mode
    jpgbuff.y = srcadr;
    pMDPParam.Capture.b_jpg_path_disen = 0;
    pMDPParam.Capture.pseudo_src_enable = 1;
    pMDPParam.Capture.pseudo_src_yuv_img_addr = jpgbuff;
    pMDPParam.Capture.pseudo_src_color_format = MhalColorFormatToMdp(srcformat);
    pMDPParam.Capture.src_img_roi.x = rcCrop.x;
    pMDPParam.Capture.src_img_roi.y = rcCrop.y;
    pMDPParam.Capture.src_img_roi.w = rcCrop.w;
    pMDPParam.Capture.src_img_roi.h = rcCrop.h;
    if (u4Orientation==0 || u4Orientation==2)
    {
        pMDPParam.Capture.src_img_size.w = mu4W_yuv;
        pMDPParam.Capture.src_img_size.h = mu4H_yuv;
    }
    else
    {
        pMDPParam.Capture.src_img_size.w = mu4H_yuv;
        pMDPParam.Capture.src_img_size.h = mu4W_yuv;
    }

    pMDPParam.Capture.jpg_yuv_color_format = 422;   //integer : 411, 422, 444, 400, 410 etc...
    pMDPParam.Capture.jpg_buffer_addr = jpgbufInfo.u4BufVA;//VA
    pMDPParam.Capture.jpg_buffer_size = jpgbufInfo.u4BufSize * jpgbufInfo.u4BufCnt;
    pMDPParam.Capture.jpg_quality = mShotParam.u4JpgQValue;    // 39~90
    pMDPParam.Capture.jpg_b_add_soi = 0;                       // 1:Add EXIF 0:none
    if (u4Orientation==0 || u4Orientation==2)
    {
        pMDPParam.Capture.jpg_img_size.w = mShotParam.frmJpg.w;
        pMDPParam.Capture.jpg_img_size.h = mShotParam.frmJpg.h;
    }
    else
    {
        pMDPParam.Capture.jpg_img_size.w = mShotParam.frmJpg.h;
        pMDPParam.Capture.jpg_img_size.h = mShotParam.frmJpg.w;
    }

    pMDPParam.Capture.b_qv_path_en = 1;
    pMDPParam.Capture.qv_path_sel =  0;  //0:auto select quick view path 1:QV_path_1 2:QV_path_2
    pMDPParam.Capture.qv_yuv_img_addr.y = bufInfo.u4BufVA; //VA
    pMDPParam.Capture.qv_color_format = YUV420_Planar;
    pMDPParam.Capture.qv_flip = 0;
    pMDPParam.Capture.qv_rotate = 0;

    pMDPParam.Capture.b_ff_path_en =  0;
    if (u4Orientation==0 || u4Orientation==2)
    {
        pMDPParam.Capture.qv_img_size.w = mShotParam.frmQv.w;
        pMDPParam.Capture.qv_img_size.h = mShotParam.frmQv.h;
    }
    else
    {
        pMDPParam.Capture.qv_img_size.w = mShotParam.frmQv.h;
        pMDPParam.Capture.qv_img_size.h = mShotParam.frmQv.w;
    }

    //  (2) Apply settings.
    ret = 0==(err = mpMdpHal->setConf(&pMDPParam));
    if  (!ret)
    {
        MY_ERR("[MDPJPGENC] Config LCE - ec(%x) \n", err);
        goto failExit;
    }

    //  (3) Start
    ret = 0==(err = mpMdpHal->start());
    if  (!ret)
    {
        MY_ERR("[MDPJPGENC] Start - ec(%x) \n", err);
        goto failExit;
    }

    //  (4) Wait Done
    ret =   waitJpgCaptureDone();
    if  (!ret)
    {
        MY_ERR("[MDPJPGENC] waitJpgCaptureDone fail \n");
        goto failExit;
    }

    ret = 0==(err = mpMdpHal->stop());
    if  (!ret)
    {
        MY_ERR("[MDPJPGENC] stop - ec(%x) \n", err);
        goto failExit;
    }
    return  ret;
failExit:
    ret = MTRUE;
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
saveRaw(MUINT8*const pRawBuf, MUINT32 const u4RawSize, char const*const szPrefix, char const*const szPostfix)
{
	char szFileName[100];
	::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%s_%dx%d_%s.raw", szPrefix, mu4W_raw, mu4H_raw, szPostfix);
	return	0 == mHalMiscDumpToFile(szFileName, pRawBuf, u4RawSize);
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
createFullFrame(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[createFullFrame] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL  ret = MTRUE; // MTRUE: No error.
	MINT32 err = 0; // 0: No error.
	MINT32 i4Iso = 0;

	using namespace NSCamera;

#if (HDR_PROFILE_CAPTURE3)
    MyDbgTimer DbgTmr("createFullFrame");
#endif


    //	(0) Config In Ports.
    PortInfo InPortInfo;
    if (meSensorType == eSensorType_RAW)
    {
         InPortInfo = PortInfo(ImgInfo(ePIXEL_FORMAT_BAYER10, mu4W_raw, mu4H_raw));
    }
    else
    {
         InPortInfo = PortInfo(ImgInfo(ePIXEL_FORMAT_YUY2, mu4W_yuv, mu4H_yuv));
    }
    PortInfo BpsPort= PortInfo(ImgInfo(ePIXEL_FORMAT_YUY2, mu4W_yuv, mu4H_yuv));

    //	(1) Config ISPParam
    halISPIFParam_t ISPParam;
    configISPParam(&ISPParam, InPortInfo, BpsPort);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: configISPParam Time");
                                        #endif
    //	(3) Set CamMode
    setIspMode(ISP_CAM_MODE_CAPTURE_FLY);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: setIspMode Time");
                                        #endif

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		MY_DBG("[createFullFrame] <%d/%d Capture>", i+1, u4OutputFrameNum);
		MY_DBG("[createFullFrame] Using buffer: %s. Addr: 0x%08X. Size: %d.", mpSourceImgBuf[i]->getPoolName(), mpSourceImgBuf[i]->getVirtAddr(), mpSourceImgBuf[i]->getPoolSize());

        //////////////////////////////////////////////////////////////////
        //                     1st VD start                             //
        //////////////////////////////////////////////////////////////////
		ret =
                //  Set ExpT and Sensor Gain to sensor driver.
				0==(err = mpHal3A->setCaptureHDRParam(i/*a_u4CntIdx*/, FALSE/*a_IsRawGain*/)) // a_u4CntIdx: the index which you want to capture. Range: 0~2; a_IsRawGain: TRUE(raw gain setting)/FALSE(sensor exposure time and gain setting).
                                        #if (HDR_PROFILE_CAPTURE3)
                                        &&	DbgTmr.print("HdrProfiling3:: setCaptureHDRParam Time")
                                        #endif
                //  Update ISO info to ISP.
				//      Get ISO from 3A hal.
			&&	0==(err = mpHal3A->getISO(1, &i4Iso))
				//      Set ISO to ISP hal.
			&&	0==(err = mpIspHal->sendCommand(ISP_CMD_SET_ISO, i4Iso))
				;
        MY_DBG("[createFullFrame] Set ExpT/SensorGain, and update ISO to ISP: %d.", i4Iso);
		if (! ret)
		{
		    MY_ERR("[createFullFrame] Set ExpT/SensorGain, and update ISO to ISP fail. - ec(%d).", err);
			goto lbExit;
		}

        // Wait for previous round's MDP done.
        if (i != 0)
        {
        	// If MDP and ISP start together, wait MDP done is enough.
        	if	( (err = mpMdpHal->waitDone(0x01)) )
        	{
        		MY_ERR("[createFullFrame] mpMdpHal->waitDone() fail. ec(%x)", err);
                mpIspHal->stop();
                mpMdpHal->stop();
        		goto lbExit;
        	}
        	MY_DBG("[createFullFrame] mpMdpHal->waitDone() done.");
                                                #if (HDR_PROFILE_CAPTURE3)
                                                DbgTmr.print("HdrProfiling3:: mpMdpHal->waitDone() Time");
                                                #endif
        }

        // Wait for SENSOR DRV ready flag.
		err = mpIspHal->sendCommand(ISP_HAL_CMD_SET_SENSOR_WAIT, ISP_HAL_SENSOR_WAIT_SHUTTER_GAIN, 500/*time-out in msec*/);
		if (err)
		{
		    MY_ERR("[createFullFrame] ISP_HAL_CMD_SET_SENSOR_WAIT %02d fail. - ec(%d).", i+1, err);
			goto lbExit;
		}
        MY_DBG("[createFullFrame] SENSOR_WAIT %02d received.", i+1);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: SENSOR_WAIT Time");
                                        #endif

        // Wait VSYNC.
    	err = mpIspHal->waitDone(ISP_HAL_IRQ_CLEAR_WAIT | ISP_HAL_IRQ_VSYNC);    // 1st VD end. 2nd VD start. ISP_HAL_IRQ_CLEAR_WAIT: clear remnant VSYNC.
		if (err)
		{
		    MY_ERR("[createFullFrame] wait VSYNC %02d fail. - ec(%d).", 2 * i + 1, err);
			goto lbExit;
		}
        MY_DBG("[createFullFrame] VSYNC %02d received.", 2 * i + 1);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: VSYNC_WAIT Time");
                                        #endif
        //////////////////////////////////////////////////////////////////
        //                     1st VD end                               //
        //////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////
        //                     2nd VD start                             //
        //////////////////////////////////////////////////////////////////

        // For previous round's MDP done. Stop ISP/MDP when MDP is done.
        if (i != 0)
        {

        	mpIspHal->stop();
        	MY_DBG("[createFullFrame] Stop ISP.");
                                                #if (HDR_PROFILE_CAPTURE3)
                                                DbgTmr.print("HdrProfiling3:: mpIspHal->stop() Time");
                                                #endif

            mpMdpHal->stop();
        	MY_DBG("[createFullFrame] Stop MDP.");
                                                #if (HDR_PROFILE_CAPTURE3)
                                                DbgTmr.print("HdrProfiling3:: mpMdpHal->stop() Time");
                                                #endif
        }

		//	(0) Config Out Ports. Each round will use different OutPort.
		PortInfo OutPortInfo= PortInfo(ImgInfo(ePIXEL_FORMAT_YV12, mu4W_yuv, mu4H_yuv), BufInfo(mpSourceImgBuf[i]->getVirtAddr(), mpSourceImgBuf[i]->getPhyAddr(), mu4SourceSize));
		PortInfo OutPortInfoSmall= PortInfo(ImgInfo(ePIXEL_FORMAT_Y800, mu4W_small, mu4H_small), BufInfo(mpSmallImgBuf[i]->getVirtAddr(), mpSmallImgBuf[i]->getPhyAddr(), mu4SmallImgSize));

		//	(1) Config MDPParam
		halIDPParam_t	MDPParam;
#if (HDR_MDP_TWO_OUT)
		configMDPParamTwoOut(&MDPParam, BpsPort, OutPortInfo, OutPortInfoSmall);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: configMDPParamTwoOut Time");
                                        #endif
#else   // original
		configMDPParam(&MDPParam, BpsPort, OutPortInfo);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: configMDPParam Time");
                                        #endif
#endif  // HDR_MDP_TWO_OUT

		//	(2) Config MDP/ISP
        ISPParam.u4IsBypassSensorDelay = 1; // This will make SP_DELAY use 0 in isp_hal.cpp. Fast Capture flow must use SP_DELAY = 0.
		ret =
			    0==(err = mpMdpHal->setConf(&MDPParam))
                                        #if (HDR_PROFILE_CAPTURE3)
                                        &&	DbgTmr.print("HdrProfiling3:: mpMdpHal->setConf Time")
                                        #endif
			&&	0==(err = mpIspHal->setConf(&ISPParam))
                                        #if (HDR_PROFILE_CAPTURE3)
                                        &&	DbgTmr.print("HdrProfiling3:: mpIspHal->setConf Time")
                                        #endif
                ;
		if	( ! ret )
		{
			MY_ERR("[createFullFrame] Config MDP/ISP fail. ec(%x)", err);
			goto lbExit;
		}

		//	(3) Set ISP Raw Gain
		ret =
				0==(err = mpHal3A->setCaptureHDRParam(i/*a_u4CntIdx*/, TRUE/*a_IsRawGain*/)) // a_u4CntIdx: the index which you want to capture. Range: 0~2; a_IsRawGain: TRUE(raw gain setting)/FALSE(sensor exposure time and gain setting).
			&&	camera_start()  // Start MDP and ISP.
				;
		if	( ! ret )
		{
			MY_ERR("[createFullFrame] camera_start %02d fail. ec(%x)", i+1, err);
			goto lbExit;
		}
        MY_DBG("[createFullFrame] camera_start() %02d done.", i+1);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: camera_start Time");
                                        #endif

        #if 1   // Wait for VSYNC.
        MUINT32 ProlongedVd = CustomHdrProlongedVdGet();
        for (MUINT32 j = 0; j < ProlongedVd; j++)
        {
    		err = mpIspHal->waitDone(ISP_HAL_IRQ_CLEAR_WAIT | ISP_HAL_IRQ_VSYNC);    // 2nd VD end. 3rd VD start. ISP_HAL_IRQ_CLEAR_WAIT: clear remnant VSYNC.
                                            #if (HDR_PROFILE_CAPTURE3)
                                            DbgTmr.print("HdrProfiling3:: VSYNC_WAIT Time");
                                            #endif
    		if (err)
    		{
    		    MY_ERR("[createFullFrame] wait VSYNC %02d fail. - ec(%d).", 2 * i + 2, err);
    			goto lbExit;
    		}
            MY_DBG("[createFullFrame] VSYNC %02d received. ProlongedVd: %d.", 2 * i + 2, ProlongedVd);
            //////////////////////////////////////////////////////////////////
            //                     2nd VD end                               //
            //////////////////////////////////////////////////////////////////

    	}
        #endif
	}

	// If MDP and ISP start together, wait MDP done is enough.
	if	( (err = mpMdpHal->waitDone(0x01)) )
	{
		MY_ERR("[createFullFrame] mpMdpHal->waitDone() fail. ec(%x)", err);
        mpIspHal->stop();
        mpMdpHal->stop();
		goto lbExit;
	}
	MY_DBG("[createFullFrame] mpMdpHal->waitDone() done.");
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: mpMdpHal->waitDone() Time");
                                        #endif
	mpIspHal->stop();
	MY_DBG("[createFullFrame] Stop ISP.");
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: mpIspHal->stop() Time");
                                        #endif

	mpMdpHal->stop();
	MY_DBG("[createFullFrame] Stop MDP.");
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: mpMdpHal->stop() Time");
                                        #endif

	#if HDR_DEBUG_SAVE_SOURCE_IMAGE	// Save source images for debug.
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		char szFileName[100];
		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_1_%s_%dx%d.yuv", mu4RunningNumber, mpSourceImgBuf[i]->getPoolName(), mu4W_yuv, mu4H_yuv);
		mHalMiscDumpToFile(szFileName, (UINT8*)mpSourceImgBuf[i]->getVirtAddr(), mu4SourceSize);
		MY_VERB("[createFullFrame] Save %s done.", szFileName);
	}
	#endif	// HDR_DEBUG_SAVE_SOURCE_IMAGE

lbExit:

#if (HDR_PROFILE_CAPTURE3)
	DbgTmr.print("HdrProfiling3:: createFullFrameFinish Time");
#endif

	MY_DBG("[createFullFrame] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
createFullFrame2Pass(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[createFullFrame2Pass] - E. u4OutputFrameNum: %d. v.07.", u4OutputFrameNum);
	MBOOL  ret = MTRUE; // MTRUE: No error.
	MINT32 err = 0; // 0: No error.
	MINT32 i4Iso = 0;

	using namespace NSCamera;

#if (HDR_PROFILE_CAPTURE3)
    MyDbgTimer DbgTmr("createFullFrame2Pass");
#endif

    //	(0) Config In Ports.
    PortInfo InPortInfo;
    PortInfo OutPortInfo;
    PortInfo BpsPort;
    PortInfo OutPortInfoSmall;

    halISPIFParam_t ISPParam;
    halIDPParam_t	MDPParam;
    char acDbgFileSaveSwitch[PROPERTY_VALUE_MAX] = {'\0'};  // For saving raw/yuv files for debug.


    InPortInfo = PortInfo(ImgInfo(ePIXEL_FORMAT_BAYER10, mu4W_raw, mu4H_raw), BufInfo(NULL, NULL, 0, 0)); // 2pass only supports RAW input format.
    BpsPort = PortInfo(ImgInfo(ePIXEL_FORMAT_YUY2, mu4W_yuv, mu4H_yuv));

    // Pass 1: Capture 3 raw image from sensor.
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		MY_DBG("[createFullFrame2Pass] <%d/%d Capture>", i+1, u4OutputFrameNum);
		MY_DBG("[createFullFrame2Pass] Using buffer: %s. Addr: 0x%08X. Size: %d.", mpRawImgBuf[i]->getPoolName(), mpRawImgBuf[i]->getVirtAddr(), mpRawImgBuf[i]->getPoolSize());

        //////////////////////////////////////////////////////////////////
        //                     1st VD start                             //
        //////////////////////////////////////////////////////////////////
		ret =
                //  Set ExpT and Sensor Gain through Feature Control Function to sensor driver.
				0==(err = mpHal3A->setCaptureHDRParam(i/*a_u4CntIdx*/, 0/*a_IsRawGain*/)) // a_u4CntIdx: the index which you want to capture. Range: 0~2; a_IsRawGain: 1(raw gain setting)/0(sensor exposure time and gain setting)/2(reset raw gain to 128(1x)).
                                        #if (HDR_PROFILE_CAPTURE3)
                                        &&	DbgTmr.print("HdrProfiling3:: setCaptureHDRParam Time")
                                        #endif
                //  Update ISO info to ISP.
				//      Get ISO from 3A hal.
			&&	0==(err = mpHal3A->getISO(1, &i4Iso))
				//      Set ISO to ISP hal.
			&&	0==(err = mpIspHal->sendCommand(ISP_CMD_SET_ISO, i4Iso))
				;
        MY_DBG("[createFullFrame2Pass] Set ExpT/SensorGain, and update ISO to ISP: %d.", i4Iso);
		if (! ret)
		{
		    MY_ERR("[createFullFrame2Pass] Set ExpT/SensorGain, and update ISO to ISP fail. - ec(%d).", err);
			goto lbExit;
		}

        // Wait for previous round's ISP done.
        #if 0  // For Fast Capture.
        if (i != 0)
        {
            // Wait ISP done.
            err = mpIspHal->waitDone(ISP_HAL_IRQ_CLEAR_WAIT | ISP_HAL_IRQ_EXPDONE);
            if  (err)
            {
                MY_ERR("[createFullFrame2Pass] mpIspHal->waitDone() %02d fail - ec(%x).", i, err);
                mpIspHal->stop();
                ret = false;
                goto lbExit;
            }

        	MY_DBG("[createFullFrame2Pass] mpIspHal->waitDone() %02d done.", i);
                                                #if (HDR_PROFILE_CAPTURE3)
                                                DbgTmr.print("HdrProfiling3:: mpIspHal->waitDone() Time");
                                                #endif
        }
        #endif  // For Fast Capture.

        OutPortInfo = PortInfo(ImgInfo(ePIXEL_FORMAT_BAYER10,  mu4W_raw, mu4H_raw), BufInfo(mpRawImgBuf[i]->getVirtAddr(), mpRawImgBuf[i]->getVirtAddr(), mu4RawSize));

        //	(1) Config ISPParam
        configISPParam(&ISPParam, InPortInfo, OutPortInfo);
                                            #if (HDR_PROFILE_CAPTURE3)
                                            DbgTmr.print("HdrProfiling3:: configISPParam Time");
                                            #endif
        //	(2) Set CamMode
        setIspMode(ISP_CAM_MODE_CAPTURE_PASS1);
                                            #if (HDR_PROFILE_CAPTURE3)
                                            DbgTmr.print("HdrProfiling3:: setIspMode Time");
                                            #endif

        // Wait for SENSOR DRV ready flag.
		err = mpIspHal->sendCommand(ISP_HAL_CMD_SET_SENSOR_WAIT, ISP_HAL_SENSOR_WAIT_SHUTTER_GAIN, 500/*time-out in msec*/);
		if (err)
		{
		    MY_ERR("[createFullFrame2Pass] ISP_HAL_CMD_SET_SENSOR_WAIT %02d fail. - ec(%d).", i+1, err);
			goto lbExit;
		}
        MY_DBG("[createFullFrame2Pass] SENSOR_WAIT %02d received.", i+1);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: SENSOR_WAIT Time");
                                        #endif

        // Set ExpT and Sensor Gain through capture function in Sensor driver to sensor.
        ISPParam.u4IsBypassSensorDelay = 1; // This will make SP_DELAY use 0 in isp_hal.cpp. Fast Capture flow must use SP_DELAY = 0.
		err = mpIspHal->setConf(&ISPParam);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: mpIspHal->setConf Time");
                                        #endif
        #if 1
        // Wait VSYNC.
    	err = mpIspHal->waitDone(ISP_HAL_IRQ_CLEAR_WAIT | ISP_HAL_IRQ_VSYNC);    // 1st VD end. 2nd VD start. ISP_HAL_IRQ_CLEAR_WAIT: clear remnant VSYNC.
		if (err)
		{
		    MY_ERR("[createFullFrame2Pass] wait VSYNC %02d fail. - ec(%d).", 2 * i + 1, err);
			goto lbExit;
		}
        MY_DBG("[createFullFrame2Pass] VSYNC %02d received.", 2 * i + 1);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: VSYNC_WAIT Time");
                                        #endif
        #endif
        //////////////////////////////////////////////////////////////////
        //                     1st VD end                               //
        //////////////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////
        //                     2nd VD start                             //
        //////////////////////////////////////////////////////////////////
        #if 0  // For Fast Capture.
        // For previous round's ISP done. Stop ISP when ISP is done.
        if (i != 0)
        {

        	err = mpIspHal->stop();
        	MY_DBG("[createFullFrame2Pass] Stop ISP %02d done.", i);
                                                #if (HDR_PROFILE_CAPTURE3)
                                                DbgTmr.print("HdrProfiling3:: mpIspHal->stop() Time");
                                                #endif
        }
        #endif  // For Fast Capture.

        // Set RawGain.
		err = mpHal3A->setCaptureHDRParam(i/*a_u4CntIdx*/, 1/*a_IsRawGain*/); // a_u4CntIdx: the index which you want to capture. Range: 0~2; a_IsRawGain: 1(raw gain setting)/0(sensor exposure time and gain setting)/2(reset raw gain to 128(1x)).
        MY_DBG("[createFullFrame2Pass] Set RawGain %02d done.", i+1);

        // Start ISP (i.e. start capture).
        err = mpIspHal->start();
        if (err)
        {
            MY_ERR("[createFullFrame2Pass] mpIspHal->start() %02d fail - ec(%x).", i+1, err);
            ret = false;
            goto lbExit;
        }
        MY_DBG("[createFullFrame2Pass] mpIspHal->start() %02d done.", i+1);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: mpIspHal->start() Time");
                                        #endif

        #if 1   // Wait for VSYNC.
        MUINT32 ProlongedVd = CustomHdrProlongedVdGet();
        for (MUINT32 j = 0; j < ProlongedVd; j++)
        {
    		err = mpIspHal->waitDone(ISP_HAL_IRQ_CLEAR_WAIT | ISP_HAL_IRQ_VSYNC);    // 2nd VD end. 3rd VD start. ISP_HAL_IRQ_CLEAR_WAIT: clear remnant VSYNC.
                                            #if (HDR_PROFILE_CAPTURE3)
                                            DbgTmr.print("HdrProfiling3:: VSYNC_WAIT Time");
                                            #endif
    		if (err)
    		{
    		    MY_ERR("[createFullFrame2Pass] wait VSYNC %02d-%d fail. - ec(%d).", 2 * i + 2, j, err);
    			goto lbExit;
    		}
            MY_DBG("[createFullFrame2Pass] VSYNC %02d-%d received. ProlongedVd: %d.", 2 * i + 2, j, ProlongedVd);
            //////////////////////////////////////////////////////////////////
            //                     2nd VD end                               //
            //////////////////////////////////////////////////////////////////

    	}
        #endif

        #if 1   // Move "Wait ISP Done" and "Stop ISP" to end of every capture.
        // Wait ISP done.
        err = mpIspHal->waitDone(ISP_HAL_IRQ_CLEAR_WAIT | ISP_HAL_IRQ_EXPDONE);
        if  (err)
        {
            MY_ERR("[createFullFrame2Pass] mpIspHal->waitDone() %02d fail - ec(%x).", i+1, err);
            mpIspHal->stop();
            ret = false;
            goto lbExit;
        }

        MY_DBG("[createFullFrame2Pass] mpIspHal->waitDone() %02d done.", i+1);
                                            #if (HDR_PROFILE_CAPTURE3)
                                            DbgTmr.print("HdrProfiling3:: mpIspHal->waitDone() Time");
                                            #endif
        // Wait for ISP Stop.
        err = mpIspHal->stop();
        MY_DBG("[createFullFrame2Pass] Stop ISP %02d done.", i+1);
                                            #if (HDR_PROFILE_CAPTURE3)
                                            DbgTmr.print("HdrProfiling3:: mpIspHal->stop() Time");
                                            #endif
        #endif   // Move "Wait ISP Done" and "Stop ISP" to end of every capture.

	}

    #if 0  // For Fast Capture.
    // Wait ISP done.
    err = mpIspHal->waitDone(ISP_HAL_IRQ_CLEAR_WAIT | ISP_HAL_IRQ_EXPDONE);
    if  (err)
    {
        MY_ERR("[createFullFrame2Pass] mpIspHal->waitDone() - ec(%x)", err);
        mpIspHal->stop();
        ret = false;
        goto lbExit;
    }

    MY_DBG("[createFullFrame2Pass] mpIspHal->waitDone() done.");
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: mpIspHal->waitDone() Time");
                                        #endif

	mpIspHal->stop();
	MY_DBG("[createFullFrame2Pass] Stop ISP.");
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: mpIspHal->stop() Time");
                                        #endif
    #endif  // For Fast Capture.

    #if 1   // Save raw file for debug.
    {
        acDbgFileSaveSwitch[0] = '0';   // Reset acDbgFileSaveSwitch[0];
        property_get("hdr.debug.saveraw", acDbgFileSaveSwitch, "0");
   		MY_VERB("[createFullFrame2Pass] hdr.debug.saveraw: %c.", acDbgFileSaveSwitch[0]);
        if (acDbgFileSaveSwitch[0] != '0')
        {
        	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
            {
        		char szFileName[100];
        		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_1_%s_%dx%d.raw", mu4RunningNumber, mpRawImgBuf[i]->getPoolName(), mu4W_raw, mu4H_raw);
        		mHalMiscDumpToFile(szFileName, (UINT8*)mpRawImgBuf[i]->getVirtAddr(), mu4RawSize);
        		MY_VERB("[createFullFrame2Pass] Save %s done.", szFileName);
        	}
        }
    }
    #endif  // Save raw file for debug.

    // Pass 2: Input 3 raw image from DRAM.
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
        InPortInfo = PortInfo(ImgInfo(ePIXEL_FORMAT_BAYER10, mu4W_raw, mu4H_raw), BufInfo(mpRawImgBuf[i]->getVirtAddr(), mpRawImgBuf[i]->getVirtAddr(), mu4RawSize));
        BpsPort = PortInfo(ImgInfo(ePIXEL_FORMAT_YUY2, mu4W_yuv, mu4H_yuv));

		//	(0) Config Out Ports. Each round will use different OutPort.
		OutPortInfo = PortInfo(ImgInfo(ePIXEL_FORMAT_YV12, mu4W_yuv, mu4H_yuv), BufInfo(mpSourceImgBuf[i]->getVirtAddr(), mpSourceImgBuf[i]->getVirtAddr(), mu4SourceSize));
		OutPortInfoSmall = PortInfo(ImgInfo(ePIXEL_FORMAT_Y800, mu4W_small, mu4H_small), BufInfo(mpSmallImgBuf[i]->getVirtAddr(), mpSmallImgBuf[i]->getVirtAddr(), mu4SmallImgSize));

        configISPParam(&ISPParam, InPortInfo, BpsPort);

		//	(1) Config MDPParam
#if (HDR_MDP_TWO_OUT)
		configMDPParamTwoOut(&MDPParam, BpsPort, OutPortInfo, OutPortInfoSmall);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: configMDPParamTwoOut Time");
                                        #endif
#else   // original
		configMDPParam(&MDPParam, BpsPort, OutPortInfo);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: configMDPParam Time");
                                        #endif
#endif  // HDR_MDP_TWO_OUT

		//	(2) Config MDP/ISP
        setIspMode(ISP_CAM_MODE_CAPTURE_PASS2);
        //ISPParam.u4IsBypassSensorDelay = 1; // This will make SP_DELAY use 0 in isp_hal.cpp. Fast Capture flow must use SP_DELAY = 0.
		ret =
			    0==(err = mpMdpHal->setConf(&MDPParam))
                                        #if (HDR_PROFILE_CAPTURE3)
                                        &&	DbgTmr.print("HdrProfiling3:: mpMdpHal->setConf Time")
                                        #endif
			&&	0==(err = mpIspHal->setConf(&ISPParam))
                                        #if (HDR_PROFILE_CAPTURE3)
                                        &&	DbgTmr.print("HdrProfiling3:: mpIspHal->setConf Time")
                                        #endif
                ;
		if	( ! ret )
		{
			MY_ERR("[createFullFrame] Config MDP/ISP fail. ec(%x)", err);
			goto lbExit;
		}

		//	(3) Set ISP Raw Gain
		ret =
				0==(err = mpHal3A->setCaptureHDRParam(i/*a_u4CntIdx*/, 2/*a_IsRawGain*/)) // a_u4CntIdx: the index which you want to capture. Range: 0~2; a_IsRawGain: 1(raw gain setting)/0(sensor exposure time and gain setting)/2(reset raw gain to 128(1x)).
			&&	do_Isp()  // Start MDP and ISP.
				;
		if	( ! ret )
		{
			MY_ERR("[createFullFrame] camera_start %02d fail. ec(%x)", i+1, err);
			goto lbExit;
		}
        MY_DBG("[createFullFrame] do_Isp() %02d done.", i+1);
                                        #if (HDR_PROFILE_CAPTURE3)
                                        DbgTmr.print("HdrProfiling3:: camera_start Time");
                                        #endif


	}

	#if 1	// Save yuv images for debug.
	{
        acDbgFileSaveSwitch[0] = '0';   // Reset acDbgFileSaveSwitch[0].
        property_get("hdr.debug.saveyuv", acDbgFileSaveSwitch, "0");
   		MY_VERB("[createFullFrame2Pass] hdr.debug.saveyuv: %c.", acDbgFileSaveSwitch[0]);
        if (acDbgFileSaveSwitch[0] != '0')
        {
        	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
        	{
        		char szFileName[100];
        		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_1_%s_%dx%d.yuv", mu4RunningNumber, mpSourceImgBuf[i]->getPoolName(), mu4W_yuv, mu4H_yuv);
        		mHalMiscDumpToFile(szFileName, (UINT8*)mpSourceImgBuf[i]->getVirtAddr(), mu4SourceSize);
        		MY_VERB("[createFullFrame] Save %s done.", szFileName);
        	}
        }
	}
	#endif	// Save yuv images for debug.


lbExit:

#if (HDR_PROFILE_CAPTURE3)
	DbgTmr.print("HdrProfiling3:: createFullFrameFinish Time");
#endif

	MY_DBG("[createFullFrame] - X. ret: %d.", ret);

	return	ret;

}


#if 1
/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
createFullFrame2PassExample(void)
{
    using namespace NSCamera;
    MY_DBG("[createFullFrame2PassExample] in v0.7");
    MBOOL  ret = MTRUE;
    MINT32 err = 0;
    PortInfo InPortInfo;
    PortInfo BpsPort;
    PortInfo OutPortInfo;

    halISPIFParam_t ISPParam;
    halIDPParam_t   MDPParam;

for (MUINT32 i = 0; i < 3; i++)
{
    InPortInfo= PortInfo(ImgInfo(ePIXEL_FORMAT_BAYER10, mu4W_raw, mu4H_raw),BufInfo(NULL, NULL, 0, 0));
    OutPortInfo= PortInfo(ImgInfo(ePIXEL_FORMAT_BAYER10, mu4W_raw, mu4H_raw),BufInfo(mpSourceImgBuf[i]->getVirtAddr(), mpSourceImgBuf[i]->getVirtAddr(), mu4SourceSize));


    configISPParam(&ISPParam, InPortInfo, OutPortInfo);
    ret =   setIspMode(ISP_CAM_MODE_CAPTURE_PASS1)
        &&  0==(err = mpHal3A->setCaptureParam(0))
        &&  0==(err = mpIspHal->setConf(&ISPParam));
    if  ( (err = mpIspHal->start()) )
    {
        MY_ERR("[createFullFrame2PassExample] mpIspHal->start() - ec(%x)", err);
        ret = false;
        goto lbExit;
    }
    if  ( (err = mpIspHal->waitDone(ISP_HAL_IRQ_EXPDONE)) )
    {
        MY_ERR("[createFullFrame2PassExample] mpIspHal->waitDone() - ec(%x)", err);
        ret = false;
        goto lbExit;
    }
    MY_INFO("[createFullFrame2PassExample] pass one done. i: %d", i);
}

#if 1
    //char szFileName[100];
    //::sprintf(szFileName, "/mnt/sdcard2/DCIM/Camera/%s_%dx%d.raw", "BayerA", mu4W_yuv,mu4H_yuv);
    //mHalMiscDumpToFile(szFileName, (UINT8*)mpBlurImg->getVirtAddr(), mu4SourceSize);
    //MY_INFO("[createFullFrame] Save File done");

    InPortInfo= PortInfo(ImgInfo(ePIXEL_FORMAT_BAYER10, mu4W_raw, mu4H_raw),BufInfo(mpSourceImgBuf[0]->getVirtAddr(), mpSourceImgBuf[0]->getVirtAddr(), mu4SourceSize));
    BpsPort= PortInfo(ImgInfo(ePIXEL_FORMAT_YUY2, mu4W_yuv, mu4H_yuv));
    mu4W_yuv=mu4W_yuv-(mu4W_yuv%4);
    if(mShotParam.u4ZoomRatio!=100)
         OutPortInfo= PortInfo(ImgInfo(ePIXEL_FORMAT_YUY2, mu4W_yuv, mu4H_yuv),BufInfo(mpSourceImgBuf[0]->getVirtAddr(), mpSourceImgBuf[0]->getVirtAddr(), mu4SourceSize));
    else
         OutPortInfo= PortInfo(ImgInfo(ePIXEL_FORMAT_YUY2, mu4W_yuv, mu4H_yuv),BufInfo(mpSourceImgBuf[0]->getVirtAddr(), mpSourceImgBuf[0]->getVirtAddr(), mu4SourceSize));

    configISPParam(&ISPParam, InPortInfo, BpsPort);
    configMDPParam(&MDPParam, BpsPort,    OutPortInfo);

    ret =   setIspMode(ISP_CAM_MODE_CAPTURE_PASS2)
        &&  0==(err = mpHal3A->setCaptureParam(1))
        &&  0==(err = mpMdpHal->setConf(&MDPParam))
        &&  0==(err = mpIspHal->setConf(&ISPParam))
            ;
    if  ( ! ret )
    {
        MY_ERR("[createFullFrame2PassExample] Config ISP/3A - ec(%x)", err);
        ret = false;
        goto lbExit;
    }

    //  (4)
    ret = do_Isp();
    if  ( ! ret )
    {
        MY_ERR("[createFullFrame2PassExample] waitDone");
        ret = false;
        goto lbExit;
    }
    MY_INFO("[createFullFrame2PassExample] pass two done");

#endif

lbExit:
    MY_DBG("[createFullFrame2PassExample] out");
    return  ret;
}
#endif



/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
createSmallImg(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[createSmallImg] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL  ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
    MyDbgTimer DbgTmr("createSmallImg");
#endif

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		ret = MDPResize((void*)mpSourceImgBuf[i]->getVirtAddr(), mu4W_yuv, mu4H_yuv, MHAL_FORMAT_YUV_420, (void*)mpSmallImgBuf[i]->getVirtAddr(), mu4W_small, mu4H_small, MHAL_FORMAT_Y800);
	}


#if (HDR_PROFILE_CAPTURE)
	DbgTmr.print("HdrProfiling:: createSmallImg Time");
#endif
	MY_DBG("[createSmallImg] - X. ret: %d.", ret);

	return	ret;

}

MBOOL
Hdr::
saveSmallImgForDebug(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
    MUINT32 i = 0;
	MY_DBG("[saveSmallImgForDebug] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL  ret = MTRUE;

	#if HDR_DEBUG_SAVE_SMALL_IMAGE	// Save Small Img for debug.
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		char szFileName[100];
		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_2_%s_%dx%d.raw", mu4RunningNumber, mpSmallImgBuf[i]->getPoolName(), mu4W_small, mu4H_small);
		mHalMiscDumpToFile(szFileName, (UINT8*)mpSmallImgBuf[i]->getVirtAddr(), mu4SmallImgSize);
		MY_VERB("[createSmallImg] Save %s done.", szFileName);
	}
	#endif	// HDR_DEBUG_SAVE_SMALL_IMAGE

	MY_DBG("[saveSmallImgForDebug] - X. ret: %d.", ret);
	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
createSwEisImg(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[createSwEisImg] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL  ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
	MyDbgTimer DbgTmr("createSwEisImg");
#endif

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		ret = MDPResize((void*)mpSmallImgBuf[i]->getVirtAddr(), mu4W_small, mu4H_small, MHAL_FORMAT_Y800, (void*)mpSwEisImgBuf[i]->getVirtAddr(), mu4W_sweis, mu4H_sweis, MHAL_FORMAT_Y800);
	}

#if (HDR_PROFILE_CAPTURE)
	DbgTmr.print("HdrProfiling:: createSwEisImg Time");
#endif

	#if HDR_DEBUG_SAVE_SW_EIS_IMAGE	// Save SwEis Img for debug.
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		char szFileName[100];
		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_4_%s_%dx%d.raw", mu4RunningNumber, mpSwEisImgBuf[i]->getPoolName(), mu4W_sweis, mu4H_sweis);
		mHalMiscDumpToFile(szFileName, (UINT8*)mpSwEisImgBuf[i]->getVirtAddr(), mu4SwEisImgSize);
		MY_VERB("[createSwEisImg] Save %s done.", szFileName);
	}
	#endif	// HDR_DEBUG_SAVE_SW_EIS_IMAGE


	MY_DBG("[createSwEisImg] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
MDPResize(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, MUINT32 srcFormat, void* desAdr, MUINT32 desWidth, MUINT32 desHeight, MUINT32 dstFormat) const
{
	MY_DBG("[MDPResize] - E. srcAdr: 0x%08X. srcW/H: (%d, %d). srcFormat: %d. desAdr: 0x%08X. desW/H: (%d, %d). dstFormat: %d.", srcAdr, srcWidth, srcHeight, srcFormat, desAdr, desWidth, desHeight, dstFormat);
	MY_VERB("[MDPResize] srcAdr: 0x%08X. srcW/H: (%d, %d). srcFormat: %d.", srcAdr, srcWidth, srcHeight, srcFormat);
	MY_VERB("[MDPResize] desAdr: 0x%08X. desW/H: (%d, %d). dstFormat: %d.", desAdr, desWidth, desHeight, dstFormat);
	MBOOL	ret = MTRUE;

	mHalBltParam_t bltParam;
	::memset(&bltParam, 0, sizeof(mHalBltParam_t));

	bltParam.srcX = 0;
	bltParam.srcY = 0;
	bltParam.srcW = srcWidth;
	bltParam.srcWStride = srcWidth;
	bltParam.srcH = srcHeight;
	bltParam.srcHStride = srcHeight;
	bltParam.srcAddr = (MUINT32)srcAdr;
//	  bltParam.srcFormat = MHAL_FORMAT_YUV_422_PL;
	bltParam.srcFormat = srcFormat;	// MHAL_FORMAT_YUV_420

	bltParam.dstW = desWidth;
	bltParam.dstH = desHeight;
	bltParam.dstAddr = (MUINT32)desAdr;
	bltParam.favor_flags = ITFF_USE_CRZ;
	//bltParam.dstFormat = MHAL_FORMAT_YUV_420;
	bltParam.dstFormat = dstFormat;	// MHAL_FORMAT_Y800

	bltParam.pitch = desWidth;

	bltParam.orientation = 0; //TODO: test 5
	bltParam.doImageProcess = 0;
	Mt6575_mHalBitblt((void *)&bltParam);

	MY_DBG("[MDPResize] - X. ret: %d.", ret);
	return	ret;

}

/*******************************************************************************
*
*******************************************************************************/
int G_u4Count = 0;

MBOOL
Hdr::
MDPRotate(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MUINT32 desWidth, MUINT32 desHeight) const
{
    mHalBltParam_t bltParam;

	MY_DBG("[MDPRotate] - E. srcAdr: 0x%08X. srcW/H: (%d, %d). desAdr: 0x%08X. desW/H: (%d, %d). mShotParam.u4JPEGOrientation: %d.", srcAdr, srcWidth, srcHeight, desAdr, desWidth, desHeight, mShotParam.u4JPEGOrientation);

    ::memset(&bltParam, 0, sizeof(mHalBltParam_t));
    bltParam.srcX = 0;
    bltParam.srcY = 0;
    bltParam.srcW = srcWidth;
    bltParam.srcWStride = srcWidth;
    bltParam.srcH = srcHeight;
    bltParam.srcHStride = srcHeight;
    bltParam.srcAddr = (MUINT32)srcAdr;
    bltParam.srcFormat = MHAL_FORMAT_YUY2;

    bltParam.dstW = desWidth;
    bltParam.dstH = desHeight;
    bltParam.dstAddr = (MUINT32)desAdr;
//    bltParam.favor_flags = ITFF_USE_CRZ;
    //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
    bltParam.dstFormat = MHAL_FORMAT_YUY2;

    bltParam.pitch = desWidth;
    bltParam.resz_up_scale_coeff = 1;    //0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable
    bltParam.resz_dn_scale_coeff = 7;       //0:linear interpolation 1:most blur 19:sharpest 15:recommeneded
    bltParam.resz_ee_h_str = 0;          //down scale only/0~15
    bltParam.resz_ee_v_str = 0;          //down scale only/0~15

    bltParam.orientation = mShotParam.u4JPEGOrientation; //TODO: test 5
    bltParam.doImageProcess = 0;
    Mdp_BitbltSlice((void *)&bltParam);

    //Sava Test
    #if 0
	MY_DBG("[MDPRotate] G_u4Count: %d.", G_u4Count);
    //if (G_u4Count == 0)
    {
       char szFileName[100];
       ::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_%s_%02d_%dx%d.yuv", mu4RunningNumber, "HdrRotate_src", G_u4Count, srcWidth, srcHeight);
       mHalMiscDumpToFile(szFileName, (UINT8*)srcAdr, srcWidth*srcHeight*2);

       ::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_%s_%02d_%dx%d.yuv", mu4RunningNumber, "HdrRotate_dst", G_u4Count, desWidth, desHeight);
       mHalMiscDumpToFile(szFileName, (UINT8*)desAdr, desWidth*desHeight*2);
       MY_DBG("[MDPRotate] Save %s done. srcAdr:0x08%x desAdr:0x08%x", szFileName, srcAdr, desAdr);
    }
    G_u4Count = (G_u4Count+1) % 2;
    #endif

	MY_DBG("[MDPRotate] - X.");

    return  MTRUE;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
Hdr::
MDPImgTrasn(void* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, void* desAdr, MHAL_BITBLT_FORMAT_ENUM srcformat, MHAL_BITBLT_FORMAT_ENUM desformat) const
{
    mHalBltParam_t bltParam;

    MY_INFO("[MDPImgTrasn] srcAdr 0x%x. srcWidth %d. srcHeight %d. srcformat: %d.",(MUINT32)srcAdr, srcWidth,srcHeight, srcformat);
    MY_INFO("[MDPImgTrasn] dstAdr 0x%x. dstWidth %d. dstHeight %d. desformat: %d.",(MUINT32)desAdr, srcWidth,srcHeight, desformat);
    ::memset(&bltParam, 0, sizeof(mHalBltParam_t));
    bltParam.srcX = 0;
    bltParam.srcY = 0;
    bltParam.srcW = srcWidth;
    bltParam.srcWStride = srcWidth;
    bltParam.srcH = srcHeight;
    bltParam.srcHStride = srcHeight;
    bltParam.srcAddr = (MUINT32)srcAdr;
    bltParam.srcFormat = srcformat;

    bltParam.dstW = srcWidth;
    bltParam.dstH = srcHeight;
    bltParam.dstAddr = (MUINT32)desAdr;
    bltParam.favor_flags = ITFF_USE_CRZ;
    //bltParam.dstFormat = MHAL_FORMAT_YUV_420;
    bltParam.dstFormat = desformat;

    bltParam.pitch = srcWidth;

    bltParam.resz_up_scale_coeff = 1;    //0:linear interpolation 1:most blur 19:sharpest 8:recommeneded >12:undesirable
    bltParam.resz_dn_scale_coeff = 7;       //0:linear interpolation 1:most blur 19:sharpest 15:recommeneded
    bltParam.resz_ee_h_str = 0;          //down scale only/0~15
    bltParam.resz_ee_v_str = 0;          //down scale only/0~15

    bltParam.orientation = 0; //TODO: test 5
    bltParam.doImageProcess = 0;
    mHalMdp_BitBlt((void *)&bltParam);

    return  MTRUE;
}

///////////////////////////////////////////////////////////////////////////
/// @brief Do small image normalization.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
do_Normalization(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[do_Normalization] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL  ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
	MyDbgTimer DbgTmr("do_Normalization");
#endif


	HDR_PIPE_CONFIG_PARAM rHdrPipeConfigParam;
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		rHdrPipeConfigParam.pSmallImgBufAddr[i] = mpSmallImgBuf[i]->getVirtAddr();	// small_image_addr[3]; // small image address
	}

	ret =
			// Config HDR Parameters.
			mpHdrHal->HdrSmallImgBufSet(rHdrPipeConfigParam)

#if (HDR_PROFILE_CAPTURE)
		&&	DbgTmr.print("HdrProfiling:: HdrSmallImgBufSet Time")
#endif
			// Normalize small images. Normalized images are put back to SmallImgbuf[].
		&&	mpHdrHal->Do_Normalization()

#if (HDR_PROFILE_CAPTURE)
		&&	DbgTmr.print("HdrProfiling:: Do_Normalization Time")
#endif
			;

	// Save normalized small image for debug.
	#if HDR_DEBUG_SAVE_NORMALIZED_SMALL_IMAGE
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		char szFileName[100];
		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_3_normalized_%s_%dx%d.raw", mu4RunningNumber, mpSmallImgBuf[i]->getPoolName(), mu4W_small, mu4H_small);
		mHalMiscDumpToFile(szFileName, (UINT8*)mpSmallImgBuf[i]->getVirtAddr(), mu4SmallImgSize);
		MY_VERB("[doHdr] Save %s done.", szFileName);
	}
	#endif	// HDR_DEBUG_SAVE_NORMALIZED_SMALL_IMAGE


	MY_DBG("[do_Normalization] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Do SW Eis to get GMV.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
do_SwEis(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[do_SwEis] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL  ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
	MyDbgTimer DbgTmr("do_SwEis");
#endif

	// Prepare SwEis Input Info.
	HDR_PIPE_SW_EIS_INPUT_INFO rHdrPipeSwEisInputInfo;
	rHdrPipeSwEisInputInfo.u2SwEisImgWidth	= mu4W_sweis;
	rHdrPipeSwEisInputInfo.u2SwEisImgHeight = mu4H_sweis;
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		rHdrPipeSwEisInputInfo.pSwEisImgBufAddr[i] = mpSwEisImgBuf[i]->getVirtAddr();
	}

	// Do SW EIS.
	ret = mpHdrHal->Do_SwEis(rHdrPipeSwEisInputInfo);


#if (HDR_PROFILE_CAPTURE)
	DbgTmr.print("HdrProfiling:: do_SwEis Time");
#endif

	MY_DBG("[do_SwEis] - X. ret: %d.", ret);

	return	ret;

}



///////////////////////////////////////////////////////////////////////////
/// @brief Do Feature Extraction.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
do_FeatureExtraction(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[do_FeatureExtraction] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL  ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
	MyDbgTimer DbgTmr("do_FeatureExtraction");
#endif


	// Do Feature Extraction and Feature Matching.
	//	   Register MAV working buffer.
	mpHdrHal->ConfigMavParam(mpMavWorkingBuf->getVirtAddr());
	//	   Config MAV hal init Info.
	HDR_PIPE_FEATURE_EXTRACT_INPUT_INFO rHdrPipeFeatureExtractInputInfo;
	rHdrPipeFeatureExtractInputInfo.u2SmallImgW = mu4W_small;
	rHdrPipeFeatureExtractInputInfo.u2SmallImgH = mu4H_small;
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		rHdrPipeFeatureExtractInputInfo.pSmallImgBufAddr[i] = mpSmallImgBuf[i]->getVirtAddr();
	}
	//	   Do Feature Extraction and Feature Matching.
	mpHdrHal->Do_FeatureExtraction(rHdrPipeFeatureExtractInputInfo);


#if (HDR_PROFILE_CAPTURE)
	DbgTmr.print("HdrProfiling:: Do_FeatureExtraction Time");
#endif

	MY_DBG("[do_FeatureExtraction] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Do Alignment.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
do_Alignment(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[do_Alignment] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL  ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
	MyDbgTimer DbgTmr("do_Alignment");
#endif


	ret =
			mpHdrHal->HdrWorkingBufSet(mpHdrWorkingBuf->getVirtAddr(), mu4HdrWorkingBufSize)

#if (HDR_PROFILE_CAPTURE)
		&&	DbgTmr.print("HdrProfiling:: HdrWorkingBufSet Time")
#endif
		&&	mpHdrHal->Do_Alignment()
			;

#if (HDR_PROFILE_CAPTURE)
	DbgTmr.print("HdrProfiling:: Do_Alignment Time");
#endif

	MY_DBG("[do_Alignment] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Get original Weighting Table.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
do_OriWeightMapGet(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[do_OriWeightMapGet] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL  ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
	MyDbgTimer DbgTmr("do_OriWeightMapGet");
#endif


	// Get the resulting Weighting Map.
	mpHdrHal->WeightingMapInfoGet(OriWeight);
	// Show obtained OriWeightingTbl info.

#if (HDR_PROFILE_CAPTURE)
	DbgTmr.print("HdrProfiling:: do_OriWeightMapGet Time");
#endif

	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
		MY_VERB("[do_OriWeightMapGet] OriWeight[%d]->W/H: (%d, %d). Addr: 0x%08X.", i, OriWeight[i]->weight_table_width, OriWeight[i]->weight_table_height, OriWeight[i]->weight_table_data);

	#if HDR_DEBUG_SAVE_WEIGHTING_MAP	// Save Weighting Map for debug.
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		char szFileName[100];
		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_5_WeightMap%d_%dx%d.raw", mu4RunningNumber, i, OriWeight[i]->weight_table_width, OriWeight[i]->weight_table_height);
		mHalMiscDumpToFile(szFileName, OriWeight[i]->weight_table_data, OriWeight[i]->weight_table_width * OriWeight[i]->weight_table_height);
		MY_VERB("[doHdr] Save %s done.", szFileName);
	}
	#endif	// HDR_DEBUG_SAVE_WEIGHTING_MAP

	MY_DBG("[do_OriWeightMapGet] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Do Down-scale Weighting Map.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
do_DownScaleWeightMap(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[do_DownScaleWeightMap] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL  ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
	MyDbgTimer DbgTmr("do_DownScaleWeightMap");
#endif


	//	   Down-size Weighting Map. And Save them for debug.
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		ret = MDPResize((void*)OriWeight[i]->weight_table_data, OriWeight[i]->weight_table_width, OriWeight[i]->weight_table_height, MHAL_FORMAT_Y800, (void*)mpDownSizedWeightMapBuf[i]->getVirtAddr(), mu4W_dsmap, mu4H_dsmap, MHAL_FORMAT_Y800);

	}

#if (HDR_PROFILE_CAPTURE)
	DbgTmr.print("HdrProfiling:: Down-scaleWeightMap Time");
#endif

	#if HDR_DEBUG_SAVE_DOWNSCALED_WEIGHTING_MAP	// Save down-scaled weighting map for debug.
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		char szFileName[100];
		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_6_%s_%dx%d.raw", mu4RunningNumber, mpDownSizedWeightMapBuf[i]->getPoolName(), mu4W_dsmap, mu4H_dsmap);
		mHalMiscDumpToFile(szFileName, (UINT8*)mpDownSizedWeightMapBuf[i]->getVirtAddr(), mu4DownSizedWeightMapSize);
		MY_VERB("[do_DownScaleWeightMap] Save %s done.", szFileName);
	}
	#endif	// HDR_DEBUG_SAVE_DOWNSCALED_WEIGHTING_MAP

	MY_DBG("[do_DownScaleWeightMap] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Do Up-scale Weighting Map.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
do_UpScaleWeightMap(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[do_UpScaleWeightMap] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL  ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
	MyDbgTimer DbgTmr("do_UpScaleWeightMap");
#endif


	// Up-sample Down-sized Weighting Map to make them blurry. And Save them for debug.
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		ret = MDPResize((void*)mpDownSizedWeightMapBuf[i]->getVirtAddr(), mu4W_dsmap, mu4H_dsmap, MHAL_FORMAT_Y800, (void*)BlurredWeight[i]->weight_table_data, BlurredWeight[i]->weight_table_width, BlurredWeight[i]->weight_table_height, MHAL_FORMAT_Y800);
	}

#if (HDR_PROFILE_CAPTURE)
	DbgTmr.print("HdrProfiling:: Up-scaleWeightMap Time");
#endif

	// Show info.
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
		MY_VERB("[do_UpScaleWeightMap] BlurredWeight[%d]->W/H: (%d, %d). Addr: 0x%08X.", i, BlurredWeight[i]->weight_table_width, BlurredWeight[i]->weight_table_height, BlurredWeight[i]->weight_table_height);

	#if HDR_DEBUG_SAVE_BLURRED_WEIGHTING_MAP	// Save blurred weighting map for debug.
	for (MUINT32 i = 0; i < u4OutputFrameNum; i++)
	{
		char szFileName[100];
		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_7_blurred_WeightMap%d_%dx%d.raw", mu4RunningNumber, i, BlurredWeight[i]->weight_table_width, BlurredWeight[i]->weight_table_height);
		mHalMiscDumpToFile(szFileName, BlurredWeight[i]->weight_table_data, BlurredWeight[i]->weight_table_width * BlurredWeight[i]->weight_table_height);
		MY_VERB("[do_UpScaleWeightMap] Save %s done.", szFileName);
	}
	#endif	// HDR_DEBUG_SAVE_BLURRED_WEIGHTING_MAP

	MY_DBG("[do_UpScaleWeightMap] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Do Fusion.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
do_Fusion(void)
{
	MUINT32 u4OutputFrameNum = OutputFrameNumGet();
	MY_DBG("[do_Fusion] - E. u4OutputFrameNum: %d.", u4OutputFrameNum);
	MBOOL  ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
	MyDbgTimer DbgTmr("do_Fusion");
#endif


	// Do Fusion.
	ret = mpHdrHal->Do_Fusion(BlurredWeight);

#if (HDR_PROFILE_CAPTURE)
	DbgTmr.print("HdrProfiling:: do_Fusion Time");
#endif

	MY_DBG("[do_Fusion] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Do .
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
do_HdrCroppedResultGet(void)
{
	MY_DBG("[do_HdrCroppedResultGet] - E.");
	MBOOL  ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
	MyDbgTimer DbgTmr("do_HdrCroppedResultGet");
#endif


	// Get HDR result.
	ret = mpHdrHal->HdrCroppedResultGet(mrHdrCroppedResult);
	MUINT32 u4HdrCroppedResultSize = mrHdrCroppedResult.output_image_width * mrHdrCroppedResult.output_image_height * 3 / 2;

#if (HDR_PROFILE_CAPTURE)
	DbgTmr.print("HdrProfiling:: do_HdrCroppedResultGet Time");
#endif

	MY_VERB("[do_HdrCroppedResultGet] rCroppedHdrResult:: W/H: (%d, %d). Addr: 0x%08X. Size: %d.", mrHdrCroppedResult.output_image_width, mrHdrCroppedResult.output_image_height, mrHdrCroppedResult.output_image_addr, u4HdrCroppedResultSize);	// *3/2: YUV420 size.

	#if HDR_DEBUG_SAVE_HDR_RESULT
	{
		char szFileName[100];
		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_8_HdrResult_%dx%d.yuv", mu4RunningNumber, mrHdrCroppedResult.output_image_width, mrHdrCroppedResult.output_image_height);
		mHalMiscDumpToFile(szFileName, (UINT8 *)mrHdrCroppedResult.output_image_addr, u4HdrCroppedResultSize);
		MY_VERB("[do_HdrCroppedResultGet] Save %s done.", szFileName);
	}
	#endif	// HDR_DEBUG_SAVE_HDR_RESULT


	MY_DBG("[do_HdrCroppedResultGet] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Do .
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
do_CroppedResultResize(void)
{
	MY_DBG("[do_CroppedResultResize] - E.");
	MBOOL  ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
	MyDbgTimer DbgTmr("do_CroppedResultResize");
#endif


	// Resize to original image size and convert to YUV22.
	ret = MDPResize((void*)mrHdrCroppedResult.output_image_addr, mrHdrCroppedResult.output_image_width, mrHdrCroppedResult.output_image_height, MHAL_FORMAT_YUV_420, (void*)mpResultImgBuf->getVirtAddr(), mu4W_yuv, mu4H_yuv, MHAL_FORMAT_YUY2 /*MHAL_FORMAT_UYVY*/);

#if (HDR_PROFILE_CAPTURE)
	DbgTmr.print("HdrProfiling:: do_CroppedResultResize Time");
#endif

	#if HDR_DEBUG_SAVE_RESIZE_HDR_RESULT
	{
		char szFileName[100];
		::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_9_final_HdrResult_%dx%d_yuv422.yuv", mu4RunningNumber, mu4W_yuv, mu4H_yuv);
		mHalMiscDumpToFile(szFileName, (UINT8 *)mpResultImgBuf->getVirtAddr(), mu4ResultImgSize);	// *2: YUV422 size.
		MY_VERB("[do_CroppedResultResize] Save %s done.", szFileName);
	}
	#endif	// HDR_DEBUG_SAVE_RESIZE_HDR_RESULT

	MY_DBG("[do_CroppedResultResize] - X. ret: %d.", ret);

	return	ret;

}


///////////////////////////////////////////////////////////////////////////
/// @brief Do HDR setting clear.
///
/// @return SUCCDSS (TRUE) or Fail (FALSE).
///////////////////////////////////////////////////////////////////////////
MBOOL
Hdr::
do_HdrSettingClear(void)
{
	MY_DBG("[do_HdrSettingClear] - E.");
	MBOOL  ret = MTRUE;

#if (HDR_PROFILE_CAPTURE)
	MyDbgTimer DbgTmr("do_HdrSettingClear");
#endif


#if 0
	mpHdrHal->SaveHdrLog(mu4RunningNumber);
#endif

	// Clear HDR Setting.
	mpHdrHal->HdrSettingClear();

#if (HDR_PROFILE_CAPTURE)
	DbgTmr.print("HdrProfiling:: do_HdrSettingClear Time");
#endif

	MY_DBG("[do_HdrSettingClear] - X. ret: %d.", ret);

	return	ret;

}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Hdr::
saveSourceJpg(void)
{
    MY_DBG("[saveSourceJpg] - E.");

    MBOOL ret = MTRUE;
	MINT32 err = 0;
    MUINT32 u4ThumbSize = 0;
    MUINT32 u4JpgPictureSize = 0;
//	char szFileName[100];
//	::sprintf(szFileName, HDR_DEBUG_OUTPUT_FOLDER "%04d_10_Source.jpg", mu4RunningNumber);

    if (mShotParam.u1FileName)  // Only save file when file name is not NULL.
    {
        ret =
    			//	Process thumbnail.
    			makeQuickViewThumbnail(u4ThumbSize)
    			//	Flatten Jpeg Picture Buffer for callback (including Exif and thumbnail).
    		&&	flattenJpgPicBuf(getJpgEncDoneSize(), u4ThumbSize, u4JpgPictureSize)
    			//	Save JPG.
            &&  invokeCallback(MHAL_CAM_CB_HDR_ORI_DATA, (MUINT8*)(mShotParam.frmJpg.virtAddr), u4JpgPictureSize)
//    		&&	( 0 == (err = mHalMiscDumpToFile((char*)mShotParam.uShotFileName, (MUINT8*)(mShotParam.frmJpg.virtAddr), u4JpgPictureSize)) )
//  		&&	( 0 == (err = mHalMiscDumpToFile(szFileName, (MUINT8*)(mShotParam.frmJpg.virtAddr), u4JpgPictureSize)) )
    			;
        
    }

	MY_DBG("[saveSourceJpg] FileName: %s. virtAddr: 0x%08X. u4ThumbSize: %d. u4JpgPictureSize: %d.", mShotParam.uShotFileName, mShotParam.frmJpg.virtAddr, u4ThumbSize, u4JpgPictureSize);

    MY_DBG("[saveSourceJpg] - X. ret: %d.", ret);

    return  ret;

}

