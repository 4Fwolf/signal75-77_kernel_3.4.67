# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


#
# libmhaldrv_plat
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(MTK_PLATFORM),$(filter $(MTK_PLATFORM),MT6575 MT6575T MT6577))

LOCAL_SRC_FILES:= \
    m4u/m4u_lib.cpp \
    isp/isp_drv.cpp \
    isp/feature_raw.cpp \
    eis/eis_drv.cpp \
    isp/sysram/sysram_mgr.cpp \
    isp/sysram/sysram_drv_imp.cpp \
    nvram/nvram_drv.cpp \
    nvram/nvram_buf_mgr.cpp \
    eeprom/eeprom_drv.cpp \
    eeprom/camera_common_calibration.cpp \
    res_mgr/res_mgr_drv.cpp \
    jpeg/jpeg_dec_hal.cpp \
    jpeg/jpeg_dec_parse.cpp \
    jpeg/jpeg_enc_hal.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature \
  $(TOP)/mediatek/kernel/core/include/mach \
  $(TOP)/mediatek/kernel/include/linux \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv2/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
  $(TOP)/$(MTK_PATH_SOURCE)/kernel/include/linux \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv2/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv2/6575/isp \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv2/6575/eis \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/jpeg \
  $(TOP)/$(MTK_PATH_PLATFORM)/kernel/drivers/jpeg \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libeis/MT6575 \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc/mdp \
  $(TOP)/$(MTK_PATH_SOURCE)/kernel/include/linux \
  
LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc \
  $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature \
  $(MTK_PATH_CUSTOM)/hal/camera \
  $(TOP)/$(MTK_PATH_SOURCE)/external/nvram/libnvram \
  $(MTK_PATH_CUSTOM)/hal/inc/aaa \
  $(TOP)/$(MTK_PATH_SOURCE)/kernel/drivers/video \
  $(TOP)/$(MTK_PATH_SOURCE)/kernel/drivers/hdmitx \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \
    libfd \

LOCAL_MODULE:= libmhaldrv_plat

include $(BUILD_STATIC_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))

endif
