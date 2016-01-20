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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkIF.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkIF.cpp
//! \brief

#define LOG_TAG "ACDK_IF"


//extern "C" {
//#include <linux/fb.h>
//#include <linux/kd.h>
//#include <linux/mtkfb.h>
//}

#include "AcdkLog.h"
#include "AcdkTypes.h"
#include "AcdkErrCode.h"
#include "AcdkCommon.h"

#include "AcdkIF.h"
#include "AcdkCCTFeature.h"
#include "AcdkCCTCtrl.h"
#include "AcdkCalibration.h"
//#include <AcdkCommon.h>

static MBOOL g_bAcdkOpend = FALSE;

using namespace NSACDK; 

static AcdkCCTCtrl *g_pAcdkCCTCtrlObj = NULL; 
static AcdkCalibration *g_pAcdkCalibrationObj = NULL; 

//extern MBOOL bAcdkFeature(UINT32 a_u4Ioctl, ACDK_FEATURE_INFO_STRUCT *a_prFeatureInfo);
//extern MBOOL bAcdkCCTFeature(UINT32 a_u4Ioctl, ACDK_FEATURE_INFO_STRUCT *a_prFeatureInfo);

//#ifdef __cplusplus
//extern "C" {
//#endif

//=====================================================
typedef enum
{
    ACDK_COMMAND_START = 0x80001000,
    ACDK_CMD_PREVIEW_START,
    ACDK_CMD_PREVIEW_STOP,
    ACDK_CMD_CAPTURE,
    ACDK_CMD_QV_IMAGE,
    ACDK_CMD_RESET_LAYER_BUFFER,
    ACDK_CMD_SET_SRC_DEV,
    ACDK_CMD_SET_OPERATION_MODE,
    ACDK_CMD_SET_SHUTTER_TIME,
    ACDK_CMD_GET_SHUTTER_TIME,
    ACDK_CMD_GET_CHECKSUM,
    ACDK_CMD_GET_AF_INFO,
    ACDK_COMMAND_END
}eACDK_COMMAND;

