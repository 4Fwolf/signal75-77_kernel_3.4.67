/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2007
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#include "jpeg_dec_hal.h"

#ifndef __JPEG_DEC_DATA_H__
#define __JPEG_DEC_DATA_H__

typedef struct JPEG_FILE_STRUCTURE
{
    UINT32 srcStreamAddr;
    UINT32 srcStreamSize;

    //UINT32 dstStreamAddr;
    //UINT32 dstStreamSize;

    //UINT32 tempBufferAddr;
    //UINT32 tempBufferSize;
    
   // JPEG file information
    UINT32 thumbnail_offset;
    UINT32 jpg_precision;
    
    UINT32 width;
    UINT32 height;
   
    UINT32 paddedWidth;             ///< the padded width after JPEG block padding
    UINT32 paddedHeight;            ///< the padded height after JPEG block padding

    UINT32 mcuRow;                  ///< number of MCU row in the JPEG file
    UINT32 mcuColumn;               ///< number of MCU column in the JPEG file

    UINT32 totalDU[3];              ///< (required by HW decoder) number of DU for each component
    UINT32 duPerMCURow[3];          ///< (required by HW decoder) DU per MCU row for each component
    UINT32 dummyDU[3];              ///< (required by HW decoder) number of dummy DU for each component

    UINT32 samplingFormat;
    UINT8 jpg_progressive;          // 0: baseline, 1:progressive
    
    // JPEG component information
    UINT32  componentNum;
    UINT32  componentID[3];          ///< Ci
    UINT32  hSamplingFactor[3];      ///< Hi
    UINT32  vSamplingFactor[3];      ///< Vi
    UINT32  qTableSelector[3];       ///< Tqi

    UINT32  hSamplingFactorMax;
    UINT32  vSamplingFactorMax;

} JPEG_FILE_INFO;

#define JPEG_MAX_OFFSET         0xFFFFFFFF

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
int jpeg_drv_parse_file(UINT8 *f_ptr, UINT32 f_size, JPEG_FILE_INFO *info);
int jpeg_drv_calculate_info(JPEG_FILE_INFO *info);


//-------------------------------------------------------------------
// JPEG MARKER
//-------------------------------------------------------------------

#define JPEG_MARKER_START_CODE   0XFF

#define JPEG_MARKER_SOF(I)       (0xC0 + I)

/* Start of Frame markers, non-differential, Huffman coding */
#define JPEG_MARKER_SOF0         0XC0
#define JPEG_MARKER_SOF1         0XC1
#define JPEG_MARKER_SOF2         0XC2
#define JPEG_MARKER_SOF3         0XC3

/* Start of Frame markers, differential, Huffman coding */
#define JPEG_MARKER_SOF5         0XC5
#define JPEG_MARKER_SOF6         0XC6
#define JPEG_MARKER_SOF7         0XC7

/* Start of Frame markers, non-differential, arithmatic coding */
#define JPEG_MARKER_JPG0         0XC8
#define JPEG_MARKER_SOF9         0XC9
#define JPEG_MARKER_SOF10        0XCA
#define JPEG_MARKER_SOF11        0XCB

/* Start of Frame markers, differential, arithmatic coding */
#define JPEG_MARKER_SOF13        0xCD
#define JPEG_MARKER_SOF14        0xCE
#define JPEG_MARKER_SOF15        0xCF

/* Huffman table specification */
#define JPEG_MARKER_DHT          0xC4  /* Define Huffman table(s) */
 
/* Arithmatic coding conditioning specification */
#define JPEG_MARKER_DAC          0xCC  /* Define arithmatic coding conditioning(s) */

/* Restart interval termination */
#define JPEG_MARKER_RST(I)       (0xD0 + I)
#define JPEG_MARKER_RST0         0xD0
#define JPEG_MARKER_RST1         0xD1
#define JPEG_MARKER_RST2         0xD2
#define JPEG_MARKER_RST3         0xD3
#define JPEG_MARKER_RST4         0xD4
#define JPEG_MARKER_RST5         0xD5
#define JPEG_MARKER_RST6         0xD6
#define JPEG_MARKER_RST7         0xD7

#define JPEG_MARKER_SOI          0xD8
#define JPEG_MARKER_EOI          0xD9
#define JPEG_MARKER_SOS          0xDA
#define JPEG_MARKER_DQT          0xDB
#define JPEG_MARKER_DNL          0xDC
#define JPEG_MARKER_DRI          0xDD
#define JPEG_MARKER_DHP          0xDE
#define JPEG_MARKER_EXP          0xDF

#define JPEG_MARKER_APP(I)       (0xE0 + I)

#define JPEG_MARKER_JPG(I)       (0xF0 + I)

#define JPEG_MARKER_TEM          0x01

#define JPEG_MARKER_ZERO         0x00

#define JPEG_MARKER_COM          0xFE


/* Definition for mandatory marker to decode a JPEG file */
#define JPEG_MANDATORY_MARKER_SOF_BIT  0x00000001
#define JPEG_MANDATORY_MARKER_DQT_BIT  0x00000002
#define JPEG_MANDATORY_MARKER_DHT_BIT  0x00000004

#define JPEG_MANDATORY_MARKER_CHECK    0x00000007

#endif   /// __JPEG_DEC_DATA_H__
