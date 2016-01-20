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
#ifndef _MY_HDR_H_
#define _MY_HDR_H_


/*******************************************************************************
*
*******************************************************************************/
//
//#define HDR_DEBUG_OUTPUT_FOLDER		"sdcard/Photo/"	// For ALPS.GB2.
#define HDR_DEBUG_OUTPUT_FOLDER		"sdcard/DCIM/Camera/"	// For ALPS.ICS.

//#define HDR_DEBUG_SAVE_SOURCE_IMAGE			(1)     // Obsolete. Use "adb shell setprop hdr.debug.saveraw 1" and "adb shell setprop hdr.debug.saveyuv 1" to save raw and yuv files (set these to 0 to disable save).
#define HDR_DEBUG_SAVE_SMALL_IMAGE				(0)
#define HDR_DEBUG_SAVE_NORMALIZED_SMALL_IMAGE	(0)
#define HDR_DEBUG_SAVE_SW_EIS_IMAGE				(0)
#define HDR_DEBUG_SAVE_WEIGHTING_MAP			(0)
#define HDR_DEBUG_SAVE_DOWNSCALED_WEIGHTING_MAP	(0)
#define HDR_DEBUG_SAVE_BLURRED_WEIGHTING_MAP	(0)
#define HDR_DEBUG_SAVE_HDR_RESULT				(0)
#define HDR_DEBUG_SAVE_RESIZE_HDR_RESULT		(0)


#define HDR_PROFILE_CAPTURE						(0)		// General.
#define HDR_PROFILE_CAPTURE2					(1)		// In capture().
#define HDR_PROFILE_CAPTURE3					(0)		// In createFullFrame().



/*******************************************************************************
*
*******************************************************************************/
#define LOG_TAG "HdrScenario"
//#include <utils/threads.h>
#include <utils/Errors.h>
#include <cutils/xlog.h>
//
#include <MediaHal.h>
#include <scenario_types.h>
#include <cam_types.h>
#include <cam_port.h>
using namespace NSCamera;
//
#include <mhal/inc/camera.h>	// For struct needed by IShot.h
#include <shot/IShot.h>
#include <shot/Shotbase.h>
#include <mdp_hal.h>
#include "IHdr.h"
#include "Hdr.h"
#include <isp_hal.h>
#include <aaa_hal_base.h>
#include <mhal_interface.h>     // For Mdp_BitbltSlice() (for HDR Rotate).


/*******************************************************************************
*
*******************************************************************************/
//#define MY_DBG(fmt, arg...)     LOGD("{HdrShot}"fmt, ##arg)
//#define MY_INFO(fmt, arg...)    LOGI("{HdrShot}"fmt, ##arg)
//#define MY_WARN(fmt, arg...)    LOGW("{HdrShot}"fmt, ##arg)
//#define MY_ERR(fmt, arg...)     LOGE("{HdrShot}<%s:#%d>"fmt, __FILE__, __LINE__, ##arg)

#define HDR_HAL_TAG				"{HdrShot} "

#define LOG_LEVEL_SILENT	8
#define LOG_LEVEL_ASSERT	7
#define LOG_LEVEL_ERROR		6
#define LOG_LEVEL_WARN		5
#define LOG_LEVEL_INFO		4
#define LOG_LEVEL_DEBUG		3
#define LOG_LEVEL_VERBOSE	2
#define HDR_SHOT_LOG_LEVEL	LOG_LEVEL_VERBOSE

#if (HDR_SHOT_LOG_LEVEL <= LOG_LEVEL_SILENT)		// 8
	#define MY_ASSERT(cond, fmt, arg...)
	#define MY_ERR(fmt, arg...)
	#define MY_WARN(fmt, arg...)
	#define MY_INFO(fmt, arg...)
	#define MY_DBG(fmt, arg...)
	#define MY_VERB(fmt, arg...)
#endif	// (HDR_LOG_LEVEL <= LOG_LEVEL_SILENT)

