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
#ifndef _SHOT_PROFILE_H_
#define _SHOT_PROFILE_H_


#include <mcam_profile.h>


/*******************************************************************************
*
********************************************************************************/
class ShotProfile
{
protected:
    char const*const    mpszTileName;
    char const*const    mpszShotName;
    mutable MINT32      mIdx;
    MINT32 const        mi4StartUs;
    mutable MINT32      mi4LastUs;
    MBOOL               mfgIsProfile;

public:
    ShotProfile(char const*const pszTitle, char const*const pszShotName = "")
        : mpszTileName(pszTitle)
        , mpszShotName(pszShotName)
        , mIdx(0)
        , mi4StartUs(getUs())
        , mi4LastUs(getUs())
        , mfgIsProfile(MFALSE)
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        ::property_get("debug.camera.profile", value, "0xFF");
        MINT32 i4ProfileOPT = 0;
        ::sscanf(value, "%x", &i4ProfileOPT);
        mfgIsProfile = 0 != ( i4ProfileOPT & MHAL_CAM_CAP_PROFILE_OPT );
    }

    inline MINT32 getUs() const
    {
        struct timeval tv;
        ::gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    inline MBOOL print(char const*const pszInfo = "") const
    {
        if  ( mfgIsProfile )
        {
            MINT32 const i4EndUs = getUs();
            if  (0==mIdx)
            {
                CAM_LOGI("{%s}[%s]%s:(%d-th) ===> [start-->now: %d ms]", mpszShotName, mpszTileName, pszInfo, mIdx++, (i4EndUs-mi4StartUs)/1000);
            }
            else
            {
                CAM_LOGI("{%s}[%s]%s:(%d-th) ===> [start-->now: %d ms] [last-->now: %d ms]", mpszShotName, mpszTileName, pszInfo, mIdx++, (i4EndUs-mi4StartUs)/1000, (i4EndUs-mi4LastUs)/1000);
            }
            mi4LastUs = i4EndUs;
        }
        return  MTRUE;
    }
};


#endif  //  _SHOT_PROFILE_H_

