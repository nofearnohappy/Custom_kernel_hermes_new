#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/workqueue.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <mach/mt_gpio.h>
#include <linux/delay.h>

#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <mach/board.h>

#define BAT_CHANGE_ALGORITHM

#ifdef BAT_CHANGE_ALGORITHM
  #include <linux/fs.h>
  #include <linux/string.h>
  #include <linux/mm.h>
  #include <linux/syscalls.h>
  #include <asm/unistd.h>
  #include <asm/uaccess.h>
  #define FILE_PATH "/sdcard/lastsoc"
  #define CPSOC  94
  #define NORMAL_CYCLE 10
#endif

#include <linux/module.h>

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/input.h>
#include <linux/ioctl.h>

#include <cust_charging.h>
#include <mach/charging.h>
#include <mach/cw2015_battery.h>


#define REG_VERSION             0x0
#define REG_VCELL               0x2
#define REG_SOC                 0x4
#define REG_RRT_ALERT           0x6
#define REG_CONFIG              0x8
#define REG_MODE                0xA
#define REG_BATINFO             0x10
#define MODE_SLEEP_MASK         (0x3<<6)
#define MODE_SLEEP              (0x3<<6)
#define MODE_NORMAL             (0x0<<6)
#define MODE_QUICK_START        (0x3<<4)
#define MODE_RESTART            (0xf<<0)

#define CONFIG_UPDATE_FLG       (0x1<<1)
#define ATHD                    (0x0<<3)        //ATHD = 0%

#define CW_I2C_SPEED            100000          // default i2c speed set 100khz
#define BATTERY_UP_MAX_CHANGE   420             // the max time allow battery change quantity
#define BATTERY_DOWN_CHANGE   60                // the max time allow battery change quantity
#define BATTERY_DOWN_MIN_CHANGE_RUN 30          // the min time allow battery change quantity when run
#define BATTERY_DOWN_MIN_CHANGE_SLEEP 1800      // the min time allow battery change quantity when run 30min

#define BATTERY_DOWN_MAX_CHANGE_RUN_AC_ONLINE 1800

#define NO_STANDARD_AC_BIG_CHARGE_MODE 1
#define BAT_LOW_INTERRUPT    1

#define USB_CHARGER_MODE        1
#define AC_CHARGER_MODE         2

static struct i2c_client *cw2015_i2c_client; /* global i2c_client to support ioctl */

#define FG_CW2015_DEBUG 1
#define FG_CW2015_TAG                  "[FG_CW2015]"
#ifdef FG_CW2015_DEBUG
#define FG_CW2015_FUN(f)               printk(KERN_ERR FG_CW2015_TAG"%s\n", __FUNCTION__)
#define FG_CW2015_ERR(fmt, args...)    printk(KERN_ERR FG_CW2015_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define FG_CW2015_LOG(fmt, args...)    printk(KERN_ERR FG_CW2015_TAG fmt, ##args)
#endif
#define I2C_BUSNUM 4
#define CW2015_DEV_NAME     "CW2015"
static const struct i2c_device_id FG_CW2015_i2c_id[] = {{CW2015_DEV_NAME,0},{}};
static struct i2c_board_info __initdata i2c_FG_CW2015={ I2C_BOARD_INFO("CW2015", 0x62)};

int g_cw2015_capacity = 0;
int g_cw2015_vol = 0;
extern int FG_charging_type;

//extern int FG_charging_status;

