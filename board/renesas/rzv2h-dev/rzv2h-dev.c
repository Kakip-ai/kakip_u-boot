#include <common.h>
#include <cpu_func.h>
#include <image.h>
#include <init.h>
#include <malloc.h>
#include <netdev.h>
#include <dm.h>
#include <dm/platform_data/serial_sh.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/rcar-mstp.h>
#include <asm/arch/sh_sdhi.h>
#include <i2c.h>
#include <mmc.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

#define	PFC_BASE			0x10410000
#define	CPG_BASE			0x10420000

#define PWPR				(PFC_BASE + 0x3C04)

#define	P_2A				(PFC_BASE + 0x002A)
#define	PM_2A				(PFC_BASE + 0x0154)
#define	PMC_2A				(PFC_BASE + 0x022A)
#define PFC_PMC26			(PFC_BASE + 0x0226)
#define PFC_PFC26			(PFC_BASE + 0x0498)
#define PFC_PMC29			(PFC_BASE + 0x0229)
#define PFC_PFC29			(PFC_BASE + 0x04A4)

#define	PMC_20				(PFC_BASE + 0x0220)
#define	PFC_20				(PFC_BASE + 0x0480)
#define PMC_23				(PFC_BASE + 0x0223)
#define PFC_23				(PFC_BASE + 0x048C)
#define	PMC_24				(PFC_BASE + 0x0224)
#define	PFC_24				(PFC_BASE + 0x0490)
#define	PMC_26				(PFC_BASE + 0x0226)
#define	PFC_26				(PFC_BASE + 0x0498)
#define	PFC_OSCBYPS			(PFC_BASE + 0x3C00)

#define PWPR_REGWE_A			BIT(6)
#define	PWPR_REGWE_B			BIT(5)

#define	CPG_CLKON_9			(CPG_BASE + 0x0624)
#define	CPG_RST_9			(CPG_BASE + 0x0924)
#define	CPG_RST_10			(CPG_BASE + 0x0928)

/* CPG */
#define CPG_BASE			0x10420000
#define CPG_SSEL0			(CPG_BASE + 0x0300)
#define CPG_SSEL1			(CPG_BASE + 0x0304)
#define CPG_CLKON_11			(CPG_BASE + 0x062C)
#define CPG_CLKON_12			(CPG_BASE + 0x0630)
#define CPG_CLKMON_5			(CPG_BASE + 0x0814)
#define CPG_CLKMON_6			(CPG_BASE + 0x0818)
#define CPG_RST_11			(CPG_BASE + 0x092C)
#define CPG_RSTMON_5			(CPG_BASE + 0x0A14)

#define CPG_RST_USB			(CPG_BASE + 0x0928)
#define CPG_RSTMON4_USB			(CPG_BASE + 0x0A10)
#define CPG_RSTMON5_USB			(CPG_BASE + 0x0A14)
#define CPG_CLKON_USB			(CPG_BASE + 0x062C)
#define CPG_CLKMON_USB			(CPG_BASE + 0x0814)

#define PFC_OEN				(PFC_BASE + 0x3C40)
#define PFC_OEN_OEN0			BIT(0)
#define PFC_OEN_OEN1			BIT(1)
#define PFC_PWPR			(PFC_BASE + 0x3C04)

#define ICU_IPTSR_REG				0x10400060

/* USB */
#define USBPHY20_BASE			(0x15830000)
#define USBPHY21_BASE			(0x15840000)
#define USBPHY20_RESET			(USBPHY20_BASE + 0x000u)
#define USBPHY21_RESET			(USBPHY21_BASE + 0x000u)

#define USB20_BASE			(0x15800000)
#define USB21_BASE			(0x15810000)
#define USBF_BASE			(0x15820000)

#define COMMCTRL			0x800
#define HcRhDescriptorA			0x048
#define LPSTS				0x102
#define USB2_PHY_UTMICTRL2		0xb04
#define USB2_PHY_RESET			0x000
#define USB2_PHY_OTGR			0x600

