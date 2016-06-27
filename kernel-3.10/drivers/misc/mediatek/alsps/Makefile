include $(srctree)/drivers/misc/mediatek/Makefile.custom

# In case the platform does NOT support this type of sensors
obj-y   +=  alsps.o
obj-y   +=  alsps_factory.o
obj-y   +=  aal_control.o

ifeq ($(CONFIG_MTK_LTR559),y)
obj-y	+=  ltr559/
endif

ifeq ($(CONFIG_MTK_STK3X1X),y)
obj-y	+=  stk3x1x/
endif

ifeq ($(CONFIG_MTK_STK3X1X_NEW),y)
obj-y	+=  stk3x1x-new/
endif
