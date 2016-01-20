#
# libmexif
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    exif_init.cpp \
    exif_make.cpp \
    exif_hdr.cpp \
    exif_ifdinit.cpp \
    exif_ifdmisc.cpp \
    exif_ifdlist.cpp \
    exif_misc.cpp
    
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/mhal

LOCAL_SHARED_LIBRARIES:= liblog

LOCAL_MODULE:= libmexif

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

