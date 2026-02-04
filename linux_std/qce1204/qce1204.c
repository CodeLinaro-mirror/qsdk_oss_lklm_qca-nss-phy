/*
* Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
* SPDX-License-Identifier: ISC
*/

#include <linux/of.h>
#include <linux/phy.h>
#include "qce1204.h"
#include "../qca81xx/qca81xx.h"

struct clk_init_entry {
	struct clk **clk_ptr;
	const char *name;
	const char *desc;
};

struct reset_init_entry {
	struct reset_control **reset_ptr;
	const char *name;
	const char *desc;
};

#define QCE1204_PHY_SPEC_CONTROL					0x10
#define QCE1204_PHY_MDI_MASK						GENMASK(6, 5)
#define QCE1204_PHY_MDI_AUTO						0x3
#define QCE1204_PHY_MDI_X						0x1
#define QCE1204_PHY_MDI							0
#define QCE1204_PHY_SPEC_STATUS						0x11
#define QCE1204_PHY_SS_LINK_STATUS					0x400
#define QCE1204_PHY_INTR_DOWNSHIFT					0x20
#define QCE1204_PHY_SS_DUPLEX_FULL					0x2000
#define QCE1204_PHY_SS_SPEED_MASK					0x380
#define QCE1204_PHY_SS_SPEED_2500					0x200
#define QCE1204_PHY_SS_SPEED_1000					0x100
#define QCE1204_PHY_SS_SPEED_100					0x80
#define QCE1204_PHY_SS_SPEED_10						0
#define QCE1204_PHY_SS_MDIX						0x40
#define QCE1204_PHY_INTR_MASK						0x12

#define QCE1204_PHY_INTR_STATUS						0x13
#define QCE1204_PHY_INTR_STATUS_DOWN					0x800
#define QCE1204_PHY_INTR_STATUS_UP					0x400
#define QCE1204_PHY_1000BASET_CONTROL					0x9
#define QCE1204_PHY_ADVERTISE_1000FULL					0x200
#define QCE1204_PHY_1000BASET_STATUS					0xa
#define QCE1204_PHY_LP_ADVERTISE_1000FULL				0x800
#define QCE1204_PHY_CONTROL						0x19
#define QCE1204_PHY_FIFO_RESET						0x3
#define QCE1204_PHY_MMD7_IPG_OP						0x901d
#define QCE1204_PHY_IPG_10_TO_11_EN					BIT(0)
#define QCE1024_PHY_2P5G_EEE_TX_LPI_QUIET_CTRL0				0xa02c
#define QCE1024_PHY_2P5G_EEE_TX_QUIET_TIME0				0x3db
#define QCE1024_PHY_2P5G_EEE_TX_LPI_QUIET_CTRL1				0xa031
#define QCE1024_PHY_2P5G_EEE_TX_QUIET_TIME1				0x14
#define QCE1024_PHY_2P5G_EEE_TX_LPI_WAKE_CTRL				0xa10c
#define QCE1024_PHY_2P5G_EEE_TX_WAKE_TIME				0x2fc

#define QCE1204_MMD3_CDT_THRESH_CTRL14					0x807f
#define QCE1204_MMD3_CDT_THRESH_CTRL14_VAL				0x9ab0
#define QCE1204_MMD3_CDT_THRESH_CTRL2					0x8073
#define QCE1204_MMD3_CDT_THRESH_CTRL2_VAL				0xb03f
#define QCE1204_MMD3_CDT_THRESH_CTRL3					0x8074
#define QCE1204_MMD3_CDT_THRESH_CTRL3_VAL				0xc040
#define QCE1204_MMD3_CDT_THRESH_CTRL4					0x8075
#define QCE1204_MMD3_CDT_THRESH_CTRL4_VAL				0xa060
#define QCE1204_MMD3_CDT_THRESH_CTRL5					0x8076
#define QCE1204_MMD3_CDT_THRESH_CTRL5_VAL				0xc040
#define QCE1204_MMD3_CDT_THRESH_CTRL6					0x8077
#define QCE1204_MMD3_CDT_THRESH_CTRL6_VAL				0xa060
#define QCE1204_MMD3_CDT_THRESH_CTRL7					0x8078
#define QCE1204_MMD3_CDT_THRESH_CTRL7_VAL				0xae50
#define QCE1204_MMD3_CDT_THRESH_CTRL9					0x807a
#define QCE1204_MMD3_CDT_THRESH_CTRL9_VAL				0xc060
#define QCE1204_MMD3_CDT_THRESH_CTRL13					0x807e
#define QCE1204_MMD3_CDT_THRESH_CTRL13_VAL				0xb060

#define QCE1204_MMD7_LED0_CTRL						0x8078
#define QCE1204_SPEED_10M_ON						BIT(4)

#define QCE1204_DEBUG_ANA_10M_DAC_CTRL0					0x2880
#define QCE1204_DEBUG_ANA_10M_DAC_CTRL0_VAL				0x7777
#define QCE1204_DEBUG_ANA_10M_DAC_CTRL1					0x3880
#define QCE1204_DEBUG_ANA_10M_DAC_CTRL1_VAL				0xa4a4
#define QCE1204_DEBUG_ANA_10M_DAC_CTRL2					0x3980
#define QCE1204_DEBUG_ANA_10M_DAC_CTRL2_VAL				0xa4a4
#define QCE1204_DEBUG_ANA_2P5G_TX_GAIN_CTRL				0xbb80
#define QCE1204_DEBUG_ANA_2P5G_TX_GAIN_VAL				0xf33
#define QCE1204_DEBUG_PLL_CTRL						0x1f
#define QCE1204_DEBUG_PLL0_FORCE_ON					BIT(2)
#define QCE1204_DEBUG_ANA_10M_DAC_CTRL3					0xc980
#define QCE1204_DEBUG_ANA_10M_DAC_CTRL3_VAL				0xc0

#define QCE1204_PCS_MMD1_CDA_CONTROL1					0x20
#define QCE1204_PCS_MMD1_CALIBRATION4					0x78
#define QCE1204_PCS_MMD1_MODE_CTRL					0x11b
#define QCE1204_PCS_MMD1_BYPASS_TUNING_IPG				0x189
#define QCE1204_PCS_MMD1_QUSGMII_RESET					0x18c
#define QCE1204_PCS_MMD1_BYPASS_TUNING_IPG_EN				0x0fff
#define QCE1204_PCS_MMD1_XPCS_MODE					0x1000
#define QCE1204_PCS_MMD1_CALIBRATION_DONE				0x80
#define QCE1204_PCS_MMD1_SSCG_ENABLE					0x8
#define QCE1204_PCS_MMD1_PLL_POWER_ON_AND_RESET				0x1e0
#define QCE1204_PCS_MMD1_ANA_SOFT_RESET_MASK				0x40
#define QCE1204_PCS_MMD1_ANA_SOFT_RESET					0
#define QCE1204_PCS_MMD1_ANA_SOFT_RELEASE				0x40
#define QCE1204_PCS_MMD1_SSC_CLK					0x166
#define QCE1204_PCS_MMD1_SSC_CLK_EN					BIT(7)
#define QCE1204_PCS_MMD1_MS_LDO0					0x16d
#define QCE1204_PCS_MMD1_MS_LDO1					0x16e
#define QCE1204_PCS_MMD3_PCS_CTRL2					0x7
#define QCE1204_PCS_MMD3_AN_LP_BASE_ABL2				0x14
#define QCE1204_PCS_MMD3_10GBASE_PCS_STATUS1				0x20
#define QCE1204_PCS_MMD3_DIG_CTRL1					0x8000
#define QCE1204_PCS_MMD3_EEE_MODE_CTRL					0x8006
#define QCE1204_PCS_MMD3_VR_RPCS_TPC					0x8007
#define QCE1204_PCS_MMD3_EEE_TX_TIMER					0x8008
#define QCE1204_PCS_MMD3_EEE_RX_TIMER					0x8009
#define QCE1204_PCS_MMD3_MII_AM_INTERVAL				0x800a
#define QCE1204_PCS_MMD3_EEE_MODE_CTRL1					0x800b
#define QCE1204_PCS_MMD3_PCS_TYPE_10GBASE_R				0
#define QCE1204_PCS_MMD3_10GBASE_UP					0x1000
#define QCE1204_PCS_MMD3_QUSGMII_EN					0x200
#define QCE1204_PCS_MMD3_QUSGMII_MODE					0x1400
#define QCE1204_PCS_MMD3_MII_AM_INTERVAL_VAL				0x6018
#define QCE1204_PCS_MMD3_XPCS_SOFT_RESET				0x8000
#define QCE1204_PCS_MMD3_XPCS_EEE_CAP					0x40
#define QCE1204_PCS_MMD3_EEE_RES_REGS					0x100
#define QCE1204_PCS_MMD3_EEE_SIGN_BIT_REGS				0x40
#define QCE1204_PCS_MMD3_EEE_EN						0x3
#define QCE1204_PCS_MMD3_EEE_TSL_REGS					0xa
#define QCE1204_PCS_MMD3_EEE_TLU_REGS					0xc0
#define QCE1204_PCS_MMD3_EEE_TWL_REGS					0x1600
#define QCE1204_PCS_MMD3_EEE_100US_REG_REGS				0xc8
#define QCE1204_PCS_MMD3_EEE_RWR_REG_REGS				0x1c00
#define QCE1204_PCS_MMD3_EEE_TRANS_LPI_MODE				0x1
#define QCE1204_PCS_MMD3_EEE_TRANS_RX_LPI_MODE				0x100
#define QCE1204_PCS_MMD3_QUSGMII_FIFO_RESET				0x400
#define QCE1204_PCS_MMD_MII_CTRL					0
#define QCE1204_PCS_SPEED_MASK						0x2060
#define QCE1204_PCS_SPEED_10M						0
#define QCE1204_PCS_MMD_MII_DIG_CTRL					0x8000
#define QCE1204_PCS_MMD_MII_AN_INT_MSK					0x8001
#define QCE1204_PCS_MMD_MII_ERR_SEL					0x8002
#define QCE1204_PCS_MMD_MII_XAUI_MODE_CTRL				0x8004
#define QCE1204_PCS_MMD_AN_COMPLETE_INT					0x1
#define QCE1204_PCS_MMD_MII_4BITS_CTRL					0x0
#define QCE1204_PCS_MMD_TX_CONFIG_CTRL					0x8
#define QCE1204_PCS_MMD_MII_AN_ENABLE					0x1000
#define QCE1204_PCS_MMD_MII_AN_RESTART					0x200
#define QCE1204_PCS_MMD_MII_AN_COMPLETE_INT				0x1
#define QCE1204_PCS_MMD_QUSGMII_FIFO_RESET				0x20
#define QCE1204_PCS_MMD_TX_IPG_CHECK_DISABLE				0x1
#define QCE1204_PCS_MMD_PHY_MODE_CTRL_EN				0x1
#define QCE1204_PCS_MMD_CH2						26
#define QCE1204_PCS_MMD_CH3						27
#define QCE1204_PCS_MMD_CH4						28

/* Valid clock rates for QCE1204 */
#define QCE1204_CLK_RATE_2P5M						2500000
#define QCE1204_CLK_RATE_25M						25000000
#define QCE1204_CLK_RATE_78P125M					78125000
#define QCE1204_CLK_RATE_104M						104166666
#define QCE1204_CLK_RATE_125M						125000000
#define QCE1204_CLK_RATE_312P5M						312500000

/* SOC SEC_TCSR registers */
#define QCE1204_EPHY_CFG						0x90F018
#define QCE1204_EPHY_LDO_CTRL						GENMASK(9, 8)
#define QCE1204_WORK_MODE_SEL						0x90f030
#define QCE1204_PHY_MODE_MASK						GENMASK(3, 0)
#define QCE1204_PHY_MODE						0xf
#define QCE1204_SWITCH_MODE_MASK					GENMASK(5, 0)
#define QCE1204_SWITCH_MODE						0x10

