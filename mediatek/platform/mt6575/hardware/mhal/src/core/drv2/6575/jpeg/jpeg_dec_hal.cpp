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
*  permission of MediaTek Inc. (C) 2009
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
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "jpeg_drv.h"

#include "jpeg_dec_hal.h"
#include "jpeg_dec_data.h"

#include "camera_sysram.h"

#include <cutils/log.h>
#include <utils/Errors.h>

#include <cutils/xlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "JpegDecDriver"

#define JPEG_DRV_NAME           "/dev/mtk_jpeg"
#define SYSRAM_DEV_NAME         "/dev/camera-sysram"

static int currentID;
static JPEG_FILE_INFO j_info;

static int sys_id;
static unsigned int allocated;

static void free_sysram(void)
{
    stSysramParam sysParams;

	if(allocated != 0)
	{
	    sysParams.u4Alignment = 2048;
        sysParams.u4Owner = ESysramUser_JPEG_CODEC;
        sysParams.u4Size = 4096;
        sysParams.u4TimeoutInMS = 100;
        sysParams.u4Addr = allocated;
        
        ioctl(sys_id, SYSRAM_S_USRFREE, &sysParams);
        allocated = 0;	    
	}    
}
//------------------------------------------------------------------
//
//------------------------------------------------------------------
JPEG_DEC_STATUS_ENUM jpegDecLockDecoder(int* drvID)
{
    *drvID = open(JPEG_DRV_NAME, O_RDONLY, 0);
    if( *drvID == -1 )
    {
        XLOGW("Open JPEG Driver Error (%s)", strerror(errno));
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
    }
    
    if(ioctl(*drvID, JPEG_DEC_IOCTL_INIT))
    {
        XLOGW("JPEG Driver->JPEG_DEC_IOCTL_INIT Error (%s)", strerror(errno));
        close(*drvID);
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
    }

    sys_id = open(SYSRAM_DEV_NAME, O_RDONLY, 0);
    if( sys_id == -1 )
    {
        XLOGW("Open SYSRAM Driver Error (%s)", strerror(errno));
        ioctl(*drvID, JPEG_DEC_IOCTL_DEINIT);
        close(*drvID);
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
    }
    
    currentID = *drvID;
    allocated = 0;
    
    return JPEG_DEC_STATUS_OK;
}


//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
JPEG_DEC_STATUS_ENUM jpegDecUnlockDecoder(int drvID)
{
    free_sysram();
    close(sys_id);
    
    if(ioctl(drvID, JPEG_DEC_IOCTL_DEINIT))
    {
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
    }
    
    close(drvID);
    return JPEG_DEC_STATUS_OK;
}


//--------------------------------------------------------------------
//
//--------------------------------------------------------------------
JPEG_DEC_STATUS_ENUM jpegDecSetSourceFile(int drvID, unsigned char *srcVA, unsigned int srcSize)
{
    int ret;
    unsigned int i, dstPA, tempPA;

    
    //if(drvID != currentID)
    //    return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;

    memset(&j_info, 0, sizeof(JPEG_FILE_INFO));

    //j_info.dstStreamSize = 0;
    j_info.jpg_progressive = -1;
    
    if(jpeg_drv_parse_file(srcVA, srcSize, &j_info) != 0)
    {
        XLOGW("JPEG Driver->jpeg_drv_parse_file Error");
        return JPEG_DEC_STATUS_ERROR_UNSUPPORTED_FORMAT;
    }
    
    if(jpeg_drv_calculate_info(&j_info) != 0)
    {
        XLOGW("JPEG Driver->jpeg_drv_calculate_info Error");
        return JPEG_DEC_STATUS_ERROR_UNSUPPORTED_FORMAT;
    }

    if(j_info.jpg_progressive != 0 || j_info.samplingFormat == 0)
        return JPEG_DEC_STATUS_ERROR_UNSUPPORTED_FORMAT;
        
    //printf(" --Format : %d\n", j_info.samplingFormat);

    
 
	return JPEG_DEC_STATUS_OK;
}


