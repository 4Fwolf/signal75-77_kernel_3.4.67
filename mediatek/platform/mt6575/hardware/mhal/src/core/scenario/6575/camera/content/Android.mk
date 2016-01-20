#
# libmhalcontent
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  ContentManager.cpp \

#
# mhal/core
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/inc
#
# scenario
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/common/inc


LOCAL_SHARED_LIBRARIES := \
  liblog \


LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE := libmhalcontent

LOCAL_PRELINK_MODULE:=false

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