static u8 config_info[SIZE_BATINFO] = {
  0x17, 0xF3, 0x63, 0x6A, 0x6A, 0x68, 0x68, 0x65, 0x63, 0x60, 
  0x5B, 0x59, 0x65, 0x5B, 0x46, 0x41, 0x36, 0x31, 0x28, 0x27, 
  0x31, 0x35, 0x43, 0x51, 0x1C, 0x3B, 0x0B, 0x85, 0x22, 0x42, 
  0x5B, 0x82, 0x99, 0x92, 0x98, 0x96, 0x3D, 0x1A, 0x66, 0x45, 
  0x0B, 0x29, 0x52, 0x87, 0x8F, 0x91, 0x94, 0x52, 0x82, 0x8C, 
  0x92, 0x96, 0x54, 0xC2, 0xBA, 0xCB, 0x2F, 0x7D, 0x72, 0xA5, 
  0xB5, 0xC1, 0xA5, 0x49
};
static u8 config_info_des[SIZE_BATINFO] = {
  0x17, 0xF9, 0x6D, 0x6D, 0x6B, 0x67, 0x65, 0x64, 0x58, 0x6D, 
  0x6D, 0x48, 0x57, 0x5D, 0x4A, 0x43, 0x37, 0x31, 0x2B, 0x20, 
  0x24, 0x35, 0x44, 0x55, 0x20, 0x37, 0x0B, 0x85, 0x2A, 0x4A, 
  0x56, 0x68, 0x74, 0x6B, 0x6D, 0x6E, 0x3C, 0x1A, 0x5C, 0x45, 
  0x0B, 0x30, 0x52, 0x87, 0x8F, 0x91, 0x94, 0x52, 0x82, 0x8C, 
  0x92, 0x96, 0x64, 0xB4, 0xDB, 0xCB, 0x2F, 0x7D, 0x72, 0xA5, 
  0xB5, 0xC1, 0xA5, 0x42
};

static int liuchao_test_hmi_battery_version = 1;

static void hmi_get_battery_version()
{
    int i = 1;
    //	i = strtol(strstr(cmdline, "batversion=")+12, 0, 10);
    liuchao_test_hmi_battery_version = i; //COS = 1, DES = 2
}

static struct cw_bat_platform_data cw_bat_platdata = {
	.dc_det_pin      = 0,
        .dc_det_level    = 0,

        .bat_low_pin    = 0,
        .bat_low_level  = 0,   
        .chg_ok_pin   = 0,
        .chg_ok_level = 0,

        .is_usb_charge = 0,
        .chg_mode_sel_pin = 0,
        .chg_mode_sel_level = 0,

        .cw_bat_config_info     = config_info,
};

struct cw_battery {
        struct i2c_client *client;
        struct workqueue_struct *battery_workqueue;
        struct delayed_work battery_delay_work;
        struct delayed_work dc_wakeup_work;
        struct delayed_work bat_low_wakeup_work;
        struct cw_bat_platform_data *plat_data;

        struct power_supply rk_bat;
        struct power_supply rk_ac;
        struct power_supply rk_usb;

        long sleep_time_capacity_change;      // the sleep time from capacity change to present, it will set 0 when capacity change 
        long run_time_capacity_change;

        long sleep_time_charge_start;      // the sleep time from insert ac to present, it will set 0 when insert ac
        long run_time_charge_start;

        int dc_online;
        int usb_online;
        int charger_mode;
        int charger_init_mode;
        int capacity;
        int voltage;
        int status;
        int time_to_empty;
        int alt;

        int bat_change;
};

struct cw_battery *CW2015_obj = NULL;
//static struct cw_battery *g_CW2015_ptr = NULL;
#ifdef BAT_CHANGE_ALGORITHM
  static int PowerResetFlag = -1;
  //static int alg_run_flag = -1;
  //static int count_num = 0;
  //static int count_sp = 0;
#endif
#ifdef BAT_CHANGE_ALGORITHM

static int cw_read(struct i2c_client *client, u8 reg, u8 buf[])
{
        int ret = 0;
#if 1
	ret = i2c_smbus_read_byte_data(client,reg);
	printk("cw_read buf = %d",ret);
	if (ret < 0)
	{
        return ret;
	}
	else
	{
	    buf[0] = ret;
	    ret = 0;
	}
#else
        ret = i2c_master_reg8_recv(client, reg, buf, 1, CW_I2C_SPEED);
#endif
        return ret;
}

static int cw_write(struct i2c_client *client, u8 reg, u8 const buf[])
{
        int ret = 0;
#if 1
	ret =  i2c_smbus_write_byte_data(client,reg,buf[0]);
#else
        ret = i2c_master_reg8_send(client, reg, buf, 1, CW_I2C_SPEED);
#endif
        return ret;
}

