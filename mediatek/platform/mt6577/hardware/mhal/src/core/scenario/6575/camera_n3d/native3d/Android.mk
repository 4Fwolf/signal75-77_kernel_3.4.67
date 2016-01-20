#
# libmhalscenario_plat_native3dshot
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  N3D_NormalShot.cpp \

#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa
#
# MediaHal.h
LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/ \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc \

#
LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/libutility \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/libmexif \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/libmpo \

LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/6575/camera_n3d \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/6575/camera_n3d/shot \

LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/inc \

LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/jpeg \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc/mdp \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \

LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE := libmhalscenario_plat_native3dshot

include $(BUILD_STATIC_LIBRARY)
