/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <linux/types.h>
#include "stk3x1x_cust_alsps.h"
#include <mach/mt_pm_ldo.h>

//#ifdef MT6573
//#include <mach/mt6573_pll.h>
//#endif
//#ifdef MT6575
//#include <mach/mt6575_pm_ldo.h>
//#endif
//#ifdef MT6577
//#include <mach/mt6577_pm_ldo.h>
//#endif
//#ifdef MT6589
//#include <mach/mt_pm_ldo.h>
//#endif
//#ifdef MT6572
//#include <mach/mt_pm_ldo.h>
//#endif
//#ifdef MT6582
//#include <mach/mt_pm_ldo.h>
//#endif

static struct alsps_hw cust_alsps_hw = {
	/* i2c bus number, for mt657x, default=0. For mt6589, default=3 */
//#ifdef MT6589	
//    .i2c_num    = 3,
//#elif defined(MT6572)	
//    .i2c_num    = 1,
//#elif defined(MT6582)
//    .i2c_num    = 2,	
//#else	
//    .i2c_num    = 0,	
//#endif	
	.i2c_num    = 2,	
	.polling_mode_ps =0,
	.polling_mode_als = 1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .i2c_addr   = {0x90, 0x00, 0x00, 0x00},	/*STK3x1x*/
    //.als_level  = {5,  9, 36, 59, 80, 120, 180, 260, 450, 845, 1136, 1545, 2364, 4655, 6982},	/* als_code */
    .als_level  = {15,  27, 108, 177, 240, 360, 540, 780, 1350, 2535, 3408, 4635, 7092, 13965, 20946},	/* als_code */
	  //.als_level  = {6, 12, 56, 78, 108, 174, 270, 360, 660, 1116, 1500, 2040, 3120, 6144, 9216},	/* als_code */
    .als_value  = {0, 50, 130, 130, 200, 250, 380, 550, 760, 1250, 1700, 2300, 4000, 5120, 7000, 10240},    /* lux */
   	.state_val = 0x0,		/* disable all */
	.psctrl_val = 0x31,		/* ps_persistance=4, ps_gain=64X, PS_IT=0.391ms */
	.alsctrl_val = 0x39, 	/* als_persistance=1, als_gain=64X, ALS_IT=50ms */
	.ledctrl_val = 0xBF,	/* 100mA IRDR, 64/64 LED duty */
	.wait_val = 0x7,		/* 50 ms */
    .ps_high_thd_val = 1700,
    .ps_low_thd_val = 1500,
};
struct alsps_hw *stx3x1x_get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}
