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
#ifndef _DEBUG_H_
#define _DEBUG_H_


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*
    (1) GLOBAL_ENABLE_MY_xxx == 0
         --> force to disable.
    (2) GLOBAL_ENABLE_MY_xxx == 1
         --> ENABLE_MY_xxx in local file decides to enable/disable.
        (2.1) ENABLE_MY_xxx in local file == 1 --> enable.
        (2.2) ENABLE_MY_xxx in local file == 0 --> disable.
        (2.3) ENABLE_MY_xxx in local file undefine
               --> ENABLE_MY_xxx in global file decides to enable/disable.
*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Global On/Off
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#undef  GLOBAL_ENABLE_MY_LOG
#define GLOBAL_ENABLE_MY_LOG        (1)

#undef  GLOBAL_ENABLE_MY_ERR
#define GLOBAL_ENABLE_MY_ERR        (1)

#undef  GLOBAL_ENABLE_MY_LOG_OBJ
#define GLOBAL_ENABLE_MY_LOG_OBJ    (1)

#undef  GLOBAL_ENABLE_MY_ASSERT
#define GLOBAL_ENABLE_MY_ASSERT     (1)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Local On/Off
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG           (1)
#endif

#ifndef ENABLE_MY_ERR
    #define ENABLE_MY_ERR           (1)
#endif

#ifndef ENABLE_MY_LOG_OBJ
    #define ENABLE_MY_LOG_OBJ       (1)
#endif

#ifndef ENABLE_MY_ASSERT
    #define ENABLE_MY_ASSERT        (1)
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (GLOBAL_ENABLE_MY_LOG != 0 && ENABLE_MY_LOG != 0)
    #define MY_LOG(fmt, arg...) XLOGD(fmt, ##arg)
#else
    #define MY_LOG(fmt, arg...)
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (GLOBAL_ENABLE_MY_ERR != 0 && ENABLE_MY_ERR != 0)
    #define MY_ERR(fmt, arg...) XLOGE("Err: %5d:, "fmt, __LINE__, ##arg)
#else
    #define MY_ERR(fmt, arg...)
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (GLOBAL_ENABLE_MY_LOG_OBJ != 0 && ENABLE_MY_LOG_OBJ != 0)
    #define MY_LOG_OBJ(fmt, arg...)\
        struct MyLogObj {\
            MyLogObj() {\
                MY_LOG("[MyLogObj] %p\n", this);\
                MY_LOG(fmt, ##arg);\
            }\
        }
    #define MY_LOG_OBJ_ONCE(fmt, arg...)\
        {static MY_LOG_OBJ(fmt, ##arg) obj;}

#else
    #define MY_LOG_OBJ(fmt, arg...)
    #define MY_LOG_OBJ_ONCE(fmt, arg...)
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if (GLOBAL_ENABLE_MY_ASSERT != 0 && ENABLE_MY_ASSERT != 0)
    #define MY_ASSERT(x, str)\
        if (x) {} \
        else   {  \
            MY_ERR("[Assert %s, %d]: %s", __FILE__, __LINE__, str); while(1); \
        }
#else
    #define MY_ASSERT(x, str)
#endif


#endif // _DEBUG_H_

