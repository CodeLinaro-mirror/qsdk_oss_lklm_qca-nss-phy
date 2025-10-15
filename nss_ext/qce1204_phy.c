/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include "nss_phy.h"
#include "qce1204_phy.h"
#include "nss_phy_c45_common.h"

int qce1204_phy_ops_init(struct nss_phy_ops *ops)
{
	ops->hibernation_set = nss_phy_common_hibernation_set;
	ops->hibernation_get = nss_phy_common_hibernation_get;
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
	ops->wol_set = nss_phy_common_wol_set;
	ops->wol_get = nss_phy_common_wol_get;
	ops->magic_frame_set = nss_phy_common_magic_frame_set;
	ops->magic_frame_get = nss_phy_common_magic_frame_get;
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
