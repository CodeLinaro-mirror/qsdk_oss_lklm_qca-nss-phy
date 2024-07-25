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
#include "qca807x_phy.h"

bool nss_phy_is_qcaphy(struct nss_phy_device *nss_phydev)
{
	return nss_phy_id_check(nss_phydev, QCA_PHY_ID, QCA_PHY_MASK);
}

static int nss_phy_powersave_set(struct nss_phy_device *nss_phydev,
	bool enable)
{
	if (nss_phy_id_check(nss_phydev, QCA8075_PHY, QCA807X_MASK))
		return qca807x_phy_powersave_set(nss_phydev, enable);

	return -NSS_PHY_EOPNOTSUPP;
}

static int nss_phy_powersave_get(struct nss_phy_device *nss_phydev,
	bool *enable)
{
	if (nss_phy_id_check(nss_phydev, QCA8075_PHY, QCA807X_MASK))
		return qca807x_phy_powersave_get(nss_phydev, enable);

	return -NSS_PHY_EOPNOTSUPP;
}

struct nss_phy_ops g_nss_phy_ops = {
	.hibernation_set = nss_phy_common_hibernation_set,
	.hibernation_get = nss_phy_common_hibernation_get,
	.powersave_set = nss_phy_powersave_set,
	.powersave_get = nss_phy_powersave_get,
};

struct nss_phy_ops *nss_phy_ops_get(void)
{
	return &g_nss_phy_ops;
}
