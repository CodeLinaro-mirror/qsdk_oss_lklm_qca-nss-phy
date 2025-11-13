/*
* Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
* SPDX-License-Identifier: ISC
*/

#ifndef _QCE1204_H_
#define _QCE1204_H_

#include <linux/phy.h>

#define QCE1204_PHY		0x004dd190

enum qce1204_addr_offset {
	PCS0_ADDR_OFFSET = 4,
	PCS1_ADDR_OFFSET = 5,
	QCE1204_SOC_ADDR_OFFSET = 6,
};

u32 __qce1204_soc_read(struct phy_device *phydev, u32 reg);
void __qce1204_soc_write(struct phy_device *phydev, u32 reg, u32 val);
int __qce1204_soc_modify(struct phy_device *phydev, u32 reg,
	u32 mask, u32 set);
u32 qce1204_soc_read(struct phy_device *phydev, u32 reg);
int qce1204_soc_modify(struct phy_device *phydev, u32 reg,
	u32 mask, u32 set);
#endif /* _QCE1204_H_ */
