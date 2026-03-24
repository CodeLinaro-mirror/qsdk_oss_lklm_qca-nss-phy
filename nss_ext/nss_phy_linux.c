/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include "nss_phy.h"
#if defined(NSS_PHY_PTP)
#include "nss_phy_ptp.h"
#endif
#if defined(CONFIG_NSSPHY_QCA807X)
#include "qca807x_phy.h"
#endif
#if defined(CONFIG_NSSPHY_QCA81XX)
#include "qca81xx_phy.h"
#endif
#if defined(CONFIG_NSSPHY_QCA808X)
#include "qca808x_phy.h"
#endif
#if defined(CONFIG_NSSPHY_QCA803X)
#include "qca803x_phy.h"
#endif
#if defined(CONFIG_NSSPHY_QCA833X)
#include "qca833x_phy.h"
#endif
#if defined(CONFIG_NSSPHY_QCE1204) || defined(CONFIG_NSSPHY_IPQ52XX)
#include "qce1204_phy.h"
#endif
#include <linux/of_device.h>
#include <linux/of_mdio.h>
#if IS_ENABLED(CONFIG_MDIO_I2C)
#include <linux/mdio/mdio-i2c.h>
#include <linux/i2c.h>
#endif
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include "nss_phy_linux_wrapper.h"

static struct nss_phy_global_manager g_nss_phy_manager = {0};

/*
 * nss_phy_ext_state_show()
 *	Sysfs callback to display extended PHY state.
 */
static ssize_t nss_phy_ext_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct phy_device *phydev = to_phy_device(dev);
	struct nss_phy_device *nss_phydev = dev_get_drvdata(&phydev->mdio.dev);
	ssize_t count = 0;
	const char *status_str;

	if (atomic_read(&nss_phydev->pcs_state))
		status_str = "enabled";
	else
		status_str = "disabled";

	count += scnprintf(buf + count, PAGE_SIZE - count, "NSS PHY Extended State\n");
	count += scnprintf(buf + count, PAGE_SIZE - count, "    %-20s : %s\n", "pcs_state", status_str);

	return count;
}

/*
 * nss_phy_ext_statistics_show()
 *	Sysfs callback to display extended PHY statistics.
 */
static ssize_t nss_phy_ext_statistics_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct phy_device *phydev = to_phy_device(dev);
	struct nss_phy_device *nss_phydev = dev_get_drvdata(&phydev->mdio.dev);
	ssize_t ret_count = 0;
	struct nss_phy_ops *ops = (struct nss_phy_ops *)phydev->drv->driver_data;

	ret_count += scnprintf(buf + ret_count, PAGE_SIZE - ret_count, "NSS PHY Extended Statistics\n");
	if (ops && ops->adjust_link_post) {
		ret_count += scnprintf(buf + ret_count, PAGE_SIZE - ret_count, "    %-20s : %llu\n", "adjust_link_post_count", atomic64_read(&nss_phydev->adjust_link_post_count));
	}

	return ret_count;
}

/*
 * nss_phy_ext_statistics_reset()
 *	Sysfs callback to clear extended PHY statistics.
 */
static ssize_t nss_phy_ext_statistics_reset(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct phy_device *phydev = to_phy_device(dev);
	struct nss_phy_device *nss_phydev = dev_get_drvdata(&phydev->mdio.dev);

	if (buf[0] == '0' || buf[0] == '\n') { /* Any write with '0' or newline clears statistics */
		atomic64_set(&nss_phydev->adjust_link_post_count, 0);
	}

	return count;
}

/*
 * nss_phy_global_state_show()
 *	Debugfs callback to display global state.
 */
