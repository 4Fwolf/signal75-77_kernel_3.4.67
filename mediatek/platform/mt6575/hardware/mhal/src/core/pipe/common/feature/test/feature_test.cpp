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
#define MY_BUILD    0
#if MY_BUILD
#define LOG_TAG "feature_test"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG           (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "debug.h"
//
#include "camera_feature.h"
#include "feature_hal.h"
//#include "feature_hal_bridge.h"
//#include "feature_hal_template.h"


using namespace NSFeature;


struct test_FeatureId
{
    test_FeatureId()
    {
        struct FID
        {
            enum
            {
                FID_AE_STROBE    =   1, 
                FID_EIS, 
                FID_SCENE_MODE, 
                FID_COLOR_EFFECT, 
                FID_CAPTURE_MODE, 
                FID_CAP_SIZE, 
                FID_PREVIEW_SIZE, 
                FID_FRAME_RATE, 
                FID_AE_FLICKER, 
                FID_FD_ON_OFF, 
                FID_AE_SCENE_MODE, 
                FID_AE_METERING, 
                FID_AE_ISO, 
                FID_AE_EV, 
                FID_AF_MODE, 
                FID_AF_METERING, 
                FID_AWB_MODE, 
                FID_ISP_EDGE, 
                FID_ISP_HUE, 
                FID_ISP_SAT, 
                FID_ISP_BRIGHT, 
                FID_ISP_CONTRAST, 
            };
        };
        #define CHECK_FID(_fid)  STATIC_CHECK((_fid==(MUINT32)FID::_fid), _fid##_NOT_EQUAL)
    
        CHECK_FID(FID_AE_STROBE);
        CHECK_FID(FID_EIS);
        CHECK_FID(FID_SCENE_MODE);
        CHECK_FID(FID_COLOR_EFFECT);
        CHECK_FID(FID_CAPTURE_MODE);
        CHECK_FID(FID_CAP_SIZE);
        CHECK_FID(FID_PREVIEW_SIZE);
        CHECK_FID(FID_FRAME_RATE);
        CHECK_FID(FID_AE_FLICKER);
        CHECK_FID(FID_FD_ON_OFF);
        CHECK_FID(FID_AE_SCENE_MODE);
        CHECK_FID(FID_AE_METERING);
        CHECK_FID(FID_AE_ISO);
        CHECK_FID(FID_AE_EV);
        CHECK_FID(FID_AF_MODE);
        CHECK_FID(FID_AF_METERING);
        CHECK_FID(FID_AWB_MODE);
        CHECK_FID(FID_ISP_EDGE);
        CHECK_FID(FID_ISP_HUE);
        CHECK_FID(FID_ISP_SAT);
        CHECK_FID(FID_ISP_BRIGHT);
        CHECK_FID(FID_ISP_CONTRAST);
    }
};


