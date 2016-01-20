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
#define LOG_TAG "NvramBufMgr"
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif
//
#include <utils/Errors.h>
#include <cutils/log.h>
//
#include "nvram_drv.h"
#include "nvram_buf_mgr.h"
//
using namespace NSNvram;
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
NvramBufMgr&
NvramBufMgr::
getInstance()
{
    static NvramBufMgr singleton;
    return singleton;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Buffers.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define SPECIALIZATION_GET_BUF(Buf_T, SensorEnum) \
    template <> \
    Buf_T* \
    NvramBufMgr::\
    getBuf<Buf_T, SensorEnum>() const \
    { \
        static Buf_T buf; \
        return &buf; \
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Map Buffer Type to Command.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MAP_BUF_TYPE_TO_CMD(Buf_T, Cmd)\
    template<> \
    struct BufT2Cmd<Buf_T> \
    { \
        static ENvramDrvCmd_T const val = Cmd; \
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  NvramDrvBase::getBufIF()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define SPECIALIZATION_GET_BUF_IF(Buf_T)\
    template <> \
    NSNvram::BufIF<Buf_T>* \
    NvramDrvBase:: \
    getBufIF() const \
    { \
        static NSNvram::ImpBufIF<Buf_T> singleton; \
        return &singleton; \
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Specialization
//      getBuf
//      BufT2Cmd
//      getBufIF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define _SPECIALIZATION_(Buf_T, Cmd) \
    namespace NSNvram { \
        SPECIALIZATION_GET_BUF(Buf_T, DUAL_CAMERA_MAIN_SENSOR); \
        SPECIALIZATION_GET_BUF(Buf_T, DUAL_CAMERA_SUB_SENSOR); \
        MAP_BUF_TYPE_TO_CMD(Buf_T, Cmd); \
    } /* NSNvram */\
    SPECIALIZATION_GET_BUF_IF(Buf_T);


/*******************************************************************************
* List of Specializations.
*******************************************************************************/
//  To-add:
_SPECIALIZATION_( NVRAM_CAMERA_ISP_PARAM_STRUCT, CAMERA_NVRAM_DATA_ISP);
_SPECIALIZATION_( NVRAM_CAMERA_SHADING_STRUCT  , CAMERA_NVRAM_DATA_SHADING);
_SPECIALIZATION_( NVRAM_CAMERA_DEFECT_STRUCT   , CAMERA_NVRAM_DATA_DEFECT);


















/*******************************************************************************
* Test
*******************************************************************************/
#if 0
void test()
{
    NvramDrvBase& rDrv = *NvramDrvBase::createInstance();

    //  NVRAM_CAMERA_ISP_PARAM_STRUCT
    {
    typedef NVRAM_CAMERA_ISP_PARAM_STRUCT   buf_t;
    NSNvram::BufIF<buf_t>*const pBufIF = rDrv.getBufIF<buf_t>();

    MINT32 err = 0;
    buf_t*      p  = pBufIF->getRefBuf(DUAL_CAMERA_MAIN_SENSOR, 0x1234);
    buf_t const*pc = pBufIF->getRefBuf(DUAL_CAMERA_MAIN_SENSOR, 0x1234);
               err = pBufIF->refresh  (DUAL_CAMERA_MAIN_SENSOR, 0x1234);
               err = pBufIF->flush    (DUAL_CAMERA_MAIN_SENSOR, 0x1234);
    }

    //  NVRAM_CAMERA_SHADING_STRUCT
    {
    typedef NVRAM_CAMERA_SHADING_STRUCT   buf_t;
    NSNvram::BufIF<buf_t>*const pBufIF = rDrv.getBufIF<buf_t>();

    MINT32 err = 0;
    buf_t*      p  = pBufIF->getRefBuf(DUAL_CAMERA_MAIN_SENSOR, 0x1234);
    buf_t const*pc = pBufIF->getRefBuf(DUAL_CAMERA_MAIN_SENSOR, 0x1234);
               err = pBufIF->refresh  (DUAL_CAMERA_MAIN_SENSOR, 0x1234);
               err = pBufIF->flush    (DUAL_CAMERA_MAIN_SENSOR, 0x1234);
    }

    //  NVRAM_CAMERA_DEFECT_STRUCT
    {
    typedef NVRAM_CAMERA_DEFECT_STRUCT   buf_t;
    NSNvram::BufIF<buf_t>*const pBufIF = rDrv.getBufIF<buf_t>();

    MINT32 err = 0;
    buf_t*      p  = pBufIF->getRefBuf(DUAL_CAMERA_MAIN_SENSOR, 0x1234);
    buf_t const*pc = pBufIF->getRefBuf(DUAL_CAMERA_MAIN_SENSOR, 0x1234);
               err = pBufIF->refresh  (DUAL_CAMERA_MAIN_SENSOR, 0x1234);
               err = pBufIF->flush    (DUAL_CAMERA_MAIN_SENSOR, 0x1234);
    }

    rDrv.destroyInstance();
}
#endif