static int nss_phy_global_state_show(struct seq_file *s, void __attribute__((unused))*data)
{
	char *state_str;

	switch (g_nss_phy_manager.init_state) {
	case NSS_PHY_INIT_START:
		state_str = "NSS PHY INIT START : Success";
		break;
	case NSS_PHY_INIT_PLATFORM_DRIVER_REGISTER_FAILURE:
		state_str = "NSS PHY INIT PLATFORM DRIVER REGISTER : Failure";
		break;
	case NSS_PHY_INIT_PHY_DRIVER_REGISTER_FAILURE:
		state_str = "NSS PHY INIT PHY DRIVER REGISTER : Failure";
		break;
	case NSS_PHY_INIT_SUCCESS:
		state_str = "NSS PHY INIT : Success";
		break;
	default:
		state_str = "NSS PHY INIT : Invalid";
		break;
	}

	seq_printf(s, "%s\n", state_str);

	return 0;
}

/*
 * nss_phy_global_statistics_show()
 *	Debugfs callback to display global statistics.
 */
static int nss_phy_global_statistics_show(struct seq_file *s, void __attribute__((unused))*data)
{
	seq_printf(s, "NSS PHY Global Statistics\n");
	seq_printf(s, "    %-20s : %u\n", "qca807x_num", atomic_read(&g_nss_phy_manager.debug_stats.qca807x_num));
	seq_printf(s, "    %-20s : %u\n", "qca81xx_num", atomic_read(&g_nss_phy_manager.debug_stats.qca81xx_num));
	seq_printf(s, "    %-20s : %u\n", "qca808x_num", atomic_read(&g_nss_phy_manager.debug_stats.qca808x_num));
	seq_printf(s, "    %-20s : %u\n", "qca803x_num", atomic_read(&g_nss_phy_manager.debug_stats.qca803x_num));
	seq_printf(s, "    %-20s : %u\n", "qca833x_num", atomic_read(&g_nss_phy_manager.debug_stats.qca833x_num));
	seq_printf(s, "    %-20s : %u\n", "unknown_phy_num", atomic_read(&g_nss_phy_manager.debug_stats.unknown_phy_num));
	seq_printf(s, "    %-20s : %llu\n", "mdio_i2c_bus_num", atomic64_read(&g_nss_phy_manager.debug_stats.mdio_i2c_bus_num));
	seq_printf(s, "    %-20s : %llu\n", "sfp_devices_num", atomic64_read(&g_nss_phy_manager.debug_stats.sfp_devices_num));

	return 0;
}

DEFINE_SHOW_ATTRIBUTE(nss_phy_global_state);
DEFINE_SHOW_ATTRIBUTE(nss_phy_global_statistics);

static DEVICE_ATTR(ext_module_state, 0444, nss_phy_ext_state_show, NULL);
static DEVICE_ATTR(ext_module_statistics, 0644, nss_phy_ext_statistics_show, nss_phy_ext_statistics_reset);

static void nss_phy_debugfs_init(struct phy_device *phydev)
{
	device_create_file(&phydev->mdio.dev, &dev_attr_ext_module_state);
	device_create_file(&phydev->mdio.dev, &dev_attr_ext_module_statistics);
}

static void nss_phy_debugfs_exit(struct phy_device *phydev)
{
	device_remove_file(&phydev->mdio.dev, &dev_attr_ext_module_state);
	device_remove_file(&phydev->mdio.dev, &dev_attr_ext_module_statistics);
}

/*
 * nss_phy_debugfs_create()
 *	Create debugfs entries for NSS PHY module.
 */
static void nss_phy_debugfs_create(void)
{
	struct nss_phy_global_manager *mgr = &g_nss_phy_manager;
	struct dentry *nss_phy_debugfs_dir;

	nss_phy_debugfs_dir = debugfs_create_dir("nss_phy", NULL);
	if (!nss_phy_debugfs_dir) {
		nss_phy_pr_info("Failed to create debugfs directory\n");
		return;
	}

	debugfs_create_file("global_state", 0444, nss_phy_debugfs_dir, NULL, &nss_phy_global_state_fops);
	debugfs_create_file("global_statistics", 0444, nss_phy_debugfs_dir, NULL, &nss_phy_global_statistics_fops);

	mgr->debugfs_root = nss_phy_debugfs_dir;
}

/*
 * nss_phy_debugfs_remove()
 *	Remove debugfs entries for NSS PHY module.
 */