struct test_FeatureIdRange
{
    test_FeatureIdRange()
    {
        struct FID_RANGE
        {
            enum
            {
            FID_PRE_BEGIN = 0, 
              //////////////////////////////////////////////////////////////////////////////
              // Scene-Independent (SI) feature id.
              FID_PRE_BEGIN_SI = 0, 
              //----------------------------------------------------------------------------
                //..........................................................................
                //Misc. feature id.
                FID_PRE_BEGIN_MISC_SI       = 0, 
    //                FID_AE_STROBE         = 1, 
    //                FID_EIS               = 2, 
                FID_OVER_LAST_MISC_SI       = 3, 
                //..........................................................................
                //RAW-only feature id.
                FID_PRE_BEGIN_RAW_ONLY_SI   = 2, 
                FID_OVER_LAST_RAW_ONLY_SI   = 3, 
                //..........................................................................
                //RAW-YUV-shared feature id.
                FID_PRE_BEGIN_RAW_YUV_SI    = 2, 
    //                FID_SCENE_MODE        = 3, 
    //                FID_COLOR_EFFECT      = 4, 
    //                FID_CAPTURE_MODE      = 5, 
    //                FID_CAP_SIZE          = 6, 
    //                FID_PREVIEW_SIZE      = 7, 
    //                FID_FRAME_RATE        = 8, 
    //                FID_AE_FLICKER        = 9, 
                FID_OVER_LAST_RAW_YUV_SI    = 10, 
                //..........................................................................
                //YUV-only feature id.
                FID_PRE_BEGIN_YUV_ONLY_SI   = 9, 
                FID_OVER_LAST_YUV_ONLY_SI   = 10, 
              //----------------------------------------------------------------------------
              FID_OVER_LAST_SI = 10, 
              //////////////////////////////////////////////////////////////////////////////
              // Scene-Dependent (SD) feature id.
              FID_PRE_BEGIN_SD = 9, 
              //----------------------------------------------------------------------------
                //..........................................................................
                //Misc. feature id.
                FID_PRE_BEGIN_MISC_SD       = 9, 
    //                FID_FD_ON_OFF         = 10, 
                FID_OVER_LAST_MISC_SD       = 11, 
                //..........................................................................
                //RAW-only feature id.
                FID_PRE_BEGIN_RAW_ONLY_SD   = 10, 
                FID_OVER_LAST_RAW_ONLY_SD   = 11, 
                //..........................................................................
                //RAW-YUV-shared feature id.
                FID_PRE_BEGIN_RAW_YUV_SD    = 10, 
    //                FID_AE_SCENE_MODE     = 11, 
    //                FID_AE_METERING       = 12, 
    //                FID_AE_ISO            = 13, 
    //                FID_AE_EV             = 14, 
    //                FID_AF_MODE           = 15, 
    //                FID_AF_METERING       = 16, 
    //                FID_AWB_MODE          = 17, 
    //                FID_ISP_EDGE          = 18, 
    //                FID_ISP_HUE           = 19, 
    //                FID_ISP_SAT           = 20, 
    //                FID_ISP_BRIGHT        = 21, 
    //                FID_ISP_CONTRAST      = 22, 
                FID_OVER_LAST_RAW_YUV_SD    = 23, 
                //..........................................................................
                //YUV-only feature id.
                FID_PRE_BEGIN_YUV_ONLY_SD = 22, 
                FID_OVER_LAST_YUV_ONLY_SD = 23, 
              //----------------------------------------------------------------------------
              FID_OVER_LAST_SD = 23, 
              //////////////////////////////////////////////////////////////////////////////
            FID_OVER_LAST = 23, 
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            };
        };
        #undef  CHECK_FID_RANGE
        #define CHECK_FID_RANGE(_fid_range)  STATIC_CHECK((_fid_range==(MUINT32)FID_RANGE::_fid_range), _fid_range##_NOT_EQUAL)

        CHECK_FID_RANGE(FID_PRE_BEGIN);
        CHECK_FID_RANGE(FID_PRE_BEGIN_SI);
        CHECK_FID_RANGE(FID_PRE_BEGIN_MISC_SI);
        CHECK_FID_RANGE(FID_OVER_LAST_MISC_SI);
        CHECK_FID_RANGE(FID_PRE_BEGIN_RAW_ONLY_SI);
        CHECK_FID_RANGE(FID_OVER_LAST_RAW_ONLY_SI);
        CHECK_FID_RANGE(FID_PRE_BEGIN_RAW_YUV_SI);
        CHECK_FID_RANGE(FID_OVER_LAST_RAW_YUV_SI);
        CHECK_FID_RANGE(FID_PRE_BEGIN_YUV_ONLY_SI);
        CHECK_FID_RANGE(FID_OVER_LAST_YUV_ONLY_SI);
        CHECK_FID_RANGE(FID_OVER_LAST_SI);
        CHECK_FID_RANGE(FID_PRE_BEGIN_SD);
        CHECK_FID_RANGE(FID_PRE_BEGIN_MISC_SD);
        CHECK_FID_RANGE(FID_OVER_LAST_MISC_SD);
        CHECK_FID_RANGE(FID_PRE_BEGIN_RAW_ONLY_SD);
        CHECK_FID_RANGE(FID_OVER_LAST_RAW_ONLY_SD);
        CHECK_FID_RANGE(FID_PRE_BEGIN_RAW_YUV_SD);
        CHECK_FID_RANGE(FID_OVER_LAST_RAW_YUV_SD);
        CHECK_FID_RANGE(FID_PRE_BEGIN_YUV_ONLY_SD);
        CHECK_FID_RANGE(FID_OVER_LAST_YUV_ONLY_SD);
        CHECK_FID_RANGE(FID_OVER_LAST_SD);
        CHECK_FID_RANGE(FID_OVER_LAST);
    }
};


