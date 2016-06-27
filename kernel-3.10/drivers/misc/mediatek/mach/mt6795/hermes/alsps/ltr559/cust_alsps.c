#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 3,
    .polling_mode_ps =0,
    .polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .als_level  = {0,  20, 40, 70, 100, 150,  200,  300, 600, 1000,  2000, 6000, 10000, 14000, 18000, 20000},
    .als_value  = {0,  20, 40, 90, 150, 190,  240,  300, 600, 1800,  4000, 6000,  8000,  8000, 10240, 10240},
    //.als_value  = {0, 0, 40,  90, 320, 320,  320,  640,  2600,  2600,  2600,  2600,  2600, 2600,  10240, 10240},
    // {40, 40, 90,  90, 160, 160,  225,  320,  640,  1280,  1280,  2600,  2600, 2600,  10240, 10240} by zjz 20130821 for w8660
    .ps_threshold_high = 350,//650,//750, //350,  //500, // 4800
    .ps_threshold_low = 320,//600,//700, //320,  //450,  //4500
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

