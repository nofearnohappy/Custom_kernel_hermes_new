#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/poll.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/kobject.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#endif

#include <mach/mt_pwm.h>
#include <mach/mt_pwm_hal.h>
#include <mach/mt_gpio.h>
#include <cust_gpio_usage.h>

#include "consumerir.h"

#ifdef GPIO_IRTX_OUT_PIN
#define CONSUMERIR_GPIO GPIO_IRTX_OUT_PIN
#else 
#ifdef GPIO_ANT_SEL0
#define CONSUMERIR_GPIO GPIO_ANT_SEL0
#else
#define CONSUMERIR_GPIO GPIO19
#endif
#endif

struct mt_consumerir mt_consumerir_dev;
void __iomem *consumerir_reg_base;
unsigned int consumerir_irq;

struct pwm_spec_config consumerir_pwm_config = {
    .pwm_no = 0,
    .mode = PWM_MODE_MEMORY,
    .clk_div = CLK_DIV1,
    .clk_src = PWM_CLK_NEW_MODE_BLOCK,
    .pmic_pad = 0,
    .PWM_MODE_MEMORY_REGS.IDLE_VALUE = IDLE_FALSE,
    .PWM_MODE_MEMORY_REGS.GUARD_VALUE = GUARD_FALSE,
    .PWM_MODE_MEMORY_REGS.STOP_BITPOS_VALUE = 31,
    .PWM_MODE_MEMORY_REGS.HDURATION = 25, // 1 microseconds, assume clock source is 26M
    .PWM_MODE_MEMORY_REGS.LDURATION = 25,
    .PWM_MODE_MEMORY_REGS.GDURATION = 0,
    .PWM_MODE_MEMORY_REGS.WAVE_NUM = 1,
};

static int dev_char_open(struct inode *inode, struct file *file)
{

#ifdef GPIO_IRTX_OUT_PIN
pr_warning("[CONSUMERIR] open GPIO_CONSUMERIR_OUT_PIN defined\n");
#else 
    #ifdef GPIO_ANT_SEL0
        pr_warning("[CONSUMERIR] open GPIO_ANT_SEL0 defined\n");
    #else
        pr_warning("[CONSUMERIR] open GPIO19 defined\n");
    #endif
#endif

    if(atomic_read(&mt_consumerir_dev.usage_cnt))
        return -EBUSY;

    pr_warning("[CONSUMERIR] open by %s\n", current->comm);
    consumerir_write32(mt_consumerir_dev.reg_base, CONSUMERIRCFG, consumerir_read32(mt_consumerir_dev.reg_base, CONSUMERIRCFG) | 0x80); //set IRTX_IRINV
    nonseekable_open(inode,file);
    atomic_inc(&mt_consumerir_dev.usage_cnt);
    return 0;
}

static int dev_char_close(struct inode *inode, struct file *file)
{
    pr_warning("[consumerir] close by %s\n", current->comm);
    atomic_dec(&mt_consumerir_dev.usage_cnt);
    return 0;
}

static ssize_t dev_char_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
    return 0;
}

