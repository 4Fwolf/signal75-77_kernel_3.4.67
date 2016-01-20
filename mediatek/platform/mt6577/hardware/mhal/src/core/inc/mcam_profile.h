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
** $Log: mcam_profile.h $
 *
*/

/*******************************************************************************
*
********************************************************************************/
#ifndef _MCAM_PROFILE_H_
#define _MCAM_PROFILE_H_

#include <cutils/properties.h>
#include "mcam_log.h"

/*******************************************************************************
*
********************************************************************************/
#define MHAL_CAM_PRV_PROFILE_OPT         (0x1)       // Preview profiling On/Off
#define MHAL_CAM_GENERIC_PROFILE_OPT  (0x2)
#define MHAL_CAM_CAP_PROFILE_OPT         (0x4)       // Capture profiling On/Off
#define MHAL_CAM_FD_PROFILE_OPT           (0x8)       // FD profiling On/Off

/*******************************************************************************
*
********************************************************************************/
class CamTimer {
public:
     //
    inline MINT32 getUsTime()  
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    //
    CamTimer(const char* info, const int type)
        : mInfo(info), mIdx(0), mType(type)
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'}; 
        property_get("debug.camera.profile", value, "0x1E");
        sscanf(value, "%x", &mProfileOPT); 
        if (mType & mProfileOPT) {
            mStartTime = getUsTime();
        }
    }
    
    //
    inline MVOID printTime()
    {
        if (mType & mProfileOPT) {
            MINT32 endTime = getUsTime();
            MCAM_DBG("[%s, %d time] =====> %d ms \n", mInfo, mIdx++, (endTime - mStartTime) / 1000);
        }
    }

    //
    inline MVOID printTime(const char* tag) 
    {
        if (mType & mProfileOPT) {
            MINT32 endTime = getUsTime(); 
            MCAM_DBG("[%s, %s time] =====> %d ms \n", mInfo, tag, (endTime - mStartTime) / 1000);
        }
    }

    //
    ~CamTimer()
    {
        mProfileOPT = 0;
    }

protected:
    const char* mInfo;
    MINT32 mStartTime;
    MINT32 mIdx;
    MINT32 mType; 
    MINT32 mProfileOPT; 
};

#endif 
