/*
* Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
* SPDX-License-Identifier: ISC
*/

#include "qce1204.h"

#define QCE1204_PHY_SPEC_STATUS						0x11
#define QCE1204_PHY_SS_LINK_STATUS					0x400
#define QCE1204_PHY_INTR_DOWNSHIFT					0x20
#define QCE1204_PHY_SS_DUPLEX_FULL					0x2000
#define QCE1204_PHY_SS_SPEED_MASK					0x380
#define QCE1204_PHY_SS_SPEED_2500					0x200
#define QCE1204_PHY_SS_SPEED_1000					0x100
#define QCE1204_PHY_SS_SPEED_100					0x80
#define QCE1204_PHY_SS_SPEED_10						0
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
#define QCE1024_PHY_2P5G_EEE_TX_LPI_CTRL				0xa10c
#define QCE1024_PHY_TX_LPI_DELAY_SEL_MASK				0xf00
#define QCE1024_PHY_TX_LPI_DELAY_SEL_1					0x100

#define QCE1204_MMD3_CDT_THRESH_CTRL14					0x807f
#define QCE1204_MMD3_CDT_THRESH_CTRL14_VAL				0xb6b0
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

#define QCE1204_PCS_MMD1_CDA_CONTROL1					0x20
#define QCE1204_PCS_MMD1_CALIBRATION4					0x78
#define QCE1204_PCS_MMD1_MODE_CTRL					0x11b
#define QCE1204_PCS_MMD1_BYPASS_TUNING_IPG				0x189
#define QCE1204_PCS_MMD1_GMII_DATAPASS_SEL				0x180
#define QCE1204_PCS_MMD1_QUSGMII_RESET					0x18c
#define QCE1204_PCS_MMD1_BYPASS_TUNING_IPG_EN				0x0fff
#define QCE1204_PCS_MMD1_XPCS_MODE					0x1000
#define QCE1204_PCS_MMD1_DATAPASS_MASK					0x1
#define QCE1204_PCS_MMD1_DATAPASS_QUSGMII				0x1
#define QCE1204_PCS_MMD1_CALIBRATION_DONE				0x80
#define QCE1204_PCS_MMD1_QUSGMII_FUNC_RESET				0x10
#define QCE1204_PCS_MMD1_SSCG_ENABLE					0x8
#define QCE1204_PCS_MMD1_PLL_POWER_ON_AND_RESET				0x1e0
#define QCE1204_PCS_MMD1_ANA_SOFT_RESET_MASK				0x40
#define QCE1204_PCS_MMD1_ANA_SOFT_RESET					0
#define QCE1204_PCS_MMD1_ANA_SOFT_RELEASE				0x40
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
#define QCE1204_PCS_MMD_QUSGMII_FIFO_RESET					0x20
#define QCE1204_PCS_MMD_TX_IPG_CHECK_DISABLE				0x1
#define QCE1204_PCS_MMD_PHY_MODE_CTRL_EN				0x1
#define QCE1204_PCS_MMD_CH2						26
#define QCE1204_PCS_MMD_CH3						27
#define QCE1204_PCS_MMD_CH4						28

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

	addr = FIELD_GET(GENMASK(28, 24), reg);
	qce1204_split_addr(reg, &reg_low, &reg_mid, &reg_high);
	/*write ahb address bit4~bit23*/
	__mdiobus_write(phydev->mdio.bus, addr, reg_high & 0x1f, reg_mid);
	udelay(100);
	/*write ahb address bit0~bit3 and read low 16bit data*/
	lo = __mdiobus_read(phydev->mdio.bus, addr, reg_low);
	/*write ahb address bit0~bit3 and read high 16 bit data*/
	hi = __mdiobus_read(phydev->mdio.bus, addr, (reg_low + 4));

	return (hi << 16) | lo;
}