static void nss_phy_debugfs_remove(void)
{
	struct nss_phy_global_manager *mgr = &g_nss_phy_manager;

	debugfs_remove_recursive(mgr->debugfs_root);
	mgr->debugfs_root = NULL;
}


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
		return -NSS_PHY_EINVAL;

	*phy_id = reg1 << 16 | reg2;

	return 0;
}

static int nss_phy_base_addr_get(struct phy_device *phydev)
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
		if (nss_phydev_id_compare(phydev, phy_id, QCA_PHY_EXACT_MASK))
			base_addr = addr;
		else
			break;
		addr--;
	}

	return base_addr;
}

static int nss_phy_base_addr_init(struct phy_device *phydev)
{
	int base_addr = 0;

	base_addr = nss_phy_base_addr_get(phydev);

	if (!phydev->shared)
		devm_phy_package_join(&phydev->mdio.dev, phydev,
			base_addr, 0);

	return 0;
}

#if defined(NSS_PHY_PTP)
static int nss_phy_ptp_ops_add(struct phy_device *phydev, struct nss_phy_ops *phy_ops)
{
	struct nss_phy_ptp_ops *ptp_ops;
	int ret;

	if (!phy_ops)
		return NSS_PHY_EINVAL;

	if (!(nss_phydev_id_compare(phydev, QCA8111_PHY, GENMASK(31, 0)) ||
	    nss_phydev_id_compare(phydev, QCA8084_PHY, GENMASK(31, 0)) ||
	    nss_phydev_id_compare(phydev, QCE1204_PHY, QCA_PHY_EXACT_MASK) ||
	    nss_phydev_id_compare(phydev, IPQ52XX_PHY, QCA_PHY_EXACT_MASK) ||
	    nss_phydev_id_compare(phydev, QCA8081_PHY, GENMASK(31, 0))))
		return 0;

	ptp_ops = devm_kzalloc(&phydev->mdio.dev, sizeof(*ptp_ops), GFP_KERNEL);
	if (!ptp_ops) {
		phydev_err(phydev, "nss phy ptp ops kzalloc failed!\n");
		return -NSS_PHY_ENOSPC;
	}

	ret = nss_phy_ptp_ops_init(ptp_ops);
	if (ret)
		return ret;

	phy_ops->ptp_ops = ptp_ops;

	return 0;
}
#endif

static int nss_phy_ops_init(struct phy_device *phydev,
		int (*ops_init)(struct nss_phy_ops *ops))
{
	int ret = -NSS_PHY_EOPNOTSUPP;
	struct nss_phy_ops *ops = NULL;

	if (phydev->drv->driver_data)
		return 0;

	ops = devm_kzalloc(&phydev->mdio.dev, sizeof(struct nss_phy_ops), GFP_KERNEL);
	if (!ops) {
		phydev_err(phydev, "nss phy ops kzalloc failed!\n");
		return -NSS_PHY_ENOSPC;
	}

	if (ops_init) {
		ret = ops_init(ops);
		if (ret < 0) {
			phydev_err(phydev, "nss phy ops init failed!\n");
			return ret;
		}
	} else {
		ret = -NSS_PHY_EOPNOTSUPP;
	}
#if defined(NSS_PHY_PTP)
	ret = nss_phy_ptp_ops_add(phydev, ops);
	if (ret) {
		return ret;
	}
#endif
	phydev->drv->driver_data = ops;

	return 0;
}

static int nss_phy_eee_status_init(struct phy_device *phydev)
{
	u32 eee_cap = linkmode_to_mii_eee_cap1_t(phydev->supported_eee);

	/* for qca81xx and qca8084, the eee_enabled is enabled by phy structure,
	for qca807x, qca8081, qca803x and qca833x, the eee_enabled is disabled here
	manually */
	if (((phydev->eee_broken_modes & eee_cap) == eee_cap) ||
		nss_phydev_id_compare(phydev, QCA8075_PHY, QCA807X_MASK) ||
		nss_phydev_id_compare(phydev, QCA8081_PHY, QCA_PHY_EXACT_MASK) ||
		nss_phydev_id_compare(phydev, QCA8033_PHY, QCA803X_MASK) ||
		nss_phydev_id_compare(phydev, QCA8337_PHY_V4, QCA8337_PHY_MASK))
		phydev->eee_enabled = false;

	return 0;
}