static long dev_char_ioctl( struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    unsigned int para = 0, gpio_id = -1, en = 0;
    unsigned long mode = 0, dir = 0, outp = 0;

    switch(cmd) {
        case CONSUMERIR_IOC_SET_CARRIER_FREQ:
            if(copy_from_user(&mt_consumerir_dev.carrier_freq, (void __user *)arg, sizeof(unsigned int))) {
                pr_err("[CONSUMERIR] CONSUMERIR_IOC_SET_CARRIER_FREQ: copy_from_user fail!\n");
                ret = -EFAULT;
            } else {
                pr_warning("[CONSUMERIR] CONSUMERIR_IOC_SET_CARRIER_FREQ: %d\n", mt_consumerir_dev.carrier_freq);
                if(!mt_consumerir_dev.carrier_freq) {
                    ret = -EINVAL;
                    mt_consumerir_dev.carrier_freq = 38000;
                }
            }
        break;

        case CONSUMERIR_IOC_SET_CONSUMERIR_LED_EN:
            if(copy_from_user(&para, (void __user *)arg, sizeof(unsigned int))) {
                pr_err("[CONSUMERIR] CONSUMERIR_IOC_SET_CONSUMERIR_LED_EN: copy_from_user fail!\n");
                ret = -EFAULT;
            } else {
                // en: bit 12;
                // gpio: bit 0-11
                gpio_id = (unsigned long)((para & 0x0FFF0000) > 16);
                en = (para & 0xF);
                pr_warning("[CONSUMERIR] CONSUMERIR_IOC_SET_CONSUMERIR_LED_EN: 0x%x, gpio_id:%ul, en:%ul\n", para, gpio_id, en);
                if (en) {
                    mode = GPIO_MODE_02;
                    dir = GPIO_DIR_OUT;
                    outp = GPIO_OUT_ZERO;    // Low means enable LED
                } else {
                    mode = GPIO_MODE_00;
                    dir = GPIO_DIR_OUT;
                    outp = GPIO_OUT_ONE;  // High means disable LED
                }
                gpio_id = CONSUMERIR_GPIO;

                mt_set_gpio_mode(gpio_id, mode);
                mt_set_gpio_dir(gpio_id, dir);
                mt_set_gpio_out(gpio_id, outp);

                pr_warning("[CONSUMERIR] CONSUMERIR_IOC_SET_CONSUMERIR_LED_EN: gpio:0x%xl, mode:%d\n", gpio_id, mt_get_gpio_mode(gpio_id));
                pr_warning("[CONSUMERIR] CONSUMERIR_IOC_SET_CONSUMERIR_LED_EN: gpio:0x%xl, dir:%d\n", gpio_id, mt_get_gpio_dir(gpio_id));
                pr_warning("[CONSUMERIR] CONSUMERIR_IOC_SET_CONSUMERIR_LED_EN: gpio:0x%xl, out:%d\n", gpio_id, mt_get_gpio_out(gpio_id));
            }

        break;

        default:
            pr_err("[CONSUMERIR] unknown ioctl cmd 0x%x\n", cmd);
            ret = -ENOTTY;
            break;
    }
    return ret;
}

static void set_consumerir_pwm(void)
{
    unsigned int ir_conf_wr;
    unsigned int L0H,L0L,L1H,L1L;
    unsigned int sync_h,sync_l;
    unsigned int cdt,cwt;
    struct consumerir_config ir_conf;

    pr_warning("[CONSUMERIR] configure CONSUMERIR software mode\n");

    ir_conf.mode = 3;
    ir_conf.start = 0;
    ir_conf.sw_o = 0;
    ir_conf.b_ord = 1; // LSB first
    ir_conf.r_ord = 0; // R0 first
    ir_conf.ir_os = 1; // modulated signal
    ir_conf.ir_inv = 1;
    ir_conf.bit_num = 0;
    ir_conf.data_inv = 0;
    L0H = 0;
    L0L = (mt_consumerir_dev.pwm_ch+1) & 0x7; // FIXME, workaround for Denali, HW will fix on Jade
    L1H = 0;
    L1L = 0;
    sync_h = 0;
    sync_l = 0;
    cwt = (CLOCK_SRC*1000*1000)/(mt_consumerir_dev.carrier_freq); // carrier freq.
    cdt = cwt/3; // duty=1/3

    memcpy(&ir_conf_wr, &ir_conf, sizeof(ir_conf));
    consumerir_write32(mt_consumerir_dev.reg_base, CONSUMERIRCFG, ir_conf_wr);
    consumerir_write32(mt_consumerir_dev.reg_base, CONSUMERIR_L0H, L0H);
    consumerir_write32(mt_consumerir_dev.reg_base, CONSUMERIR_L0L, L0L);
    consumerir_write32(mt_consumerir_dev.reg_base, CONSUMERIR_L1H, L1H);
    consumerir_write32(mt_consumerir_dev.reg_base, CONSUMERIR_L1L, L1L);
    consumerir_write32(mt_consumerir_dev.reg_base, CONSUMERIRSYNCH, sync_h);
    consumerir_write32(mt_consumerir_dev.reg_base, CONSUMERIRSYNCL, sync_l);
    consumerir_write32(mt_consumerir_dev.reg_base, CONSUMERIRMT, (cdt<<16)|(cwt&0xFFFF));
    
    pr_warning("[CONSUMERIR] configured CONSUMERIR: cfg=%x L0=%x/%x L1=%x/%x sync=%x/%x mt=%x/%x\n", 
        ir_conf_wr, L0H, L0L, L1H, L1L, sync_h, sync_l, cdt, cwt);
    pr_warning("[CONSUMERIR] configured cfg=0x%x", (unsigned int)consumerir_read32(mt_consumerir_dev.reg_base, CONSUMERIRCFG));
}

