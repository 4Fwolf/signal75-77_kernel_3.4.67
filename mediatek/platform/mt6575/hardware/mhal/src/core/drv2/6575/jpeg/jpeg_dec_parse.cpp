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

#include "jpeg_dec_data.h"
#include <cutils/log.h>
#include <cutils/xlog.h>

#undef LOG_TAG
#define LOG_TAG "mHalJpgParser"

#define CHECK_IDX_SIZE(idx, size)   \
{  \
   if((idx)>=(size)) {  \
      XLOGW("Jpeg Header Hit Broken Bitstream, idx %d, f_size %d, L:%d!!!", (idx), (size), __LINE__); \
      return -1; \
   }  \
}

/// Refer to CCITT Rec. T.81 (1992 E) Page.48 Figure B.16 - Flow of compressed data syntax
int jpeg_drv_parse_file(UINT8 *f_ptr, UINT32 f_size, JPEG_FILE_INFO *info)
{
    UINT8 byte, byte1, byte2;
    UINT32 index, index_offset, i, count, temp;
    UINT16 marker_length;   
    bool bJFIF = false;
    //info->srcStreamAddr = f_ptr;
    info->srcStreamSize = f_size;
    
    info->thumbnail_offset = JPEG_MAX_OFFSET;
    //info->file_size = f_size;
    index = 0;
    CHECK_IDX_SIZE(index+1, f_size) ;
    byte1 = f_ptr[index++];
    byte2 = f_ptr[index++];

    if ((byte1 != JPEG_MARKER_START_CODE) || (byte2 != JPEG_MARKER_SOI))
    {
        return -1;
    }
    
    while(index < f_size)
    {       
        // search 0xFF
        do{
            CHECK_IDX_SIZE(index, f_size) ;
            byte1 = f_ptr[index++];
        }while ((byte1 != JPEG_MARKER_START_CODE) && (index < f_size));

        if(index >= f_size)
            break;
        CHECK_IDX_SIZE(index, f_size) ;    
        byte2 = f_ptr[index++];

        switch (byte2)
        {
            case JPEG_MARKER_SOF0:
            case JPEG_MARKER_SOF2:
            
                if (byte2 == JPEG_MARKER_SOF0)
                {       
                    // baseline mode
                    info->jpg_progressive = 0;
                }
                else
                {
                    // progressive mode
                    info->jpg_progressive = 1;
                }
                CHECK_IDX_SIZE(index+7, f_size) ;    
                marker_length = (f_ptr[index++] << 8);
                marker_length |= f_ptr[index++] - 2;
                
                info->jpg_precision = f_ptr[index++];
                info->height = (f_ptr[index++] << 8);
                info->height |= f_ptr[index++];           
                info->width = (f_ptr[index++] << 8);
                info->width |= f_ptr[index++];
                
                info->componentNum = f_ptr[index++];
                
                if(info->componentNum != 1 && info->componentNum != 3){ 
                  XLOGW("HW decoder unsupport componentNum (%d) !!!", info->componentNum); 
                  return -1;
                }

                for(i = 0 ; i < info->componentNum ; i++)
                {
                    CHECK_IDX_SIZE(index+2, f_size) ;    
                    info->componentID[i] = f_ptr[index++];
                    info->hSamplingFactor[i] = (f_ptr[index] & 0xF0) >> 4;
                    info->vSamplingFactor[i] = f_ptr[index++] & 0x0F;
                    info->qTableSelector[i] = f_ptr[index++];
                }
        
                //byte2 = JPEG_MARKER_EOI;
                break;
                
            case JPEG_MARKER_SOS:
            case JPEG_MARKER_JPG0:
            case JPEG_MARKER_ZERO:
                break;
                
            case JPEG_MARKER_DQT:
            case JPEG_MARKER_DHT:
                if(info->thumbnail_offset == JPEG_MAX_OFFSET)
                {
                    info->thumbnail_offset = index - 2;
                }
                CHECK_IDX_SIZE(index+2, f_size) ;    
                index_offset=(f_ptr[index++] << 8);
                index_offset += (int) (f_ptr[index++]-2);
                byte = f_ptr[index];
                

                if(index_offset > 2048)
                {
                    XLOGW("Marker:%x length:%d to larger", byte2, index_offset);
                    return -1;
                }

                if(byte2 == JPEG_MARKER_DHT)
                {
                    if((byte != 0x00) && (byte != 0x01) && (byte != 0x10) && (byte != 0x11))
                    {
                        XLOGW("Marker:%x table number error : 0x%x", byte2, byte);
                        return -1;
                    }
                    count = 1;
                    while(count < index_offset)
                    {
                        temp = 0;
                        for(i = 0 ; i < 16 ; i++)
                        { 
                            if( (index + count) >= f_size ){
                              XLOGW("Marker: Broken Huffman table Li %d, %x+%x > %x!!",i, index, count, f_size);
                              return -1;                           
                            }                              
                            temp += f_ptr[index + count];
                            count++;
                        }
                        count += temp;
                        if( temp > 256){
                            XLOGW("Marker: Bad Huffman table huff count %d !!", temp);
                            return -1;                                                   
                        }else if( (index + count) >= f_size ){
                            XLOGW("Marker: Broken Huffman table %x+%x > %x!!", index, count, f_size);
                            return -1;                           
                        }                        
                        if(count >= index_offset  )
                            break;

                            
                        byte = f_ptr[index + count];
                        if((byte != 0x00) && (byte != 0x01) && (byte != 0x10) && (byte != 0x11))
                        {
                            XLOGW("Marker:%x table number error : 0x%x", byte2, byte);
                            return -1;
                        }
                        count++;
                    }
                }
                else
                {
                    if((byte >> 4) != 0)
                        return -1;
                    if(index_offset > 65)
                    {
                        CHECK_IDX_SIZE(index+65, f_size) ; 
                        byte = f_ptr[index + 65];
                        if((byte >> 4) != 0)
                        {
                            XLOGW("Marker:%x Pq number error : 0x%x", byte2, byte);
                            return -1;                        
                        }
                    }
                    if(index_offset > 65*2)
                    {
                        CHECK_IDX_SIZE(index+130, f_size) ; 
                        byte = f_ptr[index + 65*2];
                        if((byte >> 4) != 0)
                        {
                            XLOGW("Marker:%x Pq number error : 0x%x", byte2, byte);
                            return -1; 
                        }
                    }
                }

                index +=index_offset;
                break;
                
            default:
                if ((byte2==JPEG_MARKER_SOF1) ||
                   ((byte2>=JPEG_MARKER_SOF3) && (byte2<=JPEG_MARKER_SOF15))||
                   (byte2==JPEG_MARKER_DQT) || (byte2==JPEG_MARKER_DNL) ||
                   (byte2==JPEG_MARKER_DHP) ||
                   (byte2==JPEG_MARKER_EXP) || (byte2==JPEG_MARKER_COM))
                {
                        CHECK_IDX_SIZE(index+1, f_size) ; 
                        index_offset = (f_ptr[index++] << 8);
                        index_offset += (int) (f_ptr[index++]-2);
                        index += index_offset;
                }else if( byte2== JPEG_MARKER_DRI){
					           XLOGW("Marker:%x decoder unsupport DRI marker !!", byte2);
					           return -1;   
				        }else if ((byte2>=JPEG_MARKER_APP(0)) && (byte2<=JPEG_MARKER_APP(15)))
                { 
                    if(byte2 == JPEG_MARKER_APP(14) && !bJFIF)
                    {
                        XLOGW("Unsupport Adobe Marker!!!");
                        return -1;
                    }
                    else
                    {
                        CHECK_IDX_SIZE(index+1, f_size) ; 
                        bJFIF = true;
                        index_offset = (f_ptr[index++] << 8);
                        index_offset += (int) (f_ptr[index++]-2);
                        index += index_offset;                        
                    }
                }
                break;
        }

        if((byte1 == JPEG_MARKER_START_CODE) && (byte2 == JPEG_MARKER_EOI))
        {
            break;
        }
    }


   return 0;
}