static int nss_phy_probe(struct phy_device *phydev)
{
	struct nss_phy_device *nss_phydev;
	int (*ops_init)(struct nss_phy_ops *ops) = NULL;

	if (!phydev)
		return -ENODEV;

	nss_phydev = devm_kzalloc(&phydev->mdio.dev, sizeof(*nss_phydev), GFP_KERNEL);
	if (!nss_phydev)
		return -ENOMEM;

	if (nss_phydev_id_compare(phydev, QCA8075_PHY, QCA807X_MASK)) {
		atomic_inc(&g_nss_phy_manager.debug_stats.qca807x_num);
#if defined(CONFIG_NSSPHY_QCA807X)
		ops_init = qca807x_phy_ops_init;
#endif
	} else if (nss_phydev_id_compare(phydev, QCA8111_PHY, QCA81XX_MASK)) {
		atomic_inc(&g_nss_phy_manager.debug_stats.qca81xx_num);
#if defined(CONFIG_NSSPHY_QCA81XX)
		ops_init = qca81xx_phy_ops_init;
#endif
	} else if (nss_phydev_id_compare(phydev, QCE1204_PHY, QCA_PHY_EXACT_MASK) ||
		nss_phydev_id_compare(phydev, IPQ52XX_PHY, QCA_PHY_EXACT_MASK)) {
#if (defined(CONFIG_NSSPHY_QCE1204) || defined(CONFIG_NSSPHY_IPQ52XX))
		ops_init = qce1204_phy_ops_init;
#endif
	} else if (nss_phydev_id_compare(phydev, QCA8084_PHY, QCA808X_MASK)) {
		atomic_inc(&g_nss_phy_manager.debug_stats.qca808x_num);
#if defined(CONFIG_NSSPHY_QCA808X)
		ops_init = qca808x_phy_ops_init;
#endif
	} else if (nss_phydev_id_compare(phydev, QCA8033_PHY, QCA803X_MASK)) {
		atomic_inc(&g_nss_phy_manager.debug_stats.qca803x_num);
#if defined(CONFIG_NSSPHY_QCA803X)
		ops_init = qca803x_phy_ops_init;
#endif
	} else if (nss_phydev_id_compare(phydev, QCA8337_PHY_V4, QCA8337_PHY_MASK)) {
		atomic_inc(&g_nss_phy_manager.debug_stats.qca833x_num);
#if defined(CONFIG_NSSPHY_QCA833X)
		ops_init = qca833x_phy_ops_init;
#endif
	} else {
		atomic_inc(&g_nss_phy_manager.debug_stats.unknown_phy_num);
	}

	/*init nss phy ops*/
	nss_phy_ops_init(phydev, ops_init);
	/*init base addr*/
	nss_phy_base_addr_init(phydev);

	nss_phydev->phydev = phydev;
	dev_set_drvdata(&phydev->mdio.dev, nss_phydev);

	/* Initialize extended state and statistics */
	atomic_set(&nss_phydev->pcs_state, 1);
	atomic64_set(&nss_phydev->adjust_link_post_count, 0);

	nss_phy_debugfs_init(phydev);
	/* init eee status */
	nss_phy_eee_status_init(phydev);

	return 0;
}

static int nss_phy_match_phy_device(struct phy_device *phydev)
{
	if (!QCA_PHY_MATCH(nss_phydev_id_get(phydev)))
		return false;

	if (phydev->drv == NULL)
		return true;

	nss_phy_probe(phydev);

	return false;
}

static void nss_phy_remove(struct phy_device *phydev)
{
	nss_phy_debugfs_exit(phydev);
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
	.remove = nss_phy_remove,
	.match_phy_device = nss_phy_match_phy_device,
	.get_features = nss_phy_read_abilities,
	.read_status = nss_phy_read_status,
	.suspend = nss_phy_suspend,
	.resume = nss_phy_resume,
};