static int cw_update_config_info(struct cw_battery *cw_bat)
{
        int ret;
        u8 reg_val;
        int i;
        u8 reset_val;
#ifdef FG_CW2015_DEBUG
		FG_CW2015_LOG("func: %s-------\n", __func__);
#else
        //dev_info(&cw_bat->client->dev, "func: %s-------\n", __func__);
#endif        
        FG_CW2015_LOG("test cw_bat_config_info = 0x%x",config_info[0]);
        /* make sure no in sleep mode */
        ret = cw_read(cw_bat->client, REG_MODE, &reg_val);
	FG_CW2015_LOG("cw_update_config_info reg_val = 0x%x",reg_val);
        if (ret < 0)
                return ret;

        reset_val = reg_val;
        if((reg_val & MODE_SLEEP_MASK) == MODE_SLEEP) {
#ifdef FG_CW2015_DEBUG
				  FG_CW2015_ERR("Error, device in sleep mode, cannot update battery info\n");
#else
                //dev_err(&cw_bat->client->dev, "Error, device in sleep mode, cannot update battery info\n");
#endif
                return -1;
        }

        /* update new battery info */
        for (i = 0; i < SIZE_BATINFO; i++) {
#ifdef FG_CW2015_DEBUG
                //FG_CW2015_LOG("cw_bat->plat_data->cw_bat_config_info[%d] = 0x%x\n", i, \
                //                cw_bat->plat_data->cw_bat_config_info[i]);
#else
                /*dev_info(&cw_bat->client->dev, "cw_bat->plat_data->cw_bat_config_info[%d] = 0x%x\n", i, \
                                cw_bat->plat_data->cw_bat_config_info[i]);*/
#endif
                ret = cw_write(cw_bat->client, REG_BATINFO + i, &config_info[i]);

                if (ret < 0) 
                        return ret;
        }

        /* readback & check */
        for (i = 0; i < SIZE_BATINFO; i++) {
                ret = cw_read(cw_bat->client, REG_BATINFO + i, &reg_val);
                if (reg_val != config_info[i])
                        return -1;
        }
        
        /* set cw2015/cw2013 to use new battery info */
        ret = cw_read(cw_bat->client, REG_CONFIG, &reg_val);
        if (ret < 0)
                return ret;

        reg_val |= CONFIG_UPDATE_FLG;   /* set UPDATE_FLAG */
        reg_val &= 0x07;                /* clear ATHD */
        reg_val |= ATHD;                /* set ATHD */
        ret = cw_write(cw_bat->client, REG_CONFIG, &reg_val);
        if (ret < 0)
                return ret;

        /* check 2015/cw2013 for ATHD & update_flag */ 
        ret = cw_read(cw_bat->client, REG_CONFIG, &reg_val);
        if (ret < 0)
                return ret;
        
        if (!(reg_val & CONFIG_UPDATE_FLG)) {
#ifdef FG_CW2015_DEBUG
                FG_CW2015_LOG("update flag for new battery info have not set..\n");
#else
                //dev_info(&cw_bat->client->dev, "update flag for new battery info have not set..\n");
#endif
        }

        if ((reg_val & 0xf8) != ATHD) {
#ifdef FG_CW2015_DEBUG
                FG_CW2015_LOG("the new ATHD have not set..\n");
#else
                //dev_info(&cw_bat->client->dev, "the new ATHD have not set..\n");
#endif
        }

        /* reset */
        reset_val &= ~(MODE_RESTART);
        reg_val = reset_val | MODE_RESTART;
        ret = cw_write(cw_bat->client, REG_MODE, &reg_val);
        if (ret < 0)
                return ret;

        msleep(10);
        ret = cw_write(cw_bat->client, REG_MODE, &reset_val);
        if (ret < 0)
                return ret;
#ifdef  BAT_CHANGE_ALGORITHM
        PowerResetFlag = 1;
        FG_CW2015_ERR("cw2015_file_test  set PowerResetFlag/n ");
#endif
		msleep(10);
        
        return 0;
}

