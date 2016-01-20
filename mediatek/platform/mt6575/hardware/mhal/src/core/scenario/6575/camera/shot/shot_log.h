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
#ifndef _SHOT_LOG_H_
#define _SHOT_LOG_H_

#include <cutils/xlog.h>

/*******************************************************************************
*
********************************************************************************/
#define SHOT_ASSERT_ON              (1)


/*******************************************************************************
*
********************************************************************************/
#define CAM_LOGV(fmt, arg...)       XLOGV(fmt"\r\n", ##arg)
#define CAM_LOGD(fmt, arg...)       XLOGD(fmt"\r\n", ##arg)
#define CAM_LOGI(fmt, arg...)       XLOGI(fmt"\r\n", ##arg)
#define CAM_LOGW(fmt, arg...)       XLOGW(fmt"\r\n", ##arg)
#define CAM_LOGE(fmt, arg...)       XLOGE(fmt" (%s){#%d:%s}""\r\n", ##arg, __FUNCTION__, __LINE__, __FILE__)
//

#if (SHOT_ASSERT_ON)
    #define CAM_LOGA(...) \
        do { \
            CAM_LOGE("[Assert] "__VA_ARGS__); \
            CAM_LOGE("(%s){#%d:%s}""\r\n", __FUNCTION__, __LINE__, __FILE__); \
            while(1) { ::usleep(500 * 1000); } \
        } while (0)
#else
    #define CAM_LOGA(...)
#endif

/*******************************************************************************
*
********************************************************************************/
#define CAM_LOGV_IF(cond, ...)      if ( ! (cond) ) { CAM_LOGV(__VA_ARGS__); }
#define CAM_LOGD_IF(cond, ...)      if ( ! (cond) ) { CAM_LOGD(__VA_ARGS__); }
#define CAM_LOGI_IF(cond, ...)      if ( ! (cond) ) { CAM_LOGI(__VA_ARGS__); }
#define CAM_LOGW_IF(cond, ...)      if ( ! (cond) ) { CAM_LOGW(__VA_ARGS__); }
#define CAM_LOGE_IF(cond, ...)      if ( ! (cond) ) { CAM_LOGE(__VA_ARGS__); }
#define CAM_LOGA_IF(cond, ...)      if ( ! (cond) ) { CAM_LOGA(__VA_ARGS__); }


/*******************************************************************************
*
********************************************************************************/
#define MY_LOGD(fmt, arg...)        CAM_LOGD("{%s}"fmt, getShotName(), ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("{%s}"fmt, getShotName(), ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("{%s}"fmt, getShotName(), ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("{%s}"fmt, getShotName(), ##arg)
#define MY_LOGA(x, fmt, arg...)     CAM_LOGA(x, "{%s}"fmt, getShotName(), ##arg)


#endif  //  _SHOT_LOG_H_

