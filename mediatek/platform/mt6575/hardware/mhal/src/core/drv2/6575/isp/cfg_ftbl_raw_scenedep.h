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
#ifndef _CONFIG_FTBL_RAW_SCENE_DEP_H_
#define _CONFIG_FTBL_RAW_SCENE_DEP_H_


#define AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW() \
    AE_METERING_MODE_CENTER_WEIGHT, AE_METERING_MODE_SOPT, \
    AE_METERING_MODE_AVERAGE

#define AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW() \
    AE_EV_COMP_00, AE_EV_COMP_10, AE_EV_COMP_20, \
    AE_EV_COMP_30, AE_EV_COMP_n10, \
    AE_EV_COMP_n20, AE_EV_COMP_n30

#define AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() \
    AF_MODE_AFS, AF_MODE_AFC, AF_MODE_AFC_VIDEO, AF_MODE_MACRO, \
    AF_MODE_INFINITY, AF_MODE_MF, AF_MODE_FULLSCAN

#define AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW() \
    AF_METER_SPOT, AF_METER_MATRIX, AF_METER_MOVESPOT

#define AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() \
    AWB_MODE_AUTO, AWB_MODE_DAYLIGHT, AWB_MODE_CLOUDY_DAYLIGHT, \
    AWB_MODE_SHADE, AWB_MODE_TWILIGHT, AWB_MODE_FLUORESCENT, \
    AWB_MODE_WARM_FLUORESCENT, AWB_MODE_INCANDESCENT

#define ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW() \
    ISP_HUE_LOW, ISP_HUE_MIDDLE, ISP_HUE_HIGH

#define ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW() \
    ISP_BRIGHT_LOW, ISP_BRIGHT_MIDDLE, ISP_BRIGHT_HIGH