/**
 * qce1204_validate_clock_rate - Validate if clock rate is supported
 * @rate: Clock rate to validate
 * Returns: 0 if valid, -EINVAL if invalid
 */
static int qce1204_validate_clock_rate(unsigned long rate)
{
	switch (rate) {
	case QCE1204_CLK_RATE_2P5M:
	case QCE1204_CLK_RATE_25M:
	case QCE1204_CLK_RATE_78P125M:
	case QCE1204_CLK_RATE_104M:
	case QCE1204_CLK_RATE_125M:
	case QCE1204_CLK_RATE_312P5M:
		return 0;
	default:
		pr_err("Invalid clock rate: %lu Hz\n", rate);
		return -EINVAL;
	}
}

static int qce1204_soc_addr_get(struct phy_device *phydev)
{
	if (!phydev || !phydev->shared)
		return PHY_MAX_ADDR;

	return phydev->shared->addr + QCE1204_SOC_ADDR_OFFSET;
}

static void qce1204_split_addr(u32 regaddr, u16 *reg_low, u16 *reg_mid,
	u16 *reg_high)
{
	*reg_low = (regaddr & 0xc) << 1;

	*reg_mid = regaddr >> 4 & 0xffff;

	*reg_high = ((regaddr >> 20 & 0xf) << 1) | BIT(0);
}

u32 __qce1204_soc_read(struct phy_device *phydev, u32 reg)
{
	u16 reg_low, reg_mid, reg_high;
	u16 lo, hi;
	u32 addr;

	addr = qce1204_soc_addr_get(phydev);
	if (addr >= PHY_MAX_ADDR)
		return 0xFFFFFFFF;

	qce1204_split_addr(reg, &reg_low, &reg_mid, &reg_high);
	/* write ahb address bit4~bit23 */
	__mdiobus_write(phydev->mdio.bus, addr, reg_high & 0x1f, reg_mid);
	udelay(100);
	/* write ahb address bit0~bit3 and read low 16bit data */
	lo = __mdiobus_read(phydev->mdio.bus, addr, reg_low);
	/* write ahb address bit0~bit3 and read high 16 bit data */
	hi = __mdiobus_read(phydev->mdio.bus, addr, (reg_low + 4));

	return (hi << 16) | lo;
}

void __qce1204_soc_write(struct phy_device *phydev, u32 reg, u32 val)
{
	u16 reg_low, reg_mid, reg_high;
	u16 lo, hi;
	u32 addr;

	addr = qce1204_soc_addr_get(phydev);
	if (addr >= PHY_MAX_ADDR)
		return;

	qce1204_split_addr(reg, &reg_low, &reg_mid, &reg_high);
	lo = val & 0xffff;
	hi = (u16)(val >> 16);

	/* write ahb address bit4~bit23 */
	__mdiobus_write(phydev->mdio.bus, addr, reg_high & 0x1f, reg_mid);
	udelay(100);
	/* write ahb address bit0~bit3 and write low 16 bit data */
	__mdiobus_write(phydev->mdio.bus, addr, reg_low, lo);
	/* write ahb address bit0~bit3 and write high 16 bit data */
	__mdiobus_write(phydev->mdio.bus, addr, (reg_low + 4), hi);
}

int __qce1204_soc_modify(struct phy_device *phydev, u32 reg,
	u32 mask, u32 set)
{
	u32 val;

	val = __qce1204_soc_read(phydev, reg);
	val = (val & ~mask) | set;
	__qce1204_soc_write(phydev, reg, val);

	return 0;
}

u32 qce1204_soc_read(struct phy_device *phydev, u32 reg)
{
	u32 val;

	phy_lock_mdio_bus(phydev);
	val = __qce1204_soc_read(phydev, reg);
	phy_unlock_mdio_bus(phydev);

	return val;
}

int qce1204_soc_modify(struct phy_device *phydev, u32 reg,
	u32 mask, u32 set)
{
	phy_lock_mdio_bus(phydev);
	__qce1204_soc_modify(phydev, reg, mask, set);
	phy_unlock_mdio_bus(phydev);

	return 0;
}

static int qce1204_pcs_addr_get(struct phy_device *phydev, int offset)
{
	if (phydev->shared)
		return phydev->shared->addr + offset;

	return PHY_MAX_ADDR;
}

static int qce1204_pcs_read_mmd(struct phy_device *phydev, int devad, u32 regnum)
{
	int addr = qce1204_pcs_addr_get(phydev, PCS1_ADDR_OFFSET);

	if (addr >= PHY_MAX_ADDR)
		return -EINVAL;

	return mdiobus_c45_read(phydev->mdio.bus, addr, devad, regnum);
}

static int qce1204_pcs_write_mmd(struct phy_device *phydev, int devad, u32 regnum, u16 val)
{
	int addr = qce1204_pcs_addr_get(phydev, PCS1_ADDR_OFFSET);

	if (addr >= PHY_MAX_ADDR)
		return -EINVAL;

	return mdiobus_c45_write(phydev->mdio.bus, addr, devad, regnum, val);
}

static int qce1204_pcs_modify_mmd(struct phy_device *phydev,
	int devad, u32 regnum, u16 mask, u16 set)
{
	int addr = qce1204_pcs_addr_get(phydev, PCS1_ADDR_OFFSET);

	if (addr >= PHY_MAX_ADDR)
		return -EINVAL;

	mdiobus_c45_modify(phydev->mdio.bus, addr, devad, regnum,
		mask, set);
	return mdiobus_c45_read(phydev->mdio.bus, addr, devad, regnum);
}

static int qce1204_pcs_mmd_get(struct phy_device *phydev, int channel)
{
	switch(channel) {
	case 1:
		return MDIO_MMD_VEND2;
	case 2:
		return QCE1204_PCS_MMD_CH2;
	case 3:
		return QCE1204_PCS_MMD_CH3;
	case 4:
		return QCE1204_PCS_MMD_CH4;
	default:
		return -EOPNOTSUPP;
	}
}

static int qce1204_pcs_modify_channel_mmd(struct phy_device *phydev,
	u32 channel, u32 regnum, u16 mask, u16 set)
{
	int mmd_id = qce1204_pcs_mmd_get(phydev, channel);

	if (mmd_id < 0)
		return -EOPNOTSUPP;

	return qce1204_pcs_modify_mmd(phydev, mmd_id, regnum, mask, set);
}

static int qce1204_pcs_10g_linkup(struct phy_device *phydev)
{
	u16 xpcs_data = 0;
	u32 retries = 100, linkup = 0;

	/* wait 10G_R link up */
	while (linkup != QCE1204_PCS_MMD3_10GBASE_UP) {
		mdelay(1);
		if (retries-- == 0) {
			phydev_err (phydev, "10g_r link up timeout\n");
			return -ETIMEDOUT;
		}
		xpcs_data = qce1204_pcs_read_mmd(phydev, MDIO_MMD_PCS,
			QCE1204_PCS_MMD3_10GBASE_PCS_STATUS1);

		linkup = (xpcs_data & QCE1204_PCS_MMD3_10GBASE_UP);
	}

	return 0;
}

static int qce1204_pcs_soft_reset(struct phy_device *phydev)
{
	int ret = 0;
	u16 pcs_data = 0;
	u32 retries = 100, reset_done = QCE1204_PCS_MMD3_XPCS_SOFT_RESET;

	ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_DIG_CTRL1, 0x8000, QCE1204_PCS_MMD3_XPCS_SOFT_RESET);
	if (ret < 0)
		return ret;

	while (reset_done) {
		mdelay(1);
		if (retries-- == 0)
			return -ETIMEDOUT;
		pcs_data = qce1204_pcs_read_mmd(phydev, MDIO_MMD_PCS,
			QCE1204_PCS_MMD3_DIG_CTRL1);

		reset_done = (pcs_data & QCE1204_PCS_MMD3_XPCS_SOFT_RESET);
	}

	return 0;
}

static int qce1204_pcs_8023az_enable(struct phy_device *phydev)
{
	u16 pcs_data = 0;

	pcs_data = qce1204_pcs_read_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_AN_LP_BASE_ABL2);
	if (!(pcs_data & QCE1204_PCS_MMD3_XPCS_EEE_CAP))
		return -EOPNOTSUPP;

	/* Configure the EEE related timer */
	qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_EEE_MODE_CTRL, 0x0f40, QCE1204_PCS_MMD3_EEE_RES_REGS |
		QCE1204_PCS_MMD3_EEE_SIGN_BIT_REGS);

	qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_EEE_TX_TIMER, 0x1fff, QCE1204_PCS_MMD3_EEE_TSL_REGS|
		QCE1204_PCS_MMD3_EEE_TLU_REGS | QCE1204_PCS_MMD3_EEE_TWL_REGS);

	qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_EEE_RX_TIMER, 0x1fff, QCE1204_PCS_MMD3_EEE_100US_REG_REGS|
		QCE1204_PCS_MMD3_EEE_RWR_REG_REGS);

	/* enable TRN_LPI */
	qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_EEE_MODE_CTRL1, 0x101, QCE1204_PCS_MMD3_EEE_TRANS_LPI_MODE|
		QCE1204_PCS_MMD3_EEE_TRANS_RX_LPI_MODE);

	/* enable TX/RX LPI pattern */
	qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_EEE_MODE_CTRL, 0x3, QCE1204_PCS_MMD3_EEE_EN);

	return 0;
}

static int qce1204_pcs_calibration(struct phy_device *phydev)
{
	u16 pcs_data = 0;
	u32 retries = 100, calibration_done = 0;

	/* wait calibration done to uniphy */
	while (calibration_done != QCE1204_PCS_MMD1_CALIBRATION_DONE) {
		mdelay(1);
		if (retries-- == 0) {
			phydev_err(phydev, "pcs callibration time out!\n");
			return -ETIMEDOUT;
		}
		pcs_data = qce1204_pcs_read_mmd(phydev,
			MDIO_MMD_PMAPMD, QCE1204_PCS_MMD1_CALIBRATION4);

		calibration_done = (pcs_data & QCE1204_PCS_MMD1_CALIBRATION_DONE);
	}

	return 0;
}

static int qce1204_pcs_ana_assert(struct phy_device *phydev,
	bool assert)
{
	return qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PMAPMD,
		QCE1204_PCS_MMD1_PLL_POWER_ON_AND_RESET,
		QCE1204_PCS_MMD1_ANA_SOFT_RESET_MASK,
		assert ? QCE1204_PCS_MMD1_ANA_SOFT_RESET :
		QCE1204_PCS_MMD1_ANA_SOFT_RELEASE);
}

/**
 * qce1204_get_clk_data - Get clock data from PHY private structure
 * @phydev: PHY device
 * Returns: Pointer to clock data structure, or NULL if not available
 */
static struct qce1204_clk_data *qce1204_get_clk_data(struct phy_device *phydev)
{
	struct qce1204_priv *priv = (struct qce1204_priv *)phydev->priv;

	if (!priv)
		return NULL;

	return &priv->clk_data;
}

/**
 * qce1204_get_shared_clk_data - Get shared clock data
 * @phydev: PHY device
 * Returns: Pointer to shared clock data structure, or NULL if not available
 */
static struct qce1204_shared_clk_data *qce1204_get_shared_clk_data(struct phy_device *phydev)
{
	struct phy_package_shared *shared = phydev->shared;
	struct qce1204_shared_priv *priv;

	if (!shared)
		return NULL;

	priv = (struct qce1204_shared_priv *)shared->priv;
	if (!priv)
		return NULL;

	return &priv->shared_clk_data;
}

/**
 * qce1204_get_package_mode - Get package mode from shared private data
 * @phydev: PHY device
 * Returns: Package mode (phy_interface_t), or PHY_INTERFACE_MODE_NA if not available
 */
