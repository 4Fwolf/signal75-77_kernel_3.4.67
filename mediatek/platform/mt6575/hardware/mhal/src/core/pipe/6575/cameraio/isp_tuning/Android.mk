#
# libmhalpipe_isp_tuning
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  ../../../common/isp_tuning/hal/instance/isp_tuning_instance.cpp \
  ../../../common/isp_tuning/hal/bridge/isp_tuning_bridge.cpp \
  ../../../common/isp_tuning/hal/bridge/isp_tuning_bridge_user.cpp \
  ../../../common/isp_tuning/paramctrl/comm_lifetime.cpp \
  ../../../common/isp_tuning/paramctrl/comm_attributes.cpp \
  ../../../common/isp_tuning/paramctrl/comm_user.cpp\
  ../../../common/isp_tuning/paramctrl/comm.cpp \
  ../../../common/isp_tuning/paramctrl/yuv.cpp \
  ../../../common/isp_tuning/paramctrl/raw_lifetime.cpp \
  ../../../common/isp_tuning/paramctrl/raw_attributes.cpp \
  ../../../common/isp_tuning/paramctrl/raw_validate.cpp \
  ../../../common/isp_tuning/paramctrl/raw_effect.cpp \
  ../../../common/isp_tuning/paramctrl/raw_user.cpp \
  paramctrl/raw_exif.cpp \
  paramctrl/raw_frameless.cpp \
  paramctrl/raw_per_frame.cpp \
  paramctrl/yuv_frameless.cpp \
  ../../../common/isp_tuning/hwctrl/nvram_drv_mgr.cpp \
  ../../../common/isp_tuning/hwctrl/sysram_drv_mgr.cpp \
  ../../../common/isp_tuning/hwctrl/ispdrvmgr_ctx.cpp \
  hwctrl/ispmgr_mt6575.cpp \
  hwctrl/pcamgr_mt6575.cpp \
  hwctrl/ispmgr_helper.cpp \

LOCAL_C_INCLUDES:= \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
  $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/isp_tuning/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/isp_tuning/hal/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/isp_tuning/hwctrl/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/isp_tuning/paramctrl/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/cameraio/isp_tuning/hwctrl/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/cameraio/isp_tuning/paramctrl/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/isp \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/common/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc \
  $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/liblsctrans \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/camera \
  $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \

LOCAL_MODULE:= libmhalpipe_isp_tuning

include $(BUILD_STATIC_LIBRARY)