void __qce1204_soc_write(struct phy_device *phydev, u32 reg, u32 val)
{
	u16 reg_low, reg_mid, reg_high;
	u16 lo, hi;
	u32 addr;

	addr = FIELD_GET(GENMASK(28, 24), reg);

	qce1204_split_addr(reg, &reg_low, &reg_mid, &reg_high);
	lo = val & 0xffff;
	hi = (u16)(val >> 16);

	/*write ahb address bit4~bit23*/
	__mdiobus_write(phydev->mdio.bus, addr, reg_high & 0x1f, reg_mid);
	udelay(100);
	/*write ahb address bit0~bit3 and write low 16 bit data*/
	__mdiobus_write(phydev->mdio.bus, addr, reg_low, lo);
	/*write ahb address bit0~bit3 and write high 16 bit data*/
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
	if(!(pcs_data & QCE1204_PCS_MMD3_XPCS_EEE_CAP))
		return -EOPNOTSUPP;

	/*Configure the EEE related timer*/
	qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_EEE_MODE_CTRL, 0x0f40, QCE1204_PCS_MMD3_EEE_RES_REGS |
		QCE1204_PCS_MMD3_EEE_SIGN_BIT_REGS);

	qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_EEE_TX_TIMER, 0x1fff, QCE1204_PCS_MMD3_EEE_TSL_REGS|
		QCE1204_PCS_MMD3_EEE_TLU_REGS | QCE1204_PCS_MMD3_EEE_TWL_REGS);

	qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_EEE_RX_TIMER, 0x1fff, QCE1204_PCS_MMD3_EEE_100US_REG_REGS|
		QCE1204_PCS_MMD3_EEE_RWR_REG_REGS);

	/*enable TRN_LPI*/
	qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_EEE_MODE_CTRL1, 0x101, QCE1204_PCS_MMD3_EEE_TRANS_LPI_MODE|
		QCE1204_PCS_MMD3_EEE_TRANS_RX_LPI_MODE);

	/*enable TX/RX LPI pattern*/
	qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCE1204_PCS_MMD3_EEE_MODE_CTRL, 0x3, QCE1204_PCS_MMD3_EEE_EN);

	return 0;
}

static int qce1204_pcs_calibration(struct phy_device *phydev)
{
	u16 pcs_data = 0;
	u32 retries = 100, calibration_done = 0;

	/* wait calibration done to uniphy*/
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

static int qce1204_pcs_assert(struct phy_device *phydev,
	bool assert)
{
	return qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PMAPMD,
		QCE1204_PCS_MMD1_PLL_POWER_ON_AND_RESET,
		QCE1204_PCS_MMD1_ANA_SOFT_RESET_MASK,
		assert ? QCE1204_PCS_MMD1_ANA_SOFT_RESET :
		QCE1204_PCS_MMD1_ANA_SOFT_RELEASE);
}

static int _qce1204_pcs_qusgmii_mode_set(struct phy_device *phydev)
{
	int ret = 0, channel = 0;

	/* assert xpcs */
	/*clk code*/

	/* select xpcs mode */
	ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PMAPMD,
		QCE1204_PCS_MMD1_MODE_CTRL, 0x1f00, QCE1204_PCS_MMD1_XPCS_MODE);
	if (ret < 0)
		return ret;
	ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PMAPMD,
		QCE1204_PCS_MMD1_GMII_DATAPASS_SEL, QCE1204_PCS_MMD1_DATAPASS_MASK,
		QCE1204_PCS_MMD1_DATAPASS_QUSGMII);
	if (ret < 0)
		return ret;
	/* reset and release PCS GMII/XGMII and PHY GMII */
	for(channel = 1; channel <= 4; channel++) {
		/*to do clk code*/
	}
	/* ana sw reset and release */
	ret = qce1204_pcs_assert(phydev, true);
	if (ret < 0)
		return ret;
	mdelay(10);
	ret = qce1204_pcs_assert(phydev, false);
	if (ret < 0)
		return ret;
	/* Wait calibration done */
	qce1204_pcs_calibration(phydev);
	/* Enable SSCG(Spread Spectrum Clock Generator) */
	ret = qce1204_pcs_modify_mmd(phydev, MDIO_MMD_PMAPMD,
		QCE1204_PCS_MMD1_CDA_CONTROL1, 0x8, QCE1204_PCS_MMD1_SSCG_ENABLE);
	if (ret < 0)
		return ret;
	/* de-assert XPCS */
	/*clk code*/
	/* PHY software reset */
	for(channel = 1; channel <= 4; channel++) {
		mdiobus_modify(phydev->mdio.bus, phydev->shared->addr + channel - 1,
			MII_BMCR, BMCR_RESET, BMCR_RESET);
	}
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
	/* disable PCS GMII/XGMII clock and disable PHY GMII clock */
	for(channel = 1; channel <= 4; channel++)
	{
		/*to do clk code*/
	}
	/* configure qusgmii mode */
	ret = _qce1204_pcs_qusgmii_mode_set(phydev);
	if (ret < 0)
		return ret;
	/* enable auto-neg complete interrupt,Mii using mii-4bits, */
	/* configure as PHY mode, enable autoneg ability */
	for (channel = 1; channel <= 4; channel++)
	{
		/* enable auto-neg complete interrupt,Mii using mii-4bits, */
		/* configure as PHY mode */
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

	/*enable EEE for xpcs*/
	ret = qce1204_pcs_8023az_enable(phydev);

	return ret;
}