#define SYS_ADC_CFG			0x10431600

void s_init(void)
{
	*(volatile u32 *)PWPR |= (PWPR_REGWE_A | PWPR_REGWE_B);

	/* Enable ADC */
	*(volatile u32 *)(SYS_ADC_CFG) = 0;

#if CONFIG_TARGET_RZV2H_DEV
	*(volatile u8 *)PMC_2A   &= ~(0x03<<4);	/* PA5,PA4 port	*/
	*(volatile u8 *)P_2A      = (*(volatile u32 *)P_2A  & ~(0x03<<4)) | (0x01 <<5); /* PA5=1,PA4=0		*/
	*(volatile u16 *)PM_2A    = (*(volatile u32 *)PM_2A & ~(0x0f<<8)) | (0x0c <<8); /* PA5,PA4 output	*/
#endif
#if CONFIG_TARGET_RZV2H_EVK_ALPHA || CONFIG_TARGET_RZV2H_EVK_VER1
	/* SD1  */
	*(volatile u8 *)PMC_2A   &= ~(0x03 << 2);/* PA3,PA2 port */
	*(volatile u8 *)P_2A      = (*(volatile u32 *)P_2A  & ~(0x03<<2)) | (0x01 <<3); /* PA3=1,PA2=0		*/
	*(volatile u16 *)PM_2A    = (*(volatile u32 *)PM_2A & ~(0x0f<<4)) | (0x0a <<4); /* PA3,PA2 output	*/

	/* I2C3	*/
	*(volatile u32 *)PFC_23  = (*(volatile u32 *)PFC_23 & 0x00FFFFFF) | (0x01 << 28) | (0x01 << 24);
	*(volatile u8 *)PMC_23   |= (0x03) << 6;	/* P37,P36 multiplexed function	*/

	/* I2C5	*/
	*(volatile u32 *)PFC_26  = (*(volatile u32 *)PFC_26 & 0xFFFF00FF) | (0x04 << 12) | (0x04 << 8);
	*(volatile u8 *)PMC_26   |= (0x03) << 2;	/* P63,P62 multiplexed function	*/

	/* I2C6	*/
	*(volatile u32 *)PFC_24  = (*(volatile u32 *)PFC_24 & 0xFF00FFFF) | (0x01 << 20) | (0x01 << 16);
	*(volatile u8 *)PMC_24   |= (0x03) << 4;	/* P45,P44 multiplexed function	*/

	/* I2C7	*/
	*(volatile u32 *)PFC_24  = (*(volatile u32 *)PFC_24 & 0x00FFFFFF) | (0x01 << 28) | (0x01 << 24);
	*(volatile u8 *)PMC_24   |= (0x03) << 6;	/* P45,P44 multiplexed function	*/

	/* I2C3	*/
	*(volatile u32 *)CPG_CLKON_9 = 0x00800080;
	*(volatile u32 *)CPG_RST_9   = 0x08000800;
	/* I2C5	*/
	*(volatile u32 *)CPG_CLKON_9 = 0x02000200;
	*(volatile u32 *)CPG_RST_9   = 0x20002000;
	/* I2C6	*/
	*(volatile u32 *)CPG_CLKON_9 = 0x04000400;
	*(volatile u32 *)CPG_RST_9   = 0x40004000;
	/* I2C7	*/
	*(volatile u32 *)CPG_CLKON_9 = 0x08000800;
	*(volatile u32 *)CPG_RST_9   = 0x80008000;
#endif
	/* I2C8 */
	*(volatile u32 *)PFC_20  = (*(volatile u32 *)PFC_20 & 0x00FFFFFF) | (0x01 << 28) | (0x01 << 24);
	*(volatile u8 *)PMC_20   |= (0x03) << 6;	/* P07,P06 multiplexed function	*/

	*(volatile u32 *)CPG_CLKON_9 = 0x00080008;
	*(volatile u32 *)CPG_RST_10  = 0x00010001;

	/* Enale OE of IO block for xSPI */
	*(volatile u32 *)(PFC_OEN) &= ~GENMASK(5,2);

	// Use PLL clock for clk_tx_i only for RGMII mode
	// Wite OEN reg. OEN0 bit "0" for output direction
	*(volatile u32 *)(PFC_OEN) &= ~(PFC_OEN_OEN1 | PFC_OEN_OEN0);
	while((*(volatile u32 *)(PFC_OEN) & (PFC_OEN_OEN1 | PFC_OEN_OEN0)) != 0x0)
		;
	
	*(volatile u32 *)PWPR &= ~(PWPR_REGWE_A | PWPR_REGWE_B);

	/* Set Bypass and Powerdown mode for Audio OSC */
	*(volatile u32 *)(PFC_OSCBYPS) = 0x001C0406;

	*(volatile u32 *)(ICU_IPTSR_REG) = 0;
	
	/* Reset ETH 0,1 */
	*(volatile u32 *)(CPG_RST_11) = 0x00030000;
	while((*(volatile u32 *)(CPG_RSTMON_5) & 0x00000006) == 0x0)
		;

	/* Release reset ETH0,1 */
	*(volatile u32 *)(CPG_RST_11) = 0x00030003;
	while((*(volatile u32 *)(CPG_RSTMON_5) & 0x00000006) != 0x0)
		;

	/* Disable SMUX2_GBE0_RXCLK and SMUX2_GBE1_RXCLK */
	*(volatile u32 *) (CPG_SSEL0) = 0x10000000;
	*(volatile u32 *) (CPG_SSEL1) = 0x00100000;

	/* Enable SMUX2_GBE0_RXCLK and SMUX2_GBE1_RXCLK */
	*(volatile u32 *) (CPG_SSEL0) = 0x10001000;
	*(volatile u32 *) (CPG_SSEL1) = 0x00100010;

	/* Enable aclk_csr, aclk, tx, rx, tx_180, rx_180 for ETH0 */
	/* Enable tx, rx for ETH1 */
	*(volatile u32 *)(CPG_CLKON_11) = 0xFF00FF00;
	while((*(volatile u32 *)(CPG_CLKMON_5) & 0xFF000000) != 0xFF000000)
		;

	/* Enable aclk_csr, aclk, tx_180, rx_180 for ETH1 */
	*(volatile u32 *)(CPG_CLKON_12) = 0x000F000F;
	while((*(volatile u32 *)(CPG_CLKMON_6) & 0x0000000F) != 0x0000000F)
		;

}

