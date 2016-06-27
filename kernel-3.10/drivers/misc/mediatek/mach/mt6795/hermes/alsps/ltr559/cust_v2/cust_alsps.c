#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 3,
    .polling_mode_ps = 0,   
    .polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    
    .power_vol  = VOL_DEFAULT,          
    .i2c_addr   = {0x72, 0x48, 0x78, 0x00},
    .als_level  = {9, 30, 50, 200, 400, 12000, 16000, 20000, 25000, 208040},	
    .als_value  = {10, 40, 90, 160, 225, 320, 640, 1280, 2600, 10240},
    .ps_threshold_high = 220,  
    .ps_threshold_low = 196,  
    .ps_threshold = 150,
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

