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
# libmhaldrv
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libcameracustom \
    libutilitysw \

LOCAL_SHARED_LIBRARIES += \
    libnvram \

#matv define
#ifeq ($(HAVE_MATV_FEATURE),yes)
#ifeq ($(MTK_ATV_CHIP), $(filter $(MTK_ATV_CHIP),MTK_MT5192 MTK_MT5193))
    LOCAL_STATIC_LIBRARIES := 
#   LOCAL_SHARED_LIBRARIES += libmedia libmatv_cust
    LOCAL_SHARED_LIBRARIES += libmatv_cust
    LOCAL_SHARED_LIBRARIES += \
	    libutils

#	    libbinder
#endif
#endif

LOCAL_SHARED_LIBRARIES += libcam.utils

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \
    libmhaldrv_plat \
    libmhaldrv_comm \

ifeq ($(MTK_PLATFORM),MT6577)
LOCAL_CFLAGS += -DMT6577
endif
ifeq ($(MTK_PLATFORM),MT6575)
LOCAL_CFLAGS += -DMT6575
endif

LOCAL_MODULE := libmhaldrv

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY) 

include $(call all-makefiles-under,$(LOCAL_PATH))