static int nss_phy_fixup(struct phy_device *phydev)
{
	struct nss_phy_device nss_phydev = {0};
	int ret = 0;

	nss_phydev.phydev = phydev;
	if (nss_phydev_id_compare(phydev, QCA8084_PHY, QCA_PHY_EXACT_MASK)) {
#if defined(CONFIG_NSSPHY_QCA808X)
		ret = qca8084_phy_fixup(&nss_phydev);
#endif
	} else if (nss_phydev_id_compare(phydev, QCA8033_PHY, QCA803X_MASK)) {
#if defined(CONFIG_NSSPHY_QCA803X)
		ret = qca803x_phy_fixup(&nss_phydev);
#endif
	} else if (nss_phydev_id_compare(phydev, QCA8075_PHY, QCA807X_MASK)) {
#if defined(CONFIG_NSSPHY_QCA807X)
		ret = qca807x_phy_fixup(&nss_phydev);
#endif
	}

	return ret;
}

#if IS_ENABLED(CONFIG_MDIO_I2C)
static struct mii_bus *nss_phy_mdio_i2c_bus_register(struct platform_device *pdev)
{
	struct device_node *i2c_node;
	struct i2c_adapter *i2c_adpt;
	struct mii_bus *mdio_i2c;

	i2c_node = of_parse_phandle(pdev->dev.of_node, "i2c-bus", 0);
	if(!i2c_node) {
		dev_err(&pdev->dev, "i2c-bus node was not found in platform device %s\n",
			pdev->name);
		return NULL;
	}

	i2c_adpt = of_find_i2c_adapter_by_node(i2c_node);
	of_node_put(i2c_node);
	if(!i2c_adpt) {
		dev_err(&pdev->dev, "i2c adpt was not found by i2c_node:%s\n",
			i2c_node->full_name);
		return NULL;
	}
	mdio_i2c = mdio_i2c_alloc(&pdev->dev, i2c_adpt, MDIO_I2C_QCOM);
	if (!mdio_i2c) {
		dev_err(&pdev->dev, "mdio_i2c bus alloc failed with i2c_adpt:%s\n",
			i2c_adpt->name);
		put_device(&i2c_adpt->dev);
		return NULL;
	}
	mdio_i2c->name = "SFP I2C Bus";
	if (of_mdiobus_register(mdio_i2c, i2c_node) < 0) {
		dev_err(&pdev->dev, "mdio_i2c bus register failed with bus id %s\n",
			mdio_i2c->id);
		mdiobus_free(mdio_i2c);
		put_device(&i2c_adpt->dev);
		return NULL;
	}

	atomic64_inc(&g_nss_phy_manager.debug_stats.mdio_i2c_bus_num);
	return mdio_i2c;
}

static u32 nss_sfp_phy_id_get(struct mii_bus *bus)
{
	u16 org_id, rev_id;
	u32 phy_id;

	if (!bus)
		return NSS_INVALID_PHY_ID;

	org_id = mdiobus_c45_read(bus, NSS_SFP_PHY_ADDR, MDIO_MMD_AN, MDIO_DEVID1);
	if (org_id < 0)
		return NSS_INVALID_PHY_ID;
	rev_id = mdiobus_c45_read(bus, NSS_SFP_PHY_ADDR, MDIO_MMD_AN, MDIO_DEVID2);
	if (rev_id < 0)
		return NSS_INVALID_PHY_ID;
	phy_id = ((org_id << 16) | rev_id);

	return phy_id;
}

static int nss_phy_sfp_device_register(struct mii_bus *bus)
{
	struct phy_device *phydev = NULL;
	u32 phy_id = nss_sfp_phy_id_get(bus);
	int ret = 0;

	if (phy_id == NSS_INVALID_PHY_ID)
		return -EINVAL;

	if (phy_id_compare(phy_id, QCA8111_PHY, QCA_PHY_EXACT_MASK))
		phydev = get_phy_device(bus, NSS_SFP_PHY_ADDR, true);
	else
		return -EOPNOTSUPP;

	if (!phydev)
		return -EINVAL;

	ret = phy_device_register(phydev);
	if (ret) {
		phy_device_free(phydev);
		return ret;
	}

	atomic64_inc(&g_nss_phy_manager.debug_stats.sfp_devices_num);
	return 0;
}
#endif

