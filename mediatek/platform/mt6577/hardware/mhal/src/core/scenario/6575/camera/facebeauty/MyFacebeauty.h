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
#ifndef _MY_FB_H_
#define _MY_FB_H_

/*******************************************************************************
*
*******************************************************************************/
#define LOG_TAG "mHalCam"
#include <utils/threads.h>
#include <utils/Errors.h>
#include <cutils/xlog.h>
//
#include <MediaHal.h>
#include <scenario_types.h>
#include <cam_types.h>
#include <cam_port.h>
using namespace NSCamera;
//
#include <mhal/inc/camera.h>
#include <shot/IShot.h>
#include <shot/Shotbase.h>
#include "IFacebeauty.h"
#include "Facebeauty.h"
#include <isp_hal.h>
#include <mdp_hal.h>
#include <aaa_hal_base.h>
#include <mhal_interface.h>

/*******************************************************************************
*
*******************************************************************************/
#define MY_DBG(fmt, arg...)     XLOGD("{FBShot}"fmt, ##arg)
#define MY_INFO(fmt, arg...)    XLOGI("{FBShot}"fmt, ##arg)
#define MY_WARN(fmt, arg...)    XLOGW("{FBShot}"fmt, ##arg)
#define MY_ERR(fmt, arg...)     XLOGE("{FBShot}<%s:#%d>"fmt, __FILE__, __LINE__, ##arg)

/*******************************************************************************
*
*******************************************************************************/
//#define FB_PROFILE_CAPTURE 1
#if (FB_PROFILE_CAPTURE)
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
                MY_INFO("[%s]%s:(%d-th) ===> [start-->now: %d ms]", mpszName, pszInfo, mIdx++, (i4EndUs-mi4StartUs)/1000);
            }
            else
            {
                MY_INFO("[%s]%s:(%d-th) ===> [start-->now: %d ms] [last-->now: %d ms]", mpszName, pszInfo, mIdx++, (i4EndUs-mi4StartUs)/1000, (i4EndUs-mi4LastUs)/1000);
            }
            mi4LastUs = i4EndUs;
            return  MTRUE;
        }
    };
#endif  //FB_PROFILE_CAPTURE


/*******************************************************************************
*
*******************************************************************************/
#endif  //  _MY_HDR_H_

