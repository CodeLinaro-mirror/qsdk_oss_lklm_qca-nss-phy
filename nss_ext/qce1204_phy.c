/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include "nss_phy.h"
#include "qce1204_phy.h"
#include "nss_phy_c45_common.h"

static bool qce1024_phy_autoneg_check(struct nss_phy_device *nss_phydev)
{
	int  times = 500, ret = 0;

	ret = nss_phy_c45_common_autoneg_restart(nss_phydev);
	if (ret < 0)
		return false;
	while(times--) {
		ret = nss_phy_read_mmd(nss_phydev, NSS_PHY_MMD7_NUM,
			NSS_PHY_MMD7_AUTONEG_STATUS);
		if (ret < 0)
			return false;
		if ((ret & NSS_PHY_MMD7_AUTONEG_STATUS_MASK) >= NSS_PHY_MMD7_AUTONEG_ACK_DETECT)
			return true;
		nss_phy_mdelay(10);
	}
	return false;
}

static int qce1204_phy_cdt(struct nss_phy_device *nss_phydev, u32 mdi_pair,
	enum nss_phy_cable_status *status, u32 *cable_len)
{
	int ret, cdt_with_auotneg_en = 0;

	if (nss_phydev_link_get(nss_phydev) == NSS_PHY_FALSE) {
		/* run cdt between two autoneg pulse if link partner send autoneg pulse */
		if (qce1024_phy_autoneg_check(nss_phydev))
			cdt_with_auotneg_en = QCE1204_PHY_MMD7_CDT_WITH_AUTONEG;

		ret = nss_phy_modify_mmd(nss_phydev, NSS_PHY_MMD31_NUM,
			NSS_PHY_CDT_CONTROL, QCE1204_PHY_MMD7_CDT_WITH_AUTONEG,
			cdt_with_auotneg_en);
		if (ret < 0)
			return ret;
		ret = nss_phy_c45_common_cdt_start(nss_phydev);
		if (ret < 0)
			return ret;
	}

	/* if PHY link up, the result can be read without CDT */
	ret = nss_phy_common_cdt_status_get(nss_phydev, mdi_pair, status,
		cable_len);

	return ret;
}

int qce1204_phy_ops_init(struct nss_phy_ops *ops)
{
	ops->hibernation_set = nss_phy_common_hibernation_set;
	ops->hibernation_get = nss_phy_common_hibernation_get;
	ops->function_reset = nss_phy_c45_function_reset;
	ops->eee_adv_set = nss_phy_c45_common_eee_adv_set;
	ops->eee_adv_get = nss_phy_c45_common_eee_adv_get;
	ops->eee_partner_adv_get = nss_phy_c45_common_eee_partner_adv_get;
	ops->eee_cap_get = nss_phy_c45_common_eee_cap_get;
	ops->eee_status_get = nss_phy_c45_common_eee_status_get;
	ops->ieee_8023az_set = nss_phy_c45_common_8023az_set;
	ops->ieee_8023az_get = nss_phy_c45_common_8023az_get;
	ops->local_loopback_set = nss_phy_c45_common_pma_local_loopback_set;
	ops->local_loopback_get = nss_phy_c45_common_pma_local_loopback_get;
	ops->remote_loopback_set = nss_phy_common_remote_loopback_set;
	ops->remote_loopback_get = nss_phy_common_remote_loopback_get;
	ops->cdt = qce1204_phy_cdt;
	ops->wol_set = nss_phy_common_wol_set;
	ops->wol_get = nss_phy_common_wol_get;
	ops->magic_frame_set = nss_phy_common_magic_frame_set;
	ops->magic_frame_get = nss_phy_common_magic_frame_get;
	ops->mdix_set = nss_phy_c45_common_mdix_set;
	ops->mdix_get = nss_phy_c45_common_mdix_get;
	ops->mdix_status_get = nss_phy_c45_common_mdix_status_get;
	ops->stats_status_set = nss_phy_common_stats_status_set;
	ops->stats_status_get = nss_phy_common_stats_status_get;
	ops->stats_get = nss_phy_common_stats_get;
	ops->intr_mask_set = nss_phy_c45_common_intr_mask_set;
	ops->intr_mask_get = nss_phy_c45_common_intr_mask_get;
	ops->intr_status_get = nss_phy_c45_common_intr_status_get;
	ops->led_ctrl_source_set = nss_phy_2500m_led_ctrl_source_set;
	ops->led_ctrl_source_get = nss_phy_2500m_led_ctrl_source_get;

	return 0;
}
