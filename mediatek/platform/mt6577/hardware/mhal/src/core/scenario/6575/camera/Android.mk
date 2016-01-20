#
# libmhalscenario_plat_cam
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  ./3A/mHalCam3A.cpp \
  ./video/mHalCamVideo.cpp \
  mhal_cam.cpp \


#
# mhal
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc
#
# mhal\core
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/libutility
#
# scenario
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/common/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/6575/inc
#
# pipe
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/videoio/export
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/videoio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/eis
#
# custom
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa
#
# others
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include/linux
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include/
#
# algorithm
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libeis/MT6575
#
# <Remove me if possible> used by mdp driver
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc/mdp
#
# <Remove me if possible> used by eis hal
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc
#
# <Remove me if possible> should not use jpeg driver directly
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/jpeg


################################################################################
LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \
  libmhalscenario_plat_camexif \
  libmhalscenario_plat_shot \
  libmhalscenario_plat_fd \
  libmhalscenario_plat_videosnapshot \

LOCAL_WHOLE_STATIC_LIBRARIES += \
  libmhalscenario_plat_cam3dlib \

ifeq "yes" "$(strip $(MTK_CAM_HDR_SUPPORT))"
  LOCAL_WHOLE_STATIC_LIBRARIES += libmhalscenario_plat_hdr
endif

ifeq "yes" "$(strip $(MTK_MAV_SUPPORT))"
  LOCAL_WHOLE_STATIC_LIBRARIES += libmhalscenario_plat_mav
endif

ifeq "yes" "$(strip $(MTK_AUTORAMA_SUPPORT))"
  LOCAL_WHOLE_STATIC_LIBRARIES += libmhalscenario_plat_autorama
endif

ifeq "yes" "$(strip $(MTK_ASD_SUPPORT))"
  LOCAL_WHOLE_STATIC_LIBRARIES += libmhalscenario_plat_asd
endif

ifeq "yes" "$(strip $(MTK_CAM_FACEBEAUTY_SUPPORT))"
  LOCAL_WHOLE_STATIC_LIBRARIES += libmhalscenario_plat_fb
endif

ifeq "yes" "$(strip $(MTK_S3D_SUPPORT))"
  LOCAL_WHOLE_STATIC_LIBRARIES += libmhalscenario_plat_cam3dshot
endif

################################################################################
MHAL_CAM_FLICKER_SERVICE_SUPPORT:= 1    # flicker service support on/off

#
ifeq "1" "$(strip $(MHAL_CAM_FLICKER_SERVICE_SUPPORT))"
  LOCAL_CFLAGS += -DMHAL_CAM_FLICKER_SERVICE_SUPPORT
  LOCAL_WHOLE_STATIC_LIBRARIES += libmhalscenario_plat_flickerservice
endif

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common/camutils

################################################################################
LOCAL_MODULE := libmhalscenario_plat_cam

include $(BUILD_STATIC_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