static phy_interface_t qce1204_get_package_mode(struct phy_device *phydev)
{
	struct phy_package_shared *shared = phydev->shared;
	struct qce1204_shared_priv *priv;

	if (!shared)
		return PHY_INTERFACE_MODE_NA;

	priv = (struct qce1204_shared_priv *)shared->priv;
	if (!priv)
		return PHY_INTERFACE_MODE_NA;

	return priv->package_mode;
}

/**
 * qce1204_clk_get - Get a single clock resource
 */
static int qce1204_clk_get(struct phy_device *phydev, struct device *dev,
			   struct clk **clk_ptr, const char *name,
			   const char *desc)
{
	*clk_ptr = devm_clk_get_optional(dev, name);
	if (IS_ERR(*clk_ptr)) {
		phydev_err(phydev, "Failed to get %s: %ld\n", desc, PTR_ERR(*clk_ptr));
		return PTR_ERR(*clk_ptr);
	}
	if (*clk_ptr)
		phydev_dbg(phydev, "Got %s\n", desc);
	return 0;
}

/**
 * qce1204_reset_get - Get a single reset control resource
 */
static int qce1204_reset_get(struct phy_device *phydev, struct device *dev,
			     struct reset_control **reset_ptr, const char *name,
			     const char *desc)
{
	*reset_ptr = devm_reset_control_get_optional(dev, name);
	if (IS_ERR(*reset_ptr)) {
		phydev_err(phydev, "Failed to get %s: %ld\n", desc, PTR_ERR(*reset_ptr));
		return PTR_ERR(*reset_ptr);
	}
	if (*reset_ptr)
		phydev_dbg(phydev, "Got %s\n", desc);
	return 0;
}

/**
 * qce1204_phy_tx_clk_set - Enable/disable PHY TX clock
 */
static int qce1204_phy_tx_clk_set(struct phy_device *phydev, bool enable)
{
	struct qce1204_clk_data *clk_data;
	int ret = 0;

	clk_data = qce1204_get_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	/* Clock is optional, return success if not present */
	if (!clk_data->tx_clk)
		return 0;

	if (enable) {
		ret = clk_prepare_enable(clk_data->tx_clk);
		if (ret < 0)
			phydev_err(phydev, "Failed to enable PHY TX clock: %d\n", ret);
	} else {
		clk_disable_unprepare(clk_data->tx_clk);
	}

	return ret;
}

/**
 * qce1204_phy_rx_clk_set - Enable/disable PHY RX clock
 */
static int qce1204_phy_rx_clk_set(struct phy_device *phydev, bool enable)
{
	struct qce1204_clk_data *clk_data;
	int ret = 0;

	clk_data = qce1204_get_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	/* Clock is optional, return success if not present */
	if (!clk_data->rx_clk)
		return 0;

	if (enable) {
		ret = clk_prepare_enable(clk_data->rx_clk);
		if (ret < 0)
			phydev_err(phydev, "Failed to enable PHY RX clock: %d\n", ret);
	} else {
		clk_disable_unprepare(clk_data->rx_clk);
	}

	return ret;
}

static int qce1204_phy_clk_parent_init(struct phy_device *phydev)
{
	struct qce1204_shared_clk_data *clk_shared_data;
	struct qce1204_clk_data *clk_data;
	int ret;

	clk_shared_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_shared_data)
		return -EINVAL;

	if (!clk_shared_data->tx_parent || !clk_shared_data->rx_parent)
		return 0;

	clk_data = qce1204_get_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	if (!clk_data->tx_src_clk || !clk_data->rx_src_clk)
		return 0;

	ret = clk_set_parent(clk_data->tx_src_clk, clk_shared_data->tx_parent);
	if (ret)
		return ret;

	return clk_set_parent(clk_data->rx_src_clk, clk_shared_data->rx_parent);
}

/**
 * qce1204_phy_sys_clk_set - Enable/disable PHY SYS clock
 * @phydev: PHY device
 * @enable: true to enable clock, false to disable
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_phy_sys_clk_set(struct phy_device *phydev, bool enable)
{
	struct qce1204_clk_data *clk_data;
	int ret = 0;

	clk_data = qce1204_get_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	/* Clock is optional, return success if not present */
	if (!clk_data->sys_clk)
		return 0;

	if (enable) {
		ret = clk_prepare_enable(clk_data->sys_clk);
		if (ret < 0)
			phydev_err(phydev, "Failed to enable PHY SYS clock: %d\n", ret);
	} else {
		clk_disable_unprepare(clk_data->sys_clk);
	}

	return ret;
}

/**
 * qce1204_phy_clk_set - Enable/disable clocks for this PHY
 * Only enables clocks that are defined in DTS
 */
static int qce1204_phy_clk_set(struct phy_device *phydev, bool enable)
{
	int ret;

	ret = qce1204_phy_tx_clk_set(phydev, enable);
	if (ret < 0)
		return ret;
	ret = qce1204_phy_rx_clk_set(phydev, enable);

	return ret;
}

/**
 * qce1204_phy_tx_reset_assert - Assert/deassert PHY TX reset
 * @phydev: PHY device
 * @assert: true to assert reset, false to deassert
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_phy_tx_reset_assert(struct phy_device *phydev, bool assert)
{
	struct qce1204_clk_data *clk_data;
	int ret;

	clk_data = qce1204_get_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	/* Reset is optional, return success if not present */
	if (!clk_data->tx_reset)
		return 0;

	if (assert)
		ret = reset_control_assert(clk_data->tx_reset);
	else
		ret = reset_control_deassert(clk_data->tx_reset);
	if (ret < 0) {
		phydev_err(phydev, "Failed to %s TX reset: %d\n",
			   assert ? "assert" : "deassert", ret);
		return ret;
	}

	return 0;
}

/**
 * qce1204_phy_rx_reset_assert - Assert/deassert PHY RX reset
 * @phydev: PHY device
 * @assert: true to assert reset, false to deassert
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_phy_rx_reset_assert(struct phy_device *phydev, bool assert)
{
	struct qce1204_clk_data *clk_data;
	int ret;

	clk_data = qce1204_get_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	/* Reset is optional, return success if not present */
	if (!clk_data->rx_reset)
		return 0;

	if (assert)
		ret = reset_control_assert(clk_data->rx_reset);
	else
		ret = reset_control_deassert(clk_data->rx_reset);
	if (ret < 0) {
		phydev_err(phydev, "Failed to %s RX reset: %d\n",
			   assert ? "assert" : "deassert", ret);
		return ret;
	}

	return 0;
}


/**
 * qce1204_phy_clk_reset_assert - Assert/deassert all PHY clock resets
 * @phydev: PHY device
 * @assert: true to assert reset, false to deassert
 * Returns: 0 on success, negative error code on failure
 *
 * This function controls all PHY clock-related resets:
 * - TX reset
 * - RX reset
 */
static int qce1204_phy_clk_reset_assert(struct phy_device *phydev, bool assert)
{
	int ret;

	/* Control TX reset */
	ret = qce1204_phy_tx_reset_assert(phydev, assert);
	if (ret < 0)
		return ret;

	/* Control RX reset */
	ret = qce1204_phy_rx_reset_assert(phydev, assert);
	if (ret < 0)
		return ret;

	return 0;
}

static int qce1204_phy_clk_reset(struct phy_device *phydev)
{
	int ret;

	ret = qce1204_phy_clk_reset_assert(phydev, true);
	if (ret < 0)
		return ret;
	mdelay(1);
	ret = qce1204_phy_clk_reset_assert(phydev, false);
	if (ret < 0)
		return ret;

	return 0;
}

/**
 * qce1204_phy_sys_reset_assert - Assert/deassert PHY SYS reset
 * @phydev: PHY device
 * @assert: true to assert reset, false to deassert
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_phy_sys_reset_assert(struct phy_device *phydev, bool assert)
{
	struct qce1204_clk_data *clk_data;
	int ret;

	clk_data = qce1204_get_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	/* Reset is optional, return success if not present */
	if (!clk_data->sys_reset)
		return 0;

	if (assert)
		ret = reset_control_assert(clk_data->sys_reset);
	else
		ret = reset_control_deassert(clk_data->sys_reset);
	if (ret < 0) {
		phydev_err(phydev, "Failed to %s SYS reset: %d\n",
			   assert ? "assert" : "deassert", ret);
		return ret;
	}

	return 0;
}

/**
 * qce1204_phy_sys_reset - Reset PHY SYS (assert then deassert)
 * @phydev: PHY device
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_phy_sys_reset(struct phy_device *phydev)
{
	int ret;

	ret = qce1204_phy_sys_reset_assert(phydev, true);
	if (ret < 0)
		return ret;
	mdelay(10);
	ret = qce1204_phy_sys_reset_assert(phydev, false);
	if (ret < 0)
		return ret;

	return 0;
}

/**
 * qce1204_clk_set_rate - Set clock rate based on link speed
 * Only sets rate for clocks that are defined in DTS
 */
static int qce1204_pcs_clk_set_rate(struct phy_device *phydev, u32 channel,
	unsigned long gmii_clk_rate, unsigned long xgmii_clk_rate)
{
	struct qce1204_shared_clk_data *clk_data;
	struct qce1204_channel_clk *ch_clk;
	int ret;

	/* Validate channel range */
	if (channel < 1 || channel > 4) {
		phydev_err(phydev, "Invalid channel: %d (must be 1-4)\n", channel);
		return -EINVAL;
	}

	/* Validate clock rates */
	ret = qce1204_validate_clock_rate(gmii_clk_rate);
	if (ret < 0) {
		phydev_err(phydev, "Invalid GMII clock rate: %lu Hz\n", gmii_clk_rate);
		return ret;
	}

	ret = qce1204_validate_clock_rate(xgmii_clk_rate);
	if (ret < 0) {
		phydev_err(phydev, "Invalid XGMII clock rate: %lu Hz\n", xgmii_clk_rate);
		return ret;
	}

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	ch_clk = &clk_data->channels[channel - 1];

	/* Set GMII TX clock rate */
	if (ch_clk->clks[QCE1204_CLK_GMII_TX]) {
		ret = clk_set_rate(ch_clk->clks[QCE1204_CLK_GMII_TX], gmii_clk_rate);
		if (ret < 0) {
			phydev_err(phydev, "Failed to set GMII TX clock rate: %d\n", ret);
			return ret;
		}
	}

	/* Set GMII RX clock rate */
	if (ch_clk->clks[QCE1204_CLK_GMII_RX]) {
		ret = clk_set_rate(ch_clk->clks[QCE1204_CLK_GMII_RX], gmii_clk_rate);
		if (ret < 0) {
			phydev_err(phydev, "Failed to set GMII RX clock rate: %d\n", ret);
			return ret;
		}
	}

	/* Set XGMII TX clock rate */
	if (ch_clk->clks[QCE1204_CLK_XGMII_TX]) {
		ret = clk_set_rate(ch_clk->clks[QCE1204_CLK_XGMII_TX], xgmii_clk_rate);
		if (ret < 0) {
			phydev_err(phydev, "Failed to set XGMII TX clock rate: %d\n", ret);
			return ret;
		}
	}

	/* Set XGMII RX clock rate */
	if (ch_clk->clks[QCE1204_CLK_XGMII_RX]) {
		ret = clk_set_rate(ch_clk->clks[QCE1204_CLK_XGMII_RX], xgmii_clk_rate);
		if (ret < 0) {
			phydev_err(phydev, "Failed to set XGMII RX clock rate: %d\n", ret);
			return ret;
		}
	}

	return 0;
}

/**
 * qce1204_xpcs_reset_assert - Assert/deassert XPCS reset
 * @phydev: PHY device
 * @assert: true to assert reset, false to deassert
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_xpcs_reset_assert(struct phy_device *phydev, bool assert)
{
	struct qce1204_shared_clk_data *clk_data;
	int ret;

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	/* Reset is optional, return success if not present */
	if (!clk_data->xpcs_reset)
		return 0;

	if (assert)
		ret = reset_control_assert(clk_data->xpcs_reset);
	else
		ret = reset_control_deassert(clk_data->xpcs_reset);
	if (ret < 0) {
		phydev_err(phydev, "Failed to %s XPCS reset: %d\n",
			   assert ? "assert" : "deassert", ret);
		return ret;
	}

	return 0;
}

