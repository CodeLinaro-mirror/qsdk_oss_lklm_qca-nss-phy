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

#ifndef _NSS_PHY_C45_COMMON_H_
#define _NSS_PHY_C45_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */
#include "nss_phy_common.h"

/*MMD registers*/
#define NSS_PHY_MMD1_PMA_CONTROL		0x0
#define NSS_PHY_MMD1_PMA_TTYPE		0x7
#define NSS_PHY_MMD3_8023AZ_EEE_CAPABILITY1		0x15
#define NSS_PHY_MMD7_AN_CONTROL		0x0
#define NSS_PHY_MMD7_8023AZ_EEE_CTRL1		0x3e
#define NSS_PHY_MMD7_8023AZ_EEE_PARTNER1		0x3f

/*MMD registers field*/
#define NSS_PHY_MMD1_PMA_SPEED_MASK		0x207c
#define NSS_PHY_MMD1_PMA_CONTROL_10000M		0x2040
#define NSS_PHY_MMD1_PMA_CONTROL_5000M		0x205c
#define NSS_PHY_MMD1_PMA_CONTROL_2500M		0x2058
#define NSS_PHY_MMD1_PMA_CONTROL_1000M		0x40
#define NSS_PHY_MMD1_PMA_CONTROL_100M		0x2000
#define NSS_PHY_MMD1_PMA_CONTROL_10M		0x0
#define NSS_PHY_MMD1_PMA_TYPE_MASK		0x3f
#define NSS_PHY_MMD1_PMA_TYPE_10000M		0x9
#define NSS_PHY_MMD1_PMA_TYPE_5000M		0x31
#define NSS_PHY_MMD1_PMA_TYPE_2500M		0x30
#define NSS_PHY_MMD1_PMA_TYPE_1000M		0xc
#define NSS_PHY_MMD1_PMA_TYPE_100M		0xe
#define NSS_PHY_MMD1_PMA_TYPE_10M		0xf

#define NSS_PHY_MMD3_EEE_CAPABILITY_2500M		0x0001
#define NSS_PHY_MMD3_EEE_CAPABILITY_5000M		0x0002

#define NSS_PHY_MMD7_EEE_MASK1		0x0003
#define NSS_PHY_MMD7_EEE_ADV_2500M		0x0001
#define NSS_PHY_MMD7_EEE_ADV_5000M		0x0002
#define NSS_PHY_MMD7_EEE_PARTNER_ADV_2500M		0x0001
#define NSS_PHY_MMD7_EEE_PARTNER_ADV_5000M		0x0002

int nss_phy_c45_common_eee_adv_set(struct nss_phy_device *nss_phydev,
	u32 adv);
int nss_phy_c45_common_eee_adv_get(struct nss_phy_device *nss_phydev,
	u32 *adv);
int nss_phy_c45_common_eee_partner_adv_get(struct nss_phy_device *nss_phydev,
	u32 *adv);
int nss_phy_c45_common_eee_cap_get(struct nss_phy_device *nss_phydev,
	u32 *cap);
int nss_phy_c45_common_8023az_set(struct nss_phy_device *nss_phydev,
	bool enable);
int nss_phy_c45_common_8023az_get(struct nss_phy_device *nss_phydev,
	bool *enable);
int nss_phy_c45_common_autoneg_set(struct nss_phy_device *nss_phydev,
	bool enable);
int nss_phy_c45_common_force_speed_set(struct nss_phy_device *nss_phydev);
int
nss_phy_c45_common_local_loopback_set(struct nss_phy_device *nss_phydev,
	bool enable);
int nss_phy_c45_common_local_loopback_get(struct nss_phy_device *nss_phydev,
	bool *enable);
int nss_phy_c45_common_fifo_reset(struct nss_phy_device *nss_phydev,
	bool enable);
int nss_phy_c45_common_autoneg_restart(struct nss_phy_device *nss_phydev);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _NSS_PHY_C45_COMMON_H_ */