static int cw_init(struct cw_battery *cw_bat)
{
    int ret;
    int i;
    u8 reg_val = MODE_SLEEP;
    static struct devinfo_struct *devinfo_bat = NULL;
    
    hmi_get_battery_version();
    
    //devinfo_bat = kzalloc(sizeof(struct devinfo_struct), GFP_KERNEL);    
    //devinfo_bat->device_type = "BATTERY";
    //devinfo_bat->device_vendor = DEVINFO_NULL;
    //devinfo_bat->device_ic = DEVINFO_NULL;
    //devinfo_bat->device_version = DEVINFO_NULL;
    //devinfo_bat->device_info = DEVINFO_NULL;
    //devinfo_bat->device_used = DEVINFO_USED;
        
    //if (!devinfo_check_add_device(devinfo_bat)){
        //kfree(devinfo_bat);
        //dev_info(&cw_bat->client->dev, "free devinfo for not register into devinfo list .\n");
    //}else{
        //dev_info(&cw_bat->client->dev, "register devinfo into devinfo list .\n");
    //}
    if ((reg_val & MODE_SLEEP_MASK) == MODE_SLEEP) {
        reg_val = MODE_NORMAL;
        ret = cw_write(cw_bat->client, REG_MODE, &reg_val);
        if (ret < 0)
            return ret;
    }
    ret = cw_read(cw_bat->client, REG_CONFIG, &reg_val);
    if (ret < 0)
        return ret;
    
    ret = cw_read(cw_bat->client, REG_CONFIG, &reg_val);
    if (ret < 0)
        return ret;
    #ifdef FG_CW2015_DEBUG
    FG_CW2015_LOG("the new ATHD have not set reg_val = 0x%x\n",reg_val);
    #endif
    if ((reg_val & 0xf8) != ATHD) {
        #ifdef FG_CW2015_DEBUG
        FG_CW2015_LOG("the new ATHD have not set\n");
        #endif
        reg_val &= 0x07;    /* clear ATHD */
        reg_val |= ATHD;    /* set ATHD */
        ret = cw_write(cw_bat->client, REG_CONFIG, &reg_val);
        FG_CW2015_LOG("cw_init 1111\n");
        if (ret < 0)
            return ret;
    }
    
    ret = cw_read(cw_bat->client, REG_CONFIG, &reg_val);
    if (ret < 0) 
        return ret;
    FG_CW2015_LOG("cw_init REG_CONFIG = %d\n",reg_val);
    
    if (!(reg_val & CONFIG_UPDATE_FLG)) {
        #ifdef FG_CW2015_DEBUG
        FG_CW2015_LOG("update flag for new battery info have not set\n");
        #endif
        ret = cw_update_config_info(cw_bat);
        if (ret < 0)
            return ret;
    } else {
        for(i = 0; i < SIZE_BATINFO; i++) { 
            ret = cw_read(cw_bat->client, (REG_BATINFO + i), &reg_val);
            if (ret < 0)
                return ret;
            
            if (2 == liuchao_test_hmi_battery_version){
                if (config_info_des[i] != reg_val)
                    break;
            
            }else{
                if (config_info[i] != reg_val)
                    break;
            }
        }
        
        if (i != SIZE_BATINFO) {
            #ifdef FG_CW2015_DEBUG
            FG_CW2015_LOG("update flag for new battery info have not set\n"); 
            #endif
            ret = cw_update_config_info(cw_bat);
            if (ret < 0)
                return ret;
        }
    }
    
    for (i = 0; i < 30; i++) {
        ret = cw_read(cw_bat->client, REG_SOC, &reg_val);
        if (ret < 0)
            return ret;
        else if (reg_val <= 0x64) 
            break;
        
        msleep(100);
        if (i > 25){
            #ifdef FG_CW2015_DEBUG
            FG_CW2015_ERR("cw2015/cw2013 input unvalid power error\n");
            #endif
        }
        
    }
    if (i >=30){
        reg_val = MODE_SLEEP;
        ret = cw_write(cw_bat->client, REG_MODE, &reg_val);
        #ifdef FG_CW2015_DEBUG
        FG_CW2015_ERR("cw2015/cw2013 input unvalid power error_2\n");
        #endif
        return -1;
    } 
    return 0;
}

static void cw_update_time_member_charge_start(struct cw_battery *cw_bat)
{
        struct timespec ts;
        int new_run_time;
        int new_sleep_time;

        ktime_get_ts(&ts);
        new_run_time = ts.tv_sec;

        get_monotonic_boottime(&ts);
        new_sleep_time = ts.tv_sec - new_run_time;

        cw_bat->run_time_charge_start = new_run_time;
        cw_bat->sleep_time_charge_start = new_sleep_time; 
}

