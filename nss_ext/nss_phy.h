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

#ifndef _NSS_PHY_H_
#define _NSS_PHY_H_

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */
#include "nss_phy_lib.h"

#define QCA_PHY_ID		0x004dd000
#define QCA_PHY_MASK		0xfffff000
#define QCA_PHY_MATCH(phy_id)		((phy_id & QCA_PHY_MASK) == QCA_PHY_ID)
#define QCA8075_PHY		0x004dd0b1
#define QCA8072_PHY		0x004dd0b2
#define QCA807X_MASK		0xfffffff0

#define NSS_BIT(_n)		(1UL << (_n))
#define EEE_100BASE_T		0x2
#define EEE_1000BASE_T		0x4
#define EEE_2500BASE_T		0x8
#define EEE_5000BASE_T		0x10
#define EEE_10000BASE_T		0x20
#define GE_EEE		(EEE_100BASE_T | EEE_1000BASE_T)
#define XGE_EEE		(EEE_2500BASE_T | EEE_5000BASE_T | EEE_10000BASE_T)
#define ALL_SPEED_EEE		(GE_EEE | XGE_EEE)

#define LED_SOURCE_MAX		0x3
#define LED_FULL_DUPLEX_LIGHT_EN		0
#define LED_HALF_DUPLEX_LIGHT_EN		1
#define LED_POWER_ON_LIGHT_EN		2
#define LED_LINK_1000M_LIGHT_EN		3
#define LED_LINK_100M_LIGHT_EN		4
#define LED_LINK_10M_LIGHT_EN		5
#define LED_COLLISION_BLINK_EN		6
#define LED_RX_TRAFFIC_BLINK_EN		7
#define LED_TX_TRAFFIC_BLINK_EN		8
#define LED_LINK_2500M_LIGHT_EN		9
#define LED_TRAFFIC_EN		\
	(NSS_BIT(LED_RX_TRAFFIC_BLINK_EN) | NSS_BIT(LED_TX_TRAFFIC_BLINK_EN))
#define LED_TRAFFIC_COLLISION_EN		\
	(LED_TRAFFIC_EN | NSS_BIT(LED_COLLISION_BLINK_EN))
#define LED_ACTIVE_HIGH		1
#define LED_ACTIVE_LOW		0
#define LED_MAP_10M		\
	(NSS_BIT(LED_LINK_10M_LIGHT_EN) | LED_TRAFFIC_COLLISION_EN)
#define LED_MAP_100M		\
	(NSS_BIT(LED_LINK_10M_LIGHT_EN) | LED_TRAFFIC_COLLISION_EN)
#define LED_MAP_1000M		\
	(NSS_BIT(LED_LINK_1000M_LIGHT_EN) | LED_TRAFFIC_EN)
#define LED_MAP_2500M		\
	(NSS_BIT(LED_LINK_2500M_LIGHT_EN) | LED_TRAFFIC_EN)
#define LED_MAP_ALL		\
	(LED_MAP_10M | LED_MAP_100M | LED_MAP_1000M | LED_MAP_2500M)

enum nss_phy_reset {
	FIFO_RESET = 0,
	SERDES_RESET,
	SOFT_RESET,
};

enum nss_phy_medium {
	MEDIUM_COPPER = 0,
	MEDIUM_FIBER,
};

enum nss_phy_reg_pages {
	PAGE_FIBER = 0,
	PAGE_COPPER,
};

enum nss_phy_fiber_mode {
	FIBER_100FX = 0,
	FIBER_1000BX,
};

enum nss_phy_led_pattern {
	ALWAYS_OFF = 0,
	ALWAYS_BLINK,
	ALWAYS_ON,
	ACT_PHY_STATUS,
};

enum nss_phy_led_blink_freq {
	BLINK_2HZ = 0,
	BLINK_4HZ,
	BLINK_8HZ,
	BLINK_16HZ,
	BLINK_32HZ,
	BLINK_64HZ,
	BLINK_128HZ,
	BLINK_256HZ,
};

struct nss_phy_led_pattern_ctrl {
	enum nss_phy_led_pattern mode;
	u32 phy_status_bmap;
	enum nss_phy_led_blink_freq freq;
	u32 active_level;
};

struct nss_phy_ops {
	int (*hibernation_set)(struct nss_phy_device *nss_phydev, bool enable);
	int (*hibernation_get)(struct nss_phy_device *nss_phydev, bool *enable);
	int (*powersave_set)(struct nss_phy_device *nss_phydev, bool enable);
	int (*powersave_get)(struct nss_phy_device *nss_phydev, bool *enable);
	int (*function_reset)(struct nss_phy_device *nss_phydev,
		enum nss_phy_reset reset_type);
	int (*interface_set)(struct nss_phy_device *nss_phydev,
		nss_phy_interface_t interface);
	int (*interface_get)(struct nss_phy_device *nss_phydev,
		 nss_phy_interface_t *interface);
	int (*eee_adv_set)(struct nss_phy_device *nss_phydev, u32 adv);
	int (*eee_adv_get)(struct nss_phy_device *nss_phydev, u32 *adv);
	int (*eee_partner_adv_get)(struct nss_phy_device *nss_phydev, u32 *adv);
	int (*eee_cap_get)(struct nss_phy_device *nss_phydev, u32 *cap);
	int (*eee_status_get)(struct nss_phy_device *nss_phydev, u32 *status);
	int (*ieee_8023az_set)(struct nss_phy_device *nss_phydev, bool enable);
	int (*ieee_8023az_get)(struct nss_phy_device *nss_phydev, bool *enable);
	int (*local_loopback_set)(struct nss_phy_device *nss_phydev,
		bool enable);
	int (*local_loopback_get)(struct nss_phy_device *nss_phydev,
		bool *enable);
	int (*remote_loopback_set)(struct nss_phy_device *nss_phydev,
		bool enable);
	int (*remote_loopback_get)(struct nss_phy_device *nss_phydev,
		bool *enable);
	int (*combo_prefer_medium_set)(struct nss_phy_device *nss_phydev,
		enum nss_phy_medium phy_medium);
	int (*combo_prefer_medium_get)(struct nss_phy_device *nss_phydev,
		enum nss_phy_medium *phy_medium);
	int (*combo_medium_status_get)(struct nss_phy_device *nss_phydev,
		enum nss_phy_medium *phy_medium);
	int (*combo_fiber_mode_set)(struct nss_phy_device *nss_phydev,
		enum nss_phy_fiber_mode fiber_mode);
	int (*combo_fiber_mode_get)(struct nss_phy_device *nss_phydev,
		enum nss_phy_fiber_mode *fiber_mode);
	int (*led_ctrl_source_set)(struct nss_phy_device *nss_phydev,
		u32 source_id, struct nss_phy_led_pattern_ctrl *pattern);
	int (*led_ctrl_source_get)(struct nss_phy_device *nss_phydev,
		u32 source_id, struct nss_phy_led_pattern_ctrl *pattern);
};

struct nss_phy_ops *qca807x_phy_ops_get(void);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _NSS_PHY_H_ */
