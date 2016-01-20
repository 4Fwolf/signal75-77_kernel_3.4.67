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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _APP_FDVT_H
#define _APP_FDVT_H

#include "MTKDetectionCommon.h"
#include "MTKDetection.h"

#if defined(MTK_M4U_SUPPORT)
//#undef MTK_M4U_SUPPORT
#endif 

struct ImageInfo
{
    MUINT32 *pAddr;
    MUINT16 Weight;
    MUINT16 Height;
};

struct fd_info
{ 
    // Search range
    int     x0          ;   //   9 bit        
    int     y0          ;   //   8 bit
    int     x1          ;   //   9 bit    
    int     y1          ;   //   8 bit
            
    // Direction information     
    long long pose      ;   //  60 bit (0-11: ROP00, 12-23: ROP+50, 24-35: ROP-50, 36-47: ROP+90, 48-59: ROP-90)

}; 

struct    tracking_info
{
    // Active window valid information
    int     valid       ;   //   1 bit
    int     face_index  ;   //   9 bit
 
    // Search range
    int     x0          ;   //   9 bit
    int     y0          ;   //   8 bit
    int     x1          ;   //   9 bit    
    int     y1          ;   //   8 bit    
 
    // LFD detection size indicator
    int     size_index     ;   //   5 bit
    
    // LFD detected direction indicator
    int     rip_dir     ;   //   4 bit
    int     rop_dir     ;   //   3 bit (0/1/2/3/4/5 = ROP00/ROP+50/ROP-50/ROP+90/ROP-90)
};


struct tc_result
{
     int     face_index  ;   //   9 bit
     int     type        ;   //   1 bit  0/1 = FD/LFD
                             
     int     LeftR       ;   //   8 bit
     int     LeftG       ;   //   8 bit
     int     LeftB       ;   //   8 bit
                             
     int     CenterR     ;   //   8 bit
     int     CenterG     ;   //   8 bit
     int     CenterB     ;   //   8 bit
                             
     int     RightR      ;   //   8 bit
     int     RightG      ;   //   8 bit
     int     RightB      ;   //   8 bit
};
 
#if defined(MTK_M4U_SUPPORT)    
typedef struct fdvtM4UInfo_t {
    MUINT32 virtAddr; 
    MUINT32 useM4U; 
    MUINT32 M4UVa; 
    MUINT32 size; 
}FDVTM4UInfo; 
#endif 

typedef struct
{
	FDBOOL af_face_indicator ;
	FDBOOL display_flag;
	MINT32 face_display_pos_x0;
	MINT32 face_display_pos_y0;	
	MINT32 face_display_pos_x1;
	MINT32 face_display_pos_y1;
} fd_result_struct;
 
class AppFDVT : public MTKDetection {
public:
    int FaceCnt;
    
    static MTKDetection* getInstance();
    virtual void destroyInstance();
    
    AppFDVT();
    virtual ~AppFDVT();
    void FDVTInit(MUINT32 *tuning_data);
    void FDVTMain(MUINT32 image_buffer_address, FDVT_OPERATION_MODE_ENUM fd_state);
    void FDVTReset(void);
    MUINT32 FDVTGetResultSize(void);
    MUINT8 FDVTGetResult(MUINT32 FD_result_Adr);
    void FDVTDrawFaceRect(MUINT32 image_buffer_address,MUINT32 Width,MUINT32 Height,MUINT32 OffsetW,MUINT32 OffsetH,MUINT8 orientation);
    #ifdef SmileDetect
    MUINT8 FDVTGetSDResult(MUINT32 FD_result_Adr);
    void FDVTSDDrawFaceRect(MUINT32 image_buffer_address,MUINT32 Width,MUINT32 Height,MUINT32 OffsetW,MUINT32 OffsetH,MUINT8 orientation);
    #endif
private:
#if defined(MTK_M4U_SUPPORT)    
    FDVTM4UInfo mWorkingBuffer;
    FDVTM4UInfo mLearningData;
    //ISPM4UInfo mIspWritePort; 
    void SyncM4UCache(FDVTM4UInfo m4uinfo);
    MINT32 allocM4UMemory(MUINT32 virtAddr, MUINT32 size, MUINT32 *m4uVa);
    MINT32 freeM4UMemory(MUINT32 m4uVa, MUINT32 size,MUINT32 m4uMVa);
    void InitialDRAM();  
#endif    
    
};

#endif