static void _usbphy_init(void)
{
	/* Overwrite SLEEPM/SUSPENDM signals by USB2PHY Control */
	(*(volatile u32 *)(USBPHY20_BASE + USB2_PHY_UTMICTRL2)) = 0x00000303;
	(*(volatile u32 *)(USBPHY21_BASE + USB2_PHY_UTMICTRL2)) = 0x00000303;

	/* Assert USB2PHY reset */
	(*(volatile u32 *)(USBPHY20_BASE + USB2_PHY_RESET)) = 0x00000206;
	(*(volatile u32 *)(USBPHY21_BASE + USB2_PHY_RESET)) = 0x00000206;

	/* Delay 10us */
	udelay(10);

	/* De-Assert USB2PHY reset */
	(*(volatile u32 *)(USBPHY20_BASE + USB2_PHY_RESET)) = 0x00000200;
	(*(volatile u32 *)(USBPHY21_BASE + USB2_PHY_RESET)) = 0x00000200;

	/* Release overwrites of SLEEPM/SUSMENDM signals, and RESET signal */
	(*(volatile u32 *)(USBPHY20_BASE + USB2_PHY_UTMICTRL2)) = 0x00000003;
	(*(volatile u32 *)(USBPHY20_BASE + USB2_PHY_RESET)) = 0;

	(*(volatile u32 *)(USBPHY21_BASE + USB2_PHY_UTMICTRL2)) = 0x00000003;
	(*(volatile u32 *)(USBPHY21_BASE + USB2_PHY_RESET)) = 0;

	/* Activate VBUS Valid comparator */
	(*(volatile u32 *)(USBPHY20_BASE + USB2_PHY_OTGR)) = 0x00000909;
	(*(volatile u32 *)(USBPHY21_BASE + USB2_PHY_OTGR)) = 0x00000909;
}

