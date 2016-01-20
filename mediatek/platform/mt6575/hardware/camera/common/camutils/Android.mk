LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

#-----------------------------------------------------------
LOCAL_SRC_FILES := CameraProfile.cpp

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common/camutils

#-----------------------------------------------------------
ifeq "yes" "$(strip $(MTK_MMPROFILE_SUPPORT))"
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
LOCAL_SHARED_LIBRARIES += libmmprofile
endif

LOCAL_SHARED_LIBRARIES += liblog

LOCAL_MODULE := libcam.utils

#-----------------------------------------------------------
include $(BUILD_SHARED_LIBRARY)

################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

