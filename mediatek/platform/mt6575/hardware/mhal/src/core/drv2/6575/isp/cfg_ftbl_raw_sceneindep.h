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
#ifndef _CONFIG_FTBL_RAW_SCENE_INDEP_H_
#define _CONFIG_FTBL_RAW_SCENE_INDEP_H_


namespace NSFeature     {
namespace NSRAW         {
namespace NSSceneIndep  {
GETFINFO_SCENE_INDEP()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //..........................................................................
    //  Scene Mode
    CONFIG_FEATURE_SI(FID_SCENE_MODE, 
        BY_DEFAULT(SCENE_MODE_OFF), 
        SCENE_MODE_OFF,       SCENE_MODE_NORMAL,      SCENE_MODE_PORTRAIT, 
        SCENE_MODE_LANDSCAPE, SCENE_MODE_NIGHTSCENE,  SCENE_MODE_NIGHTPORTRAIT, 
        SCENE_MODE_THEATRE,   SCENE_MODE_BEACH,       SCENE_MODE_SNOW, 
        SCENE_MODE_SUNSET,    SCENE_MODE_STEADYPHOTO, SCENE_MODE_FIREWORKS, 
        SCENE_MODE_SPORTS,    SCENE_MODE_PARTY,       SCENE_MODE_CANDLELIGHT, 
        SCENE_MODE_HDR, 
    )
    //..........................................................................
    //  Effect
    CONFIG_FEATURE_SI(FID_COLOR_EFFECT, 
        BY_DEFAULT(MEFFECT_OFF), 
        MEFFECT_OFF,        MEFFECT_MONO,      MEFFECT_SEPIA, 
        MEFFECT_NEGATIVE,   MEFFECT_AQUA, 
        MEFFECT_BLACKBOARD, MEFFECT_WHITEBOARD
    )
    //..........................................................................
    //  Capture Mode
    CONFIG_FEATURE_SI(FID_CAPTURE_MODE, 
        BY_DEFAULT(CAPTURE_MODE_NORMAL), 
        CAPTURE_MODE_NORMAL,     CAPTURE_MODE_CONTINUOUS_SHOT, 
        CAPTURE_MODE_SMILE_SHOT,  
        CAPTURE_MODE_BEST_SHOT,  CAPTURE_MODE_EV_BRACKET, 
        CAPTURE_MODE_MAV,        
        CAPTURE_MODE_AUTORAMA,   CAPTURE_MODE_ASD,
        CAPTURE_MODE_PANO_3D,    CAPTURE_MODE_SINGLE_3D,
        CAPTURE_MODE_FACE_BEAUTY
    )
    //..........................................................................
    //  Capture Size
    CONFIG_FEATURE_SI(FID_CAP_SIZE, 
        BY_DEFAULT(CAPTURE_SIZE_2560_1920),
        CAPTURE_SIZE_1280_960,  CAPTURE_SIZE_1600_1200, 
        CAPTURE_SIZE_2048_1536, CAPTURE_SIZE_2560_1920,
        CAPTURE_SIZE_3264_2448
    )
    //..........................................................................
    //  Preview Size
    CONFIG_FEATURE_SI(FID_PREVIEW_SIZE, 
        BY_DEFAULT(PREVIEW_SIZE_640_480), 
        PREVIEW_SIZE_176_144, PREVIEW_SIZE_320_240,
        PREVIEW_SIZE_352_288, PREVIEW_SIZE_480_320,
        PREVIEW_SIZE_480_368, PREVIEW_SIZE_640_480,
        PREVIEW_SIZE_720_480, PREVIEW_SIZE_800_480, 
        PREVIEW_SIZE_864_480, PREVIEW_SIZE_1280_720,
        PREVIEW_SIZE_1920_1080
    )
    //..........................................................................
    //  Video Preview Size
    CONFIG_FEATURE_SI(FID_VIDEO_PREVIEW_SIZE, 
        BY_DEFAULT(VIDEO_PREVIEW_SIZE_640_480), 
        VIDEO_PREVIEW_SIZE_640_480, 
        VIDEO_PREVIEW_SIZE_800_600
    )    
    //..........................................................................
    //  Frame Rate
    CONFIG_FEATURE_SI(FID_FRAME_RATE, 
        BY_DEFAULT(FRAME_RATE_300FPS), 
        FRAME_RATE_150FPS, FRAME_RATE_240FPS, FRAME_RATE_300FPS
    )
    //..........................................................................
    //  Frame Rate Range
    CONFIG_FEATURE_SI(FID_FRAME_RATE_RANGE, 
        BY_DEFAULT(FRAME_RATE_RANGE_5_30_FPS), 
        FRAME_RATE_RANGE_5_30_FPS
    )
    //..........................................................................
    //  Focus Distance Normal
    CONFIG_FEATURE_SI(FID_FOCUS_DIST_NORMAL, 
        BY_DEFAULT(FOCUS_DIST_N_10CM), 
        FOCUS_DIST_N_10CM
    )
    //..........................................................................
    //  Focus Distance Macro
    CONFIG_FEATURE_SI(FID_FOCUS_DIST_MACRO, 
        BY_DEFAULT(FOCUS_DIST_M_5CM), 
        FOCUS_DIST_M_5CM
    )
    //..........................................................................
    //  AE Flicker
    CONFIG_FEATURE_SI(FID_AE_FLICKER, 
        BY_DEFAULT(AE_FLICKER_MODE_AUTO), 
        AE_FLICKER_MODE_60HZ, AE_FLICKER_MODE_50HZ, 
        AE_FLICKER_MODE_AUTO, AE_FLICKER_MODE_OFF
    )
    //..........................................................................
    //  Stereo 3D Capture Size
    CONFIG_FEATURE_SI(FID_STEREO_3D_CAP_SIZE, 
        BY_DEFAULT(STEREO_3D_CAPTURE_SIZE_2560_720),
        STEREO_3D_CAPTURE_SIZE_2560_720
    )
    //..........................................................................
    //  Stereo 3D Preview Size
    CONFIG_FEATURE_SI(FID_STEREO_3D_PREVIEW_SIZE, 
        BY_DEFAULT(STEREO_3D_PREVIEW_SIZE_640_360), 
        STEREO_3D_PREVIEW_SIZE_640_360, STEREO_3D_PREVIEW_SIZE_854_480,
        STEREO_3D_PREVIEW_SIZE_960_540, STEREO_3D_PREVIEW_SIZE_1280_720
    )    
    //..........................................................................
    //  Stereo 3D mode
    CONFIG_FEATURE_SI(FID_STEREO_3D_MODE, 
        BY_DEFAULT(STEREO_3D_OFF), 
        STEREO_3D_OFF, STEREO_3D_ON
    )    
    //..........................................................................
    //  Stereo 3D image format
    CONFIG_FEATURE_SI(FID_STEREO_3D_IMAGE_FORMAT, 
        BY_DEFAULT(STEREO_3D_JPS), 
        STEREO_3D_JPS, STEREO_3D_MPO
    ) 
//------------------------------------------------------------------------------
END_GETFINFO_SCENE_INDEP()
};  //  namespace NSSceneIndep
};  //  namespace NSRAW
};  //  namespace NSFeature


#endif // _CONFIG_FTBL_RAW_SCENE_INDEP_H_

