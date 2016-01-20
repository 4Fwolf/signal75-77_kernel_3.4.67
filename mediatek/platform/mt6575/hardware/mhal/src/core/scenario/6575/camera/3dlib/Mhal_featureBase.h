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

#ifndef _MHAL_3D_LIB_H
#define _MHAL_3D_LIB_H


/*******************************************************************************
*
********************************************************************************/
#define MY_DBG(fmt, arg...)         XLOGD("[%s]"fmt, getFeatureName(), ##arg)
#define MY_INFO(fmt, arg...)        XLOGI("[%s]"fmt, getFeatureName(), ##arg)
#define MY_WARN(fmt, arg...)        XLOGW("[%s]"fmt, getFeatureName(), ##arg)
#define MY_ERR(fmt, arg...)         XLOGE("[%s]"fmt, getFeatureName(), ##arg)


/*******************************************************************************
*
*******************************************************************************/

typedef enum state_s{
	MHAL_CAM_FEATURE_STATE_IDLE,
	MHAL_CAM_FEATURE_STATE_CAPTURE, 	 
	MHAL_CAM_FEATURE_STATE_MERGE,
	MHAL_CAM_FEATURE_STATE_MERGE_DONE,
	MHAL_CAM_FEATURE_STATE_UNINIT
}state_e;


/*******************************************************************************
*
*******************************************************************************/

typedef enum err_s{
   ERR_NO_MEMORY = 1000,
   ERR_RESET
}err_e;

/*******************************************************************************
*
*******************************************************************************/

class mhal_featureBase
{
public:
	mhal_featureBase(const char *fName):featureName(fName){};
	virtual ~mhal_featureBase() {};

protected:
	volatile state_e  eState;
	const char*       featureName;

public:    ////  Attribute
    virtual MINT32    getState() const              { return eState; }
    virtual MVOID     setState(MUINT32 state)       { eState = (state_e)state; }
	virtual MBOOL	  getFrame() const              { return getState() == MHAL_CAM_FEATURE_STATE_CAPTURE? true : false; }		
	virtual const char* getFeatureName() const      { return featureName; }

public:    ////  Interface for public usage
	virtual MVOID     destroyInstance() = 0;
    virtual MINT32    mHalCamFeatureInit(mhalCamParam_t *pmhalCamParam) = 0;
    virtual MVOID     mHalCamFeatureUninit() = 0;
	virtual MINT32    mHalCamFeatureProc(MUINT32 frameIndex) = 0;
	virtual MINT32    mHalCamFeatureCompress() = 0;


public:    ////  Interface for private usage
	virtual MINT32    mHalCamFeatureMerge() = 0;
	virtual MINT32    ISShot(MVOID *arg1, MBOOL &shot) = 0;
	virtual MINT32    setParameters(mhalCamParam_t *pmhalCamParam) = 0;
	

};


#endif


