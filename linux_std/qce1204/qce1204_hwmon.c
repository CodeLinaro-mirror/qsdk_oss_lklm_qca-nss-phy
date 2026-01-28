/*
* Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
* SPDX-License-Identifier: ISC
*/

#include <linux/device.h>
#include <linux/hwmon.h>
#include <linux/ctype.h>
#include "qce1204.h"

#define QCE1204_VDD4BLOW_EN					0x902038
#define QCE1204_POWER_DOWN					BIT(2)
#define QCE1204_POWER_EN					BIT(3)
#define QCE1204_QFPROM_BLOW_TIMER				0x90203C
#define QCE1204_BELOW_TIMER_MASK				GENMASK(11, 0)
#define QCE1204_BELOW_TIMER_10US				0x190
#define QCE1204_QFPROM_RAW_CALIBRATION_ROW6_MSB			0x90005C
#define QCE1204_TSENSOR_BASE_CODE_120C				GENMASK(19, 10)
#define QCE1204_TSENSOR_BASE_CODE_30C				GENMASK(9, 0)
#define QCE1204_TSENSOR0_OFFSET					GENMASK(23, 20)
#define QCE1204_QFPROM_RAW_CALIBRATION_ROW7_LSB			0x900060
#define QCE1204_TSENSOR1_OFFSET					GENMASK(3, 0)
#define QCE1204_TSENSOR_CAL_RESULT_MASK				GENMASK(22, 20)
#define QCE1204_TSENSOR_CAL_RESULT_DONE				0x3
#define QCE1204_TSENS_0_CONVERSION				0x920060
#define QCE1204_CZERO_MASK					GENMASK(9, 0)
#define QCE1204_SLOPE_MASK					GENMASK(22, 10)
#define QCE1204_TSENS_CTRL					0x920004
#define QCE1204_TSENS_GLOBAL_EN					BIT(0)
#define QCE1204_TSENSORS_EN					GENMASK(7, 3)
#define QCE1204_TSENS_0_STATUS					0x9300A0
#define QCE1204_LAST_TEMP_MASK					GENMASK(11, 0)
#define QCE1204_CZERO_DEFAULT					0x161
#define QCE1204_SLOPE_DEFAULT					0xcdb

static umode_t qce1204_hwmon_is_visible(const void *data,
	enum hwmon_sensor_types type, u32 attr, int channel)
{
	if (type != hwmon_temp)
		return 0;

	if (attr == hwmon_temp_input)
		return 0444;

	return 0;
}

static long qce1204_hwmon_temp_read(struct phy_device *phydev, int sensor_id)
{
	u32 phy_data0 = 0, phy_data1 = 0, slope = 0, index = 0;
	long code = 0, czero = 0, temp = 0;

	if (sensor_id >= 2)
		return -EOPNOTSUPP;
	index = qce1204_phy_channel_get(phydev);
	phy_lock_mdio_bus(phydev);
	__qce1204_soc_modify(phydev, QCE1204_TSENS_CTRL,
		QCE1204_TSENS_GLOBAL_EN | QCE1204_TSENSORS_EN,
		QCE1204_TSENS_GLOBAL_EN | QCE1204_TSENSORS_EN);
	if (sensor_id == 0) {
		phy_data0 = __qce1204_soc_read(phydev, QCE1204_TSENS_0_STATUS + 4);
		phy_data1 = __qce1204_soc_read(phydev, QCE1204_TSENS_0_CONVERSION);
	} else {
		phy_data0 = __qce1204_soc_read(phydev, QCE1204_TSENS_0_STATUS + 4 * index);
		phy_data1 = __qce1204_soc_read(phydev, QCE1204_TSENS_0_CONVERSION + 4 * index);
	}
	code = phy_data0 & QCE1204_LAST_TEMP_MASK;
	czero = phy_data1 & QCE1204_CZERO_MASK;
	slope = (phy_data1 & QCE1204_SLOPE_MASK) >> 10;
	temp = (code - czero) * slope >> 10;
	__qce1204_soc_modify(phydev, QCE1204_TSENS_CTRL, QCE1204_TSENS_GLOBAL_EN | QCE1204_TSENSORS_EN, 0);
	phy_unlock_mdio_bus(phydev);

	return temp;
}

static int qce1204_hwmon_read(struct device *dev, enum hwmon_sensor_types type,
	u32 attr, int sensor_id, long *value)
{
	struct phy_device *phydev = dev_get_drvdata(dev);

	if (!phydev)
		return -EINVAL;
	if (type != hwmon_temp)
		return -EOPNOTSUPP;
	if (attr == hwmon_temp_input)
		*value = qce1204_hwmon_temp_read(phydev, sensor_id);
	else
		return -EOPNOTSUPP;

	return 0;
}

static const struct hwmon_ops qce1204_hwmon_ops = {
	.is_visible = qce1204_hwmon_is_visible,
	.read = qce1204_hwmon_read,
};

static u32 qce1204_hwmon_chip_config[] = {
	HWMON_C_REGISTER_TZ,
	0,
};

static const struct hwmon_channel_info qce1204_hwmon_chip = {
	.type = hwmon_chip,
	.config = qce1204_hwmon_chip_config,
};

static u32 qce1204_hwmon_temp_config[] = {
	HWMON_T_INPUT,
	HWMON_T_INPUT,
	0,
};

