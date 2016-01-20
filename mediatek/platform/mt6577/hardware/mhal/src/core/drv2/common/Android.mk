#
# libmhaldrv_comm
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)


ifneq ($(MTK_PLATFORM),MT8320)

LOCAL_SRC_FILES:= \
  imgsensor/sensor_drv.cpp \
  imgsensor/imgsensor_drv.cpp \
  imgsensor/atvsensor_drv.cpp \
  lens/mcu_drv.cpp \
  lens/lens_drv.cpp \
  lens/lens_sensor_drv.cpp \
  strobe/strobe_drv.cpp \
  strobe/flashlight_drv.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach \
  $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc \
  $(MTK_PATH_CUSTOM)/hal/inc \
  $(MTK_PATH_CUSTOM)/hal/inc/camera_feature \
  $(MTK_PATH_CUSTOM)/hal/camera \
  $(TOP)/$(MTK_PATH_CUSTOM)/kernel/lens/inc \
  $(TOP)/$(MTK_PATH_CUSTOM)/kernel/flashlight/inc \

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/camera/inc/common/camutils

#matv define
#ifeq ($(HAVE_MATV_FEATURE),yes)
#ifeq ($(MTK_ATV_CHIP), $(filter $(MTK_ATV_CHIP),MTK_MT5192 MTK_MT5193))
    LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/external/matv_cust  \
    $(TOP)/$(MTK_PATH_SOURCE)/frameworks/libs/atvctrlservice \
    $(TOP)/frameworks/base/include/media \
    $(TOP)/frameworks/base/include/utils \
    $(TOP)/frameworks/base/include
    LOCAL_CFLAGS+= -DATVCHIP_MTK_ENABLE
#endif
#endif

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \
    
LOCAL_MODULE:= libmhaldrv_comm

include $(BUILD_STATIC_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))

endif
