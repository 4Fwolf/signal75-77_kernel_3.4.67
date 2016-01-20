#
# libfd
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

#LOCAL_PRELINK_MODULE:=false

LOCAL_SRC_FILES:= \
    MTKDetection/MTKDetection.cpp \
    fdvt/AppFDVT.cpp \
    fdvt/fdvt_driver.cpp 

#LOCAL_LDLIBS += lpthread

LOCAL_C_INCLUDES += \
    $(MTK_ROOT)/external/mhal/src/core/drv/6573/inc \
    $(MTK_ROOT)/external/mhal/src/core/drv/6573/fdvt/MTKDetection \
    $(MTK_PATH_SOURCE)/external/mhal/src/core/drv2/6573/fdvt/fdvt \
    $(MTK_PATH_SOURCE)/external/mhal/src/core/drv2/6573/fdvt/fd \
    $(MTK_PATH_PLATFORM)/kernel/core/include/mach 

LOCAL_SHARED_LIBRARIES:= libui liblog libcutils

LOCAL_MODULE:= libfd

include $(BUILD_STATIC_LIBRARY)


