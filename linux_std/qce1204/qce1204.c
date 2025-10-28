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