struct test_FeatureIdRange2
{
    test_FeatureIdRange2()
    {
        struct FID_RANGE
        {
            enum
            {
        FID_BEGIN               = 1, 
        FID_NUM                 = 22, 

      //////////////////////////////////////////////////////////////////////////////
      // Scene-Independent (SI) feature id.
        FID_BEGIN_SI            = 1, 
        FID_END_SI              = 8, 
    
        //Misc. feature id.
        FID_BEGIN_MISC_SI       = 1, 
        FID_END_MISC_SI         = 3, 
        FID_NUM_MISC_SI         = 2, 
    
        //RAW-only feature id.
        FID_BEGIN_RAW_ONLY_SI   = 3, 
        FID_END_RAW_ONLY_SI     = 3, 
        FID_NUM_RAW_ONLY_SI     = 0, 
        //RAW-YUV-shared feature id.
        FID_BEGIN_RAW_YUV_SI    = 3, 
        FID_END_RAW_YUV_SI      = 10, 
        FID_NUM_RAW_YUV_SI      = 7, 
        //YUV-only feature id.
        FID_BEGIN_YUV_ONLY_SI   = 10, 
        FID_END_YUV_ONLY_SI     = 10, 
        FID_NUM_YUV_ONLY_SI     = 0, 

        //RAW feature id.
        FID_BEGIN_RAW_SI        = 3, 
        FID_END_RAW_SI          = 10, 
        FID_NUM_RAW_SI          = 7, 
        //YUV feature id.
        FID_BEGIN_YUV_SI        = 3, 
        FID_END_YUV_SI          = 10, 
        FID_NUM_YUV_SI          = 7, 

      //////////////////////////////////////////////////////////////////////////////
      // Scene-Dependent (SD) feature id.
        FID_BEGIN_SD            = 10, 
        FID_END_SD              = 23, 
    
        //Misc. feature id.
        FID_BEGIN_MISC_SD       = 10, 
        FID_END_MISC_SD         = 11, 
        FID_NUM_MISC_SD         = 1, 

        //RAW-only feature id.
        FID_BEGIN_RAW_ONLY_SD   = 11, 
        FID_END_RAW_ONLY_SD     = 11, 
        FID_NUM_RAW_ONLY_SD     = 0, 
        //RAW-YUV-shared feature id.
        FID_BEGIN_RAW_YUV_SD    = 11, 
        FID_END_RAW_YUV_SD      = 23, 
        FID_NUM_RAW_YUV_SD      = 12, 
        //YUV-only feature id.
        FID_BEGIN_YUV_ONLY_SD   = 23, 
        FID_END_YUV_ONLY_SD     = 23, 
        FID_NUM_YUV_ONLY_SD     = 0, 

        //RAW feature id.
        FID_BEGIN_RAW_SD        = 11, 
        FID_END_RAW_SD          = 23, 
        FID_NUM_RAW_SD          = 12, 
        //YUV feature id.
        FID_BEGIN_YUV_SD        = 11, 
        FID_END_YUV_SD          = 23, 
        FID_NUM_YUV_SD          = 12, 
            };
        };
        #undef  CHECK_FID_RANGE
        #define CHECK_FID_RANGE(_fid_range)  STATIC_CHECK((_fid_range==(MUINT32)FID_RANGE::_fid_range), _fid_range##_NOT_EQUAL)
    
        CHECK_FID_RANGE(FID_BEGIN);
        CHECK_FID_RANGE(FID_NUM);
        CHECK_FID_RANGE(FID_BEGIN_MISC_SI);
        CHECK_FID_RANGE(FID_END_MISC_SI);
        CHECK_FID_RANGE(FID_NUM_MISC_SI);
        CHECK_FID_RANGE(FID_BEGIN_RAW_ONLY_SI);
        CHECK_FID_RANGE(FID_END_RAW_ONLY_SI);
        CHECK_FID_RANGE(FID_NUM_RAW_ONLY_SI);
        CHECK_FID_RANGE(FID_BEGIN_RAW_YUV_SI);
        CHECK_FID_RANGE(FID_END_RAW_YUV_SI);
        CHECK_FID_RANGE(FID_NUM_RAW_YUV_SI);
        CHECK_FID_RANGE(FID_BEGIN_YUV_ONLY_SI);
        CHECK_FID_RANGE(FID_END_YUV_ONLY_SI);
        CHECK_FID_RANGE(FID_NUM_YUV_ONLY_SI);
        CHECK_FID_RANGE(FID_BEGIN_RAW_SI);
        CHECK_FID_RANGE(FID_END_RAW_SI);
        CHECK_FID_RANGE(FID_NUM_RAW_SI);
        CHECK_FID_RANGE(FID_BEGIN_YUV_SI);
        CHECK_FID_RANGE(FID_END_YUV_SI);
        CHECK_FID_RANGE(FID_NUM_YUV_SI);
        CHECK_FID_RANGE(FID_BEGIN_SD);
        CHECK_FID_RANGE(FID_END_SD);
        CHECK_FID_RANGE(FID_BEGIN_MISC_SD);
        CHECK_FID_RANGE(FID_END_MISC_SD);
        CHECK_FID_RANGE(FID_NUM_MISC_SD);
        CHECK_FID_RANGE(FID_BEGIN_RAW_ONLY_SD);
        CHECK_FID_RANGE(FID_END_RAW_ONLY_SD);
        CHECK_FID_RANGE(FID_NUM_RAW_ONLY_SD);
        CHECK_FID_RANGE(FID_BEGIN_RAW_YUV_SD);
        CHECK_FID_RANGE(FID_END_RAW_YUV_SD);
        CHECK_FID_RANGE(FID_NUM_RAW_YUV_SD);
        CHECK_FID_RANGE(FID_BEGIN_YUV_ONLY_SD);
        CHECK_FID_RANGE(FID_END_YUV_ONLY_SD);
        CHECK_FID_RANGE(FID_NUM_YUV_ONLY_SD);
        CHECK_FID_RANGE(FID_BEGIN_RAW_SD);
        CHECK_FID_RANGE(FID_END_RAW_SD);
        CHECK_FID_RANGE(FID_NUM_RAW_SD);
        CHECK_FID_RANGE(FID_BEGIN_YUV_SD);
        CHECK_FID_RANGE(FID_END_YUV_SD);
        CHECK_FID_RANGE(FID_NUM_YUV_SD);
    }
};