/**
 * qce1204_pcs_gmii_tx_reset_assert - Assert/deassert GMII TX reset for a channel
 * @phydev: PHY device
 * @channel: Channel number (0-3)
 * @assert: true to assert reset, false to deassert
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_pcs_gmii_tx_reset_assert(struct phy_device *phydev, u32 channel, bool assert)
{
	struct qce1204_shared_clk_data *clk_data;
	struct reset_control *reset;
	int ret;

	/* Validate channel range */
	if (channel < 1 || channel > 4) {
		phydev_err(phydev, "Invalid channel: %d (must be 1-4)\n", channel);
		return -EINVAL;
	}

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	reset = clk_data->channels[channel - 1].resets[QCE1204_CLK_GMII_TX];

	/* Reset is optional, return success if not present */
	if (!reset)
		return 0;

	if (assert)
		ret = reset_control_assert(reset);
	else
		ret = reset_control_deassert(reset);
	if (ret < 0) {
		phydev_err(phydev, "Failed to %s CH%d GMII TX reset: %d\n",
			   assert ? "assert" : "deassert", channel, ret);
		return ret;
	}

	return 0;
}

/**
 * qce1204_pcs_gmii_rx_reset_assert - Assert/deassert GMII RX reset for a channel
 * @phydev: PHY device
 * @channel: Channel number (0-3)
 * @assert: true to assert reset, false to deassert
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_pcs_gmii_rx_reset_assert(struct phy_device *phydev, u32 channel, bool assert)
{
	struct qce1204_shared_clk_data *clk_data;
	struct reset_control *reset;
	int ret;

	/* Validate channel range */
	if (channel < 1 || channel > 4) {
		phydev_err(phydev, "Invalid channel: %d (must be 1-4)\n", channel);
		return -EINVAL;
	}

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	reset = clk_data->channels[channel - 1].resets[QCE1204_CLK_GMII_RX];

	/* Reset is optional, return success if not present */
	if (!reset)
		return 0;

	if (assert)
		ret = reset_control_assert(reset);
	else
		ret = reset_control_deassert(reset);
	if (ret < 0) {
		phydev_err(phydev, "Failed to %s CH%d GMII RX reset: %d\n",
			   assert ? "assert" : "deassert", channel, ret);
		return ret;
	}

	return 0;
}

/**
 * qce1204_pcs_xgmii_tx_reset_assert - Assert/deassert XGMII TX reset for a channel
 * @phydev: PHY device
 * @channel: Channel number (0-3)
 * @assert: true to assert reset, false to deassert
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_pcs_xgmii_tx_reset_assert(struct phy_device *phydev, u32 channel, bool assert)
{
	struct qce1204_shared_clk_data *clk_data;
	struct reset_control *reset;
	int ret;

	/* Validate channel range */
	if (channel < 1 || channel > 4) {
		phydev_err(phydev, "Invalid channel: %d (must be 1-4)\n", channel);
		return -EINVAL;
	}

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	reset = clk_data->channels[channel - 1].resets[QCE1204_CLK_XGMII_TX];

	/* Reset is optional, return success if not present */
	if (!reset)
		return 0;

	if (assert)
		ret = reset_control_assert(reset);
	else
		ret = reset_control_deassert(reset);
	if (ret < 0) {
		phydev_err(phydev, "Failed to %s CH%d XGMII TX reset: %d\n",
			   assert ? "assert" : "deassert", channel, ret);
		return ret;
	}

	return 0;
}

/**
 * qce1204_pcs_xgmii_rx_reset_assert - Assert/deassert XGMII RX reset for a channel
 * @phydev: PHY device
 * @channel: Channel number (0-3)
 * @assert: true to assert reset, false to deassert
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_pcs_xgmii_rx_reset_assert(struct phy_device *phydev, u32 channel, bool assert)
{
	struct qce1204_shared_clk_data *clk_data;
	struct reset_control *reset;
	int ret;

	/* Validate channel range */
	if (channel < 1 || channel > 4) {
		phydev_err(phydev, "Invalid channel: %d (must be 1-4)\n", channel);
		return -EINVAL;
	}

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	reset = clk_data->channels[channel - 1].resets[QCE1204_CLK_XGMII_RX];

	/* Reset is optional, return success if not present */
	if (!reset)
		return 0;

	if (assert)
		ret = reset_control_assert(reset);
	else
		ret = reset_control_deassert(reset);
	if (ret < 0) {
		phydev_err(phydev, "Failed to %s CH%d XGMII RX reset: %d\n",
			   assert ? "assert" : "deassert", channel, ret);
		return ret;
	}

	return 0;
}

/**
 * qce1204_pcs_clk_assert - Assert/deassert PCS resets for a channel
 * @phydev: PHY device
 * @channel: Channel number (0-3)
 * @assert: true to assert reset, false to deassert
 * Returns: 0 on success, negative error code on failure
 *
 * This function controls all PCS-related resets for a specific channel:
 * - GMII TX reset
 * - GMII RX reset
 * - XGMII TX reset
 * - XGMII RX reset
 */
static int qce1204_pcs_clk_reset_assert(struct phy_device *phydev, u32 channel, bool assert)
{
	int ret;

	/* Control GMII TX reset */
	ret = qce1204_pcs_gmii_tx_reset_assert(phydev, channel, assert);
	if (ret < 0)
		return ret;

	/* Control GMII RX reset */
	ret = qce1204_pcs_gmii_rx_reset_assert(phydev, channel, assert);
	if (ret < 0)
		return ret;

	/* Control XGMII TX reset */
	ret = qce1204_pcs_xgmii_tx_reset_assert(phydev, channel, assert);
	if (ret < 0)
		return ret;

	/* Control XGMII RX reset */
	ret = qce1204_pcs_xgmii_rx_reset_assert(phydev, channel, assert);
	if (ret < 0)
		return ret;

	return 0;
}

static int qce1204_pcs_clk_reset(struct phy_device *phydev, u32 channel)
{
	int ret;

	ret = qce1204_pcs_clk_reset_assert(phydev, channel, true);
	if (ret < 0)
		return ret;
	mdelay(1);
	ret = qce1204_pcs_clk_reset_assert(phydev, channel, false);

	return ret;
}

/**
 * qce1204_pcs_gmii_tx_clk_set - Enable/disable GMII TX clock for a channel
 * @phydev: PHY device
 * @channel: Channel number (0-3)
 * @enable: true to enable clock, false to disable
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_pcs_gmii_tx_clk_set(struct phy_device *phydev, u32 channel, bool enable)
{
	struct qce1204_shared_clk_data *clk_data;
	struct clk *clk;
	int ret;

	/* Validate channel range */
	if (channel < 1 || channel > 4) {
		phydev_err(phydev, "Invalid channel: %d (must be 1-4)\n", channel);
		return -EINVAL;
	}

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	clk = clk_data->channels[channel - 1].clks[QCE1204_CLK_GMII_TX];

	/* Clock is optional, return success if not present */
	if (!clk)
		return 0;

	if (enable) {
		ret = clk_prepare_enable(clk);
		if (ret < 0) {
			phydev_err(phydev, "Failed to enable CH%d GMII TX clock: %d\n",
				   channel, ret);
			return ret;
		}
	} else {
		clk_disable_unprepare(clk);
	}

	return 0;
}

/**
 * qce1204_pcs_gmii_rx_clk_set - Enable/disable GMII RX clock for a channel
 * @phydev: PHY device
 * @channel: Channel number (0-3)
 * @enable: true to enable clock, false to disable
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_pcs_gmii_rx_clk_set(struct phy_device *phydev, u32 channel, bool enable)
{
	struct qce1204_shared_clk_data *clk_data;
	struct clk *clk;
	int ret;

	/* Validate channel range */
	if (channel < 1 || channel > 4) {
		phydev_err(phydev, "Invalid channel: %d (must be 1-4)\n", channel);
		return -EINVAL;
	}

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	clk = clk_data->channels[channel - 1].clks[QCE1204_CLK_GMII_RX];

	/* Clock is optional, return success if not present */
	if (!clk)
		return 0;

	if (enable) {
		ret = clk_prepare_enable(clk);
		if (ret < 0) {
			phydev_err(phydev, "Failed to enable CH%d GMII RX clock: %d\n",
				   channel, ret);
			return ret;
		}
	} else {
		clk_disable_unprepare(clk);
	}

	return 0;
}

/**
 * qce1204_pcs_xgmii_tx_clk_set - Enable/disable XGMII TX clock for a channel
 * @phydev: PHY device
 * @channel: Channel number (0-3)
 * @enable: true to enable clock, false to disable
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_pcs_xgmii_tx_clk_set(struct phy_device *phydev, u32 channel, bool enable)
{
	struct qce1204_shared_clk_data *clk_data;
	struct clk *clk;
	int ret;

	/* Validate channel range */
	if (channel < 1 || channel > 4) {
		phydev_err(phydev, "Invalid channel: %d (must be 1-4)\n", channel);
		return -EINVAL;
	}

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	clk = clk_data->channels[channel - 1].clks[QCE1204_CLK_XGMII_TX];

	/* Clock is optional, return success if not present */
	if (!clk)
		return 0;

	if (enable) {
		ret = clk_prepare_enable(clk);
		if (ret < 0) {
			phydev_err(phydev, "Failed to enable CH%d XGMII TX clock: %d\n",
				   channel, ret);
			return ret;
		}
	} else {
		clk_disable_unprepare(clk);
	}

	return 0;
}

/**
 * qce1204_pcs_xgmii_rx_clk_set - Enable/disable XGMII RX clock for a channel
 * @phydev: PHY device
 * @channel: Channel number (0-3)
 * @enable: true to enable clock, false to disable
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_pcs_xgmii_rx_clk_set(struct phy_device *phydev, u32 channel, bool enable)
{
	struct qce1204_shared_clk_data *clk_data;
	struct clk *clk;
	int ret;

	/* Validate channel range */
	if (channel < 1 || channel > 4) {
		phydev_err(phydev, "Invalid channel: %d (must be 1-4)\n", channel);
		return -EINVAL;
	}

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	clk = clk_data->channels[channel - 1].clks[QCE1204_CLK_XGMII_RX];

	/* Clock is optional, return success if not present */
	if (!clk)
		return 0;

	if (enable) {
		ret = clk_prepare_enable(clk);
		if (ret < 0) {
			phydev_err(phydev, "Failed to enable CH%d XGMII RX clock: %d\n",
				   channel, ret);
			return ret;
		}
	} else {
		clk_disable_unprepare(clk);
	}

	return 0;
}

/**
 * qce1204_pcs_clk_set - Enable/disable PCS clocks for a channel
 * @phydev: PHY device
 * @channel: Channel number (0-3)
 * @enable: true to enable clocks, false to disable
 * Returns: 0 on success, negative error code on failure
 *
 * This function controls all PCS-related clocks for a specific channel:
 * - GMII TX clock
 * - GMII RX clock
 * - XGMII TX clock
 * - XGMII RX clock
 */
static int qce1204_pcs_clk_set(struct phy_device *phydev, u32 channel, bool enable)
{
	int ret;

	/* Control GMII TX clock */
	ret = qce1204_pcs_gmii_tx_clk_set(phydev, channel, enable);
	if (ret < 0)
		return ret;

	/* Control GMII RX clock */
	ret = qce1204_pcs_gmii_rx_clk_set(phydev, channel, enable);
	if (ret < 0)
		return ret;

	/* Control XGMII TX clock */
	ret = qce1204_pcs_xgmii_tx_clk_set(phydev, channel, enable);
	if (ret < 0)
		return ret;

	/* Control XGMII RX clock */
	ret = qce1204_pcs_xgmii_rx_clk_set(phydev, channel, enable);
	if (ret < 0)
		return ret;

	return 0;
}