JPEG_DEC_STATUS_ENUM jpegDecSetSourceSize(int drvID, unsigned int srcVA, unsigned int srcSize)
{
    JPEG_DEC_DRV_IN drvParams;
    stSysramParam sysParams;
    
    if( drvID != currentID)
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;  
    
    drvParams.srcStreamAddr = srcVA;
    drvParams.srcStreamSize = srcSize;
    
    //drvParams.dstBufferPA = NULL;
    //drvParams.dstBufferVA = NULL;
    //drvParams.dstBufferSize = j_info.dstStreamSize;

    //drvParams.isPhyAddr = 1;
  /*
    if(needTempBuffer == 1)
    {
        j_info.tempBufferSize= dstBufferSize * 3 / 4;

        drvParams.needTempBuffer = 1;
        drvParams.tempBufferSize = j_info.tempBufferSize;
        drvParams.tempBufferPA = &tempPA;
    }
  */ 
    //drvParams.needTempBuffer = 0;
    
    drvParams.mcuRow = j_info.mcuRow;
    drvParams.mcuColumn = j_info.mcuColumn;
    drvParams.samplingFormat = j_info.samplingFormat;
    drvParams.componentNum = j_info.componentNum;

    for(unsigned int i = 0 ; i < j_info.componentNum ; i++)
    {
        drvParams.componentID[i] = j_info.componentID[i];
        drvParams.hSamplingFactor[i] = j_info.hSamplingFactor[i];
        drvParams.vSamplingFactor[i] = j_info.vSamplingFactor[i];
        drvParams.qTableSelector[i] = j_info.qTableSelector[i];

        drvParams.totalDU[i] = j_info.totalDU[i];
        drvParams.dummyDU[i] = j_info.dummyDU[i];
        drvParams.duPerMCURow[i] = j_info.duPerMCURow[i];
    }

    sysParams.u4Alignment = 2048;
    sysParams.u4Owner = ESysramUser_JPEG_CODEC;
    sysParams.u4Size = 4096;
    sysParams.u4TimeoutInMS = 100;
    
    if(ioctl(sys_id, SYSRAM_X_USRALLOC_TIMEOUT, &sysParams) != 0)
    {
        return JPEG_DEC_STATUS_ERROR_INVALID_FILE;
    }

    allocated = sysParams.u4Addr;
    drvParams.table_addr = sysParams.u4Addr;
    
    if(ioctl(drvID, JPEG_DEC_IOCTL_CONFIG, &drvParams) != 0)
    {
        ioctl(sys_id, SYSRAM_S_USRFREE, &sysParams);
        allocated = 0;
	    return JPEG_DEC_STATUS_ERROR_INVALID_FILE;
    }
 
	return JPEG_DEC_STATUS_OK;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
/*
JPEG_DEC_STATUS_ENUM jpegDecSetRange(int drvID, int* top, int* bottom)
{
    JPEG_DEC_RANGE_IN rangeParams;
    int mcu_row_index;
    int mcu_height;
    if( drvID != currentID)
    {
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
    }   

    if(j_info.mcuRow * j_info.mcuColumn > 65535)
    {
        return JPEG_DEC_STATUS_ERROR_UNSUPPORTED_FORMAT;
    }
    
    mcu_row_index = j_info.mcuRow - 1;
    mcu_height = j_info.paddedHeight / j_info.mcuColumn;

    if((*top) % mcu_height != 0)
        (*top) -= ((*top) % mcu_height);

    if((*bottom) % mcu_height != 0)
        (*bottom) -= ((*bottom) % mcu_height);
        
    rangeParams.decID = decLockID;
    rangeParams.startIndex = (*top) / mcu_height * j_info.mcuRow;
    rangeParams.endIndex = (*bottom) / mcu_height * j_info.mcuRow - 1;
    rangeParams.skipIndex1 = 0;
    rangeParams.skipIndex2 = mcu_row_index;
    rangeParams.idctNum = mcu_row_index ;
    
    if(ioctl(drvID, JPEG_DEC_IOCTL_RANGE, &rangeParams))
    {
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
    }

    return JPEG_DEC_STATUS_OK;
}
*/
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
JPEG_DEC_STATUS_ENUM jpegDecGetFileInfo(int drvID, JPEG_FILE_INFO_IN *fileInfo)
{
    //if( drvID != currentID)
    //    return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
        
    fileInfo->width = j_info.width;
    fileInfo->height = j_info.height;

    fileInfo->padded_width = j_info.paddedWidth;
    fileInfo->padded_height = j_info.paddedHeight;

    //fileInfo->dst_buffer_pa = j_info.dstStreamAddr;
    //fileInfo->temp_buffer_pa = j_info.tempBufferAddr;

    fileInfo->total_mcu = j_info.mcuRow * j_info.mcuColumn;

    fileInfo->samplingFormat = j_info.samplingFormat;
    
    fileInfo->componentNum = j_info.componentNum;
    fileInfo->y_hSamplingFactor = j_info.hSamplingFactor[0];
    fileInfo->y_vSamplingFactor = j_info.vSamplingFactor[0];
    fileInfo->u_hSamplingFactor = j_info.hSamplingFactor[1];
    fileInfo->u_vSamplingFactor = j_info.vSamplingFactor[1];
    fileInfo->v_hSamplingFactor = j_info.hSamplingFactor[2];
    fileInfo->v_vSamplingFactor = j_info.vSamplingFactor[2];

    return JPEG_DEC_STATUS_OK;
}

//---------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------
JPEG_DEC_STATUS_ENUM jpegDecStart(int drvID, long timeout, JPEG_DEC_RESULT_ENUM *result)
{
    int ret;
    int dec_drv_result;
    unsigned int drv_result;
    JPEG_DEC_DRV_OUT drvParams;
    
    if( drvID != currentID)
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;

    // trigger decoder hw
    ret = ioctl(drvID, JPEG_DEC_IOCTL_START);
	if(0 != ret)
	{
	    free_sysram();
	    return JPEG_DEC_STATUS_ERROR_INVALID_FILE;
	}

	// wait decode result
	//drvParams.decID = decLockID;
	drvParams.result = &drv_result;
	drvParams.timeout = timeout;
	ret = ioctl(drvID, JPEG_DEC_IOCTL_WAIT, &drvParams);
    free_sysram();
	if(0 != ret)
	    return JPEG_DEC_STATUS_ERROR_INVALID_FILE;

    switch (drv_result)
    {
        case 0:
	        *result = JPEG_DEC_RESULT_DONE;
	        break;
	    case 1:
	        *result = JPEG_DEC_RESULT_PAUSE;
	        break; 
	    case 2:
	        *result = JPEG_DEC_RESULT_OVERFLOW;
	        break;
	    case 3:
	        *result = JPEG_DEC_RESULT_HALT;
	        break;
    }
    return JPEG_DEC_STATUS_OK;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
/*
JPEG_DEC_STATUS_ENUM jpegDecResume(int drvID, unsigned int srcFileAddr, unsigned int srcFileSize, 
                                   unsigned char isPhyAddr, long timeout, JPEG_DEC_RESULT_ENUM *result)
{
    int ret;
    int dec_drv_result;
    unsigned int drv_result;
    JPEG_DEC_RESUME_IN reParams;
    JPEG_DEC_DRV_OUT drvParams;
    
    if( drvID != currentID)
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;

    // resume decoder hw
    reParams.decID = decLockID;
    reParams.isPhyAddr = isPhyAddr;
    reParams.srcStreamAddr =  srcFileAddr;
    reParams.srcStreamSize = srcFileSize;
    
    ret = ioctl(drvID, JPEG_DEC_IOCTL_RESUME, &reParams);
	if(0 != ret)
	    return JPEG_DEC_STATUS_ERROR_INVALID_FILE;

	// wait decode result
	drvParams.decID = decLockID;
	drvParams.result = &drv_result;
	drvParams.timeout = timeout;
	ret = ioctl(drvID, JPEG_DEC_IOCTL_WAIT, &drvParams);
	if(0 != ret)
	    return JPEG_DEC_STATUS_ERROR_INVALID_FILE;

	switch (drv_result)
    {
        case 0:
	        *result = JPEG_DEC_RESULT_DONE;
	        break;
	    case 1:
	        *result = JPEG_DEC_RESULT_PAUSE;
	        break; 
	    case 2:
	        *result = JPEG_DEC_RESULT_OVERFLOW;
	        break;
	    case 3:
	        *result = JPEG_DEC_RESULT_HALT;
	        break;
    }
	
    return JPEG_DEC_STATUS_OK;
}

JPEG_DEC_STATUS_ENUM jpegDecGetData(int drvID)
{
    int ret;

    if( drvID != currentID)
        return JPEG_DEC_STATUS_ERROR_CODEC_UNAVAILABLE;
        
    ret = ioctl(drvID, JPEG_DEC_IOCTL_COPY, &decLockID);
	if(0 != ret)
	    return JPEG_DEC_STATUS_ERROR_INVALID_FILE;

     return JPEG_DEC_STATUS_OK;	    
}
*/
