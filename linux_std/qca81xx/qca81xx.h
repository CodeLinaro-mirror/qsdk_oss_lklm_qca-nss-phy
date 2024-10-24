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

#ifndef _QCA81XX_H_
#define _QCA81XX_H_

#include <linux/phy.h>

#define QCA8111_PHY		0x004dd1c0
/* in QCOM MDIO bus driver, bit29~31 is for soc type, 2 is for laguna */
/* and bit24~28 is for phy address, 0~23 is for soc address */
#define TO_QCA81XX_PHY_SOC_ADDR(addr, reg)		\
	((BIT(30) | reg) | (addr << 24))

int __qca81xx_phy_debug_write(struct phy_device *phydev,
	unsigned int reg, u16 val);
int qca81xx_phy_debug_write(struct phy_device *phydev,
	unsigned int reg, u16 val);
u32 __qca81xx_soc_read(struct phy_device *phydev, u32 reg);
int __qca81xx_soc_write(struct phy_device *phydev,
	u32 reg, u32 val);
int qca81xx_soc_modify(struct phy_device *phydev, u32 reg,
	u32 mask, u32 set);
#endif /* _QCA81XX_H_ */