/**
 * qce1204_ahb_clk_set_rate - Set AHB clock rate
 * @phydev: PHY device
 * @rate: Clock rate in Hz
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_ahb_clk_set_rate(struct phy_device *phydev, unsigned long rate)
{
	struct qce1204_shared_clk_data *clk_data;
	int ret;

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	/* Clock is optional, return success if not present */
	if (!clk_data->ahb_clk)
		return 0;

	/* Validate clock rate */
	ret = qce1204_validate_clock_rate(rate);
	if (ret < 0) {
		phydev_err(phydev, "Invalid AHB clock rate: %lu Hz\n", rate);
		return ret;
	}

	ret = clk_set_rate(clk_data->ahb_clk, rate);
	if (ret < 0) {
		phydev_err(phydev, "Failed to set AHB clock rate to %lu Hz: %d\n", rate, ret);
		return ret;
	}

	return 0;
}

/**
 * qce1204_pcs_sys_clk_set - Enable/disable PCS SYS clock
 * @phydev: PHY device
 * @enable: true to enable clock, false to disable
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_pcs_sys_clk_set(struct phy_device *phydev, bool enable)
{
	struct qce1204_shared_clk_data *clk_data;
	int ret = 0;

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	/* Clock is optional, return success if not present */
	if (!clk_data->pcs_sys_clk)
		return 0;

	if (enable) {
		ret = clk_prepare_enable(clk_data->pcs_sys_clk);
		if (ret < 0)
			phydev_err(phydev, "Failed to enable PCS SYS clock: %d\n", ret);
	} else {
		clk_disable_unprepare(clk_data->pcs_sys_clk);
	}

	return ret;
}

/**
 * qce1204_pcs_sys_reset_assert - Assert/deassert PCS SYS reset
 * @phydev: PHY device
 * @assert: true to assert reset, false to deassert
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_pcs_sys_reset_assert(struct phy_device *phydev, bool assert)
{
	struct qce1204_shared_clk_data *clk_data;
	int ret;

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	/* Reset is optional, return success if not present */
	if (!clk_data->pcs_sys_reset)
		return 0;

	if (assert)
		ret = reset_control_assert(clk_data->pcs_sys_reset);
	else
		ret = reset_control_deassert(clk_data->pcs_sys_reset);
	if (ret < 0) {
		phydev_err(phydev, "Failed to %s PCS SYS reset: %d\n",
			   assert ? "assert" : "deassert", ret);
		return ret;
	}

	return 0;
}

/**
 * qce1204_pcs_sys_reset - Reset PCS SYS (assert then deassert)
 * @phydev: PHY device
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_pcs_sys_reset(struct phy_device *phydev)
{
	int ret;

	ret = qce1204_pcs_sys_reset_assert(phydev, true);
	if (ret < 0)
		return ret;
	mdelay(10);
	ret = qce1204_pcs_sys_reset_assert(phydev, false);
	if (ret < 0)
		return ret;

	return 0;
}

/**
 * qce1204_pcs_sys_clk_set_rate - Set PCS system clock rate
 * @phydev: PHY device
 * @rate: Clock rate in Hz
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_pcs_sys_clk_set_rate(struct phy_device *phydev, unsigned long rate)
{
	struct qce1204_shared_clk_data *clk_data;
	int ret;

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;

	/* Clock is optional, return success if not present */
	if (!clk_data->pcs_sys_clk)
		return 0;

	/* Validate clock rate */
	ret = qce1204_validate_clock_rate(rate);
	if (ret < 0) {
		phydev_err(phydev, "Invalid PCS system clock rate: %lu Hz\n", rate);
		return ret;
	}

	ret = clk_set_rate(clk_data->pcs_sys_clk, rate);
	if (ret < 0) {
		phydev_err(phydev, "Failed to set pcs system clock rate to %lu Hz: %d\n", rate, ret);
		return ret;
	}

	return 0;
}

static int _qce1204_pcs_qusgmii_mode_set(struct phy_device *phydev)
{
	int ret = 0, channel = 0;

	/* Uniphy MSLDO settings */
	ret = qce1204_pcs_write_mmd(phydev, MDIO_MMD_PMAPMD,
		QCE1204_PCS_MMD1_MS_LDO0, 0xcd);
	if (ret < 0)
		return ret;
	ret = qce1204_pcs_write_mmd(phydev, MDIO_MMD_PMAPMD,
		QCE1204_PCS_MMD1_MS_LDO1, 0x7f6d);
	if (ret < 0)
		return ret;
	/* assert xpcs */
	ret = qce1204_xpcs_reset_assert(phydev, true);
	if (ret < 0)
		return ret;

	/* select xpcs mode */
	ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PMAPMD,
		QCE1204_PCS_MMD1_MODE_CTRL, 0x1f00, QCE1204_PCS_MMD1_XPCS_MODE);
	if (ret < 0)
		return ret;
	/* reset and release PCS GMII XGMII */
	for(channel = 1; channel <= 4; channel++) {
		ret = qce1204_pcs_clk_reset(phydev, channel);
		if (ret < 0)
			return ret;
	}
	/* ana sw reset and release */
	ret = qce1204_pcs_ana_assert(phydev, true);
	if (ret < 0)
		return ret;
	ret = qce1204_pcs_ana_assert(phydev, false);
	if (ret < 0)
		return ret;
	/* Wait calibration done */
	qce1204_pcs_calibration(phydev);
	/* open ssc clock */
	ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PMAPMD,
		QCE1204_PCS_MMD1_SSC_CLK, QCE1204_PCS_MMD1_SSC_CLK_EN,
		QCE1204_PCS_MMD1_SSC_CLK_EN);
	if (ret < 0)
		return ret;
	/* Enable SSCG(Spread Spectrum Clock Generator) */
	ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PMAPMD,
		QCE1204_PCS_MMD1_CDA_CONTROL1, 0x8, QCE1204_PCS_MMD1_SSCG_ENABLE);
	if (ret < 0)
		return ret;
	/* de-assert XPCS */
	ret = qce1204_xpcs_reset_assert(phydev, false);
	if (ret < 0)
		return ret;
	/* Set BaseR mode */
	ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_PCS_CTRL2, 0xf, QCE1204_PCS_MMD3_PCS_TYPE_10GBASE_R);
	if (ret < 0)
		return ret;
	/* wait PCS 10G link up */
	ret = qce1204_pcs_10g_linkup(phydev);
	if (ret < 0)
		return ret;
	/* enable QUSGMII mode */
	ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_DIG_CTRL1, 0x200, QCE1204_PCS_MMD3_QUSGMII_EN);
	if (ret < 0)
		return ret;
	/* set QUSGMII mode */
	ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_VR_RPCS_TPC, 0x1c00, QCE1204_PCS_MMD3_QUSGMII_MODE);
	if (ret < 0)
		return ret;
	/* set xpcs speed as 10M */
	for (channel = 1; channel <= 4; channel++) {
		ret = qce1204_pcs_modify_channel_mmd(phydev, channel, QCE1204_PCS_MMD_MII_CTRL,
			QCE1204_PCS_SPEED_MASK, QCE1204_PCS_SPEED_10M);
		if (ret < 0)
			return ret;
	}
	/* set AM interval */
	ret = qce1204_pcs_write_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_MII_AM_INTERVAL, QCE1204_PCS_MMD3_MII_AM_INTERVAL_VAL);
	if (ret < 0)
		return ret;
	/* xpcs software reset */
	ret = qce1204_pcs_soft_reset(phydev);

	return ret;
}

static int qce1204_pcs_qusgmii_mode_set(struct phy_device *phydev)
{
	int ret = 0;
	u32 channel = 0;

	/* disable IPG_tuning bypass */
	ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PMAPMD,
		QCE1204_PCS_MMD1_BYPASS_TUNING_IPG,
		QCE1204_PCS_MMD1_BYPASS_TUNING_IPG_EN, 0);
	if (ret < 0)
		return ret;
	/* configure qusgmii mode */
	ret = _qce1204_pcs_qusgmii_mode_set(phydev);
	if (ret < 0)
		return ret;
	/*
	* enable auto-neg complete interrupt,Mii using mii-4bits,
	* configure as PHY mode, enable autoneg ability
	*/
	for (channel = 1; channel <= 4; channel++)
	{
		/*
		* enable auto-neg complete interrupt,Mii using mii-4bits,
		* configure as PHY mode
		*/
		ret = qce1204_pcs_modify_channel_mmd(phydev, channel,
			QCE1204_PCS_MMD_MII_AN_INT_MSK, 0x109,
			QCE1204_PCS_MMD_AN_COMPLETE_INT |
			QCE1204_PCS_MMD_MII_4BITS_CTRL |
			QCE1204_PCS_MMD_TX_CONFIG_CTRL);
		if (ret < 0)
			return ret;

		/* enable autoneg ability */
		ret = qce1204_pcs_modify_channel_mmd(phydev, channel,
			QCE1204_PCS_MMD_MII_CTRL, QCE1204_PCS_MMD_MII_AN_ENABLE,
			QCE1204_PCS_MMD_MII_AN_ENABLE);
		if (ret < 0)
			return ret;
		/* disable TICD */
		ret = qce1204_pcs_modify_channel_mmd(phydev, channel,
			QCE1204_PCS_MMD_MII_XAUI_MODE_CTRL, 0x1,
			QCE1204_PCS_MMD_TX_IPG_CHECK_DISABLE);
		if (ret < 0)
			return ret;
		/* enable PHY mode control to sync phy link information to XPCS */
		ret = qce1204_pcs_modify_channel_mmd(phydev, channel,
			QCE1204_PCS_MMD_MII_DIG_CTRL, BIT(0),
			QCE1204_PCS_MMD_PHY_MODE_CTRL_EN);
		if (ret < 0)
			return ret;
	}

	/* enable EEE for xpcs */
	ret = qce1204_pcs_8023az_enable(phydev);

	return ret;
}

static int qce1204_pcs_speed_clock_set(struct phy_device *phydev,
	u32 channel, u32 speed)
{
	unsigned long gmii_clk_rate, xgmii_clk_rate;

	/* Determine clock rates based on speed */
	switch (speed) {
	case SPEED_2500:
		gmii_clk_rate = QCE1204_CLK_RATE_312P5M;
		xgmii_clk_rate = QCE1204_CLK_RATE_78P125M;
		break;
	case SPEED_1000:
		gmii_clk_rate = QCE1204_CLK_RATE_125M;
		xgmii_clk_rate = QCE1204_CLK_RATE_125M;
		break;
	case SPEED_100:
		gmii_clk_rate = QCE1204_CLK_RATE_25M;
		xgmii_clk_rate = QCE1204_CLK_RATE_25M;
		break;
	case SPEED_10:
		gmii_clk_rate = QCE1204_CLK_RATE_2P5M;
		xgmii_clk_rate = QCE1204_CLK_RATE_2P5M;
		break;
	default:
		phydev_err(phydev, "Unsupported speed: %d\n", speed);
		return -EOPNOTSUPP;
	}

	return qce1204_pcs_clk_set_rate(phydev, channel, gmii_clk_rate, xgmii_clk_rate);
}

static int qce1204_pcs_qusgmii_function_reset(struct phy_device *phydev,
	u32 channel)
{
	int ret = 0;

	if (channel == 1)
		ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
			QCE1204_PCS_MMD_MII_DIG_CTRL,
			0x400, QCE1204_PCS_MMD3_QUSGMII_FIFO_RESET);
	else
		ret = qce1204_pcs_modify_channel_mmd(phydev, channel,
			QCE1204_PCS_MMD_MII_DIG_CTRL,
			QCE1204_PCS_MMD_QUSGMII_FIFO_RESET,
			QCE1204_PCS_MMD_QUSGMII_FIFO_RESET);

	return ret;
}

