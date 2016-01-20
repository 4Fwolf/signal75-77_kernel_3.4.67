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
// AcdkBase.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkBase.cpp
//! \brief
#define LOG_TAG "ACDK_BASE"

#include "AcdkTypes.h"
#include "AcdkLog.h"
#include "AcdkErrCode.h"
#include "AcdkBase.h"


//////////////////////////////////////////////////////////////////////////
//  mrParseCLICmds() - 
//! @brief parse the CLI Cmds that regist in the cli cmd list 
//! @param a_cCmd: The input CLI command 
//! @param a_u4Argc: The input parameter count
//! @param a_pprArgv: The input parameters 
//////////////////////////////////////////////////////////////////////////
MRESULT AcdkBase::mrParseCLICmds(MUINT8 *a_cCmd,  MUINT32 a_u4Argc, MUINT8 *a_pprArgv[])
{
    if (m_pAcdkCLICmds == NULL)
    {
        return E_ACDK_BASE_NULL_OBJ; 
    }
    ACDK_LOGD("[mrParseCLICmds] \n");

    MINT8 i4Index = 0;
    MBOOL bFoundCmdFlag = FALSE;

    while (m_pAcdkCLICmds[i4Index].pucCmdStr != NULL)
    {
        if (strcmp((char *)m_pAcdkCLICmds[i4Index].pucCmdStr, (char*)a_cCmd) == 0)
        {
            bFoundCmdFlag = TRUE;
            break;
        }
        i4Index++;
    }

    MRESULT mrRet = S_ACDK_BASE_OK;
    if (bFoundCmdFlag)
    {
        mrRet = m_pAcdkCLICmds[i4Index].handleCmd(a_u4Argc, a_pprArgv);
    }
    else
    {
        ACDK_LOGD(" Null Cmd Received\n");
        mrRet = S_ACDK_BASE_OK; 
    }
    return mrRet;
}


//////////////////////////////////////////////////////////////////////////
//  mrQueryCLICmds() - 
//! @brief query the supported cli command 
//! @param a_pprCLICmds: output the supported cli command 
//! @param a_pu4CLICmdCnt: output the supported cli command count 
//////////////////////////////////////////////////////////////////////////
MRESULT AcdkBase::mrQueryCLICmds(Acdk_CLICmd **a_pprCLICmds, MUINT32 *a_pu4CLICmdCnt)
{

    ACDK_LOGD("mrQueryCLICmds \n"); 
    if (m_pAcdkCLICmds == NULL)
    {
        ACDK_LOGE("Null ACDK Cmds \n"); 
        *a_pprCLICmds = NULL; 
        return E_ACDK_BASE_NULL_OBJ; 
    }
    
    MUINT32 u4CLICnt = 0; 

    while (m_pAcdkCLICmds[u4CLICnt].pucCmdStr != NULL)
    {
        u4CLICnt++; 
    }    
    *a_pu4CLICmdCnt = u4CLICnt; 
    *a_pprCLICmds = m_pAcdkCLICmds;
    ACDK_LOGD("ACDK CLI Cmds Cnt:%d\n", u4CLICnt); 
    return S_ACDK_BASE_OK; 
}


