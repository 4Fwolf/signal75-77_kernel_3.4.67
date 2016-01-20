LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  CamJpg.cpp \

LOCAL_C_INCLUDES := \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/inc \
  $(TOP)$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/aaa/inc/mtk3a \
  $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/libutility \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/libmexif \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/libmpo \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/libmpo/mpoencoder \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/6575/camera \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/6575/camera/cameraio \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/6575/camera/3dlib \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/jpeg \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc/mdp \
  $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \

#mHal
LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_SOURCE)/external \

#drv 
LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \

#mpo 
LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_SOURCE)/external/mpo \
  $(TOP)/$(MTK_PATH_SOURCE)/external/mpo/inc \
  $(TOP)/$(MTK_PATH_SOURCE)/external/mpo/mpoencoder \



LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE := libmhalscenario_plat_cam3dlib

include $(BUILD_STATIC_LIBRARY)