static const struct hwmon_channel_info qce1204_hwmon_temp = {
	.type = hwmon_temp,
	.config = qce1204_hwmon_temp_config,
};

static const struct hwmon_channel_info * qce1204_hwmon_info[] = {
	&qce1204_hwmon_chip,
	&qce1204_hwmon_temp,
	NULL,
};

static const struct hwmon_chip_info qce1204_hwmon_chip_info = {
	.ops = &qce1204_hwmon_ops,
	.info = qce1204_hwmon_info,
};

int qce1204_hwmon_hw_init(struct phy_device *phydev)
{
	u32 base_code_120c, base_code_30c, tsens_offset, cal_result, index;
	u32 slope  = QCE1204_SLOPE_DEFAULT, czero = QCE1204_CZERO_DEFAULT;
	struct phy_package_shared *shared = phydev->shared;
	struct qce1204_shared_priv *priv;

	if (!shared)
		return -EINVAL;

	priv = (struct qce1204_shared_priv *)shared->priv;
	if (!priv)
		return -EINVAL;

	if (phy_package_init_once(phydev)) {
		u32 msb, lsb;
		/* configure the ready time of qfproom as 10us */
		qce1204_soc_modify(phydev, QCE1204_QFPROM_BLOW_TIMER,
			QCE1204_BELOW_TIMER_MASK, QCE1204_BELOW_TIMER_10US);
		/* enable power */
		qce1204_soc_modify(phydev, QCE1204_VDD4BLOW_EN,
			QCE1204_POWER_DOWN | QCE1204_POWER_EN, QCE1204_POWER_EN);
		mdelay(1);
		msb = qce1204_soc_read(phydev,
			QCE1204_QFPROM_RAW_CALIBRATION_ROW6_MSB);
		lsb = qce1204_soc_read(phydev,
			QCE1204_QFPROM_RAW_CALIBRATION_ROW7_LSB);
		priv->tem_base_code = ((u64)msb << 32) | lsb;
		/* disable power */
		qce1204_soc_modify(phydev, QCE1204_VDD4BLOW_EN,
			QCE1204_POWER_DOWN | QCE1204_POWER_EN, QCE1204_POWER_DOWN);
	}
	cal_result = ((u32)priv->tem_base_code & QCE1204_TSENSOR_CAL_RESULT_MASK) >> 20;
	if (cal_result == QCE1204_TSENSOR_CAL_RESULT_DONE) {
		base_code_30c = (u32)(priv->tem_base_code >> 32) & QCE1204_TSENSOR_BASE_CODE_30C;
		base_code_120c = ((u32)(priv->tem_base_code >> 32) & QCE1204_TSENSOR_BASE_CODE_120C) >> 10;
		if (base_code_120c > base_code_30c) {
			slope = 921600/(base_code_120c - base_code_30c);
		} else {
			phydev_err(phydev, "Invalid calibration data: base_code_120c=%u, base_code_30c=%u\n",
				   base_code_120c, base_code_30c);
			return -EINVAL;
		}
	}

	/* this tsensor is for SOC, need init one time */
	if (phy_package_init_once(phydev)) {
		if (cal_result == QCE1204_TSENSOR_CAL_RESULT_DONE) {
			tsens_offset = ((u32)(priv->tem_base_code >> 32) & QCE1204_TSENSOR0_OFFSET) >> 20;
			czero = base_code_30c + tsens_offset;
		}
		qce1204_soc_modify(phydev, QCE1204_TSENS_0_CONVERSION,
			QCE1204_CZERO_MASK | QCE1204_SLOPE_MASK, czero | (slope << 10));
	}
	/* init the tsensor for PHY */
	index = qce1204_phy_channel_get(phydev);
	if (cal_result == QCE1204_TSENSOR_CAL_RESULT_DONE) {
		tsens_offset = ((u32)priv->tem_base_code & (QCE1204_TSENSOR1_OFFSET << ((index - 1) * 4)))
			>> ((index - 1) * 4);
		czero = base_code_30c + tsens_offset;
	}
	qce1204_soc_modify(phydev, QCE1204_TSENS_0_CONVERSION + index * 4,
		QCE1204_CZERO_MASK | QCE1204_SLOPE_MASK, czero | (slope <<10));

	return 0;
}

/**
 * qce1204_sanitize_hwmon_name - Sanitize device name for hwmon
 * @name: Input device name string
 *
 * Remove non-alphanumeric characters from device name in-place.
 * This is required for hwmon device registration.
 */
static void qce1204_sanitize_hwmon_name(char *name)
{
	char *src = name;
	char *dst = name;

	while (*src) {
		if (isalnum(*src))
			*dst++ = *src;
		src++;
	}
	*dst = '\0';
}

int qce1204_hwmon_probe(struct phy_device *phydev)
{
	struct qce1204_priv *priv = phydev->priv;
	struct device *dev = &phydev->mdio.dev;
	char *hwmon_name;

	hwmon_name = devm_kstrdup(dev, dev_name(dev), GFP_KERNEL);
	if (!hwmon_name)
		return -ENOMEM;

	qce1204_sanitize_hwmon_name(hwmon_name);

	priv->hwmon_dev = devm_hwmon_device_register_with_info(dev, hwmon_name,
							       phydev, &qce1204_hwmon_chip_info, NULL);

	return PTR_ERR_OR_ZERO(priv->hwmon_dev);
}
