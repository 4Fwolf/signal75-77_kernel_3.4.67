#
# libmhalpipe_comm
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  flashlight/flashlight_hal.cpp \
  flashlight/flashlight_hal_base.cpp \
  pano/pano/pano_hal.cpp \
  pano/pano_hal_base.cpp \
  autorama/autorama/autorama_hal.cpp \
  autorama/autorama_hal_base.cpp \
  3Dfeature/mav/mav_hal.cpp \
  3Dfeature/pano3d/pano3d_hal.cpp \
  3Dfeature/3DF_hal_base.cpp \
  facebeautify/facebeautify/facebeautify_hal.cpp \
  facebeautify/facebeautify_hal_base.cpp \
  hdr/hdr/hdr_hal.cpp \
  hdr/hdr_hal_base.cpp \
  asd/asd/asd_hal.cpp \
  asd/asd_hal_base.cpp
  
LOCAL_C_INCLUDES:= \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/pano/pano \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/3Dfeature/mav \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/autorama/autorama \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/asd/asd \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/3Dfeature/pano3d \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/facebeautify/facebeautify \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/hdr/hdr \

#MediaHal.h 
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/ \

#for algroithm 
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libmav \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libautopano \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libasd \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libpano3d \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libwarp \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libmotion \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libutility \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libpano \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libfb \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libhdr \


LOCAL_C_INCLUDES += \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/inc \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa/mt6575/param \
  
LOCAL_SHARED_LIBRARIES := \
    libcamalgo \
 
LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE:= libmhalpipe_comm

include $(BUILD_STATIC_LIBRARY)
