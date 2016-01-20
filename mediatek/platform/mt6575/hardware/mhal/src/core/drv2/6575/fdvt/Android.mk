#
# libfd
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

#LOCAL_PRELINK_MODULE:=false

LOCAL_SRC_FILES:= \
    MTKDetection/MTKDetection.cpp \
    fdvt/AppFDVT.cpp \
    fdvt/fdvt_driver.cpp \

#LOCAL_LDLIBS += lpthread

# For MTKDetection.h 
LOCAL_C_INCLUDES += \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \

LOCAL_C_INCLUDES += \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/fdvt/MTKDetection \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv2/6575/fdvt/fdvt \
    $(MTK_PATH_PLATFORM)/hardware//mhal/src/core/drv/6575/fdvt/fdvt \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/fdvt/fd \
    $(MTK_PATH_PLATFORM)/kernel/core/include/mach \

#MediaHal.h 
LOCAL_C_INCLUDES += \
    $(MTK_PATH_PLATFORM)/hardware/mhal/inc \
    $(MTK_PATH_PLATFORM)/hardware/ \


LOCAL_SHARED_LIBRARIES:= libui liblog libcutils

LOCAL_MODULE:= libfd

include $(BUILD_STATIC_LIBRARY)


