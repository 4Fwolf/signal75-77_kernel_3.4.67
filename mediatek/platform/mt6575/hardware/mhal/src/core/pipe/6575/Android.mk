# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.



#===============================================================================


LOCAL_PATH:= $(call my-dir)
################################################################################
# libmhalpipe_plat
################################################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  aaa/aaa_hal_base.cpp \
  aaa/aaa_hal.cpp \
  aaa/aaa_hal_yuv.cpp \
  aaa/aaa_cct_feature.cpp \
  eis/eis_hal.cpp \
  n3d/n3d_hal.cpp \
  flicker/flicker_hal.cpp \
  res_mgr/res_mgr_hal.cpp \
  fdvt/fd_hal_base.cpp \
  fdvt/fdvt/fdvt_hal.cpp \
  fdvt/fd/fd_hal.cpp \
  cct_if/cct_if.cpp \
  cct_if/cct_feature.cpp \
  cct_if/nvram_cct_feature.cpp \
  cct_if/sensor_cct_feature.cpp \
  cct_if/isp_cct_feature.cpp \
  cameraio/isp_hal.cpp \
  cameraio/mdp_hal.cpp \
  cameraio/overlay_hal.cpp \
  cameraio/CameraIO75.cpp \
  jpegio/jpeg_enc_pipe.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature \
  $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/jpeg \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/fdvt/fdvt \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/fdvt/fd \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/fdvt/MTKDetection \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/eis \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/fdvt/fd \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/fdvt/fdvt \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/eis \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/n3d \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/flicker \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
  $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/isp \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/custom/inc/camera_feature \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/videoio/export  \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/videoio \
  $(MTK_ROOT)/kernel/drivers/video \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/inc \
  $(MTK_ROOT)/kernel/include/linux \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/vcodec/include  \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc/mdp \

# cct_feature.h 
LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/cameradebug/inc \

#MediaHal.h & cmaera header file in Mhal\inc
LOCAL_C_INCLUDES += \
  $(MTK_ROOT)/external/ \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc/camera \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/

#algorithm 
LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libn3d/MT6575 \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libeis/MT6575 \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/lib3a/inc/6575 \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libflicker \

# MTKDetection.h
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \
  

LOCAL_C_INCLUDES += $(MTK_ROOT)/external

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common/camutils 

# Property
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/libutility/property

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \
  libmhalpipe_isp_tuning \
  libmhalpipe_cam_feature \

LOCAL_MODULE := libmhalpipe_plat

include $(BUILD_STATIC_LIBRARY)


################################################################################
# libmhalpipe
################################################################################
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    liblog \
    libcameracustom \
    libcamalgo \
    libmhaldrv \
    libmhalmdp \

LOCAL_SHARED_LIBRARIES += \
    libeis \

# property
LOCAL_SHARED_LIBRARIES += libmhalutility

#LOCAL_SHARED_LIBRARIES += \
#    libvcodec_utility \
#    libvcodecdrv \

LOCAL_SHARED_LIBRARIES += libcam.utils

LOCAL_WHOLE_STATIC_LIBRARIES := \
    libmhalpipe_comm \
    libmhalpipe_plat \

LOCAL_MODULE := libmhalpipe

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))


#===============================================================================

