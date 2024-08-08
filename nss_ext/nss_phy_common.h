/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _NSS_PHY_COMMON_H_
#define _NSS_PHY_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */
/*PHY debug registers*/
#define NSS_PHY_DEBUG_PHY_HIBERNATION_CTRL		0xb
#define NSS_PHY_DEBUG_POWER_SAVE		0x29

/*PHY debug registers field*/
#define NSS_PHY_DEBUG_HIBERNATION_EN		0x8000
#define NSS_PHY_DEBUG_POWER_SAVE_EN		0x8000

/*mii register*/
#define NSS_PHY_CONTROL		0
#define NSS_PHY_CHIP_CONFIGURATION		0x1f
#define NSS_PHY_FIFO_CONTROL		0x19

/*mii register field*/
#define NSS_PHY_SOFT_RESET		0x8000
#define NSS_PHY_AUTONEG_RESTART		0x200
#define NSS_PHY_AUTONEG_EN		0x1000
#define NSS_PHY_LOCAL_LOOPBACK_EN		0x4000
#define NSS_PHY_PREFER_FIBER		0x400
#define NSS_PHY_FIBER_MODE_1000BX		0x100
#define NSS_PHY_BT_BX_REG_SEL		0x8000
#define NSS_PHY_FIFO_RESET_MASK		0x3
#define NSS_PHY_POWER_DOWN		0x800
#define NSS_PHY_POWER_UP		0
#define NSS_PHY_FULL_DUPLEX		0x100
#define NSS_PHY_COMMON_CTRL		0x1040
#define NSS_PHY_LOOPBACK_10M		0x4100
#define NSS_PHY_LOOPBACK_100M		0x6100
#define NSS_PHY_LOOPBACK_1000M		0x4140

/*MMD number*/
#define NSS_PHY_MMD1_NUM		0x1
#define NSS_PHY_MMD3_NUM		0x3
#define NSS_PHY_MMD7_NUM		0x7
#define NSS_PHY_MMD31_NUM		0x1f

/*MMD registers*/
#define NSS_PHY_MMD3_8023AZ_EEE_CAPABILITY		0x14
#define NSS_PHY_MMD3_REMOTE_LOOPBACK_CTRL		0x805a
#define NSS_PHY_MMD7_8023AZ_EEE_CTRL		0x3c
#define NSS_PHY_MMD7_8023AZ_EEE_PARTNER		0x3d
#define NSS_PHY_MMD7_8023AZ_EEE_STATUS		0x8000
#define NSS_PHY_MMD7_LED_BLINK_FREQ_CTRL		0x8073
#define NSS_PHY_MMD7_LED_POLARITY_CTRL		0x901a

/*MMD registers field*/
#define NSS_PHY_MMD3_EEE_CAPABILITY_100M		0x0002
#define NSS_PHY_MMD3_EEE_CAPABILITY_1000M		0x0004
#define NSS_PHY_MMD3_EEE_CAPABILITY_10000M		0x0008
#define NSS_PHY_MMD3_REMOTE_LOOPBACK_EN		0x0001
#define NSS_PHY_MMD7_EEE_MASK		0x000e
#define NSS_PHY_MMD7_EEE_ADV_100M		0x0002
#define NSS_PHY_MMD7_EEE_ADV_1000M		0x0004
#define NSS_PHY_MMD7_EEE_ADV_10000M		0x0008
#define NSS_PHY_MMD7_EEE_PARTNER_ADV_100M		0x0002
#define NSS_PHY_MMD7_EEE_PARTNER_ADV_1000M		0x0004
#define NSS_PHY_MMD7_EEE_PARTNER_ADV_10000M		0x0008
#define NSS_PHY_MMD7_EEE_STATUS_100M		0x0002
#define NSS_PHY_MMD7_EEE_STATUS_1000M		0x0004
#define NSS_PHY_MMD7_EEE_STATUS_2500M		0x0008
#define NSS_PHY_MMD7_EEE_STATUS_5000M		0x0010
#define NSS_PHY_MMD7_EEE_STATUS_10000M		0x0020
#define NSS_PHY_MMD7_LINK_1000M_LIGHT_EN		0x40
#define NSS_PHY_MMD7_LINK_100M_LIGHT_EN		0x20
#define NSS_PHY_MMD7_LINK_10M_LIGHT_EN		0x10
#define NSS_PHY_MMD7_RX_TRAFFIC_BLINK_EN		0x200
#define NSS_PHY_MMD7_TX_TRAFFIC_BLINK_EN		0x400
#define NSS_PHY_MMD7_MAP_BLINK_FREQ_MASK		0xe00
#define NSS_PHY_MMD7_ALWAYS_BLINK_FREQ_MASK		0x38
#define NSS_PHY_MMD7_BLINK_FREQ_2HZ		0x0
#define NSS_PHY_MMD7_BLINK_FREQ_4HZ		0x200
#define NSS_PHY_MMD7_BLINK_FREQ_8HZ		0x400
#define NSS_PHY_MMD7_BLINK_FREQ_16HZ		0x600
#define NSS_PHY_MMD7_BLINK_FREQ_32HZ		0x800
#define NSS_PHY_MMD7_BLINK_FREQ_64HZ		0xa00
#define NSS_PHY_MMD7_BLINK_FREQ_128HZ		0xc00
#define NSS_PHY_MMD7_BLINK_FREQ_256HZ		0xe00

