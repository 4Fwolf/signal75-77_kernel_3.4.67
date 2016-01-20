##
##
##

#ifeq ($(MTK_NFC_SUPPORT),yes)
ifeq ($(strip $(ARIMA_PROJECT_NFC_SUPPORT)),yes)

ifeq ($(strip $(MTK_NFC_PN544)),yes)
obj-$(CONFIG_PN544) += pn544.o
obj-$(CONFIG_PN544) += nfc_swp.o
endif

endif

## EOF
