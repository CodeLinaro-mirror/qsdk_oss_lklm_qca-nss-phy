/*
* Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
* SPDX-License-Identifier: ISC
*/

#include "qce1204.h"

static int qce1204_phy_probe(struct phy_device *phydev)
{
	return 0;
}

static int qce1204_phy_config_init(struct phy_device *phydev)
{
	return 0;
}

static struct phy_driver qce1204_phy_driver[] = {
{
	PHY_ID_MATCH_EXACT(QCE1204_PHY),
	.name = "Qualcomm QCE1204",
	.flags = PHY_POLL_CABLE_TEST,
	.probe = qce1204_phy_probe,
	.config_init = qce1204_phy_config_init,
	.suspend = genphy_c45_pma_suspend,
	.resume = genphy_c45_pma_resume,
},
};

module_phy_driver(qce1204_phy_driver);
MODULE_DESCRIPTION("QCE1204 PHY Driver");
MODULE_LICENSE("Dual BSD/GPL");