#define NSS_PHY_MMD7_LED_FORCE_EN		0x8000
#define NSS_PHY_MMD7_LED_FORCE_MASK		0x6000
#define NSS_PHY_MMD7_LED_FORCE_ALWAYS_OFF		0
#define NSS_PHY_MMD7_LED_FORCE_ALWAYS_ON		0x2000
#define NSS_PHY_MMD7_LED_FORCE_ALWAYS_BLINK		0x6000
#define NSS_PHY_MMD7_LED_POLARITY_MASK		0x40

#define NSS_PHY_LED_SOURCE0		0x0
#define NSS_PHY_LED_SOURCE1		0x1
#define NSS_PHY_LED_SOURCE2		0x2

int nss_phy_common_hibernation_set(struct nss_phy_device *nss_phydev,
	u32 enable);
int nss_phy_common_hibernation_get(struct nss_phy_device *nss_phydev,
	u32 *enable);
int nss_phy_common_powersave_set(struct nss_phy_device *nss_phydev,
	bool enable);
int nss_phy_common_powersave_get(struct nss_phy_device *nss_phydev,
	bool *enable);
int nss_phy_common_combo_prefer_medium_get(struct nss_phy_device *nss_phydev,
	enum nss_phy_medium *phy_medium);
int nss_phy_common_combo_prefer_medium_set(struct nss_phy_device *nss_phydev,
	enum nss_phy_medium phy_medium);
int nss_phy_common_eee_adv_set(struct nss_phy_device *nss_phydev, u32 adv);
int nss_phy_common_eee_adv_get(struct nss_phy_device *nss_phydev, u32 *adv);
int nss_phy_common_eee_partner_adv_get(struct nss_phy_device *nss_phydev,
	u32 *adv);
int nss_phy_common_eee_cap_get(struct nss_phy_device *nss_phydev, u32 *cap);
int nss_phy_common_eee_status_get(struct nss_phy_device *nss_phydev,
	u32 *status);
int nss_phy_common_ge_eee_adv_set(struct nss_phy_device *nss_phydev,
	u32 adv);
int nss_phy_common_ge_eee_adv_get(struct nss_phy_device *nss_phydev,
	u32 *adv);
int nss_phy_common_ge_eee_partner_adv_get(struct nss_phy_device *nss_phydev,
	u32 *adv);
int nss_phy_common_ge_eee_cap_get(struct nss_phy_device *nss_phydev,
	u32 *cap);
int nss_phy_common_ge_eee_status_get(struct nss_phy_device *nss_phydev,
	u32 *status);
int nss_phy_common_ge_8023az_set(struct nss_phy_device *nss_phydev,
	bool enable);
int nss_phy_common_ge_8023az_get(struct nss_phy_device *nss_phydev,
	bool *enable);
int nss_phy_common_reg_pages_sel(struct nss_phy_device *nss_phydev,
	enum nss_phy_reg_pages phy_reg_pages);
int nss_phy_common_local_loopback_set(struct nss_phy_device *nss_phydev,
	bool enable);
int nss_phy_common_local_loopback_get(struct nss_phy_device *nss_phydev,
	bool *enable);
int nss_phy_common_remote_loopback_set(struct nss_phy_device *nss_phydev,
	bool enable);
int nss_phy_common_remote_loopback_get(struct nss_phy_device *nss_phydev,
	bool *enable);
int nss_phy_common_combo_fiber_mode_set(struct nss_phy_device *nss_phydev,
	enum nss_phy_fiber_mode fiber_mode);
int nss_phy_common_combo_fiber_mode_get(struct nss_phy_device *nss_phydev,
	enum nss_phy_fiber_mode *fiber_mode);
int nss_phy_common_led_from_phy(struct nss_phy_device *nss_phydev,
	u32 *status_bmap, u16 phy_data);
int nss_phy_common_led_to_phy(struct nss_phy_device *nss_phydev,
	u32 status_bmap, u16 *phy_data);
int nss_phy_common_led_force_from_phy(struct nss_phy_device *nss_phydev,
	u32 *force_mode, u16 phy_data);
int nss_phy_common_led_force_to_phy(struct nss_phy_device *nss_phydev,
	u32 force_mode, u16 *phy_data);
int nss_phy_common_led_active_set(struct nss_phy_device *nss_phydev,
	u32 active_level);
int nss_phy_common_led_active_get(struct nss_phy_device *nss_phydev,
	u32 *active_level);
int nss_phy_common_led_blink_freq_set(struct nss_phy_device *nss_phydev,
	u32 mode, u32 freq);
int nss_phy_common_led_blink_freq_get(struct nss_phy_device *nss_phydev,
	u32 mode, u32 *freq);
int nss_phy_common_fifo_reset(struct nss_phy_device *nss_phydev, bool enable);
int nss_phy_common_soft_reset(struct nss_phy_device *nss_phydev);
int nss_phy_common_function_reset(struct nss_phy_device *nss_phydev,
	enum nss_phy_reset reset_type);
int nss_phy_common_autoneg_restart(struct nss_phy_device *nss_phydev);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _NSS_PHY_COMMON_H_ */
