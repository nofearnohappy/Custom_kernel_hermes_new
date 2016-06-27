
#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>

static struct alsps_hw cust_alsps_hw_stk3x1x = {
	.i2c_num    = 3,
	.polling_mode_ps =0,
	.polling_mode_als =1,
	.power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
	.power_vol  = VOL_DEFAULT,          /*LDO is not used*/
	.i2c_addr   = {0x90, 0x00, 0x00, 0x00},	/*STK3x1x*/
        .als_level  = {5,  9, 36, 59, 82, 132, 205, 273, 500, 845, 1136, 1545, 2364, 4655, 6982},	/* als_code */
        .als_value  = {0, 10, 40, 65, 90, 145, 225, 300, 550, 930, 1250, 1700, 2600, 5120, 7680, 10240},    /* lux */  	
	//.state_val = 0x0,		/* disable all */
	//.psctrl_val = 0x33,	// 0x31,	/* ps_persistance=1, ps_gain=64X, PS_IT=0.391ms */
	//.alsctrl_val = 0x39,	// 0x38, /* als_persistance=1, als_gain=64X, ALS_IT=50ms */
	//.ledctrl_val = 0xFF,	/* 100mA IRDR, 64/64 LED duty */
	//.wait_val = 0x9, // 0x7,		/* 50 ms */
	.ps_threshold_high = 1500, // 1700,
	.ps_threshold_low = 1300,  // 1500,
};
struct alsps_hw *get_cust_alsps_hw_stk3x1x(void) {
    return &cust_alsps_hw;
}

