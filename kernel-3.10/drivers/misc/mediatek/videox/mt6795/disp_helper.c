#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/rtpm_prio.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_irq.h>

#include "disp_drv_log.h"

#include "mt_boot.h"
#include "disp_helper.h"
#include "disp_drv_platform.h"

// use this magic_code to detect memory corruption
#define MAGIC_CODE 0xDEADAAA0U

// CONFIG_MTK_FPGA is used in linux kernel for early porting.
// if the macro name changed, please modify the code here too.
#ifdef CONFIG_MTK_FPGA
static unsigned int disp_global_stage = MAGIC_CODE | DISP_HELPER_STAGE_EARLY_PORTING;
#else
// please change this to DISP_HELPER_STAGE_NORMAL after bring up done
static unsigned int disp_global_stage = MAGIC_CODE | DISP_HELPER_STAGE_NORMAL;
#endif

static _is_E1(void)
{
	CHIP_SW_VER ver = mt_get_chip_sw_ver();
	if(CHIP_SW_VER_01 == ver)
		return 1;

	return 0;
}

static _is_E2(void)
{
	CHIP_SW_VER ver = mt_get_chip_sw_ver();
	if(CHIP_SW_VER_02 == ver)
		return 1;

	return 0;
}

static _is_E3(void)
{
	return !(_is_E1() || _is_E2());
}

static _is_early_porting_stage(void)
{
	return ((disp_global_stage&(~MAGIC_CODE)) == DISP_HELPER_STAGE_EARLY_PORTING);
}

static _is_bringup_stage(void)
{
	return ((disp_global_stage&(~MAGIC_CODE)) == DISP_HELPER_STAGE_BRING_UP);
}

static _is_normal_stage(void)
{
	return ((disp_global_stage&(~MAGIC_CODE)) == DISP_HELPER_STAGE_NORMAL);
}

// FIXME: should include header file here.
extern UINT32 DISP_GetScreenWidth(void);
extern UINT32 DISP_GetScreenHeight(void);

int disp_helper_get_option(DISP_HELPER_OPTION option)
{
	//DISPMSG("stage=0x%08x\n", disp_global_stage);
	switch(option)
	{
		case DISP_HELPER_OPTION_USE_CMDQ:
		{
			if(_is_normal_stage())
				return 1;
			else if(_is_bringup_stage())
				return 0;
			else if(_is_early_porting_stage())
				return 0;
			else
				BUG_ON(1);
		}
		case DISP_HELPER_OPTION_USE_M4U:
		{
			if(_is_normal_stage())
				return 1;
			else if(_is_bringup_stage())
				return 0;
			else if(_is_early_porting_stage())
				return 0;
			else
				BUG_ON(1);
		}
		case DISP_HELPER_OPTION_USE_CLKMGR:
		{
			if(_is_normal_stage())
				return 1;
			else if(_is_bringup_stage())
				return 0;
			else if(_is_early_porting_stage())
				return 0;
			else
				BUG_ON(1);
		}
		case DISP_HELPER_OPTION_MIPITX_ON_CHIP:
		{
			if(_is_normal_stage())
				return 1;
			else if(_is_bringup_stage())
				return 1;
			else if(_is_early_porting_stage())
				return 0;
			else
				BUG_ON(1);
		}			
		case DISP_HELPER_OPTION_FAKE_LCM_X:
		{
			int x = 0;
			#ifdef CONFIG_CUSTOM_LCM_X
				x = simple_strtoul(CONFIG_CUSTOM_LCM_X, NULL, 0);
			#endif
			return x;
		}			
		case DISP_HELPER_OPTION_FAKE_LCM_Y:
		{
			int y = 0;
			#ifdef CONFIG_CUSTOM_LCM_Y
				y = simple_strtoul(CONFIG_CUSTOM_LCM_Y, NULL, 0);
			#endif
			return y;
		}	
		case DISP_HELPER_OPTION_FAKE_LCM_WIDTH:
		{
			int x = 0;
			int w = DISP_GetScreenWidth();
			#ifdef CONFIG_CUSTOM_LCM_X
				x = simple_strtoul(CONFIG_CUSTOM_LCM_X, NULL, 0);
				if(x != 0)
				{
					w = ALIGN_TO(w, 16);
				}
			#endif
			return w;
		}			
		case DISP_HELPER_OPTION_FAKE_LCM_HEIGHT:
		{
			int h = DISP_GetScreenHeight();
			return h;
		}
		case DISP_HELPER_OPTION_DYNAMIC_SWITCH_UNDERFLOW_EN:
		{
			return 0;
		}
		case DISP_HELPER_OPTION_OVL_WARM_RESET:
		{
			return 1;
		}
		default:
			break;
	}

	BUG_ON(1);
}

DISP_HELPER_STAGE disp_helper_get_stage(void)
{
	return (disp_global_stage&(~MAGIC_CODE));
}

const char *disp_helper_stage_spy(void)
{
	if(disp_helper_get_stage() == DISP_HELPER_STAGE_EARLY_PORTING)
		return "EARLY_PORTING";
	else if(disp_helper_get_stage() == DISP_HELPER_STAGE_BRING_UP)
		return "BRINGUP";
	else if(disp_helper_get_stage() == DISP_HELPER_STAGE_NORMAL)
		return "NORMAL";
}
