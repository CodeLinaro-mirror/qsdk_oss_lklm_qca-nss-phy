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
#include "qca807x_phy.h"
#include "nss_phy_common.h"

int qca807x_phy_powersave_set(struct nss_phy_device *nss_phydev,
	bool enable)
{
	int ret = 0;
	u16 dac_qt_bias_en = 0, vd_half_bias = 0, afe_tx_am = 0;

	if (!enable) {
		dac_qt_bias_en = QCA807X_PHY_MMD3_DAC_AT_BIAS_EN;
		vd_half_bias = QCA807X_PHY_MMD3_VD_HALF_BIAS_EN;
		afe_tx_am = QCA807X_PHY_MMD3_AFE_TX_FULL_AMPLITUDE_EN;
	}
	ret = nss_phy_modify_mmd(nss_phydev, NSS_PHY_MMD3_NUM,
		QCA807X_PHY_MMD3_ADDR_8023AZ_TIMER_CTRL,
		QCA807X_PHY_MMD3_DAC_AT_BIAS_EN, dac_qt_bias_en);
	if (ret < 0)
		return ret;
	ret = nss_phy_modify_mmd(nss_phydev, NSS_PHY_MMD3_NUM,
		QCA807X_PHY_MMD3_ADDR_CLD_CTRL5,
		QCA807X_PHY_MMD3_VD_HALF_BIAS_EN, vd_half_bias);
	if (ret < 0)
		return ret;
	ret = nss_phy_modify_mmd(nss_phydev, NSS_PHY_MMD3_NUM,
		QCA807X_PHY_MMD3_ADDR_CLD_CTRL3,
		QCA807X_PHY_MMD3_AFE_TX_FULL_AMPLITUDE_EN,
		afe_tx_am);
	if (ret < 0)
		return ret;

	return nss_phy_software_reset(nss_phydev);
}

int qca807x_phy_powersave_get(struct nss_phy_device *nss_phydev,
	bool *enable)
{
	u16 phy_data = 0, phy_data1 = 0;

	phy_data = nss_phy_read_mmd(nss_phydev, NSS_PHY_MMD3_NUM,
		QCA807X_PHY_MMD3_ADDR_CLD_CTRL5);
	phy_data1 = nss_phy_read_mmd(nss_phydev, NSS_PHY_MMD3_NUM,
		QCA807X_PHY_MMD3_ADDR_CLD_CTRL3);
	if (!(phy_data & QCA807X_PHY_MMD3_VD_HALF_BIAS_EN) &&
		!(phy_data1 & QCA807X_PHY_MMD3_AFE_TX_FULL_AMPLITUDE_EN))
		*enable = true;
	else
		*enable = false;

	return 0;

}
