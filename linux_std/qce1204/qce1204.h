/*
* Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
* SPDX-License-Identifier: ISC
*/

#ifndef _QCE1204_H_
#define _QCE1204_H_

#include <linux/phy.h>
#include <linux/clk.h>
#include <linux/reset.h>

#define QCE1204_PHY				0x004dd190
#define QCE1204_TLMM_BASE			0x400000
#define QCE1204_TLMM_GPIO_OFFSET		0x1000
#define QCE1204_TO_TLMM_CFG_REG(pin)		(QCE1204_TLMM_BASE + QCE1204_TLMM_GPIO_OFFSET * pin)
#define QCE1204_TLMM_GPIO_PULL			GENMASK(1, 0)
#define QCE1204_TLMM_FUNC_MASK			GENMASK(5, 2)
#define QCE1204_TLMM_DRV			GENMASK(8, 6)
#define QCE1204_TLMM_DRV_16_MA			0x1c0
#define QCE1204_TLMM_LED_MODE			BIT(11)
enum qce1204_addr_offset {
	PCS0_ADDR_OFFSET = 4,
	PCS1_ADDR_OFFSET = 5,
	QCE1204_SOC_ADDR_OFFSET = 6,
};

struct qce1204_clk_data {
	struct clk *tx_clk;
	struct clk *rx_clk;
	struct clk *sys_clk;
	/* Reset control */
	struct reset_control *tx_reset;
	struct reset_control *rx_reset;
	struct reset_control *sys_reset;
};

struct qce1204_priv {
	struct qce1204_clk_data clk_data;
#if IS_ENABLED(CONFIG_HWMON)
	struct device *hwmon_dev;
#endif
};

/* Clock type index for each channel */
enum qce1204_clk_type {
	QCE1204_CLK_GMII_TX = 0,
	QCE1204_CLK_GMII_RX,
	QCE1204_CLK_XGMII_TX,
	QCE1204_CLK_XGMII_RX,
	QCE1204_CLK_TYPE_MAX
};

/* Per-channel clocks and resets */
struct qce1204_channel_clk {
	struct clk *clks[QCE1204_CLK_TYPE_MAX];
	struct reset_control *resets[QCE1204_CLK_TYPE_MAX];
};

struct qce1204_shared_clk_data {
	struct qce1204_channel_clk channels[4];
	struct clk *pcs_sys_clk;
	struct clk *ahb_clk;
	struct reset_control *pcs_sys_reset;
	struct reset_control *xpcs_reset;
	/* switch clocks */
	struct reset_control *switch_btq_reset;
	struct reset_control *switch_cfg_reset;
	struct reset_control *switch_core_reset;
	struct reset_control *switch_ipe_reset;
	struct reset_control *switch_mac0_reset;
	struct reset_control *switch_mac1_reset;
	struct reset_control *switch_mac2_reset;
	struct reset_control *switch_mac3_reset;
	struct reset_control *switch_mac4_reset;
	struct reset_control *switch_mac5_reset;
	struct reset_control *xgmac0_ptp_ref_reset;
	struct reset_control *xgmac1_ptp_ref_reset;
	struct reset_control *mac0_tx_reset;
	struct reset_control *mac0_rx_reset;
	struct reset_control *mac1_tx_reset;
	struct reset_control *mac1_rx_reset;
	struct reset_control *mac2_tx_reset;
	struct reset_control *mac2_rx_reset;
	struct reset_control *mac3_tx_reset;
	struct reset_control *mac3_rx_reset;
	struct reset_control *mac4_tx_reset;
	struct reset_control *mac4_rx_reset;
	struct reset_control *mac5_tx_reset;
	struct reset_control *mac5_rx_reset;
};

struct qce1204_shared_priv {
	struct qce1204_shared_clk_data shared_clk_data;
	phy_interface_t package_mode;
#if IS_ENABLED(CONFIG_HWMON)
	u64 tem_base_code;
#endif
};

enum {
	QCE1204_GPIO0_PHY_INT = 0,
	QCE1204_GPIO1_P0_LED_0,
	QCE1204_GPIO2_P1_LED_0,
	QCE1204_GPIO3_P2_LED_0,
	QCE1204_GPIO4_P3_LED_0,
	QCE1204_GPIO9_P0_WOL_INT = 9,
	QCE1204_GPIO15_P1_WOL_INT = 15,
	QCE1204_GPIO16_P2_WOL_INT,
	QCE1204_GPIO17_P3_WOL_INT,
	QCE1204_GPIO_MAX
};

u32 __qce1204_soc_read(struct phy_device *phydev, u32 reg);
void __qce1204_soc_write(struct phy_device *phydev, u32 reg, u32 val);
int __qce1204_soc_modify(struct phy_device *phydev, u32 reg,
	u32 mask, u32 set);
u32 qce1204_soc_read(struct phy_device *phydev, u32 reg);
int qce1204_soc_modify(struct phy_device *phydev, u32 reg,
	u32 mask, u32 set);
int qce1204_phy_channel_get(struct phy_device *phydev);
#if IS_ENABLED(CONFIG_HWMON)
int qce1204_hwmon_hw_init_once(struct phy_device *phydev);
int qce1204_hwmon_hw_init(struct phy_device *phydev);
int qce1204_hwmon_probe(struct phy_device *phydev);
#endif
#endif /* _QCE1204_H_ */