static int rk_usb_update_online(struct cw_battery *cw_bat)
{
        int ret = 0;
        int usb_status = 0;

	//FG_CW2015_LOG("rk_usb_update_online FG_charging_status = %d\n",FG_charging_status);
	
	#if 0	 
        if (cw_bat->plat_data->is_usb_charge == 0) {
                cw_bat->usb_online = 0;
                return 0;

        }
        #endif
		
        //usb_status = get_usb_charge_state(cw_bat);        
        if (usb_status == 2) {
                if (cw_bat->charger_mode != AC_CHARGER_MODE) {
                        cw_bat->charger_mode = AC_CHARGER_MODE;
                        ret = 1;
                }
//                if (cw_bat->plat_data->chg_mode_sel_pin != INVALID_GPIO) {
//                        if (gpio_get_value (cw_bat->plat_data->chg_mode_sel_pin) != cw_bat->plat_data->chg_mode_sel_level)
//                                gpio_direction_output(cw_bat->plat_data->chg_mode_sel_pin, (cw_bat->plat_data->chg_mode_sel_level==GPIO_HIGH) ? GPIO_HIGH : GPIO_LOW);
//                }
                
                if (cw_bat->usb_online != 1) {
                        cw_bat->usb_online = 1;
                        cw_update_time_member_charge_start(cw_bat);
                }
                
        } else if (usb_status == 1) {
                if (cw_bat->charger_mode != USB_CHARGER_MODE) {
                        cw_bat->charger_mode = USB_CHARGER_MODE;
                        ret = 1;
                }
                
//                if (cw_bat->plat_data->chg_mode_sel_pin != INVALID_GPIO) {
//                        if (gpio_get_value (cw_bat->plat_data->chg_mode_sel_pin) == cw_bat->plat_data->chg_mode_sel_level)
//                                gpio_direction_output(cw_bat->plat_data->chg_mode_sel_pin, (cw_bat->plat_data->chg_mode_sel_level==GPIO_HIGH) ? GPIO_LOW : GPIO_HIGH);
//                }
                if (cw_bat->usb_online != 1){
                        cw_bat->usb_online = 1;
                        cw_update_time_member_charge_start(cw_bat);
                }

        } else if (usb_status == 0 && cw_bat->usb_online != 0) {

//                if (cw_bat->plat_data->chg_mode_sel_pin != INVALID_GPIO) {
//                        if (gpio_get_value (cw_bat->plat_data->chg_mode_sel_pin == cw_bat->plat_data->chg_mode_sel_level))
//                                gpio_direction_output(cw_bat->plat_data->chg_mode_sel_pin, (cw_bat->plat_data->chg_mode_sel_level==GPIO_HIGH) ? GPIO_LOW : GPIO_HIGH);
//                }

//                if (cw_bat->usb_online == 0)
                        cw_bat->charger_mode = 0;

                cw_update_time_member_charge_start(cw_bat);
                cw_bat->usb_online = 0;
                ret = 1;
        }

        return ret;
}