int jpeg_drv_calculate_info(JPEG_FILE_INFO *info)
{
    INT8 h[3], v[3];
    UINT32 i;
    UINT32 width, height;
    UINT32 y_h_factor, y_v_factor;
    UINT32 comp_h_count, comp_v_count;
    
    switch (info->componentNum)
    {
        case 1:
            info->samplingFormat = 400;
            info->hSamplingFactorMax = info->hSamplingFactor[0];    
            info->vSamplingFactorMax = info->vSamplingFactor[0]; 
            break;

        case 3:
            /// using the sum of Hy & Vy as the color format index
            /// "00" : YUV 4:4:4 (Hy=1, Vy=1), (sum=2) - 2 = 0
            /// "01" : YUV 4:2:2 (Hy=2, Vy=1), (sum=3) - 2 = 1
            /// "10" : YUV 4:2:0 (Hy=2, Vy=2), (sum=4) - 2 = 2
            /// "11" : YUV 4:1:1 (Hy=4, Vy=1), (sum=5) - 2 = 3


            h[0] = info->hSamplingFactor[0];
            h[1] = info->hSamplingFactor[1];
            h[2] = info->hSamplingFactor[2];
            v[0] = info->vSamplingFactor[0];
            v[1] = info->vSamplingFactor[1];
            v[2] = info->vSamplingFactor[2];

            if (1 != (v[1] * v[2] * h[1] * h[2]))
            {
                info->samplingFormat = 0;
                return -1;
            }
            else
            {
                switch (h[0] + v[0] - 2)
                {
                    case 0:
                        info->samplingFormat = 444;
                        info->hSamplingFactorMax = h[0];
                        info->vSamplingFactorMax = v[0];
                        break;
                    
                    case 1:
                        info->samplingFormat = 422;
                        info->hSamplingFactorMax = h[0];
                        info->vSamplingFactorMax = v[0];
                        break;

                    case 2:
                        info->samplingFormat = 420;
                        info->hSamplingFactorMax = h[0];
                        info->vSamplingFactorMax = v[0]; 
                        break;

                    case 3:
                        info->samplingFormat = 411;
                        info->hSamplingFactorMax = h[0];
                        info->vSamplingFactorMax = v[0];
                        break;

                    default:   
                        info->samplingFormat = 0;
                        info->hSamplingFactorMax = h[0];
                        info->vSamplingFactorMax = v[0];
                        return -1;
                }
            }
            break;

        default:
            info->samplingFormat = 0;
            return -1;
    }
   


    /// Calculate the paddedWidth & paddedHeight
    width = info->width;
    height = info->height;
    y_h_factor = 2 + info->hSamplingFactor[0];
    y_v_factor = 2 + info->vSamplingFactor[0];
    
    info->mcuRow = (width + (1 << y_h_factor) - 1) >> y_h_factor;
    info->paddedWidth = info->mcuRow << y_h_factor;

    info->mcuColumn = (height + (1 << y_v_factor) - 1) >> y_v_factor;
    info->paddedHeight = info->mcuColumn << y_v_factor;

    if(info->hSamplingFactorMax == 0 || info->vSamplingFactorMax == 0)
    {
        XLOGW("SamplingFactorMax = 0 (%d %d) ", info->hSamplingFactorMax, info->vSamplingFactorMax);
        return -1;
    }
    
    // Calculate DU number
    for(i = 0 ; i < info->componentNum ; i++)
    {
        if(info->hSamplingFactor[i] == 0 || info->vSamplingFactor[i] == 0)
        {
            XLOGW("SamplingFactor[%d] = 0 (%d %d) ", i, info->hSamplingFactor[i], info->vSamplingFactor[i]);
            return -1;
        }
        comp_h_count = (info->hSamplingFactorMax / info->hSamplingFactor[i]) * 8;
        comp_v_count = (info->vSamplingFactorMax / info->vSamplingFactor[i]) * 8;
        
        info->duPerMCURow[i] = info->mcuRow * info->hSamplingFactor[i];  
        info->dummyDU[i] = (info->paddedWidth - info->width) * info->hSamplingFactor[i] / 
                            info->hSamplingFactorMax / 8;
        
        info->totalDU[i] = ((info->width + comp_h_count - 1) / comp_h_count) *
                           ((info->height + comp_v_count - 1) / comp_v_count);
    }

    return 0;
} /* parse_jpeg_file() */

