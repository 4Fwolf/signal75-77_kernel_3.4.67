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
** $Log: mcam_log.h $
 *
*/

#ifndef _MCAM_LOG_H_ 
#define _MCAM_LOG_H_

//
#include <cutils/xlog.h>


/*******************************************************************************
*
********************************************************************************/
#define MCAM_LOG_LEVEL        (MCAM_LOG_DBG) 
#define MCAM_ASSERT_ON               1


/*******************************************************************************
*
********************************************************************************/
#define MCAM_DBG(fmt, arg...) XLOGD(fmt, ##arg)

#define MCAM_DBG1(fmt, arg...) \
    do { \
        if (MCAM_LOG_LEVEL >= 1) { \
        	XLOGD(fmt, ##arg); \
        } \
    }while(0)

#define MCAM_DBG2(fmt, arg...) \
    do { \
        if (MCAM_LOG_LEVEL >= 2) { \
        	XLOGD(fmt, ##arg); \
        } \
    }while(0)

#define MCAM_INFO(fmt, arg...) XLOGI(fmt, ##arg)

#define MCAM_ERR(fmt, arg...)    XLOGE("Err: %5d:, "fmt, __LINE__, ##arg)
#define MCAM_WARN(fmt, arg...) XLOGW(fmt, ##arg)

#if MCAM_ASSERT_ON
#define MCAM_ASSERT(x, str); \
    if (x) {} \
    else { \
        printf("[Assert %s, %d]: %s", __FILE__, __LINE__, str); \
        XLOGE("[Assert %s, %d]: %s", __FILE__, __LINE__, str); \
        while(1) { \
            usleep(500 * 1000); \
        } \
    }
#else
#define MCAM_ASSERT(x, str)  
#endif

#endif 