static void _reset_usb2(void)
{
	/* Reset for USBTEST */
	(*(volatile u32 *)CPG_RST_USB) |= 0x80008000;
	while((*(volatile u32 *)(CPG_RSTMON5_USB) & 0x00000001) != 0x0);

	/* Reset for USB2 host 0 and 1 */
	(*(volatile u32 *)CPG_RST_USB) |= 0x30003000;
	while((*(volatile u32 *)(CPG_RSTMON4_USB) & 0x60000000) != 0x0);
}

static void board_usb_init(void)
{
	/* Reset USB*/
	_reset_usb2();

	/* Enable clock for USB */
	(*(volatile u32 *)CPG_CLKON_USB) = 0x00F800F8;
	while((*(volatile u32 *)(CPG_CLKMON_USB) & 0x00F80000) != 0x00F80000);

	/* Setup  */
	/* Disable GPIO Write Protect */
	(*(volatile u32 *)PFC_PWPR) |= (0x1u << 6);

#if CONFIG_TARGET_RZV2H_DEV
	/* Set P6_0 as Func.15 for VBUSEN */
	(*(volatile u32 *)PFC_PMC26) |= (0x1u << 0);
	(*(volatile u32 *)PFC_PFC26) &= ~(0xF << 0);
	/* Function mode 15 */
	(*(volatile u32 *)PFC_PFC26) |= (0xF << 0);

	/* Set P6_1 as Func.15 for OVRCUR */
	(*(volatile u32 *)PFC_PMC26) |= (0x1u << 1);
	(*(volatile u32 *)PFC_PFC26) &= ~(0xF << 4);
	/* Function mode 15 */
	(*(volatile u32 *)PFC_PFC26) |= (0xF << 4);

	/* Set P6_2 as Func.15 for VBUSEN */
	(*(volatile u32 *)PFC_PMC26) |= (0x1u << 2);
	(*(volatile u32 *)PFC_PFC26) &= ~(0xF << 8);
	/* Function mode 15 */
	(*(volatile u32 *)PFC_PFC26) |= (0xF << 8);

	/* Set P6_3 as Func.15 for OVRCUR */
	(*(volatile u32 *)PFC_PMC26) |= (0x1u << 3);
	(*(volatile u32 *)PFC_PFC26) &= ~(0xF << 12);
	/* Function mode 15 */
	(*(volatile u32 *)PFC_PFC26) |= (0xF << 12);
#endif /* CONFIG_TARGET_RZV2H_DEV */

#if CONFIG_TARGET_RZV2H_EVK_ALPHA || CONFIG_TARGET_RZV2H_EVK_VER1
        /* Set P9_5 as Func.14 for VBUSEN */
        /* Control mode (multiplexed function) */
        (*(volatile u32 *)PFC_PMC29) |= (0x1u << 5);
        (*(volatile u32 *)PFC_PFC29) &= ~(0xF << 20);
        /* Function mode 15 */
        (*(volatile u32 *)PFC_PFC29) |= (0x0E << 20);

        /* Set P9_6 as Func.14 for OVRCUR */
        /* Control mode (multiplexed function) */
        (*(volatile u32 *)PFC_PMC29) |= (0x1u << 6);
        (*(volatile u32 *)PFC_PFC29) &= ~(0xF << 24);
        /* Function mode 14 */
        (*(volatile u32 *)PFC_PFC29) |= (0x0E << 24);

        /* Set P6_6 as Func.14 for VBUSEN */
        /* Control mode (multiplexed function) */
         (*(volatile u32 *)PFC_PMC26) |= (0x1u << 6);
         (*(volatile u32 *)PFC_PFC26) &= ~(0xF << 24);
        /* Function mode 14 */
         (*(volatile u32 *)PFC_PFC26) |= (0x0E << 24);

        /* Set P6_7 as Func.14 for OVRCUR */
        /* Control mode (multiplexed function) */
        (*(volatile u32 *)PFC_PMC26) |= (0x1u << 7);
        (*(volatile u32 *)PFC_PFC26) &= ~(0xF << 28);
        /* Function mode 15 */
        (*(volatile u32 *)PFC_PFC26) |= (0x0E << 28);
#endif /* CONFIG_TARGET_RZV2H_EVK_ALPHA || CONFIG_TARGET_RZV2H_EVK_VER1 */

	/* Enable Write protect */
	(*(volatile u32 *)PFC_PWPR) &= ~(0x1u << 6);

	/* Initialize phy */
	_usbphy_init();

	/*USB0 is HOST*/
	(*(volatile u32 *)(USB20_BASE + COMMCTRL)) = 0;

	/*USB1 is HOST*/
	(*(volatile u32 *)(USB21_BASE + COMMCTRL)) = 0;

	/* Set USBPHY normal operation (Function only) */
	(*(volatile u16 *)(USBF_BASE + LPSTS)) |= (0x1u << 14);

	/* Overcurrent is not supported */
	(*(volatile u32 *)(USB20_BASE + HcRhDescriptorA)) |= (0x1u << 12);
	(*(volatile u32 *)(USB21_BASE + HcRhDescriptorA)) |= (0x1u << 12);
}