static void cw_bat_work(struct work_struct *work)
{
        struct delayed_work *delay_work;
        struct cw_battery *cw_bat;
        int ret;
	FG_CW2015_FUN(); 
	printk("cw_bat_work\n");

        delay_work = container_of(work, struct delayed_work, work);
        cw_bat = container_of(delay_work, struct cw_battery, battery_delay_work);
	printk("cw_bat_work 111\n");
        ret = rk_usb_update_online(cw_bat);
        if (ret == 1) {
		printk("cw_bat_work 222\n");
                //power_supply_changed(&cw_bat->rk_ac);
        }
	printk("cw_bat_work 333\n");

	//FG_CW2015_LOG("cw_bat_work FG_charging_status = %d\n",FG_charging_status);
	//cw_bat->usb_online = FG_charging_status;
        if (cw_bat->usb_online == 1) {
		printk("cw_bat_work 444\n");
                ret = rk_usb_update_online(cw_bat);
                if (ret == 1) {
				printk("cw_bat_work 555\n");
                        //power_supply_changed(&cw_bat->rk_usb);     
                        //power_supply_changed(&cw_bat->rk_ac);
                }
        }

	printk("cw_bat_work 666\n");
	g_cw2015_capacity = cw_bat->capacity;
	g_cw2015_vol = cw_bat->voltage;
	printk("cw_bat_work 777 vol = %d,cap = %d\n",cw_bat->voltage,cw_bat->capacity);
        if (cw_bat->bat_change) {
		printk("cw_bat_work 888\n");
                //power_supply_changed(&cw_bat->rk_bat); 
                cw_bat->bat_change = 0;
        }
	 queue_delayed_work(cw_bat->battery_workqueue, &cw_bat->battery_delay_work, msecs_to_jiffies(1000));
/*
        queue_delayed_work(cw_bat->battery_workqueue, &cw_bat->battery_delay_work, msecs_to_jiffies(1000));
#ifdef FG_CW2015_DEBUG
        FG_CW2015_LOG("cw_bat->bat_change = %d, cw_bat->time_to_empty = %d, cw_bat->capacity = %d, cw_bat->voltage = %d, cw_bat->dc_online = %d, cw_bat->usb_online = %d\n",\
                        cw_bat->bat_change, cw_bat->time_to_empty, cw_bat->capacity, cw_bat->voltage, cw_bat->dc_online, cw_bat->usb_online);
#else
        //dev_dbg(&cw_bat->client->dev, "cw_bat->bat_change = %d, cw_bat->time_to_empty = %d, cw_bat->capacity = %d, cw_bat->voltage = %d, cw_bat->dc_online = %d, cw_bat->usb_online = %d\n",\
       //                 cw_bat->bat_change, cw_bat->time_to_empty, cw_bat->capacity, cw_bat->voltage, cw_bat->dc_online, cw_bat->usb_online);
        
#endif
*/
}

/*----------------------------------------------------------------------------*/
static int cw2015_i2c_detect(struct i2c_client *client, struct i2c_board_info *info) 
{    
	FG_CW2015_FUN(); 
	printk("cw2015_i2c_detect\n");
	
	strcpy(info->type, CW2015_DEV_NAME);
	return 0;
}

/*----------------------------------------------------------------------------*/

static int cw2015_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
        struct cw_battery *cw_bat;
        int ret;
        //int irq;
        //int irq_flags;
        int loop = 0;

	FG_CW2015_FUN(); 
	printk("cw2015_i2c_probe\n");
        //cw_bat = devm_kzalloc(&client->dev, sizeof(*cw_bat), GFP_KERNEL);
		 cw_bat = kzalloc(sizeof(struct cw_battery), GFP_KERNEL);
        if (!cw_bat) {
#ifdef FG_CW2015_DEBUG
                FG_CW2015_ERR("fail to allocate memory\n");
#else
                //dev_err(&cw_bat->client->dev, "fail to allocate memory\n");
#endif
                return -ENOMEM;
        }
				
		 //memset(data, 0, sizeof(*cw_bat));
		 
        i2c_set_clientdata(client, cw_bat);
        cw_bat->plat_data = client->dev.platform_data;
/*
        ret = cw_bat_gpio_init(cw_bat);
        if (ret) {
#ifdef FG_CW2015_DEBUG
                FG_CW2015_ERR("cw_bat_gpio_init error\n");
#else
                //dev_err(&cw_bat->client->dev, "cw_bat_gpio_init error\n");
#endif
                return ret;
        }
*/
        cw_bat->client = client;
	cw_bat->plat_data = &cw_bat_platdata;

        ret = cw_init(cw_bat);
        while ((loop++ < 2000) && (ret != 0)) {
                ret = cw_init(cw_bat);
        }

        if (ret) 
                return ret;
