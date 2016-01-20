#
# libmhalpipe_cam_feature
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  ../../common/feature/hal/bridge/feature_hal_bridge.cpp \
  ../../common/feature/hal/bridge/feature_hal_bridge_query.cpp \
  ../../common/feature/hal/instance/feature_hal_instance.cpp \
  ../../common/feature/control/ctrl_manager/manager_lifetime.cpp \
  ../../common/feature/control/ctrl_manager/manager_raw.cpp \
  ../../common/feature/control/ctrl_manager/manager_yuv.cpp \
  ../../common/feature/control/ctrl_manager/manager_misc.cpp \
  ../../common/feature/control/ctrl_manager/manager_runtime.cpp \
  ../../common/feature/control/ctrl_manager/manager_custom.cpp \
  ../../common/feature/control/ctrl_manager/manager_correction.cpp \
  ../../common/feature/control/ctrl_manager/manager_extern_module.cpp \
  ../../common/feature/test/feature_test.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature \
  $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/feature/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/feature/hal/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/feature/control/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \

LOCAL_MODULE:= libmhalpipe_cam_feature

include $(BUILD_STATIC_LIBRARY)
