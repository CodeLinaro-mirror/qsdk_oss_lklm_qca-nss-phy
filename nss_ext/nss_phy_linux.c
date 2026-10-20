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
#include "nss_phy_c45_common.h"
#include <linux/of_device.h>
#include <linux/of_mdio.h>
#if IS_ENABLED(CONFIG_MDIO_I2C)
#include <linux/mdio/mdio-i2c.h>
#include <linux/i2c.h>
#include <linux/gpio/consumer.h>
#include <linux/bitmap.h>
#include <linux/delay.h>
#endif
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include "nss_phy_linux_wrapper.h"

#define NSS_PHY_STATUS_POLL_INTERVAL_MS	1000

static void nss_phy_status_poll_cancel(void *data)
{
	struct nss_phy_device *nss_phydev = data;

	cancel_delayed_work_sync(&nss_phydev->status_poll_work);
}

static void nss_phy_status_poll_work_fn(struct work_struct *work)
{
	struct nss_phy_device *nss_phydev =
		container_of(work, struct nss_phy_device, status_poll_work.work);
	struct nss_phy_ops *ops =
		(struct nss_phy_ops *)nss_phydev->phydev->drv->driver_data;
	bool link_up = !!nss_phydev->phydev->link;
	bool link_up_transition = link_up && !nss_phydev->status_poll_prev_link;
	bool link_down_transition = !link_up && nss_phydev->status_poll_prev_link;

	nss_phydev->status_poll_prev_link = link_up;

	/* Periodic poll tasks — add new per-feature helpers here as needed. */
	if (ops && ops->fr_cfg_set &&
	    (nss_phydev->fr_ieee_enabled || nss_phydev->fr_cisco_enabled))
		nss_phy_c45_common_fr_cnt_poll(nss_phydev, link_up_transition);

	if (ops && ops->an_fail_counter_get)
		nss_phy_common_an_fail_cnt_poll(nss_phydev,
			link_up_transition || link_down_transition);

	if (ops && ops->flap_stats_get &&
	    (link_up_transition || link_down_transition))
		nss_phy_common_flap_stats_update(nss_phydev,
			link_up_transition, link_down_transition);

	schedule_delayed_work(&nss_phydev->status_poll_work,
		msecs_to_jiffies(NSS_PHY_STATUS_POLL_INTERVAL_MS));
}

static struct nss_phy_global_manager g_nss_phy_manager = {0};

/*
 * nss_phy_ext_state_show()
 *	Sysfs callback to display extended PHY state.
 */
