LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
        ./src/AcdkBase.cpp \
        ./src/surfaceview/AcdkSurfaceView.cpp \
        ./src/surfaceview/mHalSurfaceView.cpp \
        ./src/CCTCtrl/AcdkCCTCtrl.cpp \
        ./src/CCTCtrl/AppCCTCtrl.cpp \
        ./src/AcdkImgTool.cpp \
        ./src/calibration/AcdkCalibration.cpp \
        ./src/calibration/MT6516Calibration.cpp \
        ./src/AcdkIF.cpp \

LOCAL_LDLIBS += lpthread

LOCAL_SHARED_LIBRARIES:= \
    liblog \
    libutils \
    libcutils \
    libmhalpipe \
    libmhalmdp \
    libmhaldrv \
    libcameracustom \
    libmhal \
    libmexif \
    libmhalcontent \
    libmpoencoder \

#    libmhalscenario \

LOCAL_SHARED_LIBRARIES += libstlport
LOCAL_SHARED_LIBRARIES += libcam.utils

#   camera debug header 
LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/core/cameradebug/inc \

LOCAL_C_INCLUDES += \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/inc \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/inc \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/inc \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/inc \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/common/inc \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/common/strobe \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/custom/inc \
    $(MTK_ROOT)/kernel/drivers/video \
    $(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
    $(MTK_PATH_CUSTOM)/kernel/flashlight/inc \
    $(MTK_PATH_CUSTOM)/kernel/sensor/inc \
    $(MTK_PATH_CUSTOM)/kernel/lens/inc \
    $(MTK_ROOT)/external/mhal/src/core/drv/6575/inc \
    $(MTK_PATH_CUSTOM)/hal/inc/aaa \
    $(TOP)/system/core/include/cutils \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/inc \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/mfv/mfv_common/codec/common/api/inc \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/mfv/mfv_common/codec/mpeg4_enc/inc \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/mfv/mfv_common/codec/h264_enc/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/drivers/mflexvideo/codec/common/val/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/drivers/mflexvideo/codec/common/api/inc \
    $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
    $(TOP)/system/core/include/cutils \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/jpeg \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/imagetransform \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/common/inc \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/6575/camera \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/6575/inc  \
    $(MTK_PATH_PLATFORM)/hardware/mhal/inc/mdp \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/libutility \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/common/libmexif \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/inc \

# mHal
LOCAL_C_INCLUDES += \
    $(MTK_PATH_PLATFORM)/hardware/ \
    $(MTK_PATH_PLATFORM)/hardware/mhal/inc \
    $(MTK_PATH_PLATFORM)/hardware/mhal/inc/camera \
    

# custom
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa

#
# others
LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_ROOT)/kernel/include/ \
    $(TOP)/$(MTK_ROOT)/kernel/include/linux \
  
#
# EIS 
LOCAL_C_INCLUDES += \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/eis \
    $(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libeis/MT6575 \

LOCAL_WHOLE_STATIC_LIBRARIES := \
  libmhalscenario_plat \

LOCAL_SHARED_LIBRARIES += libmhalutility

LOCAL_MODULE:= libacdk

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)


