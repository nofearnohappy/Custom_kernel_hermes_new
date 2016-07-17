

struct mt_consumerir {
	unsigned int pwm_ch;
	void __iomem *reg_base;
	unsigned int irq;
	struct platform_device *plat_dev;
	unsigned int carrier_freq;
	atomic_t usage_cnt;
};

struct consumerir_config {
	unsigned int start : 1;
	unsigned int mode : 2;
	unsigned int sw_o : 1;
	unsigned int b_ord : 1;
	unsigned int r_ord : 1;
	unsigned int ir_os : 1;
	unsigned int ir_inv : 1;
	unsigned int bit_num : 7;
	unsigned int data_inv : 1;
};

#define CONSUMERIR_IOC_SET_CARRIER_FREQ _IOW('R', 0, unsigned int)

#define CONSUMERIR_IOC_SET_CONSUMERIR_LED_EN    _IOW('R', 10, unsigned int)

#define CONSUMERIR_write32(b, a, v) mt_reg_sync_writel(v, (b)+(a))
#define CONSUMERIR_read32(b, a) ioread32((void __iomem *)((b)+(a)))

#define CLOCK_SRC 26 // MHz

#define CONSUMERIRCFG 0x0
#define CONSUMERIRD0 0x4
#define CONSUMERIRD1 0x8
#define CONSUMERIRD2 0xC
#define CONSUMERIR_L0H 0x10
#define CONSUMERIR_L0L 0x14
#define CONSUMERIR_L1H 0x18
#define CONSUMERIR_L1L 0x1C
#define CONSUMERIRSYNCH 0x20
#define CONSUMERIRSYNCL 0x24
#define CONSUMERIRMT 0x28

