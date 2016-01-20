#===============================================================================


LOCAL_PATH:= $(call my-dir)
################################################################################
# libmhalscenario_plat
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
LOCAL_SRC_FILES += mhal_cam_base.cpp

#-----------------------------------------------------------
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
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/6575/camera
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/scenario/6575/camera_n3d
#
# pipe
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/common/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/videoio/export
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/videoio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/eis
#
# algorithm 
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/algorithm/libeis/MT6575
#
# custom
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/camera_feature
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa
#
#
#-----------------------------------------------------------
#LOCAL_STATIC_LIBRARIES += lib_mhal_mfv_mp4_test

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libmhalscenario_plat_cam
LOCAL_WHOLE_STATIC_LIBRARIES += libmhalscenario_plat_cam_n3d

#-----------------------------------------------------------
LOCAL_MODULE := libmhalscenario_plat

#-----------------------------------------------------------
include $(BUILD_STATIC_LIBRARY)


################################################################################
# libmhalscenario
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog libcutils libutils libdl
LOCAL_SHARED_LIBRARIES += libstlport
LOCAL_SHARED_LIBRARIES += libcameracustom libmhalcontent libmexif
LOCAL_SHARED_LIBRARIES += libmhalpipe libmhaldrv libmhalmdp  libeis 
LOCAL_SHARED_LIBRARIES += libmpoencoder

#ifeq ($(strip $(MTK_PLATFORM)),MT6575)
#LOCAL_SHARED_LIBRARIES += libmpeg4enc_ca9
#endif

#LOCAL_SHARED_LIBRARIES += libvcodec_oal libvcodecdrv libvcodec_utility

#LOCAL_SHARED_LIBRARIES += libvcodec_utility
#LOCAL_SHARED_LIBRARIES += libmhalutility

LOCAL_SHARED_LIBRARIES += libcam.utils

#-----------------------------------------------------------
#MediaHal.h
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/pipe/6575/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/jpeg
LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/kernel/include/linux
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/kernel/core/include/mach
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/factory/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/src/core/drv/6575/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mhal/inc/mdp
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/mhal/src/core/scenario/6575/factory/MFVEMITest

#-----------------------------------------------------------
LOCAL_SRC_FILES += ../imagetransform/scenario_imagetransform_mt6575.cpp
LOCAL_SRC_FILES += ../imagecodec/mhal_jpeg_enc.cpp
#LOCAL_SRC_FILES += factory/multimediaFactoryTest.cpp
#LOCAL_SRC_FILES += factory/mdp_factory.cpp
#LOCAL_SRC_FILES += factory/MFVEMITest/CompTestBench.c
#LOCAL_SRC_FILES += factory/MFVEMITest/video_codec_if_sp.c

#-----------------------------------------------------------
LOCAL_CFLAGS += -DANDROID_LOAD
LOCAL_CFLAGS += -DSET_PRIORITY
LOCAL_CFLAGS += -DSET_MEM_BANDWIDTH_SCENARIO
#LOCAL_CFLAGS += -DREAD_VOP_TIME
LOCAL_CFLAGS += -DUSLEEP
LOCAL_CFLAGS += -DTIME_ENCODE_ONE_FRAME
LOCAL_CFLAGS += -DFACTORY_MODE_TEST

#-----------------------------------------------------------
#LOCAL_STATIC_LIBRARIES +=  lib_mhal_mfv_mp4_test

LOCAL_SHARED_LIBRARIES += libmhalutility
#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libmhalscenario_plat

#-----------------------------------------------------------
LOCAL_MODULE := libmhalscenario

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_TAGS := optional

#-----------------------------------------------------------
include $(BUILD_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))


#===============================================================================

