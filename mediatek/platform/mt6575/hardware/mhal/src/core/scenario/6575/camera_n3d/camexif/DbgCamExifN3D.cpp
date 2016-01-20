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
#define LOG_TAG "mHalCamN3D"
//
#include <utils/Errors.h>
#include <utils/threads.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>
//
#include <mhal/inc/camera.h>
//
#include "CamExifN3D.h"
#include "DbgCamExifN3D.h"
//
#include <exif_api.h>
#include <aaa_hal_base.h>
#include <isp_hal.h>


/*******************************************************************************
*
********************************************************************************/
#define MY_LOGD(fmt, arg...)        XLOGD(fmt, ##arg)
#define MY_LOGI(fmt, arg...)        XLOGI(fmt, ##arg)
#define MY_LOGW(fmt, arg...)        XLOGW(fmt, ##arg)
#define MY_LOGE(fmt, arg...)        XLOGE(fmt"<%s:#%d>", ##arg, __FILE__, __LINE__)
//
#define CAM_DEBUG_KEYID 0xF8F9FAFB
#define CAM_DEBUG_TAG_SIZE 10
#define CAM_DEBUG_MODULE_ID 0x0005
#define CAM_DEBUG_TAG_VERSION 0
//

#define MODULE_NUM(total_module, tag_module)      \
((MINT32)                                         \
 ((MUINT32)(0x00000000) |                         \
  (MUINT32)((total_module & 0xff) << 16) |        \
  (MUINT32)(tag_module & 0xff))                   \
)

#define CAMTAG(module_id, tag, line_keep)   \
( (MINT32)                                  \
  ((MUINT32)(0x00000000) |                  \
   (MUINT32)((module_id & 0xff) << 24) |    \
   (MUINT32)((line_keep & 0x01) << 23) |    \
   (MUINT32)(tag & 0xffff))                 \
)
//

struct CAM_DEBUG_TAG_T
{
    MUINT32 u4FieldID;
    MUINT32 u4FieldValue;
};

struct CAM_DEBUG_INFO_T
{
    CAM_DEBUG_TAG_T Tag[CAM_DEBUG_TAG_SIZE];
};

struct CAM_EXIF_DEBUG_INFO_T
{
    struct Header
    {
        MUINT32  u4KeyID;
        MUINT32  u4ModuleCount;
        MUINT32  u4CamDebugInfoOffset;
    } rCamHeader;

    CAM_DEBUG_INFO_T  rCamDebugInfo;
};

enum CamDebugTagID
{
    CAM_TAG_VERSION = 0,
    CAM_TAG_SHOT_MODE
};
//