#if (HDR_SHOT_LOG_LEVEL <= LOG_LEVEL_ASSERT)		// 7
	#undef  MY_ASSERT
    #define MY_ASSERT(expr, fmt, arg...)                                            \
        do {                                                                \
            if (!(expr))                                                    \
                XLOGE("%s, Line%s: ASSERTION FAILED!: " fmt, __FUNCTION__, __LINE__, ##arg);   \
        } while (0)
#endif	// (HDR_LOG_LEVEL <= LOG_LEVEL_ASSERT)

#if (HDR_SHOT_LOG_LEVEL <= LOG_LEVEL_ERROR)		// 6
	#undef  MY_ERR
	#define MY_ERR(fmt, arg...)		XLOGE(HDR_HAL_TAG "[%s, line%04d] " fmt, __FILE__, __LINE__, ##arg)	// When MP, will only show log of this level. // <Fatal>: Serious error that cause program can not execute. <Error>: Some error that causes some part of the functionality can not operate normally.
#endif	// (HDR_LOG_LEVEL <= LOG_LEVEL_ERROR)

#if (HDR_SHOT_LOG_LEVEL <= LOG_LEVEL_WARN)		// 5
	#undef  MY_WARN
	#define MY_WARN(fmt, arg...)	XLOGW(HDR_HAL_TAG fmt, ##arg)	// <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
#endif	// (HDR_LOG_LEVEL <= LOG_LEVEL_WARN)

#if (HDR_SHOT_LOG_LEVEL <= LOG_LEVEL_INFO)		// 4
	#undef  MY_INFO
	#define MY_INFO(fmt, arg...)	XLOGI(HDR_HAL_TAG fmt, ##arg)	// <Info>: Show general system information. Like OS version, start/end of Service...
#endif	// (HDR_LOG_LEVEL <= LOG_LEVEL_INFO)

#if (HDR_SHOT_LOG_LEVEL <= LOG_LEVEL_DEBUG)		// 3
	#undef  MY_DBG
	#define MY_DBG(fmt, arg...)		XLOGD(HDR_HAL_TAG fmt, ##arg)	// <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...
#endif	// (HDR_LOG_LEVEL <= LOG_LEVEL_DEBUG)

#if (HDR_SHOT_LOG_LEVEL <= LOG_LEVEL_VERBOSE)	// 2
	#undef  MY_VERB
	#define MY_VERB(fmt, arg...)	XLOGD(HDR_HAL_TAG fmt, ##arg)	// <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
#endif	// (HDR_LOG_LEVEL <= LOG_LEVEL_VERBOSE)




/*******************************************************************************
*
*******************************************************************************/
#if (HDR_PROFILE_CAPTURE || HDR_PROFILE_CAPTURE2 || HDR_PROFILE_CAPTURE3)
    class MyDbgTimer
    {
    protected:
        char const*const    mpszName;
        mutable MINT32      mIdx;
        MINT32 const        mi4StartUs;
        mutable MINT32      mi4LastUs;

    public:
        MyDbgTimer(char const*const pszTitle)
            : mpszName(pszTitle)
            , mIdx(0)
            , mi4StartUs(getUs())
            , mi4LastUs(getUs())
        {
        }

        inline MINT32 getUs() const
        {
            struct timeval tv;
            ::gettimeofday(&tv, NULL);
            return tv.tv_sec * 1000000 + tv.tv_usec;
        }

        inline MBOOL print(char const*const pszInfo = "") const
        {
            MINT32 const i4EndUs = getUs();
            if  (0==mIdx)
            {
                MY_INFO("[%s] %s:(%d-th) ===> [start-->now: %d ms]", mpszName, pszInfo, mIdx++, (i4EndUs-mi4StartUs)/1000);
            }
            else
            {
                MY_INFO("[%s] %s:(%d-th) ===> [start-->now: %d ms] [last-->now: %d ms]", mpszName, pszInfo, mIdx++, (i4EndUs-mi4StartUs)/1000, (i4EndUs-mi4LastUs)/1000);
            }
            mi4LastUs = i4EndUs;

		    //sleep(4); //wait 1 sec for AE stable

            return  MTRUE;
        }
    };
#endif  // (HDR_PROFILE_CAPTURE || HDR_PROFILE_CAPTURE2)


/*******************************************************************************
*
*******************************************************************************/
#endif  //  _MY_HDR_H_