static int qce1204_pcs_qusgmii_reset(struct phy_device *phydev, u32 channel)
{
	int ret;

	ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PMAPMD,
		QCE1204_PCS_MMD1_QUSGMII_RESET, BIT(channel - 1), 0);
	if (ret < 0)
		return ret;
	mdelay(1);
	ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PMAPMD,
		QCE1204_PCS_MMD1_QUSGMII_RESET, BIT(channel - 1), BIT(channel - 1));

	return ret;
}

static int qce1204_phy_fifo_reset(struct phy_device *phydev, bool enable)
{
	u16 phy_data = 0;

	if (!enable)
		phy_data |= QCE1204_PHY_FIFO_RESET;

	return phy_modify_mmd(phydev, MDIO_MMD_VEND2,
		QCE1204_PHY_CONTROL,
		QCE1204_PHY_FIFO_RESET, phy_data);
}

int qce1204_phy_soft_reset(struct phy_device *phydev)
{
	return phy_modify_mmd(phydev, MDIO_MMD_VEND2,
		MII_BMCR, BMCR_RESET, BMCR_RESET);
}

static int qce1204_phy_mdix_ctrl_set(struct phy_device *phydev)
{
	int ret;
	u16 val;

	switch (phydev->mdix_ctrl) {
	case ETH_TP_MDI:
		val = QCE1204_PHY_MDI;
		break;
	case ETH_TP_MDI_X:
		val = QCE1204_PHY_MDI_X;
		break;
	case ETH_TP_MDI_AUTO:
		val = QCE1204_PHY_MDI_AUTO;
		break;
	default:
		return 0;
	}
	ret = phy_modify_mmd_changed(phydev, MDIO_MMD_VEND2,
		QCE1204_PHY_SPEC_CONTROL, QCE1204_PHY_MDI_MASK,
		FIELD_PREP(QCE1204_PHY_MDI_MASK, val));
	if (ret <= 0)
		return ret;

	return qce1204_phy_soft_reset(phydev);
}

static int qce1204_phy_mdix_ctrl_get(struct phy_device *phydev)
{
	int ret;

	ret = phy_read_mmd(phydev, MDIO_MMD_VEND2, QCE1204_PHY_SPEC_CONTROL);
	if (ret < 0)
		return ret;
	switch (FIELD_GET(QCE1204_PHY_MDI_MASK, ret)) {
	case QCE1204_PHY_MDI:
		phydev->mdix_ctrl = ETH_TP_MDI;
		break;
	case QCE1204_PHY_MDI_X:
		phydev->mdix_ctrl = ETH_TP_MDI_X;
		break;
	case QCE1204_PHY_MDI_AUTO:
		phydev->mdix_ctrl = ETH_TP_MDI_AUTO;
		break;
	}

	return 0;
}

int qce1204_phy_config_aneg(struct phy_device *phydev)
{
	bool changed = false;
	u16 reg = 0;
	int ret = 0;

	if (phydev->autoneg == AUTONEG_DISABLE) {
		int duplex_val = BMCR_FULLDPLX, duplex_tmp = DUPLEX_FULL;
		/*
		* genphy_c45_pma_setup_forced only support duplex full,
		* so need to set duplex as full to configure speed
		* when duplex is half
		*/
		duplex_tmp = phydev->duplex;
		if (phydev->duplex == DUPLEX_HALF) {
			phydev->duplex = DUPLEX_FULL;
			duplex_val = 0;
		}
		ret = genphy_c45_pma_setup_forced(phydev);
		if (ret < 0)
			return ret;
		phydev->duplex = duplex_tmp;
		return phy_modify_mmd(phydev, MDIO_MMD_VEND2, MII_BMCR,
			BMCR_FULLDPLX, duplex_val);
	}

	ret = genphy_c45_an_config_aneg(phydev);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	/*
	* Clause 45 has no standardized support for 1000BaseT,
	* therefore use vendor registers
	*/
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
		phydev->advertising))
		reg |= QCE1204_PHY_ADVERTISE_1000FULL;
	ret = phy_modify_mmd_changed(phydev, MDIO_MMD_VEND2,
		QCE1204_PHY_1000BASET_CONTROL,
		QCE1204_PHY_ADVERTISE_1000FULL,
		reg);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	ret = qce1204_phy_mdix_ctrl_set(phydev);
	if (ret < 0)
		return ret;

	return genphy_c45_check_and_restart_aneg(phydev, changed);
}

int qce1204_phy_channel_get(struct phy_device *phydev)
{
	return (phydev->mdio.addr - phydev->shared->addr + 1);
}

/**
 * qce1204_phy_shared_clk_init - Initialize shared clocks and resets
 * @phydev: PHY device
 * @dev: Device structure
 * @clk_data: Shared clock data structure
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_phy_shared_clk_init(struct phy_device *phydev, struct device *dev,
				       struct qce1204_shared_clk_data *clk_data)
{
	struct clk_init_entry clk_table[] = {
		{&clk_data->channels[0].clks[QCE1204_CLK_GMII_TX], "ch0_gmii_tx_clk", "CH0 GMII TX clock"},
		{&clk_data->channels[0].clks[QCE1204_CLK_GMII_RX], "ch0_gmii_rx_clk", "CH0 GMII RX clock"},
		{&clk_data->channels[0].clks[QCE1204_CLK_XGMII_TX], "ch0_xgmii_tx_clk", "CH0 XGMII TX clock"},
		{&clk_data->channels[0].clks[QCE1204_CLK_XGMII_RX], "ch0_xgmii_rx_clk", "CH0 XGMII RX clock"},
		{&clk_data->channels[1].clks[QCE1204_CLK_GMII_TX], "ch1_gmii_tx_clk", "CH1 GMII TX clock"},
		{&clk_data->channels[1].clks[QCE1204_CLK_GMII_RX], "ch1_gmii_rx_clk", "CH1 GMII RX clock"},
		{&clk_data->channels[1].clks[QCE1204_CLK_XGMII_TX], "ch1_xgmii_tx_clk", "CH1 XGMII TX clock"},
		{&clk_data->channels[1].clks[QCE1204_CLK_XGMII_RX], "ch1_xgmii_rx_clk", "CH1 XGMII RX clock"},
		{&clk_data->channels[2].clks[QCE1204_CLK_GMII_TX], "ch2_gmii_tx_clk", "CH2 GMII TX clock"},
		{&clk_data->channels[2].clks[QCE1204_CLK_GMII_RX], "ch2_gmii_rx_clk", "CH2 GMII RX clock"},
		{&clk_data->channels[2].clks[QCE1204_CLK_XGMII_TX], "ch2_xgmii_tx_clk", "CH2 XGMII TX clock"},
		{&clk_data->channels[2].clks[QCE1204_CLK_XGMII_RX], "ch2_xgmii_rx_clk", "CH2 XGMII RX clock"},
		{&clk_data->channels[3].clks[QCE1204_CLK_GMII_TX], "ch3_gmii_tx_clk", "CH3 GMII TX clock"},
		{&clk_data->channels[3].clks[QCE1204_CLK_GMII_RX], "ch3_gmii_rx_clk", "CH3 GMII RX clock"},
		{&clk_data->channels[3].clks[QCE1204_CLK_XGMII_TX], "ch3_xgmii_tx_clk", "CH3 XGMII TX clock"},
		{&clk_data->channels[3].clks[QCE1204_CLK_XGMII_RX], "ch3_xgmii_rx_clk", "CH3 XGMII RX clock"},
		{&clk_data->pcs_sys_clk, "pcs_sys_clk", "PCS SYS clock"},
		{&clk_data->ahb_clk, "ahb_clk", "AHB clock"},
		{&clk_data->tx_parent, "tx_parent", "TX parent clock"},
		{&clk_data->rx_parent, "rx_parent", "RX parent clock"},
	};
	struct reset_init_entry reset_table[] = {
		{&clk_data->channels[0].resets[QCE1204_CLK_GMII_TX], "ch0_gmii_tx_reset", "CH0 GMII TX reset"},
		{&clk_data->channels[0].resets[QCE1204_CLK_GMII_RX], "ch0_gmii_rx_reset", "CH0 GMII RX reset"},
		{&clk_data->channels[0].resets[QCE1204_CLK_XGMII_TX], "ch0_xgmii_tx_reset", "CH0 XGMII TX reset"},
		{&clk_data->channels[0].resets[QCE1204_CLK_XGMII_RX], "ch0_xgmii_rx_reset", "CH0 XGMII RX reset"},
		{&clk_data->channels[1].resets[QCE1204_CLK_GMII_TX], "ch1_gmii_tx_reset", "CH1 GMII TX reset"},
		{&clk_data->channels[1].resets[QCE1204_CLK_GMII_RX], "ch1_gmii_rx_reset", "CH1 GMII RX reset"},
		{&clk_data->channels[1].resets[QCE1204_CLK_XGMII_TX], "ch1_xgmii_tx_reset", "CH1 XGMII TX reset"},
		{&clk_data->channels[1].resets[QCE1204_CLK_XGMII_RX], "ch1_xgmii_rx_reset", "CH1 XGMII RX reset"},
		{&clk_data->channels[2].resets[QCE1204_CLK_GMII_TX], "ch2_gmii_tx_reset", "CH2 GMII TX reset"},
		{&clk_data->channels[2].resets[QCE1204_CLK_GMII_RX], "ch2_gmii_rx_reset", "CH2 GMII RX reset"},
		{&clk_data->channels[2].resets[QCE1204_CLK_XGMII_TX], "ch2_xgmii_tx_reset", "CH2 XGMII TX reset"},
		{&clk_data->channels[2].resets[QCE1204_CLK_XGMII_RX], "ch2_xgmii_rx_reset", "CH2 XGMII RX reset"},
		{&clk_data->channels[3].resets[QCE1204_CLK_GMII_TX], "ch3_gmii_tx_reset", "CH3 GMII TX reset"},
		{&clk_data->channels[3].resets[QCE1204_CLK_GMII_RX], "ch3_gmii_rx_reset", "CH3 GMII RX reset"},
		{&clk_data->channels[3].resets[QCE1204_CLK_XGMII_TX], "ch3_xgmii_tx_reset", "CH3 XGMII TX reset"},
		{&clk_data->channels[3].resets[QCE1204_CLK_XGMII_RX], "ch3_xgmii_rx_reset", "CH3 XGMII RX reset"},
		{&clk_data->pcs_sys_reset, "pcs_sys_reset", "PCS SYS reset"},
		{&clk_data->xpcs_reset, "xpcs", "XPCS reset"},
	};
	int ret, i;

	/* Initialize all clocks */
	for (i = 0; i < ARRAY_SIZE(clk_table); i++) {
		ret = qce1204_clk_get(phydev, dev, clk_table[i].clk_ptr,
				clk_table[i].name, clk_table[i].desc);
		if (ret < 0)
			return ret;
	}

	/* Initialize all reset controls */
	for (i = 0; i < ARRAY_SIZE(reset_table); i++) {
		ret = qce1204_reset_get(phydev, dev, reset_table[i].reset_ptr,
					reset_table[i].name, reset_table[i].desc);
		if (ret < 0)
			return ret;
	}

	return 0;
}

/**
 * qce1204_shared_clk_probe - Initialize shared clocks and resets from DTS at package level
 * @phydev: PHY device
 * Returns: 0 on success, negative error code on failure
 *
 * Note: XPCS clock and reset are defined at the package level in DTS and shared
 * by all PHYs in the package. This function should only be called once during
 * package initialization. The shared->priv memory is already allocated by
 * devm_of_phy_package_join().
 */
