LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	 m4u_ut_thread.cpp

LOCAL_C_INCLUDES:= \
	$(TOP)/$(MTK_PATH_SOURCE)/external/mhal/src/core/drv/6575/inc \
  

LOCAL_SHARED_LIBRARIES := \
  libcutils \
  liblog \
  libmhaldrv \


LOCAL_SHARED_LIBRARIES += \
    libc \

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= ut_thread

include $(BUILD_EXECUTABLE)
