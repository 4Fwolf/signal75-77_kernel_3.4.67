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
#ifndef _CONFIG_FTBL_TARGET_H_
#define _CONFIG_FTBL_TARGET_H_


/*******************************************************************************
* MACRO Define: Scene Independent
*******************************************************************************/
#define GETFINFO_SCENE_INDEP()                                              \
    template <MUINT32 _RoleId>                                              \
    static                                                                  \
    FInfoIF*                                                                \
    GetFInfo(FID_T const fid)                                               \
    {                                                                       \
        enum { ECamRoleId = _RoleId   };                                    \
        MY_LOG_OBJ_ONCE("[Target][SI][GetFInfo](RoleId)=(%d)", ECamRoleId); \
        switch  (fid)                                                       \
        {

#define CONFIG_FEATURE_SI(_fid)                                             \
    case _fid:                                                              \
    {                                                                       \
        STATIC_CHECK(                                                       \
            Fid2Type<_fid>::Info::isSceneIndep,                             \
            _fid##__NOT_scene_indep                                         \
        );                                                                  \
        typedef Fid2Type<_fid> _FidTy;                                      \
        static                                                              \
        FInfo_Target<_fid, _FidTy::Num, _FidTy::Type>                       \
        FInfo;                                                              \
        return  &FInfo;                                                     \
    }

#define END_GETFINFO_SCENE_INDEP()                                          \
        default:                                                            \
            MY_WARN(                                                        \
                "[Target][SI][GetFInfo] unsupported FID:"                   \
                "(RoleId,fid)=(%d,%d)", ECamRoleId, fid                     \
            );                                                              \
            break;                                                          \
        }                                                                   \
        return  NULL;                                                       \
    }


/*******************************************************************************
* MACRO Define: Scene Dependent
*******************************************************************************/
#define GETFINFO_SCENE_DEP()                                                \
    template <MUINT32 _RoleId, MUINT32 _SceneNum>                           \
    static                                                                  \
    FInfoIF*                                                                \
    GetFInfo(FID_T const fid, SID_T const sid)                              \
    {                                                                       \
        enum { ECamRoleId = _RoleId   };                                    \
        enum { ESceneNum  = _SceneNum };                                    \
        MY_LOG_OBJ_ONCE("[Target][SD][GetFInfo](RoleId,SceneNum)=(%d,%d)"   \
            , ECamRoleId, ESceneNum                                         \
        );                                                                  \
        if  ( ESceneNum <= sid )                                            \
        {                                                                   \
            MY_WARN(                                                        \
                "[Target][SD][GetFInfo] unsupported SID:"                   \
                "(RoleId,sid)=(%d,%d)", ECamRoleId, sid                     \
            );                                                              \
            return  NULL;                                                   \
        }                                                                   \
        switch  (fid)                                                       \
        {

#define CONFIG_FEATURE_SD(_fid)                                             \
    case _fid:                                                              \
    {                                                                       \
        STATIC_CHECK(                                                       \
            Fid2Type<_fid>::Info::isSceneDep,                               \
            _fid##__NOT_scene_dep                                           \
        );                                                                  \
        typedef Fid2Type<_fid> _FidTy;                                      \
        static                                                              \
        FInfo_Target<_fid, _FidTy::Num, _FidTy::Type>                       \
        aFInfo[ESceneNum];                                                  \
        return  &aFInfo[sid];                                               \
    }

#define END_GETFINFO_SCENE_DEP()                                            \
        default:                                                            \
            MY_WARN(                                                        \
                "[Target][SD][GetFInfo] unsupported FID:"                   \
                "(RoleId,sid,fid)=(%d,%d,%d)", ECamRoleId, sid, fid         \
            );                                                              \
            break;                                                          \
        }                                                                   \
        return  NULL;                                                       \
    }



namespace NSFeature     
{
namespace NSTarget      
{


namespace NSSceneIndep  
{
GETFINFO_SCENE_INDEP()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //..........................................................................
    CONFIG_FEATURE_SI( FID_AE_STROBE );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_SCENE_MODE );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_COLOR_EFFECT );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_CAPTURE_MODE );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_CAP_SIZE );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_PREVIEW_SIZE );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_VIDEO_PREVIEW_SIZE );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_FRAME_RATE );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_FRAME_RATE_RANGE );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_FOCUS_DIST_NORMAL );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_FOCUS_DIST_MACRO );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_AE_FLICKER );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_EIS );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_ZSD );	
    //..........................................................................
    CONFIG_FEATURE_SI( FID_AWB2PASS );	
    //..........................................................................
    CONFIG_FEATURE_SI( FID_AF_LAMP );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_FAST_CONTINUOUS_SHOT );	
    //..........................................................................
    CONFIG_FEATURE_SI( FID_STEREO_3D_CAP_SIZE );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_STEREO_3D_PREVIEW_SIZE );	
    //..........................................................................    
    CONFIG_FEATURE_SI( FID_STEREO_3D_TYPE );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_STEREO_3D_MODE );
    //..........................................................................
    CONFIG_FEATURE_SI( FID_STEREO_3D_IMAGE_FORMAT );		
//------------------------------------------------------------------------------
END_GETFINFO_SCENE_INDEP()
};  //  namespace NSSceneIndep


namespace NSSceneDep  
{
GETFINFO_SCENE_DEP()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //..........................................................................
    CONFIG_FEATURE_SD( FID_FD_ON_OFF );
    //..........................................................................
    CONFIG_FEATURE_SD( FID_AE_SCENE_MODE );
    //..........................................................................
    CONFIG_FEATURE_SD( FID_AE_METERING );
    //..........................................................................
    CONFIG_FEATURE_SD( FID_AE_ISO );
    //..........................................................................
    CONFIG_FEATURE_SD( FID_AE_EV );
    //..........................................................................
    CONFIG_FEATURE_SD( FID_AF_MODE );
    //..........................................................................
    CONFIG_FEATURE_SD( FID_AF_METERING );
    //..........................................................................
    CONFIG_FEATURE_SD( FID_AWB_MODE );
    //..........................................................................
    CONFIG_FEATURE_SD( FID_ISP_EDGE );
    //..........................................................................
    CONFIG_FEATURE_SD( FID_ISP_HUE );
    //..........................................................................
    CONFIG_FEATURE_SD( FID_ISP_SAT );
    //..........................................................................
    CONFIG_FEATURE_SD( FID_ISP_BRIGHT );
    //..........................................................................
    CONFIG_FEATURE_SD( FID_ISP_CONTRAST );
    //..........................................................................
//------------------------------------------------------------------------------
END_GETFINFO_SCENE_DEP()
};  //  namespace NSSceneDep


};  //  namespace NSTarget
};  //  namespace NSFeature


#endif // _CONFIG_FTBL_TARGET_H_

