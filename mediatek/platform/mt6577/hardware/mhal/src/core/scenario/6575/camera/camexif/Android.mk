#
# libmhalscenario_plat_camexif
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  CamExif.cpp \
  DbgCamExif.cpp \

LOCAL_C_INCLUDES := \
  $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \

#MediaHal.h
LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/ \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc \

#
LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/libutility \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/libmexif \

LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/6575/inc \

LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/inc \

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE := libmhalscenario_plat_camexif

include $(BUILD_STATIC_LIBRARY)
