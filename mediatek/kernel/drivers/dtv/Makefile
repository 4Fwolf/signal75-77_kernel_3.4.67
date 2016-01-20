#coky add for NMI5625
#obj-$(CONFIG_MTK_NMI)	+= nmi5625.o
#nmi5625-objs := nmi_gpio_i2c.o nmi5625_kernel.o
#<2014/05/06-samhaung, porting Hawk35 DTV
ifeq ($(ARIMA_PROJECT_HAWK35_DTV),yes)
obj-y += nmi_gpio_i2c.o
obj-y += nmi5625_kernel.o
obj-y += nmi326_spi_drv.o
endif



