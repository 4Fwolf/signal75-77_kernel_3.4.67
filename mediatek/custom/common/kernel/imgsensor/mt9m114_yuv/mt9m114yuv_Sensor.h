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
*  permission of MediaTek Inc. (C) 2008
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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   mt9m114yuv_Sensor.h
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 *   Header file of Sensor driver
 *
 *
 * Author:
 * -------
 *
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 * 
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#ifndef __MT9M114YUV_SENSOR_H
#define __MT9M114YUV_SENSOR_H

//<2012/09/28-14462-alberthsiao,update mt9m114 driver for hawk40
#define MT9M114_WRITE_ID_1                (0xBA)  //Abico
#define MT9M114_READ_ID_1                 (0xBB)
#define MT9M114_WRITE_ID_2                (0x90)  //LiteOn
#define MT9M114_READ_ID_2                 (0x91)
//>2012/09/28-14462
//<2012/11/28 alberthsiao-17860,camera quality tuning for Hawk40
#define NONE_MTK_CAMERA_SETTING 
//<2012/12/3 alberthsiao-18115,modify front camera preview size for Hawk40
/* sensor size */
#ifdef NONE_MTK_CAMERA_SETTING
#define MT9M114_IMAGE_SENSOR_FULL_WIDTH         (1280)
#define MT9M114_IMAGE_SENSOR_FULL_HEIGHT        (960)
#define MT9M114_IMAGE_SENSOR_PV_WIDTH           (1280)
#define MT9M114_IMAGE_SENSOR_PV_HEIGHT          (960)
#else
#define MT9M114_IMAGE_SENSOR_FULL_WIDTH         (1280)
#define MT9M114_IMAGE_SENSOR_FULL_HEIGHT        (720)
#define MT9M114_IMAGE_SENSOR_PV_WIDTH           (1280)
#define MT9M114_IMAGE_SENSOR_PV_HEIGHT          (720)
#endif
//>2012/12/3 alberthsiao-18115
#define MT9M114_FULL_START_X                    2
#define MT9M114_FULL_START_Y                    2

#define MT9M114_PV_START_X                      2
#define MT9M114_PV_START_Y                      2

void MT9M114_Abico_InitialSetting(void);
void MT9M114_Abico_Preview(void);
void MT9M114_Abico_Capture(void);
void MT9M114_Abico_WB_AWB(void);
void MT9M114_Abico_WB_D65(void);
void MT9M114_Abico_WB_CWF(void);
void MT9M114_Abico_WB_TL84(void);
void MT9M114_Abico_WB_A(void);
void MT9M114_Abico_WB_D50(void);
void MT9M114_Abico_WB_H(void);
void MT9M114_Abico_Anti_Banding_50HZ(void);
void MT9M114_Abico_Anti_Banding_60HZ(void);

void MT9M114_LiteOn_InitialSetting(void);
void MT9M114_LiteOn_Preview(void);
void MT9M114_LiteOn_Capture(void);
void MT9M114_LiteOn_WB_AWB(void);
void MT9M114_LiteOn_WB_D65(void);
void MT9M114_LiteOn_WB_CWF(void);
void MT9M114_LiteOn_WB_TL84(void);
void MT9M114_LiteOn_WB_A(void);
void MT9M114_LiteOn_WB_D50(void);
void MT9M114_LiteOn_WB_H(void);
void MT9M114_LiteOn_Anti_Banding_50HZ(void);
void MT9M114_LiteOn_Anti_Banding_60HZ(void);
//>2012/11/28 alberthsiao-17860
#endif /* __MT9M114YUV_SENSOR_H */
