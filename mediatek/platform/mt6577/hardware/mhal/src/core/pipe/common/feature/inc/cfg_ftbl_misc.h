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
#ifndef _CONFIG_FTBL_MISC_H_
#define _CONFIG_FTBL_MISC_H_


namespace NSFeature
{
namespace NSMisc    
{


namespace NSSceneIndep  
{
GETFINFO_SCENE_INDEP()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //..........................................................................
    //  EIS
    CONFIG_FEATURE_SI(FID_EIS, 
        BY_DEFAULT(EIS_OFF), 
        EIS_OFF, EIS_ON
    )
    //..........................................................................
    //	ZSD
    CONFIG_FEATURE_SI(FID_ZSD, 
        BY_DEFAULT(ZSD_OFF), 
        ZSD_OFF, ZSD_ON
    )
    
	//	Continuous Shot Speed
	CONFIG_FEATURE_SI(FID_FAST_CONTINUOUS_SHOT, 
		BY_DEFAULT(FCS_OFF), 
		FCS_OFF, FCS_ON
	)

    //	AWB2PASS
    CONFIG_FEATURE_SI(FID_AWB2PASS, 
        BY_DEFAULT(AWB2PASS_ON), 
        AWB2PASS_OFF, AWB2PASS_ON
    )
//------------------------------------------------------------------------------
END_GETFINFO_SCENE_INDEP()
};  //  namespace NSSceneIndep


namespace NSSceneDep
{
GETFINFO_SCENE_DEP()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Scene Auto
    CONFIG_SCENE(SCENE_MODE_OFF)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_OFF), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Normal
    CONFIG_SCENE(SCENE_MODE_NORMAL)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF,
            BY_DEFAULT(FD_OFF),
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Portrait
    CONFIG_SCENE(SCENE_MODE_PORTRAIT)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_ON), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Landscape
    CONFIG_SCENE(SCENE_MODE_LANDSCAPE)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_OFF), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Night
    CONFIG_SCENE(SCENE_MODE_NIGHTSCENE)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_OFF), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Night Portrait
    CONFIG_SCENE(SCENE_MODE_NIGHTPORTRAIT)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_ON), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Theatre
    CONFIG_SCENE(SCENE_MODE_THEATRE)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_OFF), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Beach
    CONFIG_SCENE(SCENE_MODE_BEACH)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_OFF), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Snow
    CONFIG_SCENE(SCENE_MODE_SNOW)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_OFF), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Sunset
    CONFIG_SCENE(SCENE_MODE_SUNSET)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_OFF), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Steady photo
    CONFIG_SCENE(SCENE_MODE_STEADYPHOTO)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_OFF), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Fireworks
    CONFIG_SCENE(SCENE_MODE_FIREWORKS)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_OFF), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Sports
    CONFIG_SCENE(SCENE_MODE_SPORTS)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_OFF), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Party
    CONFIG_SCENE(SCENE_MODE_PARTY)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_OFF), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Candle light
    CONFIG_SCENE(SCENE_MODE_CANDLELIGHT)
        //......................................................................
        //  FD Mode
        CONFIG_FEATURE_SD(FID_FD_ON_OFF, 
            BY_DEFAULT(FD_OFF), 
            FD_OFF, FD_ON
        )
        //......................................................................
    END_CONFIG_SCENE()
//------------------------------------------------------------------------------
END_GETFINFO_SCENE_DEP()
};  //  namespace NSSceneDep


};  //  namespace NSMisc
};  //  namespace NSFeature


#endif // _CONFIG_FTBL_MISC_H_