namespace NSFeature     {
namespace NSRAW         {
namespace NSSceneDep    {
GETFINFO_SCENE_DEP()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Scene Auto
    CONFIG_SCENE(SCENE_MODE_OFF)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_AUTO), 
            AE_MODE_OFF,   AE_MODE_AUTO,        AE_MODE_PROGRAM, 
            AE_MODE_TV,    AE_MODE_AV,          AE_MODE_SV, 
            AE_MODE_VIDEO, AE_MODE_NIGHT,       AE_MODE_ACTION, 
            AE_MODE_BEACH, AE_MODE_CANDLELIGHT, AE_MODE_FIREWORKS, 
            /*AE_MODE_LANDSCAPE, */
            AE_MODE_PORTRAIT, AE_MODE_NIGHT_PORTRAIT, AE_MODE_PARTY, 
            AE_MODE_SNOW,     AE_MODE_SPORTS,   AE_MODE_STEADYPHOTO, 
            AE_MODE_SUNSET,   AE_MODE_THEATRE,  AE_MODE_ISO_ANTI_SHAKE, 
            AE_MODE_BRACKET_AE
        )
        //......................................................................
        //  AE Meter
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO, AE_ISO_100, AE_ISO_200, 
            AE_ISO_400,  AE_ISO_800, AE_ISO_1600
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_00), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_AFS), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_AUTO), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_MIDDLE), 
            ISP_EDGE_LOW, ISP_EDGE_MIDDLE, ISP_EDGE_HIGH
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), 
            ISP_SAT_LOW, ISP_SAT_MIDDLE, ISP_SAT_HIGH
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_LOW, ISP_CONTRAST_MIDDLE, ISP_CONTRAST_HIGH
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene HDR
    CONFIG_SCENE(SCENE_MODE_HDR)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_AUTO), 
            AE_MODE_OFF,   AE_MODE_AUTO,        AE_MODE_PROGRAM, 
            AE_MODE_TV,    AE_MODE_AV,          AE_MODE_SV, 
            AE_MODE_VIDEO, AE_MODE_NIGHT,       AE_MODE_ACTION, 
            AE_MODE_BEACH, AE_MODE_CANDLELIGHT, AE_MODE_FIREWORKS, 
            /*AE_MODE_LANDSCAPE, */
            AE_MODE_PORTRAIT, AE_MODE_NIGHT_PORTRAIT, AE_MODE_PARTY, 
            AE_MODE_SNOW,     AE_MODE_SPORTS,   AE_MODE_STEADYPHOTO, 
            AE_MODE_SUNSET,   AE_MODE_THEATRE,  AE_MODE_ISO_ANTI_SHAKE, 
            AE_MODE_BRACKET_AE
        )
        //......................................................................
        //  AE Meter
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO, AE_ISO_100, AE_ISO_200, 
            AE_ISO_400,  AE_ISO_800, AE_ISO_1600
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_00), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_AFS), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_AUTO), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_MIDDLE), 
            ISP_EDGE_LOW, ISP_EDGE_MIDDLE, ISP_EDGE_HIGH
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), 
            ISP_SAT_LOW, ISP_SAT_MIDDLE, ISP_SAT_HIGH
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_LOW, ISP_CONTRAST_MIDDLE, ISP_CONTRAST_HIGH
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Normal
    CONFIG_SCENE(SCENE_MODE_NORMAL)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE,
            BY_DEFAULT(AE_MODE_AUTO),
            AE_MODE_OFF,   AE_MODE_AUTO,        AE_MODE_PROGRAM,
            AE_MODE_TV,    AE_MODE_AV,          AE_MODE_SV,
            AE_MODE_VIDEO, AE_MODE_NIGHT,       AE_MODE_ACTION,
            AE_MODE_BEACH, AE_MODE_CANDLELIGHT, AE_MODE_FIREWORKS,
            /*AE_MODE_LANDSCAPE, */
            AE_MODE_PORTRAIT, AE_MODE_NIGHT_PORTRAIT, AE_MODE_PARTY,
            AE_MODE_SNOW,     AE_MODE_SPORTS,   AE_MODE_STEADYPHOTO,
            AE_MODE_SUNSET,   AE_MODE_THEATRE,  AE_MODE_ISO_ANTI_SHAKE,
            AE_MODE_BRACKET_AE
        )
        //......................................................................
        //  AE Meter
        CONFIG_FEATURE_SD(FID_AE_METERING,
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT),
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO,
            BY_DEFAULT(AE_ISO_AUTO),
            AE_ISO_AUTO, AE_ISO_100, AE_ISO_200,
            AE_ISO_400,  AE_ISO_800, AE_ISO_1600
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV,
            BY_DEFAULT(AE_EV_COMP_00),
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE,
            BY_DEFAULT(AF_MODE_AFC),
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING,
            BY_DEFAULT(AF_METER_SPOT),
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE,
            BY_DEFAULT(AWB_MODE_AUTO),
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE,
            BY_DEFAULT(ISP_EDGE_MIDDLE),
            ISP_EDGE_LOW, ISP_EDGE_MIDDLE, ISP_EDGE_HIGH
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE,
            BY_DEFAULT(ISP_HUE_MIDDLE),
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT,
            BY_DEFAULT(ISP_SAT_MIDDLE),
            ISP_SAT_LOW, ISP_SAT_MIDDLE, ISP_SAT_HIGH
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT,
            BY_DEFAULT(ISP_BRIGHT_MIDDLE),
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST,
            BY_DEFAULT(ISP_CONTRAST_MIDDLE),
            ISP_CONTRAST_LOW, ISP_CONTRAST_MIDDLE, ISP_CONTRAST_HIGH
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Portrait
    CONFIG_SCENE(SCENE_MODE_PORTRAIT)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_PORTRAIT), 
            AE_MODE_PORTRAIT
        )
        //......................................................................
        //  AE Meter (the same as Auto Scene)
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_00), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_AFC), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_AUTO), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_LOW), 
            ISP_EDGE_LOW
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), 
            ISP_SAT_MIDDLE
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_MIDDLE
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Landscape
    CONFIG_SCENE(SCENE_MODE_LANDSCAPE)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_LANDSCAPE), 
            AE_MODE_LANDSCAPE
        )
        //......................................................................
        //  AE Meter (the same as Auto Scene)
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_00), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_INFINITY), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_DAYLIGHT), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_HIGH), 
            ISP_EDGE_HIGH
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), 
            ISP_SAT_MIDDLE
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_MIDDLE
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Night
    CONFIG_SCENE(SCENE_MODE_NIGHTSCENE)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_NIGHT), 
            AE_MODE_NIGHT
        )
        //......................................................................
        //  AE Meter (the same as Auto Scene)
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_00), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_AFC), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_AUTO), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_LOW), 
            ISP_EDGE_LOW
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), ISP_SAT_MIDDLE
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_MIDDLE
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Night Portrait
    CONFIG_SCENE(SCENE_MODE_NIGHTPORTRAIT)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_NIGHT_PORTRAIT), 
            AE_MODE_NIGHT_PORTRAIT
        )
        //......................................................................
        //  AE Meter (the same as Auto Scene)
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_00), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_AFC), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_AUTO), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_LOW), 
            ISP_EDGE_LOW
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_LOW), 
            ISP_SAT_LOW
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_MIDDLE
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Theatre
    CONFIG_SCENE(SCENE_MODE_THEATRE)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_THEATRE), 
            AE_MODE_THEATRE
        )
        //......................................................................
        //  AE Meter (the same as Auto Scene)
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_00), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_AFC), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_AUTO), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_HIGH), 
            ISP_EDGE_HIGH
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), 
            ISP_SAT_MIDDLE
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_MIDDLE
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Beach
    CONFIG_SCENE(SCENE_MODE_BEACH)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_BEACH), 
            AE_MODE_BEACH
        )
        //......................................................................
        //  AE Meter (the same as Auto Scene)
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_10), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_AFC), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_AUTO), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_HIGH), 
            ISP_EDGE_HIGH
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), 
            ISP_SAT_MIDDLE
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_MIDDLE
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Snow
    CONFIG_SCENE(SCENE_MODE_SNOW)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_SNOW), 
            AE_MODE_SNOW
        )
        //......................................................................
        //  AE Meter (the same as Auto Scene)
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_10), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_AFC), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_AUTO), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_HIGH), 
            ISP_EDGE_HIGH
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), 
            ISP_SAT_MIDDLE
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_MIDDLE
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Sunset
    CONFIG_SCENE(SCENE_MODE_SUNSET)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_SUNSET), 
            AE_MODE_SUNSET
        )
        //......................................................................
        //  AE Meter (the same as Auto Scene)
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_00), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_AFC), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_DAYLIGHT), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_HIGH), 
            ISP_EDGE_HIGH
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), 
            ISP_SAT_MIDDLE
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_MIDDLE
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Steady photo
    CONFIG_SCENE(SCENE_MODE_STEADYPHOTO)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_STEADYPHOTO), 
            AE_MODE_STEADYPHOTO
        )
        //......................................................................
        //  AE Meter (the same as Auto Scene)
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_00), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_AFC), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_AUTO), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_MIDDLE), 
            ISP_EDGE_MIDDLE
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), 
            ISP_SAT_MIDDLE
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_MIDDLE
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Fireworks
    CONFIG_SCENE(SCENE_MODE_FIREWORKS)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_FIREWORKS), 
            AE_MODE_FIREWORKS
        )
        //......................................................................
        //  AE Meter (the same as Auto Scene)
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_00), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_AFC), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_AUTO), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_MIDDLE), 
            ISP_EDGE_MIDDLE
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), 
            ISP_SAT_MIDDLE
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_MIDDLE
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Sports
    CONFIG_SCENE(SCENE_MODE_SPORTS)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_SPORTS), 
            AE_MODE_SPORTS
        )
        //......................................................................
        //  AE Meter (the same as Auto Scene)
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_00), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_INFINITY), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_AUTO), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_MIDDLE), 
            ISP_EDGE_MIDDLE
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), 
            ISP_SAT_MIDDLE
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_MIDDLE
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Party
    CONFIG_SCENE(SCENE_MODE_PARTY)
        //......................................................................

        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_PARTY), 
            AE_MODE_PARTY
        )
        //......................................................................
        //  AE Meter (the same as Auto Scene)
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_00), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_AFC), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_AUTO), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_MIDDLE), 
            ISP_EDGE_MIDDLE
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), 
            ISP_SAT_MIDDLE
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_MIDDLE
        )
        //......................................................................
    END_CONFIG_SCENE()
    //==========================================================================
    //  Scene Candle light
    CONFIG_SCENE(SCENE_MODE_CANDLELIGHT)
        //......................................................................
        //  AE Mode
        CONFIG_FEATURE_SD(FID_AE_SCENE_MODE, 
            BY_DEFAULT(AE_MODE_CANDLELIGHT), 
            AE_MODE_CANDLELIGHT
        )
        //......................................................................
        //  AE Meter (the same as Auto Scene)
        CONFIG_FEATURE_SD(FID_AE_METERING, 
            BY_DEFAULT(AE_METERING_MODE_CENTER_WEIGHT), 
            AE_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AE ISO
        CONFIG_FEATURE_SD(FID_AE_ISO, 
            BY_DEFAULT(AE_ISO_AUTO), 
            AE_ISO_AUTO
        )
        //......................................................................
        //  AE EV
        CONFIG_FEATURE_SD(FID_AE_EV, 
            BY_DEFAULT(AE_EV_COMP_00), 
            AE_EV_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AF Mode
        CONFIG_FEATURE_SD(FID_AF_MODE, 
            BY_DEFAULT(AF_MODE_AFC), 
            AF_MODE_DEFAULT_SUPPORT_LIST_BY_RAW() 
        )
        //......................................................................
        //  AF Meter
        CONFIG_FEATURE_SD(FID_AF_METERING, 
            BY_DEFAULT(AF_METER_SPOT), 
            AF_METER_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  AWB Mode
        CONFIG_FEATURE_SD(FID_AWB_MODE, 
            BY_DEFAULT(AWB_MODE_INCANDESCENT), 
            AWB_MODE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Edge
        CONFIG_FEATURE_SD(FID_ISP_EDGE, 
            BY_DEFAULT(ISP_EDGE_MIDDLE), 
            ISP_EDGE_MIDDLE
        )
        //......................................................................
        //  ISP Hue
        CONFIG_FEATURE_SD(FID_ISP_HUE, 
            BY_DEFAULT(ISP_HUE_MIDDLE), 
            ISP_HUE_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Saturation
        CONFIG_FEATURE_SD(FID_ISP_SAT, 
            BY_DEFAULT(ISP_SAT_MIDDLE), 
            ISP_SAT_MIDDLE
        )
        //......................................................................
        //  ISP Brightness
        CONFIG_FEATURE_SD(FID_ISP_BRIGHT, 
            BY_DEFAULT(ISP_BRIGHT_MIDDLE), 
            ISP_BRIGHTNESS_DEFAULT_SUPPORT_LIST_BY_RAW()
        )
        //......................................................................
        //  ISP Contrast
        CONFIG_FEATURE_SD(FID_ISP_CONTRAST, 
            BY_DEFAULT(ISP_CONTRAST_MIDDLE), 
            ISP_CONTRAST_MIDDLE
        )
        //......................................................................
    END_CONFIG_SCENE()
//------------------------------------------------------------------------------
END_GETFINFO_SCENE_DEP()
};  //  namespace NSSceneDep
};  //  namespace NSRAW
};  //  namespace NSFeature


#endif // _CONFIG_FTBL_RAW_SCENE_DEP_H_