static int qce1204_shared_clk_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.bus->dev;
	struct device_node *old_of_node;
	struct qce1204_shared_clk_data *clk_data;
	int ret;

	if (!phydev->shared || !phydev->shared->np) {
		phydev_err(phydev, "No package device node found\n");
		return -EINVAL;
	}

	clk_data = qce1204_get_shared_clk_data(phydev);
	if (!clk_data) {
		phydev_err(phydev, "No shared clock data found\n");
		return -EINVAL;
	}

	/* Initialize structure to NULL to ensure consistent state */
	memset(clk_data, 0, sizeof(*clk_data));

	/* Access package information through phydev->shared->np */
	old_of_node = dev->of_node;
	dev->of_node = phydev->shared->np;

	/* Initialize clocks and resets using the correct device node */
	ret = qce1204_phy_shared_clk_init(phydev, dev, clk_data);

	/* Restore original of_node */
	dev->of_node = old_of_node;

	if (ret < 0)
		return ret;

	return 0;
}

/**
 * qce1204_phy_package_mode_probe - Parse package mode from DTS
 * @phydev: PHY device
 * Returns: 0 on success, negative error code on failure
 *
 * This function parses the "qcom,package-mode" property from the device tree
 * and stores it in the shared private data structure. This should only be
 * called once during package initialization.
 */
static int qce1204_phy_package_mode_probe(struct phy_device *phydev)
{
	struct phy_package_shared *shared = phydev->shared;
	struct qce1204_shared_priv *shared_priv;
	struct device_node *node;
	const char *mode_str;
	int ret;

	if (!shared || !shared->np) {
		phydev_err(phydev, "No package device node found\n");
		return -EINVAL;
	}

	shared_priv = (struct qce1204_shared_priv *)shared->priv;
	if (!shared_priv) {
		phydev_err(phydev, "No shared private data found\n");
		return -EINVAL;
	}

	node = shared->np;

	/* Parse "qcom,package-mode" property */
	ret = of_property_read_string(node, "qcom,package-mode", &mode_str);
	if (ret) {
		/* Property is optional, default to PHY_INTERFACE_MODE_NA */
		phydev_info(phydev, "No package-mode specified, using default\n");
		shared_priv->package_mode = PHY_INTERFACE_MODE_NA;
		return 0;
	}

	if (strcasecmp(mode_str, "qusgmii") == 0) {
		shared_priv->package_mode = PHY_INTERFACE_MODE_QUSGMII;
	} else if (strcasecmp(mode_str, "internal") == 0) {
		shared_priv->package_mode = PHY_INTERFACE_MODE_INTERNAL;
	} else {
		phydev_err(phydev, "Invalid package-mode: %s\n", mode_str);
		return -EINVAL;
	}

	return 0;
}

/**
 * qce1204_phy_clk_init - Initialize PHY clocks and resets
 * @phydev: PHY device
 * @dev: Device structure
 * @clk_data: PHY clock data structure
 * Returns: 0 on success, negative error code on failure
 */
static int qce1204_phy_clk_init(struct phy_device *phydev, struct device *dev,
				struct qce1204_clk_data *clk_data)
{
	struct clk_init_entry clk_table[] = {
		{&clk_data->tx_clk, "tx_clk", "TX clock"},
		{&clk_data->rx_clk, "rx_clk", "RX clock"},
		{&clk_data->sys_clk, "sys_clk", "SYS clock"},
		{&clk_data->tx_src_clk, "tx_src_clk", "TX SRC clock"},
		{&clk_data->rx_src_clk, "rx_src_clk", "RX SRC clock"},
	};
	struct reset_init_entry reset_table[] = {
		{&clk_data->tx_reset, "tx_reset", "TX reset"},
		{&clk_data->rx_reset, "rx_reset", "RX reset"},
		{&clk_data->sys_reset, "sys_reset", "SYS reset"},
	};
	int ret, i;

	/* Initialize all clocks */
	for (i = 0; i < ARRAY_SIZE(clk_table); i++) {
		ret = qce1204_clk_get(phydev, dev, clk_table[i].clk_ptr,
					clk_table[i].name, clk_table[i].desc);
		if (ret < 0)
			return ret;
	}

	/* Initialize all reset controls */
	for (i = 0; i < ARRAY_SIZE(reset_table); i++) {
		ret = qce1204_reset_get(phydev, dev, reset_table[i].reset_ptr,
					reset_table[i].name, reset_table[i].desc);
		if (ret < 0)
			return ret;
	}

	return 0;
}

/**
 * qce1204_clk_probe - Initialize clocks and resets from DTS for each PHY
 * @phydev: PHY device
 * Returns: 0 on success, negative error code on failure
 *
 * Note: All clocks and resets are optional. Different PHYs may need different
 * clock configurations, so missing clocks are not treated as errors.
 */
static int qce1204_clk_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	struct qce1204_priv *priv = (struct qce1204_priv *)phydev->priv;
	struct qce1204_clk_data *clk_data;
	int ret;

	if (!priv)
		return -EINVAL;

	clk_data = &priv->clk_data;

	/* Initialize clocks and resets */
	ret = qce1204_phy_clk_init(phydev, dev, clk_data);
	if (ret < 0)
		return ret;

	return 0;
}

int qce1204_phy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	struct qce1204_priv *priv;
	int ret;

	/* Join PHY package and allocate shared private data */
	ret = devm_of_phy_package_join(dev, phydev, sizeof(struct qce1204_shared_priv));
	if (ret < 0)
		return ret;

	/* Allocate private data structure for this PHY */
	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	phydev->priv = priv;

	/* Initialize clocks for this PHY */
	ret = qce1204_clk_probe(phydev);
	if (ret < 0) {
		phydev_err(phydev, "Failed to initialize clocks: %d\n", ret);
		return ret;
	}

	/* Initialize shared clocks only once for the package */
	if (phy_package_probe_once(phydev)) {
		ret = qce1204_shared_clk_probe(phydev);
		if (ret < 0) {
			phydev_err(phydev, "Failed to initialize XPCS clocks: %d\n", ret);
			return ret;
		}

		/* Parse package mode from DTS */
		ret = qce1204_phy_package_mode_probe(phydev);
		if (ret < 0) {
			phydev_err(phydev, "Failed to parse package mode: %d\n", ret);
			return ret;
		}
	}

#if IS_ENABLED(CONFIG_HWMON)
	qce1204_hwmon_probe(phydev);
#endif

	return 0;
}

static int qce1204_phy_ability_fix_up(struct phy_device *phydev)
{
	linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT,
		phydev->supported);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT,
		phydev->supported);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT,
		phydev->advertising);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT,
		phydev->advertising);

	return 0;
}

static int qce1204_phy_cdt_thresh_init(struct phy_device *phydev)
{

	phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_MMD3_CDT_THRESH_CTRL2,
		QCE1204_MMD3_CDT_THRESH_CTRL2_VAL);
	phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_MMD3_CDT_THRESH_CTRL3,
		QCE1204_MMD3_CDT_THRESH_CTRL3_VAL);
	phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_MMD3_CDT_THRESH_CTRL4,
		QCE1204_MMD3_CDT_THRESH_CTRL4_VAL);
	phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_MMD3_CDT_THRESH_CTRL5,
		QCE1204_MMD3_CDT_THRESH_CTRL5_VAL);
	phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_MMD3_CDT_THRESH_CTRL6,
		QCE1204_MMD3_CDT_THRESH_CTRL6_VAL);
	phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_MMD3_CDT_THRESH_CTRL7,
		QCE1204_MMD3_CDT_THRESH_CTRL7_VAL);
	phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_MMD3_CDT_THRESH_CTRL9,
		QCE1204_MMD3_CDT_THRESH_CTRL9_VAL);
	phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_MMD3_CDT_THRESH_CTRL13,
		QCE1204_MMD3_CDT_THRESH_CTRL13_VAL);
	phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_MMD3_CDT_THRESH_CTRL14,
		QCE1204_MMD3_CDT_THRESH_CTRL14_VAL);

	return 0;
}

static int qce1204_phy_eee_init(struct phy_device *phydev)
{
	int ret = 0;

	/* reduce the delay to response the quiet signal for 2.5G EEE */
	ret = phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCE1024_PHY_2P5G_EEE_TX_LPI_QUIET_CTRL0,
		QCE1024_PHY_2P5G_EEE_TX_QUIET_TIME0);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCE1024_PHY_2P5G_EEE_TX_LPI_QUIET_CTRL1,
		QCE1024_PHY_2P5G_EEE_TX_QUIET_TIME1);
	if (ret < 0)
		return ret;
	/* reduce the delay to response the wake up signal for 2.5G EEE */
	ret = phy_write_mmd(phydev, MDIO_MMD_PCS,
		QCE1024_PHY_2P5G_EEE_TX_LPI_WAKE_CTRL,
		QCE1024_PHY_2P5G_EEE_TX_WAKE_TIME);

	return ret;
}

