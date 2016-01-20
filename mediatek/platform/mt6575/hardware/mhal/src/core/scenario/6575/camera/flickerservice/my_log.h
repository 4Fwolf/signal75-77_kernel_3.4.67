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
#ifndef _MY_LOG_H_
#define _MY_LOG_H_

#include <cutils/xlog.h>

/*******************************************************************************
*
********************************************************************************/
#define ASSERT_ON       (1)


/*******************************************************************************
*
********************************************************************************/
#define MY_LOGD(fmt, arg...)        XLOGD(fmt, ##arg)
#define MY_LOGI(fmt, arg...)        XLOGI(fmt, ##arg)
#define MY_LOGW(fmt, arg...)        XLOGW(fmt, ##arg)
#define MY_LOGE(fmt, arg...)        XLOGE(fmt"<%s:#%d>", ##arg, __FILE__, __LINE__)
//
#if (ASSERT_ON)
    #define MY_LOGA(x, fmt, arg...) \
        if ( ! (x) ) { \
            printf(fmt"<Assert %s:#%d>", ##arg, __FILE__, __LINE__); \
            XLOGE  (fmt"<Assert %s:#%d>", ##arg, __FILE__, __LINE__); \
            while (1) { ::usleep(500 * 1000); } \
        }
#else
    #define MY_LOGA(x, fmt, arg...)
#endif


/*******************************************************************************
*
********************************************************************************/
#define MY_DBG(fmt, arg...)         MY_LOGD("{%s}"fmt, getFrameSvcName(), ##arg)
#define MY_INFO(fmt, arg...)        MY_LOGI("{%s}"fmt, getFrameSvcName(), ##arg)
#define MY_WARN(fmt, arg...)        MY_LOGW("{%s}"fmt, getFrameSvcName(), ##arg)
#define MY_ERR(fmt, arg...)         MY_LOGE("{%s}"fmt, getFrameSvcName(), ##arg)
#define MY_ASSERT(x, fmt, arg...)   MY_LOGA(x, "{%s}"fmt, getFrameSvcName(), ##arg)


#endif  //  _MY_LOG_H_