static ssize_t nss_phy_ext_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct phy_device *phydev = to_phy_device(dev);
	struct nss_phy_device *nss_phydev = dev_get_drvdata(&phydev->mdio.dev);
	struct nss_phy_ops *ops = (struct nss_phy_ops *)phydev->drv->driver_data;
	ssize_t count = 0;

	count += scnprintf(buf + count, PAGE_SIZE - count, "NSS PHY Extended State\n");

	if (ops && ops->pcs_status_get) {
		struct nss_phy_pcs_status status = {0};

		if (!ops->pcs_status_get(nss_phydev, &status)) {
			count += scnprintf(buf + count, PAGE_SIZE - count, "    %-20s : %u\n", "pcs_locked", status.pcs_locked);
			count += scnprintf(buf + count, PAGE_SIZE - count, "    %-20s : %u\n", "block_lock", status.block_lock);
		}
	}

	if (ops && ops->link_training_completion_get) {
		u32 training_complete = 0;

		if (!ops->link_training_completion_get(nss_phydev, &training_complete))
			count += scnprintf(buf + count, PAGE_SIZE - count, "    %-20s : 0x%x\n", "training_complete", training_complete);
	}

	/* mse_get may block ~4s at 2.5G/5G/10G while the MSE accumulator collects samples. */
	if (ops && ops->mse_get) {
		static const char * const quality_str[] = {
			[NSS_PHY_MSE_QUALITY_NA]        = "n/a",
			[NSS_PHY_MSE_QUALITY_GREAT]     = "great",
			[NSS_PHY_MSE_QUALITY_GOOD]      = "good",
			[NSS_PHY_MSE_QUALITY_NORMAL]    = "normal",
			[NSS_PHY_MSE_QUALITY_CRC]       = "crc",
			[NSS_PHY_MSE_QUALITY_LINK_DOWN] = "link_down",
		};
		struct nss_phy_mse mse = {0};
		int i;

		if (!ops->mse_get(nss_phydev, &mse)) {
			for (i = 0; i < 4; i++) {
				s32 margin = mse.snr_margin[i];
				s32 abs_margin = margin < 0 ? -margin : margin;
				const char *qstr = mse.quality[i] < ARRAY_SIZE(quality_str) ?
					quality_str[mse.quality[i]] : "?";

				count += scnprintf(buf + count, PAGE_SIZE - count,
					"    ch%d_mse           : %u (%s)\n",
					i, mse.mse[i], qstr);
				count += scnprintf(buf + count, PAGE_SIZE - count,
					"    ch%d_mse_snr_margin: %s%d.%02d dB\n",
					i, margin < 0 ? "-" : "", abs_margin / 100, abs_margin % 100);
			}
		}
	}

	/* clk_ppm_offset_get is intrusive: it forces slave mode and restarts AN,
	 * disrupting any live link for up to ~15 s when the PHY is currently master.
	 * Reading this sysfs attribute on an active link is intentional for
	 * lab/debug use only.
	 */
	if (ops && ops->clk_ppm_offset_get) {
		int ppm = 0;
		int ret = ops->clk_ppm_offset_get(nss_phydev, &ppm);

		if (!ret)
			count += scnprintf(buf + count, PAGE_SIZE - count,
				"    clk_ppm_offset      : %d\n", ppm);
		else
			count += scnprintf(buf + count, PAGE_SIZE - count,
				"    clk_ppm_offset      : n/a\n");
	}

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

	if (ops && ops->an_fail_counter_get) {
		u64 count = 0;

		ops->an_fail_counter_get(nss_phydev, &count);
		ret_count += scnprintf(buf + ret_count, PAGE_SIZE - ret_count, "    %-20s : %llu\n", "an_fail_count", count);
	}

	if (ops && ops->ldpc_stats_get) {
		struct nss_phy_ldpc_stats s = {0};
		int ret = ops->ldpc_stats_get(nss_phydev, &s);

		if (!ret)
			ret_count += scnprintf(buf + ret_count, PAGE_SIZE - ret_count,
				"    %-20s : iter0=%llu iter1=%llu iter2=%llu iter3=%llu iter4=%llu"
				" uncorrected=%u avg_iter=%d.%02d\n",
				"ldpc_stats",
				s.iter[0], s.iter[1], s.iter[2], s.iter[3], s.iter[4],
				s.uncorrected,
				s.avg_iter_x100 / 100, s.avg_iter_x100 % 100);
		else
			ret_count += scnprintf(buf + ret_count, PAGE_SIZE - ret_count,
				"    %-20s : n/a (%d)\n", "ldpc_stats", ret);
	}

	if (ops && ops->flap_stats_get) {
		struct nss_phy_link_flap_stats s = {0};

		if (!ops->flap_stats_get(nss_phydev, &s)) {
			ret_count += scnprintf(buf + ret_count, PAGE_SIZE - ret_count,
				"    %-20s : %lld\n", "flap_up_count",
				atomic64_read(&s.up_count));
			ret_count += scnprintf(buf + ret_count, PAGE_SIZE - ret_count,
				"    %-20s : %lld\n", "flap_down_count",
				atomic64_read(&s.down_count));
			ret_count += scnprintf(buf + ret_count, PAGE_SIZE - ret_count,
				"    %-20s : %lld\n", "flap_last_up_time",
				atomic64_read(&s.last_up_time));
			ret_count += scnprintf(buf + ret_count, PAGE_SIZE - ret_count,
				"    %-20s : %lld\n", "flap_last_down_time",
				atomic64_read(&s.last_down_time));
			ret_count += scnprintf(buf + ret_count, PAGE_SIZE - ret_count,
				"    %-20s : %lld\n", "flap_last_change_time",
				atomic64_read(&s.last_change_time));
		}
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
	struct nss_phy_ops *ops = (struct nss_phy_ops *)phydev->drv->driver_data;

	if (buf[0] == '0' || buf[0] == '\n') { /* Any write with '0' or newline clears statistics */
		atomic64_set(&nss_phydev->adjust_link_post_count, 0);
		if (ops && ops->an_fail_counter_reset)
			ops->an_fail_counter_reset(nss_phydev);
		if (ops && ops->flap_stats_reset)
			ops->flap_stats_reset(nss_phydev);
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
	struct nss_phy_ops *ops;
	int (*ops_init)(struct nss_phy_ops *ops) = NULL;

	if (!phydev)
		return -ENODEV;

	/* match_phy_device() calls probe() on every invocation and returns
	 * false, so phylib may call us multiple times for the same phydev.
	 * Guard against repeated init just as nss_phy_ops_init() does.
	 */
	if (dev_get_drvdata(&phydev->mdio.dev))
		return 0;

	nss_phydev = devm_kzalloc(&phydev->mdio.dev, sizeof(*nss_phydev), GFP_KERNEL);
	if (!nss_phydev)
		return -ENOMEM;

	spin_lock_init(&nss_phydev->fr_sw_cnt_lock);
	mutex_init(&nss_phydev->mse_lock);
	mutex_init(&nss_phydev->ppm_lock);
	atomic64_set(&nss_phydev->an_fail_count, 0);
	INIT_DELAYED_WORK(&nss_phydev->status_poll_work, nss_phy_status_poll_work_fn);

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

	/* Initialize extended statistics */
	atomic64_set(&nss_phydev->adjust_link_post_count, 0);
	nss_phy_common_flap_stats_reset(nss_phydev);

	nss_phy_debugfs_init(phydev);
	/* init eee status */
	nss_phy_eee_status_init(phydev);

	/*
	 * Seed the Fast Retrain enable-state cache with the current hardware
	 * state - see nss_phy_c45_common_fr_state_init().
	 */
	ops = (struct nss_phy_ops *)phydev->drv->driver_data;
	if (ops && ops->fr_cfg_set)
		nss_phy_c45_common_fr_state_init(nss_phydev);

	schedule_delayed_work(&nss_phydev->status_poll_work,
		msecs_to_jiffies(NSS_PHY_STATUS_POLL_INTERVAL_MS));

	/*
	 * nss_phy_driver never truly binds (match_phy_device returns false
	 * after calling probe), so phylib will never call .remove.  Register
	 * a devres action so the work is cancelled when the mdio device is
	 * torn down (e.g. on rmmod).
	 */
	return devm_add_action_or_reset(&phydev->mdio.dev,
		nss_phy_status_poll_cancel, nss_phydev);
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
	struct nss_phy_device *nss_phydev = dev_get_drvdata(&phydev->mdio.dev);

	/* This driver uses the parasitic probe pattern: match_phy_device()
	 * calls nss_phy_probe() and returns false, so phylib never truly
	 * binds and will not call .remove in normal operation.  The real
	 * cleanup path is the devm action registered in nss_phy_probe().
	 * Keep a cancel here as a defensive fallback.
	 */
	if (nss_phydev)
		cancel_delayed_work_sync(&nss_phydev->status_poll_work);
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
#define NSS_PHY_SFP_RESET_DELAY_US	100000

/*
 * nss_phy_mdio_i2c_reset()
 *	Pulse the optional reset GPIOs of the devices on the mdio-i2c bus.
 *
 * The reset is optional, so this never fails the bus registration that
 * calls it: with no GPIOs described, or with no memory for the values
 * bitmap, the pulse is simply skipped.
 */
static int nss_phy_mdio_i2c_reset(struct mii_bus *bus)
{
	/* mdio_i2c_alloc() returns early for MDIO_I2C_QCOM, so priv is the
	 * struct qcom_mdio_i2c_data of <linux/mdio/mdio-i2c.h> rather than
	 * the bare i2c_adapter the other protocols store there.  The
	 * reset_gpios used below is a member of that struct, and
	 * qcom_mdio_i2c_alloc() sizes the bus with
	 * devm_mdiobus_alloc_size(sizeof(*priv)), so the field comes with the
	 * bus allocation instead of being an extension made here.
	 */
	struct qcom_mdio_i2c_data *priv = bus->priv;
	unsigned long *values;

	if (!priv->reset_gpios)
		return 0;

	values = bitmap_alloc(priv->reset_gpios->ndescs, GFP_KERNEL);
	if (!values) {
		/* mdio_i2c_alloc() set bus->parent when it allocated the bus,
		 * so it is valid here even though this runs from inside
		 * of_mdiobus_register().
		 */
		dev_warn(bus->parent, "mii_bus %s reset bitmap alloc failed, skipping sfp reset\n",
			bus->id);
		return 0;
	}

	/* Assert reset on every descriptor, hold, then deassert.  The bits are
	 * logical values, so active-low lines are handled by the DT flags.
	 */
	bitmap_fill(values, priv->reset_gpios->ndescs);
	gpiod_set_array_value_cansleep(priv->reset_gpios->ndescs,
		priv->reset_gpios->desc, priv->reset_gpios->info, values);

	fsleep(NSS_PHY_SFP_RESET_DELAY_US);

	bitmap_zero(values, priv->reset_gpios->ndescs);
	gpiod_set_array_value_cansleep(priv->reset_gpios->ndescs,
		priv->reset_gpios->desc, priv->reset_gpios->info, values);
	bitmap_free(values);

	fsleep(NSS_PHY_SFP_RESET_DELAY_US);

	return 0;
}

static struct mii_bus *nss_phy_mdio_i2c_bus_register(struct platform_device *pdev)
{
	struct qcom_mdio_i2c_data *priv;
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
	if(!i2c_adpt) {
		dev_err(&pdev->dev, "i2c adpt was not found by i2c_node:%s\n",
			i2c_node->full_name);
		of_node_put(i2c_node);
		return NULL;
	}

	/* mdio_i2c_alloc() reports failure with an ERR_PTR, and for
	 * MDIO_I2C_QCOM the bus comes from devm_mdiobus_alloc_size(), so it
	 * must not be freed explicitly on the error paths below.
	 */
	mdio_i2c = mdio_i2c_alloc(&pdev->dev, i2c_adpt, MDIO_I2C_QCOM);
	if (IS_ERR(mdio_i2c)) {
		dev_err(&pdev->dev, "mdio_i2c bus alloc failed with i2c_adpt:%s (%ld)\n",
			i2c_adpt->name, PTR_ERR(mdio_i2c));
		of_node_put(i2c_node);
		put_device(&i2c_adpt->dev);
		return NULL;
	}

	/* Optional "sfp-reset-gpios" on the nss-phy node, stored in the
	 * qcom_mdio_i2c_data that MDIO_I2C_QCOM keeps in mdio_i2c->priv, and
	 * driven by nss_phy_mdio_i2c_reset() from of_mdiobus_register() below.
	 * The reset is best effort: boards that do not describe the property
	 * get a NULL array, and a board that does but whose GPIOs cannot be
	 * acquired keeps the bus, with the reset skipped.
	 *
	 * The pin may be shared with the module's TX_DISABLE, which the board
	 * declares with "reset-shared-tx-disable": the "default" pinctrl
	 * state of the nss-phy node then describes its TX_DISABLE configuration,
	 * and nss_phy_sfp_tx_disable_restore() gives the pin back once the reset
	 * has shown that there is no SFP PHY to hold in reset.
	 */
	priv = mdio_i2c->priv;
	priv->reset_gpios = devm_gpiod_get_array_optional(&pdev->dev, "sfp-reset",
		GPIOD_OUT_LOW);
	if (IS_ERR(priv->reset_gpios)) {
		dev_warn(&pdev->dev, "mii_bus %s couldn't get reset GPIO (%ld), skipping sfp reset\n",
			mdio_i2c->id, PTR_ERR(priv->reset_gpios));
		priv->reset_gpios = NULL;
	}

	mdio_i2c->name = "SFP I2C Bus";
	mdio_i2c->reset = nss_phy_mdio_i2c_reset;
	if (of_mdiobus_register(mdio_i2c, i2c_node) < 0) {
		dev_err(&pdev->dev, "mdio_i2c bus register failed with bus id %s\n",
			mdio_i2c->id);
		of_node_put(i2c_node);
		put_device(&i2c_adpt->dev);
		return NULL;
	}
	of_node_put(i2c_node);

	atomic64_inc(&g_nss_phy_manager.debug_stats.mdio_i2c_bus_num);
	return mdio_i2c;
}

/*
 * nss_phy_sfp_tx_disable_restore()
 *	Hand the shared reset line back to its TX_DISABLE function.
 *
 * A board may wire one pin as both the SFP PHY reset and the module's
 * TX_DISABLE, which it tells the driver with "reset-shared-tx-disable".
 * The two functions want opposite levels on such a pin: the reset is active
 * low, so leaving it deasserted holds the pin high, while TX_DISABLE is
 * asserted high, so that same level keeps the transmitter off.  Only one of
 * them can have the pin, which is why this runs once the post-reset probe has
 * shown that there is no SFP PHY to hold in reset: with no PHY to serve, the
 * pin belongs to TX_DISABLE.
 *
 * So drive the pin low, which deasserts TX_DISABLE and lets the module
 * transmit, then release the descriptors so that nothing drives it as a reset
 * any more.  Raw values are used because the level wanted here is TX_DISABLE's
 * and not the active-low reset polarity the descriptors were acquired with:
 * gpiod_set_value(0) would mean "reset deasserted", put the pin back high and
 * turn the transmitter off again.
 *
 * Releasing the descriptors leaves priv->reset_gpios NULL, which makes a later
 * nss_phy_mdio_i2c_reset() skip the pulse without saying anything.  That is
 * deliberate: the pin answers to TX_DISABLE from here on and must not be
 * driven as a reset again, so there is nothing to warn about.  It does not
 * arise in practice either, as mii_bus->reset runs only from
 * __mdiobus_register(), and this driver unregisters the bus on remove without
 * ever registering it a second time.
 *
 * Boards where the two are separate pins do not describe the property and are
 * left alone: their reset line stays owned by this driver, and their
 * TX_DISABLE pin is driven by whatever pinctrl state describes it.  The driver
 * cannot tell the two cases apart on its own, as TX_DISABLE is an output level
 * in a pinctrl state rather than a GPIO this driver could compare against.
 *
 * The pinmux, bias and drive strength come from the "default" pinctrl state
 * of the nss-phy node and are not disturbed by the GPIO request, so the
 * output level is all there is to restore.
 */
static void nss_phy_sfp_tx_disable_restore(struct platform_device *pdev,
	struct mii_bus *bus)
{
	struct qcom_mdio_i2c_data *priv = bus->priv;
	unsigned int i;

	if (!priv->reset_gpios)
		return;

	if (!of_property_read_bool(pdev->dev.of_node, "reset-shared-tx-disable"))
		return;

	for (i = 0; i < priv->reset_gpios->ndescs; i++)
		gpiod_set_raw_value_cansleep(priv->reset_gpios->desc[i], 0);

	devm_gpiod_put_array(&pdev->dev, priv->reset_gpios);
	priv->reset_gpios = NULL;

	dev_info(&pdev->dev, "mii_bus %s has no sfp phy, shared pin handed back to tx-disable, transmitter enabled\n",
		bus->id);
}

static u32 nss_sfp_phy_id_get(struct mii_bus *bus)
{
	int org_id, rev_id;
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

/*
 * nss_phy_sfp_device_register()
 *	Bind the SFP PHY behind an mdio-i2c bus, if this driver drives it.
 *
 * Returns 0 once a PHY is bound and -ENODEV when nothing answers on the bus.
 * Any other negative errno means a PHY is there but did not get bound, either
 * because this driver does not drive it or because its registration failed.
 *
 * The caller has to tell those two apart rather than test for plain failure:
 * only a bus with no PHY on it at all may have a shared reset pin handed back
 * to TX_DISABLE, because a PHY that is present still owns that pin.  "No PHY"
 * therefore gets an errno of its own instead of sharing -EINVAL with the
 * failures above.
 */
static int nss_phy_sfp_device_register(struct mii_bus *bus)
{
	struct phy_device *phydev;
	u32 phy_id = nss_sfp_phy_id_get(bus);
	int ret;

	if (phy_id == NSS_INVALID_PHY_ID)
		return -ENODEV;

	if (!phy_id_compare(phy_id, QCA8111_PHY, QCA_PHY_EXACT_MASK))
		return -EOPNOTSUPP;

	/* get_phy_device() reports failure with an ERR_PTR, so a NULL test
	 * would let an error pointer through to phy_device_register() and have
	 * it dereferenced.  Its -ENODEV reaches the caller as "no PHY", which
	 * is what it means: the PHY answered nss_sfp_phy_id_get() above but no
	 * longer answers the ID reads done here.
	 */
	phydev = get_phy_device(bus, NSS_SFP_PHY_ADDR, true);
	if (IS_ERR(phydev))
		return PTR_ERR(phydev);

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
	int ret;

	bus = nss_phy_mdio_i2c_bus_register(pdev);
	if (bus) {
		/* of_mdiobus_register() has pulsed the optional reset line by
		 * now, so the SFP PHY, if any, is ready to answer.  Only when
		 * nothing answers at all is there no PHY left to own the reset
		 * line, and only then may a pin shared with TX_DISABLE go back
		 * to that function.  A PHY that is present but unbound, either
		 * because this driver does not drive it or because registering
		 * it failed, keeps the pin: it may still need to be held in
		 * reset, and releasing it would let an unmanaged module start
		 * transmitting.
		 */
		ret = nss_phy_sfp_device_register(bus);
		if (ret == -ENODEV)
			nss_phy_sfp_tx_disable_restore(pdev, bus);
		else if (ret)
			dev_warn(&pdev->dev, "mii_bus %s sfp phy present but not bound (%d), reset pin not released\n",
				bus->id, ret);

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
