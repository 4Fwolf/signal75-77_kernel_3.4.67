#
# libcameracustom
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

# force libcameracustom.so to re-build, due to change sensor will not 
# change any source code. 
#$(shell touch $(TOP)/$(MTK_PATH_SOURCE)/external/mhal/src/custom/common/hal/imgsensor/$(MTK_PROJECT)/sensorlist.cpp)

#$(call config-custom-folder,common: inc:hal/inc)

hal_folder := $(call to-root,$(LOCAL_PATH))/$(MTK_PATH_CUSTOM)/hal
LOCAL_SRC_FILES += $(call all-c-cpp-files-under, $(hal_folder)/camera)
LOCAL_SRC_FILES += $(call all-c-cpp-files-under, $(hal_folder)/flashlight)
LOCAL_SRC_FILES += $(call all-c-cpp-files-under, $(hal_folder)/imgsensor)
LOCAL_SRC_FILES += $(call all-c-cpp-files-under, $(hal_folder)/lens)
LOCAL_SRC_FILES += $(call all-c-cpp-files-under, $(hal_folder)/display/display_isp_tuning)
LOCAL_SRC_FILES += $(call all-c-cpp-files-under, $(hal_folder)/eeprom)

#LOCAL_SRC_FILES += $(call all-c-cpp-files-under, common/hal/camera/camera/$(call lc,$(MTK_PLATFORM)))
#LOCAL_SRC_FILES += $(call all-c-cpp-files-under, \
#   common/hal/camera  \
#   common/hal/flashlight  \
#   common/hal/imgsensor  \
#   common/hal/lens  \
#   common/hal/display/display_isp_tuning \
#   common/hal/eeprom)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/camera/inc
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/camera \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/flashlight \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/imgsensor \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/lens \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/eeprom \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/kernel/sensor/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/kernel/eeprom/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \
    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/display_isp_tuning \
    $(TOP)/$(MTK_PATH_CUSTOM)/cgen/cfgfileinc \

LOCAL_SHARED_LIBRARIES:= liblog libcutils

LOCAL_MODULE:= libcameracustom

LOCAL_MODULE_TAGS := optional 

include $(BUILD_SHARED_LIBRARY)

