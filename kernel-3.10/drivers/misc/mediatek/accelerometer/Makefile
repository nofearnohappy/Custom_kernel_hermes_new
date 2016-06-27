include $(srctree)/drivers/misc/mediatek/Makefile.custom

# In case the platform does NOT support this type of sensors

obj-y   +=  accel.o accel_factory.o
# Vanzo:wangfei on: Wed, 12 Nov 2014 21:06:42 +0800
project_name:=$(shell echo $(VANZO_INNER_PROJECT_NAME))
 
ifneq ($(project_name),)
custom_kernel_accelerometer:= $(addsuffix /, $(shell echo $(CUSTOM_KERNEL_ACCELEROMETER)))
ifneq ($(custom_kernel_accelerometer),)
obj-y += $(custom_kernel_accelerometer)
endif
else

ifeq ($(CONFIG_MTK_BMI160_ACC),y)
obj-y   +=  bmi160_acc/
endif

endif

