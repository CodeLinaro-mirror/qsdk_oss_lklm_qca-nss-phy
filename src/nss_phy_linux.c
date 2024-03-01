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

static int nss_phy_id_get(struct phy_device *phydev, int addr, u32 *phy_id)
{
	int reg1 = 0, reg2 = 0;

	if (phydev->is_c45) {
		reg1 = mdiobus_c45_read(phydev->mdio.bus, addr, MDIO_MMD_AN,
			MII_PHYSID1);
		reg2 = mdiobus_c45_read(phydev->mdio.bus, addr, MDIO_MMD_AN,
			MII_PHYSID2);
	} else {
		reg1 = mdiobus_read(phydev->mdio.bus, addr, MII_PHYSID1);
		reg2 = mdiobus_read(phydev->mdio.bus, addr, MII_PHYSID2);
	}

	if (reg1 < 0 || reg2 < 0)
		return NSS_PHY_EINVAL;

	*phy_id = reg1 << 16 | reg2;

	return 0;
}

int nss_phy_base_addr_get(struct phy_device *phydev)
{
	int ret = 0, times = 4, base_addr = 0;
	int addr = phydev->mdio.addr;
	u32 phy_id = 0;

	if (addr >= PHY_MAX_ADDR || addr < 0)
		return -NSS_PHY_EINVAL;

	while (times--) {
		if (addr < 0)
			break;
		ret = nss_phy_id_get(phydev, addr, &phy_id);
		if (ret < 0)
			return ret;
		if (nss_phydev_id_compare(phydev, phy_id, GENMASK(31, 0)))
			base_addr = addr;
		else
			break;
		addr--;
	}

	return base_addr;
}

static int nss_phy_match_phy_device(struct phy_device *phydev)
{
	struct nss_phy_ops *ops = NULL;

	if (!QCA_PHY_MATCH(nss_phydev_id_get(phydev)))
		return -NSS_PHY_EOPNOTSUPP;

	if (phydev->drv == NULL) {
		phydev_info(phydev, "nss phy driver is used\n");
		return true;
	}
	phydev_info(phydev, "nss phy driver is used as extended driver only\n");
	ops = nss_phy_ops_get();
	if (ops) {
		if (!(phydev->drv->driver_data))
			phydev->drv->driver_data = ops;
		else
			phydev_warn(phydev,
				"driver_data have been init by other driver\n");
		/**
		* if upstream driver did not init the base addr,
		* will init it here
		*/
		if (!phydev->shared) {
			devm_phy_package_join(&phydev->mdio.dev, phydev,
				nss_phy_base_addr_get(phydev), 0);
			phydev_info(phydev, "phydev->shared->addr:0x%x\n",
				phydev->shared->addr);
		}
	} else {
		phydev_warn(phydev, "nss phy driver ops is null\n");
	}

	return false;
}

static int nss_phy_probe(struct phy_device *phydev)
{
	int base_addr = 0;

	if (!phydev->shared) {
		base_addr = nss_phy_base_addr_get(phydev);
		devm_phy_package_join(&phydev->mdio.dev, phydev, base_addr, 0);
		phydev_info(phydev, "phydev->shared->addr:0x%x\n",
			phydev->shared->addr);
	}
	return 0;
}

static int nss_phy_read_abilities(struct phy_device *phydev)
{
	if (phydev->is_c45)
		return genphy_c45_pma_read_abilities(phydev);
	else
		return genphy_read_abilities(phydev);
}

static int nss_phy_read_status(struct phy_device *phydev)
{
	if (phydev->is_c45)
		return genphy_c45_read_status(phydev);
	else
		return genphy_read_status(phydev);
}

static int nss_phy_suspend(struct phy_device *phydev)
{
	if (phydev->is_c45)
		return genphy_c45_pma_suspend(phydev);
	else
		return genphy_suspend(phydev);
}

static int nss_phy_resume(struct phy_device *phydev)
{
	if (phydev->is_c45)
		return genphy_c45_pma_resume(phydev);
	else
		return genphy_resume(phydev);
}

struct phy_driver nss_phy_driver = {
	.name = "nss phy driver",
	.probe = nss_phy_probe,
	.match_phy_device = nss_phy_match_phy_device,
	.get_features = nss_phy_read_abilities,
	.read_status = nss_phy_read_status,
	.suspend = nss_phy_suspend,
	.resume = nss_phy_resume,
};

static int __init nss_phy_module_init(void)
{
	int ret = 0;
	struct nss_phy_ops *ops = NULL;

	ops = nss_phy_ops_get();
	if (!ops)
		pr_warn("nss phy driver ops is null\n");
	else
		nss_phy_driver.driver_data = ops;

	ret = phy_driver_register(&nss_phy_driver, THIS_MODULE);
	if (!ret)
		pr_info("nss phy driver register successfully\n");

	return ret;
}

static void __exit nss_phy_module_exit(void)
{
	phy_driver_unregister(&nss_phy_driver);
}

module_init(nss_phy_module_init);
module_exit(nss_phy_module_exit);
MODULE_DESCRIPTION("NSS PHY driver");
MODULE_LICENSE("Dual BSD/GPL");