/*        
        cw_bat->rk_bat.name = "rk-bat";
        cw_bat->rk_bat.type = POWER_SUPPLY_TYPE_BATTERY;
        cw_bat->rk_bat.properties = rk_battery_properties;
        cw_bat->rk_bat.num_properties = ARRAY_SIZE(rk_battery_properties);
        cw_bat->rk_bat.get_property = rk_battery_get_property;
        ret = power_supply_register(&client->dev, &cw_bat->rk_bat);
        if(ret < 0) {
#ifdef FG_CW2015_DEBUG
                FG_CW2015_ERR("power supply register rk_bat error\n");
#else
                //dev_err(&cw_bat->client->dev, "power supply register rk_bat error\n");
#endif
                goto rk_bat_register_fail;
        }

        cw_bat->rk_ac.name = "rk-ac";
        cw_bat->rk_ac.type = POWER_SUPPLY_TYPE_MAINS;
        cw_bat->rk_ac.properties = rk_ac_properties;
        cw_bat->rk_ac.num_properties = ARRAY_SIZE(rk_ac_properties);
        cw_bat->rk_ac.get_property = rk_ac_get_property;
        ret = power_supply_register(&client->dev, &cw_bat->rk_ac);
        if(ret < 0) {
#ifdef FG_CW2015_DEBUG
                FG_CW2015_ERR("power supply register rk_ac error\n");
#else
                //dev_err(&cw_bat->client->dev, "power supply register rk_ac error\n");
#endif
                goto rk_ac_register_fail;
        }

        cw_bat->rk_usb.name = "rk-usb";
        cw_bat->rk_usb.type = POWER_SUPPLY_TYPE_USB;
        cw_bat->rk_usb.properties = rk_usb_properties;
        cw_bat->rk_usb.num_properties = ARRAY_SIZE(rk_usb_properties);
        cw_bat->rk_usb.get_property = rk_usb_get_property;
        ret = power_supply_register(&client->dev, &cw_bat->rk_usb);
        if(ret < 0) {
#ifdef FG_CW2015_DEBUG
                FG_CW2015_ERR("power supply register rk_ac error\n");
#else
                //dev_err(&cw_bat->client->dev, "power supply register rk_ac error\n");
#endif
                goto rk_usb_register_fail;
        }

        cw_bat->charger_init_mode = dwc_otg_check_dpdm();
*/
        cw_bat->dc_online = 0;
        cw_bat->usb_online = 0;
        cw_bat->charger_mode = 0;
        cw_bat->capacity = 1;
        cw_bat->voltage = 0;
        cw_bat->status = 0;
        cw_bat->time_to_empty = 0;
        cw_bat->bat_change = 0;

        cw_update_time_member_charge_start(cw_bat);

	cw_bat->battery_workqueue = create_singlethread_workqueue("rk_battery");
	INIT_DELAYED_WORK(&cw_bat->battery_delay_work, cw_bat_work);
	queue_delayed_work(cw_bat->battery_workqueue, &cw_bat->battery_delay_work, msecs_to_jiffies(10));
/*
        cw_bat->battery_workqueue = create_singlethread_workqueue("rk_battery");
        INIT_DELAYED_WORK(&cw_bat->battery_delay_work, cw_bat_work);
        INIT_DELAYED_WORK(&cw_bat->dc_wakeup_work, dc_detect_do_wakeup);
        queue_delayed_work(cw_bat->battery_workqueue, &cw_bat->battery_delay_work, msecs_to_jiffies(10));

#if 0
        if (cw_bat->plat_data->dc_det_pin != INVALID_GPIO) {
                irq = gpio_to_irq(cw_bat->plat_data->dc_det_pin);
                irq_flags = gpio_get_value(cw_bat->plat_data->dc_det_pin) ? IRQF_TRIGGER_FALLING : IRQF_TRIGGER_RISING;
                ret = request_irq(irq, dc_detect_irq_handler, irq_flags, "usb_detect", cw_bat);
                if (ret < 0) {
#ifdef FG_CW2015_DEBUG
                        FG_CW2015_ERR("request_irq failed\n",);
#else
                        //pr_err("%s: request_irq(%d) failed\n", __func__, irq);
#endif
                }
                enable_irq_wake(irq);
        }
#else
	mt_set_gpio_dir(GPIO_ALS_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_mode(GPIO_ALS_EINT_PIN, GPIO_ALS_EINT_PIN_M_EINT);
	mt_set_gpio_pull_enable(GPIO_ALS_EINT_PIN, TRUE);
	mt_set_gpio_pull_select(GPIO_ALS_EINT_PIN, GPIO_PULL_UP);

	mt_eint_set_hw_debounce(CUST_EINT_ALS_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_ALS_NUM, CUST_EINT_ALS_TYPE, apds9960_interrupt, 0);

	mt_eint_unmask(CUST_EINT_ALS_NUM);  

#endif
#ifdef BAT_LOW_INTERRUPT
        INIT_DELAYED_WORK(&cw_bat->bat_low_wakeup_work, bat_low_detect_do_wakeup);
        wake_lock_init(&bat_low_wakelock, WAKE_LOCK_SUSPEND, "bat_low_detect");
        if (cw_bat->plat_data->bat_low_pin != INVALID_GPIO) {
                irq = gpio_to_irq(cw_bat->plat_data->bat_low_pin);
                ret = request_irq(irq, bat_low_detect_irq_handler, IRQF_TRIGGER_RISING, "bat_low_detect", cw_bat);
                if (ret < 0) {
                        gpio_free(cw_bat->plat_data->bat_low_pin);
                }
                enable_irq_wake(irq);
        }
#endif 
*/
#ifdef FG_CW2015_DEBUG
        FG_CW2015_LOG("cw2015/cw2013 driver v1.2 probe sucess\n");