static int qce1204_phy_10m_dac_init(struct phy_device *phydev)
{
	int ret = 0;

	/* adjust the voltage amplitude for 10BASE-T */
	ret = qca81xx_phy_debug_write(phydev, QCE1204_DEBUG_ANA_10M_DAC_CTRL0,
		QCE1204_DEBUG_ANA_10M_DAC_CTRL0_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_debug_write(phydev, QCE1204_DEBUG_ANA_10M_DAC_CTRL1,
		QCE1204_DEBUG_ANA_10M_DAC_CTRL1_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_debug_write(phydev, QCE1204_DEBUG_ANA_10M_DAC_CTRL2,
		QCE1204_DEBUG_ANA_10M_DAC_CTRL2_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_debug_write(phydev, QCE1204_DEBUG_ANA_10M_DAC_CTRL3,
		QCE1204_DEBUG_ANA_10M_DAC_CTRL3_VAL);

	return ret;
}

static int qce1204_phy_tlmm_init(struct phy_device *phydev)
{
	int ret = 0, pin_id = 0;

	/* GPIO0, FUNC 1 */
	ret = qce1204_soc_modify(phydev, TO_TLMM_CFG_REG(QCE1204_GPIO0_PHY_INT),
		QCE1204_TLMM_FUNC_MASK, BIT(2));
	if (ret < 0)
		return ret;
	/* GPIO1~GPIO4, FUNC 1, LED_MODE, DRV_16_MA, NO_PULL */
	for (pin_id  = QCE1204_GPIO1_P0_LED_0; pin_id <= QCE1204_GPIO4_P3_LED_0; pin_id++) {
		ret = qce1204_soc_modify(phydev, TO_TLMM_CFG_REG(pin_id),
			QCE1204_TLMM_GPIO_PULL | QCE1204_TLMM_FUNC_MASK | QCE1204_TLMM_DRV | QCE1204_TLMM_LED_MODE,
			BIT(2) | QCE1204_TLMM_DRV_16_MA | QCE1204_TLMM_LED_MODE);
		if (ret < 0)
			return ret;
	}

	/* GPIO9, FUNC 4 */
	ret = qce1204_soc_modify(phydev, TO_TLMM_CFG_REG(QCE1204_GPIO9_P0_WOL_INT),
		QCE1204_TLMM_FUNC_MASK, BIT(4));
	if (ret < 0)
		return ret;

	/* GPIO15~GPIO17, FUNC 1, LED_MODE, DRV_16_MA, NO_PULL */
	for (pin_id  = QCE1204_GPIO15_P1_WOL_INT; pin_id <= QCE1204_GPIO17_P3_WOL_INT; pin_id++) {
		ret = qce1204_soc_modify(phydev, TO_TLMM_CFG_REG(pin_id),
		QCE1204_TLMM_FUNC_MASK, BIT(3));
		if (ret < 0)
			return ret;
	}

	return 0;
}


int qce1204_phy_config_init(struct phy_device *phydev)
{
	int ret = 0;
	phy_interface_t package_mode;

	package_mode = qce1204_get_package_mode(phydev);
	if (phy_package_init_once(phydev)) {
		ret = qce1204_pcs_sys_clk_set_rate(phydev, QCE1204_CLK_RATE_25M);
		if (ret < 0)
			return ret;
		if (package_mode == PHY_INTERFACE_MODE_QUSGMII) {
			/* configure work mode as PHY */
			ret = qce1204_soc_modify(phydev, QCE1204_WORK_MODE_SEL,
				QCE1204_PHY_MODE_MASK, QCE1204_PHY_MODE);
			if (ret < 0)
				return ret;
			ret = qce1204_pcs_sys_clk_set(phydev, true);
			if (ret < 0)
				return ret;
			ret = qce1204_pcs_sys_reset(phydev);
			if (ret < 0)
				return ret;
			ret = qce1204_pcs_qusgmii_mode_set(phydev);
			if (ret < 0)
				return ret;
			ret = qce1204_ahb_clk_set_rate(phydev, QCE1204_CLK_RATE_104M);
			if (ret < 0)
				return ret;
		} else if (package_mode == PHY_INTERFACE_MODE_INTERNAL) {
			/* configure work mode as switch */
			ret = qce1204_soc_modify(phydev, QCE1204_WORK_MODE_SEL,
				QCE1204_SWITCH_MODE_MASK, QCE1204_SWITCH_MODE);
			if (ret < 0)
				return ret;
		} else {
			phydev_err(phydev, "Unsupported package mode: 0x%x\n", package_mode);
			return -EINVAL;
		}
		ret = qce1204_phy_tlmm_init(phydev);
		if (ret < 0)
			return ret;
#if IS_ENABLED(CONFIG_HWMON)
		qce1204_hwmon_hw_init_once(phydev);
#endif
		/* enable efuse loading into analog circuit */
		ret = qce1204_soc_modify(phydev, QCE1204_EPHY_CFG,
			QCE1204_EPHY_LDO_CTRL, 0);
		if (ret < 0)
			return ret;
		mdelay(10);
	}

	ret = qce1204_phy_clk_parent_init(phydev);
	if (ret < 0)
		return ret;

	ret = qce1204_phy_sys_clk_set(phydev, true);
	if (ret < 0)
		return ret;
	ret = qce1204_phy_sys_reset(phydev);
	if (ret < 0)
		return ret;
	ret = qce1204_phy_eee_init(phydev);
	if (ret < 0)
		return ret;
	ret = qce1204_phy_10m_dac_init(phydev);
	if (ret < 0)
		return ret;
	/* adjust the tx gain to improve 2.5G performance */
	ret = qca81xx_phy_debug_write(phydev, QCE1204_DEBUG_ANA_2P5G_TX_GAIN_CTRL,
		QCE1204_DEBUG_ANA_2P5G_TX_GAIN_VAL);
	if (ret < 0)
		return ret;
	/* force pll0 on to improve traffic performance */
	ret = qca81xx_phy_debug_modify(phydev, QCE1204_DEBUG_PLL_CTRL,
		QCE1204_DEBUG_PLL0_FORCE_ON, QCE1204_DEBUG_PLL0_FORCE_ON);
	if (ret < 0)
		return ret;
	ret = qce1204_phy_cdt_thresh_init(phydev);
	if (ret < 0)
		return ret;
	/* 10M speed also use led0 in default as other speeds */
	ret = phy_modify_mmd(phydev, MDIO_MMD_AN, QCE1204_MMD7_LED0_CTRL,
		QCE1204_SPEED_10M_ON, QCE1204_SPEED_10M_ON);
	if (ret < 0)
		return ret;
	 if (package_mode == PHY_INTERFACE_MODE_QUSGMII) {
		ret = qce1204_phy_ability_fix_up(phydev);
		if (ret < 0)
			return ret;
	 }
#if IS_ENABLED(CONFIG_HWMON)
	qce1204_hwmon_hw_init(phydev);
#endif
	ret = qce1204_phy_soft_reset(phydev);
	if (ret < 0)
		return ret;

	return 0;
}

static int qce1204_phy_qusgmii_speed_fix_up(struct phy_device *phydev)
{
	u32 channel;
	int ret;
	bool clk_en = false;
	bool pcs_clk_enabled = false;
	bool phy_clk_enabled = false;

	/* channel is from 1 ~ 4 */
	channel = qce1204_phy_channel_get(phydev);
	/*
	 * enable pcs clocks and phy clocks for link up
	 * disable pcs clocks and phy clocks for link down
	 */
	if (phydev->link) {
		ret = qce1204_pcs_speed_clock_set(phydev, channel, phydev->speed);
		if (ret < 0)
			return ret;
		mdelay(10);
		clk_en = true;
	}
	ret = qce1204_pcs_clk_set(phydev, channel, clk_en);
	if (ret < 0) {
		phydev_err(phydev, "Failed to %s pcs clocks for CH%d, ret: %d\n",
			clk_en ? "enable" : "disable", channel, ret);
		return ret;
	}
	pcs_clk_enabled = clk_en;
	ret = qce1204_phy_clk_set(phydev, clk_en);
	if (ret < 0) {
		phydev_err(phydev, "Failed to %s phy clocks for CH%d, ret: %d\n",
			clk_en ? "enable" : "disable", channel, ret);
		goto err_disable_pcs_clk;
	}
	phy_clk_enabled = clk_en;
	mdelay(100);
	/* reset pcs clocks and phy clocks */
	ret = qce1204_pcs_clk_reset(phydev, channel);
	if (ret < 0)
		goto err_disable_clks;
	ret = qce1204_phy_clk_reset(phydev);
	if (ret < 0)
		goto err_disable_clks;
	ret = qce1204_pcs_qusgmii_reset(phydev, channel);
	if (ret < 0)
		goto err_disable_clks;
	ret = qce1204_pcs_qusgmii_function_reset(phydev, channel);
	if (ret < 0)
		goto err_disable_clks;
	ret = qce1204_phy_fifo_reset(phydev, true);
	if (ret < 0)
		goto err_disable_clks;
	mdelay(1);
	if (phydev->link) {
		ret = qce1204_phy_fifo_reset(phydev, false);
		if (ret < 0)
			goto err_disable_clks;
	}
	/* change IPG from 10 to 11 for 1G speed */
	ret = phy_modify_mmd(phydev, MDIO_MMD_AN, QCE1204_PHY_MMD7_IPG_OP,
		QCE1204_PHY_IPG_10_TO_11_EN, phydev->speed == SPEED_1000 ?
		QCE1204_PHY_IPG_10_TO_11_EN : 0);
	if (ret < 0)
		goto err_disable_clks;

	return 0;

err_disable_clks:
	if (phy_clk_enabled)
		qce1204_phy_clk_set(phydev, false);
err_disable_pcs_clk:
	if (pcs_clk_enabled)
		qce1204_pcs_clk_set(phydev, channel, false);
	return ret;
}

static int qce1204_phy_speed_clock_set(struct phy_device *phydev)
{
	unsigned long clk_rate = 0;
	struct qce1204_clk_data *clk_data = NULL;
	int ret = 0;

	/* Determine clock rates based on speed */
	switch (phydev->speed) {
	case SPEED_2500:
		clk_rate = QCE1204_CLK_RATE_312P5M;
		break;
	case SPEED_1000:
		clk_rate = QCE1204_CLK_RATE_125M;
		break;
	case SPEED_100:
		clk_rate = QCE1204_CLK_RATE_25M;
		break;
	case SPEED_10:
		clk_rate = QCE1204_CLK_RATE_2P5M;
		break;
	default:
		phydev_err(phydev, "Unsupported speed: %d\n", phydev->speed);
		return -EOPNOTSUPP;
	}
	ret = qce1204_validate_clock_rate(clk_rate);
	if (ret < 0) {
		phydev_err(phydev, "Invalid clock rate: %lu Hz\n", clk_rate);
		return ret;
	}

	clk_data = qce1204_get_clk_data(phydev);
	if (!clk_data)
		return -EINVAL;
	if (clk_data->tx_clk) {
		ret = clk_set_rate(clk_data->tx_clk, clk_rate);
		if (ret < 0) {
			phydev_err(phydev, "Failed to set TX clock rate: %d\n", ret);
			return ret;
		}
	}
	if (clk_data->rx_clk) {
		ret = clk_set_rate(clk_data->rx_clk, clk_rate);
		if (ret < 0) {
			phydev_err(phydev, "Failed to set RX clock rate: %d\n", ret);
			return ret;
		}
	}

	return 0;
}

static int qce1204_phy_internal_speed_fix_up(struct phy_device *phydev)
{
	bool clk_en = false;
	int ret;

	if (phydev->link) {
		ret = qce1204_phy_speed_clock_set(phydev);
		if (ret < 0)
			return ret;
		clk_en = true;
	}
	ret = qce1204_phy_clk_set(phydev, clk_en);
	if (ret < 0) {
		phydev_err(phydev, "Failed to %s phy clocks, ret: %d\n",
			clk_en ? "enable" : "disable", ret);
		return ret;
	}
	/* reset phy clocks */
	ret = qce1204_phy_clk_reset(phydev);
	if (ret < 0)
		goto err_disable_clks;
	ret = qce1204_phy_fifo_reset(phydev, true);
	if (ret < 0)
		goto err_disable_clks;
	mdelay(1);
	ret = qce1204_phy_fifo_reset(phydev, false);
	if (ret < 0)
		goto err_disable_clks;

	return 0;

err_disable_clks:
	if (clk_en)
		qce1204_phy_clk_set(phydev, false);

	return ret;
}

int qce1204_phy_read_status(struct phy_device *phydev)
{
	int ret = 0, old_link = 0;

	old_link = phydev->link;

	ret = genphy_c45_read_status(phydev);
	if (ret < 0)
		return ret;

	if (phydev->autoneg == AUTONEG_ENABLE) {
		ret = phy_read_mmd(phydev, MDIO_MMD_VEND2,
			QCE1204_PHY_1000BASET_STATUS);
		if (ret < 0)
			return ret;
		linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
			phydev->lp_advertising,
			ret & QCE1204_PHY_LP_ADVERTISE_1000FULL);
		/* need to get speed and duplex again with new lp adv */
		phy_resolve_aneg_linkmode(phydev);
	} else {
		ret = phy_read_mmd(phydev, MDIO_MMD_VEND2, MII_BMCR);
		if (ret < 0)
			return ret;
		phydev->duplex = (ret & BMCR_FULLDPLX) ? DUPLEX_FULL : DUPLEX_HALF;
	}

	ret = phy_read_mmd(phydev, MDIO_MMD_VEND2, QCE1204_PHY_SPEC_STATUS);
	if (ret < 0)
		return ret;
	if (ret & QCE1204_PHY_INTR_DOWNSHIFT) {
		switch (ret & QCE1204_PHY_SS_SPEED_MASK) {
		case QCE1204_PHY_SS_SPEED_2500:
			phydev->speed = SPEED_2500;
			break;
		case QCE1204_PHY_SS_SPEED_1000:
			phydev->speed = SPEED_1000;
			break;
		case QCE1204_PHY_SS_SPEED_100:
			phydev->speed = SPEED_100;
			break;
		case QCE1204_PHY_SS_SPEED_10:
			phydev->speed = SPEED_10;
			break;
		default:
			phydev->speed = SPEED_UNKNOWN;
		}
		if (ret & QCE1204_PHY_SS_DUPLEX_FULL)
			phydev->duplex = DUPLEX_FULL;
		else
			phydev->duplex = DUPLEX_HALF;
	}
	phydev->mdix = (ret & QCE1204_PHY_SS_MDIX) ? ETH_TP_MDI_X : ETH_TP_MDI;
	ret = qce1204_phy_mdix_ctrl_get(phydev);
	if (ret < 0)
		return ret;
	if (phydev->link != old_link) {
		if (phydev->interface == PHY_INTERFACE_MODE_QUSGMII) {
			ret = qce1204_phy_qusgmii_speed_fix_up(phydev);
			if (ret < 0)
				return ret;
		}
		if (phydev->interface == PHY_INTERFACE_MODE_INTERNAL) {
			ret = qce1204_phy_internal_speed_fix_up(phydev);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}
