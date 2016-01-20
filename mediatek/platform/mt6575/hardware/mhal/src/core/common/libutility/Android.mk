#
# libmhalutility
#
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    mhal_misc.cpp
    
LOCAL_C_INCLUDES := \
    system/core/include/cutils \
    $(MTK_PATH_SOURCE)/external/mhal \
    $(MTK_PATH_PLATFORM)/kernel/core/include/mach

LOCAL_SHARED_LIBRARIES:= libutils libcutils liblog

LOCAL_WHOLE_STATIC_LIBRARIES := libmhalutility_property

LOCAL_MODULE:= libmhalutility

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))
