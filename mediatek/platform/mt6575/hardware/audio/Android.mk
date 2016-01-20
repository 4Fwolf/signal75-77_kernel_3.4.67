#  only if use yusu audio will build this.
ifeq ($(strip $(BOARD_USES_MTK_AUDIO)),true)

LOCAL_PATH:= $(call my-dir)

#audiohardware
include $(CLEAR_VARS)

AudioDriverIncludePath := aud_drv

LOCAL_CFLAGS += -DMTK_AUDIO  

ifeq ($(AUDIO_POLICY_TEST),true)
    ENABLE_AUDIO_DUMP := true
endif

ifeq ($(strip $(TARGET_BUILD_VARIANT)),eng)
    LOCAL_CFLAGS += -DDEBUG_AUDIO_PCM
endif


ifeq ($(strip $(MTK_PLATFORM)),MT6577)
    LOCAL_CFLAGS += -DMT6577
    AudioDriverName := mt6577
endif

ifeq ($(MTK_DIGITAL_MIC_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_DIGITAL_MIC_SUPPORT
endif

$(warning Value of TOPDIR is '$(TOPDIR)')
$(warning Value of MTK_PATH_SOURCE is '$(MTK_PATH_SOURCE)')
$(warning Value of MTK_PATH_PLATFORM is '$(MTK_PATH_PLATFORM)')
$(warning Value of LOCAL_PATH is '$(LOCAL_PATH)')
$(warning Value of MTK_ROOT_CUSTOM_OUT is '$(MTK_ROOT_CUSTOM_OUT)')


LOCAL_C_INCLUDES:= \
    $(TOPDIR)/hardware/libhardware_legacy/include \
    $(TOPDIR)/hardware/libhardware/include \
    $(TOPDIR)/frameworks/av/include \
    $(call include-path-for, audio-utils) \
    $(call include-path-for, audio-effects) \
    $(MTK_PATH_PLATFORM)/hardware/audio/aud_drv \
    $(MTK_PATH_PLATFORM)/hardware/audio/LAD \
    $(MTK_PATH_SOURCE)/external/AudioCompensationFilter \
    $(MTK_PATH_SOURCE)/frameworks/av/include/media \
    $(MTK_PATH_SOURCE)/frameworks/av/include \
    $(MTK_PATH_SOURCE)/frameworks-ext/av/include \
    $(MTK_PATH_SOURCE)/frameworks-ext/av/include/media \
    $(MTK_PATH_SOURCE)/external/audiodcremoveflt \
    $(MTK_PATH_SOURCE)/external/audiocustparam \
    $(MTK_PATH_SOURCE)/external/AudioSpeechEnhancement/inc \
    $(MTK_PATH_SOURCE)/kernel/include \
    $(MTK_PATH_PLATFORM)/kernel/drivers/hdmitx \
    $(MTK_ROOT_CUSTOM_OUT)/hal/audioflinger \
    $(MTK_ROOT_CUSTOM_OUT)/hal/audioflinger/audio \
    $(MTK_PATH_SOURCE)/platform/common/hardware/audio/include

LOCAL_SRC_FILES+= \
    AudioHardwareGeneric.cpp \
    AudioHardwareStub.cpp \
    AudioHardwareInterface.cpp \
    audio_hw_hal.cpp \
  ../../../common/hardware/audio/aud_drv/AudioFtmBase.cpp \
  ../../../common/hardware/audio/aud_drv/AudioMTKDcRemoval.cpp \
	$(AudioDriverIncludePath)/AudioAnalogAfe.cpp \
	$(AudioDriverIncludePath)/AudioAfe.cpp \
	$(AudioDriverIncludePath)/AudioFtm.cpp \
	$(AudioDriverIncludePath)/AudioI2S.cpp \
	$(AudioDriverIncludePath)/AudioYusuHeadsetMessage.cpp \
	$(AudioDriverIncludePath)/AudioYusuHardware.cpp \
	$(AudioDriverIncludePath)/AudioYusuStreamHandler.cpp \
	$(AudioDriverIncludePath)/AudioYusuVolumeController.cpp \
	$(AudioDriverIncludePath)/AudioYusuStreamOut.cpp \
	$(AudioDriverIncludePath)/AudioYusuStreamIn.cpp \
	$(AudioDriverIncludePath)/AudioYusuApStreamIn.cpp \
	$(AudioDriverIncludePath)/AudioYusuI2SStreamIn.cpp \
	$(AudioDriverIncludePath)/AudioYusuDef.cpp \
	$(AudioDriverIncludePath)/AudioStreamInHandler.cpp \
	$(AudioDriverIncludePath)/AudioAMPControlInterface.cpp \
	$(AudioDriverIncludePath)/HeadSetDetect.cpp \
	$(AudioDriverIncludePath)/AudioParamTuning.cpp \
	$(AudioDriverIncludePath)/AudioSpeechEnhLayer.cpp \
	$(AudioDriverIncludePath)/AudioPreProcess.cpp \
	audio_hw_I2S_hal.cpp \
	LAD/AudioYusuCcci.cpp \
	LAD/AudioYusuLad.cpp \
	LAD/AudioYusuLadPlayer.cpp \
	LAD/AudioPcm2way.cpp      

#ifeq ($(MTK_VIBSPK_SUPPORT),yes)
  LOCAL_SRC_FILES += ../../../common/hardware/audio/aud_drv/AudioVIBSPKControl.cpp
#endif

ifeq ($(EXT_DAC_SUPPORT),yes)
    LOCAL_SRC_FILES+= $(AudioDriverIncludePath)/AudioCustomizationBase.cpp
    LOCAL_GENERATED_SOURCES += $(MTK_ROOT_CUSTOM_OUT)/hal/audioflinger/audio/audio_customization.cpp   
    LOCAL_CFLAGS += -DENABLE_EXT_DAC
endif

ifeq ($(strip $(MTK_AUDIO_GAIN_TABLE_SUPPORT)),yes)
    LOCAL_CFLAGS += -DMTK_AUDIO_GAIN_TABLE
endif
   
ifeq ($(MTK_EXT_AMP_SUPPORT),yes)
    LOCAL_CFLAGS += -DENABLE_EXT_AMP
endif    

ifeq ($(MTK_DT_SUPPORT),yes)
    ifeq ($(EVDO_DT_SUPPORT),yes)
        LOCAL_SRC_FILES+= \
            LAD/2nd_modem/EVDO/AudioATCommand.cpp         
    else
        LOCAL_SRC_FILES+= \
            LAD/2nd_modem/mt6252/AudioATCommand.cpp 
    endif
        LOCAL_SHARED_LIBRARIES += \
            libbtpcm
endif    

ifeq ($(ENABLE_AUDIO_DUMP),true)
    LOCAL_SRC_FILES += AudioDumpInterface.cpp
    LOCAL_CFLAGS += -DENABLE_AUDIO_DUMP
endif

LOCAL_STATIC_LIBRARIES := \
    libmedia_helper 

LOCAL_SHARED_LIBRARIES += \
    libmedia \
    libcutils \
    libutils \
    libbinder \
    libhardware_legacy \
    libhardware \
    libblisrc \
    libdl \
    libnvram \
    libaudiocompensationfilter \
    libspeech_enh_lib \
    libpowermanager \
	libaudiocustparam \
    libtinyalsa \
    libaudioutils \
    libaudiodcrflt
    
ifeq ($(HAVE_MATV_FEATURE),yes)
    ifeq ($(MTK_MATV_ANALOG_SUPPORT),yes)
        LOCAL_CFLAGS += -DMATV_AUDIO_LINEIN_PATH
    endif
endif

ifeq ($(MTK_DUAL_MIC_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_DUAL_MIC_SUPPORT
endif

ifeq ($(MTK_WB_SPEECH_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_WB_SPEECH_SUPPORT
endif

ifeq ($(strip $(MTK_FM_SUPPORT)),yes)
    ifeq ($(strip $(MTK_FM_TX_SUPPORT)),yes)
        ifeq ($(strip $(MTK_FM_TX_AUDIO)),FM_DIGITAL_OUTPUT)
            LOCAL_CFLAGS += -DFM_DIGITAL_OUT_SUPPORT
        endif
        ifeq ($(strip $(MTK_FM_TX_AUDIO)),FM_ANALOG_OUTPUT)
            LOCAL_CFLAGS += -DFM_ANALOG_OUT_SUPPORT
        endif
    endif
    ifeq ($(strip $(MTK_FM_RX_SUPPORT)),yes)
        ifeq ($(strip $(MTK_FM_RX_AUDIO)),FM_DIGITAL_INPUT)
            LOCAL_CFLAGS += -DFM_DIGITAL_IN_SUPPORT
        endif
        ifeq ($(strip $(MTK_FM_RX_AUDIO)),FM_ANALOG_INPUT)
            LOCAL_CFLAGS += -DFM_ANALOG_IN_SUPPORT
        endif
    endif
endif

ifeq ($(MTK_DUAL_MIC_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_DUAL_MIC_SUPPORT
endif

ifeq ($(MTK_WB_SPEECH_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_WB_SPEECH_SUPPORT
endif

ifeq ($(MTK_SPH_EHN_CTRL_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_SPH_EHN_CTRL_SUPPORT
endif

ifeq ($(MTK_BT_SUPPORT),yes)
    ifeq ($(MTK_BT_PROFILE_A2DP),yes)
        LOCAL_CFLAGS += -DWITH_A2DP
    endif
else
    ifeq ($(strip $(BOARD_HAVE_BLUETOOTH)),yes)
        LOCAL_CFLAGS += -DWITH_A2DP
    endif  
endif

# SRS Processing
ifeq ($(strip $(HAVE_SRSAUDIOEFFECT_FEATURE)),yes)
    LOCAL_CFLAGS += -DHAVE_SRSAUDIOEFFECT
endif
# SRS Processing

# Audio HD Record
ifeq ($(MTK_AUDIO_HD_REC_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_AUDIO_HD_REC_SUPPORT
endif
# Audio HD Record


ifeq ($(MTK_AUDIO_HD_REC_SUPPORT),yes)
	ifeq ($(MTK_AP_SPEECH_ENHANCEMENT),yes)
	LOCAL_CFLAGS += -DMTK_AP_SPEECH_ENHANCEMENT
		ifeq ($(ALL_USING_AP_ENHANCEMENT),yes)
		LOCAL_CFLAGS += -DALL_USING_AP_ENHANCEMENT
		endif
		ifeq ($(MTK_AP_SPEECH_ENHANCEMENT_VOIP),yes)
		LOCAL_CFLAGS += -DMTK_AP_SPEECH_ENHANCEMENT_VOIP
		endif
	endif
endif

LOCAL_ARM_MODE := arm
LOCAL_MODULE := libaudio.primary.default
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)


# The default audio policy, for now still implemented on top of legacy
# policy code
include $(CLEAR_VARS)

AudioDriverName := default
  
LOCAL_CFLAGS += -DMTK_AUDIO

ifeq ($(HAVE_MATV_FEATURE),yes)
    ifeq ($(MTK_MATV_ANALOG_SUPPORT),yes)
        LOCAL_CFLAGS += -DMATV_AUDIO_LINEIN_PATH
    endif
endif

ifeq ($(strip $(OPTR_SPEC_SEG_DEF)),OP01_SPEC0200_SEGC)
    LOCAL_CFLAGS += -DAUDIO_OP01_SPEC0200_SEGC
endif

ifeq ($(DISABLE_EARPIECE),yes)
    LOCAL_CFLAGS += -DDISABLE_EARPIECE
endif

LOCAL_C_INCLUDES:= \
    $(TOPDIR)/hardware/libhardware_legacy/include \
    $(TOPDIR)/frameworks/av/include \
	$(call include-path-for, audio-utils) \
    $(call include-path-for, audio-effects) \
    $(MTK_PATH_PLATFORM)/hardware/audio/aud_drv \
    $(MTK_PATH_PLATFORM)/hardware/audio/LAD \
    $(MTK_PATH_SOURCE)/external/AudioCompensationFilter \
    $(MTK_PATH_SOURCE)/external/audiocustparam \
    $(MTK_PATH_SOURCE)/frameworks/av/include/media \
    $(MTK_PATH_SOURCE)/frameworks/av/include \
    $(MTK_PATH_SOURCE)/frameworks-ext/av/include \
    $(MTK_PATH_SOURCE)/frameworks-ext/av/include/media \
    $(MTK_PATH_SOURCE)/external/audiodcremoveflt \
    $(MTK_PATH_SOURCE)/kernel/include \
    $(MTK_ROOT_CUSTOM_OUT)/hal/audioflinger \
    $(MTK_ROOT_CUSTOM_OUT)/hal/audioflinger/audio

LOCAL_SRC_FILES := \
    AudioPolicyManagerBase.cpp \
    AudioPolicyCompatClient.cpp \
    audio_policy_hal.cpp \
    AudioPolicyManagerDefault.cpp \
    AudioYusuPolicyManager.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libmedia \
    libaudiocustparam

LOCAL_STATIC_LIBRARIES := \
    libmedia_helper

ifeq ($(strip $(MTK_FM_RX_SUPPORT)),yes)
    ifeq ($(strip $(MTK_FM_RX_AUDIO)),FM_DIGITAL_INPUT)
        LOCAL_CFLAGS += -DFM_DIGITAL_IN_SUPPORT
    endif
    ifeq ($(strip $(MTK_FM_RX_AUDIO)),FM_ANALOG_INPUT)
        LOCAL_CFLAGS += -DFM_ANALOG_IN_SUPPORT
    endif
endif

ifeq ($(strip $(MTK_AUDIO_GAIN_TABLE_SUPPORT)),yes)
    LOCAL_SRC_FILES+= \
      $(AudioDriverIncludePath)/AudioUcm.cpp
    LOCAL_CFLAGS += -DMTK_AUDIO_GAIN_TABLE
endif
  
LOCAL_MODULE := audio_policy.$(AudioDriverName)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_TAGS := optional

ifeq ($(MTK_BT_SUPPORT),yes)
    ifeq ($(MTK_BT_PROFILE_A2DP),yes)
        LOCAL_CFLAGS += -DWITH_A2DP
    endif
else
    ifeq ($(strip $(BOARD_HAVE_BLUETOOTH)),yes)
        LOCAL_CFLAGS += -DWITH_A2DP
    endif  
endif

include $(BUILD_SHARED_LIBRARY)

endif
