# Custom kernel for Xiaomi Redmi Note 2 (Hermes)
# Kernel version 3.10.61
# Vendor Vanzo
Works in rom(tested 7.0.8.0) and recovery

* Works:
	* LCM(nt35596 tianma)
	* tps65132
	* Sdcard
	* Wi-fi
	* IOCTL (fixed hwcomposer and surfaceflinger)
	* da9210 (charger driver)
	* Bt
	* Button-backlight
	* Brightness
	* Leds indication only (red,green,blu)
	* MD1 and MD2(sim1 and sim2)
	* Touch(focaltech)
	* Alsps (ps bugged)
	* Accel
	* Mag
	* Giro
	* OTG
	* SOUND(Speaker,Headphones)
	* Vibrator
	* Fix Sleep
	* Touch atmel
        * LCM nt35596_auo
	* Battery 3000mah(stock table)


* Don't work:
	* LCM (nt35532_boe)
        * Leds(bugs) 
        * Gpu (bugs weekly MIUI libsrv_init.so) 
	* Imgsensor(all img sensors)
	* Lens
	* CW2015 (hw battery driver)
	* IR Blaster
	* Flashlight

=================================================
# BUILD
export TOP=$(pwd)
export CROSS_COMPILE=/home/igor/aarch64-linux-android-4.9-linaro-master/bin/aarch64-linux-android-
mkdir -p $TOP/KERNEL_OBJ
make -C kernel-3.10 O=$TOP/KERNEL_OBJ ARCH=arm64 MTK_TARGET_PROJECT=hermes TARGET_BUILD_VARIANT=user CROSS_COMPILE=$TOOLCHAIN ROOTDIR=$TOP hermes_defconfig
make -C kernel-3.10 O=$TOP/KERNEL_OBJ ROOTDIR=$TOP

# I2C

* I2C0
	* tps65132              (003e)
	* kd_camera_hw          (007f)
	* DF9761BAF             (0018) - LENS
	* CAM_CAL_DRV           (0036)

* I2C1
	* da9210                (0068)
	* tps6128x              (0075)

* I2C2
	* atmel                 (004a)
	* kd_camera_hw_bus 2    (007f)
	* FT			(0038)

* I2C3
	* akm0991               (000c)
	* yas537                (002e)
	* LSM6DS3_ACCEL         (006a)
	* LTR_559ALS		(0023)
	* LSM6DS3_GYRO		(0034)
	* stk3x1x               (0048)
	* bmi160_gyro		(0066)
	* bmi160_acc		(0068)

* I2C4
	* CW2015 		(0062)

# AUTORS
* nofearnohappy
* LazyC0DEr
* Anomalchik