/////////////////////////////////////////////////////////////////////////
//
//   MDK_Open () -
//!  brief ACDK I/F MDK_Open()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL MDK_Open()
{

    if (g_bAcdkOpend == TRUE)
    {
        ACDK_LOGE("ACDK device already opened \n!"); 
        return FALSE; 
    }    

    g_pAcdkCCTCtrlObj = AcdkCCTCtrl::createInstance();    
    g_pAcdkCalibrationObj = new AcdkCalibration(); 
    g_bAcdkOpend = TRUE; 
    
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////
//
//   MDK_Close () -
//!  brief ACDK I/F MDK_Close()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL MDK_Close()
{

    if (g_bAcdkOpend == FALSE)
    {
        ACDK_LOGE("Acdk device is not opened !\n"); 
        return FALSE; 
    }

    g_pAcdkCCTCtrlObj->destroyInstance(); 
    delete g_pAcdkCalibrationObj; 

    g_bAcdkOpend = FALSE; 
    return TRUE; 
}

/////////////////////////////////////////////////////////////////////////
//
//   MDK_Init () -
//!  brief ACDK I/F MDK_Init()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL MDK_Init()
{

    if (g_bAcdkOpend == FALSE)
    {
        ACDK_LOGE("[MDK_Init] ACDK device is not opened \n!"); 
        return FALSE; 
    }

    MRESULT mrRet;   

    mrRet = g_pAcdkCCTCtrlObj->init(); 
    if (FAILED(mrRet))
    {
        ACDK_LOGE("[MDK_Init] Fail to init CamCtrl \n"); 
        return FALSE; 
    }

    mrRet = g_pAcdkCalibrationObj->init(g_pAcdkCCTCtrlObj);
    if (FAILED(mrRet)) {
        ACDK_LOGE("[MDK_Init] Fail to enable AcdkCalibration \n");
        return FALSE;
    }
    
    return TRUE; 
}

/////////////////////////////////////////////////////////////////////////
//
//   MDK_DeInit () -
//!  brief ACDK I/F MDK_DeInit()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL MDK_DeInit()
{
    MINT32 err; 
    if (g_bAcdkOpend == FALSE) {
        ACDK_LOGE("[MDK_DeInit] ACDK device is not opened \n!"); 
        return FALSE; 
    }
      
    err = g_pAcdkCCTCtrlObj->uninit(); 
   if (err != 0) {
        ACDK_LOGE("[MDK_DeInit] Fail to disable ACDK CamCtrl  err = 0x%x\n", err); 
        return FALSE; 
   }

   err = g_pAcdkCalibrationObj->uninit(); 
   if (FAILED(err)) {
        ACDK_LOGE("Fail to disable AcdkCalibration \n");
        return FALSE; 
   }

    return TRUE; 
}


MBOOL ACDKQueryCLICmdCnt(ACDK_FEATURE_INFO_STRUCT *a_prFeatureInfo)
{
    Acdk_CLICmd *pTempCLICmds = NULL; 
    UINT32 u4CLICmdCnt = 0; 
    UINT32 *pu4CLICmdCnt = (UINT32 *)a_prFeatureInfo->puParaOut;    
    MRESULT mrRet;
    
    //Acdk Calibration Cmds
    mrRet = g_pAcdkCalibrationObj->mrQueryCLICmds(&pTempCLICmds, &u4CLICmdCnt);
    (*pu4CLICmdCnt) += u4CLICmdCnt; 

    //Acdk CamCtrl CLI Cmds
    u4CLICmdCnt = 0; 
    mrRet = g_pAcdkCCTCtrlObj->mrQueryCLICmds(&pTempCLICmds,  &u4CLICmdCnt);
    *pu4CLICmdCnt += u4CLICmdCnt;

    *a_prFeatureInfo->pu4RealParaOutLen = 4; 
    return TRUE;     
}

MBOOL  ACDKQueryCLICmds(ACDK_FEATURE_INFO_STRUCT *a_prFeatureInfo)
{
    Acdk_CLICmd *pTempCLICmds = NULL; 
    UINT32 u4CLICmdCnt = 0; 
    Acdk_CLICmd *prCLICmds = (Acdk_CLICmd *)a_prFeatureInfo->puParaOut; 
    UINT32 u4Index = 0; 
    UINT32 i = 0; 
    //Acdk Calibration CLI Cmds
    MRESULT mrRet = g_pAcdkCalibrationObj->mrQueryCLICmds(&pTempCLICmds, &u4CLICmdCnt);
    for (i = 0; i < u4CLICmdCnt; i++)    
    {
         memcpy(&prCLICmds[u4Index], &pTempCLICmds[i], sizeof(Acdk_CLICmd)); 
         u4Index++;
    }

    //Acdk CamCtrl CLI Cmds
    u4CLICmdCnt = 0; 
    pTempCLICmds = NULL; 
    mrRet = g_pAcdkCCTCtrlObj->mrQueryCLICmds(&pTempCLICmds, &u4CLICmdCnt);
    
    for (i = 0; i < u4CLICmdCnt; i++)    
    {
         memcpy(&prCLICmds[u4Index], &pTempCLICmds[i], sizeof(Acdk_CLICmd)); 
         u4Index++;
    }    

    *a_prFeatureInfo->pu4RealParaOutLen = u4CLICmdCnt * sizeof (Acdk_CLICmd); 
    return TRUE; 
}

/////////////////////////////////////////////////////////////////////////
//
//   MDK_IOControl () -
//!  brief ACDK I/F MDK_IOControl()
//!
/////////////////////////////////////////////////////////////////////////
MBOOL MDK_IOControl(UINT32 a_u4Ioctl, ACDK_FEATURE_INFO_STRUCT *a_prAcdkFeatureInfo)
{
    if (g_bAcdkOpend == FALSE)
    {
        ACDK_LOGE("[MDK_IOControl] ACDK device is not opened \n!"); 
        return FALSE; 
    }

    MBOOL bRet = TRUE;
	UINT32  mdk_command = a_u4Ioctl;
	ACDK_FEATURE_INFO_STRUCT 	*t_prAcdkFeatureInfo;
	
		// 	mapping command : 
    //	bSendDataToACDK(ACDK_CMD_GET_AF_INFO,        =====>	 ACDK_CCT_V2_OP_GET_AF_INFO   
    //	bSendDataToACDK(ACDK_CMD_PREVIEW_START,      =====>  ACDK_CCT_OP_PREVIEW_LCD_START
    //	bSendDataToACDK(ACDK_CMD_CAPTURE,            =====>  ACDK_CCT_OP_SINGLE_SHOT_CAPTURE_EX
    //	bSendDataToACDK(ACDK_CMD_PREVIEW_STOP        =====>  ACDK_CCT_OP_PREVIEW_LCD_STOP
    //	bSendDataToACDK(ACDK_CMD_SET_SHUTTER_TIME,   not support : only for auto test
    //	bSendDataToACDK(ACDK_CMD_GET_CHECKSUM,       not support : only for auto test
    	
    ACDK_LOGD("[MDK_IOControl] Reveive IOControl Code original :%x\n", a_u4Ioctl); 
	
	if (a_u4Ioctl == ACDK_CMD_PREVIEW_START)
	{
//		PACDK_PREVIEW_STRUCT pCamPreview = (ACDK_PREVIEW_STRUCT *)a_prAcdkFeatureInfo->puParaIn;
//		ACDK_CCT_CAMERA_PREVIEW_STRUCT *puParaIn; 
//		puParaIn->u2PreviewHeight = pCamPreview->u4PrvH;
//		puParaIn->u2PreviewWidth= pCamPreview->u4PrvW;
//		puParaIn->fpPrvCB= pCamPreview->fpPrvCB;		
		bRet = g_pAcdkCCTCtrlObj->sendcommand (ACDK_CCT_OP_PREVIEW_LCD_START,a_prAcdkFeatureInfo->puParaIn, 0,NULL,0,NULL); 
	}
	else if (a_u4Ioctl == ACDK_CMD_CAPTURE)
	{
//		PACDK_CAPTURE_STRUCT pCapConfig = (ACDK_CAPTURE_STRUCT *)a_prAcdkFeatureInfo->puParaIn;
//		ACDK_CCT_STILL_CAPTURE_STRUCT *puParaIn; 
//		puParaIn->eCameraMode = pCapConfig->eCameraMode;
//		puParaIn->eOutputFormat= pCapConfig->eOutputFormat;	
//		puParaIn->u2JPEGEncHeight= pCapConfig->u2JPEGEncHeight;		
//		puParaIn->u2JPEGEncWidth= pCapConfig->u2JPEGEncWidth;		
//		puParaIn->fpCapCB= pCapConfig->fpCapCB;				
		bRet = g_pAcdkCCTCtrlObj->sendcommand (ACDK_CCT_OP_SINGLE_SHOT_CAPTURE_EX,a_prAcdkFeatureInfo->puParaIn, 0,NULL,0,NULL); 
	}
	else if (a_u4Ioctl == ACDK_CMD_PREVIEW_STOP)
	{
		bRet = g_pAcdkCCTCtrlObj->sendcommand (ACDK_CCT_OP_PREVIEW_LCD_STOP,NULL,0,NULL,0,NULL); 
	}
	else if (a_u4Ioctl == ACDK_CMD_SET_SRC_DEV)
	{
		MINT8 *pSrcDev = (MINT8 *)a_prAcdkFeatureInfo->puParaIn;
		if(*pSrcDev == 1) // main:1, sub: 2
		{ 
			*pSrcDev = 0;
		}
		else if (*pSrcDev == 2)
		{
			*pSrcDev = 1;
		}
		

		bRet = g_pAcdkCCTCtrlObj->sendcommand (ACDK_CCT_FEATURE_SET_SRC_DEV, (MHAL_UINT8 *)pSrcDev, 0,NULL,0,NULL); 
	}       
    else if (a_u4Ioctl >= ACDK_CCT_CDVT_START && a_u4Ioctl < ACDK_CCT_CDVT_END)
    {
        bRet = g_pAcdkCalibrationObj->sendcommand(
                                                        a_u4Ioctl, 
                                                        a_prAcdkFeatureInfo->puParaIn, 
                                                        a_prAcdkFeatureInfo->u4ParaInLen, 
                                                        a_prAcdkFeatureInfo->puParaOut, 
                                                        a_prAcdkFeatureInfo->u4ParaOutLen, 
                                                        a_prAcdkFeatureInfo->pu4RealParaOutLen
                                                       ); 

    }
    else if (a_u4Ioctl == ACDK_CCT_V2_OP_SHADING_CAL)
    {
        bRet = g_pAcdkCalibrationObj->sendcommand(
                                                        a_u4Ioctl, 
                                                        a_prAcdkFeatureInfo->puParaIn, 
                                                        a_prAcdkFeatureInfo->u4ParaInLen, 
                                                        a_prAcdkFeatureInfo->puParaOut, 
                                                        a_prAcdkFeatureInfo->u4ParaOutLen, 
                                                        a_prAcdkFeatureInfo->pu4RealParaOutLen
                                                       ); 
    }
    else if (a_u4Ioctl >= ACDK_CCT_FEATURE_BEGIN && a_u4Ioctl < ACDK_CCT_FEATURE_END)
    {
        bRet = g_pAcdkCCTCtrlObj->sendcommand (
                                                        a_u4Ioctl, 
                                                        a_prAcdkFeatureInfo->puParaIn, 
                                                        a_prAcdkFeatureInfo->u4ParaInLen, 
                                                        a_prAcdkFeatureInfo->puParaOut, 
                                                        a_prAcdkFeatureInfo->u4ParaOutLen, 
                                                        a_prAcdkFeatureInfo->pu4RealParaOutLen
                                                       ); 
    }  
    else if (mdk_command == ACDK_CCT_FEATURE_QUERY_CLI_CMD_CNT) {
       bRet = ACDKQueryCLICmdCnt(a_prAcdkFeatureInfo);
    }
    else if (mdk_command == ACDK_CCT_FEATURE_QUERY_CLI_CMDS) {
        bRet = ACDKQueryCLICmds(a_prAcdkFeatureInfo); 
    }
//    else if (mdk_command == ACDK_CCT_FEATURE_QUERY_AUTO_TEST_NOT_SUPPORT) {
//    		UINT32 *pu4CLICmdCnt = (UINT32 *)a_prFeatureInfo->puParaOut; 
//        (*pu4CLICmdCnt) = 0x0F; 
//    }

    return TRUE; 
}

//#ifdef __cplusplus
//} // extern "C"
//#endif
