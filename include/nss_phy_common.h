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
/*mii register*/
#define NSS_PHY_CONTROL		0

/*mii register field*/
#define NSS_PHY_SOFT_RESET		0x8000

/*PHY debug registers*/
#define NSS_PHY_DEBUG_PHY_HIBERNATION_CTRL		0xb
#define NSS_PHY_DEBUG_POWER_SAVE		0x29

/*PHY debug registers field*/
#define NSS_PHY_DEBUG_HIBERNATION_EN		0x8000
#define NSS_PHY_DEBUG_POWER_SAVE_EN		0x8000

/*MMD number*/
#define NSS_PHY_MMD1_NUM		0x1
#define NSS_PHY_MMD3_NUM		0x3
#define NSS_PHY_MMD7_NUM		0x7
#define NSS_PHY_MMD31_NUM		0x1f

int nss_phy_software_reset(struct nss_phy_device *nss_phydev);
int nss_phy_common_hibernation_set(struct nss_phy_device *nss_phydev,
	bool enable);
int nss_phy_common_hibernation_get(struct nss_phy_device *nss_phydev,
	bool *enable);
int nss_phy_common_powersave_set(struct nss_phy_device *nss_phydev,
	bool enable);
int nss_phy_common_powersave_get(struct nss_phy_device *nss_phydev,
	bool *enable);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _NSS_PHY_COMMON_H_ */