static void board_pmic_i2c_init(void)
{
	struct udevice *bus, *dev;
	int ret;
	u8 reg_addr, reg_val;

	/* Get the I2C bus */
	ret = uclass_get_device_by_seq(UCLASS_I2C, 8, &bus);
	if (ret)
		goto pmic_failed;

	/* Initialize I2C device at address 0x6a */
	ret = dm_i2c_probe(bus, 0x6a, 0, &dev);
	if (ret)
		goto pmic_failed;

	/* Write 0x00 to register 0x24 of device 0x6a */
	reg_addr = 0x24;
	reg_val = 0x00;
	ret = dm_i2c_write(dev, reg_addr, &reg_val, 1);
	if (ret)
		goto pmic_failed;

	return;

pmic_failed:
	printf("Can not initialize PMIC settings via I2C8\n");
	return;
}

int board_early_init_f(void)
{

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	board_usb_init();

	/* Initialize PMIC I2C devices */
#if (CONFIG_TARGET_RZV2H_EVK_VER1)
	board_pmic_i2c_init();
#endif

	return 0;
}

static void board_nic_init(void)
{
	struct udevice *bus, *dev;
	u8 ret, count;
	u8 reg_addr, *p_addr;
	u8 eth_addr[6];

	/* Read NIC-ID from I2C5. */
	if (!eth_env_get_enetaddr("ethaddr", eth_addr)) {

		/* Get the I2C bus */
		ret = uclass_get_device_by_seq(UCLASS_I2C, 5, &bus);
		if (ret)
			goto nic_failed;

		/* Initialize I2C device at address 0x50 */
		ret = dm_i2c_probe(bus, 0x50, 0, &dev);
		if (ret)
			goto nic_failed;

		/* Read NIC-ID. */
		reg_addr = 0xFA;
		p_addr = eth_addr;
		for (count = 0; count < 6; count++) {
			ret = dm_i2c_read(dev, reg_addr, p_addr, 1);
			if (ret)
				goto nic_failed;
			reg_addr++;
			p_addr++;
		}

		/* set ethaddr. */
		eth_env_set_enetaddr("ethaddr", eth_addr);
		if (env_save())
			goto nic_failed;
	}

	return;

nic_failed:
	printf("Can not initialize nic-id via I2C5.\n");
	return;
}

int misc_init_r(void)
{
	/* initialize NIC-ID. */
	board_nic_init();
	return 0;
}

void reset_cpu(void)
{

}