static ssize_t dev_char_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    dma_addr_t wave_phy;
    void *wave_vir;
    int ret;
    int buf_size = (count + 3) / 4;  // when count is 5...

    pr_warning("[CONSUMERIR] CONSUMERIR write len=0x%x, pwm=%d\n", (unsigned int)count, (unsigned int)consumerir_pwm_config.pwm_no);
    wave_vir = dma_alloc_coherent(&mt_consumerir_dev.plat_dev->dev, count, &wave_phy, GFP_KERNEL);
    if(!wave_vir) {
        pr_err("[CONSUMERIR] alloc memory fail\n");
        return -ENOMEM;
    }
    ret = copy_from_user(wave_vir, buf, count);
    if(ret) {
        pr_err("[CONSUMERIR] write, copy from user fail %d\n", ret);
        goto exit;
    }
    
    mt_set_intr_enable(0);
    mt_set_intr_enable(1);
    mt_pwm_26M_clk_enable_hal(1);
    pr_warning("[CONSUMERIR] CONSUMERIR before read CONSUMERIRCFG:0x%x\n", (consumerir_read32(mt_consumerir_dev.reg_base, CONSUMERIRCFG)));
    consumerir_pwm_config.PWM_MODE_MEMORY_REGS.BUF0_BASE_ADDR = (U32 *)wave_phy;
    consumerir_pwm_config.PWM_MODE_MEMORY_REGS.BUF0_SIZE = (buf_size ? (buf_size -1) : 0);

    set_consumerir_pwm();
    mt_set_gpio_mode(CONSUMERIR_GPIO, GPIO_MODE_02);

    consumerir_write32(mt_consumerir_dev.reg_base, CONSUMERIRCFG, consumerir_read32(mt_consumerir_dev.reg_base, CONSUMERIRCFG)|0x1); //STRT=1
    
    mt_set_intr_ack(0);
    mt_set_intr_ack(1);
    ret = pwm_set_spec_config(&consumerir_pwm_config);
    pr_warning("[CONSUMERIR] pwm is triggered, %d\n", ret);

    msleep(count * 8 / 1000);
    msleep(100);
    ret = count;

exit:
    pr_warning("[CONSUMERIR] done, clean up\n");
    dma_free_coherent(&mt_consumerir_dev.plat_dev->dev, count, wave_vir, wave_phy);
    consumerir_write32(mt_consumerir_dev.reg_base, CONSUMERIRCFG, consumerir_read32(mt_consumerir_dev.reg_base, CONSUMERIRCFG)&0xFFFFFFF7); //SWO=0
    consumerir_write32(mt_consumerir_dev.reg_base, CONSUMERIRCFG, consumerir_read32(mt_consumerir_dev.reg_base, CONSUMERIRCFG)&0xFFFFFFFE); //STRT=0
    mt_pwm_disable(consumerir_pwm_config.pwm_no, consumerir_pwm_config.pmic_pad);

    mt_set_gpio_mode(CONSUMERIR_GPIO, GPIO_MODE_00);
    mt_set_gpio_dir(CONSUMERIR_GPIO, GPIO_DIR_OUT);
    mt_set_gpio_out(CONSUMERIR_GPIO, GPIO_OUT_ONE);

    return ret;
}

static irqreturn_t consumerir_isr(int irq, void *data)
{
    return IRQ_HANDLED;
}

#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))
static u64 consumerir_dma_mask = DMA_BIT_MASK((sizeof(unsigned long)<<3)); // TODO: 3?

static struct file_operations char_dev_fops = {
    .owner = THIS_MODULE,
    .open = &dev_char_open,
    .read = &dev_char_read,
    .write = &dev_char_write,
    .release = &dev_char_close,
    .unlocked_ioctl = &dev_char_ioctl,
};

