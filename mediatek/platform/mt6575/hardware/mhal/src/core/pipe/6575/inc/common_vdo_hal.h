/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/** 
 * @file 
 *   common_vdo_hal.h
 *
 * @par Project:
 *   MT6575 
 *
 * @par Description:
 *   common video enc driver and mhal integration wrapper
 *
 * @par Author:
 *   Jackal Chen (mtk02532)
 *
 * @par $Revision: #1 $
 * @par $Modtime:$
 * @par $Log:$ 
 *
 */
 
#ifndef __COMMON_VDO_HAL_H__
#define __COMMON_VDO_HAL_H__

/*=============================================================================
 *                              Include Files
 *===========================================================================*/

#include "MediaHal.h"
#include "val_types.h" 
#include "vcodec_if.h"
#include "video_custom_sp.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=============================================================================
 *                              Type definition
 *===========================================================================*/

/**
 * @par Enumeration
 *   MHAL_VDO_PARAM
 * @par Description
 *   This is the item used for MHAL_VDO_PARAM
 */ 
typedef enum __MHAL_VDO_PARAM_T
{
    MHAL_VDO_PARAM_GET_BS_BUF_SIZE,     ///<: get bitstream buffer size
    MHAL_VDO_PARAM_GET_BS_BUF_COUNT,    ///<: get bitstream buffer count
    
    MHAL_VDO_PARAM_MAX = 0xFFFFFFFF     ///<: Max MHAL_VDO_PARAM value
} MHAL_VDO_PARAM_T;


typedef struct VENC_BS_s{
		VAL_UINT8_T                     *u4BS_addr;			
		VAL_UINT8_T                     *u4BS_addr_PA;			
		VAL_UINT32_T                    u4BSSize;	
		VAL_UINT32_T                    u4BS_frmSize;	
		VAL_UINT32_T                    u4BS_frmCount;			
		VAL_UINT32_T                     u4BS_index;			
		VAL_UINT32_T                     u4BS_preindex;			
		VAL_UINT32_T                    u4Fillcnt;		
		VAL_UINT32_T                    Handle;
}VENC_BS_T;

/**
 * @par Structure
 *   mhalVdoDrv_t
 * @par Description
 *   This is a structure which store common video enc driver information 
 */
typedef struct mhalVdoDrv_s {
    VAL_VOID_T                      *prCodecHandle;
    VAL_UINT32_T                    u4EncodedFrameCount;
    VCODEC_ENC_CALLBACK_T           rCodecCb;
    VIDEO_ENC_API_T                 *prCodecAPI;
    VENC_BS_T                       pBSBUF;

    //--> M4U related 
    VAL_VOID_T                      *pvBSHandle; 
    VAL_UINT32_T                    u4BSVa;
    VAL_UINT32_T                    u4BSPa;
    VAL_UINT32_T                    u4BSSize;
    VAL_VOID_T                      *pvYUVHandle;
    VAL_UINT32_T                    u4YUVVa;
    VAL_UINT32_T                    u4YUVPa;
    VAL_UINT32_T                    u4YUVSize;
    //<-- M4U related
    VAL_UINT8_T                     header_buf[100];
    VAL_UINT8_T                     *tmp_buf;
    VAL_UINT32_T                    header_len;
    VAL_UINT32_T                    u4WorkingIdx;
    VAL_UINT32_T                    u4Width;
    VAL_UINT32_T                    u4Height;
    VAL_UINT32_T                    u4Fps;
    VAL_UINT32_T                    u4Bitrate;     
    VAL_UINT32_T                    sps_first;
    VAL_MEMORY_T                    rTempMem;
    VAL_MEMORY_T                    rWorkingMem;
    VAL_MEMORY_T                    rRingMem[2];
    VCODEC_ENC_BUFFER_INFO_T        EncoderInputParamNC;    
    VAL_UINT32_T                    u4CodecType;  
}mhalVdoDrv_t;


typedef struct halVdoBufInfo_s {
    MUINT32 hwIndex;             // 目前寫的 Index 
    MUINT32 fillCnt;               // 目前已經寫的buffer count. 
    MUINT32 TimeStamp;
} halVdoBufInfo_t;


/*=============================================================================
 *                             Function Declaration
 *===========================================================================*/
INT32 mHalVdoInit(VDO_ENC_FORMAT peVEncFormat, mhalCamVdoParam_t *pmhalCamVdoParam, mhalCamFrame_t *pfrmBS, MUINT32 bsIdx, mhalVdoDrv_t *pmhalVdoDrv);
INT32 mHalVdoEncodeFrame(VDO_ENC_FORMAT peVEncFormat, mhalCamFrame_t *pfrmYuv, MUINT32 yuvIdx, mhalCamFrame_t *pfrmBS, MUINT32 bsIdx,  mhalVdoDrv_t *pmhalVdoDrv);
INT32 mHalVdoUninit(VDO_ENC_FORMAT peVEncFormat, mhalVdoDrv_t *pmhalVdoDrv);
INT32 mHalVdoGetParam(VDO_ENC_FORMAT peVEncFormat, MHAL_VDO_PARAM_T eCtrlCode, MVOID *pBuffer, MUINT32 u4BufSize);
INT32 mHalVdoSetParam(VDO_ENC_FORMAT peVEncFormat, MHAL_VDO_PARAM_T eCtrlCode, MVOID *pBuffer, MUINT32 u4BufSize);

INT32 mhalVdoDequeueBuff(halVdoBufInfo_t * a_pstBuffInfo, mhalVdoDrv_t *pmhalVdoDrv);
INT32 mhalVdoEnqueueBuffer(MINT32 index, mhalVdoDrv_t *pmhalVdoDrv);
#ifdef __cplusplus
}
#endif

#endif //#ifndef __COMMON_VDO_HAL_H__