#if (SHOT_ASSERT_ON)
    #define MY_LOGA(x, fmt, arg...) \
        if ( ! (x) ) { \
            printf(fmt"<Assert %s:#%d>", ##arg, __FILE__, __LINE__); \
            LOGE  (fmt"<Assert %s:#%d>", ##arg, __FILE__, __LINE__); \
            while (1) { ::usleep(500 * 1000); } \
        }
#else
    #define MY_LOGA(x, fmt, arg...)
#endif


/*******************************************************************************
*
********************************************************************************/
inline void setDebugTag(CAM_DEBUG_INFO_T &a_rCamDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldID = CAMTAG(CAM_DEBUG_MODULE_ID, a_i4ID, 0);
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

/*******************************************************************************
*
********************************************************************************/
DbgCamExifN3D::
DbgCamExifN3D(ESensorType_t const eSensorType, EDeviceId_t const eDeviceId)
    : CamExifN3D(eSensorType, eDeviceId)
    , mpIspHal(NULL)
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DbgCamExifN3D::
init(CamExifParam const& rCamExifParam, Hal3ABase*const pHal3A, CamDbgParam const& rCamDbgParam)
{
    MINT32 err = -1;

    //  (0)
    if  ( ! CamExifN3D::init(rCamExifParam, pHal3A) )
    {
        goto lbExit;
    }


    //  (1) IspHal
    if  (
            ! (mpIspHal = IspHal::createInstance()) //fail to create
        ||  0 > ( err = mpIspHal->init() )          //fail to init
        )
    {
        MY_LOGE("[DbgCamExifN3D::init] (mpIspHal, ec)=(%p, %X)", mpIspHal, err);
        if  ( mpIspHal )
        {
            mpIspHal->destroyInstance();
            mpIspHal = NULL;
        }
        goto lbExit;
    }

    // (2)
    memset(&mCamDbgParam, 0, sizeof(CamDbgParam));
    mCamDbgParam = rCamDbgParam;
    
    
    err = 0;
lbExit:
    return  (0==err);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DbgCamExifN3D::
uninit()
{
    if  (mpIspHal)
    {
        mpIspHal->uninit();
        mpIspHal->destroyInstance();
        mpIspHal = NULL;
    }
    //
    CamExifN3D::uninit();

    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
void 
DbgCamExifN3D::
getCamDebugInfo(CAM_EXIF_DEBUG_INFO_T &a_rCamExifDebugInfo)
{
    a_rCamExifDebugInfo.rCamHeader.u4KeyID = CAM_DEBUG_KEYID;
    a_rCamExifDebugInfo.rCamHeader.u4ModuleCount = MODULE_NUM(1,1);
    a_rCamExifDebugInfo.rCamHeader.u4CamDebugInfoOffset = sizeof(a_rCamExifDebugInfo.rCamHeader);

    // Cam Debug Info
    memset(&a_rCamExifDebugInfo.rCamDebugInfo, 0, sizeof(CAM_DEBUG_INFO_T));

    // Cam Debug Version
    setDebugTag(a_rCamExifDebugInfo.rCamDebugInfo, CAM_TAG_VERSION, (MUINT32)CAM_DEBUG_TAG_VERSION);

    // Shot mode
    setDebugTag(a_rCamExifDebugInfo.rCamDebugInfo, CAM_TAG_SHOT_MODE, mCamDbgParam.u4ShotMode);

}

/*******************************************************************************
*
********************************************************************************/
MBOOL
DbgCamExifN3D::
appendDebugExif(MUINT8* const puAppnBuf, MUINT32*const pu4AppnSize)
{
    MINT32  err = 0;

    MUINT8* pDst = puAppnBuf;

    MUINT32 u4AppnSize = 0;

    // Copy 3A debug info
    if  (mpHal3A)
    {
        MUINT8* p3ADebugInfo = NULL;
        MUINT32 aaaDebugSize = 0;
        MUINT32 app4ReturnSize = 0;
        mpHal3A->getDebugInfo((void **) &p3ADebugInfo, &aaaDebugSize);
        //
        if ( p3ADebugInfo && aaaDebugSize > 0)
        {
            err = ::exifAppnMake(4, pDst, p3ADebugInfo, aaaDebugSize, &app4ReturnSize);
            if  (err) {
                goto lbExit;
            }
            pDst += app4ReturnSize;
            u4AppnSize += app4ReturnSize;
        }
    }


    // Copy ISP debug info
    {
        MUINT8 ispDbgInfo[3072] = {0};
        MINT32 ispDbgSize = 0;
        MUINT32 app5ReturnSize = 0;
        mpIspHal->sendCommand(ISP_CMD_GET_EXIF_DEBUG_INFO, (MINT32)&ispDbgInfo[0], (MINT32)&ispDbgSize);
        if  ( 3072 >= ispDbgSize )
        {
            err = ::exifAppnMake(5, pDst, ispDbgInfo, ispDbgSize, &app5ReturnSize);
            if  (err) {
                goto lbExit;
            }
            pDst += app5ReturnSize;
            u4AppnSize += app5ReturnSize;
        }
        else
        {
            MY_LOGE("[DbgCamExifN3D::appendDebugExif] ispDbgSize(%d) > 3072", ispDbgSize);
            goto lbExit;
        }
    }

    // Cam debug info
    {
        MUINT8* pcamDebugInfo = NULL;
        MUINT32 camDebugSize = 0;
        MUINT32 app6ReturnSize = 0;
        CAM_EXIF_DEBUG_INFO_T rCamExifDebugInfo;
        memset(&rCamExifDebugInfo, 0, sizeof(CAM_EXIF_DEBUG_INFO_T));

        getCamDebugInfo(rCamExifDebugInfo);
        
        pcamDebugInfo = (MUINT8*) &rCamExifDebugInfo;
        camDebugSize = sizeof(CAM_EXIF_DEBUG_INFO_T);

        if ( pcamDebugInfo && camDebugSize > 0)
        {
            err = ::exifAppnMake(6, pDst, pcamDebugInfo, camDebugSize, &app6ReturnSize);
            if  (err) {
                goto lbExit;
            }
            pDst += app6ReturnSize;
            u4AppnSize += app6ReturnSize;
        }
        else
        {
            MY_LOGE("[DbgCamExifN3D::appendDebugExif] camDebugSize(%d) < 0", camDebugSize);
            goto lbExit;
        }
    }


    if  (pu4AppnSize)
    {
        *pu4AppnSize = u4AppnSize;
    }

lbExit:
    return  (0==err);
}