static int nss_phy_platform_probe(struct platform_device *pdev)
{
#if IS_ENABLED(CONFIG_MDIO_I2C)
	struct mii_bus *bus;

	bus = nss_phy_mdio_i2c_bus_register(pdev);
	if (bus) {
		nss_phy_sfp_device_register(bus);
		platform_set_drvdata(pdev, bus);
		dev_info(&pdev->dev, "nss-phy mdio-i2c bus registered successfully\n");
	}
#endif

	return 0;
}

static int nss_phy_platform_remove(struct platform_device *pdev)
{
#if IS_ENABLED(CONFIG_MDIO_I2C)
	struct phy_device *phydev = NULL;
	struct mii_bus *bus = dev_get_drvdata(&pdev->dev);
	if (bus) {
		phydev = mdiobus_get_phy(bus, NSS_SFP_PHY_ADDR);
		if (phydev)
			phy_device_remove(phydev);
		mdiobus_unregister(bus);
	}
#endif

	return 0;
}

static const struct of_device_id nss_phy_of_match[] = {
	{.compatible = "qcom,nss-phy" },
	{}
};

MODULE_DEVICE_TABLE(of, nss_phy_of_match);

static struct platform_driver nss_phy_platform_driver = {
	.probe = nss_phy_platform_probe,
	.remove = nss_phy_platform_remove,
	.driver = {
		.name = "nss-phy",
		.of_match_table = nss_phy_of_match,
	},
};

static int __init nss_phy_module_init(void)
{
	int ret = 0;

	/* Initialize global state and statistics */
	g_nss_phy_manager.init_state = NSS_PHY_INIT_START;

	atomic_set(&g_nss_phy_manager.debug_stats.qca807x_num, 0);
	atomic_set(&g_nss_phy_manager.debug_stats.qca81xx_num, 0);
	atomic_set(&g_nss_phy_manager.debug_stats.qca808x_num, 0);
	atomic_set(&g_nss_phy_manager.debug_stats.qca803x_num, 0);
	atomic_set(&g_nss_phy_manager.debug_stats.qca833x_num, 0);
	atomic_set(&g_nss_phy_manager.debug_stats.unknown_phy_num, 0);
	atomic64_set(&g_nss_phy_manager.debug_stats.mdio_i2c_bus_num, 0);
	atomic64_set(&g_nss_phy_manager.debug_stats.sfp_devices_num, 0);

	nss_phy_debugfs_create();

	ret = platform_driver_register(&nss_phy_platform_driver);
	if (ret < 0) {
		pr_err("Failed to register nss_phy platform driver\n");
		g_nss_phy_manager.init_state = NSS_PHY_INIT_PLATFORM_DRIVER_REGISTER_FAILURE;
		return ret;
	}

	ret = phy_driver_register(&nss_phy_driver, THIS_MODULE);
	if (!ret) {
		pr_info("nss phy driver register successfully\n");
	} else {
		pr_err("Failed to register nss_phy driver\n");
		g_nss_phy_manager.init_state = NSS_PHY_INIT_PHY_DRIVER_REGISTER_FAILURE;
		platform_driver_unregister(&nss_phy_platform_driver);
		return ret;
	}

	ret = phy_register_fixup_for_uid(QCA_PHY_ID, QCA_PHY_MASK,
		nss_phy_fixup);
	if (ret < 0)
		return ret;
	g_nss_phy_manager.init_state = NSS_PHY_INIT_SUCCESS;

	return 0;
}

static void __exit nss_phy_module_exit(void)
{
	phy_driver_unregister(&nss_phy_driver);
	phy_unregister_fixup_for_uid(QCA_PHY_ID, QCA_PHY_MASK);
	platform_driver_unregister(&nss_phy_platform_driver);
	nss_phy_debugfs_remove();
}
module_init(nss_phy_module_init);
module_exit(nss_phy_module_exit);
MODULE_DESCRIPTION("NSS PHY driver");
MODULE_LICENSE("Dual BSD/GPL");