int qce1204_phy_config_aneg(struct phy_device *phydev)
{
	bool changed = false;
	u16 reg = 0;
	int ret = 0;

	if (phydev->autoneg == AUTONEG_DISABLE) {
		int duplex_val = BMCR_FULLDPLX, duplex_tmp = DUPLEX_FULL;
		/* genphy_c45_pma_setup_forced only support duplex full, */
		/* so need to set duplex as full to configure speed */
		/* when duplex is half */
		duplex_tmp = phydev->duplex;
		if(phydev->duplex == DUPLEX_HALF) {
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

	/* Clause 45 has no standardized support for 1000BaseT, */
	/* therefore use vendor registers. */
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

	return genphy_c45_check_and_restart_aneg(phydev, changed);
}

int qce1204_phy_soft_reset(struct phy_device *phydev)
{
	return phy_modify_mmd(phydev, MDIO_MMD_VEND2,
		MII_BMCR, BMCR_RESET, BMCR_RESET);
}

int qce1204_phy_ack_interrupt(struct phy_device *phydev)
{
	int ret = 0;

	ret = phy_read_mmd(phydev, MDIO_MMD_VEND2,
		QCE1204_PHY_INTR_STATUS);

	return (ret < 0) ? ret : 0;
}

int qce1204_phy_config_intr(struct phy_device *phydev)
{
	int ret = 0;
	u16 mask = 0;

	mask = QCE1204_PHY_INTR_STATUS_DOWN | QCE1204_PHY_INTR_STATUS_UP;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED) {
		ret = qce1204_phy_ack_interrupt(phydev);
		if (ret < 0)
			return ret;
		ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, QCE1204_PHY_INTR_MASK,
			mask, mask);
	} else {
		ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, QCE1204_PHY_INTR_MASK,
			mask, 0);
		if (ret < 0)
			return ret;
		ret = qce1204_phy_ack_interrupt(phydev);
	}

	return ret;
}

irqreturn_t qce1204_phy_handle_interrupt(struct phy_device *phydev)
{
	int irq_status, int_enabled;

	irq_status = phy_read_mmd(phydev, MDIO_MMD_VEND2, QCE1204_PHY_INTR_STATUS);
	if (irq_status < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	/* Read the current enabled interrupts */
	int_enabled = phy_read_mmd(phydev, MDIO_MMD_VEND2, QCE1204_PHY_INTR_MASK);
	if (int_enabled < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	/* See if this was one of our enabled interrupts */
	if (!(irq_status & int_enabled))
		return IRQ_NONE;

	phy_trigger_machine(phydev);

	return IRQ_HANDLED;
}

static int qce1204_phy_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	int ret;

	ret = devm_of_phy_package_join(dev, phydev, 0);

	return ret;
}

static int qce1204_ability_fix_up(struct phy_device *phydev)
{

	if (phydev->interface != PHY_INTERFACE_MODE_INTERNAL &&
		phydev->interface != PHY_INTERFACE_MODE_GMII) {
		linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT,
			phydev->supported);
		linkmode_clear_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT,
			phydev->supported);
		linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT,
			phydev->advertising);
		linkmode_clear_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT,
			phydev->advertising);
	}

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

static int qce1204_phy_config_init(struct phy_device *phydev)
{
	if (phydev->interface != PHY_INTERFACE_MODE_INTERNAL &&
		phydev->interface != PHY_INTERFACE_MODE_GMII) {
		if (phy_package_init_once(phydev)) {
			if (phydev->interface == PHY_INTERFACE_MODE_QUSGMII)
				qce1204_pcs_qusgmii_mode_set(phydev);
		}
	}

	/* reduce the delay to send wake up signal for 2.5G EEE */
	phy_modify_mmd_changed(phydev, MDIO_MMD_PCS,
		QCE1024_PHY_2P5G_EEE_TX_LPI_CTRL,
		QCE1024_PHY_TX_LPI_DELAY_SEL_MASK,
		QCE1024_PHY_TX_LPI_DELAY_SEL_1);

	qce1204_phy_cdt_thresh_init(phydev);

	return qce1204_ability_fix_up(phydev);
}

int qce1204_phy_read_status(struct phy_device *phydev)
{
	int ret = 0;

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

	return 0;
}

static struct phy_driver qce1204_phy_driver[] = {
{
	PHY_ID_MATCH_EXACT(QCE1204_PHY),
	.name = "Qualcomm QCE1204",
	.flags = PHY_POLL_CABLE_TEST,
	.probe = qce1204_phy_probe,
	.config_init = qce1204_phy_config_init,
	.config_aneg = qce1204_phy_config_aneg,
	.config_intr = qce1204_phy_config_intr,
	.handle_interrupt = qce1204_phy_handle_interrupt,
	.read_status = qce1204_phy_read_status,
	.suspend = genphy_c45_pma_suspend,
	.resume = genphy_c45_pma_resume,
	.soft_reset = qce1204_phy_soft_reset,
},
};

module_phy_driver(qce1204_phy_driver);
MODULE_DESCRIPTION("QCE1204 PHY Driver");
MODULE_LICENSE("Dual BSD/GPL");