#else
        //dev_info(&cw_bat->client->dev, "cw2015/cw2013 driver v1.2 probe sucess\n");
#endif
        return 0;

//rk_usb_register_fail:
        power_supply_unregister(&cw_bat->rk_bat);
//rk_ac_register_fail:
        power_supply_unregister(&cw_bat->rk_ac);
//rk_bat_register_fail:
#ifdef FG_CW2015_DEBUG
        FG_CW2015_LOG("cw2015/cw2013 driver v1.2 probe error!!!!\n");
#else
        //dev_info(&cw_bat->client->dev, "cw2015/cw2013 driver v1.2 probe error!!!!\n");
#endif
        return ret;
}

static int cw2015_i2c_remove(struct i2c_client *client)
{
	struct cw_battery *data = i2c_get_clientdata(client);

	FG_CW2015_FUN(); 
	printk("cw2015_i2c_remove\n");
	
	//__cancel_delayed_work(&data->battery_delay_work);
	cancel_delayed_work(&data->battery_delay_work);
	cw2015_i2c_client = NULL;
	i2c_unregister_device(client);
	kfree(data);

	return 0;
}

static int cw2015_i2c_suspend(struct i2c_client *client, pm_message_t mesg)
{

  struct cw_battery *cw_bat = i2c_get_clientdata(client);
  
  FG_CW2015_FUN(); 
	printk("cw2015_i2c_suspend\n");
  cancel_delayed_work(&cw_bat->battery_delay_work);

	return 0;
}

static int cw2015_i2c_resume(struct i2c_client *client)
{
	struct cw_battery *cw_bat = i2c_get_clientdata(client);
	
	FG_CW2015_FUN(); 
	printk("cw2015_i2c_resume\n");
  queue_delayed_work(cw_bat->battery_workqueue, &cw_bat->battery_delay_work, msecs_to_jiffies(100));

	return 0;
}

static struct i2c_driver cw2015_i2c_driver = {	
	.probe      = cw2015_i2c_probe,
	.remove     = cw2015_i2c_remove,
	.detect     = cw2015_i2c_detect,
	.suspend    = cw2015_i2c_suspend,
	.resume     = cw2015_i2c_resume,
	.id_table   = FG_CW2015_i2c_id,
	.driver = {
//		.owner          = THIS_MODULE,
		.name           = CW2015_DEV_NAME,
	},
};

/*----------------------------------------------------------------------------*/
static int __init cw_bat_init(void)
{
	//return i2c_add_driver(&cw_bat_driver);
	FG_CW2015_LOG("%s: \n", __func__); 
	printk("cw_bat_init\n");
	
	i2c_register_board_info(I2C_BUSNUM, &i2c_FG_CW2015, 1);
	printk("cw_bat_init 111\n");

	if(i2c_add_driver(&cw2015_i2c_driver))
	{
		FG_CW2015_ERR("add driver error\n");
		printk("cw_bat_init add driver error\n");
		return -1;
	}
#endif
	printk("cw_bat_init 222\n");
	return 0;
}

static void __exit cw_bat_exit(void)
{
	FG_CW2015_LOG("%s: \n", __func__); 
	printk("cw_bat_exit\n");
        //i2c_del_driver(&i2c_FG_CW2015);
}

module_init(cw_bat_init);
module_exit(cw_bat_exit);

MODULE_AUTHOR("xhc<xhc@rock-chips.com>");
MODULE_DESCRIPTION("cw2015/cw2013 battery driver");
MODULE_LICENSE("GPL");

