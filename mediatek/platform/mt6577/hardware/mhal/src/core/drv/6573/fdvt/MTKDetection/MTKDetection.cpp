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

#define LOG_TAG "MTKDetection"
#include "MTKDetection.h"
#include "AppFD.h"
#include "AppFDVT.h"
#include <utils/Log.h>

MTKDetection* 
MTKDetection::createInstance(DrvFDObject_e eobject)
{
    if (eobject == DRV_FD_OBJ_SW) {
        return AppFD::getInstance();
    }
    else if (eobject == DRV_FD_OBJ_HW) {
        return AppFDVT::getInstance();
    }
    else {
        return AppFDTmp::getInstance();
    }

    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
MTKDetection*
AppFDTmp::
getInstance()
{
    LOGD("[halFDTmp] getInstance \n");
    static AppFDTmp singleton;
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void   
AppFDTmp::
destroyInstance() 
{
}


void MTKDetection::FDVTInit(MUINT32 *fd_tuning_data)
{
    LOGD("MTKDetection FDVTInit \n");
}

void MTKDetection::FDVTMain(MUINT32 image_buffer_address, FDVT_OPERATION_MODE_ENUM fd_state)
{
    LOGD("MTKDetection FDVTMain \n");
}

void MTKDetection::FDVTReset(void)
{
    LOGD("MTKDetection FDVTReset \n");
}

MUINT32 MTKDetection::FDVTGetResultSize(void)
{
    LOGD("MTKDetection FDVTGetResultSize \n");
    return 0;
}
MUINT8 MTKDetection::FDVTGetResult(MUINT32 FD_result_Adr)
{
    LOGD("MTKDetection FDVTGetResult \n");
    return 0;
}    
void MTKDetection::FDVTDrawFaceRect(MUINT32 image_buffer_address,MUINT32 Width,MUINT32 Height,MUINT32 OffsetW,MUINT32 OffsetH,MUINT8 orientation)
{
    LOGD("MTKDetection FDVTGetResult \n");
} 
#ifdef SmileDetect
void MTKDetection::FDVTSDDrawFaceRect(MUINT32 image_buffer_address,MUINT32 Width,MUINT32 Height,MUINT32 OffsetW,MUINT32 OffsetH,MUINT8 orientation)
{
    LOGD("MTKDetection FDVTSDDrawFaceRect \n");        
} 
MUINT8 MTKDetection::FDVTGetSDResult(MUINT32 FD_result_Adr)
{
    return 0;
}
#endif