//my edit start
#define consumerir_driver_name "consumerir"
static int consumerir_probe(struct platform_device *plat_dev)
{
    struct cdev *c_dev;
    dev_t dev_t_consumerir;
    struct device *dev = NULL;
    static void *dev_class;
    u32 major = 0, minor = 0;
    int ret = 0;
    unsigned int gpio_id = -1;
    int error = 0;

    gpio_id = CONSUMERIR_GPIO;

    // mt_set_gpio_mode(gpio_id, GPIO_MODE_02);

#ifdef CONFIG_OF
    if(plat_dev->dev.of_node == NULL) {
        pr_err("[CONSUMERIR] consumerir OF node is NULL\n");
        return -1;
    }

    mt_set_gpio_mode(gpio_id, GPIO_MODE_00);
    mt_set_gpio_dir(gpio_id, GPIO_DIR_OUT);
    mt_set_gpio_out(gpio_id, GPIO_OUT_ONE);

    of_property_read_u32(plat_dev->dev.of_node, "major", &major);
    mt_consumerir_dev.reg_base = of_iomap(plat_dev->dev.of_node, 0);
    mt_consumerir_dev.irq = irq_of_parse_and_map(plat_dev->dev.of_node, 0);
    of_property_read_u32(plat_dev->dev.of_node, "pwm_ch", &mt_consumerir_dev.pwm_ch);
    pr_warning("[CONSUMERIR] device tree info: major=%d base=0x%p irq=%d pwm=%d\n", 
        major, mt_consumerir_dev.reg_base, mt_consumerir_dev.irq, mt_consumerir_dev.pwm_ch);
#endif

    if (!major) {
        error = alloc_chrdev_region(&dev_t_consumerir, 0, 1, consumerir_driver_name);
        if (!error) {
            major = MAJOR(dev_t_consumerir);
            minor = MINOR(dev_t_consumerir);
        }
    } else {
        dev_t_consumerir = MKDEV(major, minor);
    }

    ret = request_irq(mt_consumerir_dev.irq, consumerir_isr, IRQF_TRIGGER_FALLING, "consumerir", NULL); // TODO: trigger
    if(ret) {
        pr_err("[CONSUMERIR] request IRQ(%d) fail ret=%d\n", mt_consumerir_dev.irq, ret);
        goto exit;
    }

    consumerir_pwm_config.pwm_no = mt_consumerir_dev.pwm_ch;

    mt_consumerir_dev.plat_dev = plat_dev;
    mt_consumerir_dev.plat_dev->dev.dma_mask = &consumerir_dma_mask;
    mt_consumerir_dev.plat_dev->dev.coherent_dma_mask = consumerir_dma_mask;
    atomic_set(&mt_consumerir_dev.usage_cnt, 0);
    mt_consumerir_dev.carrier_freq = 38000; // NEC as default

    ret = register_chrdev_region(dev_t_consumerir, 1, consumerir_driver_name);
    if(ret) {
        pr_err("[CONSUMERIR] register_chrdev_region fail ret=%d\n", ret);
        goto exit;
    }
    c_dev = kmalloc(sizeof(struct cdev), GFP_KERNEL);
    if(!c_dev) {
        pr_err("[CONSUMERIR] kmalloc cdev fail\n");
        goto exit;
    }
    cdev_init(c_dev, &char_dev_fops);
    c_dev->owner = THIS_MODULE;
    ret = cdev_add(c_dev, dev_t_consumerir, 1);
    if(ret) {
        pr_err("[CONSUMERIR] cdev_add fail ret=%d\n", ret);
        goto exit;
    }
    dev_class = class_create(THIS_MODULE, consumerir_driver_name);
    dev = device_create(dev_class, NULL, dev_t_consumerir, NULL, "consumerir");
    if(IS_ERR(dev)) {
        ret = PTR_ERR(dev);
        pr_err("[CONSUMERIR] device_create fail ret=%d\n", ret);
        goto exit;
    }

exit:
    pr_warning("[CONSUMERIR] CONSUMERIR probe ret=%d\n", ret);
    return ret;
}

//MY edit Start
#ifdef CONFIG_OF
static struct platform_device mt_consumerir_dev =
{
    .name = "consumerir",
    .id = -1,
};
#endif

static struct platform_driver mt_consumerir_driver =
{
    .driver = 
    {
        .name = "consumerir",
    },
    .probe = consumerir_probe,
};

static int __init consumerir_init(void)
{
    int ret = 0;
    if(platform_device_register(&mt_consumerir_dev) != 0)
    {
        return -ENODEV;
    }
    
    ret = platform_driver_register(&mt_consumerir_driver);
    if (ret) 
    {
        return -ENODEV;
    }
    return 0;
}

module_init(consumerir_init);
//MY edit END

MODULE_AUTHOR("Xiao Wang <xiao.wang@mediatek.com>");
MODULE_DESCRIPTION("Consumer IR transmitter driver v0.1");