#if 0


using namespace NSWordSet;

template <class T>
struct MY
{
    static void f() {}
};
template <>
struct MY<NullValue>
{
};



typedef MakeValuelist<10, 23>::Result type;
typedef MakeValuelist<10, 23>::Result type1;
//typedef MakeValuelist<0, 20, 31, 32, 0, 2, 0, 3, 56>::Result type2;
typedef MakeValuelist<0, 20, 31, 0, 2, 0, 3>::Result type2;
//typedef MakeValuelist<0, 20, 31, 32, 0, 2, 13, 3, 56>::Result type3;
//typedef MakeValuelist<0, 20, 31, 32, 1, 0, 2, 13, 3, 56>::Result type4;
typedef MakeValuelist<0, 20, 31, 1, 0, 2, 13, 3>::Result type4;
//typedef MakeWordValueList<type>::Result wlist;

typedef WordSet<type4>  wordset2;

void test()
{
    STATIC_CHECK(MaxWordCount<type1>::value==1, WORD_NOT_1);
//    STATIC_CHECK(MaxWordCount<type2>::value==2, WORD_NOT_2);
    STATIC_CHECK( (WordValue<type4, 0>::value==0x8010200F), WORDVALUE_0);
//    STATIC_CHECK( (WordValue<type4, 1>::value==0x01000001), WORDVALUE_1);

    MUINT32 const w[MaxWordCount<type>::value] = {
        0, 
//          wlist::w0, 
//        wlist<0>::value
    };


    wordset2    ws2;
    ws2.GetWordCount();
    ws2.GetWord(0);
    ws2.GetWord(2);


//    type::Result a;

    STATIC_CHECK(type::V==10, NOT_10);
    STATIC_CHECK(type::Tail::V==23, NOT_23);
//    STATIC_CHECK(type::Tail::Tail::V==0xFFFFFFFF, NOT_0xFFFFFFFF);
//    STATIC_CHECK(type::Tail::Tail::V==NullValue::V, NOT_NullValue);
#if 0
    MY<int>::f();
    MY<type::Tail::Tail>::f();
#endif
}


#endif
#endif  //MY_BUILD

