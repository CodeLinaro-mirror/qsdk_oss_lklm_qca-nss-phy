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

#include "nss_phy.h"
#include "nss_phy_common.h"

int nss_phy_software_reset(struct nss_phy_device *nss_phydev)
{
	return nss_phy_modify(nss_phydev, NSS_PHY_CONTROL,
		NSS_PHY_SOFT_RESET,
		NSS_PHY_SOFT_RESET);
}

int nss_phy_common_hibernation_set(struct nss_phy_device *nss_phydev,
	bool enable)
{
	return nss_phy_modify_debug(nss_phydev,
		NSS_PHY_DEBUG_PHY_HIBERNATION_CTRL,
		NSS_PHY_DEBUG_HIBERNATION_EN,
		enable ? NSS_PHY_DEBUG_HIBERNATION_EN : 0);
}

int nss_phy_common_hibernation_get(struct nss_phy_device *nss_phydev,
	bool *enable)
{
	int ret;

	ret = nss_phy_read_debug(nss_phydev,
		NSS_PHY_DEBUG_PHY_HIBERNATION_CTRL);
	if (ret < 0)
		return ret;

	if (ret & NSS_PHY_DEBUG_HIBERNATION_EN)
		*enable = true;
	else
		*enable = false;

	return 0;
}

int nss_phy_common_powersave_set(struct nss_phy_device *nss_phydev,
	bool enable)
{
	return nss_phy_modify_debug(nss_phydev, NSS_PHY_DEBUG_POWER_SAVE,
		NSS_PHY_DEBUG_POWER_SAVE_EN,
		enable ? NSS_PHY_DEBUG_POWER_SAVE_EN : 0);
}

int nss_phy_common_powersave_get(struct nss_phy_device *nss_phydev,
	bool *enable)
{
	int ret;

	ret = nss_phy_read_debug(nss_phydev, NSS_PHY_DEBUG_POWER_SAVE);
	if (ret < 0)
		return ret;

	if (ret & NSS_PHY_DEBUG_POWER_SAVE_EN)
		*enable = true;
	else
		*enable = false;

	return 0;
}